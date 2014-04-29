#include <QTranslator>
#include <QIcon>
#include <QStyleFactory>
#if QT_VERSION < 0x050000
#include <QApplication>
#else
#include <QGuiApplication>
#endif

#include "arcadesettings.h"
#include "tweakedqmlappviewer.h"
#include "consolewindow.h"
#include "macros.h"
#include "joystick.h"
#include "keyeventfilter.h"

ArcadeSettings *globalConfig = NULL;
ConsoleWindow *consoleWindow = NULL;
int emulatorMode = QMC2_ARCADE_EMUMODE_MAME;
int consoleMode = QMC2_ARCADE_CONSOLE_TERM;
QStringList emulatorModes;
QStringList arcadeThemes;
QStringList mameThemes;
QStringList messThemes;
QStringList umeThemes;
QStringList consoleModes;
#if QT_VERSION < 0x050000
QStringList graphicsSystems;
#endif
QStringList argumentList;
bool runApp = true;
bool debugKeys = false;
#if defined(QMC2_ARCADE_ENABLE_JOYSTICK)
bool debugJoy = false;
#endif

#if QT_VERSION < 0x050000
void qtMessageHandler(QtMsgType type, const char *msg)
#else
void qtMessageHandler(QtMsgType type, const QMessageLogContext &, const QString &msg)
#endif
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

    QString defTheme = globalConfig->defaultTheme();
    QString defConsole = globalConfig->defaultConsoleType();
#if QT_VERSION < 0x050000
    QString defGSys = globalConfig->defaultGraphicsSystem();
#endif
    QString defLang = globalConfig->defaultLanguage();

    QStringList themeList;
    foreach (QString theme, arcadeThemes) {
        if ( defTheme == theme )
            themeList << "[" + theme + "]";
        else
            themeList << theme;
    }
    QString availableThemes = themeList.join(", ");

    QStringList consoleList;
    foreach (QString console, consoleModes) {
        if ( defConsole == console )
            consoleList << "[" + console + "]";
        else
            consoleList << console;
    }
    QString availableConsoles = consoleList.join(", ");

#if QT_VERSION < 0x050000
    QStringList gSysList;
    foreach (QString gSys, graphicsSystems) {
        if ( defGSys == gSys )
            gSysList << "[" + gSys + "]";
        else
            gSysList << gSys;
    }
    QString availableGraphicsSystems = gSysList.join(", ");
#endif

    QStringList langList;
    foreach (QString lang, globalConfig->languageMap.keys()) {
        if ( defLang == lang )
            langList << "[" + lang + "]";
        else
            langList << lang;
    }
    QString availableLanguages = langList.join(", ");

    QString helpMessage;
#if QT_VERSION < 0x050000
#if defined(QMC2_ARCADE_ENABLE_JOYSTICK)
    helpMessage  = "Usage: qmc2-arcade [-emu <emulator>] [-theme <theme>] [-console <type>] [-graphicssystem <engine>] [-language <lang>] [-config_path <path>] [-debugkeys] [-nojoy] [-joy <index>] [-debugjoy] [-h|-?|-help]\n\n";
#else
    helpMessage  = "Usage: qmc2-arcade [-emu <emulator>] [-theme <theme>] [-console <type>] [-graphicssystem <engine>] [-language <lang>] [-config_path <path>] [-debugkeys] [-h|-?|-help]\n\n";
#endif
#else
#if defined(QMC2_ARCADE_ENABLE_JOYSTICK)
    helpMessage  = "Usage: qmc2-arcade [-emu <emulator>] [-theme <theme>] [-console <type>] [-language <lang>] [-config_path <path>] [-debugkeys] [-nojoy] [-joy <index>] [-debugjoy] [-h|-?|-help]\n\n";
#else
    helpMessage  = "Usage: qmc2-arcade [-emu <emulator>] [-theme <theme>] [-console <type>] [-language <lang>] [-config_path <path>] [-debugkeys] [-h|-?|-help]\n\n";
#endif
#endif
    helpMessage += "Option           Meaning             Possible values ([..] = default)\n"
                   "---------------  ------------------  -----------------------------------------\n"
                   "-emu             Emulator mode       [mame], mess, ume\n";
    helpMessage += "-theme           Theme selection     " + availableThemes + "\n";
    helpMessage += "-console         Console type        " + availableConsoles + "\n";
#if QT_VERSION < 0x050000
    helpMessage += "-graphicssystem  Graphics engine     " + availableGraphicsSystems + "\n";
