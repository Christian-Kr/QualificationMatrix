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
#include "data/trainingdata/qmtrainingdatamodel.h"
#include "data/certificate/qmcertificatemodel.h"
#include "settings/qmapplicationsettings.h"
#include "framework/delegate/qmdatedelegate.h"
#include "framework/dialog/qmextendedselectiondialog.h"
#include "qmemployeedateentry.h"
#include "data/qmdatamanager.h"
#include "framework/qmlistvalidator.h"

#include <QSortFilterProxyModel>
#include <QFileDialog>
#include <QCryptographicHash>
#include <QMessageBox>
#include <QKeyEvent>
#include <QTemporaryFile>
#include <QSqlQuery>
#include <QSqlRecord>

QMNewCertificateDialog::QMNewCertificateDialog(const QSqlDatabase &db, QWidget *parent)
    : QMDialog(parent)
    , m_employeeDateModel(std::make_unique<QMEmployeeDateModel>(this))
    , m_trainViewModel(std::make_unique<QMTrainingViewModel>(this, db))
    , m_employeeViewModel(std::make_unique<QMEmployeeViewModel>(this, db))
    , m_employeeGroupViewModel(std::make_unique<QMShiftViewModel>(this, db))
    , m_certificateModel(std::make_unique<QMCertificateModel>(this, db))
{
    m_ui = new Ui::QMNewCertificateDialog;
    m_ui->setupUi(this);

    // select data from models
    m_trainViewModel->select();
    m_employeeViewModel->select();
    m_employeeGroupViewModel->select();

    // set model to ui elements
    m_ui->cbTrain->setModel(m_trainViewModel.get());
    m_ui->cbTrain->setModelColumn(1);
    m_ui->cbTrain->setValidator(new QMListValidator(*m_trainViewModel, 1, this));
    m_ui->cbTrain->setFocusOutListValidation(true);

    m_ui->cbEmployee->setModel(m_employeeViewModel.get());
    m_ui->cbEmployee->setModelColumn(1);
    m_ui->cbEmployee->setValidator(new QMListValidator(*m_employeeViewModel, 1, this));
    m_ui->cbEmployee->setFocusOutListValidation(true);

    m_ui->cbEmployeeGroup->setModel(m_employeeGroupViewModel.get());
    m_ui->cbEmployeeGroup->setModelColumn(1);
    m_ui->cbEmployeeGroup->setValidator(new QMListValidator(*m_employeeGroupViewModel, 1, this));
    m_ui->cbEmployeeGroup->setFocusOutListValidation(true);

    m_ui->tvEmployeeDateData->setModel(m_employeeDateModel.get());
    m_ui->tvEmployeeDateData->hideColumn(0);

    // set employee date data table ui
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
    // validate input data, to be sure everything has been filled with valid information to add the certificate
    QString errorMessage;
    if (!validateInputData(errorMessage))
    {
        QMessageBox::information(this, tr("Nachweis hinzufügen"), errorMessage);
        return;
    }

    // add the certificate
    int certId = addCertificate();
    if (certId == -1)
    {
        return;
    }

    // should certificate be added?
    if (m_ui->cbAppendToTrainData->isChecked())
    {
        // add certificate to training data entries, or create them is necessary and wanted
        if (!addCertificateTrainingDataEntries(errorMessage, certId))
        {
            QMessageBox::information(this, tr("Nachweis hinzufügen"), errorMessage);
            return;
        }
    }

    QMDialog::accept();
}

