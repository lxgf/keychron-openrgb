/*---------------------------------------------------------*\
| ViaSettingsWidget.cpp                                     |
|                                                           |
|   Plugin tab under Settings for custom effect options     |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QEvent>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QSpinBox>
#include <QTime>
#include <QTimeEdit>
#include <QTimer>
#include <QVBoxLayout>
#include "ViaPlaylistEditor.h"
#include "ViaRGBMatrixDevices.h"
#include "ViaSettingsWidget.h"
#include "ViaTranslation.h"

#define SAVE_DELAY_MS                   500
#define COLOR_SWATCH_WIDTH              60

ViaSettingsWidget::ViaSettingsWidget(ViaSettings* settings_ptr, QWidget* parent)
    : QWidget(parent)
{
    settings        = settings_ptr;
    effect_names    = via_all_effect_names();

    save_timer = new QTimer(this);
    save_timer->setSingleShot(true);
    save_timer->setInterval(SAVE_DELAY_MS);
    connect(save_timer, &QTimer::timeout, this, [this]{ settings->Save(); });

    scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(scroll);

    BuildContent();
}

/*---------------------------------------------------------*\
| OpenRGB reinstalls its translator on every settings save, |
| ours included, so LanguageChange arrives far more often   |
| than the language actually changes: rebuild only then.    |
| Rebuilding is simpler than retranslating every widget.    |
\*---------------------------------------------------------*/
void ViaSettingsWidget::changeEvent(QEvent* event)
{
    if(event->type() == QEvent::LanguageChange && via_ui_language() != built_language)
    {
        BuildContent();
    }

    QWidget::changeEvent(event);
}

