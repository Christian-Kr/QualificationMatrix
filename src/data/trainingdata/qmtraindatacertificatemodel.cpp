// qmtraindatacertificatemodel.cpp is part of QualificationMatrix
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

#include "qmtraindatacertificatemodel.h"

#include <QSqlQuery>
#include <QSqlRecord>

QMTrainDataCertificateModel::QMTrainDataCertificateModel(QObject *parent, QSqlDatabase db)
    : QMSqlTableModel(parent, db)
{
    // The name of the Table.
    setTable("TrainDataCertificate");

    // The edit and join mode/strategy.
    setEditStrategy(QSqlTableModel::OnManualSubmit);

    // Specifiy header data of table.
    setHeaderData(1, Qt::Horizontal, tr("Schulung"));
    setHeaderData(2, Qt::Horizontal, tr("Nachweis"));
}

int QMTrainDataCertificateModel::getIdOfTrainData(int trainDataId)
{
    Q_ASSERT(trainDataId < 1);

    // build query string
    QString qry =
            "SELECT * FROM TrainDataCertificate "
            "WHERE "
            "    train_data=%1;";

    // run query
    QSqlQuery sqlQuery(database());
    if (!sqlQuery.exec(qry.arg(trainDataId)))
    {
        return -1;
    }

    // get query result
    if (!sqlQuery.first())
    {
        // no result - train data entry is not in table
        sqlQuery.finish();
        return -1;
    }

    // just get the first value, which should be a number
    auto id = sqlQuery.value(1).toInt();
    sqlQuery.finish();

    return id;
}

bool QMTrainDataCertificateModel::addRow(int trainDataId, int certId)
{
    QSqlRecord newRecord = record();
    newRecord.setValue("train_data", trainDataId);
    newRecord.setValue("certificate", certId);

    if (!insertRecord(-1, newRecord))
    {
        qDebug() << "QMTrainDataCertificateModel::addRow: Cannot insert new record";
        return false;
    }

    if (!submitAll())
    {
        qDebug() << "QMTrainDataCertificateModel::addRow: Cannot submit all";
        return false;
    }

    return true;
}
