// qmselectfromlistdialog.h is part of QualificationMatrix
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

#ifndef QMSELECTFROMLISTDIALOG_H
#define QMSELECTFROMLISTDIALOG_H

#include "qmdialog.h"

namespace Ui
{
    class QMSelectFromListDialog;
}

// Selection mode for the dialog. If 'MULTIPLE' has been choosen, more than one selection ist allowed.
enum class SelectionMode
{
    SINGLE = 0,
    MULTPLE = 1
};

/// Dialog for selection of elements from a list.
/// \author Christian Kr, Copyright 2022
class QMSelectFromListDialog : public QMDialog
{
    Q_OBJECT

public:
    /// Constructor
    /// \param parent
    /// \param info
    /// \param mode
    explicit QMSelectFromListDialog(QWidget *parent = nullptr, const QString info = QString(), 
            const SelectionMode mode = SelectionMode::SINGLE);

    /// Destructor
    ~QMSelectFromListDialog() override;

    /// Set the list of elements the user need to choose from.
    /// \param list The list of string elements. The elements will replace all existing elements.
    void setSelectionList(const QStringList list);

    /// Get the number of selected elements.
    /// \return A list with row numbers. The first row number of the selection list is 0.
    QList<int> getSelected() const;

private:
    Ui::QMSelectFromListDialog *ui;

    QStringList *selectionList;
    SelectionMode selMode;
};

#endif // QMSELECTFROMLISTDIALOG_H
