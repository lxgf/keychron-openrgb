/*---------------------------------------------------------*\
| RGBController_ViaRGBMatrix.cpp                            |
|                                                           |
|   OpenRGB model for a VIA RGB Matrix keyboard: one zone,  |
|   one LED, Direct mode plus the firmware's effects.       |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cmath>
#include <cstdio>
#include "LogManager.h"
#include "RGBController_ViaRGBMatrix.h"

#define VIA_LOG(level, ...)     plugin_api->LogEntry(__FILE__, __LINE__, level, __VA_ARGS__)

/*---------------------------------------------------------*\
| QMK HSV uses 8 bits for each channel. Hue is scaled by    |
| 255 to match what the Keychron Launcher sends            |
| (#C600FF -> hue 0xCB, sat 0xFF).                          |
\*---------------------------------------------------------*/
static void RGBToHSV(RGBColor color, unsigned char* hue, unsigned char* sat, unsigned char* val)
{
    float r     = RGBGetRValue(color) / 255.0f;
    float g     = RGBGetGValue(color) / 255.0f;
    float b     = RGBGetBValue(color) / 255.0f;
    float max   = fmaxf(r, fmaxf(g, b));
    float min   = fminf(r, fminf(g, b));
    float delta = max - min;
    float h     = 0.0f;

    if(delta > 0.0f)
    {
        if(max == r)
        {
            h = fmodf((g - b) / delta, 6.0f);
        }
        else if(max == g)
        {
            h = ((b - r) / delta) + 2.0f;
        }
        else
        {
            h = ((r - g) / delta) + 4.0f;
        }

        h = h / 6.0f;

        if(h < 0.0f)
        {
            h += 1.0f;
        }

        /*-------------------------------------------------*\
        | Hue is meaningless for greys, keep the old one so |
        | Direct does not send a packet for nothing         |
        \*-------------------------------------------------*/
        *hue = (unsigned char)((int)lroundf(h * 255.0f) & 0xFF);
    }

    if(max > 0.0f)
    {
        *sat = (unsigned char)lroundf((delta / max) * 255.0f);
    }

    *val = (unsigned char)lroundf(max * 255.0f);
}

static RGBColor HSVToRGB(unsigned char hue, unsigned char sat, unsigned char val)
{
    float h = (hue / 255.0f) * 6.0f;
    float s = sat / 255.0f;
    float v = val / 255.0f;
    float c = v * s;
    float x = c * (1.0f - fabsf(fmodf(h, 2.0f) - 1.0f));
    float m = v - c;
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;

    switch((int)h % 6)
    {
        case 0: r = c; g = x; break;
        case 1: r = x; g = c; break;
        case 2: g = c; b = x; break;
        case 3: g = x; b = c; break;
        case 4: r = x; b = c; break;
        case 5: r = c; b = x; break;
    }

    return(ToRGBColor((int)lroundf((r + m) * 255.0f), (int)lroundf((g + m) * 255.0f), (int)lroundf((b + m) * 255.0f)));
}

RGBController_ViaRGBMatrix::RGBController_ViaRGBMatrix(ViaRGBMatrixController* controller_ptr, const via_device_entry* entry, OpenRGBPluginAPIInterface* api)
{
    controller      = controller_ptr;
    device_entry    = entry;
    plugin_api      = api;
    rgb_controller  = NULL;
    worker_running  = false;
    update_pending  = false;
    save_pending    = false;
    sent_valid      = false;
}

RGBController_ViaRGBMatrix::~RGBController_ViaRGBMatrix()
{
    Unregister();
    delete controller;
}

std::string RGBController_ViaRGBMatrix::GetPath()
{
    return(controller->GetPath());
}

