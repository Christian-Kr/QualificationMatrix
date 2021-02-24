//
// qmextendedselectiondialog.h is part of QualificationMatrix
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

#ifndef QMEXTENDEDSELECTIONDIALOG_H
#define QMEXTENDEDSELECTIONDIALOG_H

#include "framework/qmdialog.h"

/// Dialog for extended selection in models.
/// \author Christian Kr, Copyright 2020
class QMExtendedSelectionDialog: public QMDialog
{
Q_OBJECT

public:
    /// Constructor
    /// \param parent
    explicit QMExtendedSelectionDialog(QWidget *parent = nullptr);

    /// Will be called from closeEvent.
    void saveSettings() override;

    /// Will be called from constructor.
    void loadSettings() override;
};

#endif // QMEXTENDEDSELECTIONDIALOG_H
