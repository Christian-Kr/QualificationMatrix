// qmamsuseremployeeviewmodel.h is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify it under the terms
// of the GNU General Public License as published by the Free Software Foundation, either version
// 3 of the License, or (at your option) any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
// the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with QualificationMatrix.
// If not, see <http://www.gnu.org/licenses/>.

#ifndef QMAMSUSEREMPLOYEEVIEWMODEL_H
#define QMAMSUSEREMPLOYEEVIEWMODEL_H

#include "framework/qmsqltablemodel.h"

/// User view for the access management system, which show the correlation between employees and
/// user.
/// \author Christian Kr, Copyright 2023
class QMAMSUserEmployeeViewModel: public QMSqlTableModel
{
    Q_OBJECT

public:
    /// Constructor
    /// \param parent Parent object for qt system.
    /// \param db The database object to work with.
    explicit QMAMSUserEmployeeViewModel(
            QObject *parent = nullptr, const QSqlDatabase &db = QSqlDatabase());

    /// Override from QMSqlTableModel.
    void initModel() override;
};

#endif // QMAMSUSEREMPLOYEEVIEWMODEL_H
