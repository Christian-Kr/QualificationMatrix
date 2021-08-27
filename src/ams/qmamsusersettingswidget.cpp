// qmamssettingswidget.cpp is part of QualificationMatrix
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

#include "qmamsusersettingswidget.h"
#include "ui_qmamsusersettingswidget.h"
#include "ams/model/qmamsusermodel.h"
#include "ams/model/qmamsgroupmodel.h"
#include "ams/model/qmamsusergroupmodel.h"
#include "ams/qmamspassworddialog.h"
#include "ams/qmamsmanager.h"

#include <QSqlDatabase>
#include <QSqlRecord>
#include <QMessageBox>

#include <QDebug>

QMAMSUserSettingsWidget::QMAMSUserSettingsWidget(QWidget *parent)
    : QMSettingsWidget(parent, true)
    , ui(new Ui::QMAMSUserSettingsWidget)
{
    ui->setupUi(this);
}

QMAMSUserSettingsWidget::~QMAMSUserSettingsWidget()
{
    delete ui;
}

void QMAMSUserSettingsWidget::saveSettings()
{
    if (amsUserModel->isDirty())
    {
        if (!amsUserModel->submitAll())
        {
            QMessageBox::critical(this, tr("Speichern"),
                    tr("Die Änderungen konnten nicht in die Datenbank "
                    "geschrieben werden."));
        }
    }
}

void QMAMSUserSettingsWidget::revertChanges()
{
    // TODO: Revert all changes and load last state.
}

void QMAMSUserSettingsWidget::loadSettings()
{
    updateData();
}

void QMAMSUserSettingsWidget::updateData()
{
    // Get the current database and update data only when it is connected.
    if (!QSqlDatabase::contains("default") ||
        !QSqlDatabase::database("default", false).isOpen())
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

    ui->lvUserGroup->setModel(amsUserGroupModel.get());
    ui->lvUserGroup->setModelColumn(2);

    // Build some connections.
    connect(amsUserModel.get(), &QMAMSUserModel::dataChanged, this,
            &QMAMSUserSettingsWidget::settingsChanged);
    connect(ui->tvUser->selectionModel(),
            &QItemSelectionModel::currentRowChanged, this,
            &QMAMSUserSettingsWidget::userSelectionChanged);
}

void QMAMSUserSettingsWidget::addUser()
{
    // TODO: Implement

    auto record = amsUserModel->record();
    record.setValue("name", tr("Vollständiger Name"));
    record.setValue("username", tr("Benutzername"));

    // TODO: Get password and add it.
    QMAMSPasswordDialog passwordDialog(this);
    passwordDialog.exec();

    auto password = passwordDialog.getPasswort();
    if (password.isEmpty())
    {
        return;
    }

    auto hash = QMAMSManager::createPasswordHash(password);
    record.setValue("password", hash);

    if (!amsUserModel->insertRecord(-1, record) | !amsUserModel->submitAll())
    {
        qWarning() << "could not save a new record";
    }
}

void QMAMSUserSettingsWidget::addGroup()
{
    // TODO: Implement
}

void QMAMSUserSettingsWidget::removeUser()
{
    // TODO: Implement
}

void QMAMSUserSettingsWidget::removeGroup()
{
    // TODO: Implement
}

void QMAMSUserSettingsWidget::configUser()
{
    // TODO: Implement
}

void QMAMSUserSettingsWidget::configGroup()
{
    // TODO: Implement
}

void QMAMSUserSettingsWidget::userSelectionChanged(
        const QModelIndex &selected, const QModelIndex &deselected)
{
    if (!selected.isValid())
    {
        ui->gbGroups->setEnabled(false);
        ui->pbConfigUser->setEnabled(false);
        ui->pbRemoveUser->setEnabled(false);

        return;
    }

    auto selModelIndex = amsUserModel->index(selected.row(), 2);
    auto selData = amsUserModel->data(selModelIndex).toString();

    if (selData.compare("administrator") == 0)
    {
        ui->gbGroups->setEnabled(false);
        ui->pbConfigUser->setEnabled(false);
        ui->pbRemoveUser->setEnabled(false);

        return;
    }

    ui->gbGroups->setEnabled(true);
    ui->pbConfigUser->setEnabled(true);
    ui->pbRemoveUser->setEnabled(true);
}

void QMAMSUserSettingsWidget::changePassword()
{
    // TODO: Implement
}