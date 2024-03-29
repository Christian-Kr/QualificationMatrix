// qmamsmanager.cpp is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify it under the terms
// of the GNU General Public License as published by the Free Software Foundation, either version
// 3 of the License, or (at your option) any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
// the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with QualificationMatrix.
// If not, see <http://www.gnu.org/licenses/>.

#include "qmamsmanager.h"
#include "ams/model/qmamsusermodel.h"
#include "ams/model/qmamsgroupaccessmodemodel.h"
#include "ams/model/qmamsusergroupmodel.h"
#include "ams/model/qmamsaccessmodemodel.h"
#include "ams/model/qmamsgroupmodel.h"
#include "ams/model/qmamsgroupemployeemodel.h"
#include "data/employee/qmemployeemodel.h"
#include "ams/model/qmamsuseremployeeviewmodel.h"
#include "ams/model/qmamsuseremployeegroupviewmodel.h"

#include <QSqlDatabase>
#include <QSqlRecord>
#include <QSqlField>
#include <QDateTime>
#include <QCryptographicHash>

#include <botan/argon2fmt.h>
#include <botan/system_rng.h>

QMAMSManager *QMAMSManager::instance = nullptr;

QMAMSManager::QMAMSManager()
    : QObject()
    , loginState(LoginState::NOT_LOGGED_IN)
    , lastError(std::make_unique<QString>())
    , loggedinUser(std::make_unique<QMAMSUserInformation>())
{}

