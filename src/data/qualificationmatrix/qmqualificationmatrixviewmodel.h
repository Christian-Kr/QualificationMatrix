// qmqualificationmatrixviewmodel.h is part of QualificationMatrix
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

#ifndef QMQUALIFICATIONMATRIXVIEWMODEL_H
#define QMQUALIFICATIONMATRIXVIEWMODEL_H

#include "framework/qmsqltablemodel.h"

/// Qualification matrix data view table in sql.
/// \author Christian Kr, Copyright (c) 2021
class QMQualificationMatrixViewModel: public QMSqlTableModel
{
    Q_OBJECT

public:
    /// Constructor - Override from QSqlRelationalTableModel.
    /// \param parent
    /// \param db
    explicit QMQualificationMatrixViewModel(QObject *parent = nullptr, const QSqlDatabase &db = QSqlDatabase());

    /// Override from QMSqlTableModel.
    void initModel();
};

#endif // QMQUALIFICATIONMATRIXVIEWMODEL_H
