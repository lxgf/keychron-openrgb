/*---------------------------------------------------------*\
| ViaCustomEffects.cpp                                      |
|                                                           |
|   Registry of custom effects                              |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "ViaCustomEffectFactories.h"
#include "ViaCustomEffects.h"

/*---------------------------------------------------------*\
| speed_default 0 means "start from the keyboard's speed"   |
\*---------------------------------------------------------*/
const via_custom_effect_desc via_custom_effects[] =
{
    {
        "Hue Cycle",        "hue_cycle",
        MODE_FLAG_HAS_SPEED,
        1, 100, 20,
        0, { 0, 0 },
        via_create_hue_cycle
    },
    {
        "Audio Reactive",   "audio",
        MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR,
        0, 255, 0,
        2, { ToRGBColor(255, 0, 64), ToRGBColor(0, 128, 255) },
        via_create_audio
    },
    {
        "System Monitor",   "system",
        MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR,
        0, 255, 0,
        2, { ToRGBColor(0, 255, 0), ToRGBColor(255, 0, 0) },
        via_create_system_monitor
    },
    {
        "Effect Playlist",  "playlist",
        MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR,
        0, 255, 0,
        1, { ToRGBColor(198, 0, 255), 0 },
        via_create_playlist
    },
    {
        "Day / Night",      "daynight",
        MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR,
        0, 255, 0,
        2, { ToRGBColor(255, 244, 229), ToRGBColor(255, 96, 16) },
        via_create_day_night
    },
};

const unsigned int via_custom_effect_count = sizeof(via_custom_effects) / sizeof(via_custom_effects[0]);

unsigned char via_resolve_effect(const via_effect_context& context, const std::string& effect_name)
{
    int effect_id = via_find_effect(context.entry, context.max_effect, effect_name);

    return((effect_id > VIA_EFFECT_OFF) ? (unsigned char)effect_id : VIA_EFFECT_SOLID_COLOR);
}