bool QMAMSManager::checkDatabase()
{
    // INFO: For now, this function won't check the whole structure and all of its tables. This might already be
    // implemented by version check. The only check that needs to be done is: Exist an "administrator" user or not. If
    // the user does not exist, the method will try to create it with an empty password. If creating the
    // administrator user is successful the function might return true, cause the check is ok again.

    // Get the database connection.
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
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

    auto usernameFieldIndex = amsUserModel.fieldIndex("amsuser_username");

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

void QMAMSManager::logoutUser()
{
    loggedinUser = std::make_unique<QMAMSUserInformation>();
    setLoginState(LoginState::NOT_LOGGED_IN);
}

bool QMAMSManager::setFailedLoginCount(QString &name, int count)
{
    // Get the database connection.
    if (!QSqlDatabase::contains("default") ||
    !QSqlDatabase::database("default", false).isOpen())
    {
        qWarning("Database is not connected");
        return false;
    }

    auto db = QSqlDatabase::database("default");

    QMAMSUserModel amsUserModel(nullptr, db);
    amsUserModel.select();

    auto usernameFieldIndex = amsUserModel.fieldIndex("amsuser_username");
    auto failedLoginIndex = amsUserModel.fieldIndex("amsuser_unsuccess_login_num");

    for (int i = 0; i < amsUserModel.rowCount(); i++)
    {
        auto usernameModelIndex = amsUserModel.index(i, usernameFieldIndex);
        auto dbUsername = amsUserModel.data(usernameModelIndex).toString();

        if (dbUsername.toUpper().compare(name.toUpper()))
        {
            auto failedLoginModelIndex = amsUserModel.index(i,
                    failedLoginIndex);

            if (!amsUserModel.setData(failedLoginModelIndex, count))
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

bool QMAMSManager::setLastLoginDateTime(QString &name)
{
    // Get the database connection.
    if (!QSqlDatabase::contains("default") ||
        !QSqlDatabase::database("default", false).isOpen())
    {
        qWarning("Database is not connected");
        return false;
    }

    auto db = QSqlDatabase::database("default");

    QMAMSUserModel amsUserModel(nullptr, db);
    amsUserModel.select();

    auto usernameFieldIndex = amsUserModel.fieldIndex("amsuser_username");
    auto lastLoginIndex = amsUserModel.fieldIndex("amsuser_last_login");

    for (int i = 0; i < amsUserModel.rowCount(); i++)
    {
        auto usernameModelIndex = amsUserModel.index(i, usernameFieldIndex);
        auto dbUsername = amsUserModel.data(usernameModelIndex).toString();

        if (dbUsername.toUpper().compare(name.toUpper()) == 0)
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

bool QMAMSManager::setPassword(int userId, const QString &password)
{
    // This function can - for now - only be used, when you are logged in as administrator user.
    if (loginState == LoginState::NOT_LOGGED_IN || loggedinUser->username.compare("administrator") != 0)
    {
        return false;
    }

    // Get the database connection.
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        qWarning("Database is not connected");
        return false;
    }

    auto db = QSqlDatabase::database("default");

    QMAMSUserModel amsUserModel(this, db);
    amsUserModel.select();

    auto userIdFieldIndex = amsUserModel.fieldIndex("amsuser_id");
    auto passwordFieldIndex = amsUserModel.fieldIndex("amsuser_password");

    for (int i = 0; i < amsUserModel.rowCount(); i++)
    {
        auto userIdModelIndex = amsUserModel.index(i, userIdFieldIndex);
        auto dbUserId = amsUserModel.data(userIdModelIndex).toInt();

        if (dbUserId == userId)
        {
            auto passwordModelIndex = amsUserModel.index(i, passwordFieldIndex);
            auto newPassword = createPasswordHash(password);

            if (amsUserModel.setData(passwordModelIndex, newPassword) && amsUserModel.submitAll())
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }

    return false;
}

LoginResult QMAMSManager::loginUser(const QString &name, const QString &password)
{
    logoutUser();

    // Get the user data from the database.
    auto userInfo = getUserFromDatabase(name);

    // Do several tests, whether login should fail or not.
    if (!userInfo.found)
    {
        return LoginResult::USER_NOT_EXIST;
    }

    if (!userInfo.active)
    {
        return LoginResult::USER_NOT_ACTIVE;
    }

    if (userInfo.failedLoginCount > MAX_LOGIN_COUNT || userInfo.failedLoginCount < 0)
    {
        return LoginResult::FAILED_LOGIN_COUNT;
    }

    if (!userInfo.password.isEmpty())
    {
        if(checkPasswordHash(password, userInfo.password))
        {
            if (!setLastLoginDateTime(userInfo.username))
            {
                qCritical() << "cannot set last login date";
            }

            if (!setFailedLoginCount(userInfo.username, 0))
            {
                qCritical() << "cannot set failed login count";
            }

            loggedinUser->id = userInfo.id;
            loggedinUser->username = userInfo.username;
            loggedinUser->fullname = userInfo.fullname;

            auto generalPermissions = getUserGeneralAccessPermissionsFromDatabase(*loggedinUser);
            loggedinUser->generalPermissions = generalPermissions;
            loggedinUser->allowedUsersPrimaryKeys = getUserEmployeeAccessPermissionsIds(
                    *loggedinUser);
            setLoginState(LoginState::LOGGED_IN);

            loggedinUser->admin = userInfo.admin;

            return LoginResult::SUCCESSFUL;
        }
        else
        {
            if (!setFailedLoginCount(userInfo.username, ++userInfo.failedLoginCount))
            {
                qCritical() << "cannot set failed login count";
            }

            return LoginResult::WRONG_PASSWORD;
        }
    }

    return LoginResult::EMPTY_PASSWORD;
}

bool QMAMSManager::createAdminInDatabase()
{
    // Get the database connection.
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        qWarning("Database is not connected");
        return false;
    }

    auto db = QSqlDatabase::database("default");

    QMAMSUserModel amsUserModel(this, db);
    amsUserModel.select();

    auto usernameFieldIndex = amsUserModel.fieldIndex("amsuser_username");

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

    record.append(QSqlField("amsuser_name"));
    record.append(QSqlField("amsuser_username"));
    record.append(QSqlField("amsuser_password"));
    record.append(QSqlField("amsuser_last_login"));
    record.append(QSqlField("amsuser_unsuccess_login_num"));
    record.append(QSqlField("amsuser_active"));
    record.append(QSqlField("amsuser_admin"));

    record.setValue("amsuser_name", "administrator");
    record.setValue("amsuser_username", "administrator");
    record.setValue("amsuser_password", "");
    record.setValue("amsuser_last_login", "");
    record.setValue("amsuser_unsuccess_login_num", 0);
    record.setValue("amsuser_active", 1);
    record.setValue("amsuser_admin", 1);

    if (!amsUserModel.insertRecord(-1, record) || !amsUserModel.submitAll())
    {
        return false;
    }

    return true;
}

QList<int> QMAMSManager::getUserEmployeeAccessPermissionsIds(QMAMSUserInformation &userInfo)
{
    // Get the database connection.
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        qWarning("Database is not connected");
        return {};
    }

    auto db = QSqlDatabase::database("default");

    QList<int> ids;

    QMAMSUserEmployeeViewModel amsUserEmployeeViewModel(this, db);
    amsUserEmployeeViewModel.setFilter(QString("user_id=%1").arg(userInfo.id));
    amsUserEmployeeViewModel.select();

    for (int i = 0; i < amsUserEmployeeViewModel.rowCount(); i++)
    {
        auto employeeFieldIndex = amsUserEmployeeViewModel.fieldIndex("employee_id");
        auto employeeModelIndex = amsUserEmployeeViewModel.index(i, employeeFieldIndex);
        auto employeeId = amsUserEmployeeViewModel.data(employeeModelIndex).toInt();

        ids.append(employeeId);
    }

    QMAMSUserEmployeeGroupViewModel amsUserEmployeeGroupViewModel(this, db);
    amsUserEmployeeGroupViewModel.setFilter(QString("user_id=%1").arg(userInfo.id));
    amsUserEmployeeGroupViewModel.select();

    for (int i = 0; i < amsUserEmployeeGroupViewModel.rowCount(); i++)
    {
        auto employeeFieldIndex = amsUserEmployeeGroupViewModel.fieldIndex("employee_id");
        auto employeeModelIndex = amsUserEmployeeGroupViewModel.index(i, employeeFieldIndex);
        auto employeeId = amsUserEmployeeGroupViewModel.data(employeeModelIndex).toInt();

        if (!ids.contains(employeeId))
        {
            ids.append(employeeId);
        }
    }

    return ids;
}

QList<int> QMAMSManager::getEmployeePrimaryKeyFromNamesFromDatabase(const QList<QString> &employeeNames)
{
    // Get the database connection.
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        qWarning("Database is not connected");
        return {};
    }

    auto db = QSqlDatabase::database("default");

    QMEmployeeModel employeeModel(this, db);
    employeeModel.select();

    auto employeeIdFieldIndex = employeeModel.fieldIndex("id");
    auto employeeNameFieldIndex = employeeModel.fieldIndex("name");
    if (employeeIdFieldIndex < 0 || employeeNameFieldIndex < 0)
    {
        qCritical() << "Cannot find field index in employee view data";
        return {};
    }

    QList<int> employeeIds;

    for (int i = 0; i < employeeNames.count(); i++)
    {
        for (int j = 0; j < employeeModel.rowCount(); j++)
        {
            auto employeeIdModelIndex = employeeModel.index(j, employeeIdFieldIndex);
            auto dbEmployeeId = employeeModel.data(employeeIdModelIndex).toInt();

            auto employeeNameModelIndex = employeeModel.index(j, employeeNameFieldIndex);
            auto dbEmployeeName = employeeModel.data(employeeNameModelIndex).toString();

            if (dbEmployeeName.compare(employeeNames.at(i)) == 0)
            {
                if (!employeeIds.contains(dbEmployeeId))
                {
                    employeeIds.append(dbEmployeeId);
                }

                continue;
            }
        }
    }

    return employeeIds;
}

QMAMSUserGeneralAccessPermissions QMAMSManager::getUserGeneralAccessPermissionsFromDatabase(
        QMAMSUserInformation &userInfo)
{
    // Get the database connection.
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        qWarning("Database is not connected");
        return {};
    }

    auto db = QSqlDatabase::database("default");

    // To ge the necessary information, get the user id, get the connected groups to the user id and at last, get the
    // permissions that are correlated with the groups.

    QList<QString> groupNames = getUserGroupsFromDatabase(userInfo.username);
    QList<QString> activeGroupNames = getActiveGroups(groupNames);
    if (activeGroupNames.isEmpty())
    {
        return {};
    }

    QList<QString> accessModeNames = getGroupAccessModesFromDatabase(groupNames);
    if (accessModeNames.isEmpty())
    {
        return {};
    }

    QList<int> accessModeValues = getAccessModeValuesFromDatabase(accessModeNames);
    if (accessModeValues.isEmpty())
    {
        return {};
    }

    QMAMSUserGeneralAccessPermissions generalPermissions;
    generalPermissions.accessModes = accessModeValues;

    return generalPermissions;
}

QList<QString> QMAMSManager::getActiveGroups(const QList<QString> &groupNames)
{
    // Get the database connection.
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        qWarning("Database is not connected");
        return {};
    }

    auto db = QSqlDatabase::database("default");

    QList<QString> activeGroupNames;

    QMAMSGroupModel amsGroupModel(this, db);
    amsGroupModel.select();

    auto groupNameFieldIndex = amsGroupModel.fieldIndex("amsgroup_name");
    auto activeFieldIndex = amsGroupModel.fieldIndex("amsgroup_active");

    for (int i = 0; i < groupNames.count(); i ++)
    {
        auto groupName = groupNames.at(i);

        for (int j = 0; j < amsGroupModel.rowCount(); j++)
        {
            auto groupNameModelIndex = amsGroupModel.index(j, groupNameFieldIndex);
            auto activeModelIndex = amsGroupModel.index(j, activeFieldIndex);

            auto dbGroupName = amsGroupModel.data(groupNameModelIndex).toString();
            auto dbGroupActive = amsGroupModel.data(activeModelIndex).toBool();

            if (dbGroupName.compare(groupName) == 0 && dbGroupActive)
            {
                activeGroupNames.append(groupName);
                break;
            }
        }
    }

    return activeGroupNames;
}

QList<int> QMAMSManager::getAccessModeValuesFromDatabase(const QList<QString> &accessModeNames)
{
    // Get the database connection.
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        qWarning("Database is not connected");
        return {};
    }

    auto db = QSqlDatabase::database("default");

    QMAMSAccessModeModel amsAccessModeModel(this, db);
    amsAccessModeModel.select();

    QList<int> accessModeValues;

    auto nameFieldIndex = amsAccessModeModel.fieldIndex("amsaccessmode_name");
    auto valueFieldIndex = amsAccessModeModel.fieldIndex("amsaccessmode_value");

    for (int i = 0; i < accessModeNames.count(); i++)
    {
        auto &accessModeName = accessModeNames.at(i);
        for (int j = 0; j < amsAccessModeModel.rowCount(); j++)
        {
            auto nameModelIndex = amsAccessModeModel.index(j, nameFieldIndex);
            auto dbName = amsAccessModeModel.data(nameModelIndex).toString();

            if (dbName.compare(accessModeName) == 0)
            {
                auto valueModelIndex = amsAccessModeModel.index(j, valueFieldIndex);
                auto dbValue = amsAccessModeModel.data(valueModelIndex).toInt();

                if (!accessModeValues.contains(dbValue))
                {
                    accessModeValues.append(dbValue);
                }

                // If we found the right entry, go on with the names list.
                break;
            }
        }
    }

    return accessModeValues;
}

