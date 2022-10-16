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
#include "framework/delegate/qmdatedelegate.h"
#include "framework/dialog/qmextendedselectiondialog.h"
#include "qmemployeedateentry.h"

#include <QSortFilterProxyModel>
#include <QFileDialog>
#include <QCryptographicHash>
#include <QMessageBox>
#include <QKeyEvent>
#include <QTemporaryFile>

QMNewCertificateDialog::QMNewCertificateDialog(const QSqlDatabase &db, QWidget *parent)
    : QMDialog(parent)
    , m_employeeDateModel(std::make_unique<QMEmployeeDateModel>(this))
{
    m_ui = new Ui::QMNewCertificateDialog;
    m_ui->setupUi(this);

    // Table data for employee/date entries.
    m_ui->tvEmployeeDateData->setModel(m_employeeDateModel.get());
    m_ui->tvEmployeeDateData->horizontalHeader()->setMinimumSectionSize(100);
    m_ui->tvEmployeeDateData->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    m_ui->tvEmployeeDateData->setItemDelegateForColumn(1, new DateDelegate());
}

QMNewCertificateDialog::~QMNewCertificateDialog()
{
    delete m_ui;
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
    if (m_ui->cbTrain->findText(m_ui->cbTrain->currentText()) == -1)
    {
        QMessageBox::information(
                this, tr("Nachweis hinzufügen"), tr("Die eingetragene Schulung existiert nicht."));
        return false;
    }
    else
    {
        m_train = m_ui->cbTrain->currentText();
    }

    // Employee
    if (m_ui->rbEmployee->isChecked())
    {
        if (m_ui->cbEmployee->findText(m_ui->cbEmployee->currentText()) == -1)
        {
            QMessageBox::information(
                    this, tr("Nachweis hinzufügen"),
                    tr("Der eingetragene Mitarbeiter existiert nicht."));
            return false;
        }
        else
        {
            m_employee = m_ui->cbEmployee->currentText();
            m_employeeGroup = "";
        }
    }
    else
    {
        if (m_ui->cbEmployeeGroup->findText(m_ui->cbEmployeeGroup->currentText()) == -1)
        {
            QMessageBox::information(
                    this, tr("Nachweis hinzufügen"),
                    tr("Die eingetragene Mitarbeitergruppe existiert nicht."));
            return false;
        }
        else
        {
            m_employee = "";
            m_employeeGroup = m_ui->cbEmployeeGroup->currentText();
        }
    }

    // Datum
    m_trainDate = m_ui->cwTrainDate->selectedDate().toString("yyyyMMdd");

    // Certificate path
    if (m_ui->leCertificatePath->text().isEmpty())
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
    m_ui->cbTrain->setModel(trainViewModel.get());
    m_ui->cbTrain->setModelColumn(1);

    m_ui->cbEmployee->setModel(employeeViewModel.get());
    m_ui->cbEmployee->setModelColumn(1);

    m_ui->cbEmployeeGroup->setModel(employeeGroupViewModel.get());
    m_ui->cbEmployeeGroup->setModelColumn(1);
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
    m_ui->leCertificatePath->setText(fileName);
    m_certPath = fileName;
}

[[maybe_unused]] void QMNewCertificateDialog::switchEmployeeSelection()
{
    if (m_ui->rbEmployee->isChecked())
    {
        m_ui->cbEmployee->setEnabled(true);
        m_ui->cbEmployeeGroup->setEnabled(false);
    }
    else
    {
        m_ui->cbEmployee->setEnabled(false);
        m_ui->cbEmployeeGroup->setEnabled(true);
    }
}

[[maybe_unused]] void QMNewCertificateDialog::appendToTrainDataChanged(int state)
{
    bool checked = state == Qt::CheckState::Checked;
    m_ui->cbCreateTrainData->setEnabled(checked);
    m_ui->cbNoExactDate->setEnabled(checked);
    m_ui->tvEmployeeDateData->setEnabled(checked);
    m_ui->tbExtSelEmployee->setEnabled(checked);
    m_ui->tbRemove->setEnabled(checked);
    m_ui->label_4->setEnabled(checked);
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

        // set default date to selected for the training (this value might be different and editable by the user)
        employeeEntry.trainDate = m_ui->cwTrainDate->selectedDate();

        m_employeeDateModel->addEntry(employeeEntry);
    }
}

[[maybe_unused]] void QMNewCertificateDialog::removeEmployees()
{
    auto selectedEntries = m_ui->tvEmployeeDateData->selectionModel()->selectedRows();
    if (selectedEntries.isEmpty())
    {
        return;
    }

    for (const QModelIndex &modelIndex : selectedEntries)
    {
        m_employeeDateModel->removeEntry(modelIndex.row());
    }
}
