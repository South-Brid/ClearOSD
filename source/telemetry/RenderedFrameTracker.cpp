#include "RenderedFrameTracker.h"
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <evntrace.h>
#include <evntcons.h>
#include <tdh.h>
#include <atomic>
#include <cmath>
#include <cstring>
#include <deque>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

namespace clearosd_frame_trace {
// PresentMon: PresentData/ETW/Nvidia_PCL.h and HandlePclEvent.
constexpr GUID pclGuid = {0x0d216f06, 0x82a6, 0x4d49,
                         {0xbc, 0x4f, 0x8f, 0x38, 0xae, 0x56, 0xef, 0xab}};
constexpr std::uint32_t simulationStart = 0;
constexpr std::uint32_t renderSubmitStart = 2;
constexpr ULONGLONG staleMs = 1500;
constexpr ULONGLONG lostRecoveryMs = 5000;

std::uint64_t qpcNow() {
    LARGE_INTEGER value{};
    QueryPerformanceCounter(&value);
    return static_cast<std::uint64_t>(value.QuadPart);
}
bool numericProperty(EVENT_RECORD* record, const wchar_t* name, std::uint64_t& value) {
    PROPERTY_DATA_DESCRIPTOR descriptor{};
    descriptor.PropertyName = reinterpret_cast<ULONGLONG>(name);
    descriptor.ArrayIndex = ULONG_MAX;
    ULONG bytes = 0;
    if (TdhGetPropertySize(record, 0, nullptr, 1, &descriptor, &bytes) != ERROR_SUCCESS ||
        (bytes != sizeof(std::uint32_t) && bytes != sizeof(std::uint64_t))) return false;
    value = 0;
    return TdhGetProperty(record, 0, nullptr, 1, &descriptor, bytes,
                          reinterpret_cast<PBYTE>(&value)) == ERROR_SUCCESS;
}
std::wstring boundedString(const std::vector<unsigned char>& buffer, ULONG offset) {
    if (!offset || offset >= buffer.size() || offset % sizeof(wchar_t)) return {};
    const auto* text = reinterpret_cast<const wchar_t*>(buffer.data() + offset);
    const auto capacity = (buffer.size() - offset) / sizeof(wchar_t);
    for (std::size_t i = 0; i < capacity; ++i)
        if (!text[i]) return std::wstring(text, i);
    return {};
}
bool markerEvent(EVENT_RECORD* record) {
    ULONG bytes = 0;
    if (TdhGetEventInformation(record, 0, nullptr, nullptr, &bytes) != ERROR_INSUFFICIENT_BUFFER ||
        bytes < sizeof(TRACE_EVENT_INFO) || bytes > 1024 * 1024) return false;
    std::vector<unsigned char> buffer(bytes);
    auto* info = reinterpret_cast<TRACE_EVENT_INFO*>(buffer.data());
    if (TdhGetEventInformation(record, 0, nullptr, info, &bytes) != ERROR_SUCCESS) return false;
    // Modern TDH's EventNameOffset aliases the old ActivityIDNameOffset member.
    // MinGW exposes that old name; prefer this offset, then older TLG task names.
    auto name = boundedString(buffer, info->ActivityIDNameOffset);
    if (name.empty()) name = boundedString(buffer, info->TaskNameOffset);
    return name == L"PCLStatsEvent" || name == L"ReflexStatsEvent";
}
struct TraceProperties {
    EVENT_TRACE_PROPERTIES properties{};
    wchar_t name[160]{};
    wchar_t logFileName[1024]{};
};
struct TraceQueryProperties {
    EVENT_TRACE_PROPERTIES properties{};
    wchar_t loggerName[1024]{};
    wchar_t logFileName[1024]{};
};
struct Marker {
    std::uint64_t timestamp;
    std::uint64_t frameId;
};
struct MarkerWindow {
    std::deque<Marker> frames;
    std::unordered_set<std::uint64_t> ids;
    std::uint64_t total = 0;
    ULONGLONG receivedAt = 0;
    void clear() { frames.clear(); ids.clear(); total = 0; receivedAt = 0; }
    void add(std::uint64_t timestamp, std::uint64_t frameId, std::uint64_t frequency) {
        // QPC ordering is required; delayed/duplicate instrumentation isn't a new frame.
        if ((!frames.empty() && timestamp <= frames.back().timestamp) || ids.count(frameId)) return;
        const auto cutoff = timestamp > 2 * frequency ? timestamp - 2 * frequency : 0;
        while (!frames.empty() && frames.front().timestamp < cutoff) {
            ids.erase(frames.front().frameId);
            frames.pop_front();
        }
        ids.insert(frameId);
        frames.push_back({timestamp, frameId});
        ++total;
        receivedAt = GetTickCount64();
    }
    double cadence(std::uint64_t frequency, std::uint64_t now, ULONGLONG tick) const {
        if (frames.size() < 6 || !receivedAt || tick - receivedAt > staleMs) return -1;
        if (frames.back().timestamp > now ||
            now - frames.back().timestamp > frequency * staleMs / 1000) return -1;
        const auto span = frames.back().timestamp - frames.front().timestamp;
        if (span < frequency * 350 / 1000) return -1;
        const double fps = double(frames.size() - 1) * double(frequency) / double(span);
        return std::isfinite(fps) && fps > 0 && fps < 5000 ? fps : -1;
    }
};
struct LossCounters {
    ULONG events = 0;
    ULONG realtimeBuffers = 0;
    ULONG logBuffers = 0;
    bool update(const EVENT_TRACE_PROPERTIES& current) {
        const bool lost = current.EventsLost > events ||
                          current.RealTimeBuffersLost > realtimeBuffers ||
                          current.LogBuffersLost > logBuffers;
        events = current.EventsLost;
        realtimeBuffers = current.RealTimeBuffersLost;
        logBuffers = current.LogBuffersLost;
        return lost;
    }
};
}
using namespace clearosd_frame_trace;

