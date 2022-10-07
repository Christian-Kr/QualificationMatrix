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
#include "data/employee/qmemployeeviewmodel.h"
#include "data/employee/qmshiftviewmodel.h"
#include "data/training/qmtrainingviewmodel.h"
#include "data/trainingdata/qmtrainingdatamodel.h"
#include "data/trainingdata/qmtrainingdatastateviewmodel.h"
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
    , m_duplicatedEmployeeEntries(new QList<QMSigningListEmployeeInfo>())
    , m_trainDataConflictDialog(new QMTrainDataConflictDialog(this))
{
    ui = new Ui::QMSigningListDialog;
    ui->setupUi(this);

    auto &settings = QMApplicationSettings::getInstance();
    ui->leImagePath->setText(settings.read("SigningListDialog/ImagePath", "").toString());

    connect(m_trainDataConflictDialog, &QMTrainDataConflictDialog::finished, this,
            &QMSigningListDialog::trainDataConflictDialogFinished);
}

QMSigningListDialog::~QMSigningListDialog()
{
    delete ui;
    delete m_selectedEmployees;
    delete m_duplicatedEmployeeEntries;
    delete m_trainDataConflictDialog;
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
    if (!checkInput())
    {
        return;
    }

    saveSettings();

    if (ui->cbCreateTrainDataEntries->isChecked())
    {
        // Check permissions for adding training data.
        auto ams = QMAMSManager::getInstance();
        if (!ams->checkPermission(AccessMode::TD_MODE_WRITE))
        {
            QMessageBox::StandardButton ret = QMessageBox::question(this, tr("Nachweise verwalten"),
                    tr("Sie haben nicht die notwendigen Berechtigungen zum Erstellen von Schulungseinträgen. Möchten "
                       "Sie trotzdem fortfahren?"),
                    QMessageBox::Yes | QMessageBox::No);

            if (ret != QMessageBox::Yes)
            {
                return;
            }
            else
            {
                printToPDF();
            }
        }
        else
        {
            createTrainDataEntriesCheck();
        }
    }
    else
    {
        printToPDF();
    }
}

bool QMSigningListDialog::checkInput()
{
    // First of all, test for must have fields.
    if (ui->leTrainDetails->toPlainText().isEmpty())
    {
        QMessageBox::warning(this, tr("Unterschriftenliste erstellen"),
                tr("Bitte Details zu den Schulungsinhalten eingeben!"));
        return false;
    }

    if (ui->leTrainer->text().isEmpty())
    {
        QMessageBox::warning(this, tr("Unterschriftenliste erstellen"),
                tr("Bitte den Schulungsdurchführenden eintragen!"));
        return false;
    }

    if (ui->leOrganisation->text().isEmpty())
    {
        QMessageBox::warning(this, tr("Unterschriftenliste erstellen"),
                tr("Bitte die Schulungsorganisation eintragen!"));
        return false;
    }

    if (ui->lwEmployees->count() < 1)
    {
        QMessageBox::warning(this, tr("Unterschriftenliste erstellen"),
                tr("Bitte mindestens einen Mitarbeiter eintragen!"));
        return false;
    }

    return true;
}

void QMSigningListDialog::createTrainDataEntriesCheck()
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

    // Empty the duplicated entry list and fill it with the found ones.
    m_duplicatedEmployeeEntries->clear();
    for (const QMTrainDataInfo &trainDataInfo : trainDataInfoList)
    {
        QMSigningListEmployeeInfo employeeInfo;
        employeeInfo.id = trainDataInfo.employeeId;

        m_duplicatedEmployeeEntries->append(employeeInfo);
    }

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
        m_trainDataConflictDialog->setTrainingData(ids);
        m_trainDataConflictDialog->open();
    }
    else
    {
        createTrainDataEntries();
        printToPDF();
    }
}

void QMSigningListDialog::trainDataConflictDialogFinished(int result)
{
    if (result == QDialog::Accepted)
    {
        createTrainDataEntries();
    }

    printToPDF();
}

