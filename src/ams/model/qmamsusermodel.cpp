// qmamsusermodel.cpp is part of QualificationMatrix
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

#include "qmamsusermodel.h"

#include <QColor>

QMAMSUserModel::QMAMSUserModel(QObject *parent, const QSqlDatabase &db)
    : QMSqlTableModel(parent, db)
{
    initModel();
}

void QMAMSUserModel::initModel()
{
    // The name of the Table.
    setTable("AMSUser");

    // The edit and join mode/strategy.
    setJoinMode(QSqlRelationalTableModel::LeftJoin);
    setEditStrategy(QSqlTableModel::OnManualSubmit);

    // Specifiy header data of table.
    setHeaderData(1, Qt::Horizontal, tr("Name"));
    setHeaderData(2, Qt::Horizontal, tr("Benutzername"));
    setHeaderData(3, Qt::Horizontal, tr("Passwort"));
    setHeaderData(4, Qt::Horizontal, tr("Letzter Login"));
    setHeaderData(5, Qt::Horizontal, tr("Loginversuche"));
    setHeaderData(6, Qt::Horizontal, tr("Aktiv"));
}

QVariant QMAMSUserModel::data(const QModelIndex &index, int role) const
{
    bool activated = QMSqlTableModel::data(this->index(index.row(), 6), Qt::DisplayRole).toBool();
    if (!activated)
    {
        if (role == Qt::BackgroundRole)
        {
            return QVariant(QColor("#F8E6E0"));
        }
    }

    return QMSqlTableModel::data(index, role);
}

Qt::ItemFlags QMAMSUserModel::flags(const QModelIndex &index) const
{
    if (index.column() == fieldIndex("last_login") ||
        index.column() == fieldIndex("unsuccess_login_num"))
    {
        return Qt::ItemIsEditable | Qt::ItemIsSelectable;
    }

    return QSqlRelationalTableModel::flags(index);
}
