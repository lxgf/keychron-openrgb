/*---------------------------------------------------------*\
| ViaEffectPlaylist.cpp                                     |
|                                                           |
|   Plays a queue of firmware effects. Each entry has its   |
|   own duration (0 = the playlist default) and optional    |
|   color (else random or "color"). Each switch             |
|   fades the brightness out, changes the effect while dark |
|   and fades back in, which also hides the firmware's      |
|   restart of the new animation.                           |
|                                                           |
|   Settings "effects": [ { "effect": name,                 |
|                           "duration_s": n,                |
|                           "color": "#RRGGBB" or "" }, ... ]|
|   Plain name strings (older settings) are accepted too.   |
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

typedef struct
{
    unsigned char   effect;
    unsigned int    duration_ms;    /* 0 = playlist default */
    bool            has_color;
    RGBColor        color;
} playlist_entry;

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
        brightness      = 255;
        effect_speed    = 128;
        default_color   = ToRGBColor(198, 0, 255);
        position        = 0;
        phase           = PLAYLIST_SHOWING;
        phase_start_ms  = 0;
        hue             = 0;
        sat             = 255;
    }

    void Configure(const nlohmann::json& settings) override
    {
        entries.clear();

        nlohmann::json queue = settings.value("effects", nlohmann::json::array());

        for(std::size_t queue_idx = 0; queue_idx < queue.size(); queue_idx++)
        {
            playlist_entry  entry   = { VIA_EFFECT_OFF, 0, false, 0 };
            std::string     name;

            if(queue[queue_idx].is_string())
            {
                name = queue[queue_idx].get<std::string>();
            }
            else if(queue[queue_idx].is_object())
            {
                name                = queue[queue_idx].value("effect", "");
                entry.duration_ms   = (unsigned int)queue[queue_idx].value("duration_s", 0) * 1000;
                entry.has_color     = via_parse_color(queue[queue_idx].value("color", ""), &entry.color);
            }

            int effect_id = via_find_effect(context.entry, context.max_effect, name);

            if(effect_id > VIA_EFFECT_OFF)
            {
                entry.effect = (unsigned char)effect_id;
                entries.push_back(entry);
            }
        }

        /*-------------------------------------------------*\
        | Empty queue: play every effect the firmware has   |
        \*-------------------------------------------------*/
        if(entries.empty())
        {
            for(unsigned int effect_id = VIA_EFFECT_SOLID_COLOR; effect_id <= context.max_effect; effect_id++)
            {
                playlist_entry entry = { (unsigned char)effect_id, 0, false, 0 };
                entries.push_back(entry);
            }
        }

        duration_ms = (unsigned int)settings.value("duration_s", 30) * 1000;
        fade_ms     = settings.value("fade_ms", 600);
        shuffle     = settings.value("shuffle", false);
        random_hue  = settings.value("random_hue", false);
        brightness      = via_setting_brightness(settings, "brightness");
        effect_speed    = via_setting_speed(settings, "effect_speed");
        default_color   = via_setting_color(settings, "color", ToRGBColor(198, 0, 255));

        if(position >= entries.size())
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

    unsigned int Tick(unsigned long long now_ms, via_rgb_state* state) override
    {
        if(phase_start_ms == 0)
        {
            phase_start_ms = now_ms;
            PickRandomHue();
        }

        unsigned int    show_ms     = EntryDuration();
        unsigned int    elapsed     = (unsigned int)(now_ms - phase_start_ms);
        unsigned int    half_fade   = fade_ms / 2;
        float           level       = 1.0f;
        unsigned int    wait_ms     = PLAYLIST_FADE_TICK_MS;

        switch(phase)
        {
            case PLAYLIST_SHOWING:
                if(elapsed >= show_ms)
                {
                    Advance(now_ms, half_fade > 0 ? PLAYLIST_FADING_OUT : PLAYLIST_FADING_IN);
                    return(Tick(now_ms, state));
                }
                wait_ms = show_ms - elapsed;
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
                    wait_ms         = EntryDuration();
                }
                else
                {
                    level = (float)elapsed / half_fade;
                }
                break;
        }

        const playlist_entry& entry = entries[position];

        if(entry.has_color)
        {
            via_blend_hue_sat(entry.color, entry.color, 0.0f, &state->hue, &state->sat);
        }
        else if(random_hue)
        {
            state->hue = hue;
            state->sat = sat;
        }
        else
        {
            via_blend_hue_sat(default_color, default_color, 0.0f, &state->hue, &state->sat);
        }

        state->effect       = entry.effect;
        state->brightness   = (unsigned char)(brightness * level);
        state->speed        = effect_speed;

        return(wait_ms);
    }

private:
    via_effect_context          context;
    std::vector<playlist_entry> entries;
    unsigned int                duration_ms;
    unsigned int                fade_ms;
    bool                        shuffle;
    bool                        random_hue;
    unsigned char               brightness;
    unsigned char               effect_speed;
    RGBColor                    default_color;
    std::size_t                 position;
    int                         phase;
    unsigned long long          phase_start_ms;
    unsigned char               hue;
    unsigned char               sat;
    std::mt19937                random_engine;

    unsigned int EntryDuration()
    {
        unsigned int show_ms = (entries[position].duration_ms != 0) ? entries[position].duration_ms : duration_ms;

        return(show_ms < PLAYLIST_MIN_DURATION_MS ? PLAYLIST_MIN_DURATION_MS : show_ms);
    }

    /*-----------------------------------------------------*\
    | Fading in starts the next entry while it is dark      |
    \*-----------------------------------------------------*/
    void Advance(unsigned long long now_ms, int next_phase)
    {
        if(next_phase == PLAYLIST_FADING_IN)
        {
            if(shuffle && entries.size() > 1)
            {
                std::size_t next = position;

                while(next == position)
                {
                    next = std::uniform_int_distribution<std::size_t>(0, entries.size() - 1)(random_engine);
                }

                position = next;
            }
            else
            {
                position = (position + 1) % entries.size();
            }

            PickRandomHue();
        }

        phase           = next_phase;
        phase_start_ms  = now_ms;
    }

    void PickRandomHue()
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
