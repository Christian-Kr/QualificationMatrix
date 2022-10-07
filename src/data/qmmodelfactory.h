// qmmodelfactory.h is part of QualificationMatrix
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

#ifndef QMMODELFACTORY_H
#define QMMODELFACTORY_H

#include <QObject>

/// Create models by their definition.
/// \author Christian Kr, Copyright 2022
class QMModelFactory: public QObject
{
    Q_OBJECT

private:
    /// Constructor
    /// \param parent the parent object
    explicit QMModelFactory(QObject *parent = nullptr);

    /// Destructor
    ~QMModelFactory() override = default;
};

#endif // QMMODELFACTORY_H