#endif
    helpMessage += "-language        Language selection  " + availableLanguages + "\n";
    helpMessage += QString("-config_path     Configuration path  [%1], ...\n").arg(QMC2_ARCADE_DOT_PATH);
    helpMessage += "-debugkeys       Debug key-mapping   N/A\n";
#if defined(QMC2_ARCADE_ENABLE_JOYSTICK)
    helpMessage += "-nojoy           Disable joystick    N/A\n";
    helpMessage += QString("-joy             Use given joystick  SDL joystick index number [%1]\n").arg(globalConfig->joystickIndex());
    helpMessage += "-debugjoy        Debug joy-mapping   N/A\n";
#endif

    QMC2_ARCADE_LOG_STR_NT(helpMessage);
}

#if defined(QMC2_ARCADE_OS_WIN)
#if defined(TCOD_VISUAL_STUDIO)
int SDL_main(int argc, char *argv[]) {
    return main(argc, argv);
}
#endif
#if defined(QMC2_ARCADE_MINGW)
#undef main
#endif
#endif

int main(int argc, char *argv[])
{
    qsrand(QDateTime::currentDateTime().toTime_t());
#if QT_VERSION < 0x050000
    qInstallMsgHandler(qtMessageHandler);
#else
    qInstallMessageHandler(qtMessageHandler);
#endif

    // available emulator-modes, themes, console-modes and graphics-systems
    emulatorModes << "mame" << "mess" << "ume";
    arcadeThemes << "ToxicWaste" << "darkone";
    mameThemes << "ToxicWaste" << "darkone";
    // messThemes << "..."
    umeThemes << "ToxicWaste" << "darkone";
    consoleModes << "terminal" << "window" << "window-minimized";
#if QT_VERSION < 0x050000
    graphicsSystems << "raster" << "native" << "opengl" << "opengl1" << "openvg";
#endif

    // we have to make a copy of the command line arguments since QApplication's constructor "eats"
    // -graphicssystem and its value (and we *really* need to know if it has been set or not!)
    for (int i = 0; i < argc; i++)
        argumentList << argv[i];

#if QT_VERSION < 0x050000 && !defined(QMC2_ARCADE_OS_MAC)
    // work-around for a weird style-related issue with GTK (we actually don't need to set a GUI style, but
    // somehow Qt and/or GTK do this implicitly, which may cause crashes when the automatically chosen style
    // isn't bug-free, so we try to fall back to a safe built-in style to avoid this)
    QStringList wantedStyles = QStringList() << "Plastique" << "Cleanlooks" << "Fusion" << "CDE" << "Motif" << "Windows";
    QStringList availableStyles = QStyleFactory::keys();
    int styleIndex = -1;
    foreach (QString style, wantedStyles) {
        styleIndex = availableStyles.indexOf(style);
        if ( styleIndex >= 0 )
            break;
    }
    if ( styleIndex >= 0 )
        QApplication::setStyle(availableStyles[styleIndex]);

    QApplication *tempApp = new QApplication(argc, argv);
#endif

    QCoreApplication::setOrganizationName(QMC2_ARCADE_ORG_NAME);
    QCoreApplication::setOrganizationDomain(QMC2_ARCADE_ORG_DOMAIN);
    QCoreApplication::setApplicationName(QMC2_ARCADE_APP_NAME);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, QMC2_ARCADE_DYN_DOT_PATH);

#if QT_VERSION < 0x050000
    globalConfig = new ArcadeSettings;

    QString gSys = globalConfig->defaultGraphicsSystem();
    if ( QMC2_ARCADE_CLI_GSYS_VAL )
        gSys = QMC2_ARCADE_CLI_GSYS;

    delete globalConfig;
    globalConfig = NULL;

#if !defined(QMC2_ARCADE_OS_MAC)
    delete tempApp;
#endif

    if ( !graphicsSystems.contains(gSys) ) {
        QMC2_ARCADE_LOG_STR_NT(QObject::tr("%1 is not a valid graphics-system - available graphics-systems: %2").arg(gSys).arg(graphicsSystems.join(", ")));
        return 1;
    }

    QApplication::setGraphicsSystem(gSys);

    // create the actual application instance
    QScopedPointer<QApplication> app(createApplication(argc, argv));
#else
    // create the actual application instance
    QGuiApplication *app = new QGuiApplication(argc, argv);
