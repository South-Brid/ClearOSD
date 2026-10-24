#pragma once
#include <cstdint>
#include <memory>

// Counts application instrumentation, never DXGI presents or inferred FG ratios.
// NVIDIA PCL/Reflex marker definitions are documented by PresentMon/Streamline.
class RenderedFrameTracker {
public:
    enum class Status { Stopped, NoTarget, WaitingForMarkers, WarmingUp, Ready,
                        Stale, AccessDenied, StartFailed, TraceFailed, EventsLost };
    struct Sample {
        double fps = -1.0;
        double simulationFps = -1.0; // Diagnostic only: simulation may run independently.
        Status status = Status::Stopped;
        std::uint32_t errorCode = 0;
        std::uint32_t processId = 0;
        std::uint64_t renderedFrames = 0;
        std::uint64_t simulationFrames = 0;
        std::uint64_t providerEvents = 0;
        std::uint64_t decodeFailures = 0;
        std::uint32_t recentRenderedFrames = 0;
        std::uint64_t lastMarkerAgeMs = 0;
        std::uint64_t lastMarkerEventAgeMs = 0;
    };
    RenderedFrameTracker();
    ~RenderedFrameTracker();
    RenderedFrameTracker(const RenderedFrameTracker&) = delete;
    RenderedFrameTracker& operator=(const RenderedFrameTracker&) = delete;
    bool start();
    void stop();
    void setTargetProcess(std::uint32_t processId);
    Sample sample() const; // Nonblocking except for a small in-memory mutex.
private:
    struct Impl;
    std::unique_ptr<Impl> d;
};
