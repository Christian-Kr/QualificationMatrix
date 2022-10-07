// qmtraindatawidget.cpp is part of QualificationMatrix
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

#include "qmtraindatawidget.h"
#include "ui_qmtraindatawidget.h"
#include "data/qmdatamanager.h"
#include "data/employee/qmemployeeviewmodel.h"
#include "data/training/qmtrainingviewmodel.h"
#include "data/trainingdata/qmtrainingdatamodel.h"
#include "data/trainingdata/qmtrainingdatastateviewmodel.h"
#include "data/trainingdata/qmtraindatacertificatemodel.h"
#include "data/trainingdata/qmtraindatacertificateviewmodel.h"
#include "framework/delegate/qmproxysqlrelationaldelegate.h"
#include "framework/delegate/qmdatedelegate.h"
#include "certificate/qmcertificatedialog.h"
#include "qmimportcsvdialog.h"
#include "settings/qmapplicationsettings.h"
#include "qmaddmultipletraindatadialog.h"
#include "ams/qmamsmanager.h"

#include <QMessageBox>
#include <QSqlRecord>
#include <QSqlField>
#include <QModelIndexList>
#include <QSortFilterProxyModel>
#include <QItemSelection>
#include <QSqlQuery>
#include <QSqlError>

#include <QDebug>

QMTrainDataWidget::QMTrainDataWidget(QWidget *parent)
    : QMWinModeWidget(parent)
    , ui(new Ui::QMTrainDataWidget)
    , multiEditCertiId(-1)
{
    ui->setupUi(this);

    // Set ui component settings.
    ui->tvTrainData->horizontalHeader()->setStretchLastSection(false);
    ui->tvTrainData->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tvTrainData->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

    // Default settings on start.
    ui->deDateFilterTo->setDate(QDate::currentDate());
    ui->dwTrainDataCertificates->setVisible(false);
    ui->dwMultiEdit->setVisible(false);
    updateMultiEditEnabledState();

    // If do not have the permission, make a lot of stuff disable.
    // Permission check.
    auto amsManager = QMAMSManager::getInstance();
    if (!amsManager->checkPermission(AccessMode::TD_MODE_WRITE))
    {
        ui->tvTrainData->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->dwMultiEdit->setEnabled(false);
        ui->pbAddMultiple->setEnabled(false);
        ui->pbAddSingle->setEnabled(false);
        ui->pbExecMultiEdit->setEnabled(false);
        ui->pbDeleteSelected->setEnabled(false);
        ui->tbAddCertificate->setEnabled(false);
        ui->tbRemoveCertificate->setEnabled(false);
    }

    // Load settings on start.
    loadSettings();
}

QMTrainDataWidget::~QMTrainDataWidget()
{
    // Before deleting the object, save settings.
    saveSettings();

    delete ui;
}

void QMTrainDataWidget::loadSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    // ui->splitter->restoreState(settings.read("TrainData/CertificatesSplitter").toByteArray());
}

void QMTrainDataWidget::saveSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    // settings.write("TrainData/CertificatesSplitter", ui->splitter->saveState());
}

void QMTrainDataWidget::importCsv()
{
    QMImportCsvDialog importDialog(this);
    importDialog.exec();
}


