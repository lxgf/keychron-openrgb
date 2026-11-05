/*---------------------------------------------------------*\
| RGBController_ViaRGBMatrix.h                              |
|                                                           |
|   Binds a VIA RGB Matrix keyboard to an OpenRGB virtual   |
|   controller: one zone, one LED, Direct mode plus the     |
|   firmware's effects. Translates OpenRGB callbacks into   |
|   synchronous lighting updates on OpenRGB's per-device    |
|   call thread.                                            |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include "OpenRGBPluginInterface.h"
#include "ViaLightingSync.h"
#include "ViaRGBMatrixController.h"
#include "ViaRGBMatrixDevices.h"

class RGBController_ViaRGBMatrix
{
public:
    RGBController_ViaRGBMatrix(ViaRGBMatrixController* controller_ptr, const via_device_entry* entry, OpenRGBPluginAPIInterface* api);
    ~RGBController_ViaRGBMatrix();

    bool                Register();
    void                Unregister();

    const std::string&  GetPath();

private:
    ViaRGBMatrixController*     controller;
    const via_device_entry*     device_entry;
    OpenRGBPluginAPIInterface*  plugin_api;
    RGBControllerInterface*     rgb_controller;
    ViaLightingSync             sync;

    void                OnUpdateLEDs();
    void                OnUpdateMode(bool save);

    static void         DeviceUpdateLEDsCallback(void* object);
    static void         DeviceUpdateZoneLEDsCallback(void* object, int zone);
    static void         DeviceUpdateSingleLEDCallback(void* object, int led);
    static void         DeviceUpdateModeCallback(void* object);
    static void         DeviceSaveModeCallback(void* object);
};