QList<QString> QMAMSManager::getGroupEmployeeFromDatabase(const QList<QString> &groupNames)
{
    // Get the database connection.
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        qWarning("Database is not connected");
        return {};
    }

    auto db = QSqlDatabase::database("default");

    QMAMSGroupEmployeeModel amsGroupEmployeeModel(this, db);
    amsGroupEmployeeModel.select();

    QList<QString> employeeNames;

    auto groupnameFieldIndex = amsGroupEmployeeModel.fieldIndex("amsgroup_name");
    auto employeenameFieldIndex = amsGroupEmployeeModel.fieldIndex("name");

    for (int i = 0; i < amsGroupEmployeeModel.rowCount(); i++)
    {
        auto groupnameModelIndex = amsGroupEmployeeModel.index(i, groupnameFieldIndex);
        auto dbGroupname = amsGroupEmployeeModel.data(groupnameModelIndex).toString();

        for (int j = 0; j < groupNames.count(); j++)
        {
            auto &groupname = groupNames.at(j);

            if (dbGroupname.compare(groupname) == 0)
            {
                // Get the employee name.
                auto employeeModelIndex = amsGroupEmployeeModel.index(i, employeenameFieldIndex);
                auto dbEmployeename = amsGroupEmployeeModel.data(employeeModelIndex).toString();

                if (!employeeNames.contains(dbEmployeename))
                {
                    employeeNames.append(dbEmployeename);
                }

                // If one of the groups fit, we don't have to search for further, cause the employee has already
                // been added.
                break;
            }
        }
    }

    return employeeNames;
}

