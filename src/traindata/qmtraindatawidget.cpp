//
// qmtraindatawidget.cpp is part of QualificationMatrix
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

#include "qmtraindatawidget.h"
#include "ui_qmtraindatawidget.h"
#include "model/qmdatamanager.h"
#include "framework/qmproxysqlrelationaldelegate.h"
#include "framework/qmdatedelegate.h"
#include "qmimportcsvdialog.h"
#include "qmtraindatadetailsdialog.h"
#include "settings/qmapplicationsettings.h"

#include <QMessageBox>
#include <QDebug>
#include <QSqlRecord>
#include <QSqlField>
#include <QModelIndexList>
#include <QSortFilterProxyModel>
#include <QProgressDialog>

QMTrainDataWidget::QMTrainDataWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::QMTrainDataWidget), employeeModel(nullptr), trainModel(nullptr),
    trainDataModel(nullptr), trainDataStateModel(nullptr),
    employeeFilterModel(new QSortFilterProxyModel(this)),
    trainFilterModel(new QSortFilterProxyModel(this)),
    trainDataStateFilterModel(new QSortFilterProxyModel(this)), progressDialog(nullptr)
{
    ui->setupUi(this);

    // Set ui component settings.
    ui->tvTrainData->horizontalHeader()->setStretchLastSection(false);
    ui->tvTrainData->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tvTrainData->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

    // Connect to data manager, to know when model reinitialization has been done.
    connect(QMDataManager::getInstance(), &QMDataManager::modelsInitialized, this,
        &QMTrainDataWidget::updateData);
}

QMTrainDataWidget::~QMTrainDataWidget()
{
    delete ui;
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
    employeeModel = dm->getEmployeeModel();
    trainModel = dm->getTrainModel();
    trainDataModel = dm->getTrainDataModel();
    trainDataStateModel = dm->getTrainDataStateModel();

    // Update filter models.
    trainFilterModel->setSourceModel(trainDataModel.get());
    trainFilterModel->setFilterKeyColumn(2);

    employeeFilterModel->setSourceModel(trainFilterModel);
    employeeFilterModel->setFilterKeyColumn(1);

    trainDataStateFilterModel->setSourceModel(employeeFilterModel);
    trainDataStateFilterModel->setFilterKeyColumn(4);

    // Update the views.
    ui->tvTrainData->setModel(trainDataStateFilterModel);
    ui->tvTrainData->hideColumn(0);
    ui->tvTrainData->setItemDelegate(new QMProxySqlRelationalDelegate());
    ui->tvTrainData->setItemDelegateForColumn(3, new DateDelegate());

    ui->cbFilterEmployee->setModel(employeeModel.get());
    ui->cbFilterEmployee->setModelColumn(1);

    ui->cbFilterTrain->setModel(trainModel.get());
    ui->cbFilterTrain->setModelColumn(1);

    ui->cbFilterState->setModel(trainDataStateModel.get());
    ui->cbFilterState->setModelColumn(1);

    resetFilter();
}

void QMTrainDataWidget::updateFilter()
{
    trainFilterModel->setFilterFixedString(ui->cbFilterTrain->currentText());
    trainDataStateFilterModel->setFilterFixedString(ui->cbFilterState->currentText());
    employeeFilterModel->setFilterFixedString(ui->cbFilterEmployee->currentText());
}

void QMTrainDataWidget::resetFilter()
{
    ui->cbFilterEmployee->setCurrentText("");
    ui->cbFilterTrain->setCurrentText("");
    ui->cbFilterState->setCurrentText("");
}

