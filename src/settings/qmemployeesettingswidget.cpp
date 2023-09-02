// qmemployeesettingswidget.cpp is part of QualificationMatrix
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

#include "qmemployeesettingswidget.h"
#include "ui_qmemployeesettingswidget.h"
#include "data/employee/qmemployeemodel.h"
#include "framework/delegate/qmproxysqlrelationaldelegate.h"
#include "framework/delegate/qmbooleandelegate.h"
#include "qmemployeedetailsdialog.h"

#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>
#include <QModelIndexList>

QMEmployeeSettingsWidget::QMEmployeeSettingsWidget(QWidget *parent)
    : QMSettingsWidget(parent, false),
    ui(new Ui::QMEmployeeSettingsWidget),
    employeeFilterModel(new QSortFilterProxyModel(this)),
    employeeActivatedFilterModel(new QSortFilterProxyModel(this))
{
    ui->setupUi(this);

    // Set initial settings for ui elements.
    ui->tvEmployee->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    ui->tvEmployee->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    ui->tvEmployee->verticalHeader()->setVisible(true);
    ui->tvEmployee->setItemDelegateForColumn(2, new QMProxySqlRelationalDelegate());

    auto booleanDelegate = new QMBooleanDelegate(this);
    booleanDelegate->setEditable(false);
    ui->tvEmployee->setItemDelegateForColumn(3, booleanDelegate);
    ui->tvEmployee->setItemDelegateForColumn(4, new QMBooleanDelegate(this));
    ui->tvEmployee->setItemDelegateForColumn(5, new QMBooleanDelegate(this));
    ui->tvEmployee->setItemDelegateForColumn(6, new QMBooleanDelegate(this));
    ui->tvEmployee->setItemDelegateForColumn(7, new QMBooleanDelegate(this));

    ui->tvEmployeeGroups->verticalHeader()->setVisible(true);
    ui->tvEmployeeGroups->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
}

QMEmployeeSettingsWidget::~QMEmployeeSettingsWidget()
{
    delete ui;
}

void QMEmployeeSettingsWidget::updateData()
{
    // Get the current database and update data only when it is connected.
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        return;
    }

    auto db = QSqlDatabase::database("default");

    employeeModel = std::make_unique<QMEmployeeModel>(this, db);
    employeeModel->select();

    employeeGroupModel = std::make_unique<QSqlTableModel>(this, db);
    employeeGroupModel->setTable("EmployeeGroup");
    employeeGroupModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    employeeGroupModel->setHeaderData(1, Qt::Horizontal, tr("Name"));
    employeeGroupModel->sort(1, Qt::AscendingOrder);
    employeeGroupModel->select();

    employeeActivatedFilterModel->setSourceModel(employeeModel.get());
    employeeFilterModel->setSourceModel(employeeActivatedFilterModel);

    // Update the views.
    ui->tvEmployee->setModel(employeeFilterModel);
    ui->tvEmployeeGroups->setModel(employeeGroupModel.get());

    updateTableView();

    // Build connections of the new models.
    connect(employeeModel.get(), &QAbstractItemModel::dataChanged, this, &QMSettingsWidget::settingsChanged);
}

void QMEmployeeSettingsWidget::updateTableView()
{
    employeeActivatedFilterModel->setFilterKeyColumn(3);
    employeeFilterModel->setFilterKeyColumn(1);
    ui->tvEmployee->hideColumn(0);
    ui->tvEmployeeGroups->hideColumn(0);
}


void QMEmployeeSettingsWidget::saveSettings()
{
    if (employeeModel->isDirty())
    {
        employeeModel->submitAll();
    }

    if (employeeGroupModel->isDirty())
    {
        employeeGroupModel->submitAll();
        employeeModel->initModel();
        employeeModel->select();
    }
}

void QMEmployeeSettingsWidget::loadSettings()
{
    updateData();
}

void QMEmployeeSettingsWidget::revertChanges()
{
    // int current = ui->cbEmployee->currentIndex();

    employeeModel->revertAll();
    employeeGroupModel->revertAll();
}

void QMEmployeeSettingsWidget::resetFilter()
{
    ui->leEmployeeFilter->setText("");
}

void QMEmployeeSettingsWidget::updateFilter()
{
    employeeFilterModel->setFilterFixedString(ui->leEmployeeFilter->text());

    if (ui->cbHideDeactivated->isChecked())
    {
        employeeActivatedFilterModel->setFilterFixedString("1");
    }
    else
    {
        employeeActivatedFilterModel->setFilterFixedString("");
    }
}

void QMEmployeeSettingsWidget::showEmployeeDetails()
{
    QModelIndexList indexes = ui->tvEmployee->selectionModel()->selectedRows();

    if (indexes.size() != 1) {
        QMessageBox::critical(
            this, tr("Mitarbeiterdetails"), tr(
                "Es muss genau ein Mitarbeiter in der Tabelle selektiert sein."
                "\n\nDie Aktion wird abgebrochen!"
            ));

        return;
    }

    int row = indexes.first().row();
    QString id = employeeFilterModel->data(employeeFilterModel->index(row, 0)).toString();
    QString name = employeeFilterModel->data(employeeFilterModel->index(row, 1)).toString();
    QString group = employeeFilterModel->data(employeeFilterModel->index(row, 2)).toString();
    bool activated = employeeFilterModel->data(employeeFilterModel->index(row, 3)).toBool();

    if (!activated)
    {
        QMessageBox::information(
                this, tr("Mitarbeiterdetails"),
                tr("Details sind für deaktivierte Mitarbeiter nicht erlaubt."));
        return;
    }

    QMEmployeeDetailsDialog employeeDetailsDialog(id, name, group, activated, this);
    employeeDetailsDialog.updateData();
    employeeDetailsDialog.exec();
}

