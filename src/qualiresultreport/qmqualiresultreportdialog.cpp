// qmqualiresultreportdialog.cpp is part of QualificationMatrix
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

#include "qmqualiresultreportdialog.h"
#include "ui_qmqualiresultreportdialog.h"
#include "model/qmemployeeviewmodel.h"
#include "model/qmshiftviewmodel.h"
#include "model/qmtrainingviewmodel.h"
#include "model/qmtraininggroupviewmodel.h"
#include "settings/qmapplicationsettings.h"
#include "framework/qmextendedselectiondialog.h"
#include "qmqualiresultreportdocument.h"
#include "qmqualiresultreportitem.h"
#include "model/qmqualificationmatrixviewmodel.h"
#include "model/qmemployeefunctionviewmodel.h"
#include "model/qmtrainingdataviewmodel.h"

#include <QProgressDialog>
#include <QPrintPreviewWidget>
#include <QSqlQuery>
#include <QSortFilterProxyModel>
#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QDebug>

QMQualiResultReportDialog::QMQualiResultReportDialog(QWidget *parent)
    : QMDialog(parent)
    , trainList(std::make_unique<QHash<int, QString>>())
{
    ui = new Ui::QMQualiResultReportDialog;
    ui->setupUi(this);

    updateInfo();

    auto &settings = QMApplicationSettings::getInstance();
}

QMQualiResultReportDialog::~QMQualiResultReportDialog()
{
    delete ui;
}

void QMQualiResultReportDialog::saveSettings()
{
    auto &settings = QMApplicationSettings::getInstance();
}

void QMQualiResultReportDialog::loadSettings()
{
    auto &settings = QMApplicationSettings::getInstance();
}

QList<QMQualiResultReportItem> QMQualiResultReportDialog::calculateResult()
{
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        return {};
    }

    // Initialize progress dialog to informate the user about current calculation state.

    QProgressDialog progressDialog(this);
    progressDialog.setWindowTitle(tr("Report erstellen"));
    progressDialog.setMinimum(0);
    progressDialog.setMaximum((int) trainList->keys().count());
    progressDialog.setLabelText(tr("Datenbanktabellen laden"));
    progressDialog.setModal(true);
    progressDialog.setCancelButton(nullptr);
    progressDialog.setVisible(true);

    // Get data from all necessary models.

    auto db = QSqlDatabase::database("default");

    QMTrainingViewModel trainViewModel(this, db);
    trainViewModel.select();

    QMTrainingGroupViewModel trainGroupViewModel(this, db);
    trainGroupViewModel.select();

    QMTrainingDataViewModel trainDataViewModel(this, db);
    trainDataViewModel.select();

    QMEmployeeViewModel employeeViewModel(this, db);
    employeeViewModel.select();

    QMEmployeeFunctionViewModel employeeFuncViewModel(this, db);
    employeeFuncViewModel.select();

    QMQualificationMatrixViewModel qualiMatrixViewModel(this, db);
    qualiMatrixViewModel.select();

    QList<QMQualiResultReportItem> resultItems;

    auto keys = trainList->keys();
    int num = 0;

    // Go through every choosen training to find the number of good training states.
    for (auto key : keys)
    {
        // Update the progress dialog.
        progressDialog.setLabelText(tr("Berechne Schulung '%1'").arg(trainList->value(key)).left(80));
        progressDialog.setValue(num);

        // Keep the ui alive.
        QApplication::processEvents();

        num++;
        int trainResultTotal = 0;
        int trainResultOk = 0;

        QMQualiResultReportItem item(this);
        item.setTrainName(trainList->value(key));

        // With the current training, we need to find the function that needs this training.
        for (int i = 0; i < qualiMatrixViewModel.rowCount(); i++)
        {
            auto funcNameMatrix = qualiMatrixViewModel.data(qualiMatrixViewModel.index(i, 1)).toString();
            auto trainName = qualiMatrixViewModel.data(qualiMatrixViewModel.index(i, 2)).toString();
            auto qualiState = qualiMatrixViewModel.data(qualiMatrixViewModel.index(i, 3)).toString();

            // Only look further if the training in the quali matrix fits to the search one. Use the function name
            // to to look for any employee having this function.
            if (trainName.compare(trainList->value(key)) == 0 && qualiState.compare(tr("Pflicht")) == 0)
            {
                // We found a function that needs the current training. Now find the employee who has this function
                // and therefore needs the current training.
                for (int j = 0; j < employeeFuncViewModel.rowCount(); j++)
                {
                    auto funcName = employeeFuncViewModel.data(employeeFuncViewModel.index(j, 2)).toString();
                    auto employeeFuncName = employeeFuncViewModel.data(employeeFuncViewModel.index(j, 1)).toString();

                    // We found an employee that needs the current training. Now we have to test general things to be
                    // sure this is a valid training. To test, if the user is an active one, it is enough to test
                    // whether the user is part of the employee view table. Cause this sql view is already filtered
                    // to active user.
                    for (int k = 0; k < employeeViewModel.rowCount(); k++)
                    {
                        auto employeeName = employeeViewModel.data(employeeViewModel.index(k, 1)).toString();
                        if (employeeName.compare(employeeFuncName) == 0)
                        {
                            // Only look further if the function is the one from the qualification matrix. Then get the
                            // correlated employee name.
                            if (funcNameMatrix.compare(funcName) == 0)
                            {
                                // At this point the employee named, needs the training from the report list. Test
                                // whether the training state is ok or not.
                                trainResultTotal++;

                                // Find the information whether there has been a succesfull training or not.
                                // Therefore, the following information is necessary:
                                // - What is the training periodic?
                                // - When was the last training?
                                trainDataViewModel.setFilter(
                                        "name='" + employeeFuncName + "' and \"name:1\"='" + trainName + "' and " +
                                        "\"name:2\"='" + tr("Durchgeführt") + "'");

                                // If there is no result, no training has been done and the training state for this
                                // employee is not ok.
                                if (trainDataViewModel.rowCount() < 1)
                                {
                                    continue;
                                }

                                int interval = -1;
                                for (int l = 0; l < trainViewModel.rowCount(); l++)
                                {
                                    auto trainInfoName = trainViewModel.data(trainViewModel.index(l, 1)).toString();

                                    if (trainInfoName.compare(trainName) == 0)
                                    {
                                        interval = trainViewModel.data(trainViewModel.index(l, 3)).toInt();
                                        break;
                                    }
                                }

                                // The training has not been found.
                                if (interval < 0)
                                {
                                    qDebug() << QString("Training '%1' not found").arg(trainName);
                                    continue;
                                }

                                // If training interval is 0, then there no need to repeat a training preiodically.
                                // Therefore a minimum of one training is enough.
                                if (interval == 0)
                                {
                                    trainResultOk++;
                                    continue;
                                }

                                // Test if there is a training that fits the needs.
                                bool ok = false;
                                for (int l = 0; l < trainDataViewModel.rowCount(); l++)
                                {
                                    auto trainDateString = trainDataViewModel.data(
                                            trainDataViewModel.index(l, 3)).toString();
                                    QDate trainDate = QDate::fromString(trainDateString, Qt::ISODate);

                                    // The date of the training data needs to be greater or equal the until date
                                    // minus the yearly training interval.
                                    if (trainDate.addYears(interval) >= ui->deUntil->date() &&
                                        trainDate <= ui->deUntil->date())
                                    {
                                        ok = true;
                                        break;
                                    }
                                }

                                if (ok)
                                {
                                    trainResultOk++;
                                }
                            }
                        }
                    }
                }
            }
        }

        item.setTrainResultOk(trainResultOk);
        item.setTrainResultTotal(trainResultTotal);
        resultItems.append(item);
    }

    return resultItems;
}