void QMTrainDataWidget::executeMultiEdit()
{
    auto modelIndexList = ui->tvTrainData->selectionModel()->selectedRows();
    if (modelIndexList.size() <= 1)
    {
        return;
    }

    // Ask to be sure.
    auto res = QMessageBox::question(this, tr("Mehrfachänderung"),
            tr("Bist du dir sicher, dass du die Änderungen in die Datenbank schreiben möchtest? Die Rückstellung bei "
               "fehlerhaften Eintragen ist aufwendig und muss manuell durchgeführt werden."),
            QMessageBox::Yes | QMessageBox::No);
    if (res != QMessageBox::Yes)
    {
        return;
    }

    for (int i = 0; i < modelIndexList.size(); i++)
    {
        auto modelIndex = modelIndexList.at(i);
        auto modelIndexIdFieldIndex = trainDataModel->fieldIndex("id");
        auto modelIndexTrainFieldIndex = trainDataModel->fieldIndex("Train_name_2");
        auto modelIndexEmployeeFieldIndex = trainDataModel->fieldIndex("Employee_name_3");
        auto modelIndexStateFieldIndex = trainDataModel->fieldIndex("name");
        auto dateFieldIndex = trainDataModel->fieldIndex("date");

        // Set properties...
        if (ui->cbSetTrain->isChecked())
        {
            auto selTrainRow = ui->cbTrain->currentIndex();
            auto selTrainIdFieldIndex = trainViewModel->fieldIndex("id");
            auto selTrainIdModelIndex = trainViewModel->index(selTrainRow, selTrainIdFieldIndex);
            auto selTrainId = trainViewModel->data(selTrainIdModelIndex).toInt();

            auto trainDataTrainModelIndex = trainDataModel->index(modelIndex.row(), modelIndexTrainFieldIndex);
            auto ok = trainDataModel->setData(trainDataTrainModelIndex, selTrainId);

            if (!ok)
            {
                emit warnMessageAvailable(tr("Cannot den Eintrag Nr. %1 nicht ändern.").arg(modelIndex.row()));
            }
        }
        if (ui->cbSetName->isChecked())
        {
            auto selNameRow = ui->cbName->currentIndex();
            auto selNameIdFieldIndex = employeeViewModel->fieldIndex("id");
            auto selNameIdModelIndex = employeeViewModel->index(selNameRow, selNameIdFieldIndex);
            auto selNameId = employeeViewModel->data(selNameIdModelIndex).toInt();

            auto trainDataNameModelIndex = trainDataModel->index(modelIndex.row(), modelIndexEmployeeFieldIndex);
            auto ok = trainDataModel->setData(trainDataNameModelIndex, selNameId);

            if (!ok)
            {
                emit warnMessageAvailable(tr("Kann den Eintrag Nr. %1 nicht ändern.").arg(modelIndex.row()));
            }
        }
        if (ui->cbSetState->isChecked())
        {
            auto selStateRow = ui->cbTrainState->currentIndex();
            auto selStateIdFieldIndex = trainDataStateViewModel->fieldIndex("id");
            auto selStateIdModelIndex = trainDataStateViewModel->index(selStateRow, selStateIdFieldIndex);
            auto selStateId = trainDataStateViewModel->data(selStateIdModelIndex).toInt();

            auto trainDataStateModelIndex = trainDataModel->index(modelIndex.row(), modelIndexStateFieldIndex);
            auto ok = trainDataModel->setData(trainDataStateModelIndex, selStateId);

            if (!ok)
            {
                emit warnMessageAvailable(tr("Kann den Eintrag Nr. %1 nicht ändern.").arg(modelIndex.row()));
            }
        }
        if (ui->cbSetDate->isChecked())
        {
            auto dateISO = ui->cwTrainDate->selectedDate().toString("yyyy-MM-dd");

            auto trainDataDateModelIndex = trainDataModel->index(modelIndex.row(), dateFieldIndex);
            auto ok = trainDataModel->setData(trainDataDateModelIndex, dateISO);

            if (!ok)
            {
                emit warnMessageAvailable(tr("Kann den Eintrag Nr. %1 nicht ändern.").arg(modelIndex.row()));
            }
        }
        if (ui->cbAddMultiCertificate->isChecked())
        {
            if (multiEditCertiId < 0)
            {
                emit warnMessageAvailable(tr("Kann das Zertifikat nicht anhängen."));
            }

            auto trainDataIdModelIndex = trainDataModel->index(modelIndex.row(), modelIndexIdFieldIndex);
            auto trainDataId = trainDataModel->data(trainDataIdModelIndex).toInt();
            auto certId = multiEditCertiId;

            // Test whether the certificate id already exist.
            trainDataCertViewModel->setFilter(QString("train_data=%1").arg(trainDataId));
            auto exist = false;
            for (int j = 0; j < trainDataCertViewModel->rowCount(); j++)
            {
                auto tmpCertId = trainDataCertViewModel->data(trainDataCertViewModel->index(j, 2)).toInt();
                if (tmpCertId == certId)
                {
                    emit infoMessageAvailable(tr("Nachweis ist bereits angehängt"));
                    exist = true;
                    break;
                }
            }

            if (exist)
            {
                continue;
            }

            auto row = trainDataCertModel->rowCount();

            if (trainDataCertModel->insertRow(row))
            {
                qWarning() << "cannot add a new row";
                qWarning() << trainDataCertModel->lastError().text();
            }

            trainDataCertModel->setData(trainDataCertModel->index(row, 1), trainDataId);
            trainDataCertModel->setData(trainDataCertModel->index(row, 2), certId);

            if (trainDataCertModel->submitAll())
            {
                qWarning() << "cannot submit changes to database";
                qWarning() << trainDataCertModel->lastError().text();
            }

            trainDataCertModel->select();
            trainDataCertViewModel->select();
        }
    }
}

