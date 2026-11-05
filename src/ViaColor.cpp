/*---------------------------------------------------------*\
| ViaColor.cpp                                              |
|                                                           |
|   OpenRGB RGB <-> QMK 8-bit HSV                           |
|                                                           |
|   Hue is scaled by 255 to match the Keychron Launcher     |
|   (#C600FF -> hue 0xCB, sat 0xFF).                        |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cmath>
#include "ViaColor.h"

void via_rgb_to_hsv(RGBColor color, unsigned char* hue, unsigned char* sat, unsigned char* val)
{
    float r     = RGBGetRValue(color) / 255.0f;
    float g     = RGBGetGValue(color) / 255.0f;
    float b     = RGBGetBValue(color) / 255.0f;
    float max   = fmaxf(r, fmaxf(g, b));
    float min   = fminf(r, fminf(g, b));
    float delta = max - min;

    if(delta > 0.0f)
    {
        float h;

        if(max == r)
        {
            h = fmodf((g - b) / delta, 6.0f);
        }
        else if(max == g)
        {
            h = ((b - r) / delta) + 2.0f;
        }
        else
        {
            h = ((r - g) / delta) + 4.0f;
        }

        h = h / 6.0f;

        if(h < 0.0f)
        {
            h += 1.0f;
        }

        *hue = (unsigned char)((int)lroundf(h * 255.0f) & 0xFF);
    }

    if(max > 0.0f)
    {
        *sat = (unsigned char)lroundf((delta / max) * 255.0f);
    }

    *val = (unsigned char)lroundf(max * 255.0f);
}

RGBColor via_hsv_to_rgb(unsigned char hue, unsigned char sat, unsigned char val)
{
    float h = (hue / 255.0f) * 6.0f;
    float s = sat / 255.0f;
    float v = val / 255.0f;
    float c = v * s;
    float x = c * (1.0f - fabsf(fmodf(h, 2.0f) - 1.0f));
    float m = v - c;
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;

    switch((int)h % 6)
    {
        case 0: r = c; g = x; break;
        case 1: r = x; g = c; break;
        case 2: g = c; b = x; break;
        case 3: g = x; b = c; break;
        case 4: r = x; b = c; break;
        case 5: r = c; b = x; break;
    }

    return(ToRGBColor((int)lroundf((r + m) * 255.0f), (int)lroundf((g + m) * 255.0f), (int)lroundf((b + m) * 255.0f)));
}