struct RenderedFrameTracker::Impl {
    mutable std::mutex mutex;
    std::atomic<bool> stopping{false};
    std::atomic<bool> running{false};
    TRACEHANDLE session = 0;
    TRACEHANDLE trace = INVALID_PROCESSTRACE_HANDLE;
    TraceProperties properties;
    std::thread worker;
    std::thread lossMonitor;
    HANDLE stopMonitor = nullptr;
    std::uint64_t frequency = 0;
    std::uint32_t target = 0;
    std::uint64_t targetSince = 0;
    Status failure = Status::Stopped;
    std::uint32_t error = 0;
    MarkerWindow rendered;
    MarkerWindow simulation;
    std::uint64_t events = 0;
    std::uint64_t decodeFailures = 0;
    ULONGLONG lastLossAt = 0;

    void resetMeasurements() {
        // Session loss quarantine survives foreground/target changes.
        rendered.clear(); simulation.clear(); events = 0; decodeFailures = 0;
    }
    static void WINAPI eventCallback(EVENT_RECORD* record) {
        auto* self = static_cast<Impl*>(record->UserContext);
        if (!self || self->stopping || !IsEqualGUID(record->EventHeader.ProviderId, pclGuid)) return;
        {
            std::lock_guard<std::mutex> lock(self->mutex);
            if (!self->target || record->EventHeader.ProcessId != self->target) return;
            ++self->events;
        }
        // Ignore init/shutdown/input events: only per-frame marker events are counted.
        if (!markerEvent(record)) return;
        std::uint64_t marker = 0, frameId = 0;
        if (!numericProperty(record, L"Marker", marker) || !numericProperty(record, L"FrameID", frameId)) {
            std::lock_guard<std::mutex> lock(self->mutex);
            ++self->decodeFailures;
            return;
        }
        if (marker != simulationStart && marker != renderSubmitStart) return;
        const auto timestamp = static_cast<std::uint64_t>(record->EventHeader.TimeStamp.QuadPart);
        const auto now = qpcNow();
        if (!timestamp || timestamp > now) return;
        std::lock_guard<std::mutex> lock(self->mutex);
        if (record->EventHeader.ProcessId != self->target || timestamp < self->targetSince) return;
        auto& window = marker == renderSubmitStart ? self->rendered : self->simulation;
        window.add(timestamp, frameId, self->frequency);
    }
    static ULONG WINAPI bufferCallback(EVENT_TRACE_LOGFILEW*) {
        // Realtime buffer callbacks don't need to inspect Context. CloseTrace and
        // stopping our owned session interrupt ProcessTrace during shutdown.
        return TRUE;
    }
    void fail(Status status, ULONG code) {
        std::lock_guard<std::mutex> lock(mutex);
        failure = status; error = code;
    }
};

RenderedFrameTracker::RenderedFrameTracker() : d(std::make_unique<Impl>()) {}
RenderedFrameTracker::~RenderedFrameTracker() { stop(); }

