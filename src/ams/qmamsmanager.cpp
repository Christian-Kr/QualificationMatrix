// qmamsmanager.cpp is part of QualificationMatrix
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

#include "qmamsmanager.h"
#include "ams/model/qmamsusermodel.h"

#include <QSqlDatabase>
#include <QSqlRecord>
#include <QSqlField>
#include <QDateTime>
#include <QDebug>

QMAMSManager *QMAMSManager::instance = nullptr;

QMAMSManager::QMAMSManager()
    : QObject()
    , username(std::make_unique<QString>())
    , fullname(std::make_unique<QString>())
    , loginState(LoginState::NOT_LOGGED_IN)
    , lastError(std::make_unique<QString>())
{}

bool QMAMSManager::checkDatabase()
{
    // INFO: For now, this function won't check the whole structure and all of
    // its tables. This might already be implemented by version check. The only
    // check that needs to be done is: Exist an "administrator" user or not. If
    // the user does not exist, the method will try to create it with an empty
    // password. If creating the administrator user is successful the function
    // might return true, cause the check is ok again.

    // Get the database connection.
    if (!QSqlDatabase::contains("default") ||
    !QSqlDatabase::database("default", false).isOpen())
    {
        qWarning("Database is not connected");
        return false;
    }

    auto db = QSqlDatabase::database("default");

    if (checkForAdministrator(db))
    {
        return true;
    }
    else
    {
        if (createAdminInDatabase())
        {
            return true;
        }
    }

    return false;
}

bool QMAMSManager::checkForAdministrator(QSqlDatabase &database)
{
    QMAMSUserModel amsUserModel(this, database);
    amsUserModel.select();

    auto usernameFieldIndex = amsUserModel.fieldIndex("username");

    for (int i = 0; i < amsUserModel.rowCount(); i++)
    {
        auto usernameModelIndex = amsUserModel.index(i, usernameFieldIndex);
        auto dbUsername = amsUserModel.data(usernameModelIndex).toString();

        if (dbUsername == "administrator")
        {
            return true;
        }
    }

    return false;
}

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

bool QMAMSManager::setUserLastLoginDateInDatabase()
{
    // Get the database connection.
    if (!QSqlDatabase::contains("default") ||
        !QSqlDatabase::database("default", false).isOpen())
    {
        qWarning("Database is not connected");
        return false;
    }

    auto db = QSqlDatabase::database("default");

    QMAMSUserModel amsUserModel(this, db);
    amsUserModel.select();

    auto usernameFieldIndex = amsUserModel.fieldIndex("username");
    auto lastLoginIndex = amsUserModel.fieldIndex("last_login");

    for (int i = 0; i < amsUserModel.rowCount(); i++)
    {
        auto usernameModelIndex = amsUserModel.index(i, usernameFieldIndex);
        auto dbUsername = amsUserModel.data(usernameModelIndex).toString();

        if (dbUsername == *username)
        {
            auto currDateTime = QDateTime::currentDateTime();
            auto strCurrDateTime = currDateTime.toString(Qt::ISODate);
            auto lastLoginModelIndex = amsUserModel.index(i, lastLoginIndex);

            if (!amsUserModel.setData(lastLoginModelIndex, strCurrDateTime))
            {
                return false;
            }

            if (!amsUserModel.submitAll())
            {
                return false;
            }

            return true;
        }
    }

    return false;
}

bool QMAMSManager::loginAdmin(const QString &password)
{
    if (!logoutUser())
    {
        return false;
    }

    auto userInfo = getUserFromDatabase("administrator");
    if (!userInfo.found)
    {
        // The administrator user could not be found. Create the administrator
        // user with an empty password.
        if (!createAdminInDatabase())
        {
            return false;
        }

        userInfo = getUserFromDatabase("administrator");
        if (!userInfo.found)
        {
            return false;
        }
    }

    // If we are here, there is a user with the name "administrator". Go on
    // with login.
    if (password == userInfo.password)
    {
        *username = userInfo.username;
        *fullname = userInfo.fullname;

        if (!setUserLastLoginDateInDatabase())
        {
            qCritical() << "Cannot set last login date for admin user";
        }

        return true;
    }

    return false;
}

bool QMAMSManager::loginUser(const QString &name, const QString &password)
{
    if (!logoutUser())
    {
        return false;
    }

    auto userInfo = getUserFromDatabase(name);
    if (!userInfo.found)
    {
        return false;
    }

    if (password == userInfo.password)
    {
        *username = userInfo.username;
        *fullname = userInfo.fullname;

        return true;
    }

    return false;
}

bool QMAMSManager::createAdminInDatabase()
{
    // Get the database connection.
    if (!QSqlDatabase::contains("default") ||
        !QSqlDatabase::database("default", false).isOpen())
    {
        qWarning("Database is not connected");
        return false;
    }

    auto db = QSqlDatabase::database("default");

    QMAMSUserModel amsUserModel(this, db);
    amsUserModel.select();

    auto usernameFieldIndex = amsUserModel.fieldIndex("username");

    for (int i = 0; i < amsUserModel.rowCount(); i++)
    {
        auto usernameModelIndex = amsUserModel.index(i, usernameFieldIndex);
        auto dbUsername = amsUserModel.data(usernameModelIndex).toString();

        if (dbUsername == "administrator")
        {
            return false;
        }
    }

    QSqlRecord record;

    record.append(QSqlField("name"));
    record.append(QSqlField("username"));
    record.append(QSqlField("password"));
    record.append(QSqlField("last_login"));
    record.append(QSqlField("unsuccess_login_num"));
    record.append(QSqlField("active"));

    record.setValue("name", "administrator");
    record.setValue("username", "administrator");
    record.setValue("password", "");
    record.setValue("last_login", "");
    record.setValue("unsuccess_login_num", 0);
    record.setValue("active", 1);

    if (!amsUserModel.insertRecord(-1, record) ||
        !amsUserModel.submitAll())
    {
        return false;
    }

    return true;
}

QMAMSUserInformation QMAMSManager::getUserFromDatabase(const QString &username)
{
    QMAMSUserInformation userInfo;

    // Get the database connection.
    if (!QSqlDatabase::contains("default") ||
        !QSqlDatabase::database("default", false).isOpen())
    {
        qWarning("Database is not connected");
        return userInfo;
    }

    auto db = QSqlDatabase::database("default");

    QMAMSUserModel amsUserModel(this, db);
    amsUserModel.select();

    auto usernameFieldIndex = amsUserModel.fieldIndex("username");

    for (int i = 0; i < amsUserModel.rowCount(); i++)
    {
        auto usernameModelIndex = amsUserModel.index(i, usernameFieldIndex);
        auto dbUsername = amsUserModel.data(usernameModelIndex).toString();

        if (dbUsername == username)
        {
            auto nameFieldIndex = amsUserModel.fieldIndex("user");
            auto nameModelIndex = amsUserModel.index(i, nameFieldIndex);
            auto dbFullname = amsUserModel.data(nameModelIndex).toString();

            auto pwFieldIndex = amsUserModel.fieldIndex("password");
            auto pwModelIndex = amsUserModel.index(i, pwFieldIndex);
            auto dbPassword = amsUserModel.data(pwModelIndex).toString();

            userInfo.username = dbUsername;
            userInfo.password = dbPassword;
            userInfo.fullname = dbFullname;
            userInfo.found = true;

            return userInfo;
        }
    }

    return userInfo;
}
