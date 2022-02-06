// qmcertificatedialog.cpp is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify it under the terms of the GNU General
// Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option)
// any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with QualificationMatrix.
// If not, see <http://www.gnu.org/licenses/>.

#include "qmcertificatedialog.h"
#include "ui_qmcertificatedialog.h"
#include "model/qmdatamanager.h"
#include "model/qmcertificatemodel.h"
#include "model/qmtraindatacertificatemodel.h"
#include "settings/qmapplicationsettings.h"
#include "qmnewcertificatedialog.h"
#include "ams/qmamsmanager.h"

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
    : QMDialog(parent)
    , ui(new Ui::QMCertificateDialog)
    , mode(mode)
    , selectedId(-1)
    , nameFilterModel(new QSortFilterProxyModel(this))
{
    // Initialize ui from designer.
    ui->setupUi(this);

    // Initialize additional settings for the files table view.
    ui->tvFiles->horizontalHeader()->setStretchLastSection(false);
    ui->tvFiles->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

    // Show different button related to choosen mode.
    if (this->mode == Mode::MANAGE)
    {
        ui->buttonBox->setStandardButtons(QDialogButtonBox::Close);
        setWindowTitle(tr("Nachweise verwalten"));
    }
    else
    {
        ui->buttonBox->setStandardButtons(QDialogButtonBox::Ok);
        setWindowTitle(tr("Nachweis auswählen"));
    }

    // Permission check! If the right permission is not available for the logged in user, disable all controls. To get
    // full access, the user needs the TD_MODE_WRITE flag.
    if (!QMAMSManager::getInstance()->checkPermission(AccessMode::TD_MODE_WRITE))
    {
        ui->tvFiles->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->tbAdd->setEnabled(false);
        ui->tbRemove->setEnabled(false);
    }
}

QMCertificateDialog::~QMCertificateDialog()
{
    delete ui;
}

void QMCertificateDialog::accept()
{
    // Get a list of selected rows. This list might also be zero, if nothing has been selected. Only one selected row
    // at the same time is allowed.
    auto modelIndex = ui->tvFiles->selectionModel()->selectedRows();

    if (modelIndex.size() != 0)
    {
        QMessageBox::information(this, tr("Nachweis auswählen"), tr("Es muss exakt ein Nachweis ausgewählt werden."),
                QMessageBox::Button::Ok);
    }
    else
    {
        // Get the row number from the table view and set the corresponding id and name from the model. The model
        // needs to be the proxy model for filtering.
        auto row = modelIndex.at(0).row();

        selectedId = nameFilterModel->data(nameFilterModel->index(row, 0)).toInt();
        selectedName = nameFilterModel->data(nameFilterModel->index(row, 1)).toString();
    }

    saveSettings();
    QMDialog::accept();
}

void QMCertificateDialog::reject()
{
    saveSettings();
    QDialog::reject();
}

void QMCertificateDialog::loadSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    // Window size
    int width = settings.read("CertificateDialog/Width", 400).toInt();
    int height = settings.read("CertificateDialog/Height", 400).toInt();
    resize(width, height);
}

void QMCertificateDialog::saveSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    // Window size
    settings.write("CertificateDialog/Width", width());
    settings.write("CertificateDialog/Height", height());
}

void QMCertificateDialog::updateData()
{
    // Get the current database and update data only when it is connected.
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        return;
    }

    auto db = QSqlDatabase::database("default");

    certificateModel = std::make_unique<QMCertificateModel>(this, db);
    certificateModel->select();

    trainDataCertificateModel = std::make_unique<QMTrainDataCertificateModel>(this, db);
    trainDataCertificateModel->select();

    nameFilterModel->setSourceModel(certificateModel.get());
    nameFilterModel->setFilterKeyColumn(1);

    // Update the views. Only show name and type.
    ui->tvFiles->setModel(nameFilterModel);
    ui->tvFiles->hideColumn(0);
    ui->tvFiles->hideColumn(2);
    ui->tvFiles->hideColumn(3);
    ui->tvFiles->hideColumn(4);
    ui->tvFiles->hideColumn(5);
    ui->tvFiles->setColumnWidth(1, 400);

    resetFilter();
}

void QMCertificateDialog::updateFilter()
{
    nameFilterModel->setFilterFixedString(ui->leName->text());
}