void RGBController_ViaRGBMatrix::BuildModes(unsigned char max_effect, const via_rgb_state& current)
{
    modes.clear();

    /*-----------------------------------------------------*\
    | Direct: the whole keyboard as one LED, driven by      |
    | Solid Color + hue/sat/brightness, never saved         |
    \*-----------------------------------------------------*/
    mode Direct;
    Direct.name         = "Direct";
    Direct.value        = VIA_MODE_VALUE_DIRECT;
    Direct.flags        = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode   = MODE_COLORS_PER_LED;
    modes.push_back(Direct);

    if(device_entry->effect_names != NULL && device_entry->effect_count != (unsigned int)max_effect + 1)
    {
        VIA_LOG(LL_WARNING, "[VIA RGB Matrix] %s: firmware reports %u effects, name table has %u", device_entry->name, (unsigned int)max_effect + 1, device_entry->effect_count);
    }

    for(unsigned int effect_idx = 0; effect_idx <= max_effect; effect_idx++)
    {
        std::string effect_name;

        if(device_entry->effect_names != NULL && effect_idx < device_entry->effect_count)
        {
            effect_name = device_entry->effect_names[effect_idx];
        }
        else if(effect_idx == VIA_EFFECT_OFF)
        {
            effect_name = "Off";
        }
        else if(effect_idx == VIA_EFFECT_SOLID_COLOR)
        {
            effect_name = "Solid Color";
        }
        else
        {
            effect_name = "Effect " + std::to_string(effect_idx);
        }

        mode Effect;
        Effect.name             = effect_name;
        Effect.value            = effect_idx;
        Effect.flags            = MODE_FLAG_MANUAL_SAVE;
        Effect.color_mode       = MODE_COLORS_NONE;

        if(effect_idx != VIA_EFFECT_OFF)
        {
            Effect.flags           |= MODE_FLAG_HAS_BRIGHTNESS;
            Effect.brightness_min   = 0;
            Effect.brightness_max   = 255;
            Effect.brightness       = current.brightness;
        }

        if(effect_idx != VIA_EFFECT_OFF && via_effect_uses_speed(effect_name.c_str()))
        {
            Effect.flags           |= MODE_FLAG_HAS_SPEED;
            Effect.speed_min        = 0;
            Effect.speed_max        = 255;
            Effect.speed            = current.speed;
        }

        if(effect_idx != VIA_EFFECT_OFF && via_effect_uses_color(effect_name.c_str()))
        {
            Effect.flags           |= MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
            Effect.color_mode       = MODE_COLORS_MODE_SPECIFIC;
            Effect.colors_min       = 1;
            Effect.colors_max       = 1;
            Effect.colors.push_back(HSVToRGB(current.hue, current.sat, 255));
        }

        modes.push_back(Effect);
    }
}

bool RGBController_ViaRGBMatrix::Register()
{
    via_rgb_state   current;
    unsigned char   max_effect;

    if(!controller->GetEffect(&current.effect)
    || !controller->GetBrightness(&current.brightness)
    || !controller->GetSpeed(&current.speed)
    || !controller->GetColor(&current.hue, &current.sat))
    {
        VIA_LOG(LL_ERROR, "[VIA RGB Matrix] %s: failed to read lighting state", device_entry->name);
        return(false);
    }

    if(!controller->ProbeMaxEffect(&max_effect))
    {
        VIA_LOG(LL_ERROR, "[VIA RGB Matrix] %s: effect count probe failed", device_entry->name);
        return(false);
    }

    VIA_LOG(LL_INFO, "[VIA RGB Matrix] %s: protocol 0x%04X, effect %u/%u, brightness %u, speed %u, hue %u, sat %u",
            device_entry->name, controller->GetProtocolVersion(), current.effect, max_effect, current.brightness, current.speed, current.hue, current.sat);

    BuildModes(max_effect, current);

    desired     = current;
    sent        = current;
    sent_valid  = true;

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
    keyboard_led.value          = 0;

    RGBController_Setup setup{};
    setup.name          = device_entry->name;
    setup.vendor        = device_entry->vendor;
    setup.description   = "QMK/VIA RGB Matrix keyboard (stock firmware)";
    setup.version       = version;
    setup.serial        = controller->GetSerial();
    setup.location      = controller->GetLocation();
    setup.type          = DEVICE_TYPE_KEYBOARD;
    setup.flags         = CONTROLLER_FLAG_VIRTUAL;
    setup.modes         = modes;
    setup.zones.push_back(keyboard_zone);
    setup.leds.push_back(keyboard_led);
    setup.active_mode   = (current.effect <= max_effect) ? current.effect + 1 : 0;

    setup.object_ptr                = this;
    setup.DeviceUpdateLEDs          = DeviceUpdateLEDsCallback;
    setup.DeviceUpdateZoneLEDs      = DeviceUpdateZoneLEDsCallback;
    setup.DeviceUpdateSingleLED     = DeviceUpdateSingleLEDCallback;
    setup.DeviceUpdateMode          = DeviceUpdateModeCallback;
    setup.DeviceSaveMode            = DeviceSaveModeCallback;

    worker_running  = true;
    worker_thread   = std::thread(&RGBController_ViaRGBMatrix::WorkerThread, this);

    rgb_controller  = plugin_api->CreateVirtualRGBController(&setup);
    plugin_api->RegisterVirtualRGBController(rgb_controller);

    return(true);
}

void RGBController_ViaRGBMatrix::Unregister()
{
    if(rgb_controller != NULL)
    {
        plugin_api->UnregisterVirtualRGBController(rgb_controller);
        plugin_api->DeleteVirtualRGBController(rgb_controller);
        rgb_controller = NULL;
    }

    if(worker_thread.joinable())
    {
        {
            std::lock_guard<std::mutex> guard(state_lock);
            worker_running = false;
        }

        state_cv.notify_one();
        worker_thread.join();
    }
}

