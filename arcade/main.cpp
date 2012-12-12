#include <QApplication>
#include <QTranslator>
#include <QIcon>

#include "arcadesettings.h"
#include "tweakedqmlappviewer.h"
#include "consolewindow.h"
#include "macros.h"

ArcadeSettings *globalConfig = NULL;
ConsoleWindow *consoleWindow = NULL;
int emulatorMode = QMC2_ARCADE_EMUMODE_MAME;
int consoleMode = QMC2_ARCADE_CONSOLE_TERM;
QStringList emulatorModeNames;
QStringList arcadeThemes;
QStringList mameThemes;
QStringList messThemes;
QStringList umeThemes;
bool runApp = true;

void qtMessageHandler(QtMsgType type, const char *msg)
{
    if ( !runApp )
        return;

    QString msgString;

    switch ( type ) {
    case QtDebugMsg:
        msgString = "QtDebugMsg: " + QString(msg);
        break;
    case QtWarningMsg:
        msgString = "QtWarningMsg: " + QString(msg);
        break;
    case QtCriticalMsg:
        msgString = "QtCriticalMsg: " + QString(msg);
        break;
    case QtFatalMsg:
        msgString = "QtFatalMsg: " + QString(msg);
        break;
    default:
        return;
    }

    QMC2_ARCADE_LOG_STR(msgString);
}

void showHelp()
{
#if defined(QMC2_ARCADE_OS_WIN)
    // we need the console window to display the help text on Windows because we have no terminal connection
    if ( !consoleWindow ) {
        consoleWindow = new ConsoleWindow(0);
        consoleWindow->show();
    }
#endif
    QString helpMessage = QString("Usage: qmc2-arcade [-emu <emulator>] [-theme <theme>] [-console <type>] [-graphicssystem <engine>] [-config_path <path>] [-h|-?|-help]\n\n"
                                  "Option           Meaning             Possible values ([..] = default)\n"
                                  "---------------  ------------------  ------------------------------------\n"
                                  "-emu             Emulator mode       [mame], mess, ume\n"
                                  "-theme           Theme selection     [ToxicWaste]\n"
                                  "-console         Console type        [terminal], window, window-minimized\n"
                                  "-graphicssystem  Graphics engine     [raster], native, opengl\n"
                                  "-config_path     Configuration path  [%1], ...\n").arg(QMC2_ARCADE_DOT_PATH);
    QMC2_ARCADE_LOG_STR_NT(helpMessage);
}

