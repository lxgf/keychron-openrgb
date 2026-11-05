/*---------------------------------------------------------*\
| ViaCustomEffects.cpp                                      |
|                                                           |
|   Registry of custom effects and settings helpers         |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "ViaColor.h"
#include "ViaCustomEffectFactories.h"
#include "ViaCustomEffects.h"

#define VIA_PERCENT_MAX                 100
#define VIA_VALUE_MAX                   255

const via_custom_effect_desc via_custom_effects[] =
{
    { "Hue Cycle",          "hue_cycle",    via_create_hue_cycle        },
    { "Audio Reactive",     "audio",        via_create_audio            },
    { "System Monitor",     "system",       via_create_system_monitor   },
    { "Effect Playlist",    "playlist",     via_create_playlist         },
    { "Day / Night",        "daynight",     via_create_day_night        },
};

const unsigned int via_custom_effect_count = sizeof(via_custom_effects) / sizeof(via_custom_effects[0]);

unsigned char via_resolve_effect(const via_effect_context& context, const std::string& effect_name)
{
    int effect_id = via_find_effect(context.entry, context.max_effect, effect_name);

    return((effect_id > VIA_EFFECT_OFF) ? (unsigned char)effect_id : VIA_EFFECT_SOLID_COLOR);
}

/*---------------------------------------------------------*\
| Brightness and saturation are stored in percent, the     |
| keyboard takes 0-255                                      |
\*---------------------------------------------------------*/
unsigned char via_setting_percent(const nlohmann::json& settings, const char* key)
{
    int percent = settings.value(key, VIA_PERCENT_MAX);

    if(percent < 0)
    {
        percent = 0;
    }

    if(percent > VIA_PERCENT_MAX)
    {
        percent = VIA_PERCENT_MAX;
    }

    return((unsigned char)((percent * VIA_VALUE_MAX + VIA_PERCENT_MAX / 2) / VIA_PERCENT_MAX));
}

unsigned char via_setting_speed(const nlohmann::json& settings, const char* key)
{
    int speed = settings.value(key, VIA_VALUE_MAX / 2);

    return((unsigned char)((speed < 0) ? 0 : (speed > VIA_VALUE_MAX) ? VIA_VALUE_MAX : speed));
}

RGBColor via_setting_color(const nlohmann::json& settings, const char* key, RGBColor fallback)
{
    RGBColor color;

    return(via_parse_color(settings.value(key, ""), &color) ? color : fallback);
}
