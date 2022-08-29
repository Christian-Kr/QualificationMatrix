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
#include "model/qmtrainingdatamodel.h"
#include "settings/qmapplicationsettings.h"
#include "signinglist/qmsigninglistdocument.h"
#include "traindataconflict/qmtraindataconflictdialog.h"
#include "ams/qmamsmanager.h"

#include <QMessageBox>
#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QFileDialog>
#include <QSqlQuery>
#include <QSqlRecord>

QMSigningListDialog::QMSigningListDialog(QWidget *parent)
    : QMDialog(parent)
    , m_selectedEmployees(new QList<QMSigningListEmployeeInfo>())
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
    delete m_selectedEmployees;
}

[[maybe_unused]] void QMSigningListDialog::openImage()
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

    if (ui->cbCreateTrainDataEntries->isChecked())
    {
        // Check permissions for adding training data.
        auto ams = QMAMSManager::getInstance();
        if (!ams->checkPermission(AccessMode::TD_MODE_WRITE))
        {
            QMessageBox::StandardButton ret = QMessageBox::question(this, tr("Nachweise verwalten"),
                    tr("Sie haben nicht die notwendigen Berechtigungen zum Erstellen von Schulungseinträgen. Möchtest "
                       "Sie trotzdem fortfahren?"),
                    QMessageBox::Yes | QMessageBox::No);

            if (ret != QMessageBox::Yes)
            {
                return;
            }
        }
        else
        {
            createTrainDataEntries();
        }
    }

    printToPDF();
}

void QMSigningListDialog::createTrainDataEntries()
{
    // Run a query to get all exisiting training data with respect to the training, employee and date.
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        qDebug() << "QMSigningListDialog: Cannot open database";
        return;
    }

    auto db = QSqlDatabase::database("default");

    QString strTrainDataEntriesQuery =
            "SELECT "
            "   TrainData.id as traindata_id, "
            "   TrainData.employee as employee_id, "
            "   TrainData.train as train_id, "
            "   Employee.name as employee_name, "
            "   Train.name as train_name, "
            "   TrainData.date as traindata_date, "
            "   TrainData.state as traindatastate_id, "
            "   TrainDataState.name as traindatastate_name "
            "FROM "
            "   Employee, TrainData, Train, TrainDataState "
            "WHERE " 
            "   TrainDataState.id = traindatastate_id AND "
            "   Train.id = train_id AND "
            "   Employee.id = employee_id AND "
            "   employee_id IN (" + getSelectedEmployeeIds().join(",") + ") AND "
            "   train_name = '" + ui->cbTraining->currentText() + "' AND "
            "   TrainData.date = '" + ui->cwDate->selectedDate().toString(Qt::DateFormat::ISODate) + "'; ";

    QSqlQuery query(strTrainDataEntriesQuery, db);

    // Save the query result to objects with its information.
    QList<QMTrainDataInfo> trainDataInfoList;

    while (query.next())
    {
        QSqlRecord record = query.record();

        QMTrainDataInfo trainDataInfo;
        trainDataInfo.id = record.value("traindata_id").toInt();
        trainDataInfo.trainId = record.value("train_id").toInt();
        trainDataInfo.employeeId = record.value("employee_id").toInt();
        trainDataInfo.trainstateId = record.value("traindatastate_id").toInt();

        trainDataInfoList.append(trainDataInfo);
    }

    // Close query, to prevent blocking other queries.
    query.finish();

    // Every entry that is part of trainDataInfoList conflicts with the process to create a new training data entry.
    // As a result, these entries won't be created. The user has the possiblities to view the list of not created
    // entries before the process goes on.
    if (!trainDataInfoList.isEmpty())
    {
        QMessageBox::information(this, tr("Erstelle Schulungsdaten"), 
                tr("Es wurden existierende Schulungsdaten für Mitarbeiter gefunden, die zu den hier einzutragenden "
                "Daten passen. Nachfolgend wird ein Dialog mit den Einträgen angezeigt, die nicht erstellt werden, "
                "wenn Sie fortfahren."));

        // Create a temporary list of ids for every conflict entry, which is needed for the conflict dialog.
        QList<int> ids;
        for (const QMTrainDataInfo &trainDataInfo : trainDataInfoList)
        {
            ids.append(trainDataInfo.id);
        }

        // Show the conflict dialog and let user decide how to handle them.
        // TODO: Implement!
        QMTrainDataConflictDialog trainDataConflictDialog(this);
        trainDataConflictDialog.setTrainingData(ids);
        trainDataConflictDialog.exec();
    }

    // Finally create entries for the employees that are not already existing with a training data set.
    QMTrainingDataModel trainDataModel(this, db);
    trainDataModel.select();

    // TODO: Add all entries.
    // auto newRecord = trainDataModel.record();

    // // To create a new record, the id's for primary keys have to be entered.
    // newRecord.setValue(1, employeeViewModel->data(employeeViewModel->index(0, 0)));
    // newRecord.setValue(2, trainViewModel->data(trainViewModel->index(0, 0)));
    // newRecord.setValue(3, "2020-01-01");
    // //newRecord.setValue(4, trainDataStateViewModel->data(trainDataStateViewModel->index(0, 0)));
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

