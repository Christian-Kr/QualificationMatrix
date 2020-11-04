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
#include "delegate/proxysqlrelationaldelegate.h"
#include "delegate/colorchooserdelegate.h"
#include "delegate/checkboxdelegate.h"

#include <QSqlTableModel>
#include <QMessageBox>

QMTrainSettingsWidget::QMTrainSettingsWidget(QWidget *parent)
    : QMSettingsWidget(parent), ui(new Ui::QMTrainSettingsWidget), trainModel(nullptr),
    trainGroupModel(nullptr), trainFilterModel(new QSortFilterProxyModel(this))
{
    ui->setupUi(this);

    // Set initial settings for ui elements.
    ui->tvTrain->horizontalHeader()->setStretchLastSection(false);
    ui->tvTrain->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tvTrain->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    ui->tvTrain->verticalHeader()->setVisible(true);
    ui->tvTrain->setItemDelegateForColumn(2, new ProxySqlRelationalDelegate(ui->tvTrain));
    ui->tvTrain->setItemDelegateForColumn(4, new CheckBoxDelegate());

    ui->tvTrainGroups->horizontalHeader()->setStretchLastSection(false);
    ui->tvTrainGroups->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tvTrainGroups->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    ui->tvTrainGroups->verticalHeader()->setVisible(true);
    ui->tvTrainGroups->setItemDelegateForColumn(2, new ColorChooserDelegate(this));
}

QMTrainSettingsWidget::~QMTrainSettingsWidget()
{
    delete ui;
}

void QMTrainSettingsWidget::saveSettings()
{
    trainModel->submitAll();
    trainGroupModel->submitAll();

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
}

void QMTrainSettingsWidget::updateData()
{
    // Get the model data.
    auto dm = QMDataManager::getInstance();

    trainModel = dm->getTrainModel();
    trainGroupModel = dm->getTrainGroupModel();

    // Set filter model.
    trainFilterModel->setSourceModel(trainModel.get());
    trainFilterModel->setFilterKeyColumn(1);

    // Update the views.
    ui->tvTrain->setModel(trainFilterModel);
    ui->tvTrain->hideColumn(0);

    ui->tvTrainGroups->setModel(trainGroupModel.get());
    ui->tvTrainGroups->hideColumn(0);

    // Build connections of the new models.
    connect(
        trainModel.get(), &QAbstractItemModel::dataChanged, this, &QMSettingsWidget::settingsChanged
    );
    connect(
        trainGroupModel.get(), &QAbstractItemModel::dataChanged, this,
        &QMSettingsWidget::settingsChanged
    );
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
