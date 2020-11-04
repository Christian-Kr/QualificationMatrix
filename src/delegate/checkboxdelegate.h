//
// checkboxdelegate.h is part of QualificationMatrix
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

#ifndef CHECKBOXDELEGATE_H
#define CHECKBOXDELEGATE_H

#include <QItemDelegate>

/**
 * @brief Gives the ability to use a checkbox for boolean entries.
 * @author Christian Kr, Copyright (c) 2020
 */
class CheckBoxDelegate: public QItemDelegate
{
Q_OBJECT

public:
    /**
     * @brief override from QItemDelegate
     */
    virtual void paint(
        QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index
    ) const override;

    /**
     * @brief Override from QItemDelegate
     */
    virtual bool editorEvent(
        QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
        const QModelIndex &index
    ) override;
};

#endif // CHECKBOXDELEGATE_H
