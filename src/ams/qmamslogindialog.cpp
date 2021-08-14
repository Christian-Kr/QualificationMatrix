// qmamssettingsdialog.cpp is part of QualificationMatrix
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

#include "qmamslogindialog.h"
#include "ui_qmamslogindialog.h"
#include "ams/qmamsmanager.h"

#include <QMessageBox>
#include <QDebug>

QMAMSLoginDialog::QMAMSLoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QMAMSLoginDialog)
{
    ui->setupUi(this);
}

QMAMSLoginDialog::~QMAMSLoginDialog()
{
    delete ui;
}

void QMAMSLoginDialog::setUsername(QString name)
{
    ui->leUsername->setText(name);
    ui->leUsername->setReadOnly(true);
}

void QMAMSLoginDialog::login()
{
    QString username = ui->leUsername->text();
    QString password = ui->lePassword->text();

    auto am = QMAMSManager::getInstance();

    if (am->getLoginUserName()->compare(username) == 0)
    {
        QMessageBox::information(this, tr("Anmelden"),
                tr("Benutzer ist bereits angemeldet."));
        return;
    }

    if (am->getLoginState() == LoginState::LOGGED_IN)
    {
        auto res = QMessageBox::information(this, tr("Anmelden"),
                tr("Ein Benutzer ist bereits angemeldet und wird "
                "mit dem login abgemeldet. Fortfahren?"),
                QMessageBox::Yes | QMessageBox::No);

        if (res == QMessageBox::No)
        {
            return;
        }
    }

    am->loginUser(username, password);

    // Test afterwarts, whether everything goes right.
    if (am->getLoginUserName()->compare(username) != 0)
    {
        QMessageBox::information(this, tr("Anmelden"),
                tr("Bei der Anmeldung is etwas schief gelaufen."));
        return;
    }

    close();
}

void QMAMSLoginDialog::cancel()
{
    close();
}
