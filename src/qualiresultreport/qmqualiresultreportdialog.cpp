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

#include <QSortFilterProxyModel>
#include <QDebug>

QMQualiResultReportDialog::QMQualiResultReportDialog(QWidget *parent)
    : QMDialog(parent)
    , trainList(std::make_unique<QHash<int, QString>>())
{
    ui = new Ui::QMQualiResultReportDialog;
    ui->setupUi(this);

    updateData();

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

void QMQualiResultReportDialog::updateData()
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

    trainGroupViewModel = std::make_unique<QMTrainingGroupViewModel>(this, db);
    trainGroupViewModel->select();
}

void QMQualiResultReportDialog::createReport()
{
    // TODO: Create report.

    //
}

void QMQualiResultReportDialog::addTrainings()
{
    // Get the training from selection dialog.
    QMExtendedSelectionDialog extSelDialog(this, trainViewModel.get(), 1);
    extSelDialog.exec();

    QModelIndexList selIndexList = extSelDialog.getFilterSelected();
    QSortFilterProxyModel *trainFilterModel = extSelDialog.getFilterModel();

    if (!extSelDialog.isReverse())
    {
        for (int i = 0; i < selIndexList.size(); i++)
        {
            QModelIndex modelIndex = selIndexList.at(i);

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

        for (int i = 0; i < selIndexList.size(); i++)
        {
            QModelIndex modelIndex = selIndexList.at(i);

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
        for (int i = 0; i < trainViewModel->rowCount(); i++)
        {
            // Get the name of the training.
            auto trainName = trainViewModel->data(trainViewModel->index(i, 1)).toString();

            // Get the primary key id as an hash value. This value should always be unique.
            auto trainPrimaryId = trainViewModel->data(trainViewModel->index(i, 0)).toInt();

            if (!tmpHash.contains(trainPrimaryId) && !trainList->contains(trainPrimaryId))
            {
                trainList->insert(trainPrimaryId, trainName);
                ui->lwTrainings->addItem(trainName);
            }
        }
    }
}

void QMQualiResultReportDialog::addFromTrainingGroups()
{
    // Get the training group from selection dialog.
    QMExtendedSelectionDialog extSelDialog(this, trainGroupViewModel.get(), 1);
    extSelDialog.exec();

    QModelIndexList selIndexList = extSelDialog.getFilterSelected();
    QSortFilterProxyModel *trainGroupFilterModel = extSelDialog.getFilterModel();

    if (!extSelDialog.isReverse())
    {
        // Create a temporary hash list for selection.
        QHash<QString, int> tmpHash;

        for (int i = 0; i < selIndexList.size(); i++)
        {
            QModelIndex modelIndex = selIndexList.at(i);

            // Get the name of the training, which should be the filter column that has been choosen before.
            auto trainGroupName = trainGroupFilterModel->data(
                    trainGroupFilterModel->index(modelIndex.row(),
                            trainGroupFilterModel->filterKeyColumn())).toString();

            // Get the primary key id as an hash value. This value should always be unique.
            auto trainGroupPrimaryId = trainGroupFilterModel->data(
                    trainGroupFilterModel->index(modelIndex.row(), 0)).toInt();

            if (!tmpHash.contains(trainGroupName))
            {
                tmpHash.insert(trainGroupName, trainGroupPrimaryId);
            }
        }

        // Go through every training and add just the ones, that are not selected and not already inserted.
        for (int i = 0; i < trainViewModel->rowCount(); i++)
        {
            // Get the name of the training.
            auto trainName = trainViewModel->data(trainViewModel->index(i, 1)).toString();

            // Get the primary key id as an hash value. This value should always be unique.
            auto trainPrimaryId = trainViewModel->data(trainViewModel->index(i, 0)).toInt();

            // Get the group name.
            auto trainGroupName = trainViewModel->data(trainViewModel->index(i, 2)).toString();

            if (tmpHash.contains(trainGroupName) && !trainList->contains(trainPrimaryId))
            {
                trainList->insert(trainPrimaryId, trainName);
                ui->lwTrainings->addItem(trainName);
            }
        }
    }
//    else
//    {
//        // Create a temporary hash list for selection.
//        QHash<int, QString> tmpHash;

//        for (int i = 0; i < selIndexList.size(); i++)
//        {
//            QModelIndex modelIndex = selIndexList.at(i);

//            // Get the name of the training, which should be the filter column that has been choosen before.
//            auto trainName = trainFilterModel->data(
//                    trainFilterModel->index(modelIndex.row(), trainFilterModel->filterKeyColumn())).toString();

//            // Get the primary key id as an hash value. This value should always be unique.
//            auto trainPrimaryId = trainFilterModel->data(trainFilterModel->index(modelIndex.row(), 0)).toInt();

//            if (!tmpHash.contains(trainPrimaryId))
//            {
//                tmpHash.insert(trainPrimaryId, trainName);
//            }
//        }

//        // Go through every training and add just the ones, that are not selected and not already inserted.
//        for (int i = 0; i < trainViewModel->rowCount(); i++)
//        {
//            // Get the name of the training.
//            auto trainName = trainViewModel->data(trainViewModel->index(i, 1)).toString();

//            // Get the primary key id as an hash value. This value should always be unique.
//            auto trainPrimaryId = trainViewModel->data(trainViewModel->index(i, 0)).toInt();

//            if (!tmpHash.contains(trainPrimaryId) && !trainList->contains(trainPrimaryId))
//            {
//                trainList->insert(trainPrimaryId, trainName);
//                ui->lwTrainings->addItem(trainName);
//            }
//        }
//    }
}
