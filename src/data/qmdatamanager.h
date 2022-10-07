// qmdatamanager.h is part of QualificationMatrix
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

#ifndef QMDATAMANAGER_H
#define QMDATAMANAGER_H

#include "config.h"

#include <QObject>
#include <memory>

// Forward declaration
class QMSqlTableModel;
class QSqlRelationalTableModel;
class QSqlDatabase;
class QSqlTableModel;
class QMQualiResultModel;
class QMQualiMatrixModel;

// Typedefs
typedef std::shared_ptr<QMSqlTableModel> sp_qmRelTableModel;
typedef std::shared_ptr<QSqlRelationalTableModel> sp_relTableModel;
typedef std::shared_ptr<QSqlTableModel> sp_tableModel;

// Enumerations
enum class CertLoc
{
    INTERNAL,
    EXTERNAL
};

/// Manager for data including models.
/// \author Christian Kr, Copyright 2020
class QMDataManager: public QObject
{
    Q_OBJECT

public:
    /// Get a singleton instance of the data manager.
    /// \return Singleton instance of the data manager.
    static QMDataManager * getInstance()
    {
        if (instance == nullptr)
        {
            instance = new QMDataManager();
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

    static int getMinor() { return DB_MIN_MINOR; }
    static int getMajor() { return DB_MIN_MAJOR; }

    /// Get the location of the certificates. The setting is found in "Info" table.
    /// \return Certification location from enumeration type CertLoc.
    CertLoc getCertificateLocation() { return certLoc; }

    /// Get the location of the certificates. The setting is found in "Info" table.
    /// \return Certification location from enumeration type CertLoc.
    QString getCertificateLocationPath() { return certLocPath; }

    /// Read the certification location setting from table.
    /// \param db Database to work on.
    /// \return True if setting could be found and read, else false.
    bool readCertificateLocation(const QSqlDatabase &db);

    /// Read the certification location path setting from table.
    /// \param db Database to work on.
    /// \return True if setting could be found and read, else false.
    bool readCertificateLocationPath(const QSqlDatabase &db);

    // Delete functions that should not be used, cause of singleton pattern. They should be deleted
    // public for better error message if anything goes wrong.
    QMDataManager(QMDataManager const &) = delete;
    void operator=(QMDataManager const &) = delete;

    /// Test for the database version. This is only a test of an entry with the right version
    /// number. It does not include any test of available tables and/or columns.
    /// \param db The database to test.
    /// \return True if right version, else false.
    static bool testVersion(QSqlDatabase &db);

    /// Test for right table structure. This test only has a look for the number of tables and the
    /// table names. It does not has a look into the structure of a single table itself.
    /// \param db The database to test.
    /// \return True if structure is ok, else false.
    static bool testTableStructure(QSqlDatabase &db);

    /// Send a signal to all connected models.
    /// \param sender The sender of the information (mostly object calling this function).
    void sendModelChangedInformation(QObject *sender);

private:
    /// Constructor
    QMDataManager();

    /// Destructor
    ~QMDataManager() override = default;

signals:

private:
    static QMDataManager *instance;

    CertLoc certLoc;
    QString certLocPath;
};

#endif // QMDATAMANAGER_H