bool RenderedFrameTracker::start() {
    if (d->running) return true;
    stop();
    {
        std::lock_guard<std::mutex> lock(d->mutex);
        d->resetMeasurements(); d->lastLossAt = 0;
        d->targetSince = qpcNow();
        d->error = 0; d->failure = Status::WaitingForMarkers;
    }
    LARGE_INTEGER frequency{};
    if (!QueryPerformanceFrequency(&frequency) || frequency.QuadPart <= 0) {
        d->fail(Status::StartFailed, ERROR_NOT_SUPPORTED); return false;
    }
    d->frequency = static_cast<std::uint64_t>(frequency.QuadPart);
    d->properties = {};
    swprintf_s(d->properties.name, L"ClearOSD.RenderFrames.%lu.%llu",
               GetCurrentProcessId(), static_cast<unsigned long long>(qpcNow()));
    auto& properties = d->properties.properties;
    properties.Wnode.BufferSize = sizeof(TraceProperties);
    properties.Wnode.ClientContext = 1; // QPC, also retained by RAW_TIMESTAMP.
    properties.Wnode.Flags = WNODE_FLAG_TRACED_GUID;
    properties.BufferSize = 64;
    properties.MinimumBuffers = 64;
    properties.MaximumBuffers = 256;
    properties.FlushTimer = 1;
    // Isolate our buffers from other consumers of the same PCL provider. A stale
    // FrameView session must not prevent this owned session from receiving frames.
    properties.LogFileMode = EVENT_TRACE_REAL_TIME_MODE | EVENT_TRACE_INDEPENDENT_SESSION_MODE;
    properties.LoggerNameOffset = offsetof(TraceProperties, name);
    properties.LogFileNameOffset = offsetof(TraceProperties, logFileName);
    auto result = StartTraceW(&d->session, d->properties.name, &properties);
    if (result != ERROR_SUCCESS) {
        d->session = 0;
        d->fail(result == ERROR_ACCESS_DENIED ? Status::AccessDenied : Status::StartFailed, result);
        return false;
    }
    // Same provider settings as PresentMon --track_pc_latency. This doesn't change
    // game, Reflex, RTSS, NVIDIA profile or any other ETW session.
    result = EnableTraceEx2(d->session, &pclGuid, EVENT_CONTROL_CODE_ENABLE_PROVIDER,
                           TRACE_LEVEL_VERBOSE, 0, 0, 0, nullptr);
    if (result != ERROR_SUCCESS) {
        ControlTraceW(d->session, d->properties.name, &properties, EVENT_TRACE_CONTROL_STOP);
        d->session = 0;
        d->fail(result == ERROR_ACCESS_DENIED ? Status::AccessDenied : Status::StartFailed, result);
        return false;
    }
    EVENT_TRACE_LOGFILEW logfile{};
    logfile.LoggerName = d->properties.name;
    logfile.ProcessTraceMode = PROCESS_TRACE_MODE_EVENT_RECORD | PROCESS_TRACE_MODE_RAW_TIMESTAMP |
                              PROCESS_TRACE_MODE_REAL_TIME;
    logfile.EventRecordCallback = &Impl::eventCallback;
    logfile.BufferCallback = &Impl::bufferCallback;
    logfile.Context = d.get();
    d->trace = OpenTraceW(&logfile);
    if (d->trace == INVALID_PROCESSTRACE_HANDLE) {
        result = GetLastError();
        ControlTraceW(d->session, d->properties.name, &properties, EVENT_TRACE_CONTROL_STOP);
        d->session = 0; d->fail(Status::StartFailed, result); return false;
    }
    d->stopMonitor = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!d->stopMonitor) {
        result = GetLastError();
        CloseTrace(d->trace); d->trace = INVALID_PROCESSTRACE_HANDLE;
        ControlTraceW(d->session, d->properties.name, &properties, EVENT_TRACE_CONTROL_STOP);
        d->session = 0; d->fail(Status::StartFailed, result); return false;
    }
    d->stopping = false;
    d->running = true;
    d->worker = std::thread([self = d.get()] {
        const auto result = ProcessTrace(&self->trace, 1, nullptr, nullptr);
        std::lock_guard<std::mutex> lock(self->mutex);
        if (!self->stopping) { self->failure = Status::TraceFailed; self->error = result; }
        self->running = false;
    });
    // EVENT_TRACE_LOGFILE::EventsLost is documented as unused. Query authoritative
    // cumulative loss counters on our own session, using a separate output buffer.
    // The handle remains alive until this monitor has been joined by stop().
    d->lossMonitor = std::thread([self = d.get(), session = d->session,
                                 guid = d->properties.properties.Wnode.Guid] {
        LossCounters previous;
        while (WaitForSingleObject(self->stopMonitor, 500) == WAIT_TIMEOUT) {
            TraceQueryProperties query{};
            query.properties.Wnode.BufferSize = sizeof(query);
            query.properties.Wnode.Guid = guid;
            query.properties.LoggerNameOffset = offsetof(TraceQueryProperties, loggerName);
            query.properties.LogFileNameOffset = offsetof(TraceQueryProperties, logFileName);
            // Force delivery of only our buffers: sparse PCL traffic may otherwise
            // sit for ~2s, older than our freshness gate, despite FlushTimer=1.
            const auto flushed = ControlTraceW(session, nullptr, &query.properties, EVENT_TRACE_CONTROL_FLUSH);
            const auto result = flushed == ERROR_SUCCESS ?
                ControlTraceW(session, nullptr, &query.properties, EVENT_TRACE_CONTROL_QUERY) : flushed;
            if (self->stopping) return;
            std::lock_guard<std::mutex> lock(self->mutex);
            if (result != ERROR_SUCCESS) {
                self->failure = Status::TraceFailed; self->error = result;
                continue;
            }
            if (self->failure == Status::TraceFailed && self->running) {
                self->failure = Status::WaitingForMarkers; self->error = 0;
            }
            if (previous.update(query.properties)) {
                self->rendered.clear(); self->simulation.clear();
                self->lastLossAt = GetTickCount64();
            }
        }
    });
    return true;
}

