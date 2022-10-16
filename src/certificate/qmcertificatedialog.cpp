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
#include "data/qmdatamanager.h"
#include "data/certificate/qmcertificatemodel.h"
#include "data/trainingdata/qmtraindatacertificatemodel.h"
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

    if (modelIndex.size() != 1)
    {
        QMessageBox::information(this, tr("Nachweis auswählen"), tr("Es muss exakt ein Nachweis ausgewählt werden."),
                QMessageBox::Button::Ok);
    }
    else
    {
        // Get the row number from the table view and set the corresponding id and name from the data. The data
        // needs to be the proxy data for filtering.
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
    // TODO: call new certificate dialog
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

void QMCertificateDialog::filterNameReturnPressed()
{
    updateFilter();
    ui->leName->selectAll();
}
