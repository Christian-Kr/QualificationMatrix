//
// trainingmodel.h is part of QualificationMatrix
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

#ifndef TRAININGMODEL_H
#define TRAININGMODEL_H

#include <QSqlRelationalTableModel>

/**
 * @brief Employee training table in sql.
 * @author Christian Kr, Copyright (c) 2020
 */
class TrainingModel: public QSqlRelationalTableModel
{
Q_OBJECT

public:
    /**
     * @brief Constructor - Override from QSqlRelationalTableModel.
     */
    TrainingModel(QObject *parent = nullptr, QSqlDatabase db = QSqlDatabase());
};

#endif // TRAININGMODEL_H
