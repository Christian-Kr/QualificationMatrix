//
// qmtrainsettingswidget.h is part of QualificationMatrix
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

#include "qmtrainsettingswidget.h"
#include "ui_qmtrainsettingswidget.h"
#include "model/qmdatamanager.h"
#include "framework/qmproxysqlrelationaldelegate.h"
#include "framework/qmcolorchooserdelegate.h"
#include "framework/qmbooleandelegate.h"
#include "framework/qmplaintexteditdelegate.h"
#include "framework/qmsqlrelationaltablemodel.h"

#include <QSqlTableModel>
#include <QMessageBox>

QMTrainSettingsWidget::QMTrainSettingsWidget(QWidget *parent)
    : QMSettingsWidget(parent),
    ui(new Ui::QMTrainSettingsWidget),
    trainModel(nullptr),
    trainGroupModel(nullptr),
    trainDataStateModel(nullptr),
    trainDataModel(nullptr),
    trainFilterModel(new QSortFilterProxyModel(this))
{
    ui->setupUi(this);

    // Set initial settings for ui elements.
    ui->tvTrain->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    ui->tvTrain->verticalHeader()->setVisible(true);
    ui->tvTrain->setItemDelegateForColumn(2, new QMProxySqlRelationalDelegate(ui->tvTrain));
    ui->tvTrain->setItemDelegateForColumn(4, new QMBooleanDelegate());
    ui->tvTrain->setItemDelegateForColumn(5, new QMPlainTextEditDelegate());

    ui->tvTrainGroups->horizontalHeader()->setStretchLastSection(false);
    ui->tvTrainGroups->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tvTrainGroups->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    ui->tvTrainGroups->verticalHeader()->setVisible(true);
    ui->tvTrainGroups->setItemDelegateForColumn(2, new QMColorChooserDelegate(this));

    ui->tvTrainState->horizontalHeader()->setStretchLastSection(false);
    ui->tvTrainState->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tvTrainState->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    ui->tvTrainState->verticalHeader()->setVisible(true);
    ui->tvTrainState->setItemDelegateForColumn(2, new QMColorChooserDelegate(this));
}

QMTrainSettingsWidget::~QMTrainSettingsWidget()
{
    delete ui;
}

void QMTrainSettingsWidget::saveSettings()
{
    trainModel->submitAll();
    trainGroupModel->submitAll();
    trainDataStateModel->submitAll();

    // Other tables have to be updated.
    QMDataManager::getInstance()->getQualiModel()->select();
    QMDataManager::getInstance()->getTrainDataModel()->select();
}

void QMTrainSettingsWidget::loadSettings()
{
    updateData();
}

void QMTrainSettingsWidget::revertChanges()
{
    trainModel->revertAll();
    trainGroupModel->revertAll();
    trainDataStateModel->revertAll();
}

void QMTrainSettingsWidget::updateData()
{
    // Get the model data.
    auto dm = QMDataManager::getInstance();

    trainModel = dm->getTrainModel();
    trainGroupModel = dm->getTrainGroupModel();
    trainDataStateModel = dm->getTrainDataStateModel();
    trainDataModel = dm->getTrainDataModel();

    // Set filter model.
    trainFilterModel->setSourceModel(trainModel.get());
    trainFilterModel->setFilterKeyColumn(1);

    // Update the views.
    ui->tvTrain->setModel(trainFilterModel);
    ui->tvTrain->hideColumn(0);
    ui->tvTrain->setColumnWidth(5, 400);

    ui->tvTrainGroups->setModel(trainGroupModel.get());
    ui->tvTrainGroups->hideColumn(0);

    ui->tvTrainState->setModel(trainDataStateModel.get());
    ui->tvTrainState->hideColumn(0);

    // Build connections of the new models.
    connect(trainModel.get(), &QAbstractItemModel::dataChanged, this,
        &QMSettingsWidget::settingsChanged);
    connect(trainGroupModel.get(), &QAbstractItemModel::dataChanged, this,
        &QMSettingsWidget::settingsChanged);
    connect(trainDataStateModel.get(), &QAbstractItemModel::dataChanged, this,
        &QMSettingsWidget::settingsChanged);
}

void QMTrainSettingsWidget::filterTrain()
{
    trainFilterModel->setFilterRegExp(ui->leTrainFilter->text());
}

