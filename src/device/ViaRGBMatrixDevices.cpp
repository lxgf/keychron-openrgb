/*---------------------------------------------------------*\
| ViaRGBMatrixDevices.cpp                                   |
|                                                           |
|   Allowlist of supported keyboards and their RGB Matrix   |
|   effect names (VIA only reports effect ids)             |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

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

static bool name_in_list(const std::string& name, const char* const* list, unsigned int count)
{
    for(unsigned int list_idx = 0; list_idx < count; list_idx++)
    {
        if(name == list[list_idx])
        {
            return(true);
        }
    }

    return(false);
}

const via_device_entry* via_find_device(unsigned short vid, unsigned short pid)
{
    for(unsigned int device_idx = 0; device_idx < via_device_count; device_idx++)
    {
        if(via_devices[device_idx].vid == vid && via_devices[device_idx].pid == pid)
        {
            return(&via_devices[device_idx]);
        }
    }

    return(NULL);
}

/*---------------------------------------------------------*\
| QMK always builds Off (0) and Solid Color (1); anything   |
| beyond the device's name table is numbered                |
\*---------------------------------------------------------*/
std::string via_effect_name(const via_device_entry* entry, unsigned int effect_id)
{
    if(entry->effect_names != NULL && effect_id < entry->effect_count)
    {
        return(entry->effect_names[effect_id]);
    }

    switch(effect_id)
    {
        case VIA_EFFECT_OFF:            return("Off");
        case VIA_EFFECT_SOLID_COLOR:    return("Solid Color");
        default:                        return("Effect " + std::to_string(effect_id));
    }
}

bool via_effect_uses_color(const std::string& effect_name)
{
    return(!name_in_list(effect_name, no_color_effects, sizeof(no_color_effects) / sizeof(no_color_effects[0])));
}

bool via_effect_uses_speed(const std::string& effect_name)
{
    return(!name_in_list(effect_name, no_speed_effects, sizeof(no_speed_effects) / sizeof(no_speed_effects[0])));
}
