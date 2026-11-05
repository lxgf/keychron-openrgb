/*---------------------------------------------------------*\
| ViaTranslation.cpp                                        |
|                                                           |
|   UI strings for the languages the plugin speaks          |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstring>
#include <QCoreApplication>
#include "ViaTranslation.h"

typedef struct
{
    const char* english;
    const char* translation;
} via_translation;

static const via_translation russian[] =
{
    /*-----------------------------------------------------*\
    | Plugin                                                |
    \*-----------------------------------------------------*/
    { "Stock QMK/VIA keyboards with RGB Matrix: firmware effects, single-color Direct mode and host-driven custom effects",
      "Клавиатуры QMK/VIA с RGB Matrix на стоковой прошивке: эффекты прошивки, одноцветный Direct и кастомные эффекты с компьютера" },

    /*-----------------------------------------------------*\
    | Settings tab                                          |
    \*-----------------------------------------------------*/
    { "Brightness, speed and colors of each custom mode are set on the Devices tab like any other mode. The options below cover the rest and apply immediately.",
      "Яркость, скорость и цвета кастомных режимов задаются на вкладке «Устройства», как у любого режима. Здесь — остальные параметры, они применяются сразу." },
    { "Firmware effect",                    "Эффект прошивки" },
    { "Saturation",                         "Насыщенность" },
    { "Effect speed",                       "Скорость эффекта" },
    { "Mode speed: how fast the hue goes round (1 = 2 min per turn, 100 = 1.2 s).",
      "Скорость режима — скорость смены оттенка (1 = круг за 2 мин, 100 = за 1,2 с)." },

    { "Listens to the default output device. Mode speed is the resting effect speed; colors: bass, treble.",
      "Слушает устройство вывода по умолчанию. Скорость режима — скорость эффекта в покое; цвета: басы, верха." },
    { "Bass drives",                        "Басы управляют" },
    { "Brightness",                         "Яркостью" },
    { "Speed",                              "Скоростью эффекта" },
    { "Both",                               "Обоими" },
    { "Color",                              "Цвет" },
    { "First color",                        "Первый цвет" },
    { "Bass to treble blend",               "Смесь басы → верха" },
    { "Sensitivity",                        "Чувствительность" },
    { "Minimum brightness",                 "Минимальная яркость" },

    { "First color at or below Low, second at or above High, blended in between.",
      "Первый цвет — на нижнем пороге и ниже, второй — на верхнем и выше, между ними — плавный переход." },
    { "Source",                             "Источник" },
    { "CPU load",                           "Загрузка CPU" },
    { "RAM usage",                          "Занятость RAM" },
    { "GPU load (3D)",                      "Загрузка GPU (3D)" },
    { "Low",                                "Нижний порог" },
    { "High",                               "Верхний порог" },
    { "Smoothing",                          "Сглаживание" },

    { "Mode speed is the effect speed; mode color is the default entry color.",
      "Скорость режима — скорость эффектов; цвет режима — цвет пунктов по умолчанию." },
    { "Show each for",                      "Показывать каждый" },
    { "Fade",                               "Переход" },
    { "Shuffle",                            "Случайный порядок" },
    { "Random color for entries without one", "Случайный цвет для пунктов без цвета" },

    { "Colors: day, night. Mode brightness is the day brightness.",
      "Цвета: день, ночь. Яркость режима — дневная яркость." },
    { "Day starts",                         "День начинается" },
    { "Night starts",                       "Ночь начинается" },
    { "Transition",                         "Переход" },
    { "Night brightness",                   "Ночная яркость" },

    { " s",                                 " с" },
    { " ms",                                " мс" },
    { " min",                               " мин" },

    /*-----------------------------------------------------*\
    | Playlist queue                                        |
    \*-----------------------------------------------------*/
    { "Effect",                             "Эффект" },
    { "Duration",                           "Длительность" },
    { "Add",                                "Добавить" },
    { "Remove",                             "Удалить" },
    { "Up",                                 "Вверх" },
    { "Down",                               "Вниз" },
    { "default",                            "общая" },
    { "mode",                               "режима" },
    { "Entry color",                        "Цвет пункта" },
    { "Empty queue plays every effect. Duration 0 uses \"Show each for\"; no color uses the mode color (or a random one).",
      "Пустая очередь — играют все эффекты. Длительность 0 — общее время показа; без цвета — цвет режима (или случайный)." },
};

static bool via_ui_is_russian()
{
    return(QCoreApplication::translate("Settings", "English - US") == QString::fromUtf8("Русский"));
}

QString via_tr(const char* text)
{
    if(via_ui_is_russian())
    {
        for(std::size_t entry_idx = 0; entry_idx < sizeof(russian) / sizeof(russian[0]); entry_idx++)
        {
            if(strcmp(russian[entry_idx].english, text) == 0)
            {
                return(QString::fromUtf8(russian[entry_idx].translation));
            }
        }
    }

    return(QString::fromUtf8(text));
}
