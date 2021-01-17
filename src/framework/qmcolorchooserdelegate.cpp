//
// qmcolorchooserdelegate.cpp is part of QualificationMatrix
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

#include "qmcolorchooserdelegate.h"

#include <QPainter>
#include <QColor>
#include <QPen>
#include <QComboBox>
#include <QColorDialog>
#include <QDebug>

QMColorChooserDelegate::QMColorChooserDelegate(QWidget *parent)
    : QStyledItemDelegate(parent) {}

void QMColorChooserDelegate::paint(
    QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);

    // Draw the background color, that fits the text name.
    if (!(option.state & QStyle::State_Selected))
    {
        QColor color = QColor(index.data(Qt::DisplayRole).toString());
        painter->setBrush(color);
        painter->setPen(color);
        painter->drawRect(option.rect);
    }
}

QSize QMColorChooserDelegate::sizeHint(
    const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}

QWidget *QMColorChooserDelegate::createEditor(
    QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const
{
    // Create a QColorDialog for input.
    auto cd = new QColorDialog(parent);
    cd->setModal(true);

    return cd;
}

void QMColorChooserDelegate::updateEditorGeometry(
    QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::updateEditorGeometry(editor, option, index);
}

void QMColorChooserDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    // Sets the starting value, before starting the edit.
    auto cd = dynamic_cast<QColorDialog *>(editor);
    cd->setCurrentColor(index.data().toString());
}

void QMColorChooserDelegate::setModelData(
    QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    // Set the new selected color in the data model.
    auto cd = dynamic_cast<QColorDialog *>(editor);
    if (cd->result() == QDialog::Accepted)
    {
        model->setData(index, cd->selectedColor().name(), Qt::EditRole);
    }
}