void QMTrainDataWidget::updateData()
{
    // Get the current database and update data only when it is connected.
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        return;
    }

    auto db = QSqlDatabase::database("default");

    employeeViewModel = std::make_unique<QMEmployeeViewModel>(this, db);
    employeeViewModel->select();

    trainViewModel = std::make_unique<QMTrainingViewModel>(this, db);
    trainViewModel->select();

    trainDataModel = std::make_unique<QMTrainingDataModel>(this, db);
    trainDataModel->setLimit(100);

    trainDataStateViewModel = std::make_unique<QMTrainingDataStateViewModel>(this, db);
    trainDataStateViewModel->select();

    trainDataCertModel = std::make_unique<QMTrainDataCertificateModel>(this, db);
    trainDataCertModel->select();

    trainDataCertViewModel = std::make_unique<QMTrainDataCertificateViewModel>(this, db);
    trainDataCertViewModel->select();

    // Set models to the corresponding view.
    ui->tvTrainData->setModel(trainDataModel.get());
    ui->cbFilterEmployee->setModel(employeeViewModel.get());
    ui->cbFilterTrain->setModel(trainViewModel.get());
    ui->cbFilterState->setModel(trainDataStateViewModel.get());
    ui->tvCertificates->setModel(trainDataCertViewModel.get());

    ui->cbName->setModel(employeeViewModel.get());
    ui->cbTrainState->setModel(trainDataStateViewModel.get());
    ui->cbTrain->setModel(trainViewModel.get());

    ui->cbName->setModelColumn(1);
    ui->cbTrainState->setModelColumn(1);
    ui->cbTrain->setModelColumn(1);

    updateTableView();

    // If have to many entries available, informa the user.
    trainDataModel->select();
    if (trainDataModel->rowCount() >= trainDataModel->getLimit())
    {
        emit warnMessageAvailable(tr("Mehr als %1 Einträge").arg(trainDataModel->getLimit()));
    }

    resetFilter();
}

void QMTrainDataWidget::updateTableView()
{
    ui->tvTrainData->hideColumn(0);
    ui->tvTrainData->setItemDelegate(new QMProxySqlRelationalDelegate());
    ui->tvTrainData->setItemDelegateForColumn(3, new DateDelegate());
    ui->tvTrainData->selectionModel()->disconnect(this);

    ui->tvTrainData->selectionModel()->disconnect(this);
    connect(ui->tvTrainData->selectionModel(), &QItemSelectionModel::selectionChanged, this,
        &QMTrainDataWidget::trainDataSelectionChanged);

    ui->cbFilterEmployee->setModelColumn(1);

    ui->cbFilterTrain->setModelColumn(1);

    ui->cbFilterState->setModelColumn(1);

    ui->tvCertificates->hideColumn(0);
    ui->tvCertificates->hideColumn(1);
    ui->tvCertificates->hideColumn(2);
    ui->tvCertificates->hideColumn(4);
}

void QMTrainDataWidget::updateFilter()
{
    auto filter = QString("relTblAl_1.name LIKE '%%1%' AND relTblAl_2.name LIKE '%%2%' "
        "AND date > '%3' AND date < '%4' AND relTblAl_4.name LIKE '%%5%'")
        .arg(ui->cbFilterEmployee->currentText(), ui->cbFilterTrain->currentText(), ui->deDateFilterFrom->text(),
            ui->deDateFilterTo->text(), ui->cbFilterState->currentText());
    trainDataModel->setFilter(filter);
    if (trainDataModel->rowCount() >= trainDataModel->getLimit())
    {
        emit warnMessageAvailable(tr("Mehr als %1 Einträge").arg(trainDataModel->getLimit()));
    }

    // Close an open dock widget.
    ui->dwTrainDataCertificates->setVisible(false);
    ui->dwMultiEdit->setVisible(false);
}

