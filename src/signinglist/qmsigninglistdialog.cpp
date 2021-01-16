//
// qmsigninglistdialog.cpp is part of QualificationMatrix
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

#include "qmsigninglistdialog.h"
#include "ui_qmsigninglistdialog.h"
#include "model/qmdatamanager.h"
#include "settings/qmapplicationsettings.h"
#include "signinglist/qmsigninglistdocument.h"

#include <QDate>
#include <QMessageBox>
#include <QKeyEvent>
#include <QSqlRelationalTableModel>
#include <QSqlTableModel>
#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QFileDialog>

#include <QDebug>

QMSigningListDialog::QMSigningListDialog(QWidget *parent)
    : QDialog(parent)
{
    ui = new Ui::QMSigningListDialog;
    ui->setupUi(this);

    updateData();

    auto &settings = QMApplicationSettings::getInstance();
    ui->leImagePath->setText(settings.read("SigningListDialog/ImagePath", "").toString());
}

QMSigningListDialog::~QMSigningListDialog()
{
    delete ui;
}

void QMSigningListDialog::openImage()
{
    auto imagePath = QFileDialog::getOpenFileName(
            this, tr("Öffne Bilddatei"), QDir::homePath(), tr("Image Files (*.png *.jpg *.bmp)"));

    if (imagePath.isEmpty())
    {
        return;
    }
    else
    {
        ui->leImagePath->setText(imagePath);
    }
}

void QMSigningListDialog::accept()
{
    saveSettings();
    printToPDF();

    QDialog::accept();
}

void QMSigningListDialog::closeEvent(QCloseEvent *event)
{
    saveSettings();

    QDialog::closeEvent(event);
}

void QMSigningListDialog::saveSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    // Window settings.
    settings.write("SigningListDialog/Width", width());
    settings.write("SigningListDialog/Height", height());

    // Image path.
    settings.write("SigningListDialog/ImagePath", ui->leImagePath->text());
}

void QMSigningListDialog::updateData()
{
    // Get the model data.
    auto dm = QMDataManager::getInstance();

    employeeModel = dm->getEmployeeModel();
    shiftModel = dm->getShiftModel();
    trainModel = dm->getTrainModel();

    // Set ui elements.
    ui->cbTraining->setModel(trainModel.get());
    ui->cbTraining->setModelColumn(1);

    ui->cbEmployeeGroup->setModel(shiftModel.get());
    ui->cbEmployeeGroup->setModelColumn(1);

    ui->cbSingleEmployee->setModel(employeeModel.get());
    ui->cbSingleEmployee->setModelColumn(1);
}

void QMSigningListDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        return;
    }

    QDialog::keyPressEvent(event);
}

void QMSigningListDialog::removeEmployee()
{

}

void QMSigningListDialog::addEmployee()
{
    QString employeeName = ui->cbSingleEmployee->currentText();

    if (!listContainsEmployee(employeeName))
    {
        ui->lwEmployees->addItem(employeeName);
    }
}

void QMSigningListDialog::addEmployeeFromGroup()
{
    for (int i = 0; i < employeeModel->rowCount(); i++)
    {
        QString employeeName = employeeModel->data(employeeModel->index(i, 1)).toString();
        QString groupName = employeeModel->data(employeeModel->index(i, 2)).toString();

        if (groupName == ui->cbEmployeeGroup->currentText())
        {
            if (!listContainsEmployee(employeeName))
            {
                ui->lwEmployees->addItem(employeeName);
            }
        }
    }
}

bool QMSigningListDialog::listContainsEmployee(const QString &employeeName) const
{
    for (int i = 0; i < ui->lwEmployees->count(); i++)
    {
        if (employeeName == ui->lwEmployees->item(i)->text())
        {
            return true;
        }
    }

    return false;
}

void QMSigningListDialog::printToPDF()
{
    // Set up default printer.
    auto printer = new QPrinter();
    printer->setPageOrientation(QPageLayout::Orientation::Portrait);

    // Call printer dialog.
    QPrintPreviewDialog previewDialog(printer, this);
    connect(
        &previewDialog, &QPrintPreviewDialog::paintRequested, this,
        &QMSigningListDialog::paintPdfRequest);
    previewDialog.exec();
}

void QMSigningListDialog::paintPdfRequest(QPrinter *printer)
{
    // TODO: Support multiple pages when table is too long.
    QMSigningListDocument document;

    // Get all employees of the group.
    QStringList employees;

    if (ui->cbEmployeeGroup->findText(ui->cbEmployeeGroup->currentText()) < 0)
    {
        QMessageBox::warning(
            this, tr("Unterschriftenliste drucken"), tr("Gruppe nicht gültig."));
        return;
    }

    for (int i = 0; i < ui->lwEmployees->count(); i++)
    {
        QString employeeName = ui->lwEmployees->item(i)->text();
        employees.append(employeeName);
    }

    document.setEmployees(employees);
    document.setTrainer(ui->leTrainer->text());
    document.setOrganisationName(ui->leOrganisation->text());
    document.setTrainingContents(ui->leTrainDetails->text());
    document.setTrainingDate(ui->cwDate->selectedDate());
    document.setTrainingName(ui->cbTraining->currentText());
    document.setImagePath(ui->leImagePath->text());
    document.createDocument();

    // Default printer settings.
    document.print(printer);
}
