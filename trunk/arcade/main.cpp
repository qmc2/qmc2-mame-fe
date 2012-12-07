#include <QApplication>

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

void qtMessageHandler(QtMsgType type, const char *msg)
{
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
    QMC2_LOG_STR(msgString);
}

void showHelp()
{
    QString helpMessage = QObject::tr("Usage: qmc2-arcade [-emu <emulator>] [-theme <theme>] [-console <console>] [-graphicssystem <engine>] [-h|-?|-help]\n\n"
                                      "Option           Values ([..] = default)\n"
                                      "---------------  ------------------------------------\n"
                                      "-emu             [mame], mess, ume\n"
                                      "-theme           [ToxicWaste]\n"
                                      "-console         [terminal], window, window-minimized\n"
                                      "-graphicssystem  [raster], native, opengl, ...\n");
    QMC2_LOG_STR_NO_TIME(helpMessage);
}

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

    bool runApp = true;

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
        QMC2_LOG_STR_NO_TIME(QObject::tr("%1 is not valid theme - available themes: %2").arg(theme).arg(arcadeThemes.join(", ")));
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
            QMC2_LOG_STR_NO_TIME(QObject::tr("%1 is not a valid %2 theme - available %2 themes: %3").arg(theme).arg(emulatorModeNames[QMC2_ARCADE_EMUMODE_MAME]).arg(mameThemes.isEmpty() ? QObject::tr("(none)") : mameThemes.join(", ")));
            if ( !consoleWindow )
                return 1;
            else
                runApp = false;
        }
        break;
    case QMC2_ARCADE_EMUMODE_MESS:
        if ( !messThemes.contains(theme) && runApp ) {
            QMC2_LOG_STR_NO_TIME(QObject::tr("%1 is not a valid %2 theme - available %2 themes: %3").arg(theme).arg(emulatorModeNames[QMC2_ARCADE_EMUMODE_MESS]).arg(messThemes.isEmpty() ? QObject::tr("(none)") : messThemes.join(", ")));
            if ( !consoleWindow )
                return 1;
            else
                runApp = false;
        }
        break;
    case QMC2_ARCADE_EMUMODE_UME:
        if ( !umeThemes.contains(theme) && runApp ) {
            QMC2_LOG_STR_NO_TIME(QObject::tr("%1 is not a valid %2 theme - available %2 themes: %3").arg(theme).arg(emulatorModeNames[QMC2_ARCADE_EMUMODE_UME]).arg(umeThemes.isEmpty() ? QObject::tr("(none)") : umeThemes.join(", ")));
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

    int returnCode;
    if ( runApp ) {
        // log banner message
        QMC2_LOG_STR(QString(QString("%1 %2 (%3)").
                             arg(QMC2_ARCADE_APP_TITLE).
                             arg(QMC2_ARCADE_APP_VERSION).
                             arg(QString("Qt ") + qVersion() + ", " +
                                 QObject::tr("emulator: %1").arg(emulatorModeNames[emulatorMode]) + ", " +
                                 QObject::tr("theme: %1").arg(theme))));

        if ( consoleWindow )
            consoleWindow->loadSettings();

        // setup the main QML app viewer window
        TweakedQmlApplicationViewer *viewer = new TweakedQmlApplicationViewer();
        viewer->setWindowTitle(QMC2_ARCADE_APP_TITLE + " " + QMC2_ARCADE_APP_VERSION);
        viewer->setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
        QString qmlFile(QString("qml/%1/%1.qml").arg(theme));
        QMC2_LOG_STR(QObject::tr("Starting QML viewer using '%1' as main file").arg(qmlFile));
        viewer->setMainQmlFile(qmlFile.toLocal8Bit());

        // set up display mode initially...
        if ( globalConfig->fullScreen() )
            viewer->switchToFullScreen(true);
        else
            viewer->switchToWindowed(true);

        // ... and run the application
        returnCode = app->exec();

        // clean up
        delete viewer;
    } else {
        if ( consoleWindow ) {
            consoleWindow->loadSettings();
            QString consoleMessage(QObject::tr("Couldn't start QML viewer - please close the console window to exit"));
            QMC2_LOG_STR_NO_TIME(QString("-").repeated(consoleMessage.length()));
            QMC2_LOG_STR_NO_TIME(consoleMessage);
            QMC2_LOG_STR_NO_TIME(QString("-").repeated(consoleMessage.length()));
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
