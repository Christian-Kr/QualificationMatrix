//
// qmmainwindow.cpp is part of QualificationMatrix
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

#include "qmmainwindow.h"
#include "ui_qmmainwindow.h"

#include "settings/qmsettingsdialog.h"
#include "qualimatrix/qmqualimatrixmodel.h"
#include "qualimatrix/qmqualimatrixheaderview.h"
#include "qualiresult/qmqualiresultmodel.h"
#include "model/qmdatamanager.h"
#include "qualimatrix/qmqualimatrixwidget.h"
#include "qualiresult/qmqualiresultwidget.h"
#include "settings/qmapplicationsettings.h"
#include "traindata/qmtraindatadialog.h"
#include "database/qmdatabasedialog.h"
#include "database/qmdatabaseupdatedialog.h"
#include "database/qmdatabaseupdater.h"
#include "certificate/qmcertificatedialog.h"
#include "certificate/qmcertificateintegritycheckdialog.h"
#include "signinglist/qmsigninglistdialog.h"

#include <QProgressDialog>
#include <QDesktopWidget>
#include <QSqlRelationalTableModel>
#include <QMessageBox>
#include <QFileDialog>
#include <QSqlDatabase>
#include <QFileInfo>
#include <QCloseEvent>
#include <QStyledItemDelegate>
#include <QSqlRelationalDelegate>
#include <QtPrintSupport/QPrinter>
#include <QPainter>
#include <QDebug>
#include <QScreen>
#include <QDateTime>
#include <QSqlQuery>
#include <QSqlError>
#include <QTextStream>
#include <QTranslator>
#include <QWindow>

QMMainWindow::QMMainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    ui = new Ui::QMMainWindow;
    ui->setupUi(this);

    // Creating widgets here, doesn't mean they will be shown on startup.
    qualiResultWidget = std::make_unique<QMQualiResultWidget>();
    qualiMatrixWidget = std::make_unique<QMQualiMatrixWidget>();

    initConnections();

    // Load database on startup. If there are some settings for automatic loading of database on
    // startup, follow it. Otherwise show the manage database dialog, cause the application is not
    // useful if there is no database loaded.
    initDatabaseSettings();
}

QMMainWindow::~QMMainWindow()
{
    delete ui;
}

void QMMainWindow::initConnections() const
{
    auto dm = QMDataManager::getInstance();

    connect(dm, &QMDataManager::beforeInitializeModels, this,
        &QMMainWindow::beforeInitializeModels);
    connect(dm, &QMDataManager::updateInitializeModels, this, &QMMainWindow::updateProgress);
    connect(dm, &QMDataManager::afterInitializeModels, this, &QMMainWindow::afterInitializeModels);
}

void QMMainWindow::initDatabaseSettings()
{
    auto &settings = QMApplicationSettings::getInstance();
    auto loadLast = settings.read("Database/LoadLastOnStartup", false).toBool();

    if (loadLast)
    {
        if (!manageDatabaseFromSettings())
        {
            manageDatabase();
        }
    }
    else
    {
        manageDatabase();
    }
}

bool QMMainWindow::manageDatabaseFromSettings()
{
    loadDatabaseFromSettings("default");
    auto db = QSqlDatabase::database("default", false);

    // If opening the database fails, show a message and stop automatic loading the database. If
    // the opening fails, this could be due to wrong database information and/or failure in the
    // database itself.
    if (!db.open())
    {
        QMessageBox::critical(
            this, tr("Datenbankverbindung"),
            tr("Die Datenbank konnte nicht geöffnet werden, überprüfen Sie ihre Einstellungen."));
        return false;
    }

    // Run initializing actions after database has been loaded.
    initAfterDatabaseOpened();
    return true;
}

void QMMainWindow::manageDatabase()
{
    // Before the dialog for managing a database will be shown, close a currently loaded one.
    if (QSqlDatabase::contains("default") && QSqlDatabase::database("default", false).isOpen())
    {
        auto res = QMessageBox::question(
            this, tr("Datenbank verwalten"),
            tr("Es besteht bereits eine Verbindung zu einer Datenbank.\n\nJetzt trennen?"),
            QMessageBox::Yes | QMessageBox::No);

        if (res == QMessageBox::No)
        {
            return;
        }

        closeDatabase();
    }

    // The manage dialog will manipulate the data in QSqlDatabase. This is the buffer for the
    // settings which will be saved in application settings afterwards.
    QMDatabaseDialog databaseDialog("default", this);
    databaseDialog.exec();

    // If there is no default database or it is not open, something went wrong.
    if (QSqlDatabase::contains("default") && QSqlDatabase::database("default", false).isOpen())
    {
        saveDatabaseSettings();
        initAfterDatabaseOpened();
    }
}

