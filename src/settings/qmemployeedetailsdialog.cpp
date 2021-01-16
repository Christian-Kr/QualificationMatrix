//
// employeedetailsdialog.cpp is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify it under the terms of
// the GNU General Public License as published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
// the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with QualificationMatrix.
// If not, see <http://www.gnu.org/licenses/>.
//

#include "qmemployeedetailsdialog.h"
#include "ui_qmemployeedetailsdialog.h"
#include "model/qmdatamanager.h"
#include "delegate/qmproxysqlrelationaldelegate.h"

#include <QDebug>
#include <QInputDialog>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>

QMEmployeeDetailsDialog::QMEmployeeDetailsDialog(
        QString employeeId, QString employeeName, QString employeeGroup, bool activated,
        QWidget *parent)
    : QDialog(parent),
    ui(new Ui::QMEmployeeDetailsDialog),
    id(employeeId),
    name(employeeName),
    group(employeeGroup),
    active(activated),
    funcModel(nullptr),
    employeeFuncModel(nullptr),
    trainModel(nullptr),
    trainExceptionModel(nullptr),
    shiftModel(nullptr),
    employeeFilterModel(new QSortFilterProxyModel()),
    employeeFuncFilterModel(new QSortFilterProxyModel()),
    trainExceptionFilterModel(new QSortFilterProxyModel())
{
    ui->setupUi(this);

    // Set initial settings for ui elements.
    ui->laEmployeeGroup->setText(group);
    ui->laEmployeeName->setText(name);
    ui->laActivated->setText((active) ? tr("Aktiv") : tr("Inaktiv"));

    ui->tvEmployeeFunc->horizontalHeader()->setStretchLastSection(true);
    ui->tvEmployeeFunc->horizontalHeader()->setSectionResizeMode(
            QHeaderView::ResizeToContents);
    ui->tvEmployeeFunc->horizontalHeader()->setVisible(false);
    ui->tvEmployeeFunc->setItemDelegateForColumn(
            2, new QMProxySqlRelationalDelegate());

    ui->tvTrainException->horizontalHeader()->setStretchLastSection(false);
    ui->tvTrainException->horizontalHeader()->setSectionResizeMode(
            QHeaderView::ResizeToContents);
    ui->tvTrainException->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    ui->tvTrainException->setItemDelegateForColumn(
            2, new QMProxySqlRelationalDelegate());
}

QMEmployeeDetailsDialog::~QMEmployeeDetailsDialog()
{
    delete ui;
}

void QMEmployeeDetailsDialog::updateData()
{
    // Get the model data.
    auto dm = QMDataManager::getInstance();

    employeeFuncModel = dm->getEmployeeFuncModel();
    trainModel = dm->getTrainModel();
    funcModel = dm->getFuncModel();
    trainExceptionModel = dm->getTrainExceptionModel();
    shiftModel = dm->getShiftModel();

    employeeFuncFilterModel->setSourceModel(employeeFuncModel.get());
    employeeFuncFilterModel->setFilterKeyColumn(1);
    employeeFuncFilterModel->setFilterFixedString(name);

    trainExceptionFilterModel->setSourceModel(trainExceptionModel.get());
    trainExceptionFilterModel->setFilterKeyColumn(1);
    trainExceptionFilterModel->setFilterFixedString(name);

    // Update the views.
    ui->tvEmployeeFunc->setModel(employeeFuncFilterModel);
    ui->tvEmployeeFunc->hideColumn(0);
    ui->tvEmployeeFunc->hideColumn(1);
    ui->tvEmployeeFunc->hideColumn(3);

    ui->tvTrainException->setModel(trainExceptionFilterModel);
    ui->tvTrainException->hideColumn(0);
    ui->tvTrainException->hideColumn(1);
}

void QMEmployeeDetailsDialog::applyChanges()
{
    employeeFuncModel->submitAll();
    trainExceptionModel->submitAll();
}

void QMEmployeeDetailsDialog::revertChanges()
{
    employeeFuncModel->revertAll();
    trainExceptionModel->revertAll();
}

