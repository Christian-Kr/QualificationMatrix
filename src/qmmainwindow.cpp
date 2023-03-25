// qmmainwindow.cpp is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify it under the terms of the GNU General
// Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option)
// any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with QualificationMatrix. If not, see
// <http://www.gnu.org/licenses/>.

#include "qmmainwindow.h"
#include "ui_qmmainwindow.h"

#include "config.h"
#include "settings/qmsettingsdialog.h"
#include "qualimatrix/qmqualimatrixmodel.h"
#include "qualimatrix/qmqualimatrixheaderview.h"
#include "qualiresult/qmqualiresultmodel.h"
#include "data/qmdatamanager.h"
#include "qualimatrix/qmqualimatrixwidget.h"
#include "qualiresult/qmqualiresultwidget.h"
#include "settings/qmapplicationsettings.h"
#include "traindata/qmtraindatawidget.h"
#include "database/qmdatabaseupdatedialog.h"
#include "database/qmdatabaseupdater.h"
#include "framework/component/qminfolabel.h"
#include "certificate/qmcertificatedialog.h"
#include "certificate/qmcertificateintegritycheckdialog.h"
#include "certificate/qmnewcertificatedialog.h"
#include "signinglist/qmsigninglistdialog.h"
#include "framework/qmsqltablemodel.h"
#include "ams/qmamsmanager.h"
#include "ams/qmamslogindialog.h"
#include "backup/qmbackupmanager.h"

#include <QProgressDialog>
#include <QSqlRelationalTableModel>
#include <QMessageBox>
#include <QFileDialog>
#include <QSqlDatabase>
#include <QFileInfo>
#include <QCloseEvent>
#include <QStyledItemDelegate>
#include <QSqlRelationalDelegate>
#include <QPrinter>
#include <QPainter>
#include <QSqlQuery>
#include <QSqlError>
#include <QTranslator>
#include <QWindow>
#include <QStatusBar>
#include <QToolButton>

QMMainWindow::QMMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , winMode(WIN_MODE::NONE)
    , lastWinMode(WIN_MODE::NONE)
    , m_signingListDialog(std::make_unique<QMSigningListDialog>(this))
    , m_amsLoginDialog(std::make_unique<QMAMSLoginDialog>(this))
{
    ui = new Ui::QMMainWindow;
    ui->setupUi(this);

    // Initialize connections to AMS.
    auto am = QMAMSManager::getInstance();
    connect(am, &QMAMSManager::loginStateChanged, this, &QMMainWindow::handleLoginChange);

    // Initialize some ui elements.
    auto *tbAMS = dynamic_cast<QToolButton *>(ui->tbaUser->widgetForAction(ui->actAMS));
    tbAMS->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    tbAMS->setText("");
    tbAMS->setPopupMode(QToolButton::InstantPopup);
    tbAMS->removeAction(ui->actAMS);
    tbAMS->addAction(ui->actLogin);
    tbAMS->addAction(ui->actLogout);

    // QMSigningListDialog
    auto &settings = QMApplicationSettings::getInstance();

    auto varWidth = settings.read("SigningListDialog/Width");
    auto width = (varWidth.isNull()) ? 400 : varWidth.toInt();
    auto varHeight = settings.read("SigningListDialog/Height");
    auto height = (varHeight.isNull()) ? 400 : varHeight.toInt();

    m_signingListDialog->resize(width, height);
    m_signingListDialog->setModal(true);

    // QMAMSLoginDialog
    connect(m_amsLoginDialog.get(), &QMAMSLoginDialog::finished, this, &QMMainWindow::amsLoginDialogFinished);
}

QMMainWindow::~QMMainWindow()
{
    delete ui;
}

void QMMainWindow::initDatabaseSettings()
{
    auto &settings = QMApplicationSettings::getInstance();
    auto loadLast = settings.read("Database/LoadLastOnStartup", false).toBool();

    if (loadLast)
    {
        manageDatabaseFromSettings();
    }
}

bool QMMainWindow::manageDatabaseFromSettings()
{
    loadDatabaseFromSettings("default");
    auto db = QSqlDatabase::database("default", false);

    // If opening the database fails, show a message and stop automatic loading the database. If the opening fails,
    // this could be due to wrong database information and/or failure in the database itself.
    if (!db.open())
    {
        QMessageBox::critical(this, tr("Datenbankverbindung"),
                tr("Die Datenbank konnte nicht geöffnet werden, überprüfen Sie ihre Einstellungen."));
        return false;
    }

    // Run initializing actions after database has been loaded.
    initAfterDatabaseOpened();
    return true;
}

