// qmaddmultipletraindatadialog.cpp is part of QualificationMatrix
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

#include "qmaddmultipletraindatadialog.h"
#include "ui_qmaddmultipletraindatadialog.h"
#include "data/training/qmtrainingviewmodel.h"
#include "data/employee/qmemployeegroupviewmodel.h"
#include "data/trainingdata/qmtrainingdatastateviewmodel.h"
#include "data/training/qmtraininggroupviewmodel.h"
#include "data/trainingdata/qmtrainingdatamodel.h"
#include "data/employee/qmemployeeviewmodel.h"

#include <QDebug>
#include <QMessageBox>
#include <QSqlRecord>

QMAddMultipleTrainDataDialog::QMAddMultipleTrainDataDialog(QMTrainingDataModel *trainDataModel, QWidget *parent)
    : QMDialog(parent)
    , ui(new Ui::QMAddMultipleTrainDataDialog)
    , trainDataModel(trainDataModel)
    , employeeId(new QList<int>())
    , employeeName(new QList<QString>())
{
    ui->setupUi(this);

    // Update data data. Calling this method here is dangerous, cause there might be no data
    // data at the moment. There need to be an extra check in this class.
    updateData();
}

QMAddMultipleTrainDataDialog::~QMAddMultipleTrainDataDialog()
{
    delete ui;
    delete employeeId;
    delete employeeName;
}

void QMAddMultipleTrainDataDialog::updateData()
{
    if (trainDataModel == nullptr)
    {
        return;
    }

    // Get the current database and update data only when it is connected.
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        return;
    }

    auto db = QSqlDatabase::database("default");

    trainViewModel = std::make_unique<QMTrainingViewModel>(this, db);
    employeeViewModel = std::make_unique<QMEmployeeViewModel>(this, db);
    employeeGroupViewModel = std::make_unique<QMEmployeeGroupViewModel>(this, db);
    trainGroupViewModel = std::make_unique<QMTrainingGroupViewModel>(this, db);
    trainDataStateViewModel = std::make_unique<QMTrainingDataStateViewModel>(this, db);

    ui->cbTrainState->setModel(trainDataStateViewModel.get());
    ui->cbTrainState->setModelColumn(1);

    ui->cbTrain->setModel(trainViewModel.get());
    ui->cbTrain->setModelColumn(1);

    ui->cbEmployeeGroup->setModel(employeeGroupViewModel.get());
    ui->cbEmployeeGroup->setModelColumn(1);

    ui->cbSingleEmployee->setModel(employeeViewModel.get());
    ui->cbSingleEmployee->setModelColumn(1);
}

void QMAddMultipleTrainDataDialog::accept()
{
    if (ui->lwEmployees->count() < 1)
    {
        QMessageBox::information(this, tr("Mehrfacheinträge erstellen"), tr("Es wurden keine Mitarbeiter ausgewählt."));
        return;
    }

    if (ui->cbTrain->currentIndex() < 0 || ui->cbTrainState->currentIndex() < 0)
    {
        QMessageBox::information(this, tr("Mehrfacheinträge erstellen"), tr("Nicht alle Vorgaben festgelegt."));
        return;
    }

    bool error = false;

    for (int i = 0; i < employeeId->count(); i++)
    {
        // Get id for training and state.
        auto selTrainRow = ui->cbTrain->currentIndex();
        auto selTrainStateRow = ui->cbTrainState->currentIndex();

        auto selTrainIdFieldIndex = trainViewModel->fieldIndex("id");
        auto selTrainStateIdFieldIndex = trainDataStateViewModel->fieldIndex("id");

        auto selTrainIdModelIndex = trainViewModel->index(selTrainRow, selTrainIdFieldIndex);
        auto selTrainStateIdModelIndex = trainDataStateViewModel->index(selTrainStateRow, selTrainStateIdFieldIndex);

        if (selTrainIdModelIndex.row() == -1 || selTrainIdModelIndex.column() == -1 ||
            selTrainStateIdModelIndex.row() == -1 || selTrainStateIdModelIndex.column() == -1)
        {
            error = true;
            continue;
        }

        auto selTrainId = trainViewModel->data(selTrainIdModelIndex).toInt();
        auto selTrainStateId = trainDataStateViewModel->data(selTrainStateIdModelIndex).toInt();

        // Add a new entry to train data data.
        auto record = trainDataModel->record();

        record.setValue("Employee_name_3", employeeId->at(i));
        record.setValue("Train_name_2", selTrainId);
        record.setValue("name", selTrainStateId);
        record.setValue("date", ui->cwTrainDate->selectedDate().toString("yyyy-MM-dd"));

        auto result = trainDataModel->insertRecord(-1, record);

        if (!result)
        {
            error = false;
        }
    }

    if (error)
    {
        QMessageBox::critical(this, tr("Mehrfacheinträge erstellen"),
                tr("Es traten Fehler bei der Erstellung oder bei der Speicherung der Einträge auf."));
    }

    close();
}

