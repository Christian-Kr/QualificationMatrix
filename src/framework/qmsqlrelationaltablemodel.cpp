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

#include <QDebug>

QMSqlRelationalTableModel::QMSqlRelationalTableModel(QObject *parent, QSqlDatabase db)
    : QSqlRelationalTableModel(parent, db)
{
    // Fetching all sub data exceeding 255 should be true by default. This makes sure, that
    // relations work with big data sets.
    doFetchAllSub = true;

    // Fetching all data exceeding 255 can be true by default to make model work fully.
    doFetchAll = true;
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
    // Before gettin any data be sure that sub model have fetched all data.
    fetchAllSub();

    return QSqlRelationalTableModel::setData(index, value, role);
}