cmake_minimum_required(VERSION 3.21)
project(ClearOSD VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Widgets Qml Quick QuickControls2)
qt_add_executable(ClearOSD WIN32
    app/Application.cpp
    app/Controller.cpp
    main.cpp
    overlay/OverlayStyle.cpp
    platform/WindowsPlatform.cpp
    rtss/RtssClient.cpp
    settings/SettingsStore.cpp
    telemetry/SensorProvider.cpp
    telemetry/RenderedFrameTracker.cpp
    app/Application.h
    app/Controller.h
    model/MetricCatalog.h
    overlay/OverlayStyle.h
    platform/SharedMemory.h
    platform/WindowsPlatform.h
    rtss/RtssClient.h
    settings/SettingsStore.h
    telemetry/SensorProvider.h
    telemetry/RenderedFrameTracker.h
    brightness/BrightnessController.cpp
    brightness/BrightnessController.h
    brightness/core/DisplayTypes.h
    brightness/services/ConfigStore.cpp
    brightness/services/ConfigStore.h
    brightness/services/DisplayService.cpp
    brightness/services/DisplayService.h
    brightness/services/HotkeyManager.cpp
    brightness/services/HotkeyManager.h
    brightness/services/StartupManager.cpp
    brightness/services/StartupManager.h
    app.rc
)
target_include_directories(ClearOSD PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
qt_add_resources(ClearOSD qml PREFIX "/" FILES
    Main.qml
    components/qmldir
    components/Theme.qml
    components/AppearanceCard.qml
    components/BrightnessPage.qml
    components/Caption.qml
    components/ColorSwatchButton.qml
    components/Field.qml
    components/GammaSlider.qml
    components/HotkeyField.qml
    components/Icon.qml
    components/MetricStrip.qml
    components/MetricTile.qml
    components/MetricsCard.qml
    components/MonitorPage.qml
    components/MonitorTile.qml
    components/NavItem.qml
    components/OverlayWindow.qml
    components/PageScroll.qml
    components/PositionCard.qml
    components/PositionGrid.qml
    components/PreviewCard.qml
    components/ProfileChip.qml
    components/SectionCard.qml
    components/SegmentedControl.qml
    components/StatusPill.qml
    components/UiButton.qml
    components/UiComboBox.qml
    components/UiSlider.qml
    components/UiStepper.qml
    components/UiSwitch.qml
    components/UiTextField.qml
)
target_compile_definitions(ClearOSD PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
target_link_libraries(ClearOSD PRIVATE Qt6::Widgets Qt6::Qml Qt6::Quick Qt6::QuickControls2 user32 shell32 gdi32 advapi32 tdh)
if(MINGW)
    target_link_options(ClearOSD PRIVATE -static-libgcc -static-libstdc++)
endif()