void QMMainWindow::openDatabase()
{
    // before a new database will be opened, close the current one
    if (QSqlDatabase::contains("default") && QSqlDatabase::database("default", false).isOpen())
    {
        QMessageBox messageBox(this);

        messageBox.setWindowTitle(tr("Datenbank öffnen"));
        messageBox.setText(tr("Eine Datenbank ist bereits geöffnet."));
        messageBox.setInformativeText(
                tr("Wenn du fortfährst, wird die aktuelle Datebank geschlossen. "
                   "Dabei können nicht gespeicherte Daten verloren gehen.\n\n"
                   "Möchtest du trotzdem eine andere Datenbank öffnen?\n"));
        messageBox.setIcon(QMessageBox::Icon::Question);
        messageBox.setStandardButtons(QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::Cancel);

        auto buttonYes = messageBox.button(QMessageBox::StandardButton::Yes);
        Q_ASSERT(buttonYes != nullptr);
        buttonYes->setText(tr("Datenbank öffnen"));

        if (messageBox.exec() != QMessageBox::Yes)
        {
            return;
        }
    }

    // open a different database
    QFileDialog fileDialog(this);

    fileDialog.setWindowTitle(QObject::tr("Datenbank öffnen"));
    fileDialog.setFileMode(QFileDialog::FileMode::ExistingFile);
    fileDialog.setAcceptMode(QFileDialog::AcceptMode::AcceptOpen);
    fileDialog.setNameFilter(QObject::tr("QualificationMatrix Datenbank (*.qmsql)"));

    // if the user rejects the dialog just cancel everything and keep the current database loaded
    if (fileDialog.exec() == QFileDialog::Rejected) {
        return;
    }

    auto fileNames = fileDialog.selectedFiles();

    Q_ASSERT(fileNames.count() <= 1);

    // if no file has been selected, just cancel everything and keep the current database loaded
    if (fileNames.count() == 0) {
        return;
    }

    // if the selected object is not a file, just cancel everything and keep the current database loaded
    QFileInfo configFileInfo(fileNames.first());
    if (!configFileInfo.isFile()) {
        return;
    }

    closeDatabase(true);

    // create the new database; only sqlite is allowed
    QSqlDatabase::addDatabase("QSQLITE", "default");
    auto db = QSqlDatabase::database("default", false);
    db.setDatabaseName(fileNames.first());

    // try to open the database
    if (!db.open())
    {
        QMessageBox::critical(this, tr("Fehler"), tr("Die Datenbank konnte nicht geöffnet werden."));
        return;
    }

    // if there is no database, something went wrong
    if (QSqlDatabase::contains("default") && QSqlDatabase::database("default", false).isOpen())
    {
        saveDatabaseSettings();
        initAfterDatabaseOpened();
    }
}

