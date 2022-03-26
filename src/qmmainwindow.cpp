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
#include "model/qmdatamanager.h"
#include "qualimatrix/qmqualimatrixwidget.h"
#include "qualiresult/qmqualiresultwidget.h"
#include "settings/qmapplicationsettings.h"
#include "traindata/qmtraindatawidget.h"
#include "database/qmdatabasedialog.h"
#include "database/qmdatabaseupdatedialog.h"
#include "database/qmdatabaseupdater.h"
#include "framework/component/qminfolabel.h"
#include "certificate/qmcertificatedialog.h"
#include "certificate/qmcertificateintegritycheckdialog.h"
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
#include <QDateTime>
#include <QSqlQuery>
#include <QSqlError>
#include <QTranslator>
#include <QWindow>
#include <QStatusBar>
#include <QToolButton>

QMMainWindow::QMMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , winMode(WIN_MODE::NONE)
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

void QMMainWindow::manageDatabase()
{
    // Before the dialog for managing a database will be shown, close a currently loaded one.
    if (QSqlDatabase::contains("default") && QSqlDatabase::database("default", false).isOpen())
    {
        auto res = QMessageBox::question(this, tr("Datenbank verwalten"),
                tr("Es besteht bereits eine Verbindung zu einer Datenbank. Jetzt trennen?"),
                QMessageBox::Yes | QMessageBox::No);

        if (res == QMessageBox::No)
        {
            return;
        }

        closeDatabase();
    }

    // The manage dialog will manipulate the data in QSqlDatabase. This is the buffer for the settings which will be
    // saved in application settings afterwards.
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

void QMMainWindow::workloadStarts(QString info, int maxSteps)
{
    showProgress(tr("Berechnung"), info, 0, maxSteps);
}

void QMMainWindow::showProgress(const QString &title, const QString &text, const int &minSteps, const int &maxSteps)
{
    // The progress dialog object will be recreated on every use, cause it would show up by itself when ui does not
    // react for view seconds (qt related).

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

void QMMainWindow::workloadUpdates(int currentStep)
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

[[maybe_unused]] void QMMainWindow::showAboutQt()
{
    QMessageBox::aboutQt(this);
}

[[maybe_unused]] void QMMainWindow::showAbout()
{
    QMessageBox::about(this, tr("Über QualificationMatrix"),
            tr("Copyright (c) 2020 by Christian Kr"
                "\nVersion:\t" VERSION_MAJOR "." VERSION_MINOR " " RELEASE_STATE
                " // " BUILD
                "\nLizenz:\tGNU GENERAL PUBLIC LICENSE Version 3"
                "\n\nEine Kopie der Lizenz wird mit dem Quellcode der Software "
                "mitgeliefert"
                " (COPYING.txt)"
                "\n\nhttps://github.com/Christian-Kr/QualificationMatrix"
                "\n\nBug-Reports: Fehler können direkt auf GitHub gemeldet werden"
                " oder per E-Mail an CerebrosuS_aedd_gmx.net"));
}

bool QMMainWindow::closeDatabase()
{
    // If a database with a driver exist, close it.
    if (QSqlDatabase::contains("default"))
    {
        auto db = QSqlDatabase::database("default", false);

        if (db.isOpen())
        {
            auto res = QMessageBox::question(this, tr("Datenbank schließen"),
                    tr("Soll die verbundene Datenbank wirklich geschlossen werden? (Nicht gespeicherte Daten gehen "
                       "verloren!)"),
                QMessageBox::Yes | QMessageBox::No);

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
    closeCurrentWindowMode();
    ui->actSettings->setEnabled(false);
    ui->actCloseDatabase->setEnabled(false);

    // Logout from database.
    amsLogout();

    // Set ui elements.
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

[[maybe_unused]] void QMMainWindow::showSettings()
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

void QMMainWindow::showTrainingData(QString, QString)
{
    // TODO: This is not possible anymore. Think about different ways.
//    ui->twQualiMatrix->setCurrentIndex(2);
//
//    trainDataWidget->resetFilter();
//    trainDataWidget->setNameFilter(name);
//    trainDataWidget->setTrainFilter(training);
//    trainDataWidget->updateFilter();
}

[[maybe_unused]] void QMMainWindow::manageCertificate()
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

[[maybe_unused]] void QMMainWindow::showCertificateIntegrityCheck()
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

[[maybe_unused]] void QMMainWindow::showCreateSigningList()
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

[[maybe_unused]] void QMMainWindow::enterResultMode()
{
    enterWindowMode(WIN_MODE::RESULT);
}

[[maybe_unused]] void QMMainWindow::enterQualiMatrixMode()
{
    enterWindowMode(WIN_MODE::MATRIX);
}

[[maybe_unused]] void QMMainWindow::enterTrainingDataMode()
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
            connect(qualiResultWidget.get(), &QMQualiResultWidget::showTrainData, this,
                    &QMMainWindow::showTrainingData);

            qualiResultWidget->updateData();

            // Check right buttons.
            ui->actModeResult->setChecked(true);
            ui->actModeQualiMatrix->setChecked(false);
            ui->actModeTrainingData->setChecked(false);

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
        tbAMS->setIcon(QIcon(":/icons/icons/im-user-online.svg"));
    }
    else
    {
        tbAMS->setIcon(QIcon(":/icons/icons/im-user-offline.svg"));
        tbAMS->setText("");
    }
}

void QMMainWindow::amsLogin()
{
    WIN_MODE tmpMode = winMode;

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

    QMAMSLoginDialog loginDialog(this);

    if (!lastLoginName.isEmpty())
    {
        loginDialog.setUsername(lastLoginName);
    }

    loginDialog.exec();

    // Reset the win mode only when not NONE (cause it already is NONE).
    if (tmpMode != WIN_MODE::NONE)
    {
        enterWindowMode(tmpMode);
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
