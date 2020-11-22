//
// qmcertificatedialog.cpp is part of QualificationMatrix
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
#include "settings/qmapplicationsettings.h"

#include <QSqlTableModel>
#include <QSortFilterProxyModel>
#include <QFileDialog>
#include <QCryptographicHash>
#include <QDate>
#include <QMessageBox>

#include <QDebug>

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

void QMCertificateDialog::closeEvent(QCloseEvent *event)
{
    saveSettings();

    QDialog::closeEvent(event);
}

void QMCertificateDialog::saveSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    // Window settings.
    settings.write("CertificateDialog/Width", width());
    settings.write("CertificateDialog/Height", height());
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
    ui->tvFiles->hideColumn(5);

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

void QMCertificateDialog::addCertificate()
{
    auto fileName = QFileDialog::getOpenFileName(
        this, tr("Nachweis hinzufügen"), tr("All files (*.*);;JPEG (*.jpg *.jpeg);;PDF (*.pdf)" ));

    if (fileName.isEmpty())
    {
        return;
    }

    QFile file(fileName);
    file.open(QIODevice::ReadOnly);

    if (!file.isReadable() || !file.exists())
    {
        qWarning() << "certificate file does not exist or is not readable" << fileName;
        return;
    }

    auto hash = QString(QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5).toHex());

    // If we are here, adding a new file was basically ok. Now create a new model entry.
    int rowIndex = certificateModel->rowCount();
    QFileInfo fileInfo(file.fileName());

    certificateModel->insertRow(certificateModel->rowCount());
    certificateModel->setData(certificateModel->index(rowIndex, 1), fileInfo.baseName());
    certificateModel->setData(certificateModel->index(rowIndex, 2), fileInfo.completeSuffix());
    certificateModel->setData(certificateModel->index(rowIndex, 5), hash);

    // Handle related to extern/internal.
    auto dm = QMDataManager::getInstance();

    if (dm->getCertificateLocation() == CertLoc::EXTERNAL)
    {
        if (QMCertificateDialog::saveFileExternal(file).isEmpty())
        {
            QMessageBox::warning(
                this, tr("Nachweis hinzufügen"),
                tr("Der Nachweis konnte nicht hinzugefügt werden. Bitte informieren Sie den "
                   "Entwickler."));
            certificateModel->revertRow(rowIndex);
            return;
        }
    }
    else
    {
        if (!QMCertificateDialog::saveFileInternal(file))
        {
            certificateModel->revertRow(rowIndex);
        }
    }

    certificateModel->submitAll();
}

QString QMCertificateDialog::saveFileExternal(QFile &file)
{
    auto dm = QMDataManager::getInstance();

    // The path is relative to the database location. This should make sure, that the external
    // files are located in direct vicinity to the database.

    auto db = QSqlDatabase::database("default", false);
    auto dbPath = QFileInfo(db.databaseName()).absolutePath();
    auto certPath = dbPath + QDir::separator() + "certificates";

    // Create certificate path if it does not exist.
    if (!QDir(certPath).exists())
    {
        if (!QDir(certPath).mkpath(certPath))
        {
            qWarning() << "cannot create path" << certPath;
            return {};
        }
    }

    // Inside this base directory, the file should be save in the following structure:
    // certificates/<current year>/<current month>/<current day>/<filename>

    auto insidePath = QDate::currentDate().toString(Qt::ISODate).replace("-", QDir::separator());
    auto fullPath = certPath + QDir::separator() + insidePath;

    if (!QDir(fullPath).exists())
    {
        if (!QDir(fullPath).mkpath(fullPath))
        {
            qWarning() << "cannot create path" << fullPath;
            return {};
        }
    }

    auto certFileInfo = QFileInfo(file.fileName());
    auto hash = QString(QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5).toHex());
    auto fullFileName = fullPath + QDir::separator() + hash + "." + certFileInfo.completeSuffix();

    if (QDir(fullFileName).exists())
    {
        qWarning() << "file does already exist" << fullFileName;
        return {};
    }

    // Copy file to target.
    if (!file.copy(fullFileName))
    {
        qWarning() << "cannot copy file to" << fullFileName;
        return {};
    }

    return fullFileName;
}

bool QMCertificateDialog::saveFileInternal(QFile &file)
{
    return false;
}
