// qmemployeegroupviewmodel.h is part of QualificationMatrix
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

#ifndef QMEMPLOYEEGROUPVIEWMODEL_H
#define QMEMPLOYEEGROUPVIEWMODEL_H

#include "framework/qmsqltablemodel.h"

/// Employee group table in sql.
/// \author Christian Kr, Copyright 2021
class QMEmployeeGroupViewModel: public QMSqlTableModel
{
    Q_OBJECT

public:
    /// Constructor - Override from QSqlTableModel.
    /// \param parent
    /// \param db
    explicit QMEmployeeGroupViewModel(QObject *parent = nullptr, QSqlDatabase db = QSqlDatabase());

    /// Initialize data with settings.
    void initModel();
};

#endif // QMEMPLOYEEGROUPVIEWMODEL_H
