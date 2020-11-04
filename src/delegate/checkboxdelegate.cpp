//
// checkboxdelegate.cpp is part of QualificationMatrix
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

#include "checkboxdelegate.h"

#include <QPainter>
#include <QComboBox>
#include <QCheckBox>
#include <QEvent>

void CheckBoxDelegate::paint(
    QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index
) const
{
    drawCheck(painter, option, option.rect, index.data().toBool() ? Qt::Checked : Qt::Unchecked);
}

bool CheckBoxDelegate::editorEvent(
    QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
    const QModelIndex &index
)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        model->setData(index, !model->data(index).toBool());
        event->accept();
    }

    return QItemDelegate::editorEvent(event, model, option, index);
}
