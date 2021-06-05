// qmamsmanager.cpp is part of QualificationMatrix
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

#include "qmamsmanager.h"

#include <QSqlDatabase>
#include <QDebug>

QMAMSManager *QMAMSManager::instance = nullptr;

QMAMSManager::QMAMSManager()
    : QObject()
    , username(std::make_unique<QString>())
    , fullname(std::make_unique<QString>())
    , loginState(LoginState::NOT_LOGGED_IN)
    , lastError(std::make_unique<QString>())
{}

bool QMAMSManager::logoutUser()
{
    if (loginState == LoginState::NOT_LOGGED_IN)
    {
        return true;
    }

    username->clear();
    fullname->clear();

    loginState = LoginState::NOT_LOGGED_IN;

    return true;
}

bool QMAMSManager::loginUser(const QString &name, const QString &password)
{
    if (!logoutUser())
    {
        return false;
    }

    QMAMSUserInformation userInfo = getUserFromDatabase(name);
}

QMAMSUserInformation QMAMSManager::getUserFromDatabase(const QString &username)
{
    QMAMSUserInformation userInfo;

    // Get the database connection.
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        qWarning("Database is not connected");
        return userInfo;
    }

    auto db = QSqlDatabase::database("default");

    // TODO: Create database model and search for user.

    return userInfo;
}
