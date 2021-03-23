//
// qmsqlrelationaltablemodel.cpp is part of QualificationMatrix
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

#include "qmsqlrelationaltablemodel.h"
#include "model/qmdatamanager.h"

#include <QDebug>

QMSqlRelationalTableModel::QMSqlRelationalTableModel(QObject *parent, QSqlDatabase db,
    bool doFetchAll, bool doFetchAllSub)
    : QSqlRelationalTableModel(parent, db)
{
    // Fetching all sub data exceeding 255 should be true by default. This makes sure, that
    // relations work with big data sets.
    this->doFetchAllSub = doFetchAllSub;

    // Fetching all data exceeding 255 can be true by default to make model work fully.
    this->doFetchAll = doFetchAll;

    // When creating a new object. Build a connection to the datamanager, to be informed of changes
    // in other models. A derived class can then decide whether a new selection should be done
    // or not, based on the sending object. The connection will be destroyed, when the object will
    // be deleted.
    auto dm = QMDataManager::getInstance();
    connect(dm, &QMDataManager::modelChanged, this, &QMSqlRelationalTableModel::otherModelChanged);
}

bool QMSqlRelationalTableModel::select()
{
    emit beforeSelect();

    auto i = 0;
    auto res = QSqlRelationalTableModel::select();

    while (doFetchAll && canFetchMore())
    {
        i++;
        fetchMore();

        // Informate listener.
        emit nextSelect(i);
    }

    emit afterSelect();
    return res;
}

void QMSqlRelationalTableModel::fetchAllSub() const
{
    for (int i = 0; i < columnCount(); i++)
    {
        QSqlTableModel *subModel = relationModel(i);
        if (subModel == nullptr)
        {
            continue;
        }

        while(subModel->canFetchMore())
        {
            subModel->fetchMore();
        }
    }
}

QVariant QMSqlRelationalTableModel::data(const QModelIndex &index, int role) const
{
    // Before gettin any data be sure that sub model have fetched all data.
    if (doFetchAllSub)
    {
        fetchAllSub();
    }

    return QSqlRelationalTableModel::data(index, role);
}

bool QMSqlRelationalTableModel::setData(
        const QModelIndex &index, const QVariant &value, int role)
{
    // Before setting a value, look if it is a relational model. If so: Update all related table
    // models.
    QSqlTableModel *tableModel = relationModel(index.column());
    if (tableModel != nullptr)
    {
        tableModel->select();

        // Fetch all rows from related table model.
        while (tableModel->canFetchMore())
        {
            tableModel->fetchMore();
        }
    }

    // Set data of the entry.
    bool res = QSqlRelationalTableModel::setData(index, value, role);

    // If change has been done, informate other models about the change.
    if (res)
    {
        auto dm = QMDataManager::getInstance();
        dm->sendModelChangedInformation(this);
    }

    return res;
}