void QMMainWindow::loadDatabaseFromSettings(const QString &dbName)
{
    // If a database with the name exist, remove it. This is needed, because the driver can only be set during object
    // creation.
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

    // If the version of the database doesn't fit the software version, show a message asking if the database should
    // be updated or not.
    if (!QMDataManager::testVersion(db))
    {
        auto resMb = QMessageBox::question(this, tr("Datenbank laden"), tr("Die Version der Datenbank entspricht "
                "nicht der Vorgabe. Möchten Sie versuchen die Datenbank zu aktualisieren?"),
                QMessageBox::Yes | QMessageBox::No);

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

        // A database backup should only be run, when the database is a local one. On a remote database the
        // administrator of the database system has to take care of backups. For now, only a database with the
        // QSQLITE driver is a local one.
        if (db.driverName() == "QSQLITE")
        {
            if (!saveSingleDatabaseBackup(db))
            {
                QMessageBox::critical(this, tr("Datenbank-Backup"), tr("Es konnte kein Backup erstellt werden. Da für"
                        " eine Aktualisierung ein Backup notwendig ist, wird die Aktion abgebrochen und die Datenbank"
                        " geschlossen."));
                closeDatabase();
                return;
            }
        }

        QMDatabaseUpdater databaseUpdater;
        if (!databaseUpdater.updateDatabase(db))
        {
            QMessageBox::critical(this, tr("Datenbank aktualisieren"), tr("Die Datenbank konnte nicht vollständig "
                    "aktualisiert werden. Der Fehler ist kritisch. Bitte spielen Sie das vorher angelegte Backup ein."
                    " Die Datenbank wird geschlossen und nicht weiter verarbeitet."));
            closeDatabase();
            return;
        }
        else
        {
            QMessageBox::information(this, tr("Datenbank aktualisieren"), tr("Die Aktualisierung der Datenbank war "
                    "erfolgreich. Bitte heben Sie das erstellte Backup für einen späteren Fall auf."));
        }
    }

    // After database has been loaded and version is ok, load the database models and informate the user about it.
    ui->statusbar->showMessage(tr("Datenbank verbunden"));
    setWindowTitle("QualificationMatrix - " + db.databaseName());

    // Make an database backup from the auto system if wanted. Backup should only be run if the driver is QSQLITE,
    // cause this one is file based.
    auto &settings = QMApplicationSettings::getInstance();

    // Run backup trigger of the database.
    auto bm = QMBackupManager::getInstance();

    if (!bm->trigger())
    {
        QMessageBox::warning(this, tr("Backup"), tr("Das Backup konnte nicht erfolgreich durchgeführt werden."));
        return;
    }

    // Initialize the AMS system.
    auto am = QMAMSManager::getInstance();
    if (!am->checkDatabase())
    {
        QMessageBox::warning(this, tr("Rechtemanagement"),
                tr("Die Prüfung der Datenbank für das Rechtemanagement ist fehlgeschlagen."));
        return;
    }

    // Unlock all ui elements.
    ui->actSettings->setEnabled(true);
    ui->actCloseDatabase->setEnabled(true);

    // Do automatic login if wanted.
    if (settings.read("AMS/AutoLogin", false).toBool())
    {
        amsLogin();
    }
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

void QMMainWindow::workloadStarts(const QString& info, int maxSteps)
{
    showProgress(tr("Berechnung"), info, 0, maxSteps);
}

void QMMainWindow::showProgress(const QString &title, const QString &text, const int &minSteps, const int &maxSteps)
{
    // The progress dialog object will be recreated on every use, cause it would show up by itself when ui does not
    // react for view seconds (qt related).

    if (m_progressDialog == nullptr)
    {
        m_progressDialog = std::make_unique<QProgressDialog>(this);
    }

    m_progressDialog->setWindowTitle(title);
    m_progressDialog->setCancelButton(nullptr);
    m_progressDialog->setMinimum(minSteps);
    m_progressDialog->setMaximum(maxSteps);
    m_progressDialog->setLabelText(text);
    m_progressDialog->setModal(true);
    m_progressDialog->open();

    QApplication::processEvents();
}

void QMMainWindow::workloadUpdates(int currentStep)
{
    m_progressDialog->setValue(currentStep);

    // Keep the ui alive.
    QApplication::processEvents();
}

void QMMainWindow::closeProgress()
{
    if (m_progressDialog != nullptr)
    {
        m_progressDialog->close();

        // Kills the object (although method name doesn't suggest).
        m_progressDialog.reset();
    }
}

void QMMainWindow::showAboutQt()
{
    QMessageBox::aboutQt(this);
}

void QMMainWindow::showAbout()
{
    QMessageBox about(this);
    about.setIconPixmap(this->windowIcon().pixmap(QSize(64, 64)));
    about.setWindowTitle(tr("Über QualificationMatrix"));
    about.setText(tr("QualificationMatrix"));
    about.setInformativeText(
            tr("Copyright (c) 2023 by Christian Kr"
               "\nVersion: " VERSION_MAJOR "." VERSION_MINOR " " RELEASE_STATE " // " BUILD
               "\n"
               "Lizenz: GNU GENERAL PUBLIC LICENSE Version 3"
               "\n\n"
               "Eine Kopie der Lizenz wird mit dem Quellcode der Software "
               "mitgeliefert (COPYING.txt)"
               "\n\n"
               "https://github.com/Christian-Kr/QualificationMatrix"
               "\n\n"
               "Bug-Reports: Fehler können direkt auf GitHub gemeldet werden "
               "oder per E-Mail an CerebrosuS_aedd_gmx.net"
               "\n\n"));
    about.exec();
}

bool QMMainWindow::closeDatabase(bool silent)
{
    // if a database exist, close it
    if (QSqlDatabase::contains("default"))
    {
        auto db = QSqlDatabase::database("default", false);

        if (db.isOpen())
        {
            if (silent)
            {
                db.close();
            }
            else
            {
                auto res = QMessageBox::question(
                        this, tr("Datenbank schließen"),
                        tr("Soll die verbundene Datenbank wirklich geschlossen werden?"),
                        QMessageBox::Yes | QMessageBox::No);

                if (res == QMessageBox::No)
                {
                    return false;
                }
                else
                {
                    db.close();
                }
            }
        }
    }

    // After database has been closed, everything needs to be deleted or locked.
    closeCurrentWindowMode();
    ui->actSettings->setEnabled(false);
    ui->actCloseDatabase->setEnabled(false);

    // Logout from database
    amsLogout();

    // Set ui elements
    setWindowTitle(tr("QualificationMatrix"));
    ui->statusbar->showMessage(tr("Datenbank getrennt"));

    return true;
}

void QMMainWindow::closeEvent(QCloseEvent *event)
{
    QMainWindow::closeEvent(event);
    saveSettings();
}

void QMMainWindow::saveSettings()
{
    auto &settings = QMApplicationSettings::getInstance();

    settings.write("MainWin/Screen", windowHandle()->screen()->name());
    settings.write("MainWin/Width", width());
    settings.write("MainWin/Height", height());

    auto testFlag = windowState().testFlag(Qt::WindowMaximized);
    settings.write("MainWin/Maximized", testFlag);
}

void QMMainWindow::showSettings()
{
    WIN_MODE tmpMode = winMode;

    // Close all modes.
    closeCurrentWindowMode();

    // Create the settings dialog.
    QMSettingsDialog settingsDialog(this);
    settingsDialog.exec();

    // If no database has been loaded, don't update anything.
    QSqlDatabase db = QSqlDatabase::database("default", false);

    if (!db.isOpen())
    {
        return;
    }

    // Reset the win mode.
    enterWindowMode(tmpMode);
}

void QMMainWindow::addCertificate()
{
    // exit if no permission for current user
    auto amsManager = QMAMSManager::getInstance();
    if (!amsManager->checkPermission(AccessMode::TD_MODE_WRITE) &&
        !amsManager->checkPermission(AccessMode::TD_MODE_READ))
    {
        QMessageBox::warning(this, tr("Nachweise verwalten"), tr("Sie haben nicht die notwendigen Berechtigungen."));
        return;
    }

    // proceed only when database is available and connected
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        QMessageBox::warning(this, tr("Nachweise verwalten"), tr("Datenbank nicht verbunden."));
        return;
    }

    auto db = QSqlDatabase::database("default");

    m_newCertificateDialog = std::make_unique<QMNewCertificateDialog>(db, this);
    connect(m_newCertificateDialog.get(), &QMNewCertificateDialog::finished, this,
            &QMMainWindow::addCertificateFinished);

    m_newCertificateDialog->loadSettings();
    m_newCertificateDialog->open();
}

