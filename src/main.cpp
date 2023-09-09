// main.cpp is part of QualificationMatrix
//
// QualificationMatrix is free software: you can redistribute it and/or modify it under the terms
// of the GNU General Public License as published by the Free Software Foundation, either version
// 3 of the License, or (at your option) any later version.
//
// QualificationMatrix is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
// the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with QualificationMatrix.
// If not, see <http://www.gnu.org/licenses/>.

#include "qmmainwindow.h"
#include "settings/qmapplicationsettings.h"
#include "appcache/qmappcachemanager.h"
#include "config.h"

#include <QApplication>
#include <QWindow>
#include <QScreen>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDir>
#include <QtGlobal>
#include <QMessageBox>
#include <QFileDialog>
#include <QAbstractButton>

// Since Qt 5.15.0 the endl variable has been changed to Qt::endl
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
#define DEPR_ENDL Qt::endl
#else
#define DEPR_ENDL endl
#endif

// Return codes for the first start process. If any changes have been done that require a restart,
// return the *_RESTART return code.
const unsigned short FIRST_START_RET_RESTART = 1;
const unsigned short FIRST_START_RET_CODE_GOON = 2;

/// Initialize the centralized configuration.
///
/// \return the return code to know if a restart is needed before the main qt application has been
/// run
unsigned short initFirstStartProgress()
{
    auto &settings = QMApplicationSettings::getInstance();
    auto firstStart = settings.read("General/FirstStart", true).toBool();

    // if application has not been started for the first time, just go on with starting the
    // application
    if (!firstStart)
    {
        return FIRST_START_RET_CODE_GOON;
    }

    // ask whether the user wants to load a template config file to make configuration faster
    QMessageBox messageBox(nullptr);

    messageBox.setStandardButtons(
            QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No);
    messageBox.setWindowTitle(QObject::tr("Erster Start"));
    messageBox.setIcon(QMessageBox::Icon::Question);
    messageBox.setText(QObject::tr("Die Anwendung wird scheinbar das erste Mal gestartet."));
    messageBox.setInformativeText(
            QObject::tr("Möchtest du eine zentrale Konfigurationsdatei laden?\n"));
    messageBox.setDefaultButton(QMessageBox::StandardButton::Yes);

    auto *buttonYes = messageBox.button(QMessageBox::StandardButton::Yes);
    Q_ASSERT(buttonYes != nullptr);
    buttonYes->setText(QObject::tr("Konfiguration laden"));

    auto *buttonNo = messageBox.button(QMessageBox::StandardButton::No);
    Q_ASSERT(buttonNo != nullptr);
    buttonNo->setText(QObject::tr("Ohne fortfahren"));

    if (messageBox.exec() == QMessageBox::Yes)
    {
        // open the configuration file
        QFileDialog fileDialog(nullptr);

        fileDialog.setWindowTitle(QObject::tr("Zentrale Konfigurationsdatei öffnen"));
        fileDialog.setFileMode(QFileDialog::FileMode::ExistingFile);
        fileDialog.setAcceptMode(QFileDialog::AcceptMode::AcceptOpen);
        fileDialog.setNameFilter(QObject::tr("Zentrale Konfiguration (*.ini)"));

        // if user rejects the dialog, make the first start dialog appear on next start
        if (fileDialog.exec() == QFileDialog::Rejected)
        {
            return FIRST_START_RET_CODE_GOON;
        }

        auto fileNames = fileDialog.selectedFiles();

        Q_ASSERT(fileNames.count() <= 1);

        // if no file has been selected, make the first start dialog appear on next start
        if (fileNames.count() == 0)
        {
            return FIRST_START_RET_CODE_GOON;
        }

        // if the selected object is not a file (just in case), make the first start dialog appear
        // on next start
        QFileInfo configFileInfo(fileNames.first());
        if (!configFileInfo.isFile())
        {
            return FIRST_START_RET_CODE_GOON;
        }

        // tests were ok, set settings file and restart application
        settings.write("General/CentralizedPath", fileNames.first());
        settings.write("General/Centralized", true);

        // inform user, that the application will be restartet
        QMessageBox::information(nullptr, QObject::tr("Neustart"),
                QObject::tr("Die Anwendung muss neugestartet werden um die Einstellungen zu "
                            "übernehmen."));

        // safe variable, that first start has been done
        settings.write("General/FirstStart", false);

        return FIRST_START_RET_RESTART;
    }

    // safe variable, that first start has been done
    settings.write("General/FirstStart", false);

    return FIRST_START_RET_CODE_GOON;
}

