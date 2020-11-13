//
// qmtraindatadialog.cpp is part of QualificationMatrix
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

#include "qmtraindatadialog.h"
#include "ui_qmtraindatadialog.h"
#include "settings/qmapplicationsettings.h"
#include "model/qmdatamanager.h"

#include <memory>
#include <QSqlRelationalTableModel>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <QPushButton>

QMTrainDataDialog::QMTrainDataDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::QMTrainDataDialog)
{
    ui->setupUi(this);

    // Update data. They should be available before. This might include, that this dialog should
    // not be opened, when no database has been loaded.
    ui->wiTrainData->updateData();

    // Build connections.
    QPushButton *applyButton = ui->buttonBox->button(QDialogButtonBox::Apply);
    connect(applyButton, &QPushButton::clicked, this, &QMTrainDataDialog::apply);
}

QMTrainDataDialog::~QMTrainDataDialog()
{
    delete ui;
}

void QMTrainDataDialog::closeEvent(QCloseEvent *event)
{
    saveSettings();

    QDialog::closeEvent(event);
}

void QMTrainDataDialog::saveSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    // Window settings.
    settings.write("QMTrainDataDialog/Width", width());
    settings.write("QMTrainDataDialog/Height", height());
}

void QMTrainDataDialog::apply()
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

void QMTrainDataDialog::accept()
{
    saveSettings();
    apply();

    QDialog::accept();
}

void QMTrainDataDialog::reject()
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
