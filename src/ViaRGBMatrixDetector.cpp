/*---------------------------------------------------------*\
| ViaRGBMatrixDetector.cpp                                  |
|                                                           |
|   Hotplug-driven detection of allowlisted VIA keyboards   |
|                                                           |
|   The hidapi hotplug callback runs on hidapi's own event  |
|   thread and may not open devices, so it only queues the |
|   event; opening, handshake and registration happen on    |
|   the detector thread.                                    |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "LogManager.h"
#include "ViaRGBMatrixDetector.h"

#define VIA_LOG(level, ...)     plugin_api->LogEntry(__FILE__, __LINE__, level, __VA_ARGS__)

ViaRGBMatrixDetector::ViaRGBMatrixDetector(OpenRGBPluginAPIInterface* api)
{
    plugin_api  = api;
    running     = false;
}

ViaRGBMatrixDetector::~ViaRGBMatrixDetector()
{
    Stop();
}

void ViaRGBMatrixDetector::Start()
{
    running         = true;
    detector_thread = std::thread(&ViaRGBMatrixDetector::DetectorThread, this);
}

void ViaRGBMatrixDetector::Stop()
{
    if(!detector_thread.joinable())
    {
        return;
    }

    {
        std::lock_guard<std::mutex> guard(event_lock);
        running = false;
    }

    event_cv.notify_one();
    detector_thread.join();
}

int HID_API_CALL ViaRGBMatrixDetector::HotplugCallback(hid_hotplug_callback_handle /*handle*/, hid_device_info* info, hid_hotplug_event event, void* user_data)
{
    ViaRGBMatrixDetector* detector = (ViaRGBMatrixDetector*)user_data;

    via_hotplug_event new_event;
    new_event.arrived   = (event == HID_API_HOTPLUG_EVENT_DEVICE_ARRIVED);
    new_event.path      = info->path;
    new_event.entry     = NULL;

    if(new_event.arrived)
    {
        if(info->usage_page != VIA_RAW_HID_USAGE_PAGE || info->usage != VIA_RAW_HID_USAGE)
        {
            return(0);
        }

        for(unsigned int device_idx = 0; device_idx < via_device_count; device_idx++)
        {
            if(via_devices[device_idx].vid == info->vendor_id && via_devices[device_idx].pid == info->product_id)
            {
                new_event.entry = &via_devices[device_idx];
                break;
            }
        }

        if(new_event.entry == NULL)
        {
            return(0);
        }
    }

    {
        std::lock_guard<std::mutex> guard(detector->event_lock);
        detector->events.push_back(new_event);
    }

    detector->event_cv.notify_one();

    return(0);
}

void ViaRGBMatrixDetector::DetectorThread()
{
    /*-----------------------------------------------------*\
    | Let OpenRGB finish its own detection pass first       |
    \*-----------------------------------------------------*/
    plugin_api->WaitForDetection();

    for(unsigned int device_idx = 0; device_idx < via_device_count; device_idx++)
    {
        hid_hotplug_callback_handle handle;

        if(hid_hotplug_register_callback(via_devices[device_idx].vid, via_devices[device_idx].pid,
                                         HID_API_HOTPLUG_EVENT_DEVICE_ARRIVED | HID_API_HOTPLUG_EVENT_DEVICE_LEFT,
                                         HID_API_HOTPLUG_ENUMERATE, HotplugCallback, this, &handle) == 0)
        {
            hotplug_handles.push_back(handle);
        }
        else
        {
            VIA_LOG(LL_ERROR, "[VIA RGB Matrix] Failed to register hotplug callback for %04X:%04X", via_devices[device_idx].vid, via_devices[device_idx].pid);
        }
    }

    while(true)
    {
        via_hotplug_event event;

        {
            std::unique_lock<std::mutex> lock(event_lock);

            event_cv.wait(lock, [this]{ return(!running || !events.empty()); });

            if(!running)
            {
                break;
            }

            event = events.front();
            events.pop_front();
        }

        if(event.arrived)
        {
            DeviceArrived(event);
        }
        else
        {
            DeviceLeft(event);
        }
    }

    /*-----------------------------------------------------*\
    | Deregistering blocks until a running callback returns |
    \*-----------------------------------------------------*/
    for(std::size_t handle_idx = 0; handle_idx < hotplug_handles.size(); handle_idx++)
    {
        hid_hotplug_deregister_callback(hotplug_handles[handle_idx]);
    }

    hotplug_handles.clear();

    for(std::size_t device_idx = 0; device_idx < devices.size(); device_idx++)
    {
        delete devices[device_idx];
    }

    devices.clear();
}

void ViaRGBMatrixDetector::DeviceArrived(const via_hotplug_event& event)
{
    for(std::size_t device_idx = 0; device_idx < devices.size(); device_idx++)
    {
        if(devices[device_idx]->GetPath() == event.path)
        {
            return;
        }
    }

    hid_device* dev = hid_open_path(event.path.c_str());

    if(dev == NULL)
    {
        VIA_LOG(LL_WARNING, "[VIA RGB Matrix] %s: cannot open %s", event.entry->name, event.path.c_str());
        return;
    }

    ViaRGBMatrixController* controller = new ViaRGBMatrixController(dev, event.path.c_str());

    if(!controller->Handshake())
    {
        VIA_LOG(LL_WARNING, "[VIA RGB Matrix] %s: no VIA handshake on %s", event.entry->name, event.path.c_str());
        delete controller;
        return;
    }

    RGBController_ViaRGBMatrix* device = new RGBController_ViaRGBMatrix(controller, event.entry, plugin_api);

    if(!device->Register())
    {
        delete device;
        return;
    }

    devices.push_back(device);
}

void ViaRGBMatrixDetector::DeviceLeft(const via_hotplug_event& event)
{
    for(std::size_t device_idx = 0; device_idx < devices.size(); device_idx++)
    {
        if(devices[device_idx]->GetPath() == event.path)
        {
            VIA_LOG(LL_INFO, "[VIA RGB Matrix] Device removed: %s", event.path.c_str());
            delete devices[device_idx];
            devices.erase(devices.begin() + device_idx);
            return;
        }
    }
}
