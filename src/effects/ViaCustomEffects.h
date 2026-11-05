/*---------------------------------------------------------*\
| ViaCustomEffects.h                                        |
|                                                           |
|   Registry of custom effects: OpenRGB mode name, settings |
|   section and factory for each one.                       |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "ViaCustomEffect.h"

typedef struct
{
    const char*         name;               /* OpenRGB mode name                    */
    const char*         settings_section;   /* ViaSettings section                  */
    ViaCustomEffect*    (*create)(const via_effect_context& context);
} via_custom_effect_desc;

extern const via_custom_effect_desc     via_custom_effects[];
extern const unsigned int               via_custom_effect_count;