void QMCertificateDialog::setNameFilter(QString filter)
{
    ui->leName->setText(filter);
    updateFilter();
}

void QMCertificateDialog::resetFilter()
{
    ui->leName->setText("");
}

void QMCertificateDialog::addCertificate()
{
    // Get data from user
    auto &settings = QMApplicationSettings::getInstance();

    auto width = settings.read("NewCertificateDialog/Width", 400).toInt();
    auto height = settings.read("NewCertificateDialog/Height", 600).toInt();

    QMNewCertificateDialog newCertDialog(this);
    newCertDialog.resize(width, height);
    newCertDialog.setModal(true);

    if (newCertDialog.exec() != QDialog::Accepted)
    {
        return;
    }

    // Insert certificate
    QFile file(newCertDialog.getCertPath());
    file.open(QIODevice::ReadOnly);

    if (!file.isReadable() || !file.exists())
    {
        qWarning() << "certificate file does not exist or is not readable" << newCertDialog.getCertPath();
        return;
    }

    auto hash = QString(QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5).toHex());
    file.seek(0);

    // If we are here, adding a new file was basically ok. Now create a new model entry.
    auto rowIndex = certificateModel->rowCount();
    QFileInfo fileInfo(file.fileName());

    certificateModel->insertRow(certificateModel->rowCount());

    // Create the name and set it.
    QString name = newCertDialog.getTrain() + "_" + newCertDialog.getEmployee() +
            newCertDialog.getEmployeeGroup() + "_" + newCertDialog.getTrainDate();
    certificateModel->setData(certificateModel->index(rowIndex, 1), name);
    certificateModel->setData(certificateModel->index(rowIndex, 2), fileInfo.completeSuffix());
    certificateModel->setData(certificateModel->index(rowIndex, 5), hash);
    certificateModel->setData(certificateModel->index(rowIndex, 6), QDate::currentDate().toString("yyyyMMdd"));
    certificateModel->setData(certificateModel->index(rowIndex, 7), newCertDialog.getTrainDate());

    // Handle related to extern/internal. Internal files will be saved directly into the database with a blob. External
    // files will be saved on the file system and a path to that file will be saved into the database.
    // TODO: Make Modes switchable.
    auto dm = QMDataManager::getInstance();

    if (dm->getCertificateLocation() == CertLoc::EXTERNAL)
    {
        auto certificateFileName = QMCertificateDialog::saveFileExternal(file);

        if (certificateFileName.isEmpty())
        {
            QMessageBox::warning(this, tr("Nachweis hinzufügen"), tr("Der Nachweis konnte nicht hinzugefügt werden."));
            certificateModel->revertRow(rowIndex);
            return;
        }

        certificateModel->setData(certificateModel->index(rowIndex, 3), certificateFileName);

        if (!certificateModel->submitAll())
        {
            QMessageBox::warning(this, tr("Nachweis hinzufügen"),
                    tr("Der Nachweis konnte hinzugefügt aber die Tabelle nicht aktualisiert werden. "
                    "Die Datei und der Eintrag werden wieder entfernt."));
            certificateModel->revertAll();
            QFile::remove(certificateFileName);
        }
    }
    else
    {
        auto blob = file.readAll();
        file.seek(0);

        if (blob.isEmpty())
        {
            QMessageBox::warning(this, tr("Nachweis hinzufügen"),
                    tr("Der Nachweis konnte nicht hinzugefügt werden. Bitte informieren Sie den Entwickler."));
            certificateModel->revertRow(rowIndex);
            return;
        }

        certificateModel->setData(certificateModel->index(rowIndex, 4), blob);
        if (!certificateModel->submitAll())
        {
            QMessageBox::warning(this, tr("Nachweis hinzufügen"),
                    tr("Der Nachweis konnte hinzugefügt aber die Tabelle nicht aktualisiert werden. "
                    "Die Datei und der Eintrag werden wieder entfernt."));
            certificateModel->revertAll();
        }
    }
}

QString QMCertificateDialog::saveFileExternal(QFile &file)
{
    // The path is relative to the database location. This should make sure, that the external files are located in
    // direct vicinity to the database.

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
        QMessageBox::critical(this, tr("Nachweis hinzufügen"), tr("Es existiert bereits eine Datei mit dem Namen. "
                "Der gleiche Nachweis kann pro Tag nur einmal hinzugefügt werden."), QMessageBox::Button::Ok);
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
