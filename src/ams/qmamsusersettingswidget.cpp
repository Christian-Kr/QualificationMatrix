// qmamssettingswidget.cpp is part of QualificationMatrix
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

#include "qmamsusersettingswidget.h"
#include "ui_qmamsusersettingswidget.h"
#include "ams/model/qmamsusermodel.h"
#include "ams/model/qmamsgroupmodel.h"
#include "ams/model/qmamsusergroupmodel.h"
#include "ams/qmamspassworddialog.h"
#include "ams/qmamsmanager.h"
#include "framework/delegate/qmbooleandelegate.h"

#include <QSqlDatabase>
#include <QSqlRecord>
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QInputDialog>

QMAMSUserSettingsWidget::QMAMSUserSettingsWidget(QWidget *parent)
    : QMSettingsWidget(parent, true)
    , ui(new Ui::QMAMSUserSettingsWidget)
    , amsUserGroupProxyModel(std::make_unique<QSortFilterProxyModel>(this))
{
    ui->setupUi(this);

    // Ui
    auto booleanDelegate = new QMBooleanDelegate(this);
    booleanDelegate->setEditable(true);
    ui->tvUser->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tvUser->setItemDelegateForColumn(6, booleanDelegate);
    ui->tvUser->setItemDelegateForColumn(7, booleanDelegate);
}

QMAMSUserSettingsWidget::~QMAMSUserSettingsWidget()
{
    delete ui;
}

void QMAMSUserSettingsWidget::saveSettings()
{
    bool error = false;

    if (amsUserModel->isDirty())
    {
        error = error | !amsUserModel->submitAll();

        // After user has changed, some databases should be updated.
        amsUserGroupProxyModel->sourceModel();
    }

    if (amsUserGroupModel->isDirty())
    {
        error = error | !amsUserGroupModel->submitAll();
    }

    if (error)
    {
        QMessageBox::critical(this, tr("Speichern"),
                tr("Die Änderungen konnten nicht in die Datenbank geschrieben werden."));
    }
}

void QMAMSUserSettingsWidget::revertChanges()
{
    amsUserModel->revertAll();
    amsGroupModel->revertAll();
    amsUserGroupModel->revertAll();
}

void QMAMSUserSettingsWidget::loadSettings()
{
    updateData();
}

void QMAMSUserSettingsWidget::updateData()
{
    // Get the current database and update data only when it is connected.
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        return;
    }

    auto db = QSqlDatabase::database("default");

    amsUserModel = std::make_unique<QMAMSUserModel>(this, db);
    amsUserModel->select();

    amsGroupModel = std::make_unique<QMAMSGroupModel>(this, db);
    amsGroupModel->select();

    amsUserGroupModel = std::make_unique<QMAMSUserGroupModel>(this, db);
    amsUserGroupModel->select();

    ui->tvUser->setModel(amsUserModel.get());
    ui->tvUser->hideColumn(0);
    ui->tvUser->hideColumn(3);
    ui->tvUser->resizeColumnsToContents();

    ui->lvGroup->setModel(amsGroupModel.get());
    ui->lvGroup->setModelColumn(1);

    // Build some connections.
    connect(amsUserModel.get(), &QMAMSUserModel::dataChanged, this,
            &QMAMSUserSettingsWidget::settingsChanged);
    connect(ui->tvUser->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            &QMAMSUserSettingsWidget::userSelectionChanged);
    connect(ui->lvGroup->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            &QMAMSUserSettingsWidget::groupSelectionChanged);
}

