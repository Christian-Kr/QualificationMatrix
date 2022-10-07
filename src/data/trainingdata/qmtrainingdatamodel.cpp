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