void ViaSettingsWidget::BuildContent()
{
    int         scroll_position = scroll->verticalScrollBar()->value();
    QWidget*    old_content     = scroll->takeWidget();

    built_language = via_ui_language();

    if(old_content != NULL)
    {
        old_content->deleteLater();
    }

    QWidget*        content         = new QWidget();
    QVBoxLayout*    content_layout  = new QVBoxLayout(content);
    QLabel*         intro           = new QLabel(via_tr("Custom modes are set up only here: brightness, colors and speeds included. "
                                                        "On the Devices tab their controls stay inactive. Changes apply immediately."));

    intro->setWordWrap(true);
    content_layout->addWidget(intro);

    QFormLayout* form;

    form = AddGroup(content, "Hue Cycle", "Moves the hue of the firmware effect round the whole wheel or back and forth between two colors (hue speed 1 = 2 min per cycle, 100 = 1.2 s).");
    AddEffectChoice(form, "hue_cycle", "base_effect",  "Firmware effect");
    AddNumber      (form, "hue_cycle", "brightness",   "Brightness",    0, 100, " %");
    AddChoice      (form, "hue_cycle", "range",        "Colors",        { { "Whole color wheel", "full" }, { "Between two colors", "between" } });
    AddNumber      (form, "hue_cycle", "saturation",   "Saturation",    0, 100, " %");
    AddColor       (form, "hue_cycle", "from_color",   "From color");
    AddColor       (form, "hue_cycle", "to_color",     "To color");
    AddNumber      (form, "hue_cycle", "effect_speed", "Effect speed",  0, 255, "");
    AddNumber      (form, "hue_cycle", "hue_speed",    "Hue speed",     1, 100, "");

    form = AddGroup(content, "Audio Reactive", "Listens to the default output device. Effect speed is the resting speed when bass drives the speed.");
    AddEffectChoice(form, "audio", "base_effect",  "Firmware effect");
    AddNumber      (form, "audio", "brightness",   "Brightness",    0, 100, " %");
    AddColor       (form, "audio", "bass_color",   "Bass color");
    AddColor       (form, "audio", "treble_color", "Treble color");
    AddChoice      (form, "audio", "target",       "Bass drives",   { { "Brightness", "brightness" }, { "Speed", "speed" }, { "Both", "both" } });
    AddChoice      (form, "audio", "color_mode",   "Color",         { { "Bass color only", "fixed" }, { "Bass to treble blend", "spectrum" } });
    AddNumber      (form, "audio", "sensitivity",  "Sensitivity",   10, 400, " %");
    AddNumber      (form, "audio", "floor",        "Minimum brightness", 0, 100, " %");
    AddNumber      (form, "audio", "effect_speed", "Effect speed",  0, 255, "");

    form = AddGroup(content, "System Monitor", "Low color at or below the low threshold, high color at or above the high one, blended in between.");
    AddChoice      (form, "system", "source",       "Source",       { { "CPU load", "cpu" }, { "RAM usage", "ram" }, { "GPU load (3D)", "gpu" } });
    AddEffectChoice(form, "system", "base_effect",  "Firmware effect");
    AddNumber      (form, "system", "brightness",   "Brightness",   0, 100, " %");
    AddColor       (form, "system", "low_color",    "Low color");
    AddColor       (form, "system", "high_color",   "High color");
    AddNumber      (form, "system", "low",          "Low",          0, 100, " %");
    AddNumber      (form, "system", "high",         "High",         0, 100, " %");
    AddNumber      (form, "system", "smoothing",    "Smoothing",    0, 95,  " %");
    AddNumber      (form, "system", "effect_speed", "Effect speed", 0, 255, "");

    form = AddGroup(content, "Effect Playlist", "The default color is used by entries without their own color.");
    AddPlaylistQueue(form, "playlist", "effects");
    AddNumber      (form, "playlist", "brightness",   "Brightness",     0, 100, " %");
    AddColor       (form, "playlist", "color",        "Default color");
    AddNumber      (form, "playlist", "duration_s",   "Show each for",  1, 3600, " s");
    AddNumber      (form, "playlist", "fade_ms",      "Fade",           0, 5000, " ms");
    AddNumber      (form, "playlist", "effect_speed", "Effect speed",   0, 255, "");
    AddCheck       (form, "playlist", "shuffle",      "Shuffle");
    AddCheck       (form, "playlist", "random_hue",   "Random color for entries without one");

    form = AddGroup(content, "Day / Night", "Switches between the day and night look, with a gradual transition after each start time.");
    AddEffectChoice(form, "daynight", "base_effect",      "Firmware effect");
    AddNumber      (form, "daynight", "brightness",       "Day brightness",   0, 100, " %");
    AddNumber      (form, "daynight", "night_brightness", "Night brightness", 0, 100, " %");
    AddColor       (form, "daynight", "day_color",        "Day color");
    AddColor       (form, "daynight", "night_color",      "Night color");
    AddTime        (form, "daynight", "day_start",        "Day starts");
    AddTime        (form, "daynight", "night_start",      "Night starts");
    AddNumber      (form, "daynight", "transition_min",   "Transition",       0, 240, " min");
    AddNumber      (form, "daynight", "effect_speed",     "Effect speed",     0, 255, "");

    content_layout->addStretch();

    scroll->setWidget(content);

    /*-----------------------------------------------------*\
    | The new content has no layout (and no scroll range)   |
    | yet; restore the position once it has                 |
    \*-----------------------------------------------------*/
    QTimer::singleShot(0, scroll, [this, scroll_position]{ scroll->verticalScrollBar()->setValue(scroll_position); });
}

ViaSettingsWidget::~ViaSettingsWidget()
{
    if(save_timer->isActive())
    {
        save_timer->stop();
        settings->Save();
    }
}

QFormLayout* ViaSettingsWidget::AddGroup(QWidget* content, const char* title, const char* hint)
{
    QGroupBox*      group   = new QGroupBox(title);
    QFormLayout*    form    = new QFormLayout(group);
    QLabel*         label   = new QLabel(via_tr(hint));

    label->setWordWrap(true);
    form->addRow(label);

    content->layout()->addWidget(group);

    return(form);
}

void ViaSettingsWidget::Store(const char* section, const char* key, const nlohmann::json& value)
{
    settings->Set(section, key, value);
    save_timer->start();
}

