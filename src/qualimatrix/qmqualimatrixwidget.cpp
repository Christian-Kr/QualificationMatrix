// qmqualimatrixwidget.cpp is part of QualificationMatrix
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

#include "qmqualimatrixwidget.h"
#include "ui_qmqualimatrixwidget.h"
#include "qmqualimatrixmodel.h"
#include "model/qmdatamanager.h"
#include "qmqualimatrixdelegate.h"
#include "qmqualimatrixheaderview.h"
#include "settings/qmapplicationsettings.h"
#include "framework/qmextendedselectiondialog.h"

#include <QSortFilterProxyModel>
#include <QSqlTableModel>
#include <QSqlRelationalTableModel>
#include <QColor>
#include <QDebug>
#include <QHeaderView>
#include <QWidget>
#include <QTimer>

QMQualiMatrixWidget::QMQualiMatrixWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::QMQualiMatrixWidget), qualiMatrixModel(nullptr), funcModel
    (nullptr),
    trainModel(nullptr), funcGroupModel(nullptr), trainGroupModel(nullptr),
    qualiMatrixFuncFilterModel(new QSortFilterProxyModel(this)),
    lockModeTimer(new QTimer(this))
{
    ui->setupUi(this);

    lockModeTimer->setSingleShot(true);
    lockModeTimer->setInterval(10000);
    connect(lockModeTimer, &QTimer::timeout, this, &QMQualiMatrixWidget::enableLocked);

    // initial settings for table view
    ui->tvQualiMatrix->setItemDelegate(new QMQualiMatrixDelegate());
    ui->tvQualiMatrix->setHorizontalHeader(
        new QMQualiMatrixHeaderView(Qt::Orientation::Horizontal, this));
    ui->tvQualiMatrix->setVerticalHeader(
        new QMQualiMatrixHeaderView(Qt::Orientation::Vertical, this));

    // Connect horizontal header.
    connect(ui->tvQualiMatrix->horizontalHeader(), &QHeaderView::sectionClicked, this,
        &QMQualiMatrixWidget::showTrainSectionHeaderInfo);

    // default edit mode - not editable
    ui->tbLock->setChecked(false);
    ui->tvQualiMatrix->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->dwTrainDetails->setVisible(false);

    ui->splitter->setCollapsible(0, false);
    ui->splitter->setCollapsible(1, false);
    ui->splitter->setCollapsible(2, false);

    loadSettings();
}

QMQualiMatrixWidget::~QMQualiMatrixWidget()
{
    // Save settings before deleting the object.
    saveSettings();

    delete ui;
}

void QMQualiMatrixWidget::showTrainSectionHeaderInfo(int logicalIndex)
{
    ui->dwTrainDetails->setVisible(true);
    auto trainName = qualiMatrixModel->headerData(logicalIndex, Qt::Horizontal).toString();

    for(int i = 0; i < trainModel->rowCount(); i++)
    {
        auto tmpValue = trainModel->data(trainModel->index(i, 1)).toString();
        if (tmpValue == trainName)
        {
            ui->laName->setText(tmpValue);
            ui->laGroup->setText(trainModel->data(trainModel->index(i, 2)).toString());

            ui->laInterval->setText(trainModel->data(
                trainModel->index(i, 3)).toString() + tr(" Jahr(e)"));

            if (trainModel->data(trainModel->index(i, 4)).toInt() == 1)
            {
                ui->laLegallyNecessary->setText(tr("Ja"));
            }
            else
            {
                ui->laLegallyNecessary->setText(tr("Nein"));
            }

            ui->pteContent->setPlainText(trainModel->data(trainModel->index(i, 5)).toString());

            break;
        }
    }
}

void QMQualiMatrixWidget::closeTrainDetails()
{
    ui->dwTrainDetails->setVisible(false);
}

void QMQualiMatrixWidget::loadSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    ui->dwFilter->setVisible(settings.read("QualiMatrix/ShowFilter", true).toBool());
    filterVisibilityChanged();

    ui->splitter->restoreState(settings.read("QualiMatrix/FilterSplitter").toByteArray());
}

void QMQualiMatrixWidget::saveSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    settings.write("QualiMatrix/FilterSplitter", ui->splitter->saveState());
}

