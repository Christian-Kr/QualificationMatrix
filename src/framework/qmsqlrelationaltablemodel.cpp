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

#include "framework/qmsqlrelationaltablemodel.h"
#include "model/qmdatamanager.h"

#include <QDebug>
#include <QSqlRelationalTableModel>

QMSqlRelationalTableModel::QMSqlRelationalTableModel(QObject *parent, QSqlDatabase db, bool doFetchAll, bool
    doFetchAllSub)
    : QSqlRelationalTableModel(parent, db)
{
    // Fetching all sub data exceeding 255 should be true by default. This makes sure, that
    // relations work with big data sets.
    this->doFetchAllSub = doFetchAllSub;

    // Fetching all data exceeding 255 can be true by default to make model work fully.
    this->doFetchAll = doFetchAll;
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

QVariant QMSqlRelationalTableModel::data(const QModelIndex &index, int role) const
{
    // Before getting a value, look if it is a relational model. If so: Update all related table models.
    QSqlTableModel *tableModel = relationModel(index.column());
    if (tableModel != nullptr && doFetchAllSub)
    {
        // Fetch all rows from related table model.
        while (tableModel->canFetchMore())
        {
            tableModel->fetchMore();
        }
    }

    return QSqlRelationalTableModel::data(index, role);
}

bool QMSqlRelationalTableModel::setData(
        const QModelIndex &index, const QVariant &value, int role)
{
    // Before setting a value, look if it is a relational model. If so: Update all related table models.
    QSqlTableModel *tableModel = relationModel(index.column());
    if (tableModel != nullptr && doFetchAllSub)
    {
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
