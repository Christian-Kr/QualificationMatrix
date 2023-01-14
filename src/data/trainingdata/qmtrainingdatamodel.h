// qmtrainingdatamodel.h is part of QualificationMatrix
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

#ifndef QMTRAININGDATAMODEL_H
#define QMTRAININGDATAMODEL_H

#include "framework/qmsqltablemodel.h"

/// Employee training data table in sql.
/// \author Christian Kr, Copyright 2020
class QMTrainingDataModel: public QMSqlTableModel
{
    Q_OBJECT

public:
    /// Constructor - Override from QSqlRelationalTableModel.
    /// \param parent
    /// \param db
    explicit QMTrainingDataModel(QObject *parent = nullptr, const QSqlDatabase &db = QSqlDatabase());

    /// Override from QMSqlTableModel.
    void initModel() override;

    /// Override from QMSqlTableModel.
    void setFilter(const QString &filter) override;

    /// Get the id of the record in a single query.
    /// \param record the sql record without relation to other tables (relations columns will have ids)
    /// \returns the id of the first entry that has been found, else -1
    int getIdOfRecord(const QSqlRecord &record);

    /// Set the training data set by its id.
    /// \param trainDataId the id to update the values for
    /// \param record the record to get the values for update
    /// \returns true if success, else false
    bool updateById(int trainDataId, const QSqlRecord &record);
};

#endif // QMTRAININGDATAMODEL_H