void QMQualiMatrixWidget::updateFilter()
{
    // Set filter of model.
    qualiMatrixModel->setFuncFilter(ui->cbFuncFilter->currentText());
    qualiMatrixModel->setTrainFilter(ui->cbTrainFilter->currentText());
    qualiMatrixModel->setFuncGroupFilter(ui->cbFuncGroupFilter->currentText());
    qualiMatrixModel->setTrainGroupFilter(ui->cbTrainGroupFilter->currentText());

    // Legal filter depends on checkbox.
    qualiMatrixModel->setTrainLegalFilter(ui->cbLegal->isChecked());

    updateModel();
}

void QMQualiMatrixWidget::filterVisibilityChanged()
{
    if (ui->dwFilter->isVisible())
    {
        ui->tbFilterVisible->setText(tr("Filter ausblenden"));
    }
    else
    {
        ui->tbFilterVisible->setText(tr("Filter einblenden"));
    }
}

void QMQualiMatrixWidget::switchFilterVisibility()
{
    ui->dwFilter->setVisible(!ui->dwFilter->isVisible());

    auto &settings = QMApplicationSettings::getInstance();

    settings.write("QualiMatrix/ShowFilter", ui->dwFilter->isVisible());
}

void QMQualiMatrixWidget::updateData()
{
    // Get the model data.
    auto dm = QMDataManager::getInstance();

    funcModel = dm->getFuncModel();
    trainModel = dm->getTrainModel();
    funcGroupModel = dm->getFuncGroupModel();
    trainGroupModel = dm->getTrainGroupModel();
    qualiMatrixModel = dm->getQualiMatrixModel();
    qualiMatrixModel->updateModels();

    // Delete all connections.
    ui->cbFuncFilter->disconnect(this, SLOT(updateFilter()));
    ui->cbFuncGroupFilter->disconnect(this, SLOT(updateFilter()));
    ui->cbTrainFilter->disconnect(this, SLOT(updateFilter()));
    ui->cbTrainGroupFilter->disconnect(this, SLOT(updateFilter()));

    // Update the views.
    ui->tvQualiMatrix->setModel(qualiMatrixModel.get());

    // After setting the new model, the connection fo the selection model has to be reset.
    ui->tvQualiMatrix->selectionModel()->disconnect(this,
        SLOT(QMQualiMatrixWidget::selectionChanged()));
    connect(
        ui->tvQualiMatrix->selectionModel(), &QItemSelectionModel::currentChanged,
        this, &QMQualiMatrixWidget::selectionChanged);

    ui->cbFuncGroupFilter->setModel(funcGroupModel.get());
    ui->cbFuncGroupFilter->setModelColumn(1);

    ui->cbTrainGroupFilter->setModel(trainGroupModel.get());
    ui->cbTrainGroupFilter->setModelColumn(1);

    ui->cbTrainFilter->setModel(trainModel.get());
    ui->cbTrainFilter->setModelColumn(1);

    ui->cbFuncFilter->setModel(funcModel.get());
    ui->cbFuncFilter->setModelColumn(1);

    resetFilter();

    // Update the cache in the headers.
    updateHeaderCache();
}

void QMQualiMatrixWidget::resetFilter()
{
    ui->cbFuncGroupFilter->setCurrentText("");
    ui->cbTrainGroupFilter->setCurrentText("");
    ui->cbTrainFilter->setCurrentText("");
    ui->cbFuncFilter->setCurrentText("");
}

void QMQualiMatrixWidget::updateHeaderCache()
{
    auto verticalHeader = dynamic_cast<QMQualiMatrixHeaderView *>(
        ui->tvQualiMatrix->verticalHeader());
    verticalHeader->updateFunctionGroupColors();
    verticalHeader->updateTrainingGroupColors();
    auto horizontalHeader = dynamic_cast<QMQualiMatrixHeaderView *>(
        ui->tvQualiMatrix->horizontalHeader());
    horizontalHeader->updateTrainingGroupColors();
    horizontalHeader->updateFunctionGroupColors();
    horizontalHeader->updateTrainLegallyNecessary();
}

void QMQualiMatrixWidget::updateColors()
{
    auto matrixDelegate = dynamic_cast<QMQualiMatrixDelegate *>(
        ui->tvQualiMatrix->itemDelegate());
    if (matrixDelegate != nullptr)
    {
        matrixDelegate->updateColors();
    }
}

