// qmtraindataconflictdialog.h is part of QualificationMatrix
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

#ifndef QMTRAINDATACONFLICTDIALOG_H
#define QMTRAINDATACONFLICTDIALOG_H

#include "framework/dialog/qmdialog.h"

#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class QMTrainDataConflictDialog;
}
QT_END_NAMESPACE

/// Show train data entries with conflicts and give feedback options to handel.
/// \author Christian Kr, Copyright 2022
class QMTrainDataConflictDialog : public QMDialog
{
    Q_OBJECT

public:
    /// Constructor
    /// \param parent The parent object of the widget. This is important for a modal dialog.
    explicit QMTrainDataConflictDialog(QWidget *parent = nullptr);

    /// Destructor
    ~QMTrainDataConflictDialog() override;

    /// Override from QMDialog.
    void saveSettings() override;

    /// Override from QDialog.
    void loadSettings() override;

private:
    Ui::QMTrainDataConflictDialog *ui;
};

#endif // QMTRAINDATACONFLICTDIALOG_H