/// Initialize the main window state. This includes the dimension and position of the window.
///
/// \param mainWin The main window object to initialize.
void initShowMainWindow(QMMainWindow &mainWin)
{
    auto &settings = QMApplicationSettings::getInstance();
    auto showMaximized = settings.read("MainWin/Maximized", false).toBool();

    // Load the active screen from settings and put the window on this screen. Detection of the
    // right screen is done by the screen name.
    QScreen *screen;
    {
        auto screens = QGuiApplication::screens();
        auto activeScreenNumber = 0;

        if (screens.size() > 1)
        {
            auto screenName = settings.read("MainWin/Screen", "").toString();

            for (int i = 0; i < screens.size(); i++)
            {
                if (screens.at(i)->name() == screenName)
                {
                    activeScreenNumber = i;
                    break;
                }
            }
        }

        screen = QGuiApplication::screens().at(activeScreenNumber);

        if (mainWin.windowHandle() != nullptr)
        {
            mainWin.windowHandle()->setScreen(screen);
        }

        // Calling only setScreen won't put the window onto the right screen.
        mainWin.setGeometry(screen->geometry());
    }

    // Set the size and position of the window from settings.
    if (showMaximized)
    {
        mainWin.showMaximized();
    }
    else
    {
        auto savedWidth = settings.read("MainWin/Width", 1000).toInt();
        auto savedHeight = settings.read("MainWin/Height", 800).toInt();

        mainWin.resize(savedWidth, savedHeight);
        mainWin.move(screen->geometry().x() + (screen->geometry().width() - savedWidth) / 2,
                screen->geometry().y() + (screen->geometry().height() - savedHeight) / 2);

        mainWin.show();
    }

    // Load database on startup. If there are some settings for automatic loading of database on
    // startup, follow them. Otherwise show the manage database dialog, cause the application is
    // not useful if there is no database loaded.
    mainWin.initDatabaseSettings();

    // Try to bring window to top.
    mainWin.activateWindow();
    mainWin.raise();
}

/// Set the style of the application, which is "Fusion" by default.
void initApplicationStyle()
{
    auto &settings = QMApplicationSettings::getInstance();
    auto style = settings.read("General/Style", "Fusion").toString();

    QApplication::setStyle(style);
}

/// Set the style sheet of the application, which is "default.css" by default. For now only the
/// default stylesheet will be loaded.
///
/// \param app The application object to apply style sheet.
void initApplicationStyleSheet(QApplication &app)
{
    QFile file("styles/default.css");
    file.open(QFile::ReadOnly);

    auto styleSheet = QLatin1String(file.readAll());

    app.setStyleSheet(styleSheet);
}

/// Install the translator into the qt application. If the loading or installing of the translation
/// file fails, a warning message will be send for documentation.
///
/// \param name The name of the translator file.
/// \param path The path, where the tranaltor file could be found.
void installTranslator(const QString &name, const QString &path)
{
    auto translator = new QTranslator();
    auto loaded = translator->load(name, path);

    if (loaded)
    {
        // Info: Installing file fails, if there is no translation in ts file itself.
        auto installed = QApplication::installTranslator(translator);

        if (!installed)
        {
            qWarning() << "cannot install language file" << translator->filePath();
        }
    }
    else
    {
        qWarning() << "cannot load custom language file" << translator->filePath();
    }
}

/// Set the dpi settings read from the settings file at startup.
void readGuiDPISetting()
{
    auto &settings = QMApplicationSettings::getInstance();
    auto dpi = settings.read("General/DPI", 1).toInt();

    if (dpi < 1 || dpi > 5)
    {
        qWarning() << "Unknown DPI setting - Default value of 1 will be choosen";
        dpi = 1;
    }

    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
            static_cast<Qt::HighDpiScaleFactorRoundingPolicy>(dpi));
}

