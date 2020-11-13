//
// functionwidget.cpp is part of QualificationMatrix
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

#include "qmfuncsettingswidget.h"
#include "ui_qmfuncsettingswidget.h"
#include "model/qmdatamanager.h"
#include "delegate/qmproxysqlrelationaldelegate.h"
#include "delegate/qmcolorchooserdelegate.h"

#include <QSqlTableModel>
#include <QMessageBox>

QMFuncSettingsWidget::QMFuncSettingsWidget(QWidget *parent)
    : QMSettingsWidget(parent), ui(new Ui::QMFuncSettingsWidget), funcModel(nullptr),
    funcGroupModel(nullptr), funcFilterModel(new QSortFilterProxyModel(this))
{
    ui->setupUi(this);

    // Set initial settings for ui elements.
    ui->tvFunc->horizontalHeader()->setStretchLastSection(false);
    ui->tvFunc->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tvFunc->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    ui->tvFunc->verticalHeader()->setVisible(true);
    ui->tvFunc->setItemDelegateForColumn(2, new QMProxySqlRelationalDelegate(this));

    ui->tvFuncGroups->horizontalHeader()->setStretchLastSection(false);
    ui->tvFuncGroups->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tvFuncGroups->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    ui->tvFuncGroups->verticalHeader()->setVisible(true);
    ui->tvFuncGroups->setItemDelegateForColumn(2, new QMColorChooserDelegate(this));
}

QMFuncSettingsWidget::~QMFuncSettingsWidget()
{
    delete ui;
}

void QMFuncSettingsWidget::saveSettings()
{
    funcModel->submitAll();
    funcGroupModel->submitAll();
}

void QMFuncSettingsWidget::loadSettings()
{
    updateData();
}

void QMFuncSettingsWidget::revertChanges()
{
    funcModel->revertAll();
    funcGroupModel->revertAll();
}

void QMFuncSettingsWidget::updateData()
{
    // Get the model data.
    auto dm = QMDataManager::getInstance();

    funcModel = dm->getFuncModel();
    funcGroupModel = dm->getFuncGroupModel();

    // Set filter model.
    funcFilterModel->setSourceModel(funcModel.get());
    funcFilterModel->setFilterKeyColumn(1);

    // Update the views.
    ui->tvFunc->setModel(funcFilterModel);
    ui->tvFunc->hideColumn(0);

    ui->tvFuncGroups->setModel(funcGroupModel.get());
    ui->tvFuncGroups->hideColumn(0);

    // Build connections of the new models.
    connect(
        funcModel.get(), &QAbstractItemModel::dataChanged, this, &QMSettingsWidget::settingsChanged
    );
    connect(
        funcGroupModel.get(), &QAbstractItemModel::dataChanged, this,
        &QMSettingsWidget::settingsChanged
    );
}

void QMFuncSettingsWidget::filterFunc()
{
    funcFilterModel->setFilterRegExp(ui->leFuncFilter->text());
}

void QMFuncSettingsWidget::addFunc()
{
    // You should create a function group first.
    if (funcGroupModel->rowCount() < 1)
    {
        QMessageBox::information(
            this, tr("Funktion"), tr(
                "Bevor eine Funktion hinzugefügt wird, muss eine Gruppe festgelegt werden."
                "\n\nDie Aktion wird abgebrochen."
            ));
        return;
    }

    // The filter must be reseted, when a new entry will be added.
    ui->leFuncFilter->setText("");
    filterFunc();

    // Add a new temp row to the model.
    funcModel->insertRow(funcModel->rowCount());

    // Set a default function group.
    funcModel->setData(
        funcModel->index(funcModel->rowCount() - 1, 2),
        funcGroupModel->data(funcGroupModel->index(0, 0)));

    // Set a default function name and start editor.
    funcModel->setData(
        funcModel->index(funcModel->rowCount() - 1, 1), tr("Funktionsname eingeben"));

    // Start editing the name.
    ui->tvFunc->scrollToBottom();
    ui->tvFunc->edit(funcFilterModel->index(funcFilterModel->rowCount() - 1, 1));

    emit settingsChanged();
}

void QMFuncSettingsWidget::removeFunc()
{
    // Get the selected model index.
    QModelIndex selectedIndex = ui->tvFunc->selectionModel()->currentIndex();
    if (!selectedIndex.isValid())
    {
        QMessageBox::information(
            this, tr("Funktion löschen"), tr("Es wurde keine gültige Funktion ausgewählt."));
        return;
    }

    // Get the selected function name.
    QString selectedFuncName = funcFilterModel->data(funcFilterModel->index(selectedIndex.row(), 1))
        .toString();

    if (funcReference(selectedFuncName))
    {
        QMessageBox::critical(
            this, tr("Funktion löschen"), tr(
                "Es existieren Verweise auf die Funktion in den anderen"
                " Tabellen. Bitte löschen Sie zuerst die entsprechenden Verweise"
                " oder ändern Sie deren Funktionszugehörigkeit."
                "\n\nDie Aktion wird abgebrochen."
            ));
        return;
    }

    // Delete the entry.
    if (!funcFilterModel->removeRow(selectedIndex.row()))
    {
        QMessageBox::critical(
            this, tr("Funktionsgruppe löschen"), tr(
                "Die Funktionsgruppe konnte aus einem unbekannten Grund nicht gelöscht"
                " werden. Bitte informieren Sie den Entwickler zur Fehlerbehebung."
            ));
    }

    emit settingsChanged();
}

