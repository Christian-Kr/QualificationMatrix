// qmamspassworddialog.cpp is part of QualificationMatrix
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

#include "qmamspassworddialog.h"
#include "ui_qmamspassworddialog.h"
#include "ams/qmamsmanager.h"

#include <QMessageBox>

QMAMSPasswordDialog::QMAMSPasswordDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QMAMSPasswordDialog)
    , password(std::make_unique<QString>())
{
    ui->setupUi(this);
}

QMAMSPasswordDialog::~QMAMSPasswordDialog()
{
    delete ui;
}

void QMAMSPasswordDialog::acceptPassword()
{
    *password = "";

    if (ui->lePassword->text().length() < 8)
    {
        QMessageBox::information(this, tr("Passwort"), tr("Das Passwort muss mindestens 8 Zeichen haben."));
        return;
    }

    if (ui->lePassword->text() != ui->lePasswordRepeat->text())
    {
        QMessageBox::information(this, tr("Passwort"), tr("Die Passwörter stimmen nicht überein."));
        return;
    }

    *password = ui->lePassword->text();

    close();
}

void QMAMSPasswordDialog::cancel()
{
    close();
}