void QMEmployeeDetailsDialog::accept()
{
    if (!employeeFuncModel->isDirty() && !trainExceptionModel->isDirty())
    {
        QDialog::accept();
        return;
    }

    // Submit changes and close.
    auto res = QMessageBox::question(
        this, tr("Speichern"),
        tr("Sollen die Änderungen jetzt in die Datenbank geschrieben werden?"),
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

    switch (res)
    {
        case QMessageBox::Cancel:
            return;
        case QMessageBox::No:
            revertChanges();
            QDialog::reject();
            break;
        case QMessageBox::Yes:
            applyChanges();
            QDialog::accept();
            break;
        default:
            break;
    }
}

void QMEmployeeDetailsDialog::reject()
{
    revertChanges();
    QDialog::reject();
}

void QMEmployeeDetailsDialog::addFunc()
{
    // There is no need to insert an entry, if there is no func.
    if (funcModel->rowCount() < 1)
    {
        QMessageBox::information(
            this, tr("Funktion"),
            tr("Es gibt keinen Grund, eine Funktion zu einem Mitarbeiter hinzuzufügen,"
               " wenn keine Funktion existiert.\n\nDie Aktion wird abgebrochen."));
        return;
    }

    // Add a new temp row to the model.
    employeeFuncModel->insertRow(employeeFuncModel->rowCount());

    // Insert the id of the selected employee to the new data.
    employeeFuncModel->setData(employeeFuncModel->index(employeeFuncModel->rowCount() - 1, 1), id);

    // Default employee function.
    employeeFuncModel->setData(
        employeeFuncModel->index(employeeFuncModel->rowCount() - 1, 2),
        funcModel->data(funcModel->index(0, 0)));
}

void QMEmployeeDetailsDialog::addTrainException()
{
    // There is no need to insert an entry, if there is no train.
    if (trainModel->rowCount() < 1)
    {
        QMessageBox::information(
            this, tr("Schulungsausnahme"), tr(
                "Es gibt keinen Grund, eine Schulungsausnahme zu definieren, wenn keine"
                " Schulung existiert.\n\nDie Aktion wird abgebrochen."));
        return;
    }

    // Add a new temp row to the model.
    trainExceptionModel->insertRow(trainExceptionModel->rowCount());

    // Insert the id of the selected employee to the new data.
    trainExceptionModel->setData(
        trainExceptionModel->index(trainExceptionModel->rowCount() - 1, 1), id
    );

    // Add default train id.
    trainExceptionModel->setData(
        trainExceptionModel->index(trainExceptionModel->rowCount() - 1, 2),
        trainModel->data(trainModel->index(0, 0)));

    // Add default text.
    trainExceptionModel->setData(
        trainExceptionModel->index(trainExceptionModel->rowCount() - 1, 3),
        tr("Erklärung über den Grund eintragen!"));
}

void QMEmployeeDetailsDialog::removeFunc()
{
    // Remove the func.
    auto idxSel = ui->tvEmployeeFunc->selectionModel()->selectedRows();

    // Exactly one index should be selected.
    if (idxSel.size() != 1)
    {
        QMessageBox::information(
            this, tr("Funktionszugehörigkeit"),
            tr("Bitte selektieren Sie genau eine Funktion um Sie zu entfernen."
               "\n\nDie Aktion wird abgebrochen."));
        return;
    }

    employeeFuncFilterModel->removeRow(idxSel.first().row());
}

void QMEmployeeDetailsDialog::removeTrainException()
{
    // Remove the func.
    auto idxSel = ui->tvTrainException->selectionModel()->selectedRows();

    // Exactly one index should be selected.
    if (idxSel.size() != 1)
    {
        QMessageBox::information(
            this, tr("Schulungsausnahmen"),
            tr("Bitte selektieren Sie genau eine Ausnahme um Sie zu entfernen.\n\nDie Aktion wird"
               " abgebrochen."));
        return;
    }

    trainExceptionFilterModel->removeRow(idxSel.first().row());
}