void QMMainWindow::loadDatabaseFromSettings(const QString &dbName)
{
    // If a database with the name exist, remove it. This is needed, because the driver can only
    // be set during object creation.
    if (QSqlDatabase::contains(dbName))
    {
        closeDatabase();
        QSqlDatabase::removeDatabase(dbName);
    }

    auto &settings = QMApplicationSettings::getInstance();
    auto databaseType = settings.read("Database/Type", "local").toString();
    auto driver = settings.read("Database/Driver", "QSQLITE").toString();
    auto db = QSqlDatabase::addDatabase(driver, dbName);

    if (databaseType == "local")
    {
        db.setDatabaseName(settings.read("Database/FileName").toString());
    }
    else
    {
        db.setDatabaseName(settings.read("Database/Name").toString());
        db.setHostName(settings.read("Database/HostName").toString());
        db.setPort(settings.read("Database/Port").toInt());
        db.setUserName(settings.read("Database/UserName").toString());
        db.setPassword(settings.read("Database/Password").toString());
    }
}

void QMMainWindow::saveDatabaseSettings()
{
    if (QSqlDatabase::contains("default"))
    {
        auto db = QSqlDatabase::database("default", false);
        auto &settings = QMApplicationSettings::getInstance();

        if (db.driverName() == "QSQLITE")
        {
            settings.write("Database/Type", "local");
            settings.write("Database/FileName", db.databaseName());
            settings.write("Database/Driver", db.driverName());
        }
        else
        {
            settings.write("Database/Type", "remote");
            settings.write("Database/Name", db.databaseName());
            settings.write("Database/HostName", db.hostName());
            settings.write("Database/UserName", db.userName());
            settings.write("Database/Password", db.password());
            settings.write("Database/Port", db.port());
            settings.write("Database/Driver", db.driverName());
        }
    }
}

void QMMainWindow::initAfterDatabaseOpened()
{
    auto db = QSqlDatabase::database("default", false);
    auto dm = QMDataManager::getInstance();

    // If the version of the database doesn't fit the software version, show a message asking if
    // the database should be updated or not.
    if (!dm->testVersion(db))
    {
        auto resMb = QMessageBox::question(
            this, tr("Datenbank laden"), tr(
                "Die Version der Datenbank entspricht nicht der Vorgabe. Möchten Sie versuchen"
                " die Datenbank zu aktualisieren?"), QMessageBox::Yes | QMessageBox::No);

        if (resMb != QMessageBox::Yes)
        {
            closeDatabase();
            return;
        }

        auto resUd = QMDatabaseUpdateDialog("default", this).exec();

        if (resUd != QDialog::Accepted)
        {
            closeDatabase();
            return;
        }

        // A database backup should only be run, when the database is a local one. On a remote
        // database the administrator of the database system has to take care of backups. For now,
        // only a database with the QSQLITE driver is a local one.
        if (db.driverName() == "QSQLITE")
        {
            if (!saveSingleDatabaseBackup(db))
            {
                QMessageBox::critical(
                    this, tr("Datenbank-Backup"),
                    tr("Es konnte kein Backup erstellt werden. Da für eine Aktualisierung ein "
                       "Backup notwendig ist, wird die Aktion abgebrochen und die Datenbank "
                       "geschlossen."));
                closeDatabase();
                return;
            }
        }

        QMDatabaseUpdater databaseUpdater;
        if (!databaseUpdater.updateDatabase(db))
        {
            QMessageBox::critical(
                this, tr("Datenbank aktualisieren"),
                tr("Die Datenbank konnte nicht vollständig aktualisiert werden. Der Fehler ist "
                   "kritisch. Bitte spielen Sie das vorher angelegte Backup ein. Die Datenbank "
                   "wird geschlossen und nicht weiter verarbeitet."));
            closeDatabase();
            return;
        }
        else
        {
            QMessageBox::information(
                this, tr("Datenbank aktualisieren"),
                tr("Die Aktualisierung der Datenbank war erfolgreich. Bitte heben Sie das "
                   "erstellte Backup für einen späteren Fall auf."));
        }
    }

    // After database has been loaded and version is ok, load the database models and informate
    // the user about that.
    dm->initializeModels(db);
    ui->statusbar->showMessage(tr("Datenbank verbunden"));
    setWindowTitle("QualificationMatrix - " + db.databaseName());
}

