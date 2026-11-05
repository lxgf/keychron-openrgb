/*---------------------------------------------------------*\
| ViaEffectHueCycle.cpp                                     |
|                                                           |
|   Runs a firmware effect (reactive ones work best) and    |
|   moves its hue over time. "hue_speed" sets the cycle     |
|   time: 1 = 120 s, 100 = 1.2 s per cycle.                 |
|                                                           |
|   range "full":    the whole color wheel, "saturation"    |
|   range "between": back and forth from "from_color" to    |
|                    "to_color" (their own saturation)      |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cmath>
#include "ViaColor.h"
#include "ViaCustomEffectFactories.h"

#define HUE_CYCLE_PERIOD_AT_SPEED_1_MS  120000.0
#define HUE_CYCLE_MIN_TICK_MS           20
#define HUE_STEPS                       256.0

class ViaEffectHueCycle : public ViaCustomEffect
{
public:
    ViaEffectHueCycle(const via_effect_context& effect_context)
    {
        context         = effect_context;
        base_effect     = VIA_EFFECT_SOLID_COLOR;
        brightness      = 255;
        saturation      = 255;
        effect_speed    = 128;
        hue_speed       = 20;
        between         = false;
        from_color      = ToRGBColor(0, 64, 255);
        to_color        = ToRGBColor(198, 0, 255);
        hue             = 0.0;
        phase           = 0.0;
        last_ms         = 0;
    }

    void Configure(const nlohmann::json& settings) override
    {
        base_effect     = via_resolve_effect(context, settings.value("base_effect", "Solid Splash"));
        brightness      = via_setting_percent(settings, "brightness");
        saturation      = via_setting_percent(settings, "saturation");
        between         = (settings.value("range", "full") == "between");
        from_color      = via_setting_color(settings, "from_color", ToRGBColor(0, 64, 255));
        to_color        = via_setting_color(settings, "to_color",   ToRGBColor(198, 0, 255));
        effect_speed    = via_setting_speed(settings, "effect_speed");
        hue_speed       = settings.value("hue_speed", 20);
    }

    void Start(const via_rgb_state& current) override
    {
        hue     = current.hue;
        last_ms = 0;
    }

    unsigned int Tick(unsigned long long now_ms, via_rgb_state* state) override
    {
        double period_ms = HUE_CYCLE_PERIOD_AT_SPEED_1_MS / (hue_speed > 0 ? hue_speed : 1);

        if(last_ms != 0)
        {
            double advance = (now_ms - last_ms) / period_ms;

            hue     = fmod(hue + advance * HUE_STEPS, HUE_STEPS);
            phase   = fmod(phase + advance, 1.0);
        }

        last_ms             = now_ms;

        if(between)
        {
            /*---------------------------------------------*\
            | Triangle wave: there and back once per cycle  |
            \*---------------------------------------------*/
            float t = (float)(phase < 0.5 ? phase * 2.0 : (1.0 - phase) * 2.0);

            via_blend_hue_sat(from_color, to_color, t, &state->hue, &state->sat);
        }
        else
        {
            state->hue      = (unsigned char)hue;
            state->sat      = saturation;
        }

        state->effect       = base_effect;
        state->brightness   = brightness;
        state->speed        = effect_speed;

        /*-------------------------------------------------*\
        | Wake once per hue step, not faster                |
        \*-------------------------------------------------*/
        unsigned int step_ms = (unsigned int)(period_ms / HUE_STEPS);

        return(step_ms > HUE_CYCLE_MIN_TICK_MS ? step_ms : HUE_CYCLE_MIN_TICK_MS);
    }

private:
    via_effect_context  context;
    unsigned char       base_effect;
    unsigned char       brightness;
    unsigned char       saturation;
    unsigned char       effect_speed;
    int                 hue_speed;
    bool                between;
    RGBColor            from_color;
    RGBColor            to_color;
    double              hue;
    double              phase;
    unsigned long long  last_ms;
};

ViaCustomEffect* via_create_hue_cycle(const via_effect_context& context)
{
    return(new ViaEffectHueCycle(context));
}
