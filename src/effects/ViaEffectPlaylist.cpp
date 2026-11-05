/*---------------------------------------------------------*\
| ViaEffectPlaylist.cpp                                     |
|                                                           |
|   Cycles through firmware effects. Each switch fades the  |
|   brightness out, changes the effect while dark and fades |
|   back in, which also hides the firmware's restart of    |
|   the new animation. Mode speed is the effect speed.      |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <random>
#include <vector>
#include "ViaColor.h"
#include "ViaCustomEffectFactories.h"

#define PLAYLIST_FADE_TICK_MS           16
#define PLAYLIST_MIN_DURATION_MS        1000

enum
{
    PLAYLIST_SHOWING,
    PLAYLIST_FADING_OUT,
    PLAYLIST_FADING_IN,
};

class ViaEffectPlaylist : public ViaCustomEffect
{
public:
    ViaEffectPlaylist(const via_effect_context& effect_context)
        : random_engine(std::random_device{}())
    {
        context         = effect_context;
        duration_ms     = 30000;
        fade_ms         = 600;
        shuffle         = false;
        random_hue      = false;
        position        = 0;
        phase           = PLAYLIST_SHOWING;
        phase_start_ms  = 0;
        hue             = 0;
        sat             = 255;
    }

    void Configure(const nlohmann::json& settings) override
    {
        effects.clear();

        nlohmann::json names = settings.value("effects", nlohmann::json::array());

        for(std::size_t name_idx = 0; name_idx < names.size(); name_idx++)
        {
            if(!names[name_idx].is_string())
            {
                continue;
            }

            int effect_id = via_find_effect(context.entry, context.max_effect, names[name_idx].get<std::string>());

            if(effect_id > VIA_EFFECT_OFF)
            {
                effects.push_back((unsigned char)effect_id);
            }
        }

        /*-------------------------------------------------*\
        | Empty list: play every effect the firmware has    |
        \*-------------------------------------------------*/
        if(effects.empty())
        {
            for(unsigned int effect_id = VIA_EFFECT_SOLID_COLOR; effect_id <= context.max_effect; effect_id++)
            {
                effects.push_back((unsigned char)effect_id);
            }
        }

        duration_ms = (unsigned int)settings.value("duration_s", 30) * 1000;
        fade_ms     = settings.value("fade_ms", 600);
        shuffle     = settings.value("shuffle", false);
        random_hue  = settings.value("random_hue", false);

        if(duration_ms < PLAYLIST_MIN_DURATION_MS)
        {
            duration_ms = PLAYLIST_MIN_DURATION_MS;
        }

        if(position >= effects.size())
        {
            position = 0;
        }
    }

    void Start(const via_rgb_state& /*current*/) override
    {
        position        = 0;
        phase           = PLAYLIST_SHOWING;
        phase_start_ms  = 0;
    }

    unsigned int Tick(unsigned long long now_ms, const via_effect_params& params, via_rgb_state* state) override
    {
        if(phase_start_ms == 0)
        {
            phase_start_ms = now_ms;
            PickColor();
        }

        unsigned int    elapsed     = (unsigned int)(now_ms - phase_start_ms);
        unsigned int    half_fade   = fade_ms / 2;
        float           level       = 1.0f;
        unsigned int    wait_ms     = PLAYLIST_FADE_TICK_MS;

        switch(phase)
        {
            case PLAYLIST_SHOWING:
                if(elapsed >= duration_ms)
                {
                    Advance(now_ms, half_fade > 0 ? PLAYLIST_FADING_OUT : PLAYLIST_FADING_IN);
                    return(Tick(now_ms, params, state));
                }
                wait_ms = duration_ms - elapsed;
                break;

            case PLAYLIST_FADING_OUT:
                if(elapsed >= half_fade)
                {
                    Advance(now_ms, PLAYLIST_FADING_IN);
                    level = 0.0f;
                }
                else
                {
                    level = 1.0f - (float)elapsed / half_fade;
                }
                break;

            case PLAYLIST_FADING_IN:
                if(half_fade == 0 || elapsed >= half_fade)
                {
                    phase           = PLAYLIST_SHOWING;
                    phase_start_ms  = now_ms;
                    wait_ms         = duration_ms;
                }
                else
                {
                    level = (float)elapsed / half_fade;
                }
                break;
        }

        if(!random_hue)
        {
            via_blend_hue_sat(params.colors[0], params.colors[0], 0.0f, &hue, &sat);
        }

        state->effect       = effects[position];
        state->hue          = hue;
        state->sat          = sat;
        state->brightness   = (unsigned char)(params.brightness * level);
        state->speed        = params.speed;

        return(wait_ms);
    }

private:
    via_effect_context          context;
    std::vector<unsigned char>  effects;
    unsigned int                duration_ms;
    unsigned int                fade_ms;
    bool                        shuffle;
    bool                        random_hue;
    std::size_t                 position;
    int                         phase;
    unsigned long long          phase_start_ms;
    unsigned char               hue;
    unsigned char               sat;
    std::mt19937                random_engine;

    /*-----------------------------------------------------*\
    | Fading in starts the next effect while it is dark     |
    \*-----------------------------------------------------*/
    void Advance(unsigned long long now_ms, int next_phase)
    {
        if(next_phase == PLAYLIST_FADING_IN)
        {
            if(shuffle && effects.size() > 1)
            {
                std::size_t next = position;

                while(next == position)
                {
                    next = std::uniform_int_distribution<std::size_t>(0, effects.size() - 1)(random_engine);
                }

                position = next;
            }
            else
            {
                position = (position + 1) % effects.size();
            }

            PickColor();
        }

        phase           = next_phase;
        phase_start_ms  = now_ms;
    }

    void PickColor()
    {
        if(random_hue)
        {
            hue = (unsigned char)std::uniform_int_distribution<int>(0, 255)(random_engine);
            sat = 255;
        }
    }
};

ViaCustomEffect* via_create_playlist(const via_effect_context& context)
{
    return(new ViaEffectPlaylist(context));
}
