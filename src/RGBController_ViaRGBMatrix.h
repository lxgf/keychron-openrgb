/*---------------------------------------------------------*\
| RGBController_ViaRGBMatrix.h                              |
|                                                           |
|   OpenRGB model for a VIA RGB Matrix keyboard: one zone,  |
|   one LED, Direct mode plus the firmware's effects.       |
|   Registered through the plugin API as a virtual          |
|   controller; HID traffic runs on a private worker thread |
|   so OpenRGB callbacks never block on the device.         |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include "OpenRGBPluginInterface.h"
#include "ViaRGBMatrixController.h"
#include "ViaRGBMatrixDevices.h"

#define VIA_MODE_VALUE_DIRECT           0x100

typedef struct
{
    unsigned char   effect;
    unsigned char   brightness;
    unsigned char   speed;
    unsigned char   hue;
    unsigned char   sat;
} via_rgb_state;

class RGBController_ViaRGBMatrix
{
public:
    RGBController_ViaRGBMatrix(ViaRGBMatrixController* controller_ptr, const via_device_entry* entry, OpenRGBPluginAPIInterface* api);
    ~RGBController_ViaRGBMatrix();

    bool            Register();
    void            Unregister();

    std::string     GetPath();

private:
    ViaRGBMatrixController*     controller;
    const via_device_entry*     device_entry;
    OpenRGBPluginAPIInterface*  plugin_api;
    RGBControllerInterface*     rgb_controller;
    std::vector<mode>           modes;

    std::thread                 worker_thread;
    std::mutex                  state_lock;
    std::condition_variable     state_cv;
    bool                        worker_running;
    bool                        update_pending;
    bool                        save_pending;
    via_rgb_state               desired;
    via_rgb_state               sent;
    bool                        sent_valid;

    void            BuildModes(unsigned char max_effect, const via_rgb_state& current);
    void            QueueState(const via_rgb_state& next, bool save);
    void            UpdateLEDs();
    void            UpdateMode(bool save);
    void            WorkerThread();

    static void     DeviceUpdateLEDsCallback(void* object);
    static void     DeviceUpdateZoneLEDsCallback(void* object, int zone);
    static void     DeviceUpdateSingleLEDCallback(void* object, int led);
    static void     DeviceUpdateModeCallback(void* object);
    static void     DeviceSaveModeCallback(void* object);
};