[[maybe_unused]] void QMSigningListDialog::clearList()
{
    ui->lwEmployees->clear();
}

[[maybe_unused]] void QMSigningListDialog::removeEmployee()
{
    auto row = ui->lwEmployees->currentRow();
    if (row == -1)
    {
        return;
    }

    if (row > m_selectedEmployees->size() - 1)
    {
        qCritical() << "SigningList: List widget seem to be bigger than internal selected employee list";
        return;
    }

    m_selectedEmployees->takeAt(row);
    ui->lwEmployees->takeItem(row);
}

void QMSigningListDialog::addEmployee()
{
    // Selected row number from combo box
    auto selectedRow = ui->cbSingleEmployee->currentIndex();

    if (selectedRow == -1)
    {
        return;
    }

    // Column names in model
    auto colEmployeeId = employeeViewModel->fieldIndex("id");
    auto colEmployeeName = employeeViewModel->fieldIndex("name");

    if (colEmployeeId == -1 || colEmployeeName == -1)
    {
        qCritical() << "SigningList: Cannot find field index of employee model.";
        return;
    }

    // Get model values
    auto employeeName = employeeViewModel->data(employeeViewModel->index(selectedRow, colEmployeeName)).toString();
    auto employeeId = employeeViewModel->data(employeeViewModel->index(selectedRow, colEmployeeId)).toInt();

    if (employeeId < 1)
    {
        qCritical() << "SigningList: Cannot get a valid employee id";
        return;
    }

    if (!listContainsEmployee(employeeId))
    {
        QMSigningListEmployeeInfo employeeInfo;
        employeeInfo.id = employeeId;
        employeeInfo.name = employeeName;

        m_selectedEmployees->append(employeeInfo);
        ui->lwEmployees->addItem(employeeName);
    }
}

[[maybe_unused]] void QMSigningListDialog::addEmployeeFromGroup()
{
    // Stop if nothing has been selected
    if (ui->cbEmployeeGroup->currentText().isEmpty() || ui->cbEmployeeGroup->currentIndex() == -1)
    {
        return;
    }

    // Column names in model
    auto colEmployeeId = employeeViewModel->fieldIndex("id");
    auto colEmployeeName = employeeViewModel->fieldIndex("name");
    auto colEmployeeGroup = employeeViewModel->fieldIndex("Shift_name_2"); // Group name is still 'shift' cause of history.

    if (colEmployeeId == -1 || colEmployeeName == -1 || colEmployeeGroup == -1)
    {
        qCritical() << "SigningList: Cannot find field index of employee model";
        return;
    }

    // Go through all employees and take a look, whether they are in the selected group or not
    for (auto i = 0; i < employeeViewModel->rowCount(); i++)
    {
        auto employeeId = employeeViewModel->data(employeeViewModel->index(i, colEmployeeId)).toInt();
        auto employeeName = employeeViewModel->data(employeeViewModel->index(i, colEmployeeName)).toString();
        auto employeeGroupName = employeeViewModel->data(employeeViewModel->index(i, colEmployeeGroup)).toString();

        // Only strings will be compared for now and no ids
        if (employeeGroupName == ui->cbEmployeeGroup->currentText())
        {
            if (!listContainsEmployee(employeeId))
            {
                QMSigningListEmployeeInfo employeeInfo;
                employeeInfo.id = employeeId;
                employeeInfo.name = employeeName;

                m_selectedEmployees->append(employeeInfo);
                ui->lwEmployees->addItem(employeeName);
            }
        }
    }
}

bool QMSigningListDialog::listContainsEmployee(const int &employeeId) const
{
    for (const QMSigningListEmployeeInfo &employeeInfo : *m_selectedEmployees)
    {
        if (employeeId == employeeInfo.id)
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

[[maybe_unused]] void QMSigningListDialog::trainingChanged()
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

QStringList QMSigningListDialog::getSelectedEmployeeIds() const
{
    QStringList tmpLst;
    for (const QMSigningListEmployeeInfo &employeeInfo : *m_selectedEmployees)
    {
        tmpLst.append(QString::number(employeeInfo.id));
    }

    return tmpLst;
}
