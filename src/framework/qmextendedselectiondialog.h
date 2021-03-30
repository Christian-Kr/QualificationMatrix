// qmextendedselectiondialog.h is part of QualificationMatrix
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

#ifndef QMEXTENDEDSELECTIONDIALOG_H
#define QMEXTENDEDSELECTIONDIALOG_H

#include "framework/qmdialog.h"

#include <QModelIndexList>

namespace Ui
{
    class QMExtendedSelectionDialog;
}

class QAbstractItemModel;
class QSortFilterProxyModel;

/// Dialog for extended selection in models.
/// \author Christian Kr, Copyright 2020
class QMExtendedSelectionDialog: public QMDialog
{
Q_OBJECT

public:
    /// Constructor
    /// \param parent
    /// \param tableModel The data model to show for selection.
    /// \param column The column of the model to show.
    explicit QMExtendedSelectionDialog(QWidget *parent = nullptr,
        QAbstractItemModel *tableModel = nullptr, int column = 0);

    /// Get the number of rows selected.
    /// \return The selected index list of the filter model.
    QModelIndexList getFilterSelected() const;

    /// Get the filter model. This object will be destroyed on object deletion.
    /// \return The filter model that is created temporary inside this object.
    QSortFilterProxyModel* getFilterModel() { return filterModel; }

    /// Should the selection be reversed.
    /// \return True if reversed, else false.
    bool isReverse() const;

    /// Get the regular expression text of the selection.
    /// \return The regular expression text.
    QString getRegExpText() const;

public slots:
    /// Update filter.
    void updateFilter();

private:
    Ui::QMExtendedSelectionDialog *ui;

    QSortFilterProxyModel *filterModel;
};

#endif // QMEXTENDEDSELECTIONDIALOG_H
