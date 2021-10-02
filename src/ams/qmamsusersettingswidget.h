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
class QMAMSUserGroupModel;
class QSortFilterProxyModel;

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

    /// User selection changed.
    /// \param deselected Item that has been deselected.
    /// \param selected Item that has been selected.
    void userSelectionChanged(const QModelIndex &selected,
            const QModelIndex &deselected);

    /// Group selection changed.
    /// \param deselected Item that has been deselected.
    /// \param selected Item that has been selected.
    void groupSelectionChanged(const QModelIndex &selected,
            const QModelIndex &deselected);

    /// User group correlation table selection changed.
    /// \param deselected Item that has been deselected.
    /// \param selected Item that has been selected.
    void userGroupSelectionChanged(const QModelIndex &selected,
            const QModelIndex &deselected);

    /// Change the password of the selected user.
    void changePassword();

    /// Change the username of the selected user.
    void changeUsername();

    /// Change the name of the user.
    void changeName();

    /// Reset login num.
    void resetLoginNum();

    /// Changes the user from activated to deactivated and vise versa.
    void changeActiveState();

private:
    /// Get the username from the given primary id.
    /// \param primaryId The primary key id in the table AMSUser.
    /// \return The username or empty string if nothing found.
    QString getUsernameFromPrimaryId(int primaryId);

    /// Get the group from the given primary id.
    /// \param primaryId The primary key id in the table AMSGroup.
    /// \return The group or empty string if nothing found.
    QString getGroupFromPrimaryId(int primaryId);

    /// Update model data.
    void updateData();

    /// Run action for enabling the usergroup list.
    /// \param selRow Selected row.
    void activateUserGroupList(int selRow);

    /// Run action for disabling the usergroup list.
    void deactivateUserGroupList();

    /// Search for a user in user group proxy model.
    /// \param name The name of the group to search for.
    /// \return True if minimum one exist, else false.
    bool userGroupProxyContainsGroup(const QString &group);

    /// Test for username in user table.
    /// \param username The name to search for.
    /// \return True if found, else false.
    bool userContainsUsername(const QString &username);

    Ui::QMAMSUserSettingsWidget *ui;

    std::unique_ptr<QMAMSUserModel> amsUserModel;
    std::unique_ptr<QMAMSGroupModel> amsGroupModel;
    std::unique_ptr<QMAMSUserGroupModel> amsUserGroupModel;

    std::unique_ptr<QSortFilterProxyModel> amsUserGroupProxyModel;
};

#endif // QMAMSUSERSETTINGSWIDGET_H
