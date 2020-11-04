//
// csqlrelationaltablemodel.cpp is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with QualificationMatrix.  If not, see <http://www.gnu.org/licenses/>.
//

#include "qmsqlrelationaltablemodel.h"

#include <QDebug>

QMSqlRelationalTableModel::QMSqlRelationalTableModel(QObject *parent, QSqlDatabase db)
    : QSqlRelationalTableModel(parent, db)
{}

bool QMSqlRelationalTableModel::select()
{
    emit afterSelect();

    auto i = 0;
    auto res = QSqlRelationalTableModel::select();

    while (canFetchMore())
    {
        i++;
        fetchMore();

        // Informate listener.
        emit nextSelect(i);
    }

    emit afterSelect();
    return res;
}
