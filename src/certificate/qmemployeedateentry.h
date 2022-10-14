// qmemployeedateentry.h is part of QualificationMatrix
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

#ifndef QMEMPLOYEEDATEENTRY_H
#define QMEMPLOYEEDATEENTRY_H

#include <QObject>
#include <QDate>

/// Data for one entry. This includes the employee naame and the date when the training happen. This should be the
/// exact training date as written down on the signing list and not the planned date.
/// \author Christian Kr, Copyright 2022
struct QMEmployeeDateEntry
{
    int employeeId = -1;
    QString employeeName;
    QDate trainDate;
};


#endif // QMEMPLOYEEDATEENTRY_H