bool QMMainWindow::saveSingleDatabaseBackup(const QSqlDatabase &db)
{
    auto fileName = QFileDialog::getSaveFileName(this, tr("Datenbank-Backup"), QDir::homePath());

    if (fileName.isEmpty())
    {
        qWarning() << "no database backup name has been choosen";
        return false;
    }

    if (!QFile::copy(db.databaseName(), fileName))
    {
        qWarning() << "cannot create a copy of database";
        return false;
    }

    return true;
}

void QMMainWindow::beforeInitializeModels(int maxSteps)
{
    showProgress(tr("Datenbank"), tr("Daten abrufen..."), 0, maxSteps);
}

void QMMainWindow::beforeQualiResultCalculation(int maxSteps)
{
    showProgress(tr("Schulungsresultat"), tr("Berechnung..."), 0, maxSteps);
}

void QMMainWindow::beforeQualiMatrixBuildCache(int maxSteps)
{
    showProgress(tr("Qualifikationsmatrix"), tr("Cache aufbauen..."), 0, maxSteps);
}

void QMMainWindow::showProgress(
    const QString &title, const QString &text, const int &minSteps, const int &maxSteps)
{
    // The progress dialog object will be recreated on every use, cause it would show up by itself
    // when ui does not react for view seconds (qt related).

    if (progressDialog == nullptr)
    {
        progressDialog = std::make_unique<QProgressDialog>(this);
    }

    progressDialog->setWindowTitle(title);
    progressDialog->setCancelButton(nullptr);
    progressDialog->setMinimum(minSteps);
    progressDialog->setMaximum(maxSteps);
    progressDialog->setLabelText(text);
    progressDialog->setModal(true);
    progressDialog->setVisible(true);
}

void QMMainWindow::updateProgress(int currentStep)
{
    progressDialog->setValue(currentStep);

    // Keep the ui alive.
    QApplication::processEvents();
}

void QMMainWindow::closeProgress()
{
    if (progressDialog != nullptr)
    {
        progressDialog->close();

        // Kills the object (although method name doesn't suggest).
        progressDialog.reset();
    }
}

void QMMainWindow::afterInitializeModels()
{
    closeProgress();

    // All models are initialized now. The quali matrix models and the quali result models have
    // action that might take a long time. Therefore each of them needs to be connected to
    // functions showing progress dialogs. Exaclty the same as "initializeModels".

    // todo: why are the connections build here and not before?

    auto dm = QMDataManager::getInstance();

    connect(
        dm->getQualiMatrixModel().get(), &QMQualiMatrixModel::beforeBuildCache, this,
        &QMMainWindow::beforeQualiMatrixBuildCache);

    connect(
        dm->getQualiMatrixModel().get(), &QMQualiMatrixModel::updateBuildCache, this,
        &QMMainWindow::updateProgress);

    connect(
        dm->getQualiMatrixModel().get(), &QMQualiMatrixModel::afterBuildCache, this,
        &QMMainWindow::closeProgress);

    connect(
        dm->getQualiResultModel().get(), &QMQualiResultModel::beforeUpdateQualiInfo, this,
        &QMMainWindow::beforeQualiResultCalculation);

    connect(
        dm->getQualiResultModel().get(), &QMQualiResultModel::updateUpdateQualiInfo, this,
        &QMMainWindow::updateProgress);

    connect(
        dm->getQualiResultModel().get(), &QMQualiResultModel::afterUpdateQualiInfo, this,
        &QMMainWindow::closeProgress);

    // After the models have been initialized and connected, the widgets need to be set up.
    qualiResultWidget->updateData();
    ui->twQualiMatrix->addTab(qualiResultWidget.get(), tr("Qualifizierungsergebnis"));

    qualiMatrixWidget->updateData();
    ui->twQualiMatrix->addTab(qualiMatrixWidget.get(), tr("Qualifizierungsmatrix"));

    // Unlock all ui elements.
    ui->actSettings->setEnabled(true);
    ui->actTrainData->setEnabled(true);
    ui->actCloseDatabase->setEnabled(true);
}

void QMMainWindow::showAboutQt()
{
    QMessageBox::aboutQt(this);
}

void QMMainWindow::showAbout()
{
    QMessageBox::about(
        this, tr("Über QualificationMatrix"),
        tr("Copyright (c) 2020 by Christian Kr"
           "\nVersion:\t0.1-Alpha"
           "\nLizenz:\tGNU GENERAL PUBLIC LICENSE Version 3"
           "\n\nEine Kopie der Lizenz wird mit dem Quellcode der Software mitgeliefert"
           " (COPYING.txt)"
           "\n\nhttps://github.com/Christian-Kr/QualificationMatrix"
           "\n\nBug-Reports: Fehler können direkt auf GitHub gemeldet werden oder"
           "per E-Mail an CerebrosuS_aedd_gmx.net"));
}

