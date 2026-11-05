cmake_minimum_required(VERSION 3.21)

project(OpenRGBViaRGBMatrixPlugin VERSION 0.1.0 LANGUAGES CXX)

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

add_library(OpenRGBViaRGBMatrixPlugin SHARED
    src/ViaRGBMatrixPlugin.cpp
    src/ViaRGBMatrixPlugin.h
    src/ViaRGBMatrixDetector.cpp
    src/ViaRGBMatrixDetector.h
    src/RGBController_ViaRGBMatrix.cpp
    src/RGBController_ViaRGBMatrix.h
    src/ViaLightingSync.cpp
    src/ViaLightingSync.h
    src/ViaRGBMatrixModes.cpp
    src/ViaRGBMatrixModes.h
    src/ViaRGBMatrixController.cpp
    src/ViaRGBMatrixController.h
    src/ViaRGBMatrixDevices.cpp
    src/ViaRGBMatrixDevices.h
    src/ViaColor.cpp
    src/ViaColor.h
    src/ViaWorker.cpp
    src/ViaWorker.h
    src/ViaLog.cpp
    src/ViaLog.h
)

target_include_directories(OpenRGBViaRGBMatrixPlugin PRIVATE
    src
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
)
