// qmplaintexteditdelegate.cpp is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify it under the terms of the GNU General
// Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option)
// any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with QualificationMatrix.
// If not, see <http://www.gnu.org/licenses/>.

#include "qmplaintexteditdelegate.h"

#include <QPainter>
#include <QPen>
#include <QPlainTextEdit>

QMPlainTextEditDelegate::QMPlainTextEditDelegate(QWidget *parent)
    : QStyledItemDelegate(parent)
{}

void QMPlainTextEditDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);
}

QSize QMPlainTextEditDelegate::sizeHint(const QStyleOptionViewItem &option,
    const QModelIndex &index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}

QWidget *QMPlainTextEditDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
    const QModelIndex &) const
{
    auto te = new QPlainTextEdit(parent);
    te->setWordWrapMode(QTextOption::NoWrap);

    return te;
}

void QMPlainTextEditDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &) const
{
    editor->setMinimumWidth(100);
    editor->setMinimumHeight(100);
    editor->setGeometry(option.rect);
}

void QMPlainTextEditDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto te = dynamic_cast<QPlainTextEdit *>(editor);
    te->setPlainText(index.data().toString());
}

void QMPlainTextEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
    const QModelIndex &index) const
{
    auto te = dynamic_cast<QPlainTextEdit *>(editor);
    model->setData(index, te->toPlainText(), Qt::EditRole);
}
