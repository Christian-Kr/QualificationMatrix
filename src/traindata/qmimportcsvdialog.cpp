// qmimportcsvdialog.cpp is part of QualificationMatrix
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

#include "qmimportcsvdialog.h"
#include "ui_qmimportcsvdialog.h"
#include "data/training/qmtrainingviewmodel.h"
#include "data/employee/qmemployeeviewmodel.h"
#include "data/employee/qmemployeegroupviewmodel.h"
#include "data/trainingdata/qmtrainingdatastateviewmodel.h"
#include "data/trainingdata/qmtrainingdatamodel.h"
#include "data/training/qmtraininggroupviewmodel.h"
#include "framework/qmsqltablemodel.h"

#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QDebug>
#include <QSqlTableModel>
#include <QSqlRelationalTableModel>
#include <QString>
#include <QStringList>
#include <QDate>

QMImportCsvDialog::QMImportCsvDialog(QWidget *parent)
    : QMDialog(parent),
    ui(new Ui::QMImportCsvDialog)
{
    ui->setupUi(this);

    // Update data data. Calling this method here is dangerous, cause there might be no data
    // data at the moment. There need to be an extra check in this class.
    updateData();
}

QMImportCsvDialog::~QMImportCsvDialog()
{
    delete ui;
}

void QMImportCsvDialog::updateData()
{
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
    trainDataModel = std::make_unique<QMTrainingDataModel>(this, db);

    ui->cbDefaultTrainGroup->setModel(trainGroupViewModel.get());
    ui->cbDefaultTrainGroup->setModelColumn(1);

    ui->cbDefaultEmployeeGroup->setModel(employeeGroupViewModel.get());
    ui->cbDefaultEmployeeGroup->setModelColumn(1);
}

void QMImportCsvDialog::openImportFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this, tr("Import-Datei"), QDir::homePath(), tr("Comma-separated values (*.csv)"));
    if (fileName.isEmpty())
    {
        return;
    }

    ui->leImportFile->setText(fileName);
}

void QMImportCsvDialog::openBackupFile()
{
    QString fileName = QFileDialog::getSaveFileName(
        this, tr("Backup-Datei"), QDir::homePath(), tr("Comma-separated values (*.csv)"));
    if (fileName.isEmpty())
    {
        return;
    }

    ui->leBackupFile->setText(fileName);
}

void QMImportCsvDialog::accept()
{
    // Open the certificate to import.
    QFile importFile(ui->leImportFile->text());
    if (!importFile.open(QFile::ReadOnly) || !importFile.exists() || !importFile.isReadable())
    {
        qDebug() << "Function:" << __FUNCTION__ << "-" << ui->leImportFile->text();
        QMessageBox::critical(
            this, tr("Importieren"),
            tr("Die Import-Datei existiert nicht oder ist nicht lesbar. Bitte wählen Sie "
               "eine existierende Datei, die lesbar ist."
               "\n\nDie Aktion wird abgebrochen."));
        return;
    }

    parseCsv(importFile);
}

void QMImportCsvDialog::handleErrorLine(QString &line)
{
    if (!ui->gbcSaveNotImported->isChecked())
    {
        // Setting tells, that nothing should happen with the error line. This might be a problem
        // cause after importing it is not known where the problems occur.
        return;
    }

    QFile backupFile(ui->leBackupFile->text());
    if (!backupFile.open(QFile::Append))
    {
        qCritical() << "ERROR: Could not open certificate for saving.";
    }

    QTextStream backupStream(&backupFile);
    backupStream << line << "\n";
    backupFile.close();
}