bool QMMainWindow::runAutoBackup(const QString &fileName)
{
    // Look if the path exist.
    QMApplicationSettings &settings = QMApplicationSettings::getInstance();
    QFileInfo pathInfo(settings.read("Database/LocalBackupPath").toString());

    if (!pathInfo.isDir() || !pathInfo.exists() || !pathInfo.isWritable())
    {
        QMessageBox::critical(
            this, tr("Backup anlegen"), tr(
                "Der angegebene Ordner in den Einstellungen ist kein Verzeichnis, "
                " existiert nicht oder ist nicht beschreibbar."
            ));

        return false;
    }

    // Get a list of all backup files for the given database.
    QString preName = fileName.split("/").last().split(".").first();
    QDir backupDir(pathInfo.absoluteFilePath());
    QStringList filters;
    filters << QString(preName + "-*.qmsql");
    backupDir.setNameFilters(filters);
    backupDir.setSorting(QDir::Name);
    QFileInfoList backupFileList = backupDir.entryInfoList();

    // If auto backup delete is on and too many backup files exist, delete the last x backups
    // to fit the maximum number of backup counts. The files have a timestamp. Therefore the
    // sorting is from oldest to newest.

    auto varAutoBackupDelete = settings.read("Database/LocalAutoBackupDelete");
    auto autoBackupDelete = (varAutoBackupDelete.isNull()) ? false : varAutoBackupDelete.toBool();
    auto varBackupCount = settings.read("Database/LocalBackupCount");
    auto backupCount = (varBackupCount.isNull()) ? 10 : varBackupCount.toInt();

    if (autoBackupDelete && backupFileList.size() >= backupCount)
    {
        // Delete files till backupCount is ok.
        while (backupFileList.size() >= backupCount)
        {
            QFileInfo deleteFileInfo = backupFileList.takeFirst();
            QFile deleteFile(deleteFileInfo.absoluteFilePath());
            if (!deleteFile.remove())
            {
                QMessageBox::critical(
                    this, tr("Backup löschen"),
                    tr("Die Backupdatei konnte nicht gelöscht werden und wird übersprungen."));
            }
        }
    }

    // Create new backup certificate.
    QString newName = QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm-ss");
    newName = preName + "-" + newName + ".qmsql";
    QFile copyFile(fileName);
    if (!copyFile.copy(pathInfo.absoluteFilePath() + QDir::separator() + newName))
    {
        QMessageBox::critical(
            this, tr("Backup erstellen"),
            tr("Die Backupdatei konnte nicht kopiert werden und wird übersprungen."));
    }

    return true;
}

bool QMMainWindow::closeDatabase()
{
    // If a database with a driver exist, close it.
    if (QSqlDatabase::contains("default"))
    {
        auto db = QSqlDatabase::database("default", false);

        if (db.isOpen())
        {
            auto res = QMessageBox::question(
                this, tr("Datenbank schließen"),
                tr("Soll die verbundene Datenbank wirklich geschlossen werden? (Nicht gespeicherte"
                   " Daten gehen verloren!)"), QMessageBox::Yes | QMessageBox::No);

            switch (res)
            {
                case QMessageBox::Yes:
                    // TODO: save database
                    break;
                case QMessageBox::No:
                    // TODO: nothing to do
                    break;
                default:
                    break;
            }

            // close the database
            // TODO: do more related stuff to reset everything
            db.close();
        }
    }

    // After database has been closed, everything needs to be deleted or locked.
    ui->twQualiMatrix->clear();
    ui->actSettings->setEnabled(false);
    ui->actTrainData->setEnabled(false);
    ui->actCloseDatabase->setEnabled(false);

    // Set ui elements.
    setWindowTitle(tr("QualificationMatrix"));
    ui->statusbar->showMessage(tr("Datenbank getrennt"));

    return true;
}

void QMMainWindow::closeEvent(QCloseEvent *event)
{
    QMainWindow::closeEvent(event);

    // TODO: Implement further close action.
    saveSettings();
}

void QMMainWindow::saveSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    settings.write("MainWin/Screen", windowHandle()->screen()->name());
    settings.write("MainWin/Width", width());
    settings.write("MainWin/Height", height());
    settings.write("MainWin/Maximized", windowState().testFlag(Qt::WindowMaximized));
}

