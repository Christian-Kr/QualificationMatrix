// qmamsmanager.h is part of QualificationMatrix
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

#ifndef QMAMSMANAGER_H
#define QMAMSMANAGER_H

#include <QObject>
#include <memory>

#define MAX_LOGIN_COUNT 3

class QSqlDatabase;

// Enumerations for access modes. Every WRITE access includes automatic READ
// access.
// QM = QualificationMatrix
// QR = QualificationResult
// TD = TrainingData
// Having no enum flag at all means no access. This philosophie is known as
// whitelist. Meaning everything is forbidden except the application explicitly
// says that access is granted. If access to some features is given, they might
// additionally be restricted to specific user.
enum class AccessMode
{
    // Allows to enter the qualification matrix mode and view all contents.
    // Write acces gives additional possibility to
    // edit the qualification matrix.
    QM_MODE_READ = 0,
    QM_MODE_WRITE = 1,

    // Allows to view results. There is no need to give write access, cause
    // there is nothing to edit.
    QR_MODE_READ = 2,

    // Allows to view and/or edit training data.
    TD_MODE_READ = 3,
    TD_MODE_WRITE = 4,

    // Allow to view and edit persistent data in tables like functions,
    // trainings and or employees etc.
    PER_DATA_CONFIG = 5
};

/// For the login function, return a code what happens.
enum class LoginResult
{
    SUCCESSFUL,
    USER_NOT_ACTIVE,
    FAILED_LOGIN_COUNT,
    WRONG_PASSWORD,
    USER_NOT_EXIST,
    FAILED_UNKNOWN,
    EMPTY_PASSWORD
};

/// Give the state if any user is logged in or not.
enum class LoginState
{
    LOGGED_IN,
    NOT_LOGGED_IN
};


/// The struct holds information about general access permissions for a user.
struct QMAMSUserGeneralAccessPermissions
{
    // A list with access modes from the database. The codes must be available as an enum from
    // AccessMode.
    QList<int> accessModes;
};

/// The struct is an information holder for user data.
struct QMAMSUserInformation
{
    bool found = false;
    bool active = false;
    bool admin = false;
    int failedLoginCount = 0;

    // The database id representing the primary key.
    int id;

    // The name that will be used for login.
    QString username;

    QString fullname;
    QString password;

    QMAMSUserGeneralAccessPermissions generalPermissions;

    // Variable holds a list of primary keys of the users that are visible. All other users should
    // never be visible to the currently logged in user.
    QList<int> allowedUsersPrimaryKeys;
};

/// The class is the manager for the acces management system. Here you get all
/// access to functionalities that controls the access to the database content.
///
/// TODO: It might be a good practice to modify the general database table.
///
/// \author Christian Kr, Copyright 2020
class QMAMSManager: public QObject
{
    Q_OBJECT

public:
    /// Get a singleton instance of the ams manager.
    /// \return Singleton instance of the ams manager.
    static QMAMSManager * getInstance()
    {
        if (instance == nullptr)
        {
            instance = new QMAMSManager();
        }

        return instance;
    }

    /// Delete the static variable.
    static void resetInstance()
    {
        if (instance != nullptr)
        {
            delete instance;
            instance = nullptr;
        }
    }

    /// Check database for right structure and content to be sure that
    /// everything works right.
    /// \return True if everything is ok, else false. False will also be
    ///     returned, when there is no database connected, or it can't be
    ///     opened.
    bool checkDatabase();

    /// Create an administrator username.
    /// \return True if creation was successful, else false.
    bool createAdminInDatabase();

    /// Check for permission.
    /// \param mode The access mode to test for.
    /// \return False if logged in user does not have the permission, else false.
    bool checkPermission(AccessMode mode);

    /// Check whether the logged in user has administrator permissions or not. The admin
    /// permission is kind of special, cause it euqals to the user "administrator" or to a user
    /// having the admin flag set to true.
    /// \return True if a user is logged in and has admin permissions.
    bool checkAdminPermission();

    /// Get full name of the currently logged in user.
    /// \return The full name of the current login user, else empty.
    QString getLoginFullName() const { return loggedinUser->fullname; }

    /// Get user name of the current login user.
    /// \return The login name of the current login user, else empty.
    QString getLoginUserName() const { return loggedinUser->username; }

