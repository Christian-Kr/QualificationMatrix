// qmcertificateintegritycheckdialog.cpp is part of QualificationMatrix
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

#include "qmcertificateintegritycheckdialog.h"
#include "ui_qmcertificateintegritycheckdialog.h"
#include "model/certificate/qmcertificatemodel.h"
#include "settings/qmapplicationsettings.h"

#include <QSqlTableModel>
#include <QKeyEvent>
#include <QFileDialog>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QDate>
#include <QDateTime>
#include <QMessageBox>
#include <QFileInfo>
#include <QTextStream>
#include <QCryptographicHash>

#include <QDebug>

QMCertificateIntegrityCheckDialog::QMCertificateIntegrityCheckDialog(QWidget *parent)
    : QMDialog(parent)
{
    ui = new Ui::QMCertificateIntegrityCheckDialog;
    ui->setupUi(this);

    updateData();
}

QMCertificateIntegrityCheckDialog::~QMCertificateIntegrityCheckDialog()
{
    delete ui;
}

void QMCertificateIntegrityCheckDialog::accept()
{
    saveSettings();

    runCheck();

    QDialog::accept();
}

void QMCertificateIntegrityCheckDialog::runCheck()
{
    // Check data of ui elements.
    auto deleteUnlinkedFiles = ui->cbDeleteUnlinkedFiles->isChecked();
    auto recreateHash = ui->cbRecreateHash->isChecked();
    auto logFilePath = ui->leLogPath->text();

    // Open log file.
    auto logFileName =
        logFilePath + QDir::separator() + "log_" +
        QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss") + ".txt";
    QFileInfo logFileInfo(logFileName);
    QFile logFile(logFileName);

    if (logFileInfo.exists())
    {
        QMessageBox::warning(
            this, tr("Nachweisdatenbank überprüfen"), tr("Die Log-Datei existiert bereits."));
        return;
    }

    if (!logFile.open(QIODevice::ReadWrite))
    {
        QMessageBox::warning(
            this, tr("Nachweisdatenbank überprüfen"),
            tr("Die Log-Datei konnte nicht zum Schreiben geöffnet werden."));
        return;
    }

    QTextStream logStream(&logFile);

    auto db = QSqlDatabase::database("default", false);
    auto dbPath = QFileInfo(db.databaseName()).absolutePath();
    auto certPath = dbPath + QDir::separator() + "certificates";

    // This is the first check: Go through all the entries in table Certificates and look if
    // every file exist. Additionally (if wanted) generated an md5 hash tag and compare it to the
    // saved on in the table (column "md5_hash").
    for (int i = 0; i < certificateModel->rowCount(); i++)
    {
        QString tmpCertPath = certificateModel->data(certificateModel->index(i, 3)).toString();

        // Does the file really exist?
        if (!QFileInfo(tmpCertPath).exists())
        {
            logStream << tr("Nachfolgende Datei fehlt: ") << tmpCertPath << "\n";
            continue;
        }

        // Make hash test.
        if (recreateHash)
        {
            QFile tmpFile(tmpCertPath);
            tmpFile.open(QIODevice::ReadOnly);

            auto hash = QString(QCryptographicHash::hash(
                tmpFile.readAll(), QCryptographicHash::Md5).toHex());

            auto tmpHash = certificateModel->data(certificateModel->index(i, 5)).toString();

            if (hash != tmpHash)
            {
                logStream << tr("Hash weicht ab: ") << tmpCertPath << "\n";
                continue;
            }
        }
    }

    // This is the second check. Go through all files on the file system and search for files that
    // are not part of the table Certificates.
    QDirIterator it(certPath, QDir::Files, QDirIterator::Subdirectories);

    while (it.hasNext())
    {
        QString fileName = it.next();
        auto found = false;

        for (int i = 0; i < certificateModel->rowCount(); i++)
        {
            QString tmpCertPath = certificateModel->data(certificateModel->index(i, 3)).toString();

            if (fileName == tmpCertPath)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            logStream << tr("Datei existiert nicht in der Tabelle: ") << fileName;

            if (deleteUnlinkedFiles)
            {
                if (QFile::remove(fileName))
                {
                    logStream << tr("Datei gelöscht: ") << fileName;
                }
                else
                {
                    logStream << tr("Datei konnte nicht gelöscht werden: ") << fileName;
                }
            }
        }
    }

    logFile.close();
}

void QMCertificateIntegrityCheckDialog::openLogPath()
{
    auto logPath = QFileDialog::getExistingDirectory(
        this, tr("Ordner für Log-Datei"), QDir::homePath());

    if (logPath.isEmpty())
    {
        return;
    }

    if (!QDir(logPath).exists())
    {
        qWarning() << "path does not exist, although it should";
        return;
    }

    ui->leLogPath->setText(logPath);
}

void QMCertificateIntegrityCheckDialog::saveSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    // Window settings.
    settings.write("CertificateIntegrityCheckDialog/Width", width());
    settings.write("CertificateIntegrityCheckDialog/Height", height());
}

void QMCertificateIntegrityCheckDialog::updateData()
{
    // Get the current database and update data only when it is connected.
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        return;
    }

    auto db = QSqlDatabase::database("default");

    certificateModel = std::make_unique<QMCertificateModel>(this, db);
    certificateModel->select();

    // Set ui.
    ui->cbCertificateMode->clear();
    ui->cbCertificateMode->addItem(tr("Extern"));
    ui->cbCertificateMode->addItem(tr("Intern"));
    ui->cbCertificateMode->setCurrentText(tr("Extern"));
    ui->cbCertificateMode->setEnabled(false);
}
