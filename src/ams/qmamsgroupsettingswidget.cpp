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
#include "model/qmemployeemodel.h"
#include "ams/model/qmamsgroupemployeemodel.h"

#include <QInputDialog>
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
    , amsGroupEmployeeProxyModel(std::make_unique<QSortFilterProxyModel>(this))
{
    ui->setupUi(this);

    // Ui
    auto booleanDelegate = new QMBooleanDelegate(this);
    booleanDelegate->setEditable(true);
    ui->tvGroup->setItemDelegateForColumn(2, booleanDelegate);
    ui->tvGroup->setEditTriggers(QAbstractItemView::NoEditTriggers);
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

    if (amsGroupEmployeeModel->isDirty())
    {
        error = error | !amsGroupEmployeeModel->submitAll();
    }

    if (amsGroupAccessModeModel->isDirty())
    {
        error = error | !amsGroupAccessModeModel->submitAll();
    }

    if (error)
    {
        QMessageBox::critical(this, tr("Speichern"),
                tr("Die Änderungen konnten nicht in die Datenbank geschrieben werden."));
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

    amsGroupEmployeeModel = std::make_unique<QMAMSGroupEmployeeModel>(this, db);
    amsGroupEmployeeModel->select();

    employeeModel = std::make_unique<QMEmployeeModel>(this, db);
    employeeModel->select();

    ui->tvGroup->setModel(amsGroupModel.get());
    ui->tvGroup->hideColumn(0);
    ui->tvGroup->resizeColumnsToContents();

    ui->lvAccessMode->setModel(amsAccessModeModel.get());
    ui->lvAccessMode->setModelColumn(1);

    ui->lvEmployee->setModel(employeeModel.get());
    ui->lvEmployee->setModelColumn(1);

    // Build some connections.
    connect(amsGroupModel.get(), &QMAMSGroupModel::dataChanged, this,
            &QMAMSGroupSettingsWidget::settingsChanged);
    connect(ui->tvGroup->selectionModel(),
            &QItemSelectionModel::currentRowChanged, this,
            &QMAMSGroupSettingsWidget::groupSelectionChanged);
    connect(ui->lvAccessMode->selectionModel(),
            &QItemSelectionModel::currentRowChanged, this,
            &QMAMSGroupSettingsWidget::accessModeSelectionChanged);
    connect(ui->lvEmployee->selectionModel(),
            &QItemSelectionModel::selectionChanged, this,
            &QMAMSGroupSettingsWidget::employeeSelectionChanged);
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
        qCritical() << "Cannot find field index of id in AMSAccessMode";
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

void QMAMSGroupSettingsWidget::addEmployee()
{
    // If no employee is selected inform.
    auto selModelIndexes = ui->lvEmployee->selectionModel()->selectedRows(1);
    auto selCount = selModelIndexes.count();

    if (selCount < 1)
    {
        QMessageBox::information(this, tr("Mitarbeiter hinzufügen"), tr("Kein Mitarbeiter ausgewählt."));
        return;
    }

    auto employeePrimaryIdField = employeeModel->fieldIndex("id");
    auto employeeNameFieldIndex = employeeModel->fieldIndex("name");
    if (employeePrimaryIdField < 0 || employeeNameFieldIndex < 0)
    {
        qCritical() << "Cannot find field index of id and/or name in Employee";
        return;
    }

    // Exit if no group has been selected.
    auto groupIndex = ui->tvGroup->currentIndex();
    if (!groupIndex.isValid())
    {
        QMessageBox::information(this, tr("Mitarbeiter hinzufügen"), tr("Keine Gruppe ausgewählt"));
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

    for (int i = 0; i < selModelIndexes.count(); i++)
    {
        auto selModelIndex = selModelIndexes.at(i);

        if (!selModelIndex.isValid())
        {
            qCritical() << "ModelIndex of selected employee not valid";
            continue;
        }

        // Get the primary key and name of the employee.
        auto selRow = selModelIndex.row();

        auto employeeIdModelIndex = employeeModel->index(selRow, employeePrimaryIdField);
        auto employeeId = employeeModel->data(employeeIdModelIndex).toInt();

        auto employeeNameModelIndex = employeeModel->index(selRow, employeeNameFieldIndex);
        auto employeeName = employeeModel->data(employeeNameModelIndex).toString();

        // Search for duplicates.
        if (groupEmployeeProxyContainsEmployee(employeeName))
        {
            // Here we give no message, cause there might be mutiple employees already existing.
            continue;
        }

        // Create record and add.
        auto record = amsGroupEmployeeModel->record();

        record.setValue("amsgroup_name", groupPrimaryId);
        record.setValue("name", employeeId);

        if (!amsGroupEmployeeModel->insertRecord(-1, record) | !amsGroupEmployeeModel->submitAll())
        {
            QMessageBox::warning(this, tr("Mitarbeiter hinzufügen"), tr("Konnte den Mitarbeiter nicht hinzufügen"));
            return;
        }
    }
}

bool QMAMSGroupSettingsWidget::groupEmployeeProxyContainsEmployee(const QString &name)
{
    for (int i = 0; i < amsGroupEmployeeProxyModel->rowCount(); i++)
    {
        auto emplyoeeNameModelIndex = amsGroupEmployeeProxyModel->index(i, 2);
        auto employeeName = amsGroupEmployeeProxyModel->data(emplyoeeNameModelIndex).toString();
        if (employeeName.compare(name) == 0)
        {
            return true;
        }
    }

    return false;
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

void QMAMSGroupSettingsWidget::removeEmployee()
{
    auto employeeModelIndexes = ui->lvGroupEmployee->selectionModel()->selectedRows();

    for (int i = 0; i < employeeModelIndexes.count(); i++)
    {
        auto row = employeeModelIndexes.at(i).row();
        if (!amsGroupEmployeeProxyModel->removeRow(row))
        {
            qCritical() << "Cannot remove employee from group emploee model";
        }
    }

    emitSettingsChanged();
}

void QMAMSGroupSettingsWidget::removeAccessMode()
{
    auto groupAccessModeIndex = ui->lvGroupAccessMode->currentIndex();

    if (!groupAccessModeIndex.isValid())
    {
        QMessageBox::information(this, tr("Gruppe entfernen"), tr("Keine Gruppe ausgewählt."));
        return;
    }

    amsGroupAccessModeProxyModel->removeRow(groupAccessModeIndex.row());

    emitSettingsChanged();
}

void QMAMSGroupSettingsWidget::groupAccessModeSelectionChanged(const QModelIndex &selected,
        const QModelIndex &deselected)
{
    ui->lvAccessMode->reset();
    if (selected.isValid())
    {
        ui->pbRemoveAccessMode->setEnabled(true);
    }
}

void QMAMSGroupSettingsWidget::groupEmployeeSelectionChanged(const QItemSelection &selected,
        const QItemSelection &deselected)
{
    ui->lvEmployee->reset();
    ui->pbAddEmployee->setEnabled(false);

    // Set the number of selected elements.
    auto selCount = ui->lvGroupEmployee->selectionModel()->selectedRows(1).count();
    ui->laSelGroupEmployee->setText(QString("%1").arg(selCount));

    if (selCount < 1)
    {
        ui->pbRemoveEmployee->setEnabled(false);
        return;
    }
    else
    {
        ui->pbRemoveEmployee->setEnabled(true);
    }
}

void QMAMSGroupSettingsWidget::employeeSelectionChanged(const QItemSelection &selected,
        const QItemSelection &deselected)
{
    ui->lvGroupEmployee->reset();
    ui->pbRemoveEmployee->setEnabled(false);

    // Set the number of selected elements.
    auto selCount = ui->lvEmployee->selectionModel()->selectedRows(1).count();
    ui->laSelEmployee->setText(QString("%1").arg(selCount));

    if (selCount < 1)
    {
        ui->pbAddEmployee->setEnabled(false);
        return;
    }
    else
    {
        ui->pbAddEmployee->setEnabled(true);
    }
}

void QMAMSGroupSettingsWidget::selectAllEmployee()
{
    ui->lvGroupEmployee->clearSelection();
    ui->lvEmployee->selectAll();
    ui->pbAddEmployee->setEnabled(true);
    ui->pbRemoveEmployee->setEnabled(false);
}

void QMAMSGroupSettingsWidget::deselectAllEmployee()
{
    ui->lvGroupEmployee->clearSelection();
    ui->lvEmployee->clearSelection();
    ui->pbAddEmployee->setEnabled(false);
    ui->pbRemoveEmployee->setEnabled(false);
}

void QMAMSGroupSettingsWidget::selectAllGroupEmployee()
{
    ui->lvEmployee->clearSelection();
    ui->lvGroupEmployee->selectAll();
    ui->pbAddEmployee->setEnabled(false);
    ui->pbRemoveEmployee->setEnabled(true);
}

void QMAMSGroupSettingsWidget::deselectAllGroupEmployee()
{
    ui->lvGroupEmployee->clearSelection();
    ui->lvEmployee->clearSelection();
    ui->pbAddEmployee->setEnabled(false);
    ui->pbRemoveEmployee->setEnabled(false);
}

void QMAMSGroupSettingsWidget::accessModeSelectionChanged(const QModelIndex &selected, const QModelIndex &deselected)
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

void QMAMSGroupSettingsWidget::changeName()
{
    auto selRows = ui->tvGroup->selectionModel()->selectedRows();
    if (selRows.count() != 1)
    {
        qWarning() << "Wrong number of selected rows";
    }

    auto selRow = selRows.first().row();

    auto nameFieldIndex = amsGroupModel->fieldIndex("amsgroup_name");
    auto nameModelIndex = amsGroupModel->index(selRow, nameFieldIndex);
    auto selName = amsGroupModel->data(nameModelIndex).toString();

    // Get the new group name.
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
        if (!newName.contains(QRegExp("^[a-zA-Z ]+$")) || newName.length() < 6)
        {
            QMessageBox::information(this, tr("Name ändern"),
                    tr("Der Name darf nur Buchstaben und Leerzeichen enthalten und muss mindestens 6 Zeichen "
                       "lang sein."));
            continue;
        }

        // The username should not already exist.
        if (groupContainsGroupname(newName) && selName.compare(newName) != 0)
        {
            QMessageBox::information(this, tr("Name ändern"), tr("Der Name existiert bereits."));
            continue;
        }

        break;
    }

    // Change the name.
    if (!amsGroupModel->setData(nameModelIndex, newName))
    {
        QMessageBox::information(this, tr("Name ändern"), tr("Der Name konnte nicht geändert werden."));
        return;
    }

    emitSettingsChanged();
}

bool QMAMSGroupSettingsWidget::groupContainsGroupname(const QString &groupname)
{
    auto groupnameFieldIndex = amsGroupModel->fieldIndex("amsgroup_name");

    for (int i = 0; i < amsGroupModel->rowCount(); i++)
    {
        auto groupnameModelIndex = amsGroupModel->index(i, groupnameFieldIndex);
        auto tmpGroupname = amsGroupModel->data(groupnameModelIndex).toString();

        if (tmpGroupname.compare(groupname) == 0)
        {
            return true;
        }
    }

    return false;
}

void QMAMSGroupSettingsWidget::changeActiveState()
{
    auto selRows = ui->tvGroup->selectionModel()->selectedRows();
    if (selRows.count() != 1)
    {
        qWarning() << "Wrong number of selected rows";
    }

    auto selRow = selRows.first().row();

    auto activeFieldIndex = amsGroupModel->fieldIndex("amsgroup_active");
    auto activeModelIndex = amsGroupModel->index(selRow, activeFieldIndex);
    auto selActive = amsGroupModel->data(activeModelIndex).toBool();

    // Change the activated state.
    if (!amsGroupModel->setData(activeModelIndex, !selActive))
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

void QMAMSGroupSettingsWidget::groupSelectionChanged(const QModelIndex &selected, const QModelIndex &deselected)
{
    ui->lvAccessMode->reset();
    ui->pbAddAccessMode->setEnabled(false);
    ui->pbRemoveAccessMode->setEnabled(false);

    if (!selected.isValid())
    {
        deactivateGroup();
        return;
    }

    auto selModelIndex = amsGroupModel->index(selected.row(), 2);
    auto selData = amsGroupModel->data(selModelIndex).toString();

    activateGroup(selected.row());
}

void QMAMSGroupSettingsWidget::deactivateGroup()
{
    ui->gbAccessMode->setEnabled(false);
    ui->gbEmployeeAccess->setEnabled(false);
    ui->pbRemoveGroup->setEnabled(false);
    ui->pbChangeActiveState->setEnabled(false);
    ui->pbChangeActiveState->setEnabled(false);
    ui->pbChangeName->setEnabled(false);

    ui->lvGroupAccessMode->setModel(nullptr);
}

void QMAMSGroupSettingsWidget::activateGroup(int selRow)
{
    ui->gbAccessMode->setEnabled(true);
    ui->gbEmployeeAccess->setEnabled(true);
    ui->pbRemoveGroup->setEnabled(true);
    ui->pbChangeActiveState->setEnabled(true);
    ui->pbChangeName->setEnabled(true);

    auto activeFieldIndex = amsGroupModel->fieldIndex("amsgroup_active");
    auto activeModelIndex = amsGroupModel->index(selRow, activeFieldIndex);
    auto selActive = amsGroupModel->data(activeModelIndex).toBool();

    if (selActive)
    {
        ui->pbChangeActiveState->setText(tr("Deaktivieren"));
    }
    else
    {
        ui->pbChangeActiveState->setText(tr("Aktivieren"));
    }

    auto selModelIndex = amsGroupModel->index(selRow, 1);
    auto selData = amsGroupModel->data(selModelIndex).toString();

    // Set access mode proxy model.
    amsGroupAccessModeProxyModel->setSourceModel(amsGroupAccessModeModel.get());
    amsGroupAccessModeProxyModel->setFilterKeyColumn(1);
    amsGroupAccessModeProxyModel->setFilterRegExp(QString("^%1$").arg(selData));
    ui->lvGroupAccessMode->setModel(amsGroupAccessModeProxyModel.get());
    ui->lvGroupAccessMode->setModelColumn(2);

    ui->lvGroupAccessMode->selectionModel()->disconnect(this);
    connect(ui->lvGroupAccessMode->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            &QMAMSGroupSettingsWidget::groupAccessModeSelectionChanged);

    // Set employee proxy model.
    amsGroupEmployeeProxyModel->setSourceModel(amsGroupEmployeeModel.get());
    amsGroupEmployeeProxyModel->setFilterKeyColumn(1);
    amsGroupEmployeeProxyModel->setFilterRegExp(QString("^%1$").arg(selData));
    ui->lvGroupEmployee->setModel(amsGroupEmployeeProxyModel.get());
    ui->lvGroupEmployee->setModelColumn(2);

    ui->lvGroupEmployee->selectionModel()->disconnect(this);
    connect(ui->lvGroupEmployee->selectionModel(), &QItemSelectionModel::selectionChanged, this,
            &QMAMSGroupSettingsWidget::groupEmployeeSelectionChanged);
}
