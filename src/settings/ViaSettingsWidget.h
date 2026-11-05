/*---------------------------------------------------------*\
| ViaSettingsWidget.h                                       |
|                                                           |
|   Plugin tab under Settings: the custom effect options    |
|   that OpenRGB's per-mode controls (brightness, speed,    |
|   colors) cannot express. Changes apply immediately and   |
|   are written to OpenRGB.json shortly after.              |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <utility>
#include <vector>
#include <QWidget>
#include "ViaSettings.h"

class QFormLayout;
class QTimer;

class ViaSettingsWidget : public QWidget
{
public:
    ViaSettingsWidget(ViaSettings* settings_ptr, QWidget* parent = nullptr);
    ~ViaSettingsWidget();

private:
    typedef std::vector<std::pair<const char*, const char*>> choice_list;

    ViaSettings*                settings;
    QTimer*                     save_timer;
    std::vector<std::string>    effect_names;

    QFormLayout*    AddGroup(QWidget* content, const char* title, const char* hint);

    void            AddEffectChoice(QFormLayout* form, const char* section, const char* key, const char* label);
    void            AddChoice(QFormLayout* form, const char* section, const char* key, const char* label, const choice_list& choices);
    void            AddNumber(QFormLayout* form, const char* section, const char* key, const char* label, int min, int max, const char* suffix);
    void            AddCheck(QFormLayout* form, const char* section, const char* key, const char* label);
    void            AddTime(QFormLayout* form, const char* section, const char* key, const char* label);
    void            AddEffectList(QFormLayout* form, const char* section, const char* key, const char* label);

    void            Store(const char* section, const char* key, const nlohmann::json& value);
};
