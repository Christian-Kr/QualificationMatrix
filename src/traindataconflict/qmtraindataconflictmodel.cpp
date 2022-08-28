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
            "   Employee.name as employee_name, "
            "   TrainData.date as traindata_date, "
            "   TrainData.state as traindata_state "
            "FROM "
            "   TrainData, Employee, Train "
            "WHERE "
            "   TrainData.employee = Employee.id AND "
            "   TrainData.train = Train.id AND "
            "   traindata_id IN (" + strIds.join(",") + ")";

    QSqlQuery query(strTrainDataQuery, db);
    qDebug() << strTrainDataQuery;
    while (query.next())
    {
        QMTrainDataConflictEntry conflictEntry;
        conflictEntry.employeeId = query.record().value("employee_id").toInt();
        conflictEntry.employeeName = query.record().value("employee_name").toString();
        conflictEntry.trainDataId = query.record().value("traindata_id").toInt();
        conflictEntry.trainId = query.record().value("train_id").toInt();
        conflictEntry.trainName = query.record().value("train_name").toString();
        conflictEntry.trainDate = query.record().value("traindata_date").toDate();
        m_conflictEntries->append(conflictEntry);
    }

    // After finish the results are not accesible anymore.
    query.finish();

    beginInsertRows(QModelIndex(), 0, (int) m_conflictEntries->size());
    endInsertRows();

    return true;
}

QVariant QMTrainDataConflictModel::headerData(int section, Qt::Orientation orientation, int role) const
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
                return tr("Mitarbeiter");
            case 1:
                return tr("Schulung");
            case 2:
                return tr("Schulungsdatum");
            default:
                return {};
        }
    }
    else
    {
        return section + 1;
    }
}

bool QMTrainDataConflictModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (value != headerData(section, orientation, role))
    {
        // FIXME: Implement me!
        emit headerDataChanged(orientation, section, section);
        return true;
    }
    return false;
}

int QMTrainDataConflictModel::rowCount(const QModelIndex &) const
{
    return (int) m_conflictEntries->size();
}

int QMTrainDataConflictModel::columnCount(const QModelIndex &) const
{
    return 3;
}

QVariant QMTrainDataConflictModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }

    if (role == Qt::DisplayRole)
    {
        int row = index.row();
        switch (index.column())
        {
            case 0:
                return m_conflictEntries->at(row).employeeName;
            case 1:
                return m_conflictEntries->at(row).trainName;
            case 2:
                return m_conflictEntries->at(row).trainDate.toString(Qt::DateFormat::ISODate);
            default:
                return {};
        }
    }

    if (role == Qt::BackgroundRole)
    {
        // TODO:: Return background color.
    }

    return {};
}

bool QMTrainDataConflictModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return QAbstractTableModel::setData(index, value, role);
}

Qt::ItemFlags QMTrainDataConflictModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }

    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}
