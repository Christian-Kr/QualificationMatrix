// qmamsgroupmodel.cpp is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along
// with QualificationMatrix. If not, see <http://www.gnu.org/licenses/>.

#include "qmamsgroupmodel.h"

#include <QColor>

QMAMSGroupModel::QMAMSGroupModel(QObject *parent, const QSqlDatabase &db)
    : QMSqlTableModel(parent, db)
{
    initModel();
}

void QMAMSGroupModel::initModel()
{
    // The name of the Table.
    setTable("AMSGroup");

    // The edit and join mode/strategy.
    setJoinMode(QSqlRelationalTableModel::LeftJoin);
    setEditStrategy(QSqlTableModel::OnManualSubmit);

    // Specifiy header data of table.
    setHeaderData(1, Qt::Horizontal, tr("Name"));
    setHeaderData(2, Qt::Horizontal, tr("Aktiviert"));
}

QVariant QMAMSGroupModel::data(const QModelIndex &index, int role) const
{
    bool activated = QMSqlTableModel::data(this->index(index.row(), 2), Qt::DisplayRole).toBool();
    if (!activated)
    {
        if (role == Qt::BackgroundRole)
        {
            return QVariant(QColor("#F8E6E0"));
        }
    }

    return QMSqlTableModel::data(index, role);
}