[[maybe_unused]] void QMQualiResultReportDialog::createReport()
{
    // Set up default printer.
    auto printer = new QPrinter();
    printer->setPageOrientation(QPageLayout::Orientation::Portrait);
    printer->setFullPage(true);

    // Call printer dialog.
    QPrintPreviewDialog previewDialog(printer, this);

    // Set starting properties for preview widget.
    QList<QPrintPreviewWidget *> previewWidgets = previewDialog.findChildren<QPrintPreviewWidget *>();
    for (auto previewWidget : previewWidgets)
    {
        previewWidget->fitToWidth();
        previewWidget->setZoomFactor(1.0);
    }

    connect(&previewDialog, &QPrintPreviewDialog::paintRequested, this, &QMQualiResultReportDialog::paintPdfRequest);
    previewDialog.exec();
}

[[maybe_unused]] void QMQualiResultReportDialog::paintPdfRequest(QPrinter *printer)
{
    QMQualiResultReportDocument document;

    QList<QMQualiResultReportItem> resultItems = calculateResult();
    document.createDocument(resultItems, ui->deUntil->date());

    // Default printer settings.
    document.print(printer);
}

[[maybe_unused]] void QMQualiResultReportDialog::addTrainings()
{
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        return;
    }

    auto db = QSqlDatabase::database("default");

    QMTrainingViewModel trainViewModel(this, db);
    trainViewModel.select();

    QMExtendedSelectionDialog extSelDialog(this, &trainViewModel, 1);
    if (extSelDialog.exec() == QDialog::Rejected)
    {
        return;
    }

    QModelIndexList selIndexList = extSelDialog.getFilterSelected();
    QSortFilterProxyModel *trainFilterModel = extSelDialog.getFilterModel();

    if (!extSelDialog.isReverse())
    {
        for (auto modelIndex : selIndexList)
        {
            // Get the name of the training, which should be the filter column that has been choosen before.
            auto trainName = trainFilterModel->data(
                    trainFilterModel->index(modelIndex.row(), trainFilterModel->filterKeyColumn())).toString();

            // Get the primary key id as an hash value. This value should always be unique.
            auto trainPrimaryId = trainFilterModel->data(trainFilterModel->index(modelIndex.row(), 0)).toInt();

            if (!trainList->contains(trainPrimaryId))
            {
                trainList->insert(trainPrimaryId, trainName);
                ui->lwTrainings->addItem(trainName);
            }
        }
    }
    else
    {
        // Create a temporary hash list for selection.
        QHash<int, QString> tmpHash;

        for (auto modelIndex : selIndexList)
        {
            // Get the name of the training, which should be the filter column that has been choosen before.
            auto trainName = trainFilterModel->data(
                    trainFilterModel->index(modelIndex.row(), trainFilterModel->filterKeyColumn())).toString();

            // Get the primary key id as an hash value. This value should always be unique.
            auto trainPrimaryId = trainFilterModel->data(trainFilterModel->index(modelIndex.row(), 0)).toInt();

            if (!tmpHash.contains(trainPrimaryId))
            {
                tmpHash.insert(trainPrimaryId, trainName);
            }
        }

        // Go through every training and add just the ones, that are not selected and not already inserted.
        for (int i = 0; i < trainViewModel.rowCount(); i++)
        {
            // Get the name of the training.
            auto trainName = trainViewModel.data(trainViewModel.index(i, 1)).toString();

            // Get the primary key id as an hash value. This value should always be unique.
            auto trainPrimaryId = trainViewModel.data(trainViewModel.index(i, 0)).toInt();

            if (!tmpHash.contains(trainPrimaryId) && !trainList->contains(trainPrimaryId))
            {
                trainList->insert(trainPrimaryId, trainName);
                ui->lwTrainings->addItem(trainName);
            }
        }
    }

    updateInfo();
}

