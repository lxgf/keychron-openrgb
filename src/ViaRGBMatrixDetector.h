/*---------------------------------------------------------*\
| ViaRGBMatrixDetector.h                                    |
|                                                           |
|   Hotplug-driven detection of allowlisted VIA keyboards   |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <condition_variable>
#include <deque>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <hidapi.h>
#include "OpenRGBPluginInterface.h"
#include "RGBController_ViaRGBMatrix.h"

typedef struct
{
    bool                    arrived;
    std::string             path;
    const via_device_entry* entry;
} via_hotplug_event;

class ViaRGBMatrixDetector
{
public:
    ViaRGBMatrixDetector(OpenRGBPluginAPIInterface* api);
    ~ViaRGBMatrixDetector();

    void            Start();
    void            Stop();

private:
    OpenRGBPluginAPIInterface*                  plugin_api;
    std::vector<RGBController_ViaRGBMatrix*>    devices;
    std::vector<hid_hotplug_callback_handle>    hotplug_handles;

    std::thread                                 detector_thread;
    std::mutex                                  event_lock;
    std::condition_variable                     event_cv;
    std::deque<via_hotplug_event>               events;
    bool                                        running;

    void            DetectorThread();
    void            DeviceArrived(const via_hotplug_event& event);
    void            DeviceLeft(const via_hotplug_event& event);

    static int HID_API_CALL HotplugCallback(hid_hotplug_callback_handle handle, hid_device_info* info, hid_hotplug_event event, void* user_data);
};