void QMTrainSettingsWidget::addTrain()
{
    // You should create a function group first.
    if (trainGroupModel->rowCount() < 1) {
        QMessageBox::information(
            this, tr("Schulung"), tr(
                "Bevor eine Schulung hinzugefügt wird, muss eine Gruppe festgelegt werden."
                "\n\nDie Aktion wird abgebrochen."
            ));
        return;
    }

    // The filter must be reseted, when a new entry will be added.
    ui->leTrainFilter->setText("");
    filterTrain();

    // Add a new temp row to the model.
    trainModel->insertRow(trainModel->rowCount());

    // Set a default train group.
    trainModel->setData(
        trainModel->index(trainModel->rowCount() - 1, 2),
        trainGroupModel->data(trainGroupModel->index(0, 0)));

    // Set a default train name and start editor.
    trainModel->setData(
        trainModel->index(trainModel->rowCount() - 1, 1), tr("Schulungsname eingeben"));

    // Start editing the name.
    ui->tvTrain->scrollToBottom();
    ui->tvTrain->edit(trainFilterModel->index(trainFilterModel->rowCount() - 1, 1));

    emit settingsChanged();
}

void QMTrainSettingsWidget::removeTrain()
{
    // Get the selected model index.
    QModelIndex selectedIndex = ui->tvTrain->selectionModel()->currentIndex();
    if (!selectedIndex.isValid()) {
        QMessageBox::information(
            this, tr("Schulung löschen"), tr("Es wurde keine gültige Schulung ausgewählt."));
        return;
    }

    // Get the selected training name.
    QString selectedTrainName = trainFilterModel
        ->data(trainFilterModel->index(selectedIndex.row(), 1)).toString();

    if (trainReference(selectedTrainName)) {
        QMessageBox::critical(
            this, tr("Schulung löschen"), tr(
                "Es existieren Verweise auf die Schulung in den anderen"
                " Tabellen. Bitte löschen Sie zuerst die entsprechenden Verweise"
                " oder ändern Sie deren Schulungszugehörigkeit."
                "\n\nDie Aktion wird abgebrochen."
            ));
        return;
    }

    // Delete the entry.
    if (!trainFilterModel->removeRow(selectedIndex.row())) {
        QMessageBox::critical(
            this, tr("Schulung löschen"), tr(
                "Die Schulung konnte aus einem unbekannten Grund nicht gelöscht"
                " werden. Bitte informieren Sie den Entwickler zur Fehlerbehebung."
            ));
    }

    emit settingsChanged();
}

bool QMTrainSettingsWidget::trainReference(const QString &train)
{
    // The training can exist in several tables. Every table has to be searched for a relation.
    // This process might take a while, especially for the training data table, which might have
    // thousands of entries.
    auto dm = QMDataManager::getInstance();

    // Search inside quali model.
    for (int i = 0; i < dm->getQualiModel()->rowCount(); i++) {
        QString trainName = dm->getQualiModel()->data(dm->getQualiModel()->index(i, 2)).toString();
        if (train == trainName) {
            return true;
        }
    }

    // Search inside exception model.
    for (int i = 0; i < dm->getTrainExceptionModel()->rowCount(); i++) {
        QString trainName = dm->getTrainExceptionModel()
            ->data(dm->getTrainExceptionModel()->index(i, 2)).toString();
        if (train == trainName) {
            return true;
        }
    }

    return false;
}

void QMTrainSettingsWidget::revertTrain()
{
    int res = QMessageBox::question(
        this, tr("Änderungen zurücksetzen"), tr(
            "Alle temporären Änderungen an den Schulungen gehen verloren."
            "\n\nSind Sie sich sicher?"
        ), QMessageBox::Yes | QMessageBox::No
    );
    if (res == QMessageBox::No) {
        return;
    }

    // Reset temporary changes.
    trainModel->revertAll();
}

void QMTrainSettingsWidget::revertTrainStates()
{
    int res = QMessageBox::question(
            this, tr("Änderungen zurücksetzen"), tr(
                    "Alle temporären Änderungen an den Status gehen verloren."
                    "\n\nSind Sie sich sicher?"
            ), QMessageBox::Yes | QMessageBox::No
    );
    if (res == QMessageBox::No) {
        return;
    }

    // Reset temporary changes.
    trainDataStateModel->revertAll();
}

void QMTrainSettingsWidget::addTrainGroups()
{
    // Add a new temp row to the model.
    trainGroupModel->insertRow(trainGroupModel->rowCount());

    // Set a default color.
    trainGroupModel->setData(trainGroupModel->index(trainGroupModel->rowCount() - 1, 2), "#000000");

    // Set a default group name and start editor.
    trainGroupModel->setData(
        trainGroupModel->index(trainGroupModel->rowCount() - 1, 1), tr("Gruppenname eingeben"));

    // Start editing the name.
    ui->tvTrainGroups->scrollToBottom();
    ui->tvTrainGroups->edit(trainGroupModel->index(trainGroupModel->rowCount() - 1, 1));

    emit settingsChanged();
}

