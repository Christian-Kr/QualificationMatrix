//
// qmdatabaseupdater.cpp is part of QualificationMatrix
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

#include "qmdatabaseupdater.h"
#include "model/qmdatamanager.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <QFileInfo>
#include <QDir>

#include <QDebug>

QMDatabaseUpdater::QMDatabaseUpdater(QObject *parent)
    : QObject(parent)
{
    majorSource = -1;
    minorSource = -1;

    majorTarget = -1;
    minorTarget = -1;
}

bool QMDatabaseUpdater::updateDatabase(const QSqlDatabase &db)
{
    if (!db.isOpen())
    {
        return false;
    }

    // Get source version.
    readDatabaseVersion(db);

    // Get target version.
    majorTarget = QMDataManager::getMajor();
    minorTarget = QMDataManager::getMinor();

    // Get list of all scripts as string. Extract only the numbers as a list and sort them.
    auto scripts = getUpdateScriptNames();
    qDebug() << scripts;

    // Decide which script to run

    // Run the scripts for update

    return false;
}

void QMDatabaseUpdater::readDatabaseVersion(const QSqlDatabase &db)
{
    if (!db.isOpen())
    {
        return;
    }

    QSqlQuery query(db);
    QString queryText =
        "SELECT name, value FROM Info "
        "WHERE name == \"version_minor\" OR name == \"version_major\"";

    if (!query.exec(queryText))
    {
        return;
    }

    majorSource = -1;
    minorSource = -1;

    while (query.next())
    {
        if (query.value("name").toString() == "version_major")
        {
            majorSource = query.value("value").toInt();
            continue;
        }

        if (query.value("name").toString() == "version_minor")
        {
            minorSource = query.value("value").toInt();
            continue;
        }

        if (minorSource != -1 && majorSource != -1)
        {
            break;
        }
    }
}

QStringList QMDatabaseUpdater::getUpdateScriptNames()
{
    QFileInfo pathInfo("database");

    if (!pathInfo.isDir() || !pathInfo.exists())
    {
        qWarning() << "could not find update scripts folder";
        return {};
    }

    QStringList filters;
    filters << QString("sql_*.sql");

    QDir databaseDir(pathInfo.absoluteFilePath());
    databaseDir.setNameFilters(filters);
    databaseDir.setSorting(QDir::Name);

    return databaseDir.entryList();
}
