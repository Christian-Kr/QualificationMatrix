// qmamslogindialog.h is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify it under the terms of the GNU General
// Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option)
// any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with QualificationMatrix. If not,
// see <http://www.gnu.org/licenses/>.

#ifndef QMAMSLOGINDIALOG_H
#define QMAMSLOGINDIALOG_H

#include <QDialog>

namespace Ui
{
    class QMAMSLoginDialog;
}

/// Login widget for account management system.
/// \author Christian Kr, Copyright 2020
class QMAMSLoginDialog: public QDialog
{
    Q_OBJECT

public:
    /// Constructor
    /// \param parent The parent object for the qt system.
    explicit QMAMSLoginDialog(QWidget *parent = nullptr);

    /// Destructor
    ~QMAMSLoginDialog() override;

    /// Set the user name to log in. If user is set, it is not changeable by user. To reset, recreate the widget.
    /// \param username The username to log in
    void setUsername(QString name);

public slots:
    /// Login width credentials clicked.
    void login();

    /// Cancel login process.
    void cancel();

private:
    Ui::QMAMSLoginDialog *ui;
};

#endif // QMAMSLOGINDIALOG_H