void QMTrainDataWidget::resetFilter()
{
    ui->cbFilterEmployee->setCurrentText("");
    ui->cbFilterTrain->setCurrentText("");
    ui->cbFilterState->setCurrentText("");
}

void QMTrainDataWidget::setTrainFilter(QString training)
{
    ui->cbFilterTrain->setCurrentText(training);
}

void QMTrainDataWidget::setNameFilter(QString name)
{
    ui->cbFilterEmployee->setCurrentText(name);
}

void QMTrainDataWidget::clearDates()
{
    ui->deDateFilterFrom->clear();
    ui->deDateFilterTo->clear();
}

void QMTrainDataWidget::addSingleEntry()
{
    if (trainViewModel == nullptr || employeeViewModel == nullptr || trainDataStateViewModel == nullptr ||
        trainDataModel == nullptr)
    {
        qWarning() << "Models have nullptr";
        return;
    }

    if (trainViewModel->rowCount() < 1 || employeeViewModel->rowCount() < 1)
    {
        emit warnMessageAvailable(tr("Keine Schulung verfügbar"));
        return;
    }

    if (trainDataStateViewModel->rowCount() < 1)
    {
        emit warnMessageAvailable(tr("Kein Schulungsstatus verfügbar"));
        return;
    }

    // Get the current number and add the entry.
    int selRow = 0;
    QModelIndexList selIdxList = ui->tvTrainData->selectionModel()->selectedRows();
    if (!selIdxList.isEmpty())
    {
        selRow = selIdxList.first().row();
    }

    // Remove selections.
    ui->tvTrainData->selectionModel()->clear();

    // Get an empty record set to fill with default values.
    auto newRecord = trainDataModel->record();

    // If only one line exist. Add a new line with default values. If there are more than one line
    // available, use the values of the last entry as default.
    if (trainDataModel->rowCount() == 1 || selRow == 0)
    {
        newRecord.setValue(1, employeeViewModel->data(employeeViewModel->index(0, 0)));
        newRecord.setValue(2, trainViewModel->data(trainViewModel->index(0, 0)));
        newRecord.setValue(3, "2020-01-01");
        newRecord.setValue(4, trainDataStateViewModel->data(trainDataStateViewModel->index(0, 0)));
    }
    else
    {
        newRecord = trainDataModel->record(selRow);

        QModelIndexList indexes = employeeViewModel->match(
            employeeViewModel->index(0, employeeViewModel->fieldIndex("name")), Qt::DisplayRole,
            trainDataModel->data(trainDataModel->index(selRow, 1)), 1, Qt::MatchFixedString);

        if (indexes.size() == 1)
        {
            newRecord.setValue(1, employeeViewModel->data(employeeViewModel->index(indexes.first().row(), 0)));
        }
        else
        {
            // If no employee has been found, the last entry might be a deactivated employee. Just
            // give an information to the user about it.
            emit infoMessageAvailable(tr("Der Mitarbeiter existiert nicht oder ist deaktiviert"));
            newRecord.setValue(1, employeeViewModel->data(employeeViewModel->index(0, 0)));
        }

        indexes = trainViewModel->match(
                trainViewModel->index(0, trainViewModel->fieldIndex("name")), Qt::DisplayRole,
                trainDataModel->data(trainDataModel->index(selRow, 2)), 1,
                Qt::MatchFixedString);

        if (indexes.size() == 1)
        {
            newRecord.setValue(2, trainViewModel->data(trainViewModel->index(indexes.first().row(), 0)));
        }
        else
        {
            // If no training has been found, the last entry might be a deactivated training. Just
            // give an information to the user about it.
            emit infoMessageAvailable(tr("Die Schulung existiert nicht oder ist deaktiviert"));
            newRecord.setValue(2, trainViewModel->data(trainViewModel->index(0, 0)));
        }

        newRecord.setValue(3, trainDataModel->data(trainDataModel->index(selRow, 3)));

        indexes = trainDataStateViewModel->match(
                trainDataStateViewModel->index(0, trainViewModel->fieldIndex("name")), Qt::DisplayRole,
                trainDataModel->data(trainDataModel->index(selRow, 4)), 1,
                Qt::MatchFixedString);

        if (indexes.size() == 1)
        {
            newRecord.setValue(4, trainDataStateViewModel->data(
                trainDataStateViewModel->index(indexes.first().row(), 0)));
        }
        else
        {
            newRecord.setValue(4, trainDataStateViewModel->data(trainDataStateViewModel->index(0, 0)));
        }
    }

    // Remove the record value id, cause this will be filled out by the database as primary key.
    newRecord.remove(0);

    if (!trainDataModel->insertRecord(0, newRecord))
    {
        qDebug() << trainDataModel->database().lastError().driverText();
        emit warnMessageAvailable(tr("Der Eintrag konnte nicht erstellt werden."));
        return;
    }

    ui->tvTrainData->scrollToTop();
    ui->tvTrainData->selectRow(0);
}

