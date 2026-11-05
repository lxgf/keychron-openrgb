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
    { "Custom modes are set up only here: brightness, colors and speeds included. On the Devices tab their controls stay inactive. Changes apply immediately.",
      "Кастомные режимы настраиваются только здесь, включая яркость, цвета и скорости. На вкладке «Устройства» их элементы управления неактивны. Изменения применяются сразу." },

    { "Firmware effect",                    "Эффект прошивки" },
    { "Brightness",                         "Яркость" },
    { "Saturation",                         "Насыщенность" },
    { "Effect speed",                       "Скорость эффекта" },
    { "Hue speed",                          "Скорость смены оттенка" },

    { "Walks the hue of the firmware effect round the color wheel (hue speed 1 = 2 min per turn, 100 = 1.2 s).",
      "Ведёт оттенок эффекта прошивки по цветовому кругу (скорость смены оттенка 1 = круг за 2 мин, 100 = за 1,2 с)." },

    { "Listens to the default output device. Effect speed is the resting speed when bass drives the speed.",
      "Слушает устройство вывода по умолчанию. Скорость эффекта — скорость в покое, когда басы управляют скоростью." },
    { "Bass color",                         "Цвет басов" },
    { "Treble color",                       "Цвет верхов" },
    { "Bass drives",                        "Басы управляют" },
    { "Speed",                              "Скоростью эффекта" },
    { "Both",                               "Обоими" },
    { "Color",                              "Цвет" },
    { "Bass color only",                    "Только цвет басов" },
    { "Bass to treble blend",               "Смесь басы → верха" },
    { "Sensitivity",                        "Чувствительность" },
    { "Minimum brightness",                 "Минимальная яркость" },

    { "Low color at or below the low threshold, high color at or above the high one, blended in between.",
      "Нижний цвет — на нижнем пороге и ниже, верхний — на верхнем и выше, между ними — плавный переход." },
    { "Source",                             "Источник" },
    { "CPU load",                           "Загрузка CPU" },
    { "RAM usage",                          "Занятость RAM" },
    { "GPU load (3D)",                      "Загрузка GPU (3D)" },
    { "Low color",                          "Нижний цвет" },
    { "High color",                         "Верхний цвет" },
    { "Low",                                "Нижний порог" },
    { "High",                               "Верхний порог" },
    { "Smoothing",                          "Сглаживание" },

    { "The default color is used by entries without their own color.",
      "Общий цвет используется для пунктов без своего цвета." },
    { "Default color",                      "Общий цвет" },
    { "Show each for",                      "Показывать каждый" },
    { "Fade",                               "Переход" },
    { "Shuffle",                            "Случайный порядок" },
    { "Random color for entries without one", "Случайный цвет для пунктов без цвета" },

    { "Switches between the day and night look, with a gradual transition after each start time.",
      "Переключает дневной и ночной вид с плавным переходом после каждого времени начала." },
    { "Day brightness",                     "Дневная яркость" },
    { "Night brightness",                   "Ночная яркость" },
    { "Day color",                          "Дневной цвет" },
    { "Night color",                        "Ночной цвет" },
    { "Day starts",                         "День начинается" },
    { "Night starts",                       "Ночь начинается" },
    { "Transition",                         "Переход" },

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
    { "shared",                             "общий" },
    { "Entry color",                        "Цвет пункта" },
    { "Empty queue plays every effect. Default duration is \"Show each for\"; default color is the playlist's \"Default color\" (or a random one).",
      "Пустая очередь — играют все эффекты. Общая длительность — «Показывать каждый», общий цвет — «Общий цвет» плейлиста (или случайный)." },
};

QString via_ui_language()
{
    return(QCoreApplication::translate("Settings", "English - US"));
}

static bool via_ui_is_russian()
{
    return(via_ui_language() == QString::fromUtf8("Русский"));
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
