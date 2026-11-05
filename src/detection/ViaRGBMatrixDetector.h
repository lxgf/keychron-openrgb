/*---------------------------------------------------------*\
| ViaRGBMatrixDetector.h                                    |
|                                                           |
|   Hotplug-driven detection of allowlisted VIA keyboards.  |
|   Owns the registered devices; the device list is only    |
|   touched on the worker thread.                           |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <vector>
#include <hidapi.h>
#include "OpenRGBPluginInterface.h"
#include "RGBController_ViaRGBMatrix.h"
#include "ViaWorker.h"

class ViaRGBMatrixDetector
{
public:
    ViaRGBMatrixDetector(OpenRGBPluginAPIInterface* api, ViaWorker* worker_ptr);

    void            Start();
    void            Stop();

private:
    OpenRGBPluginAPIInterface*                  plugin_api;
    ViaWorker*                                  worker;
    std::vector<hid_hotplug_callback_handle>    hotplug_handles;
    std::vector<RGBController_ViaRGBMatrix*>    devices;
    bool                                        stopped;

    void            StartHotplug();
    void            StopAll();
    void            DeviceArrived(const std::string& path, const via_device_entry* entry);
    void            DeviceLeft(const std::string& path);
    void            RemoveDevice(std::size_t device_idx);

    static int HID_API_CALL HotplugCallback(hid_hotplug_callback_handle handle, hid_device_info* info, hid_hotplug_event event, void* user_data);
};
