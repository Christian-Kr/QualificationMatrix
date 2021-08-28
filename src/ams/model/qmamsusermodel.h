// qmamsusermodel.h is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
// or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along
// with QualificationMatrix. If not, see <http://www.gnu.org/licenses/>.

#ifndef QMAMSUSERMODEL_H
#define QMAMSUSERMODEL_H

#include "framework/qmsqltablemodel.h"

/// User table for the access management system.
/// \author Christian Kr, Copyright 2020
class QMAMSUserModel: public QMSqlTableModel
{
    Q_OBJECT

public:
    /// Constructor
    /// \param parent Parent object for qt system.
    /// \param db The database object to work with.
    explicit QMAMSUserModel(QObject *parent = nullptr,
            const QSqlDatabase &db = QSqlDatabase());

    /// Override from QSqlRelationTableModel
    /// \param index
    /// \param role
    /// \return
    QVariant data(const QModelIndex &index,
            int role = Qt::DisplayRole) const override;

    /// Override from QSqlRelationalTableModel
    /// \param index
    /// \return
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /// Override from QMSqlTableModel.
    void initModel() override;
};

#endif // QMAMSUSERMODEL_H
