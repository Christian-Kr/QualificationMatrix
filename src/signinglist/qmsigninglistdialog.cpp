// qmsigninglistdialog.cpp is part of QualificationMatrix
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

#include "qmsigninglistdialog.h"
#include "ui_qmsigninglistdialog.h"
#include "model/view/qmemployeeviewmodel.h"
#include "model/view/qmshiftviewmodel.h"
#include "model/view/qmtrainingviewmodel.h"
#include "settings/qmapplicationsettings.h"
#include "signinglist/qmsigninglistdocument.h"

#include <QDate>
#include <QMessageBox>
#include <QSqlTableModel>
#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QFileDialog>

#include <QDebug>

QMSigningListDialog::QMSigningListDialog(QWidget *parent)
    : QMDialog(parent)
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
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        return;
    }

    auto db = QSqlDatabase::database("default");

    employeeViewModel = std::make_unique<QMEmployeeViewModel>(this, db);
    employeeViewModel->select();

    shiftViewModel = std::make_unique<QMShiftViewModel>(this, db);
    shiftViewModel->select();

    trainViewModel = std::make_unique<QMTrainingViewModel>(this, db);
    trainViewModel->select();

    // Set ui elements.
    ui->cbTraining->setModel(trainViewModel.get());
    ui->cbTraining->setModelColumn(1);

    ui->cbEmployeeGroup->setModel(shiftViewModel.get());
    ui->cbEmployeeGroup->setModelColumn(1);

    ui->cbSingleEmployee->setModel(employeeViewModel.get());
    ui->cbSingleEmployee->setModelColumn(1);
}

void QMSigningListDialog::clearList()
{
    ui->lwEmployees->clear();
}

void QMSigningListDialog::removeEmployee()
{
    auto row = ui->lwEmployees->currentRow();
    if (row < 0)
    {
        return;
    }

    ui->lwEmployees->takeItem(row);
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
    for (int i = 0; i < employeeViewModel->rowCount(); i++)
    {
        QString employeeName = employeeViewModel->data(employeeViewModel->index(i, 1)).toString();
        QString groupName = employeeViewModel->data(employeeViewModel->index(i, 2)).toString();

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
    // First of all, test for must have fields.
    if (ui->leTrainDetails->toPlainText().isEmpty())
    {
        QMessageBox::warning(this, tr("Unterschriftenliste erstellen"),
            tr("Bitte Details zu den Schulungsinhalten eingeben!"));
        return;
    }

    if (ui->leTrainer->text().isEmpty())
    {
        QMessageBox::warning(this, tr("Unterschriftenliste erstellen"),
             tr("Bitte der Schulungsdurchführenden eintragen!"));
        return;
    }

    if (ui->leOrganisation->text().isEmpty())
    {
        QMessageBox::warning(this, tr("Unterschriftenliste erstellen"),
                             tr("Bitte die Schulungsorganisation eintragen!"));
        return;
    }

    if (ui->lwEmployees->count() < 1)
    {
        QMessageBox::warning(this, tr("Unterschriftenliste erstellen"),
                             tr("Bitte die mindestens einen Mitarbeiter eintragen!"));
        return;
    }

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

    document.setEmptyEmployees(ui->spEmpty->value());
    document.setEmployees(employees);
    document.setTrainer(ui->leTrainer->text());
    document.setOrganisationName(ui->leOrganisation->text());
    document.setTrainingContents(ui->leTrainDetails->toPlainText());
    document.setTrainingDate(ui->cwDate->selectedDate());
    document.setTrainingName(ui->cbTraining->currentText());
    document.setImagePath(ui->leImagePath->text());
    document.setSortType((EmployeeSort) ui->cbSort->currentIndex());
    document.createDocument();

    // Default printer settings.
    document.print(printer);
}

void QMSigningListDialog::trainingChanged()
{
    auto contentDesc = trainViewModel->data(trainViewModel->index(ui->cbTraining->currentIndex(), 5)).toString();

    if (contentDesc.isEmpty())
    {
        return;
    }

    if (!ui->leTrainDetails->toPlainText().isEmpty())
    {
        auto res = QMessageBox::question(this, tr("Unterschriftenliste"),
            tr("Soll der existierende Inhalt der Schulungsdetails durch die Standardbeschreibung"
               " in der Datenbank ersetzt werden?"), QMessageBox::Yes | QMessageBox::No);

        if (res == QMessageBox::No)
        {
            return;
        }
    }

    ui->leTrainDetails->setPlainText(contentDesc);
}
