/*---------------------------------------------------------*\
| ViaPlaylistEditor.cpp                                     |
|                                                           |
|   Queue editor for the Effect Playlist                    |
|                                                           |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <QColorDialog>
#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include "ViaPlaylistEditor.h"

#define PLAYLIST_TABLE_MIN_HEIGHT       180
#define PLAYLIST_MAX_DURATION_S         3600
#define PLAYLIST_SWATCH_SIZE            20

enum
{
    COLUMN_EFFECT,
    COLUMN_DURATION,
    COLUMN_COLOR,
    COLUMN_COUNT,
};

ViaPlaylistEditor::ViaPlaylistEditor(const std::vector<std::string>& names, const nlohmann::json& queue, std::function<void(const nlohmann::json&)> on_changed, QWidget* parent)
    : QWidget(parent)
{
    effect_names    = names;
    changed         = on_changed;

    /*-----------------------------------------------------*\
    | Accept plain names from older settings                |
    \*-----------------------------------------------------*/
    for(std::size_t queue_idx = 0; queue_idx < queue.size(); queue_idx++)
    {
        via_playlist_item item = { "", 0, "" };

        if(queue[queue_idx].is_string())
        {
            item.effect     = queue[queue_idx].get<std::string>();
        }
        else if(queue[queue_idx].is_object())
        {
            item.effect     = queue[queue_idx].value("effect", "");
            item.duration_s = queue[queue_idx].value("duration_s", 0);
            item.color      = queue[queue_idx].value("color", "");
        }

        if(!item.effect.empty())
        {
            items.push_back(item);
        }
    }

    table = new QTableWidget(0, COLUMN_COUNT);
    table->setHorizontalHeaderLabels({ "Effect", "Duration", "Color" });
    table->horizontalHeader()->setSectionResizeMode(COLUMN_EFFECT,   QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(COLUMN_DURATION, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(COLUMN_COLOR,    QHeaderView::ResizeToContents);
    table->verticalHeader()->setSectionsMovable(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setMinimumHeight(PLAYLIST_TABLE_MIN_HEIGHT);

    QPushButton* add_button     = new QPushButton("Add");
    QPushButton* remove_button  = new QPushButton("Remove");
    QPushButton* up_button      = new QPushButton("Up");
    QPushButton* down_button    = new QPushButton("Down");

    connect(add_button,    &QPushButton::clicked, this, [this]{ Add(); });
    connect(remove_button, &QPushButton::clicked, this, [this]{ RemoveSelected(); });
    connect(up_button,     &QPushButton::clicked, this, [this]{ MoveSelected(-1); });
    connect(down_button,   &QPushButton::clicked, this, [this]{ MoveSelected(1); });

    QHBoxLayout* buttons = new QHBoxLayout();
    buttons->addWidget(add_button);
    buttons->addWidget(remove_button);
    buttons->addStretch();
    buttons->addWidget(up_button);
    buttons->addWidget(down_button);

    QLabel* hint = new QLabel("Empty queue plays every effect. Duration 0 uses \"Show each for\"; no color uses the mode color (or a random one).");
    hint->setWordWrap(true);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(table);
    layout->addLayout(buttons);
    layout->addWidget(hint);

    Rebuild(-1);
}

void ViaPlaylistEditor::Rebuild(int select_row)
{
    /*-----------------------------------------------------*\
    | Dropping the rows first hides the old cell widgets at |
    | once; they are only deleted later, and replacing them |
    | in place would leave them drawn under the new ones    |
    \*-----------------------------------------------------*/
    table->setRowCount(0);
    table->setRowCount((int)items.size());

    for(int row = 0; row < (int)items.size(); row++)
    {
        /*-------------------------------------------------*\
        | Effect                                            |
        \*-------------------------------------------------*/
        QComboBox* combo = new QComboBox();

        for(std::size_t name_idx = 0; name_idx < effect_names.size(); name_idx++)
        {
            combo->addItem(QString::fromStdString(effect_names[name_idx]));
        }

        if(combo->findText(QString::fromStdString(items[row].effect)) < 0)
        {
            combo->addItem(QString::fromStdString(items[row].effect));
        }

        combo->setCurrentText(QString::fromStdString(items[row].effect));

        connect(combo, &QComboBox::currentTextChanged, this, [this, row](const QString& text)
        {
            items[row].effect = text.toStdString();
            Notify();
        });

        table->setCellWidget(row, COLUMN_EFFECT, combo);

        /*-------------------------------------------------*\
        | Duration                                          |
        \*-------------------------------------------------*/
        QSpinBox* spin = new QSpinBox();

        spin->setRange(0, PLAYLIST_MAX_DURATION_S);
        spin->setSuffix(" s");
        spin->setSpecialValueText("default");
        spin->setValue(items[row].duration_s);

        connect(spin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this, row](int value)
        {
            items[row].duration_s = value;
            Notify();
        });

        table->setCellWidget(row, COLUMN_DURATION, spin);

        /*-------------------------------------------------*\
        | Color: swatch opens a picker, x clears it         |
        \*-------------------------------------------------*/
        QWidget*        color_cell  = new QWidget();
        QHBoxLayout*    color_box   = new QHBoxLayout(color_cell);
        QPushButton*    swatch      = new QPushButton();
        QToolButton*    clear       = new QToolButton();

        color_box->setContentsMargins(2, 0, 2, 0);

        if(items[row].color.empty())
        {
            swatch->setText("mode");
        }
        else
        {
            swatch->setFixedWidth(PLAYLIST_SWATCH_SIZE * 2);
            swatch->setStyleSheet(QString("background-color: %1;").arg(QString::fromStdString(items[row].color)));
        }

        clear->setText("x");
        clear->setEnabled(!items[row].color.empty());

        connect(swatch, &QPushButton::clicked, this, [this, row]{ PickColor(row); });
        connect(clear,  &QToolButton::clicked, this, [this, row]
        {
            items[row].color.clear();
            Rebuild(row);
            Notify();
        });

        color_box->addWidget(swatch);
        color_box->addWidget(clear);

        table->setCellWidget(row, COLUMN_COLOR, color_cell);
    }

    if(select_row >= 0 && select_row < (int)items.size())
    {
        table->selectRow(select_row);
    }
}

void ViaPlaylistEditor::Add()
{
    int                 row     = table->currentRow();
    via_playlist_item   item    = { effect_names.empty() ? "Solid Color" : effect_names[0], 0, "" };

    /*-----------------------------------------------------*\
    | Insert below the selection, or at the end             |
    \*-----------------------------------------------------*/
    int insert_at = (row >= 0) ? row + 1 : (int)items.size();

    items.insert(items.begin() + insert_at, item);
    Rebuild(insert_at);
    Notify();
}

void ViaPlaylistEditor::RemoveSelected()
{
    int row = table->currentRow();

    if(row < 0 || row >= (int)items.size())
    {
        return;
    }

    items.erase(items.begin() + row);
    Rebuild(row < (int)items.size() ? row : row - 1);
    Notify();
}

void ViaPlaylistEditor::MoveSelected(int offset)
{
    int row     = table->currentRow();
    int target  = row + offset;

    if(row < 0 || target < 0 || target >= (int)items.size())
    {
        return;
    }

    std::swap(items[row], items[target]);
    Rebuild(target);
    Notify();
}

void ViaPlaylistEditor::PickColor(int row)
{
    QColor initial = items[row].color.empty() ? QColor(Qt::white) : QColor(QString::fromStdString(items[row].color));
    QColor picked  = QColorDialog::getColor(initial, this, "Entry color");

    if(!picked.isValid())
    {
        return;
    }

    items[row].color = picked.name(QColor::HexRgb).toStdString();
    Rebuild(row);
    Notify();
}

void ViaPlaylistEditor::Notify()
{
    nlohmann::json queue = nlohmann::json::array();

    for(std::size_t item_idx = 0; item_idx < items.size(); item_idx++)
    {
        queue.push_back(
        {
            { "effect",     items[item_idx].effect },
            { "duration_s", items[item_idx].duration_s },
            { "color",      items[item_idx].color },
        });
    }

    changed(queue);
}