void QMAMSUserSettingsWidget::addUser()
{
    // TODO: No user with same username.

    auto record = amsUserModel->record();
    record.setValue("amsuser_name", tr("Vollständiger Name"));
    record.setValue("amsuser_username", tr("Benutzername"));
    record.setValue("amsuser_unsuccess_login_num", 0);

    // TODO: Get password and add it.
    QMAMSPasswordDialog passwordDialog(this);
    passwordDialog.exec();

    auto password = passwordDialog.getPasswort();
    if (password.isEmpty())
    {
        return;
    }

    auto hash = QMAMSManager::createPasswordHash(password);
    record.setValue("amsuser_password", hash);

    if (!amsUserModel->insertRecord(-1, record) | !amsUserModel->submitAll())
    {
        qCritical() << "could not save a new record";
    }
}

void QMAMSUserSettingsWidget::addGroup()
{
    // Get the primary key id of selected element.
    auto groupIndex = ui->lvGroup->currentIndex();
    if (!groupIndex.isValid())
    {
        QMessageBox::information(this, tr("Gruppe hinzufügen"), tr("Keine Gruppe ausgewählt"));
        return;
    }

    auto groupModelPrimaryIdField = amsGroupModel->fieldIndex("amsgroup_id");
    if (groupModelPrimaryIdField < 0)
    {
        qCritical() << "Cannot find field index of id in AMSGroup";
        return;
    }

    auto groupModelIndex = amsGroupModel->index(groupIndex.row(), groupModelPrimaryIdField);
    auto groupPrimaryId = amsGroupModel->data(groupModelIndex).toInt();

    // Get primary key id of selected user.
    auto userIndex = ui->tvUser->currentIndex();
    if(!userIndex.isValid())
    {
        QMessageBox::information(this, tr("Gruppe hinzufügen"), tr("Kein Benutzer ausgewählt"));
        return;
    }

    auto userModelPrimaryIdField = amsUserModel->fieldIndex("amsuser_id");
    if (userModelPrimaryIdField < 0)
    {
        qCritical() << "Cannot find field index of id in AMSUser";
        return;
    }

    auto userModelIndex = amsUserModel->index(userIndex.row(), userModelPrimaryIdField);
    auto userPrimaryId = amsUserModel->data(userModelIndex).toInt();

    // Search for duplicates.
    if (userGroupProxyContainsGroup(groupIndex.data().toString()))
    {
        QMessageBox::information(this, tr("Gruppe hinzufügen"), tr("Die Gruppe wurde bereits hinzugefügt."));
        return;
    }

    // Create record and add.
    auto record = amsUserGroupModel->record();

    record.setValue("amsuser_username", userPrimaryId);
    record.setValue("amsgroup_name", groupPrimaryId);

    if (!amsUserGroupModel->insertRecord(-1, record) | !amsUserGroupModel->submitAll())
    {
        QMessageBox::warning(this, tr("Gruppe hinzufügen"),
                tr("Konnte die Änderung nicht in die Datenbank schreiben."));
        return;
    }
}

QString QMAMSUserSettingsWidget::getUsernameFromPrimaryId(int primaryId)
{
    auto usernameFieldIndex = amsUserModel->fieldIndex("amsuser_username");
    auto idFieldIndex = amsUserModel->fieldIndex("amsuser_id");
    if (usernameFieldIndex < 0 || idFieldIndex < 0)
    {
        return {};
    }

    for (int i = 0; i < amsUserModel->rowCount(); i++)
    {
        auto id = amsUserModel->data(amsUserModel->index(i, idFieldIndex)).toInt();
        auto username = amsUserModel->data(amsUserModel->index(i, usernameFieldIndex)).toString();

        if (id == primaryId)
        {
            return username;
        }
    }

    return {};
}

QString QMAMSUserSettingsWidget::getGroupFromPrimaryId(int primaryId)
{
    auto groupFieldIndex = amsGroupModel->fieldIndex("amsgroup_name");
    auto idFieldIndex = amsGroupModel->fieldIndex("amsgroup_id");
    if (groupFieldIndex < 0 || idFieldIndex < 0)
    {
        return {};
    }

    for (int i = 0; i < amsGroupModel->rowCount(); i++)
    {
        auto id = amsGroupModel->data(amsGroupModel->index(i, idFieldIndex)).toInt();
        auto group = amsGroupModel->data(amsGroupModel->index(i, groupFieldIndex)).toString();

        if (id == primaryId)
        {
            return group;
        }
    }

    return {};
}

