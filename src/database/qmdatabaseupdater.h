//
// qmdatabaseupdater.h is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify it under the terms of
// the GNU General Public License as published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
// the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with QualificationMatrix.
// If not, see <http://www.gnu.org/licenses/>.
//

#ifndef QMDATABASEUPDATER_H
#define QMDATABASEUPDATER_H

#include <QObject>

class QSqlDatabase;

/// Update a database to a current version.
/// \author Christian Kr, Copyright 2020
class QMDatabaseUpdater: public QObject
{
Q_OBJECT

public:
    /// Constructor
    /// \param parent The parent obejct for the qt system.
    explicit QMDatabaseUpdater(QObject *parent = nullptr);

    /// Destructor
    ~QMDatabaseUpdater() override = default;

signals:
    /// \brief always when an update will be send while updating a database
    /// \param msg what is the progress doing now
    void updateProgressState(QString msg);

public slots:
    /// \brief includes all steps before and after updating the database
    /// \param db needs to be opened already
    bool updateDatabase(const QSqlDatabase &db);

private:
    /// Read the database version.
    void readDatabaseVersion(const QSqlDatabase &db);

    /// Get a list of all scripts available to update the sql database.
    /// \return A list with all found script names.
    static QStringList getUpdateScriptNames();

    int majorSource;
    int minorSource;

    int majorTarget;
    int minorTarget;
};

#endif // QMDATABASEUPDATER_H