void ViaSettingsWidget::AddEffectChoice(QFormLayout* form, const char* section, const char* key, const char* label)
{
    QComboBox*  combo   = new QComboBox();
    std::string current = settings->GetSection(section).value(key, "");

    for(std::size_t name_idx = 0; name_idx < effect_names.size(); name_idx++)
    {
        combo->addItem(QString::fromStdString(effect_names[name_idx]));
    }

    if(combo->findText(QString::fromStdString(current)) < 0 && !current.empty())
    {
        combo->addItem(QString::fromStdString(current));
    }

    combo->setCurrentText(QString::fromStdString(current));

    connect(combo, &QComboBox::currentTextChanged, this, [this, section, key](const QString& text)
    {
        Store(section, key, text.toStdString());
    });

    form->addRow(via_tr(label), combo);
}

void ViaSettingsWidget::AddChoice(QFormLayout* form, const char* section, const char* key, const char* label, const choice_list& choices)
{
    QComboBox*  combo   = new QComboBox();
    std::string current = settings->GetSection(section).value(key, "");

    for(std::size_t choice_idx = 0; choice_idx < choices.size(); choice_idx++)
    {
        combo->addItem(via_tr(choices[choice_idx].first), QString(choices[choice_idx].second));

        if(current == choices[choice_idx].second)
        {
            combo->setCurrentIndex((int)choice_idx);
        }
    }

    connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, combo, section, key](int)
    {
        Store(section, key, combo->currentData().toString().toStdString());
    });

    form->addRow(via_tr(label), combo);
}

void ViaSettingsWidget::AddNumber(QFormLayout* form, const char* section, const char* key, const char* label, int min, int max, const char* suffix)
{
    QSpinBox* spin = new QSpinBox();

    spin->setRange(min, max);
    spin->setSuffix(via_tr(suffix));
    spin->setValue(settings->GetSection(section).value(key, min));

    connect(spin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this, section, key](int value)
    {
        Store(section, key, value);
    });

    form->addRow(via_tr(label), spin);
}

void ViaSettingsWidget::AddCheck(QFormLayout* form, const char* section, const char* key, const char* label)
{
    QCheckBox* check = new QCheckBox(via_tr(label));

    check->setChecked(settings->GetSection(section).value(key, false));

    connect(check, &QCheckBox::toggled, this, [this, section, key](bool checked)
    {
        Store(section, key, checked);
    });

    form->addRow(check);
}

void ViaSettingsWidget::AddTime(QFormLayout* form, const char* section, const char* key, const char* label)
{
    QTimeEdit*  edit    = new QTimeEdit();
    QTime       current = QTime::fromString(QString::fromStdString(settings->GetSection(section).value(key, "00:00")), "HH:mm");

    edit->setDisplayFormat("HH:mm");
    edit->setTime(current.isValid() ? current : QTime(0, 0));

    connect(edit, &QTimeEdit::timeChanged, this, [this, section, key](const QTime& time)
    {
        Store(section, key, time.toString("HH:mm").toStdString());
    });

    form->addRow(via_tr(label), edit);
}

void ViaSettingsWidget::AddPlaylistQueue(QFormLayout* form, const char* section, const char* key)
{
    nlohmann::json      queue   = settings->GetSection(section).value(key, nlohmann::json::array());
    ViaPlaylistEditor*  editor  = new ViaPlaylistEditor(effect_names, queue, [this, section, key](const nlohmann::json& new_queue)
    {
        Store(section, key, new_queue);
    });

    form->addRow(editor);
}

void ViaSettingsWidget::AddColor(QFormLayout* form, const char* section, const char* key, const char* label)
{
    QPushButton*    swatch  = new QPushButton();
    QString         current = QString::fromStdString(settings->GetSection(section).value(key, "#ffffff"));

    swatch->setFixedWidth(COLOR_SWATCH_WIDTH);
    swatch->setStyleSheet(QString("background-color: %1;").arg(current));

    connect(swatch, &QPushButton::clicked, this, [this, swatch, section, key, label]
    {
        QColor initial = QColor(QString::fromStdString(settings->GetSection(section).value(key, "#ffffff")));
        QColor picked  = QColorDialog::getColor(initial, this, via_tr(label));

        if(!picked.isValid())
        {
            return;
        }

        QString name = picked.name(QColor::HexRgb);

        swatch->setStyleSheet(QString("background-color: %1;").arg(name));
        Store(section, key, name.toStdString());
    });

    form->addRow(via_tr(label), swatch);
}
