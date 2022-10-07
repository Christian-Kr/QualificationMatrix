// qmtraindatacertificatemodel.cpp is part of QualificationMatrix
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

#include "qmtraindatacertificateviewmodel.h"

QMTrainDataCertificateViewModel::QMTrainDataCertificateViewModel(QObject *parent, QSqlDatabase db)
    : QMSqlTableModel(parent, db)
{
    // The name of the Table.
    setTable("TrainDataCertificateView");

    // The edit and join mode/strategy.
    setEditStrategy(QSqlTableModel::OnManualSubmit);

    // Specifiy header data of table.
    setHeaderData(1, Qt::Horizontal, tr("Schulungsnummer"));
    setHeaderData(2, Qt::Horizontal, tr("Nachweisnummer"));
    setHeaderData(3, Qt::Horizontal, tr("Nachweis"));
    setHeaderData(4, Qt::Horizontal, tr("Hash"));
}
