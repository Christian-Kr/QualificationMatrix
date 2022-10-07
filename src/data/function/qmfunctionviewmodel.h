// qmfunctionviewmodel.h is part of QualificationMatrix
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

#ifndef QMFUNCTIONVIEWMODEL_H
#define QMFUNCTIONVIEWMODEL_H

#include "framework/qmsqltablemodel.h"

/// Sql data for the function view table.
/// \author Christian Kr, Copyright 2021
class QMFunctionViewModel: public QMSqlTableModel
{
    Q_OBJECT

public:
    /// Constructor
    /// \param parent Parent object for the qt system.
    /// \param db The database to work with.
    explicit QMFunctionViewModel(QObject *parent = nullptr, const QSqlDatabase &db = QSqlDatabase());

    /// Initialize the data with its settings.
    void initModel();
};

#endif // QMFUNCTIONVIEWMODEL_H
