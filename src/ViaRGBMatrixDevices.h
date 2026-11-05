/*---------------------------------------------------------*\
| ViaRGBMatrixDevices.h                                     |
|                                                           |
|   Allowlist of supported keyboards and their RGB Matrix   |
|   effect names (VIA only reports effect ids)             |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>

#define VIA_EFFECT_OFF                  0
#define VIA_EFFECT_SOLID_COLOR          1

typedef struct
{
    unsigned short      vid;
    unsigned short      pid;
    const char*         vendor;
    const char*         name;
    const char* const*  effect_names;   /* index = effect id, NULL = "Effect N" */
    unsigned int        effect_count;
} via_device_entry;

extern const via_device_entry   via_devices[];
extern const unsigned int       via_device_count;

const via_device_entry* via_find_device(unsigned short vid, unsigned short pid);

std::string via_effect_name(const via_device_entry* entry, unsigned int effect_id);
bool        via_effect_uses_color(const std::string& effect_name);
bool        via_effect_uses_speed(const std::string& effect_name);