/// Initialize the application with the translation files (custom and qt-based).
void initApplicationTranslation()
{
    auto &settings = QMApplicationSettings::getInstance();
    auto lang = settings.read("General/Lang", "de").toString();

    installTranslator("qm_" + lang + ".qm", QDir::currentPath());

    // Qt and qtbase translation files come from installed qt version or they
    // need to be deployed with the application and its libraries.
    installTranslator("qt_" + lang, QLibraryInfo::path(QLibraryInfo::TranslationsPath));
    installTranslator("qtbase_" + lang, QLibraryInfo::path(QLibraryInfo::TranslationsPath));
}

/// Take all messages coming from qWarning, qDebug etc. and write them into a log file. In general,
/// all message will be written into a log file. If this log file is not writeable, the message
/// will be printed on stdout. Debug messages will always only be printed on stdout. The log file
/// will be create in user directory, to be readable and individual by every user.
///
/// \param type The message type (debug, warning etc.).
/// \param msg The message to display/log.
void customMessageHandler(QtMsgType type, const QMessageLogContext &, const QString &msg)
{
    auto txt = QDateTime::currentDateTime().toString();

    switch (type)
    {
        case QtDebugMsg:
        {
            txt += QString(" Debug: %1").arg(msg);

            // Debug messages will also be written to the terminal window.
            QTextStream ts(stdout);
            ts << txt << DEPR_ENDL;

            return;
        }
        case QtWarningMsg:
            txt += QString(" Warning: %1").arg(msg);
            break;
        case QtCriticalMsg:
            txt += QString(" Critical: %1").arg(msg);
            break;
        case QtFatalMsg:
            txt += QString(" Fatal: %1").arg(msg);
            break;
        default:
            return;
    }

    QFile out(QDir::homePath() + QDir::separator() + "qm_log.txt");
    auto opened = out.open(QIODevice::WriteOnly | QIODevice::Append);

    if (opened)
    {
        QTextStream ts(&out);
        ts << txt << DEPR_ENDL;
    }
    else
    {
        QTextStream ts(stdout);
        ts << txt << DEPR_ENDL;
    }

    out.close();
}

/// Initialize the application if wanted.
void initApplicationCache()
{
    auto &settings = QMApplicationSettings::getInstance();

    auto appCacheActive = settings.read("AppCache/Active", false).toBool();
    if (!appCacheActive)
    {
        return;
    }

    auto acm = QMAppCacheManager::getInstance();
    if (!acm->trigger())
    {
        return;
    }
}

/// Well, ... main entry function.
///
/// \param argc Number of arguments, including the name of the application.
/// \param argv List of arguments, including the name of the application.
/// \return Zero if exit success, else -1.
int main(int argc, char *argv[])
{
    // general information, used by qt - example: QSettings certificate naming
    QCoreApplication::setOrganizationName("Kr");
    QCoreApplication::setOrganizationDomain("Kr");
    QCoreApplication::setApplicationVersion(QString("%1.%2").arg(VERSION_MAJOR, VERSION_MINOR));

    if (QString(RELEASE_STATE).compare("beta") == 0) {
        QCoreApplication::setApplicationName(QString("QualificationMatrix_%1").arg(RELEASE_STATE));
    } else {
        QCoreApplication::setApplicationName(QString("QualificationMatrix"));
    }

    // the application loop is for restarting the application before any qt application loop is
    // running
    while (true)
    {
        // create application and main window object
        QApplication app(argc, argv);
        qInstallMessageHandler(customMessageHandler);

        initApplicationTranslation();
        initApplicationStyle();
        initApplicationStyleSheet(app);

        // if any first start progress needs a restart as follow up, do so
        if (initFirstStartProgress() == FIRST_START_RET_RESTART) {
            QMApplicationSettings::getInstance().clear();
            continue;
        }

        // run application cache
        initApplicationCache();

        // set up gui dpi settings
        readGuiDPISetting();

        QMMainWindow win;
        initShowMainWindow(win);

        // run main application loop
        return QApplication::exec();
    }
}
