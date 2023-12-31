// qmdatamanager.cpp is part of QualificationMatrix
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

#include "qmdatamanager.h"
#include "data/trainingdata/qmtraindatacertificateviewmodel.h"

#include <QSqlQuery>
#include <QSqlError>

QMDataManager *QMDataManager::instance = nullptr;

QMDataManager::QMDataManager()
    : QObject()
{
    certLoc = CertLoc::EXTERNAL;
}

int QMDataManager::testVersion(QSqlDatabase &db)
{
    QSqlQuery query(db);
    QString queryText =
        "SELECT name, value FROM Info "
        "WHERE name == \"MINOR\" OR name == \"MAJOR\"";

    if (!query.exec(queryText))
    {
        return -2;
    }

    int major = -1;
    int minor = -1;

    while (query.next())
    {
        if (query.value("name").toString() == "MAJOR")
        {
            major = query.value("value").toInt();
            continue;
        }

        if (query.value("name").toString() == "MINOR")
        {
            minor = query.value("value").toInt();
            continue;
        }

        if (minor != -1 && major != -1)
        {
            break;
        }
    }

    // Test if version numbers are equal.
    if (major == QMDataManager::getMajor() && minor == QMDataManager::getMinor())
    {
        return 0;
    }

    // Test if database version is smaller than the software requires.
    if (major < QMDataManager::getMajor() ||
        (major == QMDataManager::getMajor() && minor < QMDataManager::getMinor()))
    {
        return -1;
    }

    // Test if database version is higher than the software requires.
    if (major > QMDataManager::getMajor() ||
        (major == QMDataManager::getMajor() && minor > QMDataManager::getMajor()))
    {
        return 1;
    }

    return -2;
}

bool QMDataManager::testTableStructure(QSqlDatabase &db)
{
    return true;
}

bool QMDataManager::readCertificateLocation(const QSqlDatabase &db)
{
    QSqlQuery query(db);
    QString queryText =
        "SELECT name, value FROM Info "
        "WHERE name == \"certificate_location\"";

    if (!query.exec(queryText))
    {
        qWarning() << "cannot execute query";
        qWarning() << query.lastError().text();
        return false;
    }

    auto found = false;
    while (query.next())
    {
        if (query.value("name").toString() == "certificate_location")
        {
            found = true;
            auto value = query.value("value").toString();

            if (value == "internal")
            {
                certLoc = CertLoc::INTERNAL;
            }
            else if (value == "external")
            {
                certLoc = CertLoc::EXTERNAL;
            }
            else
            {
                qWarning() << "wrong value for certificate location setting in table";
                return false;
            }

            break;
        }
    }

    if (!found)
    {
        qWarning() << "cannot find certificate location setting";
        return false;
    }

    return false;
}

bool QMDataManager::readCertificateLocationPath(const QSqlDatabase &db)
{
    QSqlQuery query(db);
    QString queryText =
        "SELECT name, value FROM Info "
        "WHERE name == \"certificate_location\"";

    if (!query.exec(queryText))
    {
        qWarning() << "cannot execute query";
        qWarning() << query.lastError().text();
        return false;
    }

    auto found = false;
    while (query.next())
    {
        if (query.value("name").toString() == "certificate_location")
        {
            found = true;
            auto value = query.value("value").toString();
            certLocPath = value;

            break;
        }
    }

    if (!found)
    {
        qWarning() << "cannot find certificate location path setting";
        return false;
    }

    return false;
}
