/*---------------------------------------------------------*\
| ViaCustomEffectFactories.h                                |
|                                                           |
|   One factory per custom effect, used by the registry     |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include "ViaCustomEffect.h"

ViaCustomEffect*    via_create_hue_cycle(const via_effect_context& context);
ViaCustomEffect*    via_create_audio(const via_effect_context& context);
ViaCustomEffect*    via_create_system_monitor(const via_effect_context& context);
ViaCustomEffect*    via_create_playlist(const via_effect_context& context);
ViaCustomEffect*    via_create_day_night(const via_effect_context& context);
