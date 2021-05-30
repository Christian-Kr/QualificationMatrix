// qmemployeemodel.h is part of QualificationMatrix
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

#ifndef QMEMPLOYEEMODEL_H
#define QMEMPLOYEEMODEL_H

#include "framework/qmsqltablemodel.h"

/// Employee table in sql.
/// \author Christian Kr, Copyright 2020
class QMEmployeeModel: public QMSqlTableModel
{
    Q_OBJECT

public:
    /// Constructor
    /// \param parent Parent object for qt system.
    /// \param db The database object to work with.
    explicit QMEmployeeModel(QObject *parent = nullptr, const QSqlDatabase &db = QSqlDatabase());

    /// Override from QSqlRelationTableModel
    /// \param index
    /// \param role
    /// \return
    QVariant data(const QModelIndex &index, int role) const override;

    /// Override from QSqlRelationalTableModel
    /// \param index
    /// \return
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /// Override from QMSqlTableModel.
    void initModel() override;
};

#endif // QMEMPLOYEEMODEL_H
