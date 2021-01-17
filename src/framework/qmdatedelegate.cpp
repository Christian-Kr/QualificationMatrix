//
// qmdatedelegate.cpp is part of QualificationMatrix
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

#include "qmdatedelegate.h"

#include <QPainter>
#include <QPen>
#include <QDateEdit>
#include <QDate>

DateDelegate::DateDelegate(QWidget *parent)
    : QStyledItemDelegate(parent)
{}

void DateDelegate::paint(
    QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);
}

QSize DateDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}

QWidget *DateDelegate::createEditor(
    QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &) const
{
    auto de = new QDateEdit(parent);
    de->setGeometry(option.rect);
    de->setCalendarPopup(true);
    de->setDisplayFormat("yyyy-MM-dd");
    return de;
}

void DateDelegate::updateEditorGeometry(
    QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
    editor->setMinimumWidth(100);
    editor->setGeometry(option.rect);
}

void DateDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto de = dynamic_cast<QDateEdit *>(editor);
    de->setDate(QDate::fromString(index.data().toString()));
}

void DateDelegate::setModelData(
    QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    auto de = dynamic_cast<QDateEdit *>(editor);
    model->setData(index, de->date().toString("yyyy-MM-dd"), Qt::EditRole);
}