void QMMainWindow::addCertificateFinished(int)
{
    m_newCertificateDialog->saveSettings();
    m_newCertificateDialog.reset();
}

void QMMainWindow::manageCertificate()
{
    // If do not have the permission, make a lot of stuff disabled.
    auto amsManager = QMAMSManager::getInstance();
    if (!amsManager->checkPermission(AccessMode::TD_MODE_WRITE) &&
        !amsManager->checkPermission(AccessMode::TD_MODE_READ))
    {
        QMessageBox::warning(this, tr("Nachweise verwalten"), tr("Sie haben nicht die notwendigen Berechtigungen."));
        return;
    }

    auto &settings = QMApplicationSettings::getInstance();

    auto width = settings.read("CertificateDialog/Width", 400).toInt();
    auto height = settings.read("CertificateDialog/Height", 400).toInt();

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
    m_signingListDialog->updateData();
    m_signingListDialog->open();
}

void QMMainWindow::enterResultMode()
{
    enterWindowMode(WIN_MODE::RESULT);
}

void QMMainWindow::enterQualiMatrixMode()
{
    enterWindowMode(WIN_MODE::MATRIX);
}

void QMMainWindow::enterTrainingDataMode()
{
    enterWindowMode(WIN_MODE::TRAININGDATA);
}

