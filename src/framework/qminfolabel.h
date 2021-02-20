//
// qminfolabel.h is part of QualificationMatrix
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

#ifndef QMINFOLABEL_H
#define QMINFOLABEL_H

#include <QLabel>

class QTimer;

/// An information label which shows messages inside the application. This might be useful for
/// messages, where showing a dialog is too much.
/// \author Christian Kr, Copyright 2020
class QMInfoLabel: public QLabel
{
Q_OBJECT

public:
    /// Constructor
    /// \param parent
    explicit QMInfoLabel(QWidget *parent = nullptr);

    /// Will be called from constructor.
    void loadSettings();

    /// Show a message.
    /// \param msg
    void showInfoMessage(QString msg);

    /// Show a warning message.
    /// \param msg
    void showWarnMessage(QString msg);

private:
    QTimer *timer;
};

#endif // QMINFOLABEL_H
