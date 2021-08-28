// qmamsgroupsettingswidget.cpp is part of QualificationMatrix
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

#include "qmamsgroupsettingswidget.h"
#include "ui_qmamsgroupsettingswidget.h"
#include "ams/model/qmamsgroupmodel.h"
#include "ams/model/qmamsgroupaccessmodemodel.h"
#include "ams/model/qmamsaccessmodemodel.h"
#include "ams/qmamspassworddialog.h"
#include "ams/qmamsmanager.h"
#include "ams/qmamspassworddialog.h"
#include "framework/qmbooleandelegate.h"

#include <QSqlDatabase>
#include <QSqlRecord>
#include <QMessageBox>
#include <QSortFilterProxyModel>

#include <QDebug>

QMAMSGroupSettingsWidget::QMAMSGroupSettingsWidget(QWidget *parent)
    : QMSettingsWidget(parent, true)
    , ui(new Ui::QMAMSGroupSettingsWidget)
    , amsGroupAccessModeProxyModel(std::make_unique<QSortFilterProxyModel>(this))
{
    ui->setupUi(this);

    // Ui
    auto booleanDelegate = new QMBooleanDelegate(this);
    booleanDelegate->setEditable(true);
    ui->tvGroup->setItemDelegateForColumn(2, booleanDelegate);
}

QMAMSGroupSettingsWidget::~QMAMSGroupSettingsWidget()
{
    delete ui;
}

void QMAMSGroupSettingsWidget::saveSettings()
{
    bool error = false;

    if (amsGroupModel->isDirty())
    {
        error = error | !amsGroupModel->submitAll();
    }

    if (amsGroupAccessModeModel->isDirty())
    {
        error = error | !amsGroupAccessModeModel->submitAll();
    }

    if (error)
    {
        QMessageBox::critical(this, tr("Speichern"),
                tr("Die Änderungen konnten nicht in die Datenbank "
                "geschrieben werden."));
    }
}

void QMAMSGroupSettingsWidget::revertChanges()
{
    // TODO: Revert all changes and load last state.
}

void QMAMSGroupSettingsWidget::loadSettings()
{
    updateData();
}

void QMAMSGroupSettingsWidget::updateData()
{
    // Get the current database and update data only when it is connected.
    if (!QSqlDatabase::contains("default") ||
        !QSqlDatabase::database("default", false).isOpen())
    {
        return;
    }

    auto db = QSqlDatabase::database("default");

    amsGroupModel = std::make_unique<QMAMSGroupModel>(this, db);
    amsGroupModel->select();

    amsAccessModeModel = std::make_unique<QMAMSAccessModeModel>(this, db);
    amsAccessModeModel->select();

    amsGroupAccessModeModel = std::make_unique<QMAMSGroupAccessModeModel>(this, db);
    amsGroupAccessModeModel->select();

    ui->tvGroup->setModel(amsGroupModel.get());
    ui->tvGroup->hideColumn(0);
    ui->tvGroup->resizeColumnsToContents();

    ui->lvAccessMode->setModel(amsAccessModeModel.get());
    ui->lvAccessMode->setModelColumn(1);

    // Build some connections.
    connect(amsGroupModel.get(), &QMAMSGroupModel::dataChanged, this,
            &QMAMSGroupSettingsWidget::settingsChanged);
    connect(ui->tvGroup->selectionModel(),
            &QItemSelectionModel::currentRowChanged, this,
            &QMAMSGroupSettingsWidget::groupSelectionChanged);
    connect(ui->lvAccessMode->selectionModel(),
            &QItemSelectionModel::currentRowChanged, this,
            &QMAMSGroupSettingsWidget::accessModeSelectionChanged);
}

void QMAMSGroupSettingsWidget::addGroup()
{
    // TODO: Implement

//    auto record = amsGroupModel->record();
//    record.setValue("name", tr("Vollständiger Name"));
//    record.setValue("username", tr("Benutzername"));
//
//    // TODO: Get password and add it.
//    QMAMSPasswordDialog passwordDialog(this);
//    passwordDialog.exec();
//
//    auto password = passwordDialog.getPasswort();
//    if (password.isEmpty())
//    {
//        return;
//    }
//
//    auto hash = QMAMSManager::createPasswordHash(password);
//    record.setValue("password", hash);
//
//    if (!amsUserModel->insertRecord(-1, record) | !amsUserModel->submitAll())
//    {
//        qWarning() << "could not save a new record";
//    }
}

