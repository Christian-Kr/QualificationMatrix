//
// qmtrainingmodel.h is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify it under the terms of
// the GNU General Public License as published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
// the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with QualificationMatrix.
// If not, see <http://www.gnu.org/licenses/>.
//

#ifndef QMTRAININGMODEL_H
#define QMTRAININGMODEL_H

#include <QSqlRelationalTableModel>

/// Employee training table in sql.
/// \author Christian Kr, Copyright 2020
class QMTrainingModel: public QSqlRelationalTableModel
{
Q_OBJECT

public:
    /// Constructor - Override from QSqlRelationalTableModel.
    /// \param parent
    /// \param db
    explicit QMTrainingModel(QObject *parent = nullptr, QSqlDatabase db = QSqlDatabase());
};

#endif // QMTRAININGMODEL_H