bool QMNewCertificateDialog::addCertificateTrainingDataEntries(QString &errorMessage, int certId)
{
    // certification id should be bigger than 0
    if (certId < 1)
    {
        return false;
    }

    // TODO: Implement adding the certificate to the training data entries.

    // get database for running a query
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        qCritical() << "QMNewCertificateDialog: Cannot open database";
        errorMessage = tr("Es konnte keine Verbindung zur Datenbank hergestellt werden.");
        return false;
    }

    auto db = QSqlDatabase::database("default");

    // filter by id and get the right certificate entry
    m_certificateModel->setFilter(QString("id=%1").arg(certId));
    auto rowIdx = m_certificateModel->rowCount() - 1;
    if (rowIdx != 0)
    {
        qCritical() << "QMNewCertificateDialog: Could not find certification id=" << certId;
        errorMessage = tr("Der gesuchte Nachweis konnte nicht gefunden werden.");
        return false;
    }

    // create query template
    QString queryTemplate =
            "SELECT "
            "    TrainData.id as traindata_id, "
            "    TrainData.employee as employee_id, "
            "    TrainData.train as train_id, "
            "    Employee.name as employee_name, "
            "    Train.name as train_name, "
            "    TrainData.date as traindata_date, "
            "    TrainData.state as traindatastate_id, "
            "    TrainDataState.name as traindatastate_name "
            "FROM "
            "    Employee, TrainData, Train, TrainDataState "
            "WHERE "
            "    TrainDataState.id = traindatastate_id AND "
            "    Train.id = train_id AND "
            "    Employee.id = employee_id AND "
            "    employee_id=%1 AND "
            "    train_id=%2"
            "ORDER BY"
            "    traindata_date DESC;";

    // get train information
    auto trainIdFieldIdx = m_trainViewModel->fieldIndex("id");
    auto trainIdIdx = m_trainViewModel->index(m_ui->cbTrain->currentIndex(), trainIdFieldIdx);
    auto trainId = m_trainViewModel->data(trainIdIdx).toInt();

    // go through every employee
    for (auto i = 0; i < m_employeeDateModel->rowCount(); i++)
    {
        // get employee information
        auto employeeDateEntry = m_employeeDateModel->getEntry(i);

        // create and run query
        auto queryString = queryTemplate.arg(employeeDateEntry.employeeId).arg(trainId);
        QSqlQuery query(queryString, db);

        // the first result is the newest (order by traindata_date DESC)
        // get the first query - the other entries are not needed
        if (!query.first())
        {
            // 1) if there is no train data entry for this constelation -> just create it
            query.finish();

            QMTrainingDataModel trainDataModel(this, db);
            auto newRecord = trainDataModel.record();
            newRecord.setValue("employee", employeeDateEntry.employeeId);
            newRecord.setValue("train", trainId);
            newRecord.setValue("date", employeeDateEntry.trainDate.toString(Qt::ISODate));

            // 1 - executed/done; 2 - planned
            newRecord.setValue("state", 1);

            // -1 for row == append to the end
            trainDataModel.insertRecord(-1, newRecord);

            trainDataModel.submitAll();

            // 2) add the certificate to the created train data entry
            qDebug() << "get the id: " << trainDataModel.query().lastInsertId();
        }
        else
        {
            // there is an entry - check whether it is the searched one

        }

        // Close query, to prevent blocking other queries.
        query.finish();
    }

    return true;
}

bool QMNewCertificateDialog::validateInputData(QString &errorMessage)
{
    // search for training name
    if (m_ui->cbTrain->findText(m_ui->cbTrain->currentText()) == -1)
    {
        errorMessage = tr("Die Schulung existiert nicht.");
        return false;
    }

    // search for employee
    if (m_ui->rbEmployee->isChecked())
    {
        if (m_ui->cbEmployee->findText(m_ui->cbEmployee->currentText()) == -1)
        {
            errorMessage = tr("Der Mitarbeiter existiert nicht.");
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
            errorMessage = tr("Die Mitarbeitergruppe existiert nicht.");
            return false;
        }
        else
        {
            m_employee = "";
            m_employeeGroup = m_ui->cbEmployeeGroup->currentText();
        }
    }

    // date
    m_trainDate = m_ui->cwTrainDate->selectedDate().toString("yyyyMMdd");

    // proof the certificate path - just whether it is empty or not
    if (m_ui->leCertificatePath->text().isEmpty())
    {
        errorMessage = tr("Keine Nacheisdatei angegeben.");
        return false;
    }

    return true;
}

void QMNewCertificateDialog::saveSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    // window settings.
    settings.write("NewCertificateDialog/Width", width());
    settings.write("NewCertificateDialog/Height", height());
}

void QMNewCertificateDialog::loadSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    // window settings
    auto width = settings.read("NewCertificateDialog/Width", 400).toInt();
    auto height = settings.read("NewCertificateDialog/Height", 600).toInt();

    resize(width, height);
}