void QMTrainDataWidget::addSingleEntry()
{
    if (trainModel->rowCount() < 1 || employeeModel->rowCount() < 1)
    {
        QMessageBox::critical(
            this, tr("Eintrag hinzufügen"),
            tr("Es gibt keine definierte Schulung und/oder keinen definierten Mitarbeiter."
               " Bitte definieren Sie beides zuerst in den Einstellungen."
               "\n\nDie Aktion wird abgebrochen."));
        return;
    }

    if (trainDataStateModel->rowCount() < 1)
    {
        QMessageBox::critical(
            this, tr("Eintrag hinzufügen"),
            tr("Es gibt keinen Status der zugeordnet werden könnte. Bitte definieren Sie"
               " in den Einstellungen zuerst einen Status für die Schulungseinträge."
               "\n\nDie Aktion wird abgebrochen."));
        return;
    }

    // Before Adding a single entry, the filter has to be reset. Otherwise it might be possible,
    // that the new entry is not visible.
    resetFilter();
    updateFilter();

    // Remove selections.
    ui->tvTrainData->selectionModel()->clear();

    // Get the current number and add the entry.
    int selRow = trainDataModel->rowCount();
    QModelIndexList selIdxList = ui->tvTrainData->selectionModel()->selectedRows();
    if (!selIdxList.isEmpty())
    {
        selRow = selIdxList.first().row();
    }
    trainDataModel->insertRow(selRow, QModelIndex());

    // If only one line exist. Add a new line with default values. If there are more than one line
    // available, user the values of the last entry as default.
    if (trainDataModel->rowCount() == 1 || selRow == 0)
    {
        trainDataModel->setData(
            trainDataModel->index(selRow, 1), employeeModel->data(employeeModel->index(0, 0)));
        trainDataModel->setData(
            trainDataModel->index(selRow, 2), trainModel->data(trainModel->index(0, 0)));
        trainDataModel->setData(trainDataModel->index(selRow, 3), "2020.01.01");
        trainDataModel->setData(
            trainDataModel->index(selRow, 4),
            trainDataStateModel->data(trainDataStateModel->index(0, 0)));
    }
    else
    {
        QModelIndexList indexes = employeeModel->match(
            employeeModel->index(0, employeeModel->fieldIndex("name")), Qt::DisplayRole,
            trainDataModel->data(trainDataModel->index(selRow - 1, 1)), Qt::MatchFixedString
        );
        if (indexes.size() == 1)
        {
            trainDataModel->setData(
                trainDataModel->index(selRow, 1),
                employeeModel->data(employeeModel->index(indexes.first().row(), 0)));
        }

        indexes = trainModel->match(
            trainModel->index(0, trainModel->fieldIndex("name")), Qt::DisplayRole,
            trainDataModel->data(trainDataModel->index(selRow - 1, 2)), Qt::MatchFixedString
        );
        if (indexes.size() == 1)
        {
            trainDataModel->setData(
                trainDataModel->index(selRow, 2),
                trainModel->data(trainModel->index(indexes.first().row(), 0)));
        }

        trainDataModel->setData(
            trainDataModel->index(selRow, 3), trainModel->data(trainModel->index(selRow - 1, 3)));

        indexes = trainDataStateModel->match(
            trainDataStateModel->index(0, trainModel->fieldIndex("name")), Qt::DisplayRole,
            trainDataModel->data(trainDataModel->index(selRow - 1, 4)), Qt::MatchFixedString
        );
        if (indexes.size() == 1)
        {
            trainDataModel->setData(
                trainDataModel->index(selRow, 4),
                trainDataStateModel->data(trainDataStateModel->index(indexes.first().row(), 0)));
        }
    }

    ui->tvTrainData->scrollToBottom();
    ui->tvTrainData->selectRow(selRow);
}

void QMTrainDataWidget::deleteSelected()
{
    // For the seletion of entries in train data, there is no special need of restriction. Cause
    // there is no table in the structure that is connected ti primary key of it.
    QModelIndexList idxList = ui->tvTrainData->selectionModel()->selectedRows();
    if (idxList.isEmpty())
    {
        QMessageBox::information(
            this, tr("Eintrag löschen"),
            tr("Es muss zumindest ein Eintrag selektiert werden, der gelöscht werden kann."
               "\n\nDie Aktion wird abgebrochen."));
        return;
    }

    if (idxList.size() != 1)
    {
        QMessageBox::information(
            this, tr("Eintrag löschen"),
            tr("Aktuell sind mehrere Einträge selektiert, bitte selektieren Sie genau einen"
               " Eintrag zum Löschen.\n\nDie Aktion wird abgebrochen."));
        return;
    }

    // Delete all selected entries.
    trainDataStateFilterModel->removeRow(idxList.first().row());
}

void QMTrainDataWidget::showTrainDataDetails()
{
    auto modelIndexList = ui->tvTrainData->selectionModel()->selectedRows();

    if (modelIndexList.size() != 1)
    {
        QMessageBox::information(
            this, tr("Schulungseintrag"),
            tr("Es muss gneau ein Eintrag selektiert sein um Details anzuzeigen."));
        return;
    }

    auto modelIndex = modelIndexList.at(0);

    auto &settings = QMApplicationSettings::getInstance();

    auto varWidth = settings.read("TrainDataDetailsDialog/Width");
    auto width = (varWidth.isNull()) ? 400 : varWidth.toInt();
    auto varHeight = settings.read("TrainDataDetailsDialog/Height");
    auto height = (varHeight.isNull()) ? 400 : varHeight.toInt();

    QMTrainDataDetailsDialog detailsDialog(trainDataStateFilterModel, modelIndex.row(), this);
    detailsDialog.resize(width, height);
    detailsDialog.setModal(true);
    detailsDialog.exec();
}
