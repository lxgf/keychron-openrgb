/*---------------------------------------------------------*\
| ViaRGBMatrixDevices.cpp                                   |
|                                                           |
|   Allowlist of supported keyboards and their RGB Matrix   |
|   effect names (VIA only reports effect ids)             |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstring>
#include "ViaRGBMatrixDevices.h"

/*---------------------------------------------------------*\
| Keychron C2 Pro (V1) RGB, stock firmware                  |
| Max effect id 22 confirmed by clamp probe on hardware     |
\*---------------------------------------------------------*/
static const char* const keychron_c2_pro_effects[] =
{
    "Off",
    "Solid Color",
    "Breathing",
    "Band Spiral Val",
    "Cycle All",
    "Cycle Left Right",
    "Cycle Up Down",
    "Rainbow Moving Chevron",
    "Cycle Out In",
    "Cycle Out In Dual",
    "Rainbow Pinwheels",
    "Cycle Spiral",
    "Dual Beacon",
    "Rainbow Beacon",
    "Jellybean Raindrops",
    "Pixel Rain",
    "Typing Heatmap",
    "Digital Rain",
    "Reactive Simple",
    "Reactive Multiwide",
    "Reactive Multinexus",
    "Splash",
    "Solid Splash",
};

const via_device_entry via_devices[] =
{
    { 0x3434, 0x0520, "Keychron", "Keychron C2 Pro", keychron_c2_pro_effects, sizeof(keychron_c2_pro_effects) / sizeof(keychron_c2_pro_effects[0]) },
};

const unsigned int via_device_count = sizeof(via_devices) / sizeof(via_devices[0]);

/*---------------------------------------------------------*\
| QMK effects that ignore the configured hue/saturation     |
\*---------------------------------------------------------*/
static const char* const no_color_effects[] =
{
    "Off",
    "Cycle All",
    "Cycle Left Right",
    "Cycle Up Down",
    "Cycle Out In",
    "Cycle Out In Dual",
    "Cycle Pinwheel",
    "Cycle Spiral",
    "Rainbow Beacon",
    "Rainbow Pinwheels",
    "Rainbow Moving Chevron",
    "Jellybean Raindrops",
    "Pixel Rain",
    "Pixel Fractal",
    "Typing Heatmap",
    "Splash",
    "Multisplash",
};

/*---------------------------------------------------------*\
| QMK effects that are static                               |
\*---------------------------------------------------------*/
static const char* const no_speed_effects[] =
{
    "Off",
    "Solid Color",
};

static bool name_in_list(const char* name, const char* const* list, unsigned int count)
{
    for(unsigned int list_idx = 0; list_idx < count; list_idx++)
    {
        if(strcmp(name, list[list_idx]) == 0)
        {
            return(true);
        }
    }

    return(false);
}

bool via_effect_uses_color(const char* effect_name)
{
    return(!name_in_list(effect_name, no_color_effects, sizeof(no_color_effects) / sizeof(no_color_effects[0])));
}

bool via_effect_uses_speed(const char* effect_name)
{
    return(!name_in_list(effect_name, no_speed_effects, sizeof(no_speed_effects) / sizeof(no_speed_effects[0])));
}
