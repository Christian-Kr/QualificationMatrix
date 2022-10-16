// qmaddmultipletraindatadialog.h is part of QualificationMatrix
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

#ifndef QMADDMULTIPLETRAINDATADIALOG_H
#define QMADDMULTIPLETRAINDATADIALOG_H

#include "framework/dialog/qmdialog.h"

#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui
{
class QMAddMultipleTrainDataDialog;
}
QT_END_NAMESPACE

class QSqlTableModel;
class QMSqlTableModel;
class QMTrainingDataModel;

/// Add multiple entries to train data, with default values.
/// \author Christian Kr, Copyright 2021
class QMAddMultipleTrainDataDialog: public QMDialog
{
Q_OBJECT

public:
    /// Constructor
    /// \param parent The main parent object.
    explicit QMAddMultipleTrainDataDialog(QMTrainingDataModel *trainDataModel, QWidget *parent = nullptr);

    /// Destructor
    ~QMAddMultipleTrainDataDialog() override;

    /// Override from QMDialog
    void loadSettings() override {};

    // Override from QMDialog
    void saveSettings() override {};

public slots:
    /// Start the process of importing.
    void accept() override;

    /// Update data data.
    void updateData();

    /// Add simgle employee to list.
    void addEmployee();

    /// Add all employees from selected group.
    void addEmployeeFromGroup();

    /// Reset the list of employees.
    void resetList();

    /// Remove an employee from the list.
    void removeEmployee();

private:
    /// Update employee list.
    void updateEmployeeListWidget();

    Ui::QMAddMultipleTrainDataDialog *ui;

    std::unique_ptr<QSqlTableModel> trainViewModel;
    std::unique_ptr<QSqlTableModel> employeeViewModel;
    std::unique_ptr<QSqlTableModel> trainDataStateViewModel;
    std::unique_ptr<QSqlTableModel> shiftViewModel;
    std::unique_ptr<QSqlTableModel> trainGroupViewModel;
    QMTrainingDataModel *trainDataModel;

    QList<int> *employeeId;
    QList<QString> *employeeName;
};

#endif // QMADDMULTIPLETRAINDATADIALOG_H
