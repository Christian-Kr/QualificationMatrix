// main.cpp is part of QualificationMatrix
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

#include "general/qmmainwindow.h"
#include "settings/qmapplicationsettings.h"
#include "config.h"

#include <QApplication>
#include <QWindow>
#include <QScreen>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDir>
#include <QtGlobal>

#include <QDebug>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
#define DEPR_ENDL Qt::endl
#else
#define DEPR_ENDL endl
#endif

/// Initialize the main window state with saved data.
/// \param mainWin The main window object to initialize.
void initShowMainWindow(QMainWindow &mainWin)
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
        mainWin.windowHandle()->setScreen(screen);

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
        auto savedWidth = settings.read("MainWin/Width", 500).toInt();
        auto savedHeight = settings.read("MainWin/Height", 500).toInt();

        mainWin.resize(savedWidth, savedHeight);
        mainWin.move(screen->geometry().x() + (screen->geometry().width() - savedWidth) / 2,
            screen->geometry().y() + (screen->geometry().height() - savedHeight) / 2);

        mainWin.show();
    }
}

/// Set the style of the application, which is "Fusion" by default.
void initApplicationStyle()
{
    auto &settings = QMApplicationSettings::getInstance();
    auto style = settings.read("General/Style", "Fusion").toString();

    QApplication::setStyle(style);
}

/// Install the translator into the qt application. If the loading or installing of the
/// translation certificate fails, a warning message will be send for documentation.
/// \param name The name of the translator certificate.
/// \param path The path, where the tranaltor certificate could be found.
void installTranslator(const QString &name, const QString &path)
{
    auto translator = new QTranslator();
    auto loaded = translator->load(name, path);

    if (loaded)
    {
        // Info: Installing certificate fails, if there is no translation in ts certificate itself.
        auto installed = QApplication::installTranslator(translator);

        if (!installed)
        {
            qWarning() << "cannot install language certificate" << name << "in" << path;
        }
    }
    else
    {
        qWarning() << "cannot load custom language certificate" << name << "in" << path;
    }
}

/// Initialize the application with the translation files (custom and qt-based).
void initApplicationTranslation()
{
    auto &settings = QMApplicationSettings::getInstance();
    auto lang = settings.read("General/Lang", "de").toString();

    installTranslator("qm_" + lang + ".qm", QDir::currentPath());

    // Qt and qtbase translation files come from installed qt version or they need to be deployed
    // with the application and its libraries.
    installTranslator("qt_" + lang, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    installTranslator("qtbase_" + lang, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
}

/// Take all messages coming from qWarning, qDebug etc. and write them into a log certificate. In
/// general, all message will be written into a log certificate. If this log certificate is not
/// writeable, the message will be printed on stdout. Debug messages will always only be printed on
/// stdout.
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

    QFile out("log.txt");
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

/// Well, ... main entry function.
/// \param argc Number of arguments, including the name of the application.
/// \param argv List of arguments, including the name of the application.
/// \return Zero if exit success, else -1.
int main(int argc, char *argv[])
{
    // General information, used by qt - example: QSettings certificate naming
    QCoreApplication::setOrganizationName("Kr");
    QCoreApplication::setOrganizationDomain("Kr");
    QCoreApplication::setApplicationVersion(QString("%1.%2").arg(VERSION_MAJOR, VERSION_MINOR));

    if (QString(RELEASE_STATE).compare("beta") == 0)
    {
        QCoreApplication::setApplicationName(QString("QualificationMatrix_%1").arg(RELEASE_STATE));
    }
    else
    {
        QCoreApplication::setApplicationName(QString("QualificationMatrix"));
    }

    // Create application and main window object.
    QApplication a(argc, argv);
    qInstallMessageHandler(customMessageHandler);
    initApplicationTranslation();
    initApplicationStyle();

    QMMainWindow w;
    initShowMainWindow(w);

    return QApplication::exec();
}
