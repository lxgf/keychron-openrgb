/*---------------------------------------------------------*\
| ViaRGBMatrixModes.cpp                                     |
|                                                           |
|   OpenRGB mode list for a VIA RGB Matrix keyboard         |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "ViaColor.h"
#include "ViaLog.h"
#include "ViaRGBMatrixModes.h"

#define VIA_VALUE_MIN                   0
#define VIA_VALUE_MAX                   255

/*---------------------------------------------------------*\
| Direct: the whole keyboard as one LED, driven through     |
| Solid Color + hue/sat/brightness, never saved             |
\*---------------------------------------------------------*/
static mode via_build_direct_mode()
{
    mode Direct;

    Direct.name         = "Direct";
    Direct.value        = VIA_MODE_DIRECT;
    Direct.flags        = MODE_FLAG_HAS_PER_LED_COLOR;
    Direct.color_mode   = MODE_COLORS_PER_LED;

    return(Direct);
}

static mode via_build_effect_mode(const via_device_entry* entry, unsigned char effect, const via_rgb_state& current)
{
    mode Effect;

    Effect.name         = via_effect_name(entry, effect);
    Effect.value        = effect;
    Effect.flags        = MODE_FLAG_MANUAL_SAVE;
    Effect.color_mode   = MODE_COLORS_NONE;

    if(effect == VIA_EFFECT_OFF)
    {
        return(Effect);
    }

    Effect.flags           |= MODE_FLAG_HAS_BRIGHTNESS;
    Effect.brightness_min   = VIA_VALUE_MIN;
    Effect.brightness_max   = VIA_VALUE_MAX;
    Effect.brightness       = current.brightness;

    if(via_effect_uses_speed(Effect.name))
    {
        Effect.flags       |= MODE_FLAG_HAS_SPEED;
        Effect.speed_min    = VIA_VALUE_MIN;
        Effect.speed_max    = VIA_VALUE_MAX;
        Effect.speed        = current.speed;
    }

    if(via_effect_uses_color(Effect.name))
    {
        Effect.flags       |= MODE_FLAG_HAS_MODE_SPECIFIC_COLOR;
        Effect.color_mode   = MODE_COLORS_MODE_SPECIFIC;
        Effect.colors_min   = 1;
        Effect.colors_max   = 1;
        Effect.colors.push_back(via_hsv_to_rgb(current.hue, current.sat, VIA_VALUE_MAX));
    }

    return(Effect);
}

std::vector<mode> via_build_modes(const via_device_entry* entry, unsigned char max_effect, const via_rgb_state& current)
{
    std::vector<mode> modes;

    if(entry->effect_names != NULL && entry->effect_count != (unsigned int)max_effect + 1)
    {
        VIA_LOG(LL_WARNING, "%s: firmware reports %u effects, name table has %u", entry->name, (unsigned int)max_effect + 1, entry->effect_count);
    }

    modes.reserve((std::size_t)max_effect + 2);
    modes.push_back(via_build_direct_mode());

    for(unsigned int effect = 0; effect <= max_effect; effect++)
    {
        modes.push_back(via_build_effect_mode(entry, (unsigned char)effect, current));
    }

    return(modes);
}