void QMAMSGroupSettingsWidget::addAccessMode()
{
    // TODO: Implement
}

void QMAMSGroupSettingsWidget::removeGroup()
{
    // TODO: Implement
}

void QMAMSGroupSettingsWidget::removeAccessMode()
{
    auto groupAccessModeIndex = ui->lvGroupAccessMode->currentIndex();

    if (!groupAccessModeIndex.isValid())
    {
        QMessageBox::information(this, tr("Gruppe entfernen"),
                tr("Keine Gruppe ausgewählt."));
        return;
    }

    amsGroupAccessModeProxyModel->removeRow(groupAccessModeIndex.row());

    emitSettingsChanged();
}

void QMAMSGroupSettingsWidget::groupAccessModeSelectionChanged(
        const QModelIndex &selected, const QModelIndex &deselected)
{
    ui->lvAccessMode->reset();
    if (selected.isValid())
    {
        ui->pbRemoveAccessMode->setEnabled(true);
    }
}

void QMAMSGroupSettingsWidget::accessModeSelectionChanged(
        const QModelIndex &selected, const QModelIndex &deselected)
{
    ui->lvGroupAccessMode->reset();
    ui->pbRemoveAccessMode->setEnabled(false);

    if (!selected.isValid())
    {
        ui->pbAddAccessMode->setEnabled(false);
        return;
    }

    auto group = selected.data().toString();

    if (groupAccessModeProxyContainsGroup(group))
    {
        ui->pbAddAccessMode->setEnabled(false);
    }
    else
    {
        ui->pbAddAccessMode->setEnabled(true);
    }
}

bool QMAMSGroupSettingsWidget::groupAccessModeProxyContainsGroup(
        const QString &accessMode)
{
    for (int i = 0; i < amsGroupAccessModeProxyModel->rowCount(); i++)
    {
        auto groupModelIndex = amsGroupAccessModeProxyModel->index(i, 2);
        auto groupName = amsGroupAccessModeProxyModel->data(groupModelIndex)
                .toString();

        if (groupName.compare(accessMode) == 0)
        {
            return true;
        }
    }

    return false;
}

void QMAMSGroupSettingsWidget::groupSelectionChanged(
        const QModelIndex &selected, const QModelIndex &deselected)
{
    ui->lvAccessMode->reset();
    ui->pbAddAccessMode->setEnabled(false);
    ui->pbRemoveAccessMode->setEnabled(false);

    if (!selected.isValid())
    {
        deactivateGroupAccessModeList();
        return;
    }

    auto selModelIndex = amsGroupModel->index(selected.row(), 2);
    auto selData = amsGroupModel->data(selModelIndex).toString();

    activateGroupAccessModeList(selected.row());
}

void QMAMSGroupSettingsWidget::deactivateGroupAccessModeList()
{
    ui->gbAccessMode->setEnabled(false);
    ui->pbRemoveGroup->setEnabled(false);

    ui->lvGroupAccessMode->setModel(nullptr);
}

void QMAMSGroupSettingsWidget::activateGroupAccessModeList(int selRow)
{
    ui->gbAccessMode->setEnabled(true);
    ui->pbRemoveGroup->setEnabled(true);

    auto selModelIndex = amsGroupModel->index(selRow, 2);
    auto selData = amsGroupModel->data(selModelIndex).toString();

    amsGroupAccessModeProxyModel->setSourceModel(amsGroupAccessModeModel.get());
    amsGroupAccessModeProxyModel->setFilterKeyColumn(1);
    amsGroupAccessModeProxyModel->setFilterFixedString(selData);
    ui->lvGroupAccessMode->setModel(amsGroupAccessModeProxyModel.get());
    ui->lvGroupAccessMode->setModelColumn(2);

    ui->lvGroupAccessMode->selectionModel()->disconnect(this);
    connect(ui->lvGroupAccessMode->selectionModel(),
            &QItemSelectionModel::currentRowChanged, this,
            &QMAMSGroupSettingsWidget::groupAccessModeSelectionChanged);
}
