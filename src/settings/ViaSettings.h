/*---------------------------------------------------------*\
| ViaSettings.h                                             |
|                                                           |
|   Custom effect settings, one JSON section per effect,    |
|   persisted in OpenRGB.json through the plugin API.       |
|   Defaults are merged in on load, so readers always see   |
|   every key.                                              |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <functional>
#include <mutex>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "OpenRGBPluginInterface.h"

typedef std::function<void()> via_settings_listener;

class ViaSettings
{
public:
    ViaSettings(OpenRGBPluginAPIInterface* api);

    nlohmann::json  GetSection(const std::string& section);
    void            Set(const std::string& section, const std::string& key, const nlohmann::json& value);
    void            Save();

    unsigned int    AddListener(via_settings_listener listener);
    void            RemoveListener(unsigned int listener_id);

private:
    OpenRGBPluginAPIInterface*          plugin_api;
    std::mutex                          settings_lock;
    nlohmann::json                      settings;

    std::mutex                          listener_lock;
    std::vector<unsigned int>           listener_ids;
    std::vector<via_settings_listener>  listeners;
    unsigned int                        next_listener_id;
};
