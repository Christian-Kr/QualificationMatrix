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

// Options how the conflict should be solved.
enum class SolveAction
{
    IGNORE = 1 << 0,
    DELETE = 1 << 1,
    OVERWRITE = 1 << 2
};

class QMTrainDataConflictModel;

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

    /// Set the ids of the training data that have a conflict.
    /// \param ids The ids as a list of interger values.
    void setTrainingData(const QList<int> &ids);

private:
    Ui::QMTrainDataConflictDialog *ui;

    QMTrainDataConflictModel *m_trainDataConflictModel;
};

#endif // QMTRAINDATACONFLICTDIALOG_H
