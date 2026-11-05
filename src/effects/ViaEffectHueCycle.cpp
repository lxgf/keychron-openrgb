/*---------------------------------------------------------*\
| ViaEffectHueCycle.cpp                                     |
|                                                           |
|   Runs a firmware effect (reactive ones work best) and    |
|   walks its hue round the color wheel. "hue_speed" sets   |
|   the cycle time: 1 = 120 s, 100 = 1.2 s per turn.        |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cmath>
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
        hue             = 0.0;
        last_ms         = 0;
    }

    void Configure(const nlohmann::json& settings) override
    {
        base_effect     = via_resolve_effect(context, settings.value("base_effect", "Solid Splash"));
        brightness      = via_setting_brightness(settings, "brightness");
        saturation      = (unsigned char)settings.value("saturation", 255);
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
            hue = fmod(hue + ((now_ms - last_ms) / period_ms) * HUE_STEPS, HUE_STEPS);
        }

        last_ms             = now_ms;

        state->effect       = base_effect;
        state->hue          = (unsigned char)hue;
        state->sat          = saturation;
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
    double              hue;
    unsigned long long  last_ms;
};

ViaCustomEffect* via_create_hue_cycle(const via_effect_context& context)
{
    return(new ViaEffectHueCycle(context));
}
