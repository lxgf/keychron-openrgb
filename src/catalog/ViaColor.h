/*---------------------------------------------------------*\
| ViaColor.h                                                |
|                                                           |
|   OpenRGB RGB <-> QMK 8-bit HSV                           |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include "RGBControllerInterface.h"

/*---------------------------------------------------------*\
| hue/sat are left unchanged where they are undefined       |
| (greys keep hue, black keeps hue and sat)                 |
\*---------------------------------------------------------*/
void        via_rgb_to_hsv(RGBColor color, unsigned char* hue, unsigned char* sat, unsigned char* val);
RGBColor    via_hsv_to_rgb(unsigned char hue, unsigned char sat, unsigned char val);

/*---------------------------------------------------------*\
| Blend two colors in HSV (hue takes the shorter way round) |
| t = 0 gives from, t = 1 gives to                          |
\*---------------------------------------------------------*/
void        via_blend_hue_sat(RGBColor from, RGBColor to, float t, unsigned char* hue, unsigned char* sat);

/*---------------------------------------------------------*\
| "#RRGGBB" to color; false if the text is not one          |
\*---------------------------------------------------------*/
bool        via_parse_color(const std::string& text, RGBColor* color);
