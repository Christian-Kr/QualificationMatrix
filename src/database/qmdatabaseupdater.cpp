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
#include <QSqlError>
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
    scripts.sort();

    // Go through the scripts from first to last and see whether it should be run or not.
    for (int i = 0; i < scripts.size(); i++)
    {
        auto &scriptName = scripts.at(i);
        int tmpMajor = QMDatabaseUpdater::getMajorFromScriptName(scriptName);
        int tmpMinor = QMDatabaseUpdater::getMinorFromScriptName(scriptName);

        if (tmpMajor < 0 && tmpMinor < 0)
        {
            qWarning() << "cannot update database due to unreadable major and/or minor version";
            return false;
        }

        // Continue if the major number of the script is smaller than the database major number or
        // bigger than the target version number.
        if (tmpMajor < majorSource ||
            tmpMajor > majorTarget)
        {
            continue;
        }

        // If the major number is equal and the minor number of the script is equal or smaller
        // continue.
        if ((tmpMajor == majorSource && tmpMinor <= minorSource) ||
            (tmpMajor == majorTarget && tmpMinor > minorTarget))
        {
            continue;
        }

        // Try to run script to database for an update.
        emit updateProgressState(tr("Update to version %1.%2").arg(tmpMajor).arg(tmpMinor));

        if (!QMDatabaseUpdater::runScriptOnDatabase(db, scriptName))
        {
            return false;
        }
    }

    return true;
}

bool QMDatabaseUpdater::runScriptOnDatabase(const QSqlDatabase &db, const QString &scriptName)
{
    QFileInfo scriptFileInfo(QDir("database"), scriptName);

    if (!scriptFileInfo.exists() || !scriptFileInfo.isFile())
    {
        qWarning() << "script certificate" << scriptName << "does not exist";
        return false;
    }

    QFile scriptFile(scriptFileInfo.absoluteFilePath());

    if (!scriptFile.open(QIODevice::Text | QIODevice::ReadOnly))
    {
        qWarning() << "cannot open script certificate" << scriptName;
        return false;
    }

    QTextStream scriptStream(&scriptFile);
    auto fullScript = scriptStream.readAll();

    auto queries = fullScript.split(";");
    QSqlQuery sqlQuery(db);

    for (int i = 0; i < queries.size(); i++)
    {
        auto query = queries.at(i).simplified();

        if (query.isEmpty())
        {
            continue;
        }

        if (!sqlQuery.exec(queries.at(i)))
        {
            qWarning() << "cannot run query on database from script certificate" << scriptName;
            qWarning() << queries.at(i);
            qWarning() << sqlQuery.lastError().text();
            return false;
        }
    }

    return true;
}

int QMDatabaseUpdater::getMajorFromScriptName(const QString &scriptName)
{
    auto tmp = scriptName.split("_");
    bool ok;
    int tmpMajor = tmp.at(1).toInt(&ok);

    if (ok)
    {
        return tmpMajor;
    }
    else
    {
        return -1;
    }
}

int QMDatabaseUpdater::getMinorFromScriptName(const QString &scriptName)
{
    auto tmp = scriptName.split("_");
    if (tmp.size() < 3)
    {
        return -1;
    }

    tmp = tmp.at(2).split(".");
    if (tmp.size() < 1)
    {
        return -1;
    }

    bool ok;
    int tmpMinor = tmp.at(0).toInt(&ok);

    if (ok)
    {
        return tmpMinor;
    }
    else
    {
        return -1;
    }
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
        "WHERE name == \"MINOR\" OR name == \"MAJOR\"";

    if (!query.exec(queryText))
    {
        return;
    }

    majorSource = -1;
    minorSource = -1;

    while (query.next())
    {
        if (query.value("name").toString() == "MAJOR")
        {
            majorSource = query.value("value").toInt();
            continue;
        }

        if (query.value("name").toString() == "MINOR")
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
