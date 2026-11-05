/*---------------------------------------------------------*\
| RGBController_ViaRGBMatrix.cpp                            |
|                                                           |
|   Binds a VIA RGB Matrix keyboard to an OpenRGB virtual   |
|   controller                                              |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstdio>
#include "RGBController_ViaRGBMatrix.h"
#include "ViaColor.h"
#include "ViaLog.h"
#include "ViaRGBMatrixModes.h"

RGBController_ViaRGBMatrix::RGBController_ViaRGBMatrix(ViaRGBMatrixController* controller_ptr, const via_device_entry* entry, OpenRGBPluginAPIInterface* api, ViaSettings* settings)
    : sync(controller_ptr), engine(&sync, settings)
{
    controller      = controller_ptr;
    device_entry    = entry;
    plugin_api      = api;
    rgb_controller  = NULL;
    max_effect      = 0;
}

RGBController_ViaRGBMatrix::~RGBController_ViaRGBMatrix()
{
    Unregister();
    delete controller;
}

const std::string& RGBController_ViaRGBMatrix::GetPath()
{
    return(controller->GetPath());
}

bool RGBController_ViaRGBMatrix::Register()
{
    via_rgb_state current;

    if(!controller->ReadState(&current))
    {
        VIA_LOG(LL_ERROR, "%s: failed to read lighting state", device_entry->name);
        return(false);
    }

    if(!controller->ProbeMaxEffect(&max_effect))
    {
        VIA_LOG(LL_ERROR, "%s: effect count probe failed", device_entry->name);
        return(false);
    }

    VIA_LOG(LL_INFO, "%s: protocol 0x%04X, effect %u of %u, brightness %u, speed %u, hue %u, sat %u",
            device_entry->name, controller->GetProtocolVersion(), current.effect, max_effect,
            current.brightness, current.speed, current.hue, current.sat);

    sync.Init(current);

    via_effect_context context;
    context.entry       = device_entry;
    context.max_effect  = max_effect;
    engine.SetContext(context);

    char version[16];
    snprintf(version, sizeof(version), "VIA 0x%04X", controller->GetProtocolVersion());

    zone keyboard_zone;
    keyboard_zone.name          = "Keyboard";
    keyboard_zone.type          = ZONE_TYPE_SINGLE;
    keyboard_zone.leds_count    = 1;
    keyboard_zone.leds_min      = 1;
    keyboard_zone.leds_max      = 1;

    led keyboard_led;
    keyboard_led.name           = "Keyboard";

    RGBController_Setup setup{};
    setup.name                  = device_entry->name;
    setup.vendor                = device_entry->vendor;
    setup.description           = "QMK/VIA RGB Matrix keyboard (stock firmware)";
    setup.version               = version;
    setup.serial                = controller->GetSerial();
    setup.location              = controller->GetLocation();
    setup.type                  = DEVICE_TYPE_KEYBOARD;
    setup.flags                 = CONTROLLER_FLAG_VIRTUAL;
    setup.modes                 = via_build_modes(device_entry, max_effect, current);
    setup.active_mode           = (current.effect <= max_effect) ? via_effect_to_mode(current.effect) : VIA_MODE_DIRECT;
    setup.zones.push_back(keyboard_zone);
    setup.leds.push_back(keyboard_led);

    setup.object_ptr            = this;
    setup.DeviceUpdateLEDs      = DeviceUpdateLEDsCallback;
    setup.DeviceUpdateZoneLEDs  = DeviceUpdateZoneLEDsCallback;
    setup.DeviceUpdateSingleLED = DeviceUpdateSingleLEDCallback;
    setup.DeviceUpdateMode      = DeviceUpdateModeCallback;
    setup.DeviceSaveMode        = DeviceSaveModeCallback;

    rgb_controller = plugin_api->CreateVirtualRGBController(&setup);
    plugin_api->RegisterVirtualRGBController(rgb_controller);

    return(true);
}

/*---------------------------------------------------------*\
| Deleting the virtual controller joins its device thread,  |
| so no callback is running or pending once this returns;   |
| only then can the engine be stopped for good              |
\*---------------------------------------------------------*/
void RGBController_ViaRGBMatrix::Unregister()
{
    if(rgb_controller == NULL)
    {
        return;
    }

    plugin_api->UnregisterVirtualRGBController(rgb_controller);
    plugin_api->DeleteVirtualRGBController(rgb_controller);
    rgb_controller = NULL;

    engine.Stop();
}