QList<QString> QMAMSManager::getGroupAccessModesFromDatabase(const QList<QString> &groupNames)
{
    // Get the database connection.
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        qWarning("Database is not connected");
        return {};
    }

    auto db = QSqlDatabase::database("default");

    QMAMSGroupAccessModeModel amsGroupAccessModeModel(this, db);
    amsGroupAccessModeModel.select();

    QList<QString> accessModeNames;

    auto accessmodeFieldIndex = amsGroupAccessModeModel.fieldIndex("amsaccessmode_name");
    auto groupnameFieldIndex = amsGroupAccessModeModel.fieldIndex("amsgroup_name");
    for (int i = 0; i < amsGroupAccessModeModel.rowCount(); i++)
    {
        auto groupnameModelIndex = amsGroupAccessModeModel.index(i, groupnameFieldIndex);
        auto dbGroupname = amsGroupAccessModeModel.data(groupnameModelIndex).toString();

        for (int j = 0; j < groupNames.count(); j++)
        {
            auto &groupname = groupNames.at(j);

            if (dbGroupname.compare(groupname) == 0)
            {
                // Get the access mode.
                auto accessmodeModelIndex = amsGroupAccessModeModel.index(i, accessmodeFieldIndex);
                auto dbAccessmode = amsGroupAccessModeModel.data(accessmodeModelIndex).toString();

                if (!accessModeNames.contains(dbAccessmode))
                {
                    accessModeNames.append(dbAccessmode);
                }

                // If one of the groups fit, we don't have to search for further, cause the access mode has already
                // been added.
                break;
            }
        }
    }

    return accessModeNames;
}

