//
// qmprogresslogdialog.h is part of QualificationMatrix
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

#ifndef QMPROGRESSLOGDIALOG_H
#define QMPROGRESSLOGDIALOG_H

#include <QDialog>

namespace Ui
{
class QMProgressLogDialog;
}

/// Showing a progress log of what is happening.
/// \author Christian Kr, Copyright (c) 2021
class QMProgressLogDialog: public QDialog
{
Q_OBJECT

public:
    /// Constructor
    /// \param parent
    explicit QMProgressLogDialog(QWidget *parent = nullptr);

    /// Destructor
    ~QMProgressLogDialog() override;

private:
    Ui::QMProgressLogDialog *ui;
};

#endif // QMPROGRESSLOGDIALOG_H
