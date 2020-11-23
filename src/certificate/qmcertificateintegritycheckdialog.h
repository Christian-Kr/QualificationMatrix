//
// qmcertificateintegritycheckdialog.h is part of QualificationMatrix
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

#ifndef QMCERTIFICATEINTEGRITYCHECKDIALOG_H
#define QMCERTIFICATEINTEGRITYCHECKDIALOG_H

#include <QDialog>
#include <memory>

// Forward declaration.
class QSqlTableModel;

QT_BEGIN_NAMESPACE
namespace Ui
{
class QMCertificateIntegrityCheckDialog;
}
QT_END_NAMESPACE

/// Show dialog for integrity check of certificate database.
/// \author Christian Kr, Copyright 2020
class QMCertificateIntegrityCheckDialog: public QDialog
{
Q_OBJECT

public:
    /// Constructor
    /// \param parent The parent object of the widgt. This is important for a modal dialog.
    explicit QMCertificateIntegrityCheckDialog(QWidget *parent = nullptr);

    /// Destructor
    ~QMCertificateIntegrityCheckDialog() override;

    /// Save settings from the dialog.
    void saveSettings();

    /// Accept dialog: Run check!
    void accept() override;

protected:
    /// Override from QDialog.
    void closeEvent(QCloseEvent *event) override;

    /// Override from QDialog.
    void keyPressEvent(QKeyEvent *event) override;

public slots:
    /// Update the models, cause they might have changed.
    void updateData();

private:
    Ui::QMCertificateIntegrityCheckDialog *ui;

    std::shared_ptr<QSqlTableModel> certificateModel;
};

#endif // QMCERTIFICATEINTEGRITYCHECKDIALOG_H