void RenderedFrameTracker::stop() {
    d->stopping = true;
    if (d->stopMonitor) SetEvent(d->stopMonitor);
    if (d->lossMonitor.joinable()) d->lossMonitor.join();
    if (d->session) {
        ControlTraceW(d->session, d->properties.name, &d->properties.properties, EVENT_TRACE_CONTROL_STOP);
        d->session = 0;
    }
    if (d->trace != INVALID_PROCESSTRACE_HANDLE) CloseTrace(d->trace);
    if (d->worker.joinable()) d->worker.join();
    d->trace = INVALID_PROCESSTRACE_HANDLE;
    if (d->stopMonitor) { CloseHandle(d->stopMonitor); d->stopMonitor = nullptr; }
    d->running = false;
    std::lock_guard<std::mutex> lock(d->mutex);
    d->failure = Status::Stopped; d->error = 0; d->resetMeasurements(); d->lastLossAt = 0;
}

void RenderedFrameTracker::setTargetProcess(std::uint32_t processId) {
    std::lock_guard<std::mutex> lock(d->mutex);
    if (d->target == processId) return;
    d->target = processId;
    d->targetSince = qpcNow();
    d->resetMeasurements();
}

RenderedFrameTracker::Sample RenderedFrameTracker::sample() const {
    Sample result;
    std::lock_guard<std::mutex> lock(d->mutex);
    result.processId = d->target; result.errorCode = d->error;
    result.renderedFrames = d->rendered.total; result.simulationFrames = d->simulation.total;
    result.providerEvents = d->events; result.decodeFailures = d->decodeFailures;
    result.recentRenderedFrames = static_cast<std::uint32_t>(d->rendered.frames.size());
    result.status = d->failure;
    if (!d->running) return result;
    if (d->failure == Status::TraceFailed) return result;
    if (!d->target) { result.status = Status::NoTarget; return result; }
    const auto tick = GetTickCount64();
    const auto now = qpcNow();
    if (d->lastLossAt && tick - d->lastLossAt < lostRecoveryMs) {
        result.status = Status::EventsLost; return result;
    }
    result.simulationFps = d->simulation.cadence(d->frequency, now, tick);
    if (!d->rendered.total) { result.status = Status::WaitingForMarkers; return result; }
    result.lastMarkerAgeMs = tick - d->rendered.receivedAt;
    if (!d->rendered.frames.empty() && now >= d->rendered.frames.back().timestamp)
        result.lastMarkerEventAgeMs = (now - d->rendered.frames.back().timestamp) * 1000 / d->frequency;
    result.fps = d->rendered.cadence(d->frequency, now, tick);
    if (result.fps >= 0) result.status = Status::Ready;
    else if (result.lastMarkerAgeMs > staleMs ||
             (!d->rendered.frames.empty() && now > d->rendered.frames.back().timestamp +
              d->frequency * staleMs / 1000)) result.status = Status::Stale;
    else result.status = Status::WarmingUp;
    return result;
}
