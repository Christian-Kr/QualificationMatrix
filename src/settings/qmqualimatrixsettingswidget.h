// qmqualimatrixsettingswidget.h is part of QualificationMatrix
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

#ifndef QMQUALIMATRIXSETTINGSWIDGET_H
#define QMQUALIMATRIXSETTINGSWIDGET_H

#include "qmsettingswidget.h"

namespace Ui
{
class QMQualiMatrixSettingsWidget;
}

/// Settings widget for qualification matrix.
/// \author Christian Kr, Copyright 2020
class QMQualiMatrixSettingsWidget: public QMSettingsWidget
{
Q_OBJECT

public:
    /// Constructor
    /// \param parent The parent object for the qt system.
    explicit QMQualiMatrixSettingsWidget(QWidget *parent = nullptr);

    /// Destructor
    ~QMQualiMatrixSettingsWidget() override;

    /// This slot has to be implemented by parent class. When calling this function, it should
    /// take a QSettings object or data Model (bspw. SqlModel) to save every changes to a certificate
    /// based system.
    void saveSettings() override;

    /// Override from QMSettingsWidget.
    void revertChanges() override;

    /// Override from QMSettingsWidget.
    void loadSettings() override;

public slots:
    /// Button for changing the grid color has been pressed.
    void changeGridColor();

    /// Button for changing the selection color has been pressed.
    void changeSelectionColor();

private:
    Ui::QMQualiMatrixSettingsWidget *ui;
};

#endif // QMQUALIMATRIXSETTINGSWIDGET_H