void QMQualiMatrixWidget::switchLockMode()
{
    ui->tbLock->setChecked(ui->tbLock->isChecked());

    if (!ui->tbLock->isChecked())
    {
        ui->tvQualiMatrix->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }
    else
    {
        ui->tvQualiMatrix->setEditTriggers(
            QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);

        // After lock mode has been opened, start a timer to trigger the end of editing at a
        // specific time.
        lockModeTimer->stop();
        lockModeTimer->start();
    }
}

void QMQualiMatrixWidget::enableLocked()
{
    ui->tbLock->setChecked(false);
    ui->tvQualiMatrix->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void QMQualiMatrixWidget::selectionChanged(
    const QModelIndex & current, const QModelIndex & previous)
{
    extendDisableLocked();
}

void QMQualiMatrixWidget::extendDisableLocked()
{
    if (lockModeTimer->isActive())
    {
        lockModeTimer->stop();
        lockModeTimer->start();
    }
}

void QMQualiMatrixWidget::extSelTrainGroup()
{
    QMExtendedSelectionDialog extSelDialog(this, trainGroupModel.get(), 1);
    auto res = extSelDialog.exec();
    auto modelIndexList = extSelDialog.getFilterSelected();

    if (modelIndexList.size() < 1 || res == QDialog::Rejected)
    {
        return;
    }

    ui->cbTrainGroupFilter->setCurrentText(extSelDialog.getRegExpText());
    updateFilter();
}

void QMQualiMatrixWidget::extSelFuncGroup()
{
    QMExtendedSelectionDialog extSelDialog(this, funcGroupModel.get(), 1);
    auto res = extSelDialog.exec();
    auto modelIndexList = extSelDialog.getFilterSelected();

    if (modelIndexList.size() < 1 || res == QDialog::Rejected)
    {
        return;
    }

    ui->cbFuncGroupFilter->setCurrentText(extSelDialog.getRegExpText());
    updateFilter();
}

void QMQualiMatrixWidget::extSelFunc()
{
    QMExtendedSelectionDialog extSelDialog(this, funcModel.get(), 1);
    auto res = extSelDialog.exec();
    auto modelIndexList = extSelDialog.getFilterSelected();

    if (modelIndexList.size() < 1 || res == QDialog::Rejected)
    {
        return;
    }

    ui->cbFuncFilter->setCurrentText(extSelDialog.getRegExpText());
    updateFilter();
}

void QMQualiMatrixWidget::extSelTrain()
{
    QMExtendedSelectionDialog extSelDialog(this, trainModel.get(), 1);
    auto res = extSelDialog.exec();
    auto modelIndexList = extSelDialog.getFilterSelected();

    if (modelIndexList.size() < 1 || res == QDialog::Rejected)
    {
        return;
    }

    ui->cbTrainFilter->setCurrentText(extSelDialog.getRegExpText());
    updateFilter();
}

void QMQualiMatrixWidget::updateModel()
{
    // Rebuild cache of model.
    qualiMatrixModel->buildCache();
    ui->tvQualiMatrix->clearSelection();
    ui->tvQualiMatrix->selectionModel()->clearCurrentIndex();
    ui->tvQualiMatrix->update();
}

void QMQualiMatrixWidget::resetTrain()
{
    if (ui->cbTrainFilter->currentText().isEmpty())
    {
        return;
    }

    ui->cbTrainFilter->clearEditText();
    updateFilter();
}

void QMQualiMatrixWidget::resetTrainGroup()
{
    if (ui->cbTrainGroupFilter->currentText().isEmpty())
    {
        return;
    }

    ui->cbTrainGroupFilter->clearEditText();
    updateFilter();
}

void QMQualiMatrixWidget::resetFunc()
{
    if (ui->cbFuncFilter->currentText().isEmpty())
    {
        return;
    }

    ui->cbFuncFilter->clearEditText();
    updateFilter();
}

void QMQualiMatrixWidget::resetFuncGroup()
{
    if (ui->cbFuncGroupFilter->currentText().isEmpty())
    {
        return;
    }

    ui->cbFuncGroupFilter->clearEditText();
    updateFilter();
}
