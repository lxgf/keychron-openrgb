/*---------------------------------------------------------*\
| ViaCustomEffect.h                                         |
|                                                           |
|   Host-driven effect: modulates the firmware's effect,    |
|   hue, saturation, brightness and speed over time while   |
|   the firmware keeps animating per key.                   |
|                                                           |
|   Everything an effect needs comes from its settings      |
|   section; OpenRGB's mode controls are not used.          |
|   Every method runs on the effect engine thread.          |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include "RGBControllerInterface.h"
#include "ViaRGBMatrixController.h"
#include "ViaRGBMatrixDevices.h"

/*---------------------------------------------------------*\
| What the effect knows about the keyboard                  |
\*---------------------------------------------------------*/
typedef struct
{
    const via_device_entry* entry;
    unsigned char           max_effect;
} via_effect_context;

/*---------------------------------------------------------*\
| Settings helpers                                          |
\*---------------------------------------------------------*/
unsigned char   via_resolve_effect(const via_effect_context& context, const std::string& effect_name);
unsigned char   via_setting_percent(const nlohmann::json& settings, const char* key);
unsigned char   via_setting_speed(const nlohmann::json& settings, const char* key);
RGBColor        via_setting_color(const nlohmann::json& settings, const char* key, RGBColor fallback);

class ViaCustomEffect
{
public:
    virtual                 ~ViaCustomEffect() {}

    /*-----------------------------------------------------*\
    | Called before the first Tick and on settings changes  |
    \*-----------------------------------------------------*/
    virtual void            Configure(const nlohmann::json& settings)               = 0;

    virtual void            Start(const via_rgb_state& current)                     = 0;

    /*-----------------------------------------------------*\
    | Fill state for time now_ms, return ms until next tick |
    \*-----------------------------------------------------*/
    virtual unsigned int    Tick(unsigned long long now_ms, via_rgb_state* state)   = 0;

    virtual void            Stop()                                                  {}
};
