/*---------------------------------------------------------*\
| ViaSettingsWidget.cpp                                     |
|                                                           |
|   Plugin tab under Settings for custom effect options     |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <QCheckBox>
#include <QComboBox>
#include <QEvent>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QScrollArea>
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
| OpenRGB swaps its translator when the language changes;   |
| rebuilding is simpler than retranslating every widget     |
\*---------------------------------------------------------*/
void ViaSettingsWidget::changeEvent(QEvent* event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        BuildContent();
    }

    QWidget::changeEvent(event);
}

void ViaSettingsWidget::BuildContent()
{
    QWidget* old_content = scroll->takeWidget();

    if(old_content != NULL)
    {
        old_content->deleteLater();
    }

    QWidget*        content         = new QWidget();
    QVBoxLayout*    content_layout  = new QVBoxLayout(content);
    QLabel*         intro           = new QLabel(via_tr("Brightness, speed and colors of each custom mode are set on the Devices tab like any other mode. "
                                                        "The options below cover the rest and apply immediately."));

    intro->setWordWrap(true);
    content_layout->addWidget(intro);

    QFormLayout* form;

    form = AddGroup(content, "Hue Cycle", "Mode speed: how fast the hue goes round (1 = 2 min per turn, 100 = 1.2 s).");
    AddEffectChoice(form, "hue_cycle", "base_effect",  "Firmware effect");
    AddNumber      (form, "hue_cycle", "saturation",   "Saturation",    0, 255, "");
    AddNumber      (form, "hue_cycle", "effect_speed", "Effect speed",  0, 255, "");

    form = AddGroup(content, "Audio Reactive", "Listens to the default output device. Mode speed is the resting effect speed; colors: bass, treble.");
    AddEffectChoice(form, "audio", "base_effect", "Firmware effect");
    AddChoice      (form, "audio", "target",      "Bass drives",  { { "Brightness", "brightness" }, { "Speed", "speed" }, { "Both", "both" } });
    AddChoice      (form, "audio", "color_mode",  "Color",        { { "First color", "fixed" }, { "Bass to treble blend", "spectrum" } });
    AddNumber      (form, "audio", "sensitivity", "Sensitivity",  10, 400, " %");
    AddNumber      (form, "audio", "floor",       "Minimum brightness", 0, 100, " %");

    form = AddGroup(content, "System Monitor", "First color at or below Low, second at or above High, blended in between.");
    AddChoice      (form, "system", "source",      "Source",          { { "CPU load", "cpu" }, { "RAM usage", "ram" }, { "GPU load (3D)", "gpu" } });
    AddEffectChoice(form, "system", "base_effect", "Firmware effect");
    AddNumber      (form, "system", "low",         "Low",             0, 100, " %");
    AddNumber      (form, "system", "high",        "High",            0, 100, " %");
    AddNumber      (form, "system", "smoothing",   "Smoothing",       0, 95,  " %");

    form = AddGroup(content, "Effect Playlist", "Mode speed is the effect speed; mode color is the default entry color.");
    AddPlaylistQueue(form, "playlist", "effects");
    AddNumber      (form, "playlist", "duration_s", "Show each for", 1, 3600, " s");
    AddNumber      (form, "playlist", "fade_ms",    "Fade",          0, 5000, " ms");
    AddCheck       (form, "playlist", "shuffle",    "Shuffle");
    AddCheck       (form, "playlist", "random_hue", "Random color for entries without one");

    form = AddGroup(content, "Day / Night", "Colors: day, night. Mode brightness is the day brightness.");
    AddEffectChoice(form, "daynight", "base_effect",      "Firmware effect");
    AddTime        (form, "daynight", "day_start",        "Day starts");
    AddTime        (form, "daynight", "night_start",      "Night starts");
    AddNumber      (form, "daynight", "transition_min",   "Transition",       0, 240, " min");
    AddNumber      (form, "daynight", "night_brightness", "Night brightness", 0, 100, " %");

    content_layout->addStretch();

    scroll->setWidget(content);
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
