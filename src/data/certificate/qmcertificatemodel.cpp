// qmcertificatemodel.cpp is part of QualificationMatrix
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

#include "qmcertificatemodel.h"

#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlError>

QMCertificateModel::QMCertificateModel(QObject *parent, QSqlDatabase db)
    : QMSqlTableModel(parent, db)
{
    // The name of the Table.
    setTable("Certificate");

    // The edit and join mode/strategy.
    setEditStrategy(QSqlTableModel::OnManualSubmit);

    // Specifiy header data of table.
    setHeaderData(1, Qt::Horizontal, tr("Name"));
    setHeaderData(2, Qt::Horizontal, tr("Typ"));
    setHeaderData(3, Qt::Horizontal, tr("Pfad"));
    setHeaderData(6, Qt::Horizontal, tr("Hinzugefügt"));

    // Most of the time people add a new certificate and want to use it directly. So sorting by add date rather than
    // training might be a good choice for the default setup.
    setSort(6, Qt::DescendingOrder);

    setHeaderData(7, Qt::Horizontal, tr("Schulungstag"));
}

int QMCertificateModel::getIdOfRecord(const QSqlRecord &record)
{
    // get variables
    auto name = (record.contains("name")) ? record.value("name").toString() : QString();
    auto type = (record.contains("type")) ? record.value("type").toString() : QString();
    auto addDate = (record.contains("add_date")) ? record.value("add_date").toString() : QString();
    auto trainDate = (record.contains("train_date")) ? record.value("train_date").toString() : QString();
    auto md5Hash = (record.contains("md5_hash")) ? record.value("md5_hash").toString() : QString();

    // if no variable has been defined, just exit
    if (name.isEmpty() && type.isEmpty() && addDate.isEmpty() && trainDate.isEmpty() && md5Hash.isEmpty())
    {
        return -1;
    }

    // build query string
    QString qry =
            "SELECT * FROM Certificate "
            "WHERE "
            + ((!name.isEmpty()) ? QString("name='%1' AND ").arg(name) : QString())
            + ((!type.isEmpty()) ? QString("type='%1' AND ").arg(type) : QString())
            + ((!addDate.isEmpty()) ? QString("add_date='%1' AND ").arg(addDate) : QString())
            + ((!trainDate.isEmpty()) ? QString("train_date='%1' AND ").arg(trainDate) : QString())
            + ((!md5Hash.isEmpty()) ? QString("md5_hash='%1' AND ").arg(md5Hash) : QString());

    qry.chop(5);
    qry.append(";");

    // run query
    QSqlQuery sqlQuery(database());
    if (!sqlQuery.exec(qry))
    {
        qDebug() << "QMCertificateModel::getIdOfRecord: " << sqlQuery.lastError().text();
        return -1;
    }

    // get query result
    if (!sqlQuery.first())
    {
        // no result
        sqlQuery.finish();
        return -1;
    }

    // just get the first value, which should be a number
    auto id = sqlQuery.value(1).toInt();
    sqlQuery.finish();

    return id;
}
