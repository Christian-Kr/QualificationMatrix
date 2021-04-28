// qmemployeeviewmodel.cpp is part of QualificationMatrix
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

#include "qmemployeeviewmodel.h"

QMEmployeeViewModel::QMEmployeeViewModel(QObject *parent, const QSqlDatabase &db)
    : QMSqlRelationalTableModel(parent, db)
{
    // The name of the Table.
    QSqlRelationalTableModel::setTable("EmployeeView");

    // The edit and join mode/strategy.
    QSqlRelationalTableModel::setJoinMode(QSqlRelationalTableModel::LeftJoin);
    QSqlRelationalTableModel::setEditStrategy(QSqlTableModel::OnManualSubmit);

    // Specifiy header data of table.
    QSqlRelationalTableModel::setHeaderData(1, Qt::Horizontal, tr("Name"));
    QSqlRelationalTableModel::setHeaderData(2, Qt::Horizontal, tr("Gruppe"));
    QSqlRelationalTableModel::setRelation(2, QSqlRelation("Shift", "id", "name"));

    sort(1, Qt::AscendingOrder);
}
