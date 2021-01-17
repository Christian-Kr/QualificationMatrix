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
#include "qmnewcertificatedialog.h"

#include <QSqlTableModel>
#include <QSortFilterProxyModel>
#include <QFileDialog>
#include <QCryptographicHash>
#include <QDate>
#include <QMessageBox>
#include <QKeyEvent>
#include <QDesktopServices>
#include <QTemporaryFile>

#include <QDebug>

QMCertificateDialog::QMCertificateDialog(Mode mode, QWidget *parent)
    : QDialog(parent)
{
    ui = new Ui::QMCertificateDialog;
    ui->setupUi(this);

    runMode = mode;
    selectedId = -1;

    typeFilterModel = new QSortFilterProxyModel(this);
    nameFilterModel = new QSortFilterProxyModel(this);

    if (runMode == Mode::MANAGE)
    {
        ui->buttonBox->setStandardButtons(QDialogButtonBox::Close);
        setWindowTitle(tr("Nachweise verwalten"));
    }
    else
    {
        ui->buttonBox->setStandardButtons(QDialogButtonBox::Ok);
        setWindowTitle(tr("Nachweis auswählen"));
    }
}

QMCertificateDialog::~QMCertificateDialog()
{
    delete ui;
}

void QMCertificateDialog::accept()
{
    auto modelIndex = ui->tvFiles->selectionModel()->selectedRows();

    if (modelIndex.size() > 0)
    {
        auto row = modelIndex.at(0).row();
        selectedId = nameFilterModel->data(nameFilterModel->index(row, 0)).toInt();
    }

    QDialog::accept();
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
    trainDataCertificateModel = dm->getTrainDataCertificateModel();

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

void QMCertificateDialog::addCertificate()
{
    QMNewCertificateDialog newCertDialog(this);
    newCertDialog.setModal(true);
    newCertDialog.exec();
    return;
    auto fileName = QFileDialog::getOpenFileName(
        this, tr("Nachweis hinzufügen"), QDir::homePath(),
        tr("All files (*.*);;JPEG (*.jpg *.jpeg);;PDF (*.pdf)" ));

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
    file.seek(0);

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
        auto certificateFileName = QMCertificateDialog::saveFileExternal(file);

        if (certificateFileName.isEmpty())
        {
            QMessageBox::warning(
                this, tr("Nachweis hinzufügen"),
                tr("Der Nachweis konnte nicht hinzugefügt werden. Bitte informieren Sie den "
                   "Entwickler."));
            certificateModel->revertRow(rowIndex);
            return;
        }

        certificateModel->setData(certificateModel->index(rowIndex, 3), certificateFileName);
    }
    else
    {
        auto blob = file.readAll();
        file.seek(0);

        if (blob.isEmpty())
        {
            QMessageBox::warning(
                this, tr("Nachweis hinzufügen"),
                tr("Der Nachweis konnte nicht hinzugefügt werden. Bitte informieren Sie den "
                   "Entwickler."));
            certificateModel->revertRow(rowIndex);
            return;
        }

        certificateModel->setData(certificateModel->index(rowIndex, 4), blob);
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

    file.seek(0);

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

void QMCertificateDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        return;
    }

    QDialog::keyPressEvent(event);
}

void QMCertificateDialog::showCertificate()
{
    auto modelIndex = ui->tvFiles->selectionModel()->selectedRows();

    if (modelIndex.size() != 1)
    {
        QMessageBox::information(
            this, tr("Nachweis anzeigen"), tr("Es wurde kein Eintrag selektiert."));
        return;
    }

    // Handle related to extern/internal. Create a temporary file and open it.
    auto dm = QMDataManager::getInstance();
    auto row = modelIndex.at(0).row();

    if (dm->getCertificateLocation() == CertLoc::EXTERNAL)
    {
        QString fileName = nameFilterModel->data(nameFilterModel->index(row, 3)).toString();
        QString md5 = nameFilterModel->data(nameFilterModel->index(row, 5)).toString();
        QString type = nameFilterModel->data(nameFilterModel->index(row, 2)).toString();
        QFile file(fileName);
        file.open(QIODevice::ReadOnly);

        QTemporaryFile temp(QDir::tempPath() + "/" + md5 + "XXXXXX" + "." + type, this);
        temp.setAutoRemove(false);
        temp.open();

        temp.write(file.readAll());
        file.close();

        qDebug() << temp.fileName();

        QDesktopServices::openUrl(QUrl::fromLocalFile(temp.fileName()));
    }
    else
    {
        // TODO: Handle internal.
    }
}

void QMCertificateDialog::removeCertificate()
{
    auto res = QMessageBox::question(
        this, tr("Nachweis entfernen"),
        tr("Die Aktion kann zu Schulungseinträgen ohne Nachweis führen. Fortfahren?"),
        QMessageBox::Yes | QMessageBox::No);

    if (res != QMessageBox::Yes)
    {
        return;
    }

    auto modelIndex = ui->tvFiles->selectionModel()->selectedRows();

    if (modelIndex.size() != 1)
    {
        QMessageBox::information(
            this, tr("Nachweis entfernen"), tr("Es wurde kein Eintrag selektiert."));
        return;
    }

    // Look if there is any entry in train data, that has this certificate and delete the entries.
    auto row = modelIndex.at(0).row();
    auto certId = nameFilterModel->data(nameFilterModel->index(row, 0)).toInt();

    for (int i = 0; i < trainDataCertificateModel->rowCount(); i++)
    {
        auto corrId = trainDataCertificateModel->data(
            trainDataCertificateModel->index(i, 2)).toInt();

        if (corrId == certId)
        {
            trainDataCertificateModel->removeRow(i);
        }
    }

    // Remove the file if external system is on.
    auto dm = QMDataManager::getInstance();

    if (dm->getCertificateLocation() == CertLoc::EXTERNAL)
    {
        QString fileName = nameFilterModel->data(nameFilterModel->index(row, 3)).toString();
        QFile::remove(fileName);
    }

    trainDataCertificateModel->submitAll();

    nameFilterModel->removeRow(row);
    certificateModel->submitAll();
}