#endif

    if ( !QMC2_ARCADE_CLI_EMU_UNK ) {
        emulatorMode = QMC2_ARCADE_CLI_EMU_MAME ? QMC2_ARCADE_EMUMODE_MAME : QMC2_ARCADE_CLI_EMU_MESS ? QMC2_ARCADE_EMUMODE_MESS : QMC2_ARCADE_CLI_EMU_UME ? QMC2_ARCADE_EMUMODE_UME : QMC2_ARCADE_EMUMODE_UNK;
        if ( emulatorMode == QMC2_ARCADE_EMUMODE_UNK ) {
            showHelp();
            return 1;
        }
    } else if ( !emulatorModes.contains(QMC2_ARCADE_CLI_EMU) ) {
        QMC2_ARCADE_LOG_STR_NT(QObject::tr("%1 is not a valid emulator-mode - available emulator-modes: %2").arg(QMC2_ARCADE_CLI_EMU).arg(emulatorModes.join(", ")));
        return 1;
    }

    globalConfig = new ArcadeSettings;

    QString console = globalConfig->defaultConsoleType();
    if ( QMC2_ARCADE_CLI_CONS_VAL )
        console = QMC2_ARCADE_CLI_CONS;

    if ( console == "window" || console == "window-minimized" ) {
        consoleMode = console == "window" ? QMC2_ARCADE_CONSOLE_WIN : QMC2_ARCADE_CONSOLE_WINMIN;
        consoleWindow = new ConsoleWindow(0);
        if ( consoleMode == QMC2_ARCADE_CONSOLE_WINMIN )
            consoleWindow->showMinimized();
        else
            consoleWindow->show();
    } else if ( !consoleModes.contains(console) ) {
        QMC2_ARCADE_LOG_STR_NT(QObject::tr("%1 is not a valid console-mode - available console-modes: %2").arg(console).arg(consoleModes.join(", ")));
        return 1;
    }

    if ( QMC2_ARCADE_CLI_HELP || QMC2_ARCADE_CLI_INVALID ) {
        showHelp();
        if ( !consoleWindow ) {
            delete globalConfig;
            return 1;
        } else
            runApp = false;
    }

    QString theme = globalConfig->defaultTheme();
    if ( QMC2_ARCADE_CLI_THEME_VAL )
        theme = QMC2_ARCADE_CLI_THEME;

    if ( !arcadeThemes.contains(theme) && runApp ) {
        QMC2_ARCADE_LOG_STR_NT(QObject::tr("%1 is not valid theme - available themes: %2").arg(theme).arg(arcadeThemes.join(", ")));
        if ( !consoleWindow ) {
            delete globalConfig;
            return 1;
        } else
            runApp = false;
    }

    delete globalConfig;
    globalConfig = NULL;

    switch ( emulatorMode ) {
    case QMC2_ARCADE_EMUMODE_MAME:
        if ( !mameThemes.contains(theme) && runApp ) {
            QMC2_ARCADE_LOG_STR_NT(QObject::tr("%1 is not a valid %2 theme - available %2 themes: %3").arg(theme).arg(emulatorModes[QMC2_ARCADE_EMUMODE_MAME]).arg(mameThemes.isEmpty() ? QObject::tr("(none)") : mameThemes.join(", ")));
            if ( !consoleWindow )
                return 1;
            else
                runApp = false;
        }
        break;
    case QMC2_ARCADE_EMUMODE_MESS:
        if ( !messThemes.contains(theme) && runApp ) {
            QMC2_ARCADE_LOG_STR_NT(QObject::tr("%1 is not a valid %2 theme - available %2 themes: %3").arg(theme).arg(emulatorModes[QMC2_ARCADE_EMUMODE_MESS]).arg(messThemes.isEmpty() ? QObject::tr("(none)") : messThemes.join(", ")));
            if ( !consoleWindow )
                return 1;
            else
                runApp = false;
        }
        break;
    case QMC2_ARCADE_EMUMODE_UME:
        if ( !umeThemes.contains(theme) && runApp ) {
            QMC2_ARCADE_LOG_STR_NT(QObject::tr("%1 is not a valid %2 theme - available %2 themes: %3").arg(theme).arg(emulatorModes[QMC2_ARCADE_EMUMODE_UME]).arg(umeThemes.isEmpty() ? QObject::tr("(none)") : umeThemes.join(", ")));
            if ( !consoleWindow )
                return 1;
            else
                runApp = false;
        }
        break;
    }

    // create final instance of the global settings object
    globalConfig = new ArcadeSettings(theme);
    globalConfig->setApplicationVersion(QMC2_ARCADE_APP_VERSION);

    // set default font
    QString font = globalConfig->defaultFont();
    if ( !font.isEmpty() ) {
      QFont f;
      f.fromString(font);
      app->setFont(f); 
    }

    // set language
    QString language = globalConfig->defaultLanguage();
    if ( QMC2_ARCADE_CLI_LANG_VAL )
        language = QMC2_ARCADE_CLI_LANG;
    if ( !globalConfig->languageMap.contains(language) ) {
        if ( QMC2_ARCADE_CLI_LANG_VAL ) {
            QMC2_ARCADE_LOG_STR_NT(QString("%1 is not a valid language - available languages: %2").arg(language).arg(QStringList(globalConfig->languageMap.keys()).join(", ")));
            delete globalConfig;
            return 1;
        } else
            language = "us";
    }

    // load translator
    QTranslator qmc2ArcadeTranslator;
    if ( qmc2ArcadeTranslator.load(QString("qmc2-arcade_%1").arg(language), ":/translations") )
        app->installTranslator(&qmc2ArcadeTranslator);

    int returnCode;

    if ( runApp ) {
        // log banner message
        QString bannerMessage = QString("%1 %2 (%3)").
                                arg(QMC2_ARCADE_APP_TITLE).
#if defined(QMC2_ARCADE_SVN_REV)
        #if QMC2_ARCADE_SVN_REV > 0
                                arg(QMC2_ARCADE_APP_VERSION + QString(", SVN r%1").arg(XSTR(QMC2_ARCADE_SVN_REV))).
        #else
                                arg(QMC2_ARCADE_APP_VERSION).
        #endif
#else
                                arg(QMC2_ARCADE_APP_VERSION).
#endif
                                arg(QString("Qt") + " " + qVersion() + ", " +
                                    QObject::tr("emulator-mode: %1").arg(emulatorModes[emulatorMode]) + ", " +
                                    QObject::tr("console-mode: %1").arg(consoleModes[consoleMode]) + ", " +
#if QT_VERSION < 0x050000
                                    QObject::tr("graphics-system: %1").arg(gSys) + ", " +
#endif
                                    QObject::tr("language: %1").arg(language) + ", " +
                                    QObject::tr("theme: %1").arg(theme));

        QMC2_ARCADE_LOG_STR(bannerMessage);

        if ( consoleWindow )
            consoleWindow->loadSettings();

        // debug options
        debugKeys = QMC2_ARCADE_CLI_DEBUG_KEYS;
#if defined(QMC2_ARCADE_ENABLE_JOYSTICK)
        debugJoy = QMC2_ARCADE_CLI_DEBUG_JOY;
#endif

        // set up the main QML app viewer window
        TweakedQmlApplicationViewer *viewer = new TweakedQmlApplicationViewer();

        // install our key-event filter to remap key-sequences, if applicable
        KeyEventFilter keyEventFilter(viewer->keySequenceMap);
        app->installEventFilter(&keyEventFilter);

#if QT_VERSION < 0x050000
        viewer->setWindowTitle(QMC2_ARCADE_APP_TITLE + " " + QMC2_ARCADE_APP_VERSION + " [Qt " + qVersion() + "]");
        viewer->setWindowIcon(QIcon(QLatin1String(":/images/qmc2-arcade.png")));
        viewer->setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
        viewer->setStyleSheet("background-color: black");
#else
        viewer->setTitle(QMC2_ARCADE_APP_TITLE + " " + QMC2_ARCADE_APP_VERSION + " [Qt " + qVersion() + "]");
        viewer->winId(); // see QTBUG-33370 QQuickView does not set icon correctly
        viewer->setIcon(QIcon(QLatin1String(":/images/qmc2-arcade.png")));
        viewer->setColor(QColor(0,0,0,255));
#endif

        QMC2_ARCADE_LOG_STR(QObject::tr("Starting QML viewer using theme '%1'").arg(theme));

#if QT_VERSION < 0x050000
        viewer->setSource(QString("qrc:/qml/%1/1.1/%1.qml").arg(theme));
#else
        viewer->setSource(QString("qrc:/qml/%1/2.0/%1.qml").arg(theme));
#endif

        // set up display mode initially
        if ( globalConfig->fullScreen() )
            viewer->switchToFullScreen(true);
        else
            viewer->switchToWindowed(true);

        // start counting frames per second
        if ( viewer->rootObject() )
            viewer->frameCheckTimer.start(QMC2_ARCADE_FPS_UPDATE_INTERVAL);

        // run the event loop
        returnCode = app->exec();

        // remove the key-event filter before destroying the viewer, otherwise there's a small possibility
        // for an exit-crash because the event-filter uses the key-sequence-map instance from the viewer
        app->removeEventFilter(&keyEventFilter);
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
