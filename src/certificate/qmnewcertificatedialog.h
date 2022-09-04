// qmnewcertificatedialog.h is part of QualificationMatrix
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

#ifndef QMNEWCERTIFICATEDIALOG_H
#define QMNEWCERTIFICATEDIALOG_H

#include "framework/dialog/qmdialog.h"
#include <memory>

class QMSqlTableModel;
class QSqlRelationalTableModel;
class QSqlTableModel;
class QMEmployeeDateModel;
class QMExtendedSelectionDialog;

QT_BEGIN_NAMESPACE
namespace Ui
{
class QMNewCertificateDialog;
}
QT_END_NAMESPACE

/// Take all information to add a new certificate.
/// \author Christian Kr, Copyright 2020
class QMNewCertificateDialog: public QMDialog
{
Q_OBJECT

public:
    /// Constructor
    /// \param mode The ode of the dialog to choose.
    explicit QMNewCertificateDialog(QWidget *parent = nullptr);

    /// Destructor
    ~QMNewCertificateDialog() override;

    /// Override from QMDialog.
    void saveSettings() override;

    /// Get employee
    /// \return Name of the employee
    QString getEmployee() { return employee; }

    /// Get employee group
    /// \return Name of the employee group
    QString getEmployeeGroup() { return employeeGroup; }

    /// Get training
    /// \return Name of the training
    QString getTrain() { return train; }

    /// Get train date
    /// \return Name of the training date
    QString getTrainDate() { return trainDate; }

    /// Get cert path
    /// \return Path of the certification path
    QString getCertPath() { return certPath; }

    /// Update database model data.
    void updateData();

public slots:
    /// Open certificate path file.
    [[maybe_unused]] void openCertificatePath();

    /// Override from QDialog.
    void accept() override;

    /// Make controls editable if corresponding radio button is selected.
    [[maybe_unused]] void switchEmployeeSelection();

    /// Called when the check box of append to train data changed.
    /// \param state The current state of the check box.
    [[maybe_unused]] void appendToTrainDataChanged(int state);

    /// Add employees to define the training date.
    [[maybe_unused]] void addEmployees();

    /// The extended selection of employees has been finished.
    /// \param result The result id the dialog has been closed with.
    [[maybe_unused]] void extSelEmployeeFinished(int result);

private:
    /// Check the data in the ui elements to be valid.
    /// \return True if ok, else false.
    bool checkInputData();

    Ui::QMNewCertificateDialog *ui;

    QString employeeGroup;
    QString employee;
    QString train;
    QString trainDate;
    QString certPath;

    std::unique_ptr<QMExtendedSelectionDialog> m_extSelEmployeeDialog;
    std::unique_ptr<QMEmployeeDateModel> m_employeeDateModel;
    std::unique_ptr<QSqlTableModel> trainViewModel;
    std::unique_ptr<QSqlTableModel> employeeViewModel;
    std::unique_ptr<QSqlTableModel> employeeGroupViewModel;
};

#endif // QMNEWCERTIFICATEDIALOG_H
