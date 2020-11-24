//
// qmsigninglistdialog.cpp is part of QualificationMatrix
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

#include "qmsigninglistdialog.h"
#include "ui_qmsigninglistdialog.h"
#include "model/qmdatamanager.h"
#include "settings/qmapplicationsettings.h"

#include <QDate>
#include <QMessageBox>
#include <QKeyEvent>
#include <QSqlRelationalTableModel>
#include <QSqlTableModel>

#include <QDebug>

QMSigningListDialog::QMSigningListDialog(QWidget *parent)
    : QDialog(parent)
{
    ui = new Ui::QMSigningListDialog;
    ui->setupUi(this);

    updateData();
}

QMSigningListDialog::~QMSigningListDialog()
{
    delete ui;
}

void QMSigningListDialog::accept()
{
    QDialog::accept();
}

void QMSigningListDialog::closeEvent(QCloseEvent *event)
{
    saveSettings();

    QDialog::closeEvent(event);
}

void QMSigningListDialog::saveSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    // Window settings.
    settings.write("SigningListDialog/Width", width());
    settings.write("SigningListDialog/Height", height());
}

void QMSigningListDialog::updateData()
{
    // Get the model data.
    auto dm = QMDataManager::getInstance();

    employeeModel = dm->getEmployeeModel();
    shiftModel = dm->getShiftModel();
    trainModel = dm->getTrainModel();

    // Set ui elements.
    ui->cbTraining->setModel(trainModel.get());
    ui->cbTraining->setModelColumn(1);

    ui->cbEmployeeGroup->setModel(shiftModel.get());
    ui->cbEmployeeGroup->setModelColumn(1);

    ui->cbSingleEmployee->setModel(employeeModel.get());
    ui->cbSingleEmployee->setModelColumn(1);
}

void QMSigningListDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        return;
    }

    QDialog::keyPressEvent(event);
}

void QMSigningListDialog::removeEmployee()
{

}

void QMSigningListDialog::addEmployee()
{

}
