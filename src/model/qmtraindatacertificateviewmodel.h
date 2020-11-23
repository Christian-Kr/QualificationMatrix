//
// qmtraindatacertificateviewmodel.h is part of QualificationMatrix
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

#ifndef QMTRAINDATACERTIFICATEVIEWMODEL_H
#define QMTRAINDATACERTIFICATEVIEWMODEL_H

#include <QSqlTableModel>

/// Table for correlation of train data and certificates. This allow flexible handle of the nubmer
/// of certificates for every train data entry.
/// \author Christian Kr, Copyright 2020
class QMTrainDataCertificateViewModel: public QSqlTableModel
{
Q_OBJECT

public:
    /// Constructor - Override from QSqlTableModel.
    /// \param parent
    /// \param db
    explicit QMTrainDataCertificateViewModel(
        QObject *parent = nullptr, QSqlDatabase db = QSqlDatabase());
};

#endif // QMTRAINDATACERTIFICATEVIEWMODEL_H
