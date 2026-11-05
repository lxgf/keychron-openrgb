/*---------------------------------------------------------*\
| ViaCustomEffects.h                                        |
|                                                           |
|   Registry of custom effects: OpenRGB mode description,   |
|   settings section and factory for each one.              |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "ViaCustomEffect.h"

typedef struct
{
    const char*         name;               /* OpenRGB mode name                    */
    const char*         settings_section;   /* ViaSettings section                  */
    unsigned int        flags;              /* MODE_FLAG_* besides brightness       */
    unsigned int        speed_min;
    unsigned int        speed_max;
    unsigned int        speed_default;      /* 0 = keyboard's current speed         */
    unsigned int        colors_count;
    RGBColor            colors_default[VIA_EFFECT_MAX_COLORS];
    ViaCustomEffect*    (*create)(const via_effect_context& context);
} via_custom_effect_desc;

extern const via_custom_effect_desc     via_custom_effects[];
extern const unsigned int               via_custom_effect_count;
