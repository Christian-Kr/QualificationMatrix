//
// qmnewcertificatedialog.h is part of QualificationMatrix
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

#ifndef QMNEWCERTIFICATEDIALOG_H
#define QMNEWCERTIFICATEDIALOG_H

#include <QDialog>
#include <memory>

// Forward declaration.
class QSqlTableModel;

QT_BEGIN_NAMESPACE
namespace Ui
{
class QMNewCertificateDialog;
}
QT_END_NAMESPACE

/// Take all information to add a new certificate.
/// \author Christian Kr, Copyright 2020
class QMNewCertificateDialog: public QDialog
{
Q_OBJECT

public:
    /// Constructor
    /// \param mode The ode of the dialog to choose.
    explicit QMNewCertificateDialog(QWidget *parent = nullptr);

    /// Destructor
    ~QMNewCertificateDialog() override;

    /// Save settings from the dialog.
    void saveSettings();

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

    /// Update database model data.
    void updateData();

protected:
    /// Override from QDialog.
    void closeEvent(QCloseEvent *event) override;

    /// Override from QDialog.
    void keyPressEvent(QKeyEvent *event) override;

public slots:
    /// Open certificate path file.
    void openCertificatePath();

    /// Override from QDialog.
    void accept() override;

private:
    Ui::QMNewCertificateDialog *ui;

    QString employeeGroup;
    QString employee;
    QString train;
    QString trainDate;

    std::shared_ptr<QSqlTableModel> certificateModel;
    std::shared_ptr<QSqlTableModel> trainDataCertificateModel;
};

#endif // QMNEWCERTIFICATEDIALOG_H
