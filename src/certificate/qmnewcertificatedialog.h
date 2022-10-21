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

class QFile;
class QSqlDatabase;
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
    explicit QMNewCertificateDialog(const QSqlDatabase &db, QWidget *parent = nullptr);

    /// Destructor
    ~QMNewCertificateDialog() override;

    /// Override from QMDialog.
    void saveSettings() override;

    /// Override from QMDialog
    void loadSettings() override;

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

    /// Remove an employee by selected id.
    [[maybe_unused]] void removeEmployees();

    /// The extended selection of employees has been finished.
    /// \param result The result id the dialog has been closed with.
    [[maybe_unused]] void extSelEmployeeFinished(int result);

    /// Add a new certificate.
    /// \returns true if successfull, else false
    [[maybe_unused]] bool addCertificate();

private:
    /// Save the given file external: Copy to file system structure.
    /// \param file File to save external.
    /// \return New file name if success, else empty string.
    QString saveFileExternal(QFile &file);

    /// Check the data in the ui elements to be valid.
    /// \param errorMessage the error message if the validation failed and the return value is false
    /// \returns true if input fields have valid data, else false
    bool validateInputData(QString &errorMessage);

    /// Add the certificate to training data entries or create them.
    /// \param errorMessage the error message if adding the certificate failed and the return value is false
    /// \returns false is adding certificate to training data failed, else true
    [[maybe_unused]] bool addCertificateTrainingDataEntries(QString &errorMessage);

    Ui::QMNewCertificateDialog *m_ui;

    QString m_employeeGroup;
    QString m_employee;
    QString m_train;
    QString m_trainDate;
    QString m_certPath;

    std::unique_ptr<QSqlTableModel> m_certificateModel;
    std::unique_ptr<QMExtendedSelectionDialog> m_extSelEmployeeDialog;
    std::unique_ptr<QMEmployeeDateModel> m_employeeDateModel;
    std::unique_ptr<QSqlTableModel> m_trainViewModel;
    std::unique_ptr<QSqlTableModel> m_employeeViewModel;
    std::unique_ptr<QSqlTableModel> m_employeeGroupViewModel;
};

#endif // QMNEWCERTIFICATEDIALOG_H