[[maybe_unused]] void QMQualiResultReportDialog::addFromTrainingGroups()
{
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        return;
    }

    auto db = QSqlDatabase::database("default");

    QMTrainingViewModel trainViewModel(this, db);
    trainViewModel.select();

    QMTrainingGroupViewModel trainGroupViewModel(this, db);
    trainGroupViewModel.select();

    QMExtendedSelectionDialog extSelDialog(this, &trainGroupViewModel, 1);
    if (extSelDialog.exec() == QDialog::Rejected)
    {
        return;
    }

    QModelIndexList selIndexList = extSelDialog.getFilterSelected();
    QSortFilterProxyModel *trainGroupFilterModel = extSelDialog.getFilterModel();

    // Create a temporary hash list for selection.
    QHash<QString, int> tmpHash;

    for (auto modelIndex : selIndexList)
    {
        // Get the name of the training, which should be the filter column that has been choosen before.
        auto trainGroupName = trainGroupFilterModel->data(
                trainGroupFilterModel->index(modelIndex.row(), trainGroupFilterModel->filterKeyColumn())).toString();

        // Get the primary key id as an hash value. This value should always be unique.
        auto trainGroupPrimaryId = trainGroupFilterModel->data(
                trainGroupFilterModel->index(modelIndex.row(), 0)).toInt();

        if (!tmpHash.contains(trainGroupName))
        {
            tmpHash.insert(trainGroupName, trainGroupPrimaryId);
        }
    }

    for (int i = 0; i < trainViewModel.rowCount(); i++)
    {
        // Get the name of the training.
        auto trainName = trainViewModel.data(trainViewModel.index(i, 1)).toString();

        // Get the primary key id as an hash value. This value should always be unique.
        auto trainPrimaryId = trainViewModel.data(trainViewModel.index(i, 0)).toInt();

        // Get the group name.
        auto trainGroupName = trainViewModel.data(trainViewModel.index(i, 2)).toString();

        if (!extSelDialog.isReverse())
        {
            if (tmpHash.contains(trainGroupName) && !trainList->contains(trainPrimaryId))
            {
                trainList->insert(trainPrimaryId, trainName);
                ui->lwTrainings->addItem(trainName);
            }
        }
        else
        {
            if (!tmpHash.contains(trainGroupName) && !trainList->contains(trainPrimaryId))
            {
                trainList->insert(trainPrimaryId, trainName);
                ui->lwTrainings->addItem(trainName);
            }
        }
    }

    updateInfo();
}