void QMAMSUserSettingsWidget::removeUser()
{
    auto selRows = ui->tvUser->selectionModel()->selectedRows();
    if (selRows.count() < 1)
    {
        QMessageBox::information(this, tr("Nutzer löschen"), tr("Es wurde kein Nutzer ausgewählt."));
        return;
    }

    if (selRows.count() > 1)
    {
        QMessageBox::information(this, tr("Nutzer löschen"), tr("Es darf nur ein Nutzer ausgewählt werden."));
        return;
    }

    amsUserModel->removeRow(selRows.first().row());

    // All connection between the user and the group have to be deleted.
    amsUserGroupProxyModel->removeRows(0, amsUserGroupProxyModel->rowCount());

    emitSettingsChanged();
}

void QMAMSUserSettingsWidget::removeGroup()
{
    auto userGroupIndex = ui->lvUserGroup->currentIndex();

    if (!userGroupIndex.isValid())
    {
        QMessageBox::information(this, tr("Gruppe entfernen"), tr("Keine Gruppe ausgewählt."));
        return;
    }

    amsUserGroupProxyModel->removeRow(userGroupIndex.row());
    emitSettingsChanged();
}

void QMAMSUserSettingsWidget::userGroupSelectionChanged(const QModelIndex &selected, const QModelIndex &deselected)
{
    ui->lvGroup->reset();
    if (selected.isValid())
    {
        ui->pbRemoveGroup->setEnabled(true);
    }
}

void QMAMSUserSettingsWidget::groupSelectionChanged(const QModelIndex &selected, const QModelIndex &deselected)
{
    ui->lvUserGroup->reset();
    ui->pbRemoveGroup->setEnabled(false);

    if (!selected.isValid())
    {
        ui->pbAddGroup->setEnabled(false);
        return;
    }

    auto group = selected.data().toString();

    if (userGroupProxyContainsGroup(group))
    {
        ui->pbAddGroup->setEnabled(false);
    }
    else
    {
        ui->pbAddGroup->setEnabled(true);
    }
}

bool QMAMSUserSettingsWidget::userGroupProxyContainsGroup(const QString &group)
{
    for (int i = 0; i < amsUserGroupProxyModel->rowCount(); i++)
    {
        auto groupModelIndex = amsUserGroupProxyModel->index(i, 2);
        auto groupName = amsUserGroupProxyModel->data(groupModelIndex).toString();

        if (groupName.compare(group) == 0)
        {
            return true;
        }
    }

    return false;
}

void QMAMSUserSettingsWidget::userSelectionChanged(
        const QModelIndex &selected, const QModelIndex &deselected)
{
    if (!selected.isValid())
    {
        deactivateUserGroupList();
        return;
    }

    // update ui buttons for changing user settings
    auto activeFieldIndex = amsUserModel->fieldIndex("amsuser_active");
    auto activeModelIndex = amsUserModel->index(selected.row(), activeFieldIndex);
    auto selActive = amsUserModel->data(activeModelIndex).toBool();

    if (selActive)
    {
        ui->pbChangeActiveState->setText(tr("Deaktivieren"));
    }
    else
    {
        ui->pbChangeActiveState->setText(tr("Aktivieren"));
    }

    auto adminFieldIndex = amsUserModel->fieldIndex("amsuser_admin");
    auto adminModelIndex = amsUserModel->index(selected.row(), adminFieldIndex);
    auto selAdmin = amsUserModel->data(adminModelIndex).toBool();

    if (selAdmin)
    {
        ui->pbChangeAdminState->setText(tr("Admin-Flag deakt."));
    }
    else
    {
        ui->pbChangeAdminState->setText(tr("Admin-Flag akt."));
    }

    // update ui group parts
    ui->lvGroup->reset();
    ui->pbAddGroup->setEnabled(false);
    ui->pbRemoveGroup->setEnabled(false);

    auto selModelIndex = amsUserModel->index(selected.row(), 2);
    auto selData = amsUserModel->data(selModelIndex).toString();

    if (selData.compare("administrator") == 0)
    {
        deactivateUserGroupList();
        return;
    }

    activateUserGroupList(selected.row());
}

