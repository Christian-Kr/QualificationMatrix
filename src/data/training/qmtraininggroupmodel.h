// qmtraininggroupmodel.h is part of QualificationMatrix
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

#ifndef QMTRAININGGROUPMODEL_H
#define QMTRAININGGROUPMODEL_H

#include "framework/qmsqltablemodel.h"

/// Employee training group table in sql.
/// \author Christian Kr, Copyright 2020
class QMTrainingGroupModel: public QMSqlTableModel
{
    Q_OBJECT

public:
    /// Constructor - Override from QSqlTableModel.
    /// \param parent
    /// \param db
    explicit QMTrainingGroupModel(QObject *parent = nullptr, QSqlDatabase db = QSqlDatabase());

    /// Override from QMSqlTableModel.
    void initModel() override;
};

#endif // QMTRAININGGROUPMODEL_H