void RGBController_ViaRGBMatrix::QueueState(const via_rgb_state& next, bool save)
{
    {
        std::lock_guard<std::mutex> guard(state_lock);

        desired         = next;
        update_pending  = true;
        save_pending    = save_pending || save;
    }

    state_cv.notify_one();
}

void RGBController_ViaRGBMatrix::UpdateLEDs()
{
    int active_mode = rgb_controller->GetActiveMode();

    if(active_mode < 0 || active_mode >= (int)modes.size() || modes[active_mode].value != VIA_MODE_VALUE_DIRECT)
    {
        return;
    }

    via_rgb_state next;
    {
        std::lock_guard<std::mutex> guard(state_lock);
        next = desired;
    }

    next.effect = VIA_EFFECT_SOLID_COLOR;
    RGBToHSV(rgb_controller->GetColor(0), &next.hue, &next.sat, &next.brightness);

    QueueState(next, false);
}

void RGBController_ViaRGBMatrix::UpdateMode(bool save)
{
    int active_mode = rgb_controller->GetActiveMode();

    if(active_mode < 0 || active_mode >= (int)modes.size())
    {
        return;
    }

    if(modes[active_mode].value == VIA_MODE_VALUE_DIRECT)
    {
        /*-------------------------------------------------*\
        | Direct is host-driven and never touches EEPROM    |
        \*-------------------------------------------------*/
        UpdateLEDs();
        return;
    }

    via_rgb_state next;
    {
        std::lock_guard<std::mutex> guard(state_lock);
        next = desired;
    }

    unsigned int flags = modes[active_mode].flags;

    next.effect = (unsigned char)modes[active_mode].value;

    if(flags & MODE_FLAG_HAS_BRIGHTNESS)
    {
        next.brightness = (unsigned char)rgb_controller->GetModeBrightness(active_mode);
    }

    if(flags & MODE_FLAG_HAS_SPEED)
    {
        next.speed = (unsigned char)rgb_controller->GetModeSpeed(active_mode);
    }

    if(flags & MODE_FLAG_HAS_MODE_SPECIFIC_COLOR)
    {
        unsigned char unused_val;
        RGBToHSV(rgb_controller->GetModeColor(active_mode, 0), &next.hue, &next.sat, &unused_val);
    }

    QueueState(next, save);
}

void RGBController_ViaRGBMatrix::WorkerThread()
{
    while(true)
    {
        via_rgb_state   target;
        bool            save;

        {
            std::unique_lock<std::mutex> lock(state_lock);

            state_cv.wait(lock, [this]{ return(!worker_running || update_pending || save_pending); });

            if(!worker_running)
            {
                break;
            }

            target          = desired;
            save            = save_pending;
            update_pending  = false;
            save_pending    = false;
        }

        bool ok = true;

        if(!sent_valid || target.effect != sent.effect)
        {
            ok = ok && controller->SetEffect(target.effect);
        }

        if(!sent_valid || target.hue != sent.hue || target.sat != sent.sat)
        {
            ok = ok && controller->SetColor(target.hue, target.sat);
        }

        if(!sent_valid || target.brightness != sent.brightness)
        {
            ok = ok && controller->SetBrightness(target.brightness);
        }

        if(!sent_valid || target.speed != sent.speed)
        {
            ok = ok && controller->SetSpeed(target.speed);
        }

        sent        = target;
        sent_valid  = ok;

        if(save && ok)
        {
            ok = controller->Save();
        }

        if(!ok)
        {
            VIA_LOG(LL_WARNING, "[VIA RGB Matrix] %s: HID transfer failed", device_entry->name);
        }
    }
}

void RGBController_ViaRGBMatrix::DeviceUpdateLEDsCallback(void* object)
{
    ((RGBController_ViaRGBMatrix*)object)->UpdateLEDs();
}

void RGBController_ViaRGBMatrix::DeviceUpdateZoneLEDsCallback(void* object, int /*zone*/)
{
    ((RGBController_ViaRGBMatrix*)object)->UpdateLEDs();
}

void RGBController_ViaRGBMatrix::DeviceUpdateSingleLEDCallback(void* object, int /*led*/)
{
    ((RGBController_ViaRGBMatrix*)object)->UpdateLEDs();
}

void RGBController_ViaRGBMatrix::DeviceUpdateModeCallback(void* object)
{
    ((RGBController_ViaRGBMatrix*)object)->UpdateMode(false);
}

void RGBController_ViaRGBMatrix::DeviceSaveModeCallback(void* object)
{
    ((RGBController_ViaRGBMatrix*)object)->UpdateMode(true);
}
