//
// qmtraindatadetailsdialog.cpp is part of QualificationMatrix
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

#include "qmtraindatadetailsdialog.h"
#include "ui_qmtraindatadetailsdialog.h"
#include "settings/qmapplicationsettings.h"
#include "model/qmdatamanager.h"

#include <memory>
#include <QSqlRelationalTableModel>
#include <QSortFilterProxyModel>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QKeyEvent>

QMTrainDataDetailsDialog::QMTrainDataDetailsDialog(
    QSortFilterProxyModel *trainDataModel, int selRow, QWidget *parent)
    : QDialog(parent)
{
    ui = new Ui::QMTrainDataDetailsDialog();
    ui->setupUi(this);

    selRowEdit = selRow;
    trainDataModelEdit = trainDataModel;

    // Build connections.
    QPushButton *applyButton = ui->buttonBox->button(QDialogButtonBox::Apply);
    connect(applyButton, &QPushButton::clicked, this, &QMTrainDataDetailsDialog::apply);

    // Load model.
    auto dm = QMDataManager::getInstance();

    trainDataCertificateModel = dm->getTrainDataCertificateModel();
    employeeModel = dm->getEmployeeModel();
    trainModel = dm->getTrainModel();
    trainDataStateModel = dm->getTrainDataStateModel();

    updateUi();
}

QMTrainDataDetailsDialog::~QMTrainDataDetailsDialog()
{
    delete ui;
}

void QMTrainDataDetailsDialog::updateUi()
{
    ui->cbEmployee->setModel(employeeModel.get());
    ui->cbEmployee->setModelColumn(1);
    ui->cbState->setModel(trainDataStateModel.get());
    ui->cbState->setModelColumn(1);
    ui->cbTraining->setModel(trainModel.get());
    ui->cbTraining->setModelColumn(1);

    // Set model content to values in ui elements.
    auto employeeName = trainDataModelEdit->data(
        trainDataModelEdit->index(selRowEdit, 1)).toString();
    auto trainName = trainDataModelEdit->data(
        trainDataModelEdit->index(selRowEdit, 2)).toString();
    auto date = trainDataModelEdit->data(
        trainDataModelEdit->index(selRowEdit, 3)).toString();
    auto state = trainDataModelEdit->data(
        trainDataModelEdit->index(selRowEdit, 4)).toString();

    ui->cbEmployee->setCurrentText(employeeName);
    ui->cbTraining->setCurrentText(trainName);
    ui->cwDate->setSelectedDate(QDate::fromString(date, Qt::DateFormat::ISODate));
    ui->cbState->setCurrentText(state);
}

void QMTrainDataDetailsDialog::closeEvent(QCloseEvent *event)
{
    saveSettings();

    QDialog::closeEvent(event);
}

void QMTrainDataDetailsDialog::saveSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    // Window settings.
    settings.write("TrainDataDetailsDialog/Width", width());
    settings.write("TrainDataDetailsDialog/Height", height());
}

void QMTrainDataDetailsDialog::apply()
{
    // Test whether data train model is dirty or not.
    auto dm = QMDataManager::getInstance();
    if (dm->getTrainDataModel()->isDirty())
    {
        auto res = QMessageBox::question(
            this, tr("Schulungsdaten - Speichern"),
            tr("Die Änderungen werden in die Datenbank geschrieben.\n\nSind Sie sicher?"),
            QMessageBox::Yes | QMessageBox::No);

        if (res == QMessageBox::Yes)
        {
            // Save all changes to the model.
            dm->getTrainDataModel()->submitAll();
        }
        else
        {
            // Don't save changes and got back to the dialog.
            return;
        }
    }
}

void QMTrainDataDetailsDialog::accept()
{
    saveSettings();
    apply();

    QDialog::accept();
}

void QMTrainDataDetailsDialog::reject()
{
    saveSettings();

    // Reject all data if changes have been made.
    auto dm = QMDataManager::getInstance();
    if (dm->getTrainDataModel()->isDirty())
    {
        auto res = QMessageBox::question(
            this, tr("Schulungsdaten - Zurücksetzen"),
            tr("Alle Änderungen werden unwiderruflich verworfen!\n\nSind Sie sicher?"),
            QMessageBox::Yes | QMessageBox::No);

        if (res == QMessageBox::Yes)
        {
            // Revert all changes to the model.
            dm->getTrainDataModel()->revertAll();
        }
        else
        {
            // Don't revert changes and cancel the close action.
            return;
        }
    }

    QDialog::reject();
}

void QMTrainDataDetailsDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        return;
    }

    QDialog::keyPressEvent(event);
}
