//
// qualimatrixdelegate.cpp is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify it under the terms of
// the GNU General Public License as published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
// the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with QualificationMatrix.
// If not, see <http://www.gnu.org/licenses/>.
//

#include "qmqualimatrixdelegate.h"
#include "settings/qmapplicationsettings.h"

#include <QPainter>
#include <QColor>
#include <QPen>
#include <QComboBox>

QMQualiMatrixDelegate::QMQualiMatrixDelegate(QWidget *parent)
    : QStyledItemDelegate(parent),
    gridColor(QColor("#ffffff")),
    selectionColor(QColor("#ffffff"))
{
    updateColors();
}

void QMQualiMatrixDelegate::updateColors()
{
    auto &settings = QMApplicationSettings::getInstance();
    selectionColor = QColor(settings.read("QualiMatrix/SelectionColor", "#ffffff").toString());
    gridColor = QColor(settings.read("QualiMatrix/GridColor", "#ffffff").toString());
}

void QMQualiMatrixDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    auto value = index.data().toString();

    if (option.state & QStyle::State_Selected)
    {
        painter->setBrush(selectionColor);
    }
    else
    {
        if (value == "Pflicht") {
            painter->setBrush(QColor("#ffcccc"));
        }
        else if (value == "Angebot") {
            painter->setBrush(QColor("#cce6ff"));
        }
        else if (value == "Sonstiges") {
            painter->setBrush(QColor("#ffe6cc"));
        }
        else {
            painter->setBrush(QColor("#FFFFFF"));
        }
    }

    painter->setPen(gridColor);
    painter->drawRect(option.rect);

    if (!value.isEmpty())
    {
        value = value.at(0);
        painter->drawRect(option.rect.x(), option.rect.y(), option.rect.width(),
            option.rect.height());
        painter->setPen(Qt::black);
        painter->drawText(option.rect, Qt::AlignCenter, value);
    }
}

QSize QMQualiMatrixDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    return QSize(30, 30);
}

QWidget *QMQualiMatrixDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
    const QModelIndex &) const
{
    auto cb = new QComboBox(parent);

    cb->addItem("Entfernen");
    cb->addItem("Pflicht");
    cb->addItem("Angebot");
    cb->addItem("Sonstiges");
    cb->setGeometry(option.rect);

    return cb;
}

void QMQualiMatrixDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &) const
{
    editor->setMinimumWidth(100);
    editor->setGeometry(option.rect);
}

void QMQualiMatrixDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto cb = dynamic_cast<QComboBox *>(editor);

    cb->setCurrentText(index.data(Qt::DisplayRole).toString());
}

void QMQualiMatrixDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
    const QModelIndex &index) const
{
    auto cb = dynamic_cast<QComboBox *>(editor);

    if (cb->currentIndex() == 0)
    {
        model->setData(index, -1, Qt::EditRole);
    }
    else
    {
        model->setData(index, cb->currentIndex(), Qt::EditRole);
    }
}
