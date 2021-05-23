// qmwinmodewidget.h is part of QualificationMatrix
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

#ifndef QMWINMODEWIDGET_H
#define QMWINMODEWIDGET_H

#include <QWidget>

/// General widget which should be used for a mode. Several functions should be implemented for the modes to make it
/// fit into the system.
/// \author Christian Kr, Copyright 2021
class QMWinModeWidget: public QWidget
{
    Q_OBJECT

public:
    /// Constructor
    /// \param parent
    explicit QMWinModeWidget(QWidget *parent = nullptr);

signals:
    /// Informate about starting work load.
    /// \param info Message to display.
    /// \param max Maximum process value.
    void startWorkload(QString info, int max);

    /// Informate about work load update.
    /// \param info Message to display.
    /// \param curr The current value.
    void updateWorkload(int curr);

    /// Informate about ending work load.
    void endWorkload();
};

#endif // QMWINMODEWIDGET_H
