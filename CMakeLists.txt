cmake_minimum_required(VERSION 3.21)

project(OpenRGBViaRGBMatrixPlugin VERSION 0.3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

if(NOT WIN32)
    message(FATAL_ERROR "Only Windows builds are supported for now")
endif()

# Must match the Qt major/minor of the OpenRGB build the plugin is loaded into (OpenRGB 1.1: Qt 6.8.3)
find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Widgets)

set(OPENRGB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/OpenRGB)
set(HIDAPI_DIR  ${OPENRGB_DIR}/dependencies/hidapi-hotplug-win)

if(NOT EXISTS ${OPENRGB_DIR}/OpenRGBPluginInterface.h)
    message(FATAL_ERROR "OpenRGB submodule missing: git submodule update --init")
endif()

find_package(Git QUIET)
set(VIA_PLUGIN_COMMIT "")
if(GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    OUTPUT_VARIABLE VIA_PLUGIN_COMMIT
                    OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
endif()

# OpenRGB reads this metadata before loading the DLL (OpenRGBPluginAPIVersion and Id are required)
configure_file(src/plugin/ViaRGBMatrixPlugin.json.in ${CMAKE_CURRENT_BINARY_DIR}/ViaRGBMatrixPlugin.json @ONLY)

add_library(OpenRGBViaRGBMatrixPlugin SHARED
    src/plugin/ViaRGBMatrixPlugin.cpp
    src/plugin/ViaRGBMatrixPlugin.h
    src/plugin/ViaWorker.cpp
    src/plugin/ViaWorker.h
    src/detection/ViaRGBMatrixDetector.cpp
    src/detection/ViaRGBMatrixDetector.h
    src/device/RGBController_ViaRGBMatrix.cpp
    src/device/RGBController_ViaRGBMatrix.h
    src/device/ViaRGBMatrixModes.cpp
    src/device/ViaRGBMatrixModes.h
    src/effects/ViaAudioCapture.cpp
    src/effects/ViaAudioCapture.h
    src/effects/ViaCustomEffect.h
    src/effects/ViaCustomEffectFactories.h
    src/effects/ViaCustomEffects.cpp
    src/effects/ViaCustomEffects.h
    src/effects/ViaEffectAudio.cpp
    src/effects/ViaEffectDayNight.cpp
    src/effects/ViaEffectEngine.cpp
    src/effects/ViaEffectEngine.h
    src/effects/ViaEffectHueCycle.cpp
    src/effects/ViaEffectPlaylist.cpp
    src/effects/ViaEffectSystemMonitor.cpp
    src/effects/ViaSystemSensors.cpp
    src/effects/ViaSystemSensors.h
    src/settings/ViaPlaylistEditor.cpp
    src/settings/ViaPlaylistEditor.h
    src/settings/ViaSettings.cpp
    src/settings/ViaSettings.h
    src/settings/ViaSettingsWidget.cpp
    src/settings/ViaSettingsWidget.h
    src/settings/ViaTranslation.cpp
    src/settings/ViaTranslation.h
    src/catalog/ViaColor.cpp
    src/catalog/ViaColor.h
    src/catalog/ViaRGBMatrixDevices.cpp
    src/catalog/ViaRGBMatrixDevices.h
    src/protocol/ViaLightingSync.cpp
    src/protocol/ViaLightingSync.h
    src/protocol/ViaRGBMatrixController.cpp
    src/protocol/ViaRGBMatrixController.h
    src/common/ViaLog.cpp
    src/common/ViaLog.h
)

# Layers, top to bottom; each one only includes the layers below it
target_include_directories(OpenRGBViaRGBMatrixPlugin PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    src/plugin
    src/detection
    src/device
    src/effects
    src/settings
    src/catalog
    src/protocol
    src/common
    ${OPENRGB_DIR}
    ${OPENRGB_DIR}/RGBController
    ${OPENRGB_DIR}/dependencies/json
    ${HIDAPI_DIR}/include
)

target_compile_definitions(OpenRGBViaRGBMatrixPlugin PRIVATE
    VIA_PLUGIN_VERSION="${PROJECT_VERSION}"
    VIA_PLUGIN_COMMIT="${VIA_PLUGIN_COMMIT}"
    _CRT_SECURE_NO_WARNINGS
    NOMINMAX
)

target_compile_options(OpenRGBViaRGBMatrixPlugin PRIVATE /utf-8 /W3)

# Link against the same hidapi build that ships with OpenRGB (hidapi-hotplug.dll is already loaded in the process)
target_link_libraries(OpenRGBViaRGBMatrixPlugin PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    ${HIDAPI_DIR}/x64/hidapi-hotplug.lib
    ole32
    pdh
)