#if defined(QMC2_ARCADE_OS_WIN)
#idef main
#undef main
#endif
#endif

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    qsrand(QDateTime::currentDateTime().toTime_t());
    qInstallMsgHandler(qtMessageHandler);

    QScopedPointer<QApplication> app(createApplication(argc, argv));

    // avalibale emulator modes & themes
    emulatorModeNames << QObject::tr("MAME") << QObject::tr("MESS") << QObject::tr("UME");
    arcadeThemes << "ToxicWaste";
    mameThemes << "ToxicWaste";
    // messThemes << "..."
    umeThemes << "ToxicWaste";

    QString console = QMC2_ARCADE_CLI_CONS;

    if ( console == "window" || console == "window-minimized" ) {
        consoleMode = console == "window" ? QMC2_ARCADE_CONSOLE_WIN : QMC2_ARCADE_CONSOLE_WINMIN;
        consoleWindow = new ConsoleWindow(0);
        if ( consoleMode == QMC2_ARCADE_CONSOLE_WINMIN )
            consoleWindow->showMinimized();
        else
            consoleWindow->show();
    }

    // process command line arguments
    if ( QMC2_ARCADE_CLI_HELP || QMC2_ARCADE_CLI_INVALID ) {
        showHelp();
        if ( !consoleWindow )
            return 1;
        else
            runApp = false;
    }

    QString theme = QMC2_ARCADE_CLI_THEME;

    if ( !arcadeThemes.contains(theme) && runApp ) {
        QMC2_ARCADE_LOG_STR_NT(QObject::tr("%1 is not valid theme - available themes: %2").arg(theme).arg(arcadeThemes.join(", ")));
        if ( !consoleWindow )
            return 1;
        else
            runApp = false;
    }

    if ( !QMC2_ARCADE_CLI_EMU_UNK && runApp ) {
        emulatorMode = QMC2_ARCADE_CLI_EMU_MAME ? QMC2_ARCADE_EMUMODE_MAME : QMC2_ARCADE_CLI_EMU_MESS ? QMC2_ARCADE_EMUMODE_MESS : QMC2_ARCADE_CLI_EMU_UME ? QMC2_ARCADE_EMUMODE_UME : QMC2_ARCADE_EMUMODE_UNK;
        if ( emulatorMode == QMC2_ARCADE_EMUMODE_UNK ) {
            showHelp();
            if ( !consoleWindow )
                return 1;
            else
                runApp = false;
        }
    }

    switch ( emulatorMode ) {
    case QMC2_ARCADE_EMUMODE_MAME:
        if ( !mameThemes.contains(theme) && runApp ) {
            QMC2_ARCADE_LOG_STR_NT(QObject::tr("%1 is not a valid %2 theme - available %2 themes: %3").arg(theme).arg(emulatorModeNames[QMC2_ARCADE_EMUMODE_MAME]).arg(mameThemes.isEmpty() ? QObject::tr("(none)") : mameThemes.join(", ")));
            if ( !consoleWindow )
                return 1;
            else
                runApp = false;
        }
        break;
    case QMC2_ARCADE_EMUMODE_MESS:
        if ( !messThemes.contains(theme) && runApp ) {
            QMC2_ARCADE_LOG_STR_NT(QObject::tr("%1 is not a valid %2 theme - available %2 themes: %3").arg(theme).arg(emulatorModeNames[QMC2_ARCADE_EMUMODE_MESS]).arg(messThemes.isEmpty() ? QObject::tr("(none)") : messThemes.join(", ")));
            if ( !consoleWindow )
                return 1;
            else
                runApp = false;
        }
        break;
    case QMC2_ARCADE_EMUMODE_UME:
        if ( !umeThemes.contains(theme) && runApp ) {
            QMC2_ARCADE_LOG_STR_NT(QObject::tr("%1 is not a valid %2 theme - available %2 themes: %3").arg(theme).arg(emulatorModeNames[QMC2_ARCADE_EMUMODE_UME]).arg(umeThemes.isEmpty() ? QObject::tr("(none)") : umeThemes.join(", ")));
            if ( !consoleWindow )
                return 1;
            else
                runApp = false;
        }
        break;
    }

    // settings management
    QCoreApplication::setOrganizationName(QMC2_ARCADE_ORG_NAME);
    QCoreApplication::setOrganizationDomain(QMC2_ARCADE_ORG_DOMAIN);
    QCoreApplication::setApplicationName(QMC2_ARCADE_APP_NAME);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, QMC2_ARCADE_DYN_DOT_PATH);
    globalConfig = new ArcadeSettings(theme);
    globalConfig->setApplicationVersion(QMC2_ARCADE_APP_VERSION);

    // load translator
    QString language = globalConfig->language();
    if ( !globalConfig->languageMap.contains(language) )
        language = "us";
    QTranslator qmc2ArcadeTranslator;
    if ( qmc2ArcadeTranslator.load(QString("qmc2-arcade_%1").arg(language), ":/translations") )
        app->installTranslator(&qmc2ArcadeTranslator);


    int returnCode;
    if ( runApp ) {
        // log banner message
        QString bannerMessage = QString("%1 %2 (%3)").
                                arg(QMC2_ARCADE_APP_TITLE).
#if defined(QMC2_ARCADE_SVN_REV)
                                arg(QMC2_ARCADE_APP_VERSION + QString(", SVN r%1").arg(XSTR(QMC2_ARCADE_SVN_REV))).
#else
                                arg(QMC2_ARCADE_APP_VERSION).
#endif
                                arg(QString("Qt ") + qVersion() + ", " +
                                    QObject::tr("emulator: %1").arg(emulatorModeNames[emulatorMode]) + ", " +
                                    QObject::tr("theme: %1").arg(theme));
        QMC2_ARCADE_LOG_STR(bannerMessage);

        if ( consoleWindow )
            consoleWindow->loadSettings();

        // set up the main QML app viewer window
        TweakedQmlApplicationViewer *viewer = new TweakedQmlApplicationViewer();
        viewer->setWindowIcon(QIcon(QLatin1String(":/images/qmc2-arcade.png")));
        viewer->setWindowTitle(QMC2_ARCADE_APP_TITLE + " " + QMC2_ARCADE_APP_VERSION);
        viewer->setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
        QMC2_ARCADE_LOG_STR(QObject::tr("Starting QML viewer using theme '%1'").arg(theme));
        viewer->setSource(QString("qrc:/qml/%1/%1.qml").arg(theme));

        // set up display mode initially...
        if ( globalConfig->fullScreen() )
            viewer->switchToFullScreen(true);
        else
            viewer->switchToWindowed(true);

        // ... and run the application
        returnCode = app->exec();

        delete viewer;
    } else {
        if ( consoleWindow ) {
            consoleWindow->loadSettings();
            QString consoleMessage(QObject::tr("QML viewer not started - please close the console window to exit"));
            QMC2_ARCADE_LOG_STR_NT(QString("-").repeated(consoleMessage.length()));
            QMC2_ARCADE_LOG_STR_NT(consoleMessage);
            QMC2_ARCADE_LOG_STR_NT(QString("-").repeated(consoleMessage.length()));
            consoleWindow->showNormal();
            consoleWindow->raise();
            app->exec();
        }
        returnCode = 1;
    }

    if ( consoleWindow ) {
        consoleWindow->saveSettings();
        delete consoleWindow;
    }

    delete globalConfig;

    return returnCode;
}
