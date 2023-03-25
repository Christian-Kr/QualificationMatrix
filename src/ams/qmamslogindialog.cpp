// qmamssettingsdialog.cpp is part of QualificationMatrix
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

#include "qmamslogindialog.h"
#include "ui_qmamslogindialog.h"
#include "ams/qmamsmanager.h"
#include "settings/qmapplicationsettings.h"

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
    ui->lePassword->setFocus();
}

void QMAMSLoginDialog::login()
{
    QString username = ui->leUsername->text();
    QString password = ui->lePassword->text();

    auto am = QMAMSManager::getInstance();

    if (am->getLoginUserName().compare(username) == 0)
    {
        QMessageBox::information(this, tr("Anmelden"), tr("Benutzer ist bereits angemeldet."));
        return;
    }

    if (am->getLoginState() == LoginState::LOGGED_IN)
    {
        auto res = QMessageBox::information(this, tr("Anmelden"),
                tr("Ein Benutzer ist bereits angemeldet und wird mit dem login abgemeldet. Fortfahren?"),
                QMessageBox::Yes | QMessageBox::No);

        if (res == QMessageBox::No)
        {
            return;
        }
    }

    QString result = "";
    auto res = am->loginUser(username, password);

    if (res == LoginResult::SUCCESSFUL)
    {
        if (username.compare("administrator") != 0)
        {
            // Save the login if wanted.
            auto &settings = QMApplicationSettings::getInstance();
            auto saveLastLogin = settings.read("AMS/SaveLastLogin", false).toBool();

            if (saveLastLogin) {
                settings.write("AMS/LastLoginName", username);
            }
        }

        close();
        return;
    }

    switch (res)
    {
        case LoginResult::EMPTY_PASSWORD:
            result = tr("Ein leeres Passwort in der Datenbank ist nicht "
                    "erlaubt.");
            break;
        case LoginResult::WRONG_PASSWORD:
            result = tr("Falsches Passwort.");
            break;
        case LoginResult::FAILED_LOGIN_COUNT:
            result = tr("Zu viele Fehlversuche für den Nutzer.");
            break;
        case LoginResult::USER_NOT_EXIST:
            result = tr("Der Nutzer existiert nicht.");
            break;
        case LoginResult::USER_NOT_ACTIVE:
            result = tr("Nutzer ist deaktiviert.");
            break;
        case LoginResult::FAILED_UNKNOWN:
            result = tr("Unbekannter Fehler.");
            break;
        case LoginResult::SUCCESSFUL:
            break;
    }

    // informate user about error
    QMessageBox messageBox(this);

    messageBox.setWindowTitle(tr("Anmelden"));
    messageBox.setText(tr("Die Anmeldung ist fehlgeschlagen."));
    messageBox.setInformativeText(result + "\n");
    messageBox.setIcon(QMessageBox::Icon::Warning);
    messageBox.setStandardButtons(QMessageBox::StandardButton::Ok);

    messageBox.exec();

    // select all password text for feast retype
    ui->lePassword->setFocus();
    ui->lePassword->selectAll();
}

void QMAMSLoginDialog::cancel()
{
    close();
}