    /// Login with the given credentials. Any logged in user will
    /// automatically be logged out.
    /// \param password The password for login.
    /// \param username The username for login.
    /// \return Return code of login result.
    LoginResult loginUser(const QString &username, const QString &password);

    /// Logout the current user.
    void logoutUser();

    /// Call if you want to know whether a user is logged in or not.
    /// \return
    LoginState getLoginState() { return loginState; }

    /// Create a hash value for the given password.
    /// \param pw The clear password to hash.
    /// \return The total hash string which might be additionally encoded.
    static QString createPasswordHash(const QString &pw);

    /// Check the given hash against the given password.
    /// \param pw The clear password to check against.
    /// \param hash The total hash string including all aditional information.
    /// \return True if equal, else false.
    static bool checkPasswordHash(const QString &pw, const QString &hash);

    /// Set the password for the given user.
    /// \param password The new clear text password.
    /// \param userId The user id which is the primary key of the user in the AMSUser table.
    /// \return True if successfull, else false.
    bool setPassword(int userId, const QString &password);

    /// Get primary keys of allowed employees.
    /// \return A list of allowed employee primary keys.
    QList<int> getEmployeePrimaryKeys() { return loggedinUser->allowedUsersPrimaryKeys; }

signals:
    /// Emited when the login state changed.
    /// \param before Login state before the change.
    /// \param current Current login state after change.
    void loginStateChanged(LoginState before, LoginState current);

private:
    /// Constructor
    QMAMSManager();

    /// Destructor
    ~QMAMSManager() override = default;

    /// Set the login state and send a signal for changing state.
    /// \param state The login state.
    void setLoginState(LoginState state);

    /// Search for administrator user.
    /// \return True if exist, else false.
    bool checkForAdministrator(QSqlDatabase &database);

    /// Filter the list with group names of active name.
    /// \param groupNames The group names to filter.
    /// \return A list with groups, that are all active.
    QList<QString> getActiveGroups(const QList<QString> &groupNames);

    /// Get information from a user. This function will take the information
    /// from the database directly.
    /// \param username The username to get information from.
    /// \return User information as a struct.
    QMAMSUserInformation getUserFromDatabase(const QString &username);

    /// Get the general permission information from a user.
    /// \param userInfo The user information struct to find the user.
    /// \return General user permission struct.
    QMAMSUserGeneralAccessPermissions getUserGeneralAccessPermissionsFromDatabase(QMAMSUserInformation &userInfo);

    /// Get the employee permission information from a user.
    /// \param userInfo The user information struct to find the user.
    /// \return General employee permission list.
    QList<int> getUserEmployeeAccessPermissionsFromDatabase(QMAMSUserInformation &userInfo);

    /// Get a list of all group names, that correlate to the given username.
    /// \param username The name of the user to get the information for.
    /// \return A list of group names, the user ist part of.
    QList<QString> getUserGroupsFromDatabase(const QString &username);

    /// Get a list of all access modes for the list of group names.
    /// \param groupNames The list of group names to get the access modes for.
    /// \return A list of all access modes for the given list of group names.
    QList<QString> getGroupAccessModesFromDatabase(const QList<QString> &groupNames);

    /// Get a list of all employees for the list of group names.
    /// \param groupNames The list of group names to get the employees for.
    /// \return A list of all employees for the given list of group names.
    QList<QString> getGroupEmployeeFromDatabase(const QList<QString> &groupNames);

    /// Get a list of primary keys from employees.
    /// \param employeeNames The employee names list.
    /// \return A list of all primary keys from employee names.
    QList<int> getEmployeePrimaryKeyFromNamesFromDatabase(const QList<QString> &employeeNames);

    /// Get a list of values from a list access mode names.
    /// \param accessModeNames The acces mode names to translate.
    /// \return A list with all values for access mode names.
    QList<int> getAccessModeValuesFromDatabase(const QList<QString> &accessModeNames);

    /// Set the timestamp to the last login user.
    /// \param user The user to set the last login timestamp.
    /// \return True is success, else false.
    static bool setLastLoginDateTime(QString &user);

    /// Set the failed login count to a specific number.
    /// \param name The username to set the count for.
    /// \param count The count to set.
    static bool setFailedLoginCount(QString &name, int count);

    // Variables

    static QMAMSManager *instance;

    std::unique_ptr<QMAMSUserInformation> loggedinUser;
    LoginState loginState;
    std::unique_ptr<QString> lastError;
};

#endif // QMAMSMANAGER_H
