// qmdatedelegate.h is part of QualificationMatrix
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

#ifndef QMDATEDELEGATE_H
#define QMDATEDELEGATE_H

#include <QStyledItemDelegate>

/// Edit date in table view.
/// \author Christian Kr, Copyright 2020
class DateDelegate: public QStyledItemDelegate
{
Q_OBJECT

public:
    /// Constructor
    /// \param parent
    explicit DateDelegate(QWidget *parent = nullptr);

    // Destructor
    ~DateDelegate() override = default;

    /// Override from QStyleItemDelegate.
    /// \param option
    /// \param index
    /// \param painter
    void paint(
        QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;

    /// Override from QStyleItemDelegate.
    /// \param option
    /// \param index
    /// \return
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    /// Override from QStyleItemDelegate.
    /// \param parent
    /// \param option
    /// \param index
    /// \return
    QWidget *createEditor(
        QWidget *parent, const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;

    /// Override from QStyleItemDelegate.
    /// \param editor
    /// \param index
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    /// Override from QStyleItemDelegate.
    /// \param index
    /// \param model
    /// \param editor
    void setModelData(
        QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

    /// Override from QStyleItemDelegate.
    /// \param editor
    /// \param index
    /// \param option
    void updateEditorGeometry(
        QWidget *editor, const QStyleOptionViewItem &option,
        const QModelIndex &index) const override;
};

#endif // QMDATEDELEGATE_H
