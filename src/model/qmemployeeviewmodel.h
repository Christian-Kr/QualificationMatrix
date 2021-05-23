// qmemployeeviewmodel.h is part of QualificationMatrix
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

#ifndef QMEMPLOYEEVIEWMODEL_H
#define QMEMPLOYEEVIEWMODEL_H

#include "framework/qmsqlrelationaltablemodel.h"

/// Employee view table in sql.
/// \author Christian Kr, Copyright 2021
class QMEmployeeViewModel: public QMSqlRelationalTableModel
{
Q_OBJECT

public:
    /// Constructor
    /// \param parent Parent object for qt system.
    /// \param db The database object to work with.
    explicit QMEmployeeViewModel(QObject *parent = nullptr, const QSqlDatabase &db = QSqlDatabase());
};

#endif // QMEMPLOYEEVIEWMODEL_H
