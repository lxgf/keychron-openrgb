/*---------------------------------------------------------*\
| ViaRGBMatrixModes.h                                       |
|                                                           |
|   OpenRGB mode list for a VIA RGB Matrix keyboard.        |
|   Layout: mode 0 = Direct, mode N + 1 = firmware effect N |
|   so mode index and effect id map without a lookup table. |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <vector>
#include "RGBControllerInterface.h"
#include "ViaRGBMatrixController.h"
#include "ViaRGBMatrixDevices.h"

#define VIA_MODE_DIRECT                 0
#define VIA_MODE_FIRST_EFFECT           1

std::vector<mode>   via_build_modes(const via_device_entry* entry, unsigned char max_effect, const via_rgb_state& current);

inline int via_effect_to_mode(unsigned char effect)
{
    return(effect + VIA_MODE_FIRST_EFFECT);
}

inline unsigned char via_mode_to_effect(int mode_idx)
{
    return((unsigned char)(mode_idx - VIA_MODE_FIRST_EFFECT));
}
