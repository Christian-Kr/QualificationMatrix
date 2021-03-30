// qmfunctionmodel.h is part of QualificationMatrix
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

#ifndef QMFUNCTIONMODEL_H
#define QMFUNCTIONMODEL_H

#include "framework/qmsqlrelationaltablemodel.h"

/// Sql model for the function table.
/// \author Christian Kr, Copyright 2020
class QMFunctionModel: public QMSqlRelationalTableModel
{
Q_OBJECT

public:
    /// Constructor
    /// \param parent Parent object for the qt system.
    /// \param db The database to work with.
    explicit QMFunctionModel(QObject *parent = nullptr, const QSqlDatabase &db = QSqlDatabase());

    /// Override from QMSqlRelationalTableModel.
    void initModel() override;
};

#endif // QMFUNCTIONMODEL_H
