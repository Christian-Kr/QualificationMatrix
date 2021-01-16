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

#ifndef QMBOOLEANDELEGATE_H
#define QMBOOLEANDELEGATE_H

#include <QStyledItemDelegate>

/// Gives the ability to use a checkbox for boolean entries.
/// \author Christian Kr, Copyright 2020
class QMBooleanDelegate: public QStyledItemDelegate
{
Q_OBJECT

public:
    /// Constructor
    /// \param
    QMBooleanDelegate(QObject *parent = nullptr);

    /// Override from QStyledItemDelegate.
    /// \param painter
    /// \param option
    /// \param index
    void paint(
            QPainter *painter, const QStyleOptionViewItem &option,
            const QModelIndex &index) const override;

    /// Override from QStyledItemDelegate
    /// \param aParent
    /// \param option
    /// \param index
    /// \return
    QWidget *createEditor(
            QWidget *aParent, const QStyleOptionViewItem &option,
            const QModelIndex &index) const override;

    /// Override from QStyledItemDelegate
    /// \param editor
    /// \param index
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;

    /// Override from QStyledItemDelegate
    /// \param editor
    /// \param model
    /// \param index
    void setModelData(
            QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

    /// Set editable state for delegate.
    /// \param value Yes if editable, else false.
    void setEditable(bool value) { editable = value; }

    /// Override from QStyledItemDelegate
    /// \param event
    /// \param model
    /// \param option
    /// \param index
    bool editorEvent(
            QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
            const QModelIndex &index) override;

private:
    bool editable;
};

#endif // QMBOOLEANDELEGATE_H
