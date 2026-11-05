/*---------------------------------------------------------*\
| ViaSettings.cpp                                           |
|                                                           |
|   Custom effect settings persisted in OpenRGB.json        |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include "ViaSettings.h"

#define VIA_SETTINGS_KEY                "VIARGBMatrixPlugin"

static nlohmann::json via_settings_defaults()
{
    return(nlohmann::json
    {
        { "hue_cycle",
            {
                { "base_effect",        "Solid Splash" },
                { "saturation",         255 },
                { "effect_speed",       128 },
            }
        },
        { "audio",
            {
                { "base_effect",        "Solid Color" },
                { "target",             "brightness" },
                { "color_mode",         "fixed" },
                { "sensitivity",        100 },
                { "floor",              5 },
            }
        },
        { "system",
            {
                { "source",             "cpu" },
                { "base_effect",        "Solid Color" },
                { "low",                10 },
                { "high",               90 },
                { "smoothing",          60 },
            }
        },
        { "playlist",
            {
                { "effects",            nlohmann::json::array() },
                { "duration_s",         30 },
                { "fade_ms",            600 },
                { "shuffle",            false },
                { "random_hue",         false },
            }
        },
        { "daynight",
            {
                { "base_effect",        "Solid Color" },
                { "day_start",          "07:00" },
                { "night_start",        "22:00" },
                { "transition_min",     30 },
                { "night_brightness",   25 },
            }
        },
    });
}

ViaSettings::ViaSettings(OpenRGBPluginAPIInterface* api)
{
    plugin_api          = api;
    next_listener_id    = 1;
    settings            = via_settings_defaults();

    nlohmann::json stored = plugin_api->GetSettings(VIA_SETTINGS_KEY);

    if(stored.is_object())
    {
        settings.merge_patch(stored);
    }
}

nlohmann::json ViaSettings::GetSection(const std::string& section)
{
    std::lock_guard<std::mutex> guard(settings_lock);

    if(settings.contains(section))
    {
        return(settings[section]);
    }

    return(nlohmann::json::object());
}

void ViaSettings::Set(const std::string& section, const std::string& key, const nlohmann::json& value)
{
    {
        std::lock_guard<std::mutex> guard(settings_lock);
        settings[section][key] = value;
    }

    std::vector<via_settings_listener> to_notify;
    {
        std::lock_guard<std::mutex> guard(listener_lock);
        to_notify = listeners;
    }

    for(std::size_t listener_idx = 0; listener_idx < to_notify.size(); listener_idx++)
    {
        to_notify[listener_idx]();
    }
}

void ViaSettings::Save()
{
    nlohmann::json snapshot;
    {
        std::lock_guard<std::mutex> guard(settings_lock);
        snapshot = settings;
    }

    plugin_api->SetSettings(VIA_SETTINGS_KEY, snapshot);
    plugin_api->SaveSettings();
}

unsigned int ViaSettings::AddListener(via_settings_listener listener)
{
    std::lock_guard<std::mutex> guard(listener_lock);

    unsigned int listener_id = next_listener_id++;

    listener_ids.push_back(listener_id);
    listeners.push_back(listener);

    return(listener_id);
}

void ViaSettings::RemoveListener(unsigned int listener_id)
{
    std::lock_guard<std::mutex> guard(listener_lock);

    for(std::size_t listener_idx = 0; listener_idx < listener_ids.size(); listener_idx++)
    {
        if(listener_ids[listener_idx] == listener_id)
        {
            listener_ids.erase(listener_ids.begin() + listener_idx);
            listeners.erase(listeners.begin() + listener_idx);
            return;
        }
    }
}
