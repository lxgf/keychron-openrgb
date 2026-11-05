/*---------------------------------------------------------*\
| ViaRGBMatrixDetector.cpp                                  |
|                                                           |
|   Hotplug-driven detection of allowlisted VIA keyboards.  |
|                                                           |
|   The hidapi hotplug callback runs on hidapi's own event  |
|   thread and may not open devices, so it only posts a     |
|   task; opening, handshake and registration happen on the |
|   worker.                                                 |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "ViaLog.h"
#include "ViaRGBMatrixDetector.h"

ViaRGBMatrixDetector::ViaRGBMatrixDetector(OpenRGBPluginAPIInterface* api, ViaWorker* worker_ptr, ViaSettings* settings_ptr)
{
    plugin_api  = api;
    worker      = worker_ptr;
    settings    = settings_ptr;
    stopped     = false;
}

void ViaRGBMatrixDetector::Start()
{
    worker->Post([this]{ StartHotplug(); });
}

/*---------------------------------------------------------*\
| Queues teardown; it has run once the worker is stopped    |
\*---------------------------------------------------------*/
void ViaRGBMatrixDetector::Stop()
{
    worker->Post([this]{ StopAll(); });
}

void ViaRGBMatrixDetector::StartHotplug()
{
    /*-----------------------------------------------------*\
    | Let OpenRGB finish its own detection pass first       |
    \*-----------------------------------------------------*/
    plugin_api->WaitForDetection();

    for(unsigned int device_idx = 0; device_idx < via_device_count; device_idx++)
    {
        hid_hotplug_callback_handle handle;

        int result = hid_hotplug_register_callback(via_devices[device_idx].vid, via_devices[device_idx].pid,
                                                   HID_API_HOTPLUG_EVENT_DEVICE_ARRIVED | HID_API_HOTPLUG_EVENT_DEVICE_LEFT,
                                                   HID_API_HOTPLUG_ENUMERATE, HotplugCallback, this, &handle);

        if(result == 0)
        {
            hotplug_handles.push_back(handle);
        }
        else
        {
            VIA_LOG(LL_ERROR, "failed to register hotplug callback for %04X:%04X", via_devices[device_idx].vid, via_devices[device_idx].pid);
        }
    }
}

void ViaRGBMatrixDetector::StopAll()
{
    /*-----------------------------------------------------*\
    | Arrivals queued before deregistration are dropped     |
    \*-----------------------------------------------------*/
    stopped = true;

    /*-----------------------------------------------------*\
    | Deregistering waits for a running callback to return  |
    \*-----------------------------------------------------*/
    for(std::size_t handle_idx = 0; handle_idx < hotplug_handles.size(); handle_idx++)
    {
        hid_hotplug_deregister_callback(hotplug_handles[handle_idx]);
    }

    hotplug_handles.clear();

    while(!devices.empty())
    {
        RemoveDevice(devices.size() - 1);
    }
}

int HID_API_CALL ViaRGBMatrixDetector::HotplugCallback(hid_hotplug_callback_handle /*handle*/, hid_device_info* info, hid_hotplug_event event, void* user_data)
{
    ViaRGBMatrixDetector*   detector    = (ViaRGBMatrixDetector*)user_data;
    std::string             path        = info->path;

    if(event == HID_API_HOTPLUG_EVENT_DEVICE_LEFT)
    {
        detector->worker->Post([detector, path]{ detector->DeviceLeft(path); });
        return(0);
    }

    if(info->usage_page != VIA_RAW_HID_USAGE_PAGE || info->usage != VIA_RAW_HID_USAGE)
    {
        return(0);
    }

    const via_device_entry* entry = via_find_device(info->vendor_id, info->product_id);

    if(entry != NULL)
    {
        detector->worker->Post([detector, path, entry]{ detector->DeviceArrived(path, entry); });
    }

    return(0);
}

void ViaRGBMatrixDetector::DeviceArrived(const std::string& path, const via_device_entry* entry)
{
    if(stopped)
    {
        return;
    }

    for(std::size_t device_idx = 0; device_idx < devices.size(); device_idx++)
    {
        if(devices[device_idx]->GetPath() == path)
        {
            return;
        }
    }

    hid_device* dev = hid_open_path(path.c_str());

    if(dev == NULL)
    {
        VIA_LOG(LL_WARNING, "%s: cannot open %s", entry->name, path.c_str());
        return;
    }

    ViaRGBMatrixController* controller = new ViaRGBMatrixController(dev, path.c_str());

    if(!controller->Handshake())
    {
        VIA_LOG(LL_WARNING, "%s: no VIA handshake on %s", entry->name, path.c_str());
        delete controller;
        return;
    }

    RGBController_ViaRGBMatrix* device = new RGBController_ViaRGBMatrix(controller, entry, plugin_api, settings);

    if(!device->Register())
    {
        delete device;
        return;
    }

    devices.push_back(device);
}

void ViaRGBMatrixDetector::DeviceLeft(const std::string& path)
{
    for(std::size_t device_idx = 0; device_idx < devices.size(); device_idx++)
    {
        if(devices[device_idx]->GetPath() == path)
        {
            VIA_LOG(LL_INFO, "device removed: %s", path.c_str());
            RemoveDevice(device_idx);
            return;
        }
    }
}

void ViaRGBMatrixDetector::RemoveDevice(std::size_t device_idx)
{
    delete devices[device_idx];
    devices.erase(devices.begin() + device_idx);
}