void QMTrainDataWidget::deleteSelected()
{
    // Deleting a training data entry needs to delete all certificate correlations that might be
    // connected.

    auto modelIndexList = ui->tvTrainData->selectionModel()->selectedRows();
    if (modelIndexList.isEmpty())
    {
        emit infoMessageAvailable(tr("Kein Eintrag zum Löschen selektiert"));
        return;
    }

    if (modelIndexList.size() != 1)
    {
        emit infoMessageAvailable(tr("Zum Löschen darf nur genau ein Eintrag selektiert sein"));
        return;
    }

    auto modelIndex = modelIndexList.at(0);

    // Delete all entries in tvCertificate. Therefore select a row and run removeCertificate, till
    // data has no entries left. It is important to be sure that the certificate correlation data
    // has been filtered. Otherwise other entries might be deleted. Therefore make a check before
    // running the deletion process.

    auto trainDataId = trainDataModel->data(trainDataModel->index(modelIndex.row(), 0)).toInt();

    for (int i = 0; i < trainDataCertViewModel->rowCount(); i++)
    {
        // Get the train_data id.
        auto trainDataCertId = trainDataCertViewModel->data(
            trainDataCertViewModel->index(i, 1)).toInt();

        if (trainDataCertId != trainDataId)
        {
            emit warnMessageAvailable(tr("Unzureichende Plausibilitätsprüfung vor der Löschung"));
            return;
        }
    }

    // Check was ok, start with selecting rows in tvCertificates and delete them.
    while (trainDataCertViewModel->rowCount())
    {
        ui->tvCertificates->selectRow(0);
        removeCertificate();
    }

    // Make a check at the end.
    if (trainDataCertViewModel->rowCount() > 0)
    {
        emit warnMessageAvailable(tr("Es konnten nicht alle Schulungsnachweise gelöscht werden"));
        return;
    }

    // Delete all selected entries.
    if (!trainDataModel->removeRow(modelIndex.row()) || !trainDataModel->submitAll())
    {
        emit warnMessageAvailable(tr("Der Eintrag konnte nicht gelöscht werden. Möglicherweise existiert "
            "ein Schreibschutz auf der Datenbank."));
        trainDataModel->revertAll();
        return;
    }

    trainDataModel->select();
}

void QMTrainDataWidget::showMultiEdit()
{
    auto modelIndexList = ui->tvTrainData->selectionModel()->selectedRows();
    if (modelIndexList.size() <= 1)
    {
        return;
    }

    ui->laSelCount->setText(QString::number(modelIndexList.size()));

    // Show the details docked widget (might be invisible).
    ui->dwTrainDataCertificates->setVisible(false);
    ui->dwMultiEdit->setVisible(true);
}

void QMTrainDataWidget::addMultipleEntries()
{
    QMAddMultipleTrainDataDialog addMultipleDialog(trainDataModel.get(), this);
    addMultipleDialog.exec();

    trainDataModel->select();
}