void QMAddMultipleTrainDataDialog::addEmployee()
{
    auto selEmployeeRow = ui->cbSingleEmployee->currentIndex();
    auto selEmployeeIdFieldIndex = employeeViewModel->fieldIndex("id");
    auto selEmployeeNameFieldIndex = employeeViewModel->fieldIndex("name");
    auto selEmployeeIdModelIndex = employeeViewModel->index(selEmployeeRow, selEmployeeIdFieldIndex);
    auto selEmployeeNameModelIndex = employeeViewModel->index(selEmployeeRow, selEmployeeNameFieldIndex);

    auto selEmployeeId = employeeViewModel->data(selEmployeeIdModelIndex).toInt();
    auto selEmployeeName = employeeViewModel->data(selEmployeeNameModelIndex).toString();

    // Only add if employee id is not already in list.
    if (employeeId->contains(selEmployeeId))
    {
        return;
    }

    // Add to list and update list.
    employeeId->append(selEmployeeId);
    employeeName->append(selEmployeeName);

    updateEmployeeListWidget();
}

void QMAddMultipleTrainDataDialog::addEmployeeFromGroup()
{
    auto selEmployeeGroupRow = ui->cbEmployeeGroup->currentIndex();
    auto selEmployeeGroupNameFieldIndex = employeeGroupViewModel->fieldIndex("name");
    auto selEmployeeGroupNameModelIndex = employeeGroupViewModel->index(selEmployeeGroupRow, selEmployeeGroupNameFieldIndex);

    auto selEmployeeGroupName = employeeGroupViewModel->data(selEmployeeGroupNameModelIndex).toString();

    // Go through all employee and test for right group.
    for (int i = 0; i < employeeViewModel->rowCount(); i++)
    {
        auto selEmployeeRow = i;
        auto selEmployeeIdFieldIndex = employeeViewModel->fieldIndex("id");
        auto selEmployeeNameFieldIndex = employeeViewModel->fieldIndex("name");
        auto selEmployeeGroupFieldIndex = employeeViewModel->fieldIndex("EmployeeGroup_name_2");
        auto selEmployeeIdModelIndex = employeeViewModel->index(selEmployeeRow, selEmployeeIdFieldIndex);
        auto selEmployeeNameModelIndex = employeeViewModel->index(selEmployeeRow, selEmployeeNameFieldIndex);
        auto selEmployeeGroupModelIndex = employeeViewModel->index(selEmployeeRow, selEmployeeGroupFieldIndex);

        auto selEmployeeId = employeeViewModel->data(selEmployeeIdModelIndex).toInt();
        auto selEmployeeName = employeeViewModel->data(selEmployeeNameModelIndex).toString();
        auto selEmployeeGroup = employeeViewModel->data(selEmployeeGroupModelIndex).toString();

        // If the group is wrong don't add.
        if (selEmployeeGroup.compare(selEmployeeGroupName) != 0)
        {
            continue;
        }

        // Only add if employee id is not already in list.
        if (employeeId->contains(selEmployeeId))
        {
            continue;
        }

        // Add to list and update list.
        employeeId->append(selEmployeeId);
        employeeName->append(selEmployeeName);
    }

    updateEmployeeListWidget();
}

void QMAddMultipleTrainDataDialog::resetList()
{
    employeeId->clear();
    employeeName->clear();

    updateEmployeeListWidget();
}

void QMAddMultipleTrainDataDialog::removeEmployee()
{
    int selRow = ui->lwEmployees->currentRow();

    if (selRow < 0)
    {
        return;
    }

    employeeId->removeAt(selRow);
    employeeName->removeAt(selRow);

    updateEmployeeListWidget();
}

void QMAddMultipleTrainDataDialog::updateEmployeeListWidget()
{
    ui->lwEmployees->clear();
    ui->lwEmployees->addItems(*employeeName);
}
