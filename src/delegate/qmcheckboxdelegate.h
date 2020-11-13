//
// qmcheckboxdelegate.h is part of QualificationMatrix
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

#ifndef QMCHECKBOXDELEGATE_H
#define QMCHECKBOXDELEGATE_H

#include <QItemDelegate>

/// Gives the ability to use a checkbox for boolean entries.
/// \author Christian Kr, Copyright 2020
class QMCheckBoxDelegate: public QItemDelegate
{
Q_OBJECT

public:
    /// Override from QItemDelegate.
    /// \param painter
    /// \param option
    /// \param index
    void paint(
        QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;

    /// Override from QItemDelegate.
    /// \param index
    /// \param event
    /// \param model
    /// \param option
    bool editorEvent(
        QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
        const QModelIndex &index) override;
};

#endif // QMCHECKBOXDELEGATE_H
