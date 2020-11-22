//
// qmfiledialog.cpp is part of QualificationMatrix
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

#include "qmcertificatedialog.h"
#include "ui_qmcertificatedialog.h"
#include "model/qmdatamanager.h"
#include "model/qmcertificatemodel.h"

#include <QSqlTableModel>
#include <QSortFilterProxyModel>

QMCertificateDialog::QMCertificateDialog(QWidget *parent)
    : QDialog(parent)
{
    ui = new Ui::QMCertificateDialog;
    ui->setupUi(this);

    typeFilterModel = new QSortFilterProxyModel(this);
    nameFilterModel = new QSortFilterProxyModel(this);

    // Connect to data manager, to know when model reinitialization has been done.
    connect(QMDataManager::getInstance(), &QMDataManager::modelsInitialized, this,
        &QMCertificateDialog::updateData);
}

QMCertificateDialog::~QMCertificateDialog()
{
    delete ui;
}

void QMCertificateDialog::updateData()
{
    // Get the model data.
    auto dm = QMDataManager::getInstance();
    certificateModel = dm->getCertificateModel();

    // Update filter models.
    typeFilterModel->setSourceModel(certificateModel.get());
    typeFilterModel->setFilterKeyColumn(2);

    nameFilterModel->setSourceModel(typeFilterModel);
    nameFilterModel->setFilterKeyColumn(1);

    // Update the views. Only show name and type.
    ui->tvFiles->setModel(nameFilterModel);
    ui->tvFiles->hideColumn(0);
    ui->tvFiles->hideColumn(3);
    ui->tvFiles->hideColumn(4);

    resetFilter();
}

void QMCertificateDialog::updateFilter()
{
    typeFilterModel->setFilterFixedString(ui->cbType->currentText());
    nameFilterModel->setFilterFixedString(ui->leName->text());
}

void QMCertificateDialog::resetFilter()
{
    ui->cbType->setCurrentText("");
    ui->leName->setText("");
}