void QMAMSUserSettingsWidget::changeAdminState()
{
    auto selRows = ui->tvUser->selectionModel()->selectedRows();
    if (selRows.count() != 1)
    {
        qWarning() << "wrong number of selected rows";
    }

    auto selRow = selRows.first().row();

    auto usernameFieldIndex = amsUserModel->fieldIndex("amsuser_username");
    auto usernameModelIndex = amsUserModel->index(selRow, usernameFieldIndex);
    auto selUsername = amsUserModel->data(usernameModelIndex).toString();

    // Don't change if selected user is administrator.
    if (selUsername.compare("administrator") == 0)
    {
        QMessageBox::information(this, tr("Admin-Status ändern"),
                tr("Der Admin-Status von 'administrator' kann nicht geändert werden."));
        return;
    }

    auto adminFieldIndex = amsUserModel->fieldIndex("amsuser_admin");
    auto adminModelIndex = amsUserModel->index(selRow, adminFieldIndex);
    auto selAdmin = amsUserModel->data(adminModelIndex).toBool();

    // Change the admin state.
    if (!amsUserModel->setData(adminModelIndex, !selAdmin))
    {
        QMessageBox::information(this, tr("Admin-Status ändern"),
                tr("Der Admin-Status konnte nicht geändert werden."));
        return;
    }

    if (!selAdmin)
    {
        ui->pbChangeAdminState->setText(tr("Admin-Flag deakt."));
    }
    else
    {
        ui->pbChangeAdminState->setText(tr("Admin-Flag akt."));
    }

    emitSettingsChanged();
}

void QMAMSUserSettingsWidget::changeActiveState()
{
    auto selRows = ui->tvUser->selectionModel()->selectedRows();
    if (selRows.count() != 1)
    {
        qWarning() << "wrong number of selected rows";
    }

    auto selRow = selRows.first().row();

    auto usernameFieldIndex = amsUserModel->fieldIndex("amsuser_username");
    auto usernameModelIndex = amsUserModel->index(selRow, usernameFieldIndex);
    auto selUsername = amsUserModel->data(usernameModelIndex).toString();

    // Don't change if selected user is administrator.
    if (selUsername.compare("administrator") == 0)
    {
        QMessageBox::information(this, tr("Aktiviert-Status ändern"),
                tr("Der Aktiviert-Status von 'administrator' kann nicht geändert werden."));
        return;
    }

    auto activeFieldIndex = amsUserModel->fieldIndex("amsuser_active");
    auto activeModelIndex = amsUserModel->index(selRow, activeFieldIndex);
    auto selActive = amsUserModel->data(activeModelIndex).toBool();

    // Change the activated state.
    if (!amsUserModel->setData(activeModelIndex, !selActive))
    {
        QMessageBox::information(this, tr("Aktiviert-Status ändern"),
                tr("Der Aktiviert-Status konnte nicht geändert werden."));
        return;
    }

    if (!selActive)
    {
        ui->pbChangeActiveState->setText(tr("Deaktivieren"));
    }
    else
    {
        ui->pbChangeActiveState->setText(tr("Aktivieren"));
    }

    emitSettingsChanged();
}

void QMAMSUserSettingsWidget::deactivateUserGroupList()
{
    ui->gbGroups->setEnabled(false);
    ui->pbRemoveUser->setEnabled(false);
    ui->pbChangeName->setEnabled(false);
    ui->pbChangeUsername->setEnabled(false);
    ui->pbChangeActiveState->setEnabled(false);

    ui->lvUserGroup->setModel(nullptr);
}