void QMTrainDataWidget::showTrainDataCertificates()
{
    auto modelIndexList = ui->tvTrainData->selectionModel()->selectedRows();
    if (modelIndexList.size() != 1)
    {
        return;
    }

    auto modelIndex = modelIndexList.at(0);

    // Show information of selected entry.
    auto selRow = modelIndex.row();

    auto idFieldIndex = trainDataModel->fieldIndex("id");
    auto employeeFieldIndex = trainDataModel->fieldIndex("Employee_name_3");
    auto trainFieldIndex = trainDataModel->fieldIndex("Train_name_2");
    auto dateFieldIndex = trainDataModel->fieldIndex("date");
    auto stateFieldIndex = trainDataModel->fieldIndex("name");

    ui->laEmployeeName->setText(trainDataModel->data(trainDataModel->index(selRow, employeeFieldIndex)).toString());
    ui->laTraining->setText(trainDataModel->data(trainDataModel->index(selRow, trainFieldIndex)).toString());
    ui->laDate->setText(trainDataModel->data(trainDataModel->index(selRow, dateFieldIndex)).toString());
    ui->laState->setText(trainDataModel->data(trainDataModel->index(selRow, stateFieldIndex)).toString());

    // Get the train data id (primary key) and use it to filter the certificate table.
    auto trainDataId = trainDataModel->data(trainDataModel->index(selRow, idFieldIndex)).toInt();
    trainDataCertViewModel->setFilter(QString("train_data=%1").arg(trainDataId));

    // Show the details docked widget (might be invisible).
    ui->dwMultiEdit->setVisible(false);
    ui->dwTrainDataCertificates->setVisible(true);
}

void QMTrainDataWidget::trainDataSelectionChanged(const QItemSelection &selected,
    const QItemSelection &deselected)
{
    auto modelIndexList = ui->tvTrainData->selectionModel()->selectedRows();
    if (modelIndexList.size() == 1)
    {
        showTrainDataCertificates();
        return;
    }

    if (modelIndexList.size() > 1)
    {
        showMultiEdit();
        return;
    }

    ui->dwMultiEdit->setVisible(false);
    ui->dwTrainDataCertificates->setVisible(false);
}

void QMTrainDataWidget::updateMultiEditEnabledState()
{
    ui->cbTrainState->setEnabled(ui->cbSetState->isChecked());
    ui->cbName->setEnabled(ui->cbSetName->isChecked());
    ui->cwTrainDate->setEnabled(ui->cbSetDate->isChecked());
    ui->cbTrain->setEnabled(ui->cbSetTrain->isChecked());
    ui->leCertName->setEnabled(ui->cbAddMultiCertificate->isChecked());
    ui->pbChooseCertificate->setEnabled(ui->cbAddMultiCertificate->isChecked());
}

void QMTrainDataWidget::chooseMultiCertificate()
{
    // In order to add a certificate, the general certificate will be used. This dialog has a
    // special mode, which returns the id of a selected certificate. With this system, it is also
    // possible to upload new certificate.
    auto &settings = QMApplicationSettings::getInstance();

    auto varWidth = settings.read("CertificateDialog/Width");
    auto width = (varWidth.isNull()) ? 400 : varWidth.toInt();
    auto varHeight = settings.read("CertificateDialog/Height");
    auto height = (varHeight.isNull()) ? 400 : varHeight.toInt();

    QMCertificateDialog certDialog(Mode::CHOOSE, this);
    certDialog.updateData();
    certDialog.resize(width, height);
    certDialog.setModal(true);
    certDialog.exec();

    multiEditCertiId = certDialog.getSelectedId();

    if (multiEditCertiId < 0)
    {
        return;
    }

    ui->leCertName->setText(certDialog.getSelectedName());
}

int QMTrainDataWidget::chooseCertificate()
{
    // In order to add a certificate, the general certificate will be used. This dialog has a
    // special mode, which returns the id of a selected certificate. With this system, it is also
    // possible to upload new certificate.
    auto &settings = QMApplicationSettings::getInstance();

    auto varWidth = settings.read("CertificateDialog/Width");
    auto width = (varWidth.isNull()) ? 400 : varWidth.toInt();
    auto varHeight = settings.read("CertificateDialog/Height");
    auto height = (varHeight.isNull()) ? 400 : varHeight.toInt();

    QMCertificateDialog certDialog(Mode::CHOOSE, this);
    certDialog.updateData();
    certDialog.resize(width, height);
    certDialog.setModal(true);
    certDialog.exec();

    return certDialog.getSelectedId();
}

