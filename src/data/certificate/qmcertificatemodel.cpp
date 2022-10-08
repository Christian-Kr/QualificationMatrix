// qmcertificatemodel.cpp is part of QualificationMatrix
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

#include "qmcertificatemodel.h"

QMCertificateModel::QMCertificateModel(QObject *parent, QSqlDatabase db)
    : QMSqlTableModel(parent, db)
{
    // The name of the Table.
    setTable("Certificate");

    // The edit and join mode/strategy.
    setEditStrategy(QSqlTableModel::OnManualSubmit);

    // Specifiy header data of table.
    setHeaderData(1, Qt::Horizontal, tr("Name"));
    setHeaderData(2, Qt::Horizontal, tr("Typ"));
    setHeaderData(3, Qt::Horizontal, tr("Pfad"));
    setHeaderData(6, Qt::Horizontal, tr("Hinzugefügt"));

    // Most of the time people add a new certificate and want to use it directly. So sorting by add date rather than
    // training might be a good choice for the default setup.
    setSort(6, Qt::DescendingOrder);

    setHeaderData(7, Qt::Horizontal, tr("Schulungstag"));
}