[[maybe_unused]] void QMQualiResultReportDialog::removeTrainings()
{
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        return;
    }

    auto db = QSqlDatabase::database("default");

    QMTrainingViewModel trainViewModel(this, db);
    trainViewModel.select();

    QMExtendedSelectionDialog extSelDialog(this, &trainViewModel, 1);
    if (extSelDialog.exec() == QDialog::Rejected)
    {
        return;
    }

    QModelIndexList selIndexList = extSelDialog.getFilterSelected();
    QSortFilterProxyModel *trainFilterModel = extSelDialog.getFilterModel();

    if (!extSelDialog.isReverse())
    {
        for (auto modelIndex : selIndexList)
        {
            // Get the primary key id as an hash value. This value should always be unique.
            auto trainPrimaryId = trainFilterModel->data(trainFilterModel->index(modelIndex.row(), 0)).toInt();

            if (trainList->contains(trainPrimaryId))
            {
                trainList->remove(trainPrimaryId);
            }
        }
    }
    else
    {
        // Create a temporary hash list for selection.
        QHash<int, QString> tmpHash;

        for (auto modelIndex : selIndexList)
        {
            // Get the name of the training, which should be the filter column that has been choosen before.
            auto trainName = trainFilterModel->data(
                    trainFilterModel->index(modelIndex.row(), trainFilterModel->filterKeyColumn())).toString();

            // Get the primary key id as an hash value. This value should always be unique.
            auto trainPrimaryId = trainFilterModel->data(trainFilterModel->index(modelIndex.row(), 0)).toInt();

            if (!tmpHash.contains(trainPrimaryId))
            {
                tmpHash.insert(trainPrimaryId, trainName);
            }
        }

        // Go through every training and remove just the ones, that are not selected.
        for (int i = 0; i < trainViewModel.rowCount(); i++)
        {
            // Get the primary key id as an hash value. This value should always be unique.
            auto trainPrimaryId = trainViewModel.data(trainViewModel.index(i, 0)).toInt();

            if (!tmpHash.contains(trainPrimaryId) && trainList->contains(trainPrimaryId))
            {
                trainList->remove(trainPrimaryId);
            }
        }
    }

    ui->lwTrainings->clear();
    ui->lwTrainings->addItems(trainList->values());
    updateInfo();
}

[[maybe_unused]] void QMQualiResultReportDialog::removeFromTrainingGroups()
{
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        return;
    }

    auto db = QSqlDatabase::database("default");

    QMTrainingViewModel trainViewModel(this, db);
    trainViewModel.select();

    QMTrainingGroupViewModel trainGroupViewModel(this, db);
    trainGroupViewModel.select();

    QMExtendedSelectionDialog extSelDialog(this, &trainGroupViewModel, 1);
    if (extSelDialog.exec() == QDialog::Rejected)
    {
        return;
    }

    QModelIndexList selIndexList = extSelDialog.getFilterSelected();
    QSortFilterProxyModel *trainGroupFilterModel = extSelDialog.getFilterModel();

    // Create a temporary hash list for selection.
    QHash<QString, int> tmpHash;

    for (auto modelIndex : selIndexList)
    {
        // Get the name of the training, which should be the filter column that has been choosen before.
        auto trainGroupName = trainGroupFilterModel->data(
                trainGroupFilterModel->index(modelIndex.row(), trainGroupFilterModel->filterKeyColumn())).toString();

        // Get the primary key id as an hash value. This value should always be unique.
        auto trainGroupPrimaryId = trainGroupFilterModel->data(
                trainGroupFilterModel->index(modelIndex.row(), 0)).toInt();

        if (!tmpHash.contains(trainGroupName))
        {
            tmpHash.insert(trainGroupName, trainGroupPrimaryId);
        }
    }

    for (int i = 0; i < trainViewModel.rowCount(); i++)
    {
        // Get the primary key id as an hash value. This value should always be unique.
        auto trainPrimaryId = trainViewModel.data(trainViewModel.index(i, 0)).toInt();

        // Get the group name.
        auto trainGroupName = trainViewModel.data(trainViewModel.index(i, 2)).toString();

        if (!extSelDialog.isReverse())
        {
            if (tmpHash.contains(trainGroupName) && trainList->contains(trainPrimaryId)) {
                trainList->remove(trainPrimaryId);
            }
        }
        else
        {
            if (!tmpHash.contains(trainGroupName) && trainList->contains(trainPrimaryId))
            {
                trainList->remove(trainPrimaryId);
            }
        }
    }

    ui->lwTrainings->clear();
    ui->lwTrainings->addItems(trainList->values());
    updateInfo();
}

[[maybe_unused]] void QMQualiResultReportDialog::resetTrainingList()
{
    trainList->clear();
    ui->lwTrainings->clear();
    updateInfo();
}

void QMQualiResultReportDialog::updateInfo()
{
    ui->laElementCount->setText(tr("%1 Schulung(en)").arg(trainList->count()));
}
