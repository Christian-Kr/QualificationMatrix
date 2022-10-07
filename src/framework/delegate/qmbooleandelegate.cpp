// qmcheckboxdelegate.cpp is part of QualificationMatrix
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

#include "qmbooleandelegate.h"

#include <QPainter>
#include <QComboBox>
#include <QEvent>
#include <QApplication>

#include <QDebug>

QMBooleanDelegate::QMBooleanDelegate(QObject *parent)
    : QStyledItemDelegate(parent),
    editable(true)
{}

void QMBooleanDelegate::paint(
    QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem itemOption(option);
    bool value = index.data().toBool();

    initStyleOption(&itemOption, index);

    if (value)
    {
        itemOption.text = tr("Ja");
    }
    else
    {
        itemOption.text = tr("Nein");
    }

    QApplication::style()->drawControl(
            QStyle::CE_ItemViewItem, &itemOption, painter, nullptr);
}

bool QMBooleanDelegate::editorEvent(
        QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
        const QModelIndex &index)
{
    if (!editable)
    {
        event->ignore();
        return false;
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

QWidget *QMBooleanDelegate::createEditor(
        QWidget *aParent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!editable)
    {
        return nullptr;
    }

    // Create a combo box with "Yes" and "No". This will be the standard combo box. There cannot be
    // anything else for handling boolean values in this application.

    auto combo = new QComboBox(aParent);

    combo->addItems({tr("Ja"), tr("Nein")});
    combo->setEditable(false);

    return combo;
}

void QMBooleanDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    // Get the combo box selected value and set it to the data.

    auto combo = dynamic_cast<QComboBox *>(editor);
    if (combo == nullptr)
    {
        qWarning() << "cannot get combo box editor for qm data";
        return;
    }

    if (index.data().toBool())
    {
        combo->setCurrentIndex(0);
    }
    else
    {
        combo->setCurrentIndex(1);
    }
}

void QMBooleanDelegate::setModelData(
        QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    /// Data in the data must be set with the right boolean value.

    if (!index.isValid())
    {
        return;
    }

    auto combo = dynamic_cast<QComboBox *>(editor);
    if (combo == nullptr)
    {
        qWarning() << "cannot get combo box editor for saving data data";
        return;
    }

    QString value = combo->currentText();
    bool bValue = false;
    if (value == tr("Ja"))
    {
        bValue = true;
    }

    model->setData(index, bValue);
}