void QMMainWindow::showTrainData()
{
    auto &settings = QMApplicationSettings::getInstance();
    auto varWidth = settings.read("TrainDataDialog/Width");
    auto width = (varWidth.isNull()) ? 400 : varWidth.toInt();
    auto varHeight = settings.read("TrainDataDialog/Height");
    auto height = (varHeight.isNull()) ? 400 : varHeight.toInt();

    QMTrainDataDialog trainDataDialog(this);
    trainDataDialog.resize(width, height);
    trainDataDialog.setModal(true);
    trainDataDialog.exec();
}

void QMMainWindow::showSettings()
{
    // Create the settings dialog.
    QMSettingsDialog settingsDialog(this);
    settingsDialog.exec();

    // If no database has been loaded, don't update anything.
    QSqlDatabase db = QSqlDatabase::database("default", false);
    if (!db.isOpen())
    {
        return;
    }

    // Reset text in filter, cause when the models will be updated by changed settings, it will
    // reset the filter entries to the first model values.

    // For the quali matrix, the whole matrix needs to be reseted.
    qualiMatrixWidget->resetFilter();
    qualiMatrixWidget->updateFilter();
    qualiMatrixWidget->updateHeaderCache();
    qualiMatrixWidget->updateColors();

    // For the quali result, reseting is not a good option, cause this might take a while. Instead
    // the result should be reseted.
    qualiResultWidget->resetFilter();
    qualiResultWidget->resetModel();
}

void QMMainWindow::createEmptyDatabase()
{
    // For a local database as a certificate, it will be created. For a remote database, the database have
    // to exist. Then this function will only create the needed tables.

    // Create a temporary database connection.
    // TODO: Won't work anymore, cause the api of QDatabaseDialog has changed.
    QMDatabaseDialog databaseDialog("tmp", this);
    if (databaseDialog.exec() != QDialog::DialogCode::Accepted)
    {
        return;
    }

    if (!QSqlDatabase::contains("tmp"))
    {
        return;
    }

    {
        QSqlDatabase db = QSqlDatabase::database("tmp");
        if (!db.open())
        {
            QSqlDatabase::removeDatabase("tmp");
            return;
        }

        // Open sql certificate for query.
        QString fileName = QFileDialog::getOpenFileName(
            this, tr("Öffne Sql-Datei"), QDir::homePath(), tr("Sql (*.sql)"));
        if (fileName.isEmpty())
        {
            db.close();
            QSqlDatabase::removeDatabase("tmp");

            return;
        }

        QSqlQuery query(db);
        QFile fileSql(fileName);
        fileSql.open(QIODevice::Text | QIODevice::ReadWrite);
        QString script = QTextStream(&fileSql).readAll();
        QStringList statements = script.split(";");
        for (int i = 0; i < statements.size(); i++)
        {
            qDebug() << query.exec(statements.at(i)) << query.lastError().text();
        }

        db.close();
    }

    QSqlDatabase::removeDatabase("tmp");
}

void QMMainWindow::manageCertificate()
{
    auto &settings = QMApplicationSettings::getInstance();

    auto varWidth = settings.read("CertificateDialog/Width");
    auto width = (varWidth.isNull()) ? 400 : varWidth.toInt();
    auto varHeight = settings.read("CertificateDialog/Height");
    auto height = (varHeight.isNull()) ? 400 : varHeight.toInt();

    QMCertificateDialog certDialog(Mode::MANAGE, this);
    certDialog.updateData();
    certDialog.resize(width, height);
    certDialog.setModal(true);
    certDialog.exec();
}

void QMMainWindow::showCertificateIntegrityCheck()
{
    auto &settings = QMApplicationSettings::getInstance();

    auto varWidth = settings.read("CertificateIntegrityCheckDialog/Width");
    auto width = (varWidth.isNull()) ? 400 : varWidth.toInt();
    auto varHeight = settings.read("CertificateIntegrityCheckDialog/Height");
    auto height = (varHeight.isNull()) ? 400 : varHeight.toInt();

    QMCertificateIntegrityCheckDialog certIntCheckDialog(this);
    certIntCheckDialog.resize(width, height);
    certIntCheckDialog.setModal(true);
    certIntCheckDialog.exec();
}

void QMMainWindow::showCreateSigningList()
{
    auto &settings = QMApplicationSettings::getInstance();

    auto varWidth = settings.read("SigningListDialog/Width");
    auto width = (varWidth.isNull()) ? 400 : varWidth.toInt();
    auto varHeight = settings.read("SigningListDialog/Height");
    auto height = (varHeight.isNull()) ? 400 : varHeight.toInt();

    QMSigningListDialog signingListDialog(this);
    signingListDialog.resize(width, height);
    signingListDialog.setModal(true);
    signingListDialog.exec();
}