void QMTrainSettingsWidget::addTrainState()
{
    // Add a new temp row to the model.
    trainDataStateModel->insertRow(trainDataStateModel->rowCount());

    // Set a default color.
    trainDataStateModel->setData(
            trainDataStateModel->index(trainDataStateModel->rowCount() - 1, 2), "#000000");

    // Set a default status name and start editor.
    trainDataStateModel->setData(
            trainDataStateModel->index(trainDataStateModel->rowCount() - 1, 1),
            tr("Statusname eingeben"));

    // Start editing the name.
    ui->tvTrainState->scrollToBottom();
    ui->tvTrainState->edit(trainDataStateModel->index(trainDataStateModel->rowCount() - 1, 1));

    emit settingsChanged();
}

void QMTrainSettingsWidget::removeTrainGroups()
{
    // Get the selected model index.
    QModelIndex selectedIndex = ui->tvTrainGroups->selectionModel()->currentIndex();
    if (!selectedIndex.isValid()) {
        QMessageBox::information(
            this, tr("Schulungsgruppe löschen"), tr("Es wurde keine gültige Gruppe ausgewählt."));
        return;
    }

    // Get the selected group name.
    QString selectedGroupName = trainGroupModel
        ->data(trainGroupModel->index(selectedIndex.row(), 1)).toString();

    // Do not delete when entries in train model have a reference to the group.
    bool found = false;
    for (int i = 0; i < trainModel->rowCount(); i++) {
        QString trainGroupName = trainModel->data(trainModel->index(i, 2)).toString();
        if (selectedGroupName == trainGroupName) {
            found = true;
            break;
        }
    }

    if (found) {
        QMessageBox::critical(
            this, tr("Schulungsgruppe löschen"), tr(
                "Es existieren Verweise auf die Schulungsgruppe in den"
                " Schulungsdefinitionen. Bitte löschen Sie zuerst die entsprechende Schulung"
                " oder ändern Sie deren Gruppenzugehörigkeit."
                "\n\nDie Aktion wird abgebrochen."
            ));
        return;
    }

    // Delete the entry.
    if (!trainGroupModel->removeRow(selectedIndex.row())) {
        QMessageBox::critical(
            this, tr("Schulungsgruppe löschen"), tr(
                "Die Schulungsgruppe konnte aus einem unbekannten Grund nicht gelöscht"
                " werden. Bitte informieren Sie den Entwickler zur Fehlerbehebung."
            ));
    }

    emit settingsChanged();
}

void QMTrainSettingsWidget::removeTrainState()
{
    // Get the selected model index.
    QModelIndex selectedIndex = ui->tvTrainState->selectionModel()->currentIndex();
    if (!selectedIndex.isValid())
    {
        QMessageBox::information(
                this, tr("Schulungsstatus löschen"),
                tr("Es wurde kein gültiger Status ausgewählt."));
        return;
    }

    // Get the selected state name.
    QString selectedStateName = trainDataStateModel->data(
            trainDataStateModel->index(selectedIndex.row(), 1)).toString();

    // Do not delete when entries in train data model have a reference to the state.
    bool found = false;
    for (int i = 0; i < trainDataModel->rowCount(); i++)
    {
        QString trainStateName = trainDataModel->data(trainDataModel->index(i, 4)).toString();
        if (selectedStateName == trainStateName) {
            found = true;
            break;
        }
    }

    if (found)
    {
        QMessageBox::critical(
                this, tr("Schulungsstatus löschen"),
                tr("Es existieren Verweise auf den Status in den"
                    " Schulungsdaten. Bitte löschen Sie zuerst den entsprechende Schulungssatz"
                    " oder ändern Sie deren Statuszugehörigkeit."
                    "\n\nDie Aktion wird abgebrochen."));
        return;
    }

    // Delete the entry.
    if (!trainDataStateModel->removeRow(selectedIndex.row()))
    {
        QMessageBox::critical(
                this, tr("Schulungsstatus löschen"), tr(
                        "Der Status konnte aus einem unbekannten Grund nicht gelöscht"
                        " werden. Bitte informieren Sie den Entwickler zur Fehlerbehebung."
                ));
    }

    emit settingsChanged();
}

void QMTrainSettingsWidget::revertTrainGroups()
{
    int res = QMessageBox::question(
        this, tr("Änderungen zurücksetzen"), tr(
            "Alle temporären Änderungen an den Schulungsgruppen gehen verloren."
            "\n\nSind Sie sich sicher?"
        ), QMessageBox::Yes | QMessageBox::No
    );
    if (res == QMessageBox::No) {
        return;
    }

    // Reset temporary changes.
    trainGroupModel->revertAll();
}
