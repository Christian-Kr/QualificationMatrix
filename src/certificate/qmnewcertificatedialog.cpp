//
// qmcertificatedialog.cpp is part of QualificationMatrix
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

#include "qmnewcertificatedialog.h"
#include "ui_qmnewcertificatedialog.h"
#include "model/qmdatamanager.h"
#include "settings/qmapplicationsettings.h"

#include <QSqlRelationalTableModel>
#include <QSortFilterProxyModel>
#include <QFileDialog>
#include <QCryptographicHash>
#include <QMessageBox>
#include <QKeyEvent>
#include <QTemporaryFile>

#include <QDebug>

QMNewCertificateDialog::QMNewCertificateDialog(QWidget *parent)
    : QMDialog(parent)
{
    ui = new Ui::QMNewCertificateDialog;
    ui->setupUi(this);

    updateData();
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

    QDialog::accept();
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
            employeeGroup = ui->cbEmployeeGroup->currentText();;
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
    // Get the model data.
    auto dm = QMDataManager::getInstance();
    trainModel = dm->getTrainModel();
    employeeModel = dm->getEmployeeModel();
    employeeGroupModel = dm->getShiftModel();

    // Set model to ui elements.
    ui->cbTrain->setModel(trainModel.get());
    ui->cbTrain->setModelColumn(1);

    ui->cbEmployee->setModel(employeeModel.get());
    ui->cbEmployee->setModelColumn(1);

    ui->cbEmployeeGroup->setModel(employeeGroupModel.get());
    ui->cbEmployeeGroup->setModelColumn(1);
}

void QMNewCertificateDialog::openCertificatePath()
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

void QMNewCertificateDialog::switchEmployeeSelection()
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
