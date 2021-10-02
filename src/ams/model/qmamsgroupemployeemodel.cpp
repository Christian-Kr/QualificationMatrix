// qmamsgroupemployeemodel.cpp is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify it under the terms of the GNU General
// Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option)
// any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with QualificationMatrix. If not, see
// <http://www.gnu.org/licenses/>.

#include "qmamsgroupemployeemodel.h"

QMAMSGroupEmployeeModel::QMAMSGroupEmployeeModel(QObject *parent,
        const QSqlDatabase &db)
    : QMSqlTableModel(parent, db)
{
    initModel();
}

void QMAMSGroupEmployeeModel::initModel()
{
    // The name of the Table.
    setTable("AMSGroupEmployee");

    // The edit and join mode/strategy.
    setJoinMode(QSqlRelationalTableModel::LeftJoin);
    setEditStrategy(QSqlTableModel::OnManualSubmit);

    // Specifiy header data of table.
    setHeaderData(1, Qt::Horizontal, tr("Gruppe"));
    setRelation(1, QSqlRelation("AMSGroup", "amsgroup_id", "amsgroup_name"));
    setHeaderData(2, Qt::Horizontal, tr("Mitarbeiter"));
    setRelation(2, QSqlRelation("Employee", "id", "name"));
}
