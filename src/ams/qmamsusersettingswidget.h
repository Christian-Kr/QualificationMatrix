// qmamssettingswidget.h is part of QualificationMatrix
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

#ifndef QMAMSUSERSETTINGSWIDGET_H
#define QMAMSUSERSETTINGSWIDGET_H

#include "settings/qmsettingswidget.h"

class QMAMSGroupModel;
class QMAMSUserModel;

namespace Ui
{
    class QMAMSUserSettingsWidget;
}

/// Settings widget for account management system.
/// \author Christian Kr, Copyright 2020
class QMAMSUserSettingsWidget: public QMSettingsWidget
{
    Q_OBJECT

public:
    /// Constructor
    /// \param parent The parent object for the qt system.
    explicit QMAMSUserSettingsWidget(QWidget *parent = nullptr);

    /// Destructor
    ~QMAMSUserSettingsWidget() override;

    /// This slot has to be implemented by parent class. When calling this
    /// function, it should take a QSettings object or data Model
    /// (bspw. SqlModel) to save every changes to a certificate based system.
    void saveSettings() override;

    /// Override from QMSettingsWidget.
    void revertChanges() override;

    /// Override from QMSettingsWidget.
    void loadSettings() override;

public slots:
    /// Add a new user.
    void addUser();

    /// Add a new group.
    void addGroup();

    /// Remove a user.
    void removeUser();

    /// Remove a group.
    void removeGroup();

    /// Config the user.
    void configUser();

    /// Config the group.
    void configGroup();

private:
    /// Update model data.
    void updateData();

    Ui::QMAMSUserSettingsWidget *ui;

    std::unique_ptr<QMAMSUserModel> amsUserModel;
    std::unique_ptr<QMAMSGroupModel> amsGroupModel;
};

#endif // QMAMSUSERSETTINGSWIDGET_H
