// qmamspassworddialog.h is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along
// with QualificationMatrix. If not, see <http://www.gnu.org/licenses/>.

#ifndef QMAMSPASSWORDDIALOG_H
#define QMAMSPASSWORDDIALOG_H

#include <QDialog>

namespace Ui
{
    class QMAMSPasswordDialog;
}

/// Password dialog for account management system.
/// \author Christian Kr, Copyright 2020
class QMAMSPasswordDialog: public QDialog
{
    Q_OBJECT

public:
    /// Constructor
    /// \param parent The parent object for the qt system.
    explicit QMAMSPasswordDialog(QWidget *parent = nullptr);

    /// Destructor
    ~QMAMSPasswordDialog() override;

    /// Get the password. If something has been wrong, the password variable
    /// is empty.
    /// \return Empty string if something went wrong, else the password.
    QString getPasswort() const { return *password; }

public slots:
    /// Accept password.
    void acceptPassword();

    /// Cancel login process.
    void cancel();

private:
    Ui::QMAMSPasswordDialog *ui;

    std::unique_ptr<QString> password;
};

#endif // QMAMSPASSWORDDIALOG_H
