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
#include "certificate/qmcertificatedialog.h"

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

    trainDataCertFilterModel = new QSortFilterProxyModel(this);
    trainDataCertFilterModel->setSourceModel(trainDataCertificateModel.get());
    trainDataCertFilterModel->setFilterKeyColumn(1);

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

    // Update the certificate ui elements.
    ui->tvCertificates->setModel(trainDataCertFilterModel);
    ui->tvCertificates->hideColumn(0);
    ui->tvCertificates->hideColumn(1);
    trainDataCertFilterModel->setFilterFixedString(
        QString::number(trainDataModelEdit->data(trainDataModelEdit->index(
            selRowEdit, 0)).toInt()));
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
    // Set model content to values in ui elements.
    auto employeeId = employeeModel->data(
        employeeModel->index(ui->cbEmployee->currentIndex(), 0)).toInt();
    trainDataModelEdit->setData(trainDataModelEdit->index(selRowEdit, 1), employeeId);

    auto trainId = trainModel->data(
        trainModel->index(ui->cbTraining->currentIndex(), 0)).toInt();
    trainDataModelEdit->setData(trainDataModelEdit->index(selRowEdit, 2), trainId);

    trainDataModelEdit->setData(
        trainDataModelEdit->index(selRowEdit, 3),
        ui->cwDate->selectedDate().toString(Qt::DateFormat::ISODate));

    auto trainDataModelEditId = trainDataModelEdit->data(
        trainDataModelEdit->index(ui->cbState->currentIndex(), 0)).toInt();
    trainDataModelEdit->setData(trainDataModelEdit->index(selRowEdit, 4), trainDataModelEditId);
}

bool QMTrainDataDetailsDialog::checkData() const
{
    auto employeeName = ui->cbEmployee->currentText();

    if (ui->cbEmployee->findText(employeeName) == -1)
    {
        return false;
    }

    auto trainName = ui->cbTraining->currentText();

    if (ui->cbTraining->findText(trainName) == -1)
    {
        return false;
    }

    return true;
}

void QMTrainDataDetailsDialog::accept()
{
    saveSettings();

    if (!checkData())
    {
        QMessageBox::information(
            this, tr("Daten validieren"),
            tr("Der Mitarbeiter oder die Schulung existiert nicht. Bitte wählen Sie existierende "
               "Einträge."));
        return;
    }

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

void QMTrainDataDetailsDialog::addCertificate()
{
    // TODO: Add certificate with the manage dialog in different mode (getting back the id).
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

    auto selId = certDialog.getSelectedId();

    if (selId == -1)
    {
        return;
    }

    auto trainDataId = trainDataModelEdit->data(trainDataModelEdit->index(selRowEdit, 0)).toInt();
    auto certId = selId;

    for (int i = 0; i < trainDataCertFilterModel->rowCount(); i++)
    {
        auto tmpCertId = trainDataCertFilterModel->data(
            trainDataCertFilterModel->index(i, 2)).toInt();

        if (tmpCertId == certId)
        {
            QMessageBox::information(
                this, tr("Nachweis anhängen"), tr("Der Nachweis existiert bereits."));
            return;
        }
    }

    auto row = trainDataCertificateModel->rowCount();
    trainDataCertificateModel->insertRow(row);

    trainDataCertificateModel->setData(trainDataCertificateModel->index(row, 1), trainDataId);
    trainDataCertificateModel->setData(trainDataCertificateModel->index(row, 2), certId);

    trainDataCertificateModel->submitAll();
}

void QMTrainDataDetailsDialog::removeCertificate()
{
    // TODO: Remove certificate and delete entry in traindatacertificate table.
}
