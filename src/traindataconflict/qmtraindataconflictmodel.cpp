// qmtraindataconflictmodel.cpp is part of QualificationMatrix
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

#include "qmtraindataconflictmodel.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlResult>
#include <QSqlRecord>

QMTrainDataConflictModel::QMTrainDataConflictModel(QObject *parent)
    : QAbstractTableModel(parent)
    , m_conflictEntries(new QList<QMTrainDataConflictEntry>())
{}

QMTrainDataConflictModel::~QMTrainDataConflictModel()
{
    delete m_conflictEntries;
}

bool QMTrainDataConflictModel::updateFromTrainDataIds(const QList<int> &ids)
{
    m_conflictEntries->clear();

    // Convert integer list of ids to QStringList.
    QStringList strIds;
    for (auto id : ids)
    {
        strIds << QString::number(id);
    }

    auto db = QSqlDatabase::database("default");

    QString strTrainDataQuery =
            "SELECT "
            "   TrainData.id as traindata_id, "
            "   TrainData.train as train_id, "
            "   Train.name as train_name, "
            "   TrainData.employee as employee_id, "
            "   Employee.name as employee.name, "
            "   TrainData.date as traindata_date, "
            "   TrainData.state as traindata_state "
            "FROM "
            "   TrainData, Employee, Train "
            "WHERE "
            "   TrainData.employee = Employee.id AND "
            "   TrainData.train = Train.id AND "
            "   traindata_id IN (" + strIds.join(",") + ")";

    QSqlQuery query(strTrainDataQuery, db);

    while (query.next())
    {
        QMTrainDataConflictEntry conflictEntry;
        conflictEntry.employeeId = query.record().value("employee_id").toInt();
        conflictEntry.employeeName = query.record().value("employee_name").toString();
        conflictEntry.trainDataId = query.record().value("traindata_id").toInt();
        conflictEntry.trainId = query.record().value("train_id").toInt();
        conflictEntry.trainName = query.record().value("train_name").toString();
        m_conflictEntries->append(conflictEntry);
    }

    // After finish the results are not accesible anymore.
    query.finish();

    beginInsertRows(QModelIndex(), 0, (int) resultRecords->size() - 1);
    endInsertRows();

    return true;
}

QMQualiResultRecord* QMQualiResultModel::getQualiResultRecord(const int &num)
{
    if (num >= resultRecords->count() || num < 0)
    {
        return nullptr;
    }

    return resultRecords->at(num);
}

QVariant QMQualiResultModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return {};
    }

    if (orientation == Qt::Orientation::Horizontal)
    {
        switch (section)
        {
            case 0:
                return tr("Name");
            case 1:
                return tr("Funktion");
            case 2:
                return tr("Schulung");
            case 3:
                return tr("Intervall");
            case 4:
                return tr("Notwendigkeit");
            case 5:
                return tr("Letzte Schulung");
            case 6:
                return tr("Angemeldete Schulung");
            case 7:
                return tr("Schulungsstatus");
            case 8:
                return tr("Information");
        }
    }
    else
    {
        return section + 1;
    }

    return {};
}

bool QMQualiResultModel::setHeaderData(
    int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (value != headerData(section, orientation, role))
    {
        // FIXME: Implement me!
        emit headerDataChanged(orientation, section, section);
        return true;
    }
    return false;
}

int QMQualiResultModel::rowCount(const QModelIndex &) const
{
    return 0;
}

int QMQualiResultModel::columnCount(const QModelIndex &) const
{
    return 9;
}

QVariant QMQualiResultModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if (role == Qt::DisplayRole)
    {
        int row = index.row();
        switch (index.column())
        {
            case 0:
                return {};
            case 1:
                return {};
        }

        return resultRecords->at(index.row())->getFirstName();
    }

    if (role == Qt::BackgroundRole)
    {
        // TODO:: Return background color.
    }

    return QVariant();
}

bool QMQualiResultModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return QAbstractTableModel::setData(index, value, role);
}

Qt::ItemFlags QMQualiResultModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }

    return Qt::ItemIsSelectable | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}