bool QMImportCsvDialog::checkBackupFile()
{
    if (ui->gbcSaveNotImported->isChecked())
    {
        QFileInfo fileInfo(ui->leBackupFile->text());
        if (!fileInfo.exists())
        {
            // If the certificate does not exist, try to create it.
            QFile file(ui->leBackupFile->text());
            if (!file.open(QFile::ReadWrite))
            {
                QMessageBox::critical(
                    this, tr("Importieren"),
                    tr("Die Backup-Datei existiert nicht und konnte auch nicht erstellt"
                       " werden. Bitte korrigieren Sie ihre Berechtigungen im Dateisystem"
                       " oder wählen Sie eine existierende Datei mit "
                       " Schreibberechtigungen."
                       "\n\nDie Aktion wird abgebrochen."));
                return false;
            }

            file.close();
        }

        // The certificate might have been created, so update the certificate info object. The certificate must be
        // writable.
        fileInfo = QFileInfo(ui->leBackupFile->text());
        if (!fileInfo.isWritable())
        {
            QMessageBox::critical(
                this, tr("Importieren"),
                tr("Sie verfügen über keine Schreiberechtigungne für die Backup-Datei."
                   " Bitte überprüfen Sie die Berechtigungen."
                   "\n\nDie Aktion wird abgebrochen."));
            return false;
        }
    }

    return true;
}

