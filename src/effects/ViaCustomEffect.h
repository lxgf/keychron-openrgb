/*---------------------------------------------------------*\
| ViaCustomEffect.h                                         |
|                                                           |
|   Host-driven effect: modulates the firmware's effect,    |
|   hue, saturation, brightness and speed over time while   |
|   the firmware keeps animating per key.                   |
|                                                           |
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

#define VIA_EFFECT_MAX_COLORS           2

/*---------------------------------------------------------*\
| Values from the OpenRGB mode controls                     |
\*---------------------------------------------------------*/
typedef struct
{
    unsigned char   brightness;
    unsigned char   speed;
    RGBColor        colors[VIA_EFFECT_MAX_COLORS];
} via_effect_params;

/*---------------------------------------------------------*\
| What the effect knows about the keyboard                  |
\*---------------------------------------------------------*/
typedef struct
{
    const via_device_entry* entry;
    unsigned char           max_effect;
} via_effect_context;

/*---------------------------------------------------------*\
| Firmware effect id for a name, Solid Color if unknown     |
\*---------------------------------------------------------*/
unsigned char via_resolve_effect(const via_effect_context& context, const std::string& effect_name);

class ViaCustomEffect
{
public:
    virtual                 ~ViaCustomEffect() {}

    /*-----------------------------------------------------*\
    | Called before the first Tick and on settings changes  |
    \*-----------------------------------------------------*/
    virtual void            Configure(const nlohmann::json& settings)                                           = 0;

    virtual void            Start(const via_rgb_state& current)                                                 = 0;

    /*-----------------------------------------------------*\
    | Fill state for time now_ms, return ms until next tick |
    \*-----------------------------------------------------*/
    virtual unsigned int    Tick(unsigned long long now_ms, const via_effect_params& params, via_rgb_state* state) = 0;

    virtual void            Stop()                                                                              {}
};
