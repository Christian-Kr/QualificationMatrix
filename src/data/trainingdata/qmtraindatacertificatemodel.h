// qmtraindatacertificatemodel.h is part of QualificationMatrix
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

#ifndef QMTRAINDATACERTIFICATEMODEL_H
#define QMTRAINDATACERTIFICATEMODEL_H

#include "framework/qmsqltablemodel.h"

/// Table for correlation of train data and certificates. This allow flexible handle of the nubmer
/// of certificates for every train data entry.
/// \author Christian Kr, Copyright 2020
class QMTrainDataCertificateModel: public QMSqlTableModel
{
    Q_OBJECT

public:
    /// Constructor - Override from QSqlTableModel.
    /// \param parent
    /// \param db
    explicit QMTrainDataCertificateModel(QObject *parent = nullptr, QSqlDatabase db = QSqlDatabase());

    /// Get the id of the first entry with the given train data id.
    /// \param trainDataId the id of the train data to search for
    /// \returns the id of the first entry that has the train data id, else -1
    int getIdOfTrainData(int trainDataId);

    /// Add a new entry.
    /// \param trainDataId id of the train data entry
    /// \param certId id of the certificate
    ///\returns true if success, else false
    bool addRow(int trainDataId, int certId);
};

#endif // QMTRAINDATACERTIFICATEMODEL_H
