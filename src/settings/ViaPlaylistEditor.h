/*---------------------------------------------------------*\
| ViaPlaylistEditor.h                                       |
|                                                           |
|   Queue editor for the Effect Playlist: add, remove and   |
|   reorder entries; each entry has an effect, its own      |
|   duration (0 = playlist default) and an optional color.  |
|   Reports the whole queue as JSON on every change.        |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <functional>
#include <string>
#include <vector>
#include <QWidget>
#include <nlohmann/json.hpp>

class QTableWidget;

typedef struct
{
    std::string     effect;
    int             duration_s;
    std::string     color;      /* "#RRGGBB", empty = mode color */
} via_playlist_item;

class ViaPlaylistEditor : public QWidget
{
public:
    ViaPlaylistEditor(const std::vector<std::string>& effect_names, const nlohmann::json& queue, std::function<void(const nlohmann::json&)> on_changed, QWidget* parent = nullptr);

private:
    std::vector<std::string>                    effect_names;
    std::vector<via_playlist_item>              items;
    std::function<void(const nlohmann::json&)>  changed;
    QTableWidget*                               table;

    void    Rebuild(int select_row);
    void    Add();
    void    RemoveSelected();
    void    MoveSelected(int offset);
    void    PickColor(int row);
    void    Notify();
};