void QMAMSUserSettingsWidget::activateUserGroupList(int selRow)
{
    ui->gbGroups->setEnabled(true);
    ui->pbRemoveUser->setEnabled(true);
    ui->pbChangeName->setEnabled(true);
    ui->pbChangeUsername->setEnabled(true);
    ui->pbChangeActiveState->setEnabled(true);

    auto selModelIndex = amsUserModel->index(selRow, 2);
    auto selData = amsUserModel->data(selModelIndex).toString();

    amsUserGroupModel->select();
    amsUserGroupProxyModel->setSourceModel(amsUserGroupModel.get());
    amsUserGroupProxyModel->setFilterKeyColumn(1);
    amsUserGroupProxyModel->setFilterRegularExpression(QString("^%1$").arg(selData));
    ui->lvUserGroup->setModel(amsUserGroupProxyModel.get());
    ui->lvUserGroup->setModelColumn(2);

    ui->lvUserGroup->selectionModel()->disconnect(this);
    connect(ui->lvUserGroup->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            &QMAMSUserSettingsWidget::userGroupSelectionChanged);
}

void QMAMSUserSettingsWidget::resetLoginNum()
{
    auto selRows = ui->tvUser->selectionModel()->selectedRows();
    if (selRows.count() != 1)
    {
        qWarning() << "wrong number of selected rows";
    }

    auto selRow = selRows.first().row();

    auto loginNumFieldIndex = amsUserModel->fieldIndex("amsuser_unsuccess_login_num");
    auto loginNumModelIndex = amsUserModel->index(selRow, loginNumFieldIndex);

    if (!amsUserModel->setData(loginNumModelIndex, 0))
    {
        QMessageBox::information(this, tr("Loginversuche zurücksetzen"),
                tr("Die Anzahl der Loginversuche konnte nicht zurückgesetzt werden."));
        return;
    }

    emitSettingsChanged();
}

void QMAMSUserSettingsWidget::changeName()
{
    auto selRows = ui->tvUser->selectionModel()->selectedRows();
    if (selRows.count() != 1)
    {
        qWarning() << "wrong number of selected rows";
    }

    auto selRow = selRows.first().row();

    auto usernameFieldIndex = amsUserModel->fieldIndex("amsuser_username");
    auto usernameModelIndex = amsUserModel->index(selRow, usernameFieldIndex);
    auto selUsername = amsUserModel->data(usernameModelIndex).toString();

    // Don't change if selected user is administrator.
    if (selUsername.compare("administrator") == 0)
    {
        QMessageBox::information(this, tr("Name ändern"),
                tr("Der Name von 'administrator' kann nicht geändert werden."));
        return;
    }

    auto nameFieldIndex = amsUserModel->fieldIndex("amsuser_name");
    auto nameModelIndex = amsUserModel->index(selRow, nameFieldIndex);
    auto selName = amsUserModel->data(nameModelIndex).toString();

    // Get the new username.
    bool ok = false;
    bool first = true;
    QString newName = selName;
    while (ok || first)
    {
        first = false;
        newName = QInputDialog::getText(this, tr("Name von %1 ändern").arg(selName), tr("Name"), QLineEdit::Normal,
                newName, &ok);

        if (!ok) {
            return;
        }

        // The name should only consist of letters and spaces.
        if (!newName.contains(QRegularExpression("^[a-zA-Z ]+$")) || newName.length() < 6)
        {
            QMessageBox::information(this, tr("Name ändern"),
                    tr("Der Name darf nur Buchstaben und Leerzeichen enthalten und muss mindestens 6 Zeichen "
                    "lang sein."));
            continue;
        }

        break;
    }

    // Change the name.
    if (!amsUserModel->setData(nameModelIndex, newName))
    {
        QMessageBox::information(this, tr("Name ändern"), tr("Der Name konnte nicht geändert werden."));
        return;
    }

    emitSettingsChanged();
}

