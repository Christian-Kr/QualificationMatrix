// qmdatabasemanager.h is part of QualificationMatrix
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

#ifndef QMDATABASEMANAGER_H
#define QMDATABASEMANAGER_H

#include <QObject>

class QSqlDatabase;

/// Update a database to a current version.
/// \author Christian Kr, Copyright 2020
class QMDatabaseManager: public QObject
{
    Q_OBJECT

public:
    /// Constructor
    /// \param parent The parent obejct for the qt system.
    explicit QMDatabaseManager(QObject *parent = nullptr);

    /// Destructor
    ~QMDatabaseManager() override = default;

    /// Parse major number from script name string.
    /// \param scriptName The name of the script which includes the number by definition.
    /// \return Major number or -1 if conversion fails.
    static int getMajorFromScriptName(const QString &scriptName);

    /// Parse minor number from script name string.
    /// \param scriptName The name of the script which includes the number by definition.
    /// \return Major number or -1 if conversion fails.
    static int getMinorFromScriptName(const QString &scriptName);

signals:
    /// \brief always when an update will be send while updating a database
    /// \param msg what is the progress doing now
    void updateProgressState(QString msg);

public slots:
    /// \brief includes all steps before and after updating the database
    /// \param db needs to be opened already
    bool updateDatabase(const QSqlDatabase &db);

    /// Run initial script to a brand new database.
    /// \param db needs to be opened already; needs to be empty, otherwise the process will fail
    static bool initializeDatabase(const QSqlDatabase &db);

private:
    /// Run the given script on the database.
    /// \param db The database to run the script on.
    /// \param scriptName The name of the script to run.
    /// \return True if success, else false.
    static bool runScriptOnDatabase(const QSqlDatabase &db, const QString &sciptName);

    /// Read the database version.
    void readDatabaseVersion(const QSqlDatabase &db);

    /// Get a list of all scripts available to update the sql database.
    /// \return A list with all found script names.
    static QStringList getUpdateScriptNames();

    /// Get the name of the initialize script.
    /// \return the absolute path including the file name, or an empty string if anything fails
    static QString getInitializeScriptName();

    int majorSource;
    int minorSource;

    int majorTarget;
    int minorTarget;
};

#endif // QMDATABASEMANAGER_H
