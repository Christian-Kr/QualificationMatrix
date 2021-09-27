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
#include "ams/qmamsmanager.h"
#include "ams/model/qmamsusergroupmodel.h"
#include "framework/qmbooleandelegate.h"

#include <QSqlDatabase>
#include <QSqlRecord>
#include <QSqlError>
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
    amsGroupModel->revertAll();
    amsGroupAccessModeModel->revertAll();
    amsAccessModeModel->revertAll();
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
    auto record = amsGroupModel->record();
    record.setValue("amsgroup_name", tr("Gruppe"));

    if (!amsGroupModel->insertRecord(-1, record) | !amsGroupModel->submitAll())
    {
        qWarning() << "could not save a new record";
    }
}

void QMAMSGroupSettingsWidget::addAccessMode()
{
    // Get primary key id of selected element.
    auto accessModeIndex = ui->lvAccessMode->currentIndex();
    if (!accessModeIndex.isValid())
    {
        QMessageBox::information(this, tr("Zugriffsmodus hinzufügen"), tr("Kein Zugriffsmodus ausgewählt."));
        return;
    }

    auto accessModePrimaryIdField = amsAccessModeModel->fieldIndex("amsaccessmode_id");
    if (accessModePrimaryIdField < 0)
    {
        qCritical() << "Cannto find field index of id in AMSAccessMode";
        return;
    }

    auto accessModeModelIndex = amsAccessModeModel->index(accessModeIndex.row(), accessModePrimaryIdField);
    auto accessModePrimaryId = amsAccessModeModel->data(accessModeModelIndex).toInt();

    // Get primary key of selected group.
    auto groupIndex = ui->tvGroup->currentIndex();
    if (!groupIndex.isValid())
    {
        QMessageBox::information(this, tr("Zugriffsmodus hinzufügen"), tr("Keine Gruppe ausgewählt"));
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

    // Search for duplicates.
    if (groupAccessModeProxyContainsAccessMode(accessModeIndex.data().toString()))
    {
        QMessageBox::information(this, tr("Zugriffsmodus hinzufügen"),
                tr("Der Zugriffsmodus wurde bereits hinzugefügt."));
        return;
    }

    // Create record and add.
    auto record = amsGroupAccessModeModel->record();

    record.setValue("amsgroup_name", groupPrimaryId);
    record.setValue("amsaccessmode_name", accessModePrimaryId);

    if (!amsGroupAccessModeModel->insertRecord(-1, record) |
        !amsGroupAccessModeModel->submitAll())
    {
        QMessageBox::warning(this, tr("Zugriffsmodus hinzufügen"),
                tr("Konnte die Änderung nicht in die Datenbank schreiben."));
        return;
    }
}

bool QMAMSGroupSettingsWidget::groupAccessModeProxyContainsAccessMode(
        const QString &accessMode)
{
    for (int i = 0; i < amsGroupAccessModeProxyModel->rowCount(); i++)
    {
        auto accessModeModelIndex = amsGroupAccessModeProxyModel->index(i, 2);
        auto accessModeName = amsGroupAccessModeProxyModel->data(
                accessModeModelIndex).toString();
        if (accessModeName.compare(accessMode) == 0)
        {
            return true;
        }
    }

    return false;
}

void QMAMSGroupSettingsWidget::removeGroup()
{
    // Get the current database and update data only when it is connected.
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        qCritical() << "Cannot connect to db.";
        return;
    }

    auto db = QSqlDatabase::database("default");

    // Get the selected group.
    auto selRows = ui->tvGroup->selectionModel()->selectedRows();
    if (selRows.count() != 1)
    {
        qCritical() << "Wrong number of selected rows";
        return;
    }
    auto selRow = selRows.first().row();

    auto groupNameFieldIndex = amsGroupModel->fieldIndex("amsgroup_name");
    auto groupNameModelIndex = amsGroupModel->index(selRow, groupNameFieldIndex);
    auto groupName = amsGroupModel->data(groupNameModelIndex).toString();

    QMAMSUserGroupModel amsUserGroupModel(this, db);
    amsUserGroupModel.select();

    auto groupFieldIndex = amsUserGroupModel.fieldIndex("amsgroup_name");

    for (int i = 0; i < amsUserGroupModel.rowCount(); i++)
    {
        auto groupModelIndex = amsUserGroupModel.index(i, groupFieldIndex);
        auto dbGroup = amsUserGroupModel.data(groupModelIndex).toString();

        if(groupName.compare(dbGroup) == 0)
        {
            QMessageBox::information(this, tr("Gruppe löschen"),
                    tr("Ein Benutzer ist mit der Gruppe verbunden. Bitte löschen Sie zuerst die Verknüpfung."));
            return;
        }
    }

    // Delete group.
    if (!amsGroupAccessModeProxyModel->removeRows(0, amsGroupAccessModeProxyModel->rowCount()) ||
        !amsGroupModel->removeRow(selRow))
    {
        QMessageBox::information(this, tr("Gruppe löschen"), tr("Die Gruppe konnte nicht gelöscht werden."));
        return;
    }

    emitSettingsChanged();
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

    auto accessMode = selected.data().toString();

    if (groupAccessModeProxyContainsAccessMode(accessMode))
    {
        ui->pbAddAccessMode->setEnabled(false);
    }
    else
    {
        ui->pbAddAccessMode->setEnabled(true);
    }
}

void QMAMSGroupSettingsWidget::groupSelectionChanged(const QModelIndex &selected, const QModelIndex &deselected)
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

    auto selModelIndex = amsGroupModel->index(selRow, 1);
    auto selData = amsGroupModel->data(selModelIndex).toString();

    amsGroupAccessModeProxyModel->setSourceModel(amsGroupAccessModeModel.get());
    amsGroupAccessModeProxyModel->setFilterKeyColumn(1);
    amsGroupAccessModeProxyModel->setFilterRegExp(QString("^%1$").arg(selData));
    ui->lvGroupAccessMode->setModel(amsGroupAccessModeProxyModel.get());
    ui->lvGroupAccessMode->setModelColumn(2);

    ui->lvGroupAccessMode->selectionModel()->disconnect(this);
    connect(ui->lvGroupAccessMode->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            &QMAMSGroupSettingsWidget::groupAccessModeSelectionChanged);
}