void QMTrainDataWidget::addCertificate()
{
    // Get the certificate id.
    auto selId = chooseCertificate();

    if (selId == -1)
    {
        emit infoMessageAvailable(tr("Keinen gültigen Nachweis ausgewählt"));
        return;
    }

    // Get the train data id.
    auto modelIndexList = ui->tvTrainData->selectionModel()->selectedRows();

    if (modelIndexList.size() != 1)
    {
        emit warnMessageAvailable(tr("Kein Schulungseintrag ausgewählt"));
        return;
    }

    auto modelIndex = modelIndexList.at(0);
    auto trainDataId = trainDataModel->data(trainDataModel->index(modelIndex.row(), 0)).toInt();
    auto certId = selId;

    // Test whether the certificate id already exist.
    for (int i = 0; i < trainDataCertViewModel->rowCount(); i++)
    {
        auto tmpCertId = trainDataCertViewModel->data(trainDataCertViewModel->index(i, 2)).toInt();
        if (tmpCertId == certId)
        {
            emit infoMessageAvailable(tr("Nachweis ist bereits angehängt"));
            return;
        }
    }

    auto row = trainDataCertModel->rowCount();

    if (trainDataCertModel->insertRow(row))
    {
        qWarning() << "cannot add a new row";
        qWarning() << trainDataCertModel->lastError().text();
    }

    trainDataCertModel->setData(trainDataCertModel->index(row, 1), trainDataId);
    trainDataCertModel->setData(trainDataCertModel->index(row, 2), certId);

    if (trainDataCertModel->submitAll())
    {
        qWarning() << "cannot submit changes to database";
        qWarning() << trainDataCertModel->lastError().text();
    }

    trainDataCertModel->select();
    trainDataCertViewModel->select();
}

void QMTrainDataWidget::removeCertificate()
{
    auto modelIndexList = ui->tvCertificates->selectionModel()->selectedRows();

    if (modelIndexList.size() != 1)
    {
        emit infoMessageAvailable(tr("Kein Schulungsnachweis ausgewählt"));
        return;
    }

    // Get primary key id from train data certificate correlation table.
    auto modelIndex = modelIndexList.at(0);
    auto trainDataCertId = trainDataCertViewModel->data(
        trainDataCertViewModel->index(modelIndex.row(), 0)).toInt();

    trainDataCertModel->setFilter(QString("id=%1").arg(trainDataCertId));

    if (trainDataCertModel->rowCount() != 1)
    {
        emit warnMessageAvailable(tr("Es existiert kein Korrelationseintrag mit der id"));
        return;
    }

    // From this point, only one row (0) should exist.
    if (!trainDataCertModel->removeRow(0))
    {
        emit warnMessageAvailable(tr("Korrelationseintrag kann nicht gelöscht werden"));
        return;
    }

    if (!trainDataCertModel->submitAll())
    {
        emit warnMessageAvailable(tr("Korrelationseintrag kann nicht gelöscht werden"));
        return;
    }

    trainDataCertViewModel->select();
}

void QMTrainDataWidget::showCert()
{
    auto modelIndexList = ui->tvCertificates->selectionModel()->selectedRows();

    if (modelIndexList.size() != 1)
    {
        emit infoMessageAvailable(tr("Kein Schulungsnachweis ausgewählt"));
        return;
    }

    auto modelIndex = modelIndexList.at(0);
    auto certName = trainDataCertViewModel->data(
        trainDataCertViewModel->index(modelIndex.row(), 3)).toString();

    // Open certification dialog.
    auto &settings = QMApplicationSettings::getInstance();

    auto varWidth = settings.read("CertificateDialog/Width");
    auto width = (varWidth.isNull()) ? 400 : varWidth.toInt();
    auto varHeight = settings.read("CertificateDialog/Height");
    auto height = (varHeight.isNull()) ? 400 : varHeight.toInt();

    QMCertificateDialog certDialog(Mode::MANAGE, this);
    certDialog.updateData();
    certDialog.setNameFilter(certName);
    certDialog.resize(width, height);
    certDialog.setModal(true);
    certDialog.exec();
}