QList<QString> QMAMSManager::getUserGroupsFromDatabase(const QString &username)
{
    // Get the database connection.
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        qWarning("Database is not connected");
        return {};
    }

    auto db = QSqlDatabase::database("default");

    QMAMSUserGroupModel amsUserGroupModel(this, db);
    amsUserGroupModel.select();

    QList<QString> groupNames;

    auto usernameFieldIndex = amsUserGroupModel.fieldIndex("amsuser_username");
    auto groupnameFieldIndex = amsUserGroupModel.fieldIndex("amsgroup_name");
    for (int i = 0; i < amsUserGroupModel.rowCount(); i++)
    {
        auto usernameModelIndex = amsUserGroupModel.index(i, usernameFieldIndex);
        auto dbUsername = amsUserGroupModel.data(usernameModelIndex).toString();
        if (dbUsername.toUpper().compare(username.toUpper()) == 0)
        {
            // Get the group name.
            auto groupnameModelIndex = amsUserGroupModel.index(i, groupnameFieldIndex);
            auto dbGroupname = amsUserGroupModel.data(groupnameModelIndex).toString();

            if (!groupNames.contains(dbGroupname))
            {
                groupNames.append(dbGroupname);
            }
        }
    }

    return groupNames;
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

    QMAMSUserModel amsUserModel(this, db);
    amsUserModel.select();

    auto usernameFieldIndex = amsUserModel.fieldIndex("amsuser_username");

    for (int i = 0; i < amsUserModel.rowCount(); i++)
    {
        auto usernameModelIndex = amsUserModel.index(i, usernameFieldIndex);
        auto dbUsername = amsUserModel.data(usernameModelIndex).toString();

        if (dbUsername.toUpper().compare(username.toUpper()) == 0)
        {
            auto idFieldIndex = amsUserModel.fieldIndex("amsuser_id");
            auto idModelIndex = amsUserModel.index(i, idFieldIndex);
            auto dbId = amsUserModel.data(idModelIndex).toInt();

            auto nameFieldIndex = amsUserModel.fieldIndex("amsuser_name");
            auto nameModelIndex = amsUserModel.index(i, nameFieldIndex);
            auto dbFullname = amsUserModel.data(nameModelIndex).toString();

            auto pwFieldIndex = amsUserModel.fieldIndex("amsuser_password");
            auto pwModelIndex = amsUserModel.index(i, pwFieldIndex);
            auto dbPassword = amsUserModel.data(pwModelIndex).toString();

            auto failedLoginFieldIndex = amsUserModel.fieldIndex("amsuser_unsuccess_login_num");
            auto failedLoginModelIndex = amsUserModel.index(i, failedLoginFieldIndex);
            auto dbFailedLoginCount = amsUserModel.data(failedLoginModelIndex).toInt();

            auto activeFieldIndex = amsUserModel.fieldIndex("amsuser_active");
            auto activeModelIndex = amsUserModel.index(i, activeFieldIndex);
            auto active = amsUserModel.data(activeModelIndex).toBool();

            auto adminFieldIndex = amsUserModel.fieldIndex("amsuser_admin");
            auto adminModelIndex = amsUserModel.index(i, adminFieldIndex);
            auto admin = amsUserModel.data(adminModelIndex).toBool();

            userInfo.username = dbUsername;
            userInfo.password = dbPassword;
            userInfo.fullname = dbFullname;
            userInfo.id = dbId;
            userInfo.found = true;
            userInfo.failedLoginCount = dbFailedLoginCount;
            userInfo.active = active;
            userInfo.admin = admin;

            return userInfo;
        }
    }

    return userInfo;
}

