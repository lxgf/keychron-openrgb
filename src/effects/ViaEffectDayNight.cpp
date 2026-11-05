/*---------------------------------------------------------*\
| ViaEffectDayNight.cpp                                     |
|                                                           |
|   Day color/brightness by day, night color and dimmed     |
|   brightness by night, with a linear transition after    |
|   each switch time. Colors: [0] day, [1] night.           |
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
        day_start           = 7 * 60;
        night_start         = 22 * 60;
        transition          = 30;
        night_brightness    = 25;
    }

    void Configure(const nlohmann::json& settings) override
    {
        base_effect         = via_resolve_effect(context, settings.value("base_effect", "Solid Color"));
        day_start           = parse_minutes(settings.value("day_start", "07:00"), 7 * 60);
        night_start         = parse_minutes(settings.value("night_start", "22:00"), 22 * 60);
        transition          = settings.value("transition_min", 30);
        night_brightness    = settings.value("night_brightness", 25);
    }

    void Start(const via_rgb_state& /*current*/) override
    {
    }

    unsigned int Tick(unsigned long long /*now_ms*/, const via_effect_params& params, via_rgb_state* state) override
    {
        SYSTEMTIME local_time;
        GetLocalTime(&local_time);

        float   now         = local_time.wHour * 60 + local_time.wMinute + local_time.wSecond / 60.0f;
        int     now_minute  = (int)now;
        float   night       = NightAmount(now, now_minute);
        bool    in_ramp     = (transition > 0) && (minutes_since(now_minute, night_start) < transition || minutes_since(now_minute, day_start) < transition);

        float   day_bri     = params.brightness;
        float   night_bri   = params.brightness * night_brightness / 100.0f;

        via_blend_hue_sat(params.colors[0], params.colors[1], night, &state->hue, &state->sat);

        state->effect       = base_effect;
        state->brightness   = (unsigned char)(day_bri + (night_bri - day_bri) * night);
        state->speed        = params.speed;

        return(in_ramp ? DAY_NIGHT_TICK_TRANSITION_MS : DAY_NIGHT_TICK_STEADY_MS);
    }

private:
    via_effect_context  context;
    unsigned char       base_effect;
    int                 day_start;
    int                 night_start;
    int                 transition;
    int                 night_brightness;

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
