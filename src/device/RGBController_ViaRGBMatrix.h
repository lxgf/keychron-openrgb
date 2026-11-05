/*---------------------------------------------------------*\
| RGBController_ViaRGBMatrix.h                              |
|                                                           |
|   Binds a VIA RGB Matrix keyboard to an OpenRGB virtual   |
|   controller: one zone, one LED, Direct mode, the         |
|   firmware's effects and the custom effects. Firmware     |
|   effects and Direct apply synchronously on OpenRGB's     |
|   per-device call thread; custom effects run on the       |
|   effect engine with their settings from the plugin tab.  |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include "OpenRGBPluginInterface.h"
#include "ViaEffectEngine.h"
#include "ViaLightingSync.h"
#include "ViaRGBMatrixController.h"
#include "ViaRGBMatrixDevices.h"
#include "ViaSettings.h"

class RGBController_ViaRGBMatrix
{
public:
    RGBController_ViaRGBMatrix(ViaRGBMatrixController* controller_ptr, const via_device_entry* entry, OpenRGBPluginAPIInterface* api, ViaSettings* settings);
    ~RGBController_ViaRGBMatrix();

    bool                Register();
    void                Unregister();

    const std::string&  GetPath();

private:
    ViaRGBMatrixController*     controller;
    const via_device_entry*     device_entry;
    OpenRGBPluginAPIInterface*  plugin_api;
    RGBControllerInterface*     rgb_controller;
    unsigned char               max_effect;
    ViaLightingSync             sync;
    ViaEffectEngine             engine;

    /*-----------------------------------------------------*\
    | Firmware state saved when Direct or a custom effect   |
    | takes over, put back when the device is unregistered  |
    \*-----------------------------------------------------*/
    bool                        host_driven;
    via_rgb_state               firmware_state;

    void                EnterHostDriven();
    void                OnUpdateLEDs();
    void                OnUpdateMode(bool save);
    void                ApplyFirmwareEffect(int mode_idx, bool save);
    void                RunCustomEffect(int mode_idx);

    static void         DeviceUpdateLEDsCallback(void* object);
    static void         DeviceUpdateZoneLEDsCallback(void* object, int zone);
    static void         DeviceUpdateSingleLEDCallback(void* object, int led);
    static void         DeviceUpdateModeCallback(void* object);
    static void         DeviceSaveModeCallback(void* object);
};