bool QMFuncSettingsWidget::funcReference(const QString &func)
{
    // The function can exist in several tables. Every table has to be searched for a relation.
    // This process might take a while, especially for the training data table, which might have
    // thousands of entries.
    auto dm = QMDataManager::getInstance();

    for (int i = 0; i < dm->getQualiModel()->rowCount(); i++)
    {
        QString funcName = dm->getQualiModel()->data(dm->getQualiModel()->index(i, 1)).toString();
        if (func == funcName)
        {
            return true;
        }
    }

    for (int i = 0; i < dm->getEmployeeFuncModel()->rowCount(); i++)
    {
        QString funcName = dm->getEmployeeFuncModel()->data(dm->getEmployeeFuncModel()->index(i, 2))
            .toString();
        if (func == funcName)
        {
            return true;
        }
    }

    return false;
}

void QMFuncSettingsWidget::revertFunc()
{
    auto res = QMessageBox::question(
        this, tr("Änderungen zurücksetzen"),
        tr("Alle temporären Änderungen an den Funktionen gehen verloren."
           "\n\nSind Sie sich sicher?"), QMessageBox::Yes | QMessageBox::No);

    if (res == QMessageBox::No)
    {
        return;
    }

    // Reset temporary changes.
    funcModel->revertAll();
}

void QMFuncSettingsWidget::addFuncGroups()
{
    // Add a new temp row to the model.
    funcGroupModel->insertRow(funcGroupModel->rowCount());

    // Set a default color.
    funcGroupModel->setData(funcGroupModel->index(funcGroupModel->rowCount() - 1, 2), "#000000");

    // Set a default group name and start editor.
    funcGroupModel->setData(
        funcGroupModel->index(funcGroupModel->rowCount() - 1, 1), tr("Gruppenname eingeben"));

    // Start editing the name.
    ui->tvFuncGroups->scrollToBottom();
    ui->tvFuncGroups->edit(funcGroupModel->index(funcGroupModel->rowCount() - 1, 1));

    emit settingsChanged();
}

void QMFuncSettingsWidget::removeFuncGroups()
{
    // Get the selected model index.
    auto selectedIndex = ui->tvFuncGroups->selectionModel()->currentIndex();
    if (!selectedIndex.isValid())
    {
        QMessageBox::information(
            this, tr("Funktionsgruppe löschen"), tr("Es wurde keine gültige Gruppe ausgewählt."));
        return;
    }

    // Get the selected group name.
    auto selectedGroupName = funcGroupModel->data(
        funcGroupModel->index(selectedIndex.row(), 1)).toString();

    // Do not delete when entries in function model have a reference to the group.
    auto found = false;

    for (int i = 0; i < funcModel->rowCount(); i++)
    {
        auto funcGroupName = funcModel->data(funcModel->index(i, 2)).toString();

        if (selectedGroupName == funcGroupName)
        {
            found = true;
            break;
        }
    }

    if (found)
    {
        QMessageBox::critical(
            this, tr("Funktionsgruppe löschen"),
            tr("Es existieren Verweise auf die Funktionsgruppe in den"
               " Funktiondefinitionen. Bitte löschen Sie zuerst die entsprechende Funktion"
               " oder ändern Sie deren Gruppenzugehörigkeit."
               "\n\nDie Aktion wird abgebrochen."));
        return;
    }

    // Delete the entry.
    if (!funcGroupModel->removeRow(selectedIndex.row()))
    {
        QMessageBox::critical(
            this, tr("Funktionsgruppe löschen"),
            tr("Die Funktionsgruppe konnte aus einem unbekannten Grund nicht gelöscht"
               " werden. Bitte informieren Sie den Entwickler zur Fehlerbehebung."));
    }

    emit settingsChanged();
}

void QMFuncSettingsWidget::revertFuncGroups()
{
    auto res = QMessageBox::question(
        this, tr("Änderungen zurücksetzen"),
        tr("Alle temporären Änderungen an den Funktiongruppen gehen verloren."
           "\n\nSind Sie sich sicher?"), QMessageBox::Yes | QMessageBox::No);

    if (res == QMessageBox::No)
    {
        return;
    }

    // Reset temporary changes.
    funcGroupModel->revertAll();
}