void QMAMSManager::setLoginState(LoginState state)
{
    auto lastState = getLoginState();
    loginState = state;

    if (lastState == getLoginState())
    {
        return;
    }

    emit loginStateChanged(lastState, getLoginState());
}

QString QMAMSManager::createPasswordHash(const QString &pw)
{
    auto tmpStdHash = Botan::argon2_generate_pwhash(pw.toStdString().c_str(), pw.length(),
            Botan::system_rng(), 1, 8192, 100);
    return QString::fromStdString(tmpStdHash);
}

bool QMAMSManager::checkPasswordHash(const QString &pw, const QString &hash)
{
    auto tmpStdHash = hash.toStdString();
    return Botan::argon2_check_pwhash(pw.toStdString().c_str(), pw.length(), tmpStdHash);
}

bool QMAMSManager::checkPermission(AccessMode mode)
{
    if (loginState == LoginState::NOT_LOGGED_IN)
    {
        return false;
    }

    if (loggedinUser->generalPermissions.accessModes.isEmpty())
    {
        return false;
    }

    return loggedinUser->generalPermissions.accessModes.contains(static_cast<int>(mode));
}

bool QMAMSManager::checkAdminPermission()
{
    if (loginState == LoginState::NOT_LOGGED_IN)
    {
        return false;
    }

    if (loggedinUser->username.compare("administrator") == 0) {
        return true;
    }

    if (loggedinUser->admin)
    {
        return true;
    }

    return false;
}