void QMImportCsvDialog::parseCsv(QFile &importFile)
{
    // Before importing process starts it should be testes wheather the backup certificate exist or not.
    // This test might be redundant, but is important for running a save process. Otherwise backup
    // data are not available and import process might have failures without the ability to detect
    // where it happens. (When a backup ist wanted.)
    if (!checkBackupFile())
    {
        return;
    }

    QTextStream importStream(&importFile);
    int skipLines = 1;

    // There are two ways of reading data. First one is to read in the full certificate and then parse
    // data. This might ba a problem, when the certificate is very big. Second way is to read in line by
    // line and progress every line directly. This might take a bit more time, when the certificate is
    // small, but will be more efficient when the certificate is big.
    // We go on with the second way here. This might change or extend in future.
    while (!importStream.atEnd())
    {
        QString line = importStream.readLine();

        // Skip the first "skipLines" lines.
        if (skipLines > 0)
        {
            skipLines--;
            continue;
        }

        // The format of the certificate should be fixed for now. In future there should be a way to
        // define which data exist in which column.
        //
        // The second point is, that data might have two columns for names. One column for the
        // first and a separate for the second. In this case, there is no equal condition for
        // name strings but two include conditions for a name. This progress should be implemented
        // in future versions.

        QStringList columns = line.split(",");

        // Test for number of columns.
        if ((ui->cbTwoNameColumns->isChecked() && columns.size() != 5) ||
            (!ui->cbTwoNameColumns->isChecked() && columns.size() != 4))
        {
            handleErrorLine(line);
            continue;
        }

        // Get the first column (or first and second) for name and search for right name. At the
        // end, the first (or first and second) will be removed.
        int colEmployeeName = employeeViewModel->fieldIndex("name");
        int colEmployeeId = employeeViewModel->fieldIndex("id");
        int primaryKeyEmployeeName = -1;
        for (int i = 0; i < employeeViewModel->rowCount(); i++)
        {
            QString name = employeeViewModel->data(employeeViewModel->index(i, colEmployeeName)).toString();
            if (ui->cbTwoNameColumns->isChecked())
            {
                // First and last name are separated.
                if (name.contains(columns.at(0)) && name.contains(columns.at(1)))
                {
                    primaryKeyEmployeeName = employeeViewModel->data(
                            employeeViewModel->index(i, colEmployeeId)).toInt();
                    columns.removeFirst();
                    columns.removeFirst();
                    break;
                }
                else if (ui->cbCheckForUmlauts->isChecked())
                {
                    // Check again with umlauts.
                    auto firstName = columns.at(0);
                    firstName = firstName.replace("ae", "ä").replace("ue", "ü").replace("oe", "ö");
                    auto lastName = columns.at(1);
                    lastName = lastName.replace("ae", "ä").replace("ue", "ü").replace("oe", "ö");
                    if (name.contains(firstName) && name.contains(lastName))
                    {
                        primaryKeyEmployeeName = employeeViewModel->data(
                                employeeViewModel->index(i, colEmployeeId)).toInt();
                        columns.removeFirst();
                        columns.removeFirst();
                        break;
                    }
                }
            }
            else
            {
                // One Field for first and last name.
                if (name == columns.at(0))
                {
                    primaryKeyEmployeeName = employeeViewModel
                        ->data(employeeViewModel->index(i, colEmployeeId)).toInt();
                    columns.removeFirst();
                    break;
                }
                else if (ui->cbCheckForUmlauts->isChecked())
                {
                    auto fullName = columns.at(0);
                    fullName = fullName.replace("ae", "ä").replace("ue", "ü").replace("oe", "ö");
                    if (name == fullName)
                    {
                        primaryKeyEmployeeName = employeeViewModel
                            ->data(employeeViewModel->index(i, colEmployeeId)).toInt();
                        columns.removeFirst();
                        break;
                    }
                }
            }
        }

        // Test if a primary key has been found. If not, just give the line to handel error.
        // TODO: Later a non-existence should lead to create the employee.
        if (primaryKeyEmployeeName == -1)
        {
            qDebug() << "MESSAGE: Employee not found";
            handleErrorLine(line);
            continue;
        }

        // Get the first column for training name and search for existing. At the end, remove the
        // the column from the list.
        int colTrainName = trainViewModel->fieldIndex("name");
        int colTrainId = trainViewModel->fieldIndex("id");
        int primaryKeyTrainName = -1;
        for (int i = 0; i < trainViewModel->rowCount(); i++)
        {
            QString name = trainViewModel->data(trainViewModel->index(i, colTrainName)).toString();
            if (name == columns.at(0))
            {
                primaryKeyTrainName = trainViewModel->data(trainViewModel->index(i, colTrainId)).toInt();
                columns.removeFirst();
                break;
            }
        }

        // Test if a primary key has been found. If not, just give the line to handel error.
        // TODO: Later a non-existence should lead to create the train.
        if (primaryKeyTrainName == -1)
        {
            qDebug() << "MESSAGE: Training not found";
            handleErrorLine(line);
            continue;
        }

        // Parsing the date is way more complex. There shouldn't be any " or ' sign. So it should
        // not be declared as a string. The date should also be written in iso format.
        // TODO: Support more locale formats of date.
        auto date = QDate::fromString(columns.at(0), "dd.MM.yyyy");
        columns.removeFirst();
        if (!date.isValid())
        {
            qDebug() << "MESSAGE: Invalid date";
            handleErrorLine(line);
            continue;
        }

        // Get the first column for training name and search for existing. At the end, remove the
        // the column from the list.
        auto colTrainStateName = trainDataStateViewModel->fieldIndex("name");
        auto colTrainStateId = trainDataStateViewModel->fieldIndex("id");
        auto primaryKeyTrainStateName = -1;
        for (int i = 0; i < trainDataStateViewModel->rowCount(); i++)
        {
            QString name = trainDataStateViewModel->data(
                    trainDataStateViewModel->index(i, colTrainStateName)).toString();
            if (name == columns.at(0))
            {
                primaryKeyTrainStateName = trainDataStateViewModel
                    ->data(trainDataStateViewModel->index(i, colTrainStateId)).toInt();
                columns.removeFirst();
                break;
            }
        }

        // Test if a primary key has been found. If not, just give the line to handel error.
        // TODO: Later a non-existence should lead to create the train state.
        if (primaryKeyTrainStateName == -1)
        {
            qDebug() << "MESSAGE: State not found";
            handleErrorLine(line);
            continue;
        }

        // Finally create the data train data entry.
        trainDataModel->insertRow(trainDataModel->rowCount());
        auto rowNew = trainDataModel->rowCount() - 1;

        trainDataModel->setData(trainDataModel->index(rowNew, 1), primaryKeyEmployeeName);
        trainDataModel->setData(trainDataModel->index(rowNew, 2), primaryKeyTrainName);
        trainDataModel->setData(
            trainDataModel->index(rowNew, trainDataModel->fieldIndex("date")),
            date.toString("yyyy-MM-dd"));
        trainDataModel->setData(trainDataModel->index(rowNew, 4), primaryKeyTrainStateName);
    }

    // Accept Actionand close the dialog.
    QDialog::accept();
}
