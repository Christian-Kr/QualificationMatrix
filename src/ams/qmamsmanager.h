// qmamsmanager.h is part of QualificationMatrix
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

#ifndef QMAMSMANAGER_H
#define QMAMSMANAGER_H

#include <QObject>
#include <memory>

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
    QM_MODE_READ,
    QM_MODE_WRITE,

    // Allows to view results. There is no need to give write access, cause
    // there is nothing to edit.
    QR_MODE_READ,

    // Allows to view and/or edit training data.
    TD_MODE_READ,
    TD_MODE_WRITE,

    // Allow to view and/or edit persistent data in tables like functions,
    // trainings and or employees etc.
    PER_DATA_READ,
    PER_DATA_WRITE
};

/// Give the state if any user is logged in or not.
enum class LoginState
{
    LOGGED_IN,
    NOT_LOGGED_IN
};

/// The struct is an information holder for user data.
struct QMAMSUserInformation
{
    bool found = false;
    int failedLoginCount = 0;
    QString username;
    QString fullname;
    QString password;
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

    /// Get full name of the currently logged in user.
    /// \return The full name of the current login user, else empty.
    QString * getLoginFullName() const { return fullname.get(); }

    /// Get user name of the current login user.
    /// \return The login name of the current login user, else empty.
    QString * getLoginUserName() const { return username.get(); }

    /// Login with the given credentials. Any logged in user will
    /// automatically be logged out.
    /// \param password The password for login.
    /// \param username The username for login.
    bool loginUser(const QString &username, const QString &password);

    /// Logout the current user.
    /// \return True on success, else false. When no user is logged in,
    ///     method will always return true.
    bool logoutUser();

    /// Call if you want to know whether a user is logged in or not.
    /// \return
    LoginState getLoginState() { return loginState; }

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

    /// Get the password from a user. This function will take the information
    /// from the database directly.
    /// \param username The username to get information from.
    /// \return User information as a struct.
    QMAMSUserInformation getUserFromDatabase(const QString &username);

    /// Set the timestamp to the last login user.
    /// \param user The user to set the last login timestamp
    /// \return True is success, else false
    static bool setLastLoginDateTime(QString user);

    // Variables

    static QMAMSManager *instance;

    std::unique_ptr<QString> username;
    std::unique_ptr<QString> fullname;

    LoginState loginState;

    std::unique_ptr<QString> lastError;
};

#endif // QMAMSMANAGER_H