bool QMMainWindow::closeCurrentWindowMode()
{
    if (winMode == WIN_MODE::NONE)
    {
        return true;
    }

    QWidget *widget = nullptr;

    switch (winMode)
    {
        case WIN_MODE::TRAININGDATA:
        {
            widget = trainDataWidget.release();
        }
        break;
        case WIN_MODE::MATRIX:
        {
            widget = qualiMatrixWidget.release();
        }
        break;
        case WIN_MODE::RESULT:
        {
            widget = qualiResultWidget.release();
        }
        break;
        default:
            qWarning("Unknown window mode");
            break;
    }

    if (widget != nullptr)
    {
        ui->laInfo->setVisible(false);
        ui->centralwidget->layout()->removeWidget(widget);
        widget->disconnect();
        delete widget;

        lastWinMode = winMode;
        winMode = WIN_MODE::NONE;

        // Check right buttons.
        ui->actModeResult->setChecked(false);
        ui->actModeQualiMatrix->setChecked(false);
        ui->actModeTrainingData->setChecked(false);

        return true;
    }

    return false;
}

void QMMainWindow::enterWindowMode(WIN_MODE mode)
{
    // Close the information label.
    ui->laInfo->setVisible(false);

    // Close the current window mode.
    if (winMode != WIN_MODE::NONE)
    {
        closeCurrentWindowMode();
    }

    // If no database is connected, don't switch to mode.
    if (!QSqlDatabase::contains("default") || !QSqlDatabase::database("default", false).isOpen())
    {
        QMessageBox::information(this, tr("Achtung"), tr("Datenbank is nicht geöffnet. Aktion wird abgebrochen."));

        // Check right buttons.
        ui->actModeResult->setChecked(false);
        ui->actModeQualiMatrix->setChecked(false);
        ui->actModeTrainingData->setChecked(false);

        return;
    }

    auto *layout = dynamic_cast<QVBoxLayout *>(ui->centralwidget->layout());
    if (layout == nullptr)
    {
        qWarning("Could not cast layout to QVBoxLayout");
        return;
    }

    switch (mode)
    {
        case WIN_MODE::TRAININGDATA:
        {
            // Permission check.
            auto amsManager = QMAMSManager::getInstance();
            if (!amsManager->checkPermission(AccessMode::TD_MODE_READ) &&
                !amsManager->checkPermission(AccessMode::TD_MODE_WRITE))
            {
                QMessageBox::warning(this, tr("Schulungsdaten"),
                        tr("Sie haben nicht die notwendigen Berechtigungen."));
                ui->actModeTrainingData->setChecked(false);
                return;
            }

            trainDataWidget = std::make_unique<QMTrainDataWidget>();
            layout->insertWidget(0, trainDataWidget.get());

            connect(trainDataWidget.get(), &QMTrainDataWidget::infoMessageAvailable, ui->laInfo,
                    &QMInfoLabel::showInfoMessage);

            connect(trainDataWidget.get(), &QMTrainDataWidget::warnMessageAvailable, ui->laInfo,
                    &QMInfoLabel::showWarnMessage);

            trainDataWidget->updateData();

            // Check right buttons.
            ui->actModeResult->setChecked(false);
            ui->actModeQualiMatrix->setChecked(false);
            ui->actModeTrainingData->setChecked(true);

            lastWinMode = winMode;
            winMode = WIN_MODE::TRAININGDATA;
        }
        break;
        case WIN_MODE::MATRIX:
        {
            // Permission check.
            auto amsManager = QMAMSManager::getInstance();
            if (!amsManager->checkPermission(AccessMode::QM_MODE_READ) &&
                !amsManager->checkPermission(AccessMode::QM_MODE_WRITE))
            {
                QMessageBox::warning(this, tr("Qualifizierungsmatrix"),
                        tr("Sie haben nicht die notwendigen Berechtigungen."));
                ui->actModeQualiMatrix->setChecked(false);
                return;
            }

            qualiMatrixWidget = std::make_unique<QMQualiMatrixWidget>();
            layout->insertWidget(0, qualiMatrixWidget.get());

            connect(qualiMatrixWidget.get(), &QMWinModeWidget::startWorkload, this, &QMMainWindow::workloadStarts);
            connect(qualiMatrixWidget.get(), &QMWinModeWidget::updateWorkload, this, &QMMainWindow::workloadUpdates);
            connect(qualiMatrixWidget.get(), &QMWinModeWidget::endWorkload, this, &QMMainWindow::closeProgress);

            qualiMatrixWidget->updateData();

            // Check right buttons.
            ui->actModeResult->setChecked(false);
            ui->actModeQualiMatrix->setChecked(true);
            ui->actModeTrainingData->setChecked(false);

            lastWinMode = winMode;
            winMode = WIN_MODE::MATRIX;
        }
        break;
        case WIN_MODE::RESULT:
        {
            // Permission check.
            auto amsManager = QMAMSManager::getInstance();
            if (!amsManager->checkPermission(AccessMode::QR_MODE_READ))
            {
                QMessageBox::warning(this, tr("Qualifizierungsresultat"),
                        tr("Sie haben nicht die notwendigen Berechtigungen."));
                ui->actModeResult->setChecked(false);
                return;
            }

            qualiResultWidget = std::make_unique<QMQualiResultWidget>();
            layout->insertWidget(0, qualiResultWidget.get());

            connect(qualiResultWidget.get(), &QMWinModeWidget::startWorkload, this, &QMMainWindow::workloadStarts);
            connect(qualiResultWidget.get(), &QMWinModeWidget::updateWorkload, this, &QMMainWindow::workloadUpdates);
            connect(qualiResultWidget.get(), &QMWinModeWidget::endWorkload, this, &QMMainWindow::closeProgress);
//            connect(qualiResultWidget.get(), &QMQualiResultWidget::showTrainData, this,
//                    &QMMainWindow::showTrainingData);

            qualiResultWidget->updateData();

            // Check right buttons.
            ui->actModeResult->setChecked(true);
            ui->actModeQualiMatrix->setChecked(false);
            ui->actModeTrainingData->setChecked(false);

            lastWinMode = winMode;
            winMode = WIN_MODE::RESULT;
        }
        break;
        default:
            qWarning("Unknown window mode");
            break;
    }
}

