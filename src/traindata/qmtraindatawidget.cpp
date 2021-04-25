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
#include "model/qmdatamanager.h"
#include "framework/qmproxysqlrelationaldelegate.h"
#include "framework/qmdatedelegate.h"
#include "certificate/qmcertificatedialog.h"
#include "qmimportcsvdialog.h"
#include "settings/qmapplicationsettings.h"
#include "framework/qmsqlrelationaltablemodel.h"

#include <QSqlRecord>
#include <QSqlField>
#include <QModelIndexList>
#include <QSortFilterProxyModel>
#include <QItemSelection>
#include <QSqlQuery>
#include <QSqlError>

#include <QDebug>

QMTrainDataWidget::QMTrainDataWidget(QWidget *parent)
    : QWidget(parent),
    ui(new Ui::QMTrainDataWidget)
{
    ui->setupUi(this);

    // Set ui component settings.
    ui->tvTrainData->horizontalHeader()->setStretchLastSection(false);
    ui->tvTrainData->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tvTrainData->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

    // Connect to data manager, to know when model reinitialization has been done.
    connect(QMDataManager::getInstance(), &QMDataManager::modelsInitialized, this, &QMTrainDataWidget::updateData);

    // Default settings on start.
    ui->deDateFilterTo->setDate(QDate::currentDate());
    ui->dwTrainDataCertificates->setVisible(false);

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

    ui->splitter->restoreState(settings.read("TrainData/CertificatesSplitter").toByteArray());
}

void QMTrainDataWidget::saveSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    settings.write("TrainData/CertificatesSplitter", ui->splitter->saveState());
}

void QMTrainDataWidget::importCsv()
{
    QMImportCsvDialog importDialog(this);
    importDialog.exec();
}

void QMTrainDataWidget::updateData()
{
    // Get the model data.
    auto dm = QMDataManager::getInstance();
    employeeModel = dm->getEmployeeViewModel();
    trainModel = dm->getTrainModel();
    trainDataModel = dm->getTrainDataModel();
    trainDataStateModel = dm->getTrainDataStateModel();
    trainDataCertModel = dm->getTrainDataCertificateModel();
    trainDataCertViewModel = dm->getTrainDataCertificateViewModel();

    // Set models to the corresponding view.
    ui->tvTrainData->setModel(trainDataModel.get());
    ui->cbFilterEmployee->setModel(employeeModel.get());
    ui->cbFilterTrain->setModel(trainModel.get());
    ui->cbFilterState->setModel(trainDataStateModel.get());
    ui->tvCertificates->setModel(trainDataCertViewModel.get());

    updateTableView();

    // If have to many entries available, informa the user.
    trainDataModel->select();
    if (trainDataModel->canFetchMore())
    {
        emit warnMessageAvailable(tr("Mehr als 255 Ergebnisse an Schulungsdaten verfügbar"));
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
        .arg(ui->cbFilterEmployee->currentText())
        .arg(ui->cbFilterTrain->currentText())
        .arg(ui->deDateFilterFrom->text())
        .arg(ui->deDateFilterTo->text())
        .arg(ui->cbFilterState->currentText());
    trainDataModel->setFilter(filter);

    // If have to many entries available, informa the user.
    if (trainDataModel->canFetchMore())
    {
        emit warnMessageAvailable(tr("Mehr als 255 Ergebnisse an Schulungsdaten verfügbar"));
    }

    // Close an open certificate widget.
    ui->dwTrainDataCertificates->setVisible(false);
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
    if (trainModel->rowCount() < 1 || employeeModel->rowCount() < 1)
    {
        emit warnMessageAvailable(tr("Keine Schulung verfügbar"));
        return;
    }

    if (trainDataStateModel->rowCount() < 1)
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
        newRecord.setValue(1, employeeModel->data(employeeModel->index(0, 0)));
        newRecord.setValue(2, trainModel->data(trainModel->index(0, 0)));
        newRecord.setValue(3, "2020-01-01");
        newRecord.setValue(4, trainDataStateModel->data(trainDataStateModel->index(0, 0)));
    }
    else
    {
        newRecord = trainDataModel->record(selRow);

        QModelIndexList indexes = employeeModel->match(
            employeeModel->index(0, employeeModel->fieldIndex("name")), Qt::DisplayRole,
            trainDataModel->data(trainDataModel->index(selRow, 1)), 1, Qt::MatchFixedString);

        if (indexes.size() == 1)
        {
            newRecord.setValue(1, employeeModel->data(employeeModel->index(indexes.first().row(), 0)));
        }
        else
        {
            // If no employee has been found, the last entry might be a deactivated employee. Just
            // give an information to the user about it.
            emit infoMessageAvailable(tr("Der Mitarbeiter existiert nicht oder ist deaktiviert"));
            newRecord.setValue(1, employeeModel->data(employeeModel->index(0, 0)));
        }

        indexes = trainModel->match(
                trainModel->index(0, trainModel->fieldIndex("name")), Qt::DisplayRole,
                trainDataModel->data(trainDataModel->index(selRow, 2)), 1,
                Qt::MatchFixedString);

        if (indexes.size() == 1)
        {
            newRecord.setValue(2, trainModel->data(trainModel->index(indexes.first().row(), 0)));
        }
        else
        {
            // If no training has been found, the last entry might be a deactivated training. Just
            // give an information to the user about it.
            emit infoMessageAvailable(tr("Die Schulung existiert nicht oder ist deaktiviert"));
            newRecord.setValue(2, trainModel->data(trainModel->index(0, 0)));
        }

        newRecord.setValue(3, trainDataModel->data(trainDataModel->index(selRow, 3)));

        indexes = trainDataStateModel->match(
                trainDataStateModel->index(0, trainModel->fieldIndex("name")), Qt::DisplayRole,
                trainDataModel->data(trainDataModel->index(selRow, 4)), 1,
                Qt::MatchFixedString);

        if (indexes.size() == 1)
        {
            newRecord.setValue(4, trainDataStateModel->data(trainDataStateModel->index(indexes.first().row(), 0)));
        }
    }

    // Remove the record value id, cause this will be filled out by the database as primary key.
    newRecord.remove(0);

    if (!trainDataModel->insertRecord(0, newRecord))
    {
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
    // model has no entries left. It is important to be sure that the certificate correlation model
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

void QMTrainDataWidget::showTrainDataCertificates()
{
    auto modelIndexList = ui->tvTrainData->selectionModel()->selectedRows();

    if (modelIndexList.size() != 1)
    {
        emit infoMessageAvailable(tr("Details werden nur bei genau einer Selektion angezeigt"));
        ui->dwTrainDataCertificates->setVisible(false);
        return;
    }

    auto modelIndex = modelIndexList.at(0);

    // Get the train data id (primary key) and use it to filter the certificate table.
    auto trainDataId = trainDataModel->data(trainDataModel->index(modelIndex.row(), 0)).toInt();
    trainDataCertViewModel->setFilter(QString("train_data=%1").arg(trainDataId));

    // Show the details docked widget (might be invisible).
    ui->dwTrainDataCertificates->setVisible(true);
}

void QMTrainDataWidget::trainDataSelectionChanged(const QItemSelection &selected,
    const QItemSelection &deselected)
{
    if (selected.indexes().size() > 0)
    {
        showTrainDataCertificates();
    }
}

void QMTrainDataWidget::addCertificate()
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

    // Get the certificate id.
    auto selId = certDialog.getSelectedId();

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
