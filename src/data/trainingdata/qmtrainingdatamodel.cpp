// qmtrainingdatamodel.cpp is part of QualificationMatrix
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

#include "qmtrainingdatamodel.h"
#include "ams/qmamsmanager.h"

#include <QSqlRecord>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlError>

#include <QDebug>

QMTrainingDataModel::QMTrainingDataModel(QObject *parent, const QSqlDatabase &db)
    : QMSqlTableModel(parent, db, false, true)
{
    initModel();
}

void QMTrainingDataModel::initModel()
{
    // The name of the Table.
    setTable("TrainData");

    // The edit and join mode/strategy.
    setJoinMode(QSqlRelationalTableModel::LeftJoin);
    setEditStrategy(QSqlTableModel::OnFieldChange);

    // Specifiy header data of table.
    setHeaderData(1, Qt::Horizontal, tr("Mitarbeiter"));
    setRelation(1, QSqlRelation("Employee", "id", "name"));
    setHeaderData(2, Qt::Horizontal, tr("Schulung"));
    setRelation(2, QSqlRelation("Train", "id", "name"));
    setHeaderData(3, Qt::Horizontal, tr("Schulungsdatum"));
    setHeaderData(4, Qt::Horizontal, tr("Status"));
    setRelation(4, QSqlRelation("TrainDataState", "id", "name"));
    setHeaderData(5, Qt::Horizontal, tr("Datei"));
    setRelation(5, QSqlRelation("Files", "id", "name"));

    // Default sort.
    setSort(3, Qt::DescendingOrder);

    // Set Filter for initial change.
    setFilter("");
}

void QMTrainingDataModel::setFilter(const QString &filter)
{
    auto amsManager = QMAMSManager::getInstance();
    QStringList primaryKeysString;
    QList<int> primaryKeyInt = amsManager->getEmployeePrimaryKeys();
    for (int i = 0; i < primaryKeyInt.count(); i++)
    {
        if (i != 0)
        {
            primaryKeysString << ",";
        }

        primaryKeysString << QString("%1").arg(primaryKeyInt.at(i));
    }
    auto employeeFilter = QString("%1.id IN (%2)").arg("relTblAl_1", primaryKeysString.join(""));

    if (filter.isEmpty())
    {
        QMSqlTableModel::setFilter(employeeFilter);
    }
    else
    {
        QMSqlTableModel::setFilter(QString("%1 AND %2").arg(filter, employeeFilter));
    }
}

int QMTrainingDataModel::getIdOfRecord(const QSqlRecord &record)
{
    // get variables
    auto employee = (record.contains("employee")) ? record.value("employee").toInt() : -1;
    auto train = (record.contains("train")) ? record.value("train").toInt() : -1;
    auto state = (record.contains("state")) ? record.value("state").toInt() : -1;
    auto date = (record.contains("date")) ? record.value("date").toString() : QString();

    // if no variable has been defined, just exit
    if (employee == -1 && train == -1 && state == -1 && date.isEmpty())
    {
        return -1;
    }

    // build query string
    QString qry =
            "SELECT * FROM TrainData "
            "WHERE "
            + ((employee != -1) ? QString("employee=%1 AND ").arg(employee) : QString())
            + ((train != -1) ? QString("train=%1 AND ").arg(train) : QString())
            + ((state != -1) ? QString("state=%1 AND ").arg(state) : QString())
            + ((!date.isEmpty()) ? QString("date='%1' AND ").arg(date) : QString());

    qry.chop(5);
    qry.append(";");

    // run query
    QSqlQuery sqlQuery(database());
    if (!sqlQuery.exec(qry))
    {
        qDebug() << "QMTrainingDataModel::getIdOfRecord: " << sqlQuery.lastError().text();
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
    auto id = sqlQuery.value("id").toInt();
    sqlQuery.finish();

    return id;
}


bool QMTrainingDataModel::updateById(int trainDataId, const QSqlRecord &record)
{
    Q_ASSERT(trainDataId > 1);

    // get variables
    auto employee = (record.contains("employee")) ? record.value("employee").toInt() : -1;
    auto train = (record.contains("train")) ? record.value("train").toInt() : -1;
    auto state = (record.contains("state")) ? record.value("state").toInt() : -1;
    auto date = (record.contains("date")) ? record.value("date").toString() : QString();

    // if no variable has been defined, just exit
    if (employee == -1 && train == -1 && state == -1 && date.isEmpty())
    {
        return false;
    }

    // build query string
    QString qry =
            "UPDATE TrainData "
            "SET "
            + ((employee != -1) ? QString("employee=%1, ").arg(employee) : QString())
            + ((train != -1) ? QString("train=%1, ").arg(train) : QString())
            + ((state != -1) ? QString("state=%1, ").arg(state) : QString())
            + ((!date.isEmpty()) ? QString("date='%1', ").arg(date) : QString());
    qry.chop(2);
    qry += " WHERE"
           "    id=" + QString::number(trainDataId) + ";";

    // run query
    QSqlQuery sqlQuery(database());
    if (!sqlQuery.exec(qry))
    {
        qDebug() << "QMTrainingDataModel::updateById: " << sqlQuery.lastError().text();
        return false;
    }

    return true;
}
