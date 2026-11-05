/*---------------------------------------------------------*\
| ViaEffectDayNight.cpp                                     |
|                                                           |
|   Day color and brightness by day, night color and night  |
|   brightness by night, with a linear transition after     |
|   each switch time.                                       |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstdio>
#include <windows.h>
#include "ViaColor.h"
#include "ViaCustomEffectFactories.h"

#define MINUTES_PER_DAY                 1440
#define DAY_NIGHT_TICK_STEADY_MS        60000
#define DAY_NIGHT_TICK_TRANSITION_MS    10000

static int parse_minutes(const std::string& hh_mm, int fallback)
{
    int hours   = 0;
    int minutes = 0;

    if(sscanf(hh_mm.c_str(), "%d:%d", &hours, &minutes) != 2 || hours < 0 || hours > 23 || minutes < 0 || minutes > 59)
    {
        return(fallback);
    }

    return(hours * 60 + minutes);
}

static int minutes_since(int now, int start)
{
    return((now - start + MINUTES_PER_DAY) % MINUTES_PER_DAY);
}

class ViaEffectDayNight : public ViaCustomEffect
{
public:
    ViaEffectDayNight(const via_effect_context& effect_context)
    {
        context             = effect_context;
        base_effect         = VIA_EFFECT_SOLID_COLOR;
        day_brightness      = 255;
        night_brightness    = 64;
        effect_speed        = 128;
        day_color           = ToRGBColor(255, 244, 229);
        night_color         = ToRGBColor(255, 96, 16);
        day_start           = 7 * 60;
        night_start         = 22 * 60;
        transition          = 30;
    }

    void Configure(const nlohmann::json& settings) override
    {
        base_effect         = via_resolve_effect(context, settings.value("base_effect", "Solid Color"));
        day_brightness      = via_setting_percent(settings, "brightness");
        night_brightness    = via_setting_percent(settings, "night_brightness");
        effect_speed        = via_setting_speed(settings, "effect_speed");
        day_color           = via_setting_color(settings, "day_color",   ToRGBColor(255, 244, 229));
        night_color         = via_setting_color(settings, "night_color", ToRGBColor(255, 96, 16));
        day_start           = parse_minutes(settings.value("day_start", "07:00"), 7 * 60);
        night_start         = parse_minutes(settings.value("night_start", "22:00"), 22 * 60);
        transition          = settings.value("transition_min", 30);
    }

    void Start(const via_rgb_state& /*current*/) override
    {
    }

    unsigned int Tick(unsigned long long /*now_ms*/, via_rgb_state* state) override
    {
        SYSTEMTIME local_time;
        GetLocalTime(&local_time);

        float   now         = local_time.wHour * 60 + local_time.wMinute + local_time.wSecond / 60.0f;
        int     now_minute  = (int)now;
        float   night       = NightAmount(now, now_minute);
        bool    in_ramp     = (transition > 0) && (minutes_since(now_minute, night_start) < transition || minutes_since(now_minute, day_start) < transition);

        via_blend_hue_sat(day_color, night_color, night, &state->hue, &state->sat);

        state->effect       = base_effect;
        state->brightness   = (unsigned char)(day_brightness + ((float)night_brightness - day_brightness) * night);
        state->speed        = effect_speed;

        return(in_ramp ? DAY_NIGHT_TICK_TRANSITION_MS : DAY_NIGHT_TICK_STEADY_MS);
    }

private:
    via_effect_context  context;
    unsigned char       base_effect;
    unsigned char       day_brightness;
    unsigned char       night_brightness;
    unsigned char       effect_speed;
    RGBColor            day_color;
    RGBColor            night_color;
    int                 day_start;
    int                 night_start;
    int                 transition;

    /*-----------------------------------------------------*\
    | 0 = full day, 1 = full night                          |
    \*-----------------------------------------------------*/
    float NightAmount(float now, int now_minute)
    {
        float since_night   = minutes_since(now_minute, night_start) + (now - now_minute);
        float since_day     = minutes_since(now_minute, day_start)   + (now - now_minute);

        if(transition > 0 && since_night < transition)
        {
            return(since_night / transition);
        }

        if(transition > 0 && since_day < transition)
        {
            return(1.0f - since_day / transition);
        }

        /*-------------------------------------------------*\
        | Night if night_start came more recently than      |
        | day_start                                         |
        \*-------------------------------------------------*/
        return(since_night < since_day ? 1.0f : 0.0f);
    }
};

ViaCustomEffect* via_create_day_night(const via_effect_context& context)
{
    return(new ViaEffectDayNight(context));
}
