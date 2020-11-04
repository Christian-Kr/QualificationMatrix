//
// colorchooserdelegate.cpp is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with QualificationMatrix.  If not, see <http://www.gnu.org/licenses/>.
//

#include "colorchooserdelegate.h"

#include <QPainter>
#include <QColor>
#include <QPen>
#include <QComboBox>
#include <QColorDialog>
#include <QDebug>

ColorChooserDelegate::ColorChooserDelegate(QWidget *parent)
    : QStyledItemDelegate(parent)
{}

ColorChooserDelegate::~ColorChooserDelegate()
{}

void ColorChooserDelegate::paint(
    QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index
) const
{
    QStyledItemDelegate::paint(painter, option, index);

    // Draw the background color, that fits the text name.
    if (!(option.state & QStyle::State_Selected)) {
        QColor color = QColor(index.data(Qt::DisplayRole).toString());
        painter->setBrush(color);
        painter->setPen(color);
        painter->drawRect(option.rect);
    }
}

QSize
ColorChooserDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}

QWidget *ColorChooserDelegate::createEditor(
    QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &
) const
{
    // Create a QColorDialog for input.
    QColorDialog *cd = new QColorDialog(parent);
    cd->setModal(true);

    return cd;
}

void ColorChooserDelegate::updateEditorGeometry(
    QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index
) const
{
    QStyledItemDelegate::updateEditorGeometry(editor, option, index);
}

void ColorChooserDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    // Sets the starting value, before starting the edit.
    QColorDialog *cd = dynamic_cast<QColorDialog *>(editor);
    cd->setCurrentColor(index.data().toString());
}

void ColorChooserDelegate::setModelData(
    QWidget *editor, QAbstractItemModel *model, const QModelIndex &index
) const
{
    // Set the new selected color in the data model.
    QColorDialog *cd = dynamic_cast<QColorDialog *>(editor);
    if (cd->result() == QDialog::Accepted) {
        model->setData(index, cd->selectedColor().name(), Qt::EditRole);
    }
}
