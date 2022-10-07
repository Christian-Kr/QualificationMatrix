// qmsqlrelationaltablemodel.cpp is part of QualificationMatrix
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

#include "framework/qmsqltablemodel.h"
#include "model/qmdatamanager.h"

#include <QSqlRelationalTableModel>
#include <QSqlQuery>

QMSqlTableModel::QMSqlTableModel(QObject *parent, const QSqlDatabase &db, bool doFetchAll, bool doFetchAllSub)
    : QSqlRelationalTableModel(parent, db)
    , limit(0)
{
    // Fetching all sub data exceeding 255 should be true by default. This makes sure, that
    // relations work with big data sets.
    this->doFetchAllSub = doFetchAllSub;

    // Fetching all data exceeding 255 can be true by default to make model work fully.
    this->doFetchAll = doFetchAll;
}

bool QMSqlTableModel::select()
{
    auto res = QSqlRelationalTableModel::select();

    while (doFetchAll && canFetchMore())
    {
        fetchMore();
    }

    return res;
}

QVariant QMSqlTableModel::data(const QModelIndex &index, int role) const
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

bool QMSqlTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
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

    return res;
}

QString QMSqlTableModel::selectStatement() const
{
    QString query = QSqlRelationalTableModel::selectStatement();

    if (limit > 0)
    {
        query += QString(" LIMIT %1").arg(limit);
    }

    return query;
}