void QMMainWindow::handleLoginChange(LoginState, LoginState current)
{
    auto *tbAMS = dynamic_cast<QToolButton *>(ui->tbaUser->widgetForAction(ui->actAMS));

    if (current == LoginState::LOGGED_IN)
    {
        auto am = QMAMSManager::getInstance();

        tbAMS->setText(am->getLoginUserName());
        tbAMS->setIcon(QIcon(":/icons/line-awesome/svg/icons/line-awesome/user-check-solid.svg"));
    }
    else
    {
        tbAMS->setIcon(QIcon(":/icons/line-awesome/svg/icons/line-awesome/user-times-solid.svg"));
        tbAMS->setText("");
    }
}

void QMMainWindow::amsLogin()
{
    // Close all modes.
    closeCurrentWindowMode();

    auto am = QMAMSManager::getInstance();

    if (am->getLoginState() == LoginState::LOGGED_IN)
    {
        QMessageBox::StandardButton res = QMessageBox::question(this, tr("Anmelden"),
                tr("Ein Nutzer ist bereits angemeldet. Möchten Sie den Nutzer abmelden?"),
                QMessageBox::Yes | QMessageBox::No);

        if (res != QMessageBox::Yes)
        {
            return;
        }

        amsLogout();
        if (am->getLoginState() == LoginState::LOGGED_IN)
        {
            return;
        }
    }

    auto &settings = QMApplicationSettings::getInstance();
    auto saveLastLogin = settings.read("AMS/SaveLastLogin", false).toBool();
    QString lastLoginName = "";

    if (saveLastLogin)
    {
        lastLoginName = settings.read("AMS/LastLoginName", "").toString();
    }

    if (!lastLoginName.isEmpty())
    {
        m_amsLoginDialog->setUsername(lastLoginName);
    }

    m_amsLoginDialog->open();
}

void QMMainWindow::amsLoginDialogFinished(int)
{
    // Reset the win mode only when not NONE (cause it already is NONE).
    if (lastWinMode != WIN_MODE::NONE)
    {
        enterWindowMode(lastWinMode);
    }
}

void QMMainWindow::amsLogout()
{
    WIN_MODE tmpMode = winMode;

    // Close all modes.
    closeCurrentWindowMode();

    auto am = QMAMSManager::getInstance();
    am->logoutUser();

    // Reset the win mode only when not NONE (cause it already is NONE).
    if (tmpMode != WIN_MODE::NONE)
    {
        enterWindowMode(tmpMode);
    }
}
