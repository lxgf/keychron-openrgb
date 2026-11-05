/*---------------------------------------------------------*\
| ViaEffectAudio.cpp                                        |
|                                                           |
|   Drives a firmware effect from what is playing:          |
|   - target "brightness": bass pulses the brightness       |
|     between "floor" % and "brightness"                    |
|   - target "speed": bass pushes the effect speed from     |
|     "effect_speed" towards maximum                        |
|   - target "both": both of the above                      |
|   Color: "fixed" uses "bass_color"; "spectrum" blends     |
|   from "bass_color" to "treble_color" as treble grows.    |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "ViaAudioCapture.h"
#include "ViaColor.h"
#include "ViaCustomEffectFactories.h"
#include "ViaLog.h"

#define AUDIO_TICK_MS                   16
#define AUDIO_RETRY_MS                  2000
#define AUDIO_SPEED_MAX                 255.0f
#define AUDIO_SPECTRUM_BIAS             0.1f

class ViaEffectAudio : public ViaCustomEffect
{
public:
    ViaEffectAudio(const via_effect_context& effect_context)
    {
        context         = effect_context;
        base_effect     = VIA_EFFECT_SOLID_COLOR;
        drive_bright    = true;
        drive_speed     = false;
        spectrum        = false;
        sensitivity     = 1.0f;
        floor_level     = 0.05f;
        brightness      = 255;
        effect_speed    = 128;
        bass_color      = ToRGBColor(255, 0, 64);
        treble_color    = ToRGBColor(0, 128, 255);
        last_ms         = 0;
        retry_at_ms     = 0;
        warned          = false;
    }

    void Configure(const nlohmann::json& settings) override
    {
        std::string target = settings.value("target", "brightness");

        base_effect     = via_resolve_effect(context, settings.value("base_effect", "Solid Color"));
        drive_bright    = (target == "brightness" || target == "both");
        drive_speed     = (target == "speed"      || target == "both");
        spectrum        = (settings.value("color_mode", "fixed") == "spectrum");
        sensitivity     = settings.value("sensitivity", 100) / 100.0f;
        floor_level     = settings.value("floor", 5) / 100.0f;
        brightness      = via_setting_percent(settings, "brightness");
        effect_speed    = via_setting_speed(settings, "effect_speed");
        bass_color      = via_setting_color(settings, "bass_color",   ToRGBColor(255, 0, 64));
        treble_color    = via_setting_color(settings, "treble_color", ToRGBColor(0, 128, 255));
    }

    void Start(const via_rgb_state& /*current*/) override
    {
        last_ms     = 0;
        retry_at_ms = 0;
    }

    unsigned int Tick(unsigned long long now_ms, via_rgb_state* state) override
    {
        float bass      = 0.0f;
        float treble    = 0.0f;
        float elapsed_s = (last_ms != 0) ? (now_ms - last_ms) / 1000.0f : 0.0f;

        last_ms = now_ms;

        if(!capture.IsOpen() && now_ms >= retry_at_ms)
        {
            if(!capture.Open())
            {
                if(!warned)
                {
                    VIA_LOG(LL_WARNING, "cannot open audio loopback capture, retrying");
                    warned = true;
                }

                retry_at_ms = now_ms + AUDIO_RETRY_MS;
            }
        }

        if(capture.IsOpen() && !capture.Process(elapsed_s, &bass, &treble))
        {
            retry_at_ms = now_ms + AUDIO_RETRY_MS;
        }

        float level = bass * sensitivity;

        if(level > 1.0f)
        {
            level = 1.0f;
        }

        if(spectrum)
        {
            /*---------------------------------------------*\
            | The bias keeps near-silence on the first      |
            | color instead of flickering on noise          |
            \*---------------------------------------------*/
            via_blend_hue_sat(bass_color, treble_color, treble / (bass + treble + AUDIO_SPECTRUM_BIAS), &state->hue, &state->sat);
        }
        else
        {
            via_blend_hue_sat(bass_color, bass_color, 0.0f, &state->hue, &state->sat);
        }

        state->effect       = base_effect;
        state->brightness   = drive_bright ? (unsigned char)(brightness * (floor_level + (1.0f - floor_level) * level)) : brightness;
        state->speed        = drive_speed  ? (unsigned char)(effect_speed + (AUDIO_SPEED_MAX - effect_speed) * level)          : effect_speed;

        return(capture.IsOpen() ? AUDIO_TICK_MS : AUDIO_RETRY_MS);
    }

    void Stop() override
    {
        capture.Close();
    }

private:
    via_effect_context  context;
    ViaAudioCapture     capture;
    unsigned char       base_effect;
    bool                drive_bright;
    bool                drive_speed;
    bool                spectrum;
    float               sensitivity;
    float               floor_level;
    unsigned char       brightness;
    unsigned char       effect_speed;
    RGBColor            bass_color;
    RGBColor            treble_color;
    unsigned long long  last_ms;
    unsigned long long  retry_at_ms;
    bool                warned;
};

ViaCustomEffect* via_create_audio(const via_effect_context& context)
{
    return(new ViaEffectAudio(context));
}
