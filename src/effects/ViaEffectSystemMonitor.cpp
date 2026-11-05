/*---------------------------------------------------------*\
| ViaEffectSystemMonitor.cpp                                |
|                                                           |
|   Tints a firmware effect by CPU, RAM or GPU load:        |
|   "low_color" at or below "low" %, "high_color" at or     |
|   above "high" %, blended in between.                     |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "ViaColor.h"
#include "ViaCustomEffectFactories.h"
#include "ViaSystemSensors.h"

#define SYSTEM_MONITOR_TICK_MS          500

class ViaEffectSystemMonitor : public ViaCustomEffect
{
public:
    ViaEffectSystemMonitor(const via_effect_context& effect_context)
    {
        context         = effect_context;
        base_effect     = VIA_EFFECT_SOLID_COLOR;
        brightness      = 255;
        effect_speed    = 128;
        low_color       = ToRGBColor(0, 255, 0);
        high_color      = ToRGBColor(255, 0, 0);
        sensor          = VIA_SENSOR_CPU;
        low             = 10.0f;
        high            = 90.0f;
        smoothing       = 0.6f;
        value           = -1.0f;
    }

    void Configure(const nlohmann::json& settings) override
    {
        std::string source = settings.value("source", "cpu");

        base_effect     = via_resolve_effect(context, settings.value("base_effect", "Solid Color"));
        brightness      = via_setting_percent(settings, "brightness");
        effect_speed    = via_setting_speed(settings, "effect_speed");
        low_color       = via_setting_color(settings, "low_color",  ToRGBColor(0, 255, 0));
        high_color      = via_setting_color(settings, "high_color", ToRGBColor(255, 0, 0));
        sensor          = (source == "ram") ? VIA_SENSOR_RAM : (source == "gpu") ? VIA_SENSOR_GPU : VIA_SENSOR_CPU;
        low             = (float)settings.value("low", 10);
        high            = (float)settings.value("high", 90);
        smoothing       = settings.value("smoothing", 60) / 100.0f;
        value           = -1.0f;
    }

    void Start(const via_rgb_state& /*current*/) override
    {
        value = -1.0f;
    }

    unsigned int Tick(unsigned long long /*now_ms*/, via_rgb_state* state) override
    {
        float sample = sensors.Read(sensor);

        value = (value < 0.0f) ? sample : (value * smoothing + sample * (1.0f - smoothing));

        float range = (high > low) ? (high - low) : 1.0f;

        via_blend_hue_sat(low_color, high_color, (value - low) / range, &state->hue, &state->sat);

        state->effect       = base_effect;
        state->brightness   = brightness;
        state->speed        = effect_speed;

        return(SYSTEM_MONITOR_TICK_MS);
    }

private:
    via_effect_context  context;
    ViaSystemSensors    sensors;
    unsigned char       base_effect;
    unsigned char       brightness;
    unsigned char       effect_speed;
    RGBColor            low_color;
    RGBColor            high_color;
    int                 sensor;
    float               low;
    float               high;
    float               smoothing;
    float               value;
};

ViaCustomEffect* via_create_system_monitor(const via_effect_context& context)
{
    return(new ViaEffectSystemMonitor(context));
}
