//
// qmdialog.h is part of QualificationMatrix
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

#ifndef QMDIALOG_H
#define QMDIALOG_H

#include <QDialog>

/// General dialog class which implements functionalities, that should be available in all other
/// dialogs.
/// \author Christian Kr, Copyright 2020
class QMDialog: public QDialog
{
Q_OBJECT

public:
    /// Constructor
    /// \param parent
    explicit QMDialog(QWidget *parent = nullptr);

    /// Will be called from closeEvent.
    virtual void saveSettings() {};

    /// Will be called from constructor.
    virtual void loadSettings() {};

    /// Override from QDialog. Do several actions on close.
    /// \param event
    void closeEvent(QCloseEvent *event) override;

protected:
    /// Override from QDialog. Permit close dialog on pressing enter.
    void keyPressEvent(QKeyEvent *event) override;
};

#endif // QMDIALOG_H
