// qmcertificatedialog.cpp is part of QualificationMatrix
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

#include "qmnewcertificatedialog.h"
#include "ui_qmnewcertificatedialog.h"
#include "qmemployeedatemodel.h"
#include "data/training/qmtrainingviewmodel.h"
#include "data/employee/qmemployeeviewmodel.h"
#include "data/employee/qmshiftviewmodel.h"
#include "settings/qmapplicationsettings.h"
#include "framework/dialog/qmextendedselectiondialog.h"

#include <QSortFilterProxyModel>
#include <QFileDialog>
#include <QCryptographicHash>
#include <QMessageBox>
#include <QKeyEvent>
#include <QTemporaryFile>

QMNewCertificateDialog::QMNewCertificateDialog(QWidget *parent)
    : QMDialog(parent)
    , m_employeeDateModel(std::make_unique<QMEmployeeDateModel>(this))
{
    ui = new Ui::QMNewCertificateDialog;
    ui->setupUi(this);

    // Table data for employee/date entries.
    ui->tvEmployeeDateData->setModel(m_employeeDateModel.get());
}

QMNewCertificateDialog::~QMNewCertificateDialog()
{
    delete ui;
}

void QMNewCertificateDialog::accept()
{
    if (!checkInputData())
    {
        return;
    }

    QMDialog::accept();
}

bool QMNewCertificateDialog::checkInputData()
{
    // Training name
    if (ui->cbTrain->findText(ui->cbTrain->currentText()) == -1)
    {
        QMessageBox::information(
                this, tr("Nachweis hinzufügen"), tr("Die eingetragene Schulung existiert nicht."));
        return false;
    }
    else
    {
        train = ui->cbTrain->currentText();
    }

    // Employee
    if (ui->rbEmployee->isChecked())
    {
        if (ui->cbEmployee->findText(ui->cbEmployee->currentText()) == -1)
        {
            QMessageBox::information(
                    this, tr("Nachweis hinzufügen"),
                    tr("Der eingetragene Mitarbeiter existiert nicht."));
            return false;
        }
        else
        {
            employee = ui->cbEmployee->currentText();
            employeeGroup = "";
        }
    }
    else
    {
        if (ui->cbEmployeeGroup->findText(ui->cbEmployeeGroup->currentText()) == -1)
        {
            QMessageBox::information(
                    this, tr("Nachweis hinzufügen"),
                    tr("Die eingetragene Mitarbeitergruppe existiert nicht."));
            return false;
        }
        else
        {
            employee = "";
            employeeGroup = ui->cbEmployeeGroup->currentText();
        }
    }

    // Datum
    trainDate = ui->cwTrainDate->selectedDate().toString("yyyyMMdd");

    // Certificate path
    if (ui->leCertificatePath->text().isEmpty())
    {
        QMessageBox::information(
                this, tr("Nachweis hinzufügen"), tr("Es wurde kein Zertifikat ausgewählt."));
        return false;
    }

    return true;
}

void QMNewCertificateDialog::saveSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    // Window settings.
    settings.write("NewCertificateDialog/Width", width());
    settings.write("NewCertificateDialog/Height", height());
}

void QMNewCertificateDialog::updateData()
{
    // Get the current database and update data only when it is connected.
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        return;
    }

    auto db = QSqlDatabase::database("default");

    trainViewModel = std::make_unique<QMTrainingViewModel>(this, db);
    trainViewModel->select();

    employeeViewModel = std::make_unique<QMEmployeeViewModel>(this, db);
    employeeViewModel->select();

    employeeGroupViewModel = std::make_unique<QMShiftViewModel>(this, db);
    employeeGroupViewModel->select();

    // Set data to ui elements.
    ui->cbTrain->setModel(trainViewModel.get());
    ui->cbTrain->setModelColumn(1);

    ui->cbEmployee->setModel(employeeViewModel.get());
    ui->cbEmployee->setModelColumn(1);

    ui->cbEmployeeGroup->setModel(employeeGroupViewModel.get());
    ui->cbEmployeeGroup->setModelColumn(1);
}

[[maybe_unused]] void QMNewCertificateDialog::openCertificatePath()
{
    auto fileName = QFileDialog::getOpenFileName(
        this, tr("Nachweis hinzufügen"), QDir::homePath(),
        tr("All files (*.*);;JPEG (*.jpg *.jpeg);;PDF (*.pdf)" ));

    if (fileName.isEmpty())
    {
        return;
    }

    QFile file(fileName);
    file.open(QIODevice::ReadOnly);

    if (!file.isReadable() || !file.exists())
    {
        qWarning() << "certificate file does not exist or is not readable" << fileName;
        return;
    }

    file.close();
    ui->leCertificatePath->setText(fileName);
    certPath = fileName;
}

[[maybe_unused]] void QMNewCertificateDialog::switchEmployeeSelection()
{
    if (ui->rbEmployee->isChecked())
    {
        ui->cbEmployee->setEnabled(true);
        ui->cbEmployeeGroup->setEnabled(false);
    }
    else
    {
        ui->cbEmployee->setEnabled(false);
        ui->cbEmployeeGroup->setEnabled(true);
    }
}

[[maybe_unused]] void QMNewCertificateDialog::appendToTrainDataChanged(int state)
{
    bool checked = state == Qt::CheckState::Checked;
    ui->cbCreateTrainData->setEnabled(checked);
    ui->cbNoExactDate->setEnabled(checked);
    ui->tvEmployeeDateData->setEnabled(checked);
    ui->tbExtSelEmployee->setEnabled(checked);
    ui->tbRemove->setEnabled(checked);
    ui->label_4->setEnabled(checked);
}

[[maybe_unused]] void QMNewCertificateDialog::addEmployees()
{
    m_extSelEmployeeDialog = std::make_unique<QMExtendedSelectionDialog>(this, employeeViewModel.get(), 1);
    connect(m_extSelEmployeeDialog.get(), &QMExtendedSelectionDialog::finished, this,
            &QMNewCertificateDialog::extSelEmployeeFinished);
    m_extSelEmployeeDialog->open();
}

[[maybe_unused]] void QMNewCertificateDialog::extSelEmployeeFinished(int result)
{
    if (result != QDialog::DialogCode::Accepted)
    {
        return;
    }

    auto modelIndexList = m_extSelEmployeeDialog->getFilterSelected();
    if (modelIndexList.isEmpty())
    {
        return;
    }

    auto employeeIdField = employeeViewModel->fieldIndex("id");
    auto employeeNameField = employeeViewModel->fieldIndex("name");

    // Add the selected employees to the data.
    for (const QModelIndex &modelIndex : modelIndexList)
    {
        QMEmployeeDateEntry employeeEntry;

        // Get the name of the employee and the id of the employee.
        auto employeeId = employeeViewModel->data(
                employeeViewModel->index(modelIndex.row(), employeeIdField)).toInt();
        auto employeeName = employeeViewModel->data(
                employeeViewModel->index(modelIndex.row(), employeeNameField)).toString();

        employeeEntry.employeeId = employeeId;
        employeeEntry.employeeName = employeeName;

        m_employeeDateModel->addEntry(employeeEntry);
    }
}