void QMEmployeeSettingsWidget::deactivate()
{
    QModelIndexList indexes = ui->tvEmployee->selectionModel()->selectedRows();

    if (indexes.size() != 1) {
        QMessageBox::critical(
                this, tr("Mitarbeiterdetails"), tr(
                        "Es muss genau ein Mitarbeiter in der Tabelle selektiert sein."
                        "\n\nDie Aktion wird abgebrochen!"
                ));

        return;
    }

    int row = indexes.first().row();
    QString id = employeeFilterModel->data(employeeFilterModel->index(row, 0)).toString();
    bool activated = employeeFilterModel->data(employeeFilterModel->index(row, 3)).toBool();

    if (!activated)
    {
        return;
    }

    auto answer = QMessageBox::warning(
            this, tr("Mitarbeiter deaktivieren"),
            tr("Ein deaktivierter Mitarbeiter kann nurnoch vom Administrator in der Datenbank"
               " aktiviert werden!\n\nSind Sie sich sicher?"), QMessageBox::Yes | QMessageBox::No);

    if (answer != QMessageBox::Yes)
    {
        return;
    }

    employeeFilterModel->setData(employeeFilterModel->index(row, 3), false);
}

void QMEmployeeSettingsWidget::addEmployee()
{
    // Reset the filter. Otherwise, they might be a new employee no one can see.
    resetFilter();

    // Add a new line.
    if (!employeeFilterModel->insertRow(employeeModel->rowCount())) {
        QMessageBox::critical(
            this, tr("Mitarbeiter hinzufügen"), tr(
                "Der Mitarbeiter konnt enciht hinzugefügt werden. Bitte informieren Sie den"
                " den Entwickler."
            ));

        return;
    }

    // Set the new employee name in edit mode.
    employeeModel->setData(
        employeeModel->index(employeeModel->rowCount() - 1, 3), 1);
    employeeModel->setData(
        employeeModel->index(employeeModel->rowCount() - 1, 1), tr("Name eingeben"));
    ui->tvEmployee->edit(employeeFilterModel->index(employeeModel->rowCount() - 1, 1));

    // Information to settings parent.
    settingsChanged();
}

void QMEmployeeSettingsWidget::addEmployeeGroup()
{
    // Add a new temp row to the data.
    employeeGroupModel->insertRow(employeeGroupModel->rowCount());

    // Set a default group name and start editor.
    employeeGroupModel->setData(
            employeeGroupModel->index(employeeGroupModel->rowCount() - 1, 1), tr("Gruppenname eingeben"));

    // Start editing the name.
    ui->tvEmployeeGroups->scrollToBottom();
    ui->tvEmployeeGroups->edit(employeeGroupModel->index(employeeGroupModel->rowCount() - 1, 1));

    emit settingsChanged();
}

void QMEmployeeSettingsWidget::removeEmployeeGroup()
{
    // Get the selected data index.
    auto selectedIndex = ui->tvEmployeeGroups->selectionModel()->currentIndex();
    if (!selectedIndex.isValid())
    {
        QMessageBox::information(
                this, tr("Mitarbeitergruppe löschen"),
                tr("Es wurde keine gültige Gruppe ausgewählt."));
        return;
    }

    // Get the selected group name.
    auto selectedGroupName = employeeGroupModel->data(employeeGroupModel->index(selectedIndex.row(), 1)).toString();

    // Do not delete when entries in employee data have a reference to the group. This will only
    // search for the name as a text and not for the unique id!
    auto found = false;

    for (int i = 0; i < employeeModel->rowCount(); i++)
    {
        auto employeeGroupName = employeeModel->data(employeeModel->index(i, 2)).toString();

        if (selectedGroupName == employeeGroupName)
        {
            found = true;
            break;
        }
    }

    if (found)
    {
        QMessageBox::critical(
                this, tr("Mitarbeitergruppe löschen"),
                tr("Es existieren Verweise auf die Gruppe in den"
                   " Mitarbeiterdefinitionen. Bitte löschen Sie zuerst die entsprechenden Verweise"
                   " oder ändern Sie deren Gruppenzugehörigkeit."
                   "\n\nDie Aktion wird abgebrochen."));
        return;
    }

    // Delete the entry.
    if (!employeeGroupModel->removeRow(selectedIndex.row()))
    {
        QMessageBox::critical(
                this, tr("Mitarbeitergruppe löschen"),
                tr("Die Mitarbeitergruppe konnte aus einem unbekannten Grund nicht gelöscht"
                   " werden. Bitte informieren Sie den Entwickler zur Fehlerbehebung."));
    }

    emit settingsChanged();
}

void QMEmployeeSettingsWidget::switchDeactivatedVisibility(bool)
{
    updateFilter();
}
