// qmamsmanager.h is part of QualificationMatrix
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

#ifndef QMAMSMANAGER_H
#define QMAMSMANAGER_H

#include <QObject>
#include <memory>

// Enumerations for access modes. Every WRITE access includes automatic READ access.
// QM = QualificationMatrix
// QR = QualificationResult
// TD = TrainingData
// Having no enum flag at all means no access. This philosophie is known as whitelist. Meaning everything is forbidden
// except the application explicitly says that access is granted. If access to some features is given, they might
// additionally be restricted to specific user.
enum class AccessMode
{
    // Allows to enter the qualification matrix mode and view all contents. Write acces gives additional possibility to
    // edit the qualification matrix.
    QM_MODE_READ,
    QM_MODE_WRITE,

    // Allows to view results. There is no need to give write access, cause there is nothing to edit.
    QR_MODE_READ,

    // Allows to view and/or edit training data.
    TD_MODE_READ,
    TD_MODE_WRITE,

    // Allow to view and/or edit persistent data in tables like functions, trainings and or employees etc.
    PER_DATA_READ,
    PER_DATA_WRITE
};

/// Thi class is the manager for the acces management system. Here you get all access to functionalities that
/// controls the access to the database content.
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

private:
    /// Constructor
    QMAMSManager();

    /// Destructor
    ~QMAMSManager() override = default;

signals:

private:
    static QMAMSManager *instance;
};

#endif // QMAMSMANAGER_H