[[maybe_unused]] void QMNewCertificateDialog::openCertificatePath()
{
    auto fileName = QFileDialog::getOpenFileName(this, tr("Nachweis hinzufügen"), QDir::homePath(),
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
    m_extSelEmployeeDialog = std::make_unique<QMExtendedSelectionDialog>(this, m_employeeViewModel.get(), 1);
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

    auto employeeIdField = m_employeeViewModel->fieldIndex("id");
    auto employeeNameField = m_employeeViewModel->fieldIndex("name");

    // Add the selected employees to the data.
    for (const QModelIndex &modelIndex : modelIndexList)
    {
        QMEmployeeDateEntry employeeEntry;

        // Get the name of the employee and the id of the employee.
        auto employeeId = m_employeeViewModel->data(
                m_employeeViewModel->index(modelIndex.row(), employeeIdField)).toInt();
        auto employeeName = m_employeeViewModel->data(
                m_employeeViewModel->index(modelIndex.row(), employeeNameField)).toString();

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

[[maybe_unused]] int QMNewCertificateDialog::addCertificate()
{
    // open the certificate file
    QFile file(m_certPath);
    file.open(QIODevice::ReadOnly);

    if (!file.isReadable() || !file.exists())
    {
        qWarning() << "certificate file does not exist or is not readable" << m_certPath;
        return false;
    }

    // create the hash value as an md5 sum
    auto hash = QString(QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5).toHex());
    file.seek(0);

    // create the new data entry
    m_certificateModel->select();
    auto rowIndex = m_certificateModel->rowCount();
    QFileInfo fileInfo(file.fileName());

    m_certificateModel->insertRow(m_certificateModel->rowCount());

    // Create the name and set it.
    QString name = m_train + "_" + m_employee + m_employeeGroup + "_" + m_trainDate;
    m_certificateModel->setData(m_certificateModel->index(rowIndex, 1), name);
    m_certificateModel->setData(m_certificateModel->index(rowIndex, 2), fileInfo.completeSuffix());
    m_certificateModel->setData(m_certificateModel->index(rowIndex, 5), hash);
    m_certificateModel->setData(m_certificateModel->index(rowIndex, 6), QDate::currentDate().toString("yyyyMMdd"));
    m_certificateModel->setData(m_certificateModel->index(rowIndex, 7), m_trainDate);

    // Handle related to extern/internal. Internal files will be saved directly into the database as a blob. External
    // files will be saved on the file system and a path to that file will be saved into the database.
    // TODO: Make Modes switchable.
    auto dm = QMDataManager::getInstance();

    if (dm->getCertificateLocation() == CertLoc::EXTERNAL)
    {
        auto certificateFileName = saveFileExternal(file);

        if (certificateFileName.isEmpty())
        {
            QMessageBox::warning(this, tr("Nachweis hinzufügen"), tr("Der Nachweis konnte nicht hinzugefügt werden."));
            m_certificateModel->revertRow(rowIndex);
            return false;
        }

        m_certificateModel->setData(m_certificateModel->index(rowIndex, 3), certificateFileName);

        if (!m_certificateModel->submitAll())
        {
            QMessageBox::warning(this, tr("Nachweis hinzufügen"),
                    tr("Der Nachweis konnte hinzugefügt, aber die Tabelle nicht aktualisiert werden. "
                       "Die Datei und der Eintrag werden wieder entfernt."));
            m_certificateModel->revertAll();
            QFile::remove(certificateFileName);

            return -1;
        }
    }
    else
    {
        auto blob = file.readAll();
        file.seek(0);

        if (blob.isEmpty())
        {
            QMessageBox::warning(this, tr("Nachweis hinzufügen"),
                    tr("Der Nachweis konnte nicht hinzugefügt werden. Bitte informieren Sie den Entwickler."));
            m_certificateModel->revertRow(rowIndex);

            return -1;
        }

        m_certificateModel->setData(m_certificateModel->index(rowIndex, 4), blob);
        if (!m_certificateModel->submitAll())
        {
            QMessageBox::warning(this, tr("Nachweis hinzufügen"),
                    tr("Der Nachweis konnte hinzugefügt aber die Tabelle nicht aktualisiert werden. "
                       "Die Datei und der Eintrag werden wieder entfernt."));
            m_certificateModel->revertAll();

            return -1;
        }
    }

    return m_certificateModel->data(m_certificateModel->index(rowIndex, 0)).toInt();
}

QString QMNewCertificateDialog::saveFileExternal(QFile &file)
{
    // for the files placeds in direct vicinity to the database, relative path begins at the database location
    auto db = QSqlDatabase::database("default", false);
    auto dbPath = QFileInfo(db.databaseName()).absolutePath();
    auto certPath = dbPath + QDir::separator() + "certificates";

    // create the certificate path if it does not exist
    if (!QDir(certPath).exists())
    {
        if (!QDir(certPath).mkpath(certPath))
        {
            qWarning() << "cannot create path" << certPath;
            return {};
        }
    }

    // structure for a certificate file
    // certificates/<current year>/<current month>/<current day>/<filename>
    auto insidePath = QDate::currentDate().toString(Qt::ISODate).replace("-", QDir::separator());
    auto fullPath = certPath + QDir::separator() + insidePath;

    if (!QDir(fullPath).exists())
    {
        if (!QDir(fullPath).mkpath(fullPath))
        {
            qWarning() << "cannot create path" << fullPath;
            return {};
        }
    }

    auto certFileInfo = QFileInfo(file.fileName());
    auto hash = QString(QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5).toHex());
    auto fullFileName = fullPath + QDir::separator() + hash + "." + certFileInfo.completeSuffix();

    file.seek(0);

    if (QDir(fullFileName).exists())
    {
        QMessageBox::critical(this, tr("Nachweis hinzufügen"),
                tr("Es existiert bereits eine Datei mit dem Namen. Der gleiche Nachweis kann pro Tag nur einmal "
                   "hinzugefügt werden."), QMessageBox::Button::Ok);
        qWarning() << "file does already exist" << fullFileName;
        return {};
    }

    // copy file to target
    if (!file.copy(fullFileName))
    {
        qWarning() << "cannot copy file to" << fullFileName;
        return {};
    }

    return fullFileName;
}