void RGBController_ViaRGBMatrix::OnUpdateLEDs()
{
    if(rgb_controller->GetActiveMode() != VIA_MODE_DIRECT)
    {
        return;
    }

    engine.Stop();

    via_rgb_state next = sync.GetState();

    next.effect = VIA_EFFECT_SOLID_COLOR;
    via_rgb_to_hsv(rgb_controller->GetColor(0), &next.hue, &next.sat, &next.brightness);

    sync.Apply(next, false);
}

void RGBController_ViaRGBMatrix::OnUpdateMode(bool save)
{
    int active_mode = rgb_controller->GetActiveMode();

    if(active_mode >= via_first_custom_mode(max_effect) && active_mode < (int)rgb_controller->GetModeCount())
    {
        RunCustomEffect(active_mode);
        return;
    }

    engine.Stop();

    if(active_mode == VIA_MODE_DIRECT)
    {
        /*-------------------------------------------------*\
        | Direct is host-driven and never touches EEPROM    |
        \*-------------------------------------------------*/
        OnUpdateLEDs();
    }
    else if(via_mode_is_effect(active_mode, max_effect))
    {
        ApplyFirmwareEffect(active_mode, save);
    }
}

void RGBController_ViaRGBMatrix::ApplyFirmwareEffect(int mode_idx, bool save)
{
    via_rgb_state   next    = sync.GetState();
    unsigned int    flags   = rgb_controller->GetModeFlags(mode_idx);

    next.effect = via_mode_to_effect(mode_idx);

    if(flags & MODE_FLAG_HAS_BRIGHTNESS)
    {
        next.brightness = (unsigned char)rgb_controller->GetModeBrightness(mode_idx);
    }

    if(flags & MODE_FLAG_HAS_SPEED)
    {
        next.speed = (unsigned char)rgb_controller->GetModeSpeed(mode_idx);
    }

    if(flags & MODE_FLAG_HAS_MODE_SPECIFIC_COLOR)
    {
        unsigned char unused_val;
        via_rgb_to_hsv(rgb_controller->GetModeColor(mode_idx, 0), &next.hue, &next.sat, &unused_val);
    }

    sync.Apply(next, save);
}

void RGBController_ViaRGBMatrix::RunCustomEffect(int mode_idx)
{
    via_effect_params   params;
    unsigned int        color_count = rgb_controller->GetModeColorsCount(mode_idx);

    params.brightness   = (unsigned char)rgb_controller->GetModeBrightness(mode_idx);
    params.speed        = (unsigned char)rgb_controller->GetModeSpeed(mode_idx);

    for(unsigned int color_idx = 0; color_idx < VIA_EFFECT_MAX_COLORS; color_idx++)
    {
        params.colors[color_idx] = (color_idx < color_count) ? rgb_controller->GetModeColor(mode_idx, color_idx) : 0;
    }

    engine.Run((unsigned int)(mode_idx - via_first_custom_mode(max_effect)), params);
}

void RGBController_ViaRGBMatrix::DeviceUpdateLEDsCallback(void* object)
{
    ((RGBController_ViaRGBMatrix*)object)->OnUpdateLEDs();
}

void RGBController_ViaRGBMatrix::DeviceUpdateZoneLEDsCallback(void* object, int /*zone*/)
{
    ((RGBController_ViaRGBMatrix*)object)->OnUpdateLEDs();
}

void RGBController_ViaRGBMatrix::DeviceUpdateSingleLEDCallback(void* object, int /*led*/)
{
    ((RGBController_ViaRGBMatrix*)object)->OnUpdateLEDs();
}

void RGBController_ViaRGBMatrix::DeviceUpdateModeCallback(void* object)
{
    ((RGBController_ViaRGBMatrix*)object)->OnUpdateMode(false);
}

void RGBController_ViaRGBMatrix::DeviceSaveModeCallback(void* object)
{
    ((RGBController_ViaRGBMatrix*)object)->OnUpdateMode(true);
}