void QMAMSUserSettingsWidget::changeUsername()
{
    auto selRows = ui->tvUser->selectionModel()->selectedRows();
    if (selRows.count() != 1)
    {
        qWarning() << "wrong number of selected rows";
    }

    auto selRow = selRows.first().row();

    auto usernameFieldIndex = amsUserModel->fieldIndex("amsuser_username");
    auto usernameModelIndex = amsUserModel->index(selRow, usernameFieldIndex);
    auto selUsername = amsUserModel->data(usernameModelIndex).toString();

    // Don't change if selected user is administrator.
    if (selUsername.compare("administrator") == 0)
    {
        QMessageBox::information(this, tr("Benutzername ändern"),
                tr("Der Benutzername von 'administrator' kann nicht geändert werden."));
        return;
    }

    // Get the new username.
    bool ok = false;
    bool first = true;
    QString newUsername = selUsername;
    while (ok || first)
    {
        first = false;
        newUsername = QInputDialog::getText(this, tr("Benutzername von %1 ändern").arg(selUsername),
                tr("Benutzername"), QLineEdit::Normal, newUsername, &ok);

        if (!ok) {
            return;
        }

        // The username should consist of letters and numbers.
        if (!newUsername.contains(QRegularExpression("^[a-zA-Z0-9]+$")) || newUsername.length() < 5)
        {
            QMessageBox::information(this, tr("Benutzername ändern"),
                    tr("Der Benutzername darf nur Buchstaben enthalten und muss mindestens 6 Zeichen lang sein."));
            continue;
        }

        // The username should not already exist.
        if (userContainsUsername(newUsername) && selUsername.compare(newUsername) != 0)
        {
            QMessageBox::information(this, tr("Benutzername ändern"), tr("Der Benutzername existiert bereits."));
            continue;
        }

        break;
    }

    // Change the username.
    if (!amsUserModel->setData(usernameModelIndex, newUsername))
    {
        QMessageBox::information(this, tr("Benutzername ändern"),
                tr("Der Benutzername konnte nicht geändert werden."));
        return;
    }

    emitSettingsChanged();
}

bool QMAMSUserSettingsWidget::userContainsUsername(const QString &username)
{
    auto usernameFieldIndex = amsUserModel->fieldIndex("amsuser_username");

    for (int i = 0; i < amsUserModel->rowCount(); i++)
    {
        auto usernameModelIndex = amsUserModel->index(i, usernameFieldIndex);
        auto tmpUsername = amsUserModel->data(usernameModelIndex).toString();

        if (tmpUsername.compare(username) == 0)
        {
            return true;
        }
    }

    return false;
}

void QMAMSUserSettingsWidget::changePassword()
{
    QMAMSPasswordDialog passwordDialog(this);
    passwordDialog.exec();

    auto password = passwordDialog.getPasswort();
    if (password.isEmpty())
    {
        return;
    }

    auto selRows = ui->tvUser->selectionModel()->selectedRows();
    if (selRows.count() != 1)
    {
        qWarning() << "wrong number of selected rows";
    }

    auto userIdFieldIndex = amsUserModel->fieldIndex("amsuser_id");
    auto userId = amsUserModel->data(amsUserModel->index(selRows.first().row(), userIdFieldIndex)).toInt();

    auto amsManager = QMAMSManager::getInstance();
    if (!amsManager->setPassword(userId, password))
    {
        QMessageBox::warning(this, tr("Passwort ändern"), tr("Das Passwort konnte nicht festgelegt werden."));
        return;
    }
    else
    {
        QMessageBox::information(this, tr("Passwort ändern"), tr("Das Passwort wurde geändert."));
        return;
    }
}