void QMSigningListDialog::createTrainDataEntries()
{
    // Check for and get the database object.
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        qDebug() << "QMSigningListDialog: Cannot open database";
        return;
    }

    auto db = QSqlDatabase::database("default");

    // Finally create entries for the employees that are not already existing within a training data set.
    QMTrainingDataModel trainDataModel(this, db);
    trainDataModel.select();

    // Get current selected training id.
    auto trainRow = ui->cbTraining->currentIndex();
    auto trainIdFieldColumn = m_trainViewModel->fieldIndex("id");
    auto selectedTrainId = m_trainViewModel->data(m_trainViewModel->index(trainRow, trainIdFieldColumn)).toInt();

    // Get current selected date.
    auto selectedDate = ui->cwDate->selectedDate().toString(Qt::DateFormat::ISODate);

    // The train data state can only be the default one for registered. For now, the id is defaulted to 2 which
    // is registered in database. This is a bad handle.
    auto trainDataStateRow = ui->cbTrainingState->currentIndex();
    auto trainDataStateIdFieldColumn = m_trainDataStateViewModel->fieldIndex("id");
    auto selectedTrainDataStateRow = m_trainDataStateViewModel->data(
            m_trainDataStateViewModel->index(trainDataStateRow, trainDataStateIdFieldColumn)).toInt();

    auto error = false;

    for (const QMSigningListEmployeeInfo &employeeInfo : *m_selectedEmployees)
    {
        // Jump over and don't create if an entry for the employee already exist. This test has already been done in
        // createTrainDataEntriesCheck() and the result has been saved to m_duplicatedEmployeeEntries. Because the
        // only variable that might change on duplicate are the employees, the duplicated training data entries might
        // directly correlate with the employee column and therefore a list of employees that are duplicate is
        // sufficient.
        bool duplicate = false;

        for (const QMSigningListEmployeeInfo &duplicatedEmployeeInfo : *m_duplicatedEmployeeEntries)
        {
            if (duplicatedEmployeeInfo.id == employeeInfo.id)
            {
                duplicate = true;
                break;
            }
        }

        if (duplicate)
        {
            continue;
        }

        // Go on with creating a new record.
        auto record = trainDataModel.record();

        // To create a new record, the id's for primary keys have to be entered.
        record.setValue(1, employeeInfo.id);
        record.setValue(2, selectedTrainId);
        record.setValue(3, selectedDate);
        record.setValue(4, selectedTrainDataStateRow);

        auto res = trainDataModel.insertRecord(-1, record);
        if (!res)
        {
            qDebug() << "QMSigningListDialog: Cannot add train data entry";
            error = true;
        }
    }

    if (!trainDataModel.submitAll() || error)
    {
        QMessageBox::warning(this, tr("Erstelle Schulungsdaten"),
                tr("Es sind Fehler beim Erstellen der Schulungsdaten aufgetreten."));
    }
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

    m_trainDataStateViewModel = std::make_unique<QMTrainingDataStateViewModel>(this, db);
    m_trainDataStateViewModel->select();

    m_employeeViewModel = std::make_unique<QMEmployeeViewModel>(this, db);
    m_employeeViewModel->select();

    m_shiftViewModel = std::make_unique<QMShiftViewModel>(this, db);
    m_shiftViewModel->select();

    m_trainViewModel = std::make_unique<QMTrainingViewModel>(this, db);
    m_trainViewModel->select();

    // Set ui elements.
    ui->cbTrainingState->setModel(m_trainDataStateViewModel.get());
    ui->cbTrainingState->setModelColumn(1);

    ui->cbTraining->setModel(m_trainViewModel.get());
    ui->cbTraining->setModelColumn(1);

    ui->cbEmployeeGroup->setModel(m_shiftViewModel.get());
    ui->cbEmployeeGroup->setModelColumn(1);

    ui->cbSingleEmployee->setModel(m_employeeViewModel.get());
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

    // Column names in data
    auto colEmployeeId = m_employeeViewModel->fieldIndex("id");
    auto colEmployeeName = m_employeeViewModel->fieldIndex("name");

    if (colEmployeeId == -1 || colEmployeeName == -1)
    {
        qCritical() << "SigningList: Cannot find field index of employee data.";
        return;
    }

    // Get data values
    auto employeeName = m_employeeViewModel->data(m_employeeViewModel->index(selectedRow, colEmployeeName)).toString();
    auto employeeId = m_employeeViewModel->data(m_employeeViewModel->index(selectedRow, colEmployeeId)).toInt();

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

    // Column names in data
    auto colEmployeeId = m_employeeViewModel->fieldIndex("id");
    auto colEmployeeName = m_employeeViewModel->fieldIndex("name");
    auto colEmployeeGroup = m_employeeViewModel->fieldIndex("Shift_name_2"); // Group name is still 'shift' cause of history.

    if (colEmployeeId == -1 || colEmployeeName == -1 || colEmployeeGroup == -1)
    {
        qCritical() << "SigningList: Cannot find field index of employee data";
        return;
    }

    // Go through all employees and take a look, whether they are in the selected group or not
    for (auto i = 0; i < m_employeeViewModel->rowCount(); i++)
    {
        auto employeeId = m_employeeViewModel->data(m_employeeViewModel->index(i, colEmployeeId)).toInt();
        auto employeeName = m_employeeViewModel->data(m_employeeViewModel->index(i, colEmployeeName)).toString();
        auto employeeGroupName = m_employeeViewModel->data(m_employeeViewModel->index(i, colEmployeeGroup)).toString();

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
    auto contentDesc = m_trainViewModel->data(m_trainViewModel->index(ui->cbTraining->currentIndex(), 5)).toString();

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

[[maybe_unused]] void QMSigningListDialog::createTrainDataEntriesChanged(int state)
{
    if (state == Qt::CheckState::Checked)
    {
        ui->cbTrainingState->setEnabled(true);
    }
    else
    {
        ui->cbTrainingState->setEnabled(false);
    }
}
