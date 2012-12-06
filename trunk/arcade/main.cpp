#include <QApplication>

#include "arcadesettings.h"
#include "tweakedqmlappviewer.h"
#include "macros.h"

ArcadeSettings *globalConfig = NULL;
int emulatorMode = QMC2_ARCADE_MODE_MAME;
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

int showHelp()
{
    QString helpMessage = QObject::tr("Usage: qmc2-arcade [-emu <emulator>] [-theme <theme>] [-graphicssystem <engine>] [-h|-?|-help]\n\n"
                                      "Where <emulator> = mame (default), mess or ume\n"
                                      "      <theme>    = ToxicWaste (default)\n"
                                      "      <engine>   = raster (default) or opengl\n");
    QMC2_LOG_STR_NO_TIME(helpMessage);
    return 1;
}

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    qsrand(QDateTime::currentDateTime().toTime_t());
    qInstallMsgHandler(qtMessageHandler);

    QScopedPointer<QApplication> app(createApplication(argc, argv));

    emulatorModeNames << QObject::tr("MAME") << QObject::tr("MESS") << QObject::tr("UME");
    arcadeThemes << "ToxicWaste";
    mameThemes << "ToxicWaste";
    //messThemes << "..."
    umeThemes << "ToxicWaste";

    QString theme = QMC2_ARCADE_CLI_THEME;

    if ( !arcadeThemes.contains(theme) ) {
        QMC2_LOG_STR_NO_TIME(QObject::tr("%1 is not valid theme - available themes: %2").arg(theme).arg(arcadeThemes.join(", ")));
        return showHelp();
    }

    if ( !QMC2_ARCADE_CLI_EMU_UNK || QMC2_ARCADE_CLI_HELP ) {
        emulatorMode = QMC2_ARCADE_CLI_EMU_MAME ? QMC2_ARCADE_MODE_MAME : QMC2_ARCADE_CLI_EMU_MESS ? QMC2_ARCADE_MODE_MESS : QMC2_ARCADE_CLI_EMU_UME ? QMC2_ARCADE_MODE_UME : QMC2_ARCADE_MODE_UNK;
        if ( emulatorMode == QMC2_ARCADE_MODE_UNK )
            return showHelp();
    }

    switch ( emulatorMode ) {
    case QMC2_ARCADE_MODE_MAME:
        if ( !mameThemes.contains(theme) ) {
            QMC2_LOG_STR_NO_TIME(QObject::tr("%1 is not a valid %2 theme - available themes: %3").arg(theme).arg(emulatorModeNames[QMC2_ARCADE_MODE_MAME]).arg(mameThemes.join(", ")));
            return 1;
        }
        break;
    case QMC2_ARCADE_MODE_MESS:
        if ( !messThemes.contains(theme) ) {
            QMC2_LOG_STR_NO_TIME(QObject::tr("%1 is not a valid %2 theme - available themes: %3").arg(theme).arg(emulatorModeNames[QMC2_ARCADE_MODE_MESS]).arg(messThemes.join(", ")));
            return 1;
        }
        break;
    case QMC2_ARCADE_MODE_UME:
        if ( !umeThemes.contains(theme) ) {
            QMC2_LOG_STR_NO_TIME(QObject::tr("%1 is not a valid %2 theme - available themes: %3").arg(theme).arg(emulatorModeNames[QMC2_ARCADE_MODE_UME]).arg(umeThemes.join(", ")));
            return 1;
        }
        break;
    }

    QMC2_LOG_STR(QString(QString("%1 %2 (%3)").arg(QMC2_ARCADE_APP_TITLE).arg(QMC2_ARCADE_APP_VERSION).arg(QString("Qt ") + qVersion()) + ", " + QObject::tr("running in %1 emulator mode").arg(emulatorModeNames[emulatorMode])));

    QCoreApplication::setOrganizationName(QMC2_ARCADE_ORG_NAME);
    QCoreApplication::setOrganizationDomain(QMC2_ARCADE_ORG_DOMAIN);
    QCoreApplication::setApplicationName(QMC2_ARCADE_APP_NAME);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, QMC2_ARCADE_DYN_DOT_PATH);

    QMC2_LOG_STR(QObject::tr("Loading arcade theme %1").arg(theme));

    globalConfig = new ArcadeSettings(theme);
    globalConfig->setApplicationVersion(QMC2_ARCADE_APP_VERSION);

    TweakedQmlApplicationViewer *viewer = new TweakedQmlApplicationViewer();

    viewer->setWindowTitle(QMC2_ARCADE_APP_TITLE);
    viewer->setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    viewer->setMainQmlFile(QString("qml/%1/%1.qml").arg(theme).toLatin1());

    if ( globalConfig->fullScreen() )
        viewer->switchToFullScreen(true);
    else
        viewer->switchToWindowed(true);

    int returnCode = app->exec();

    delete viewer;
    delete globalConfig;

    QMC2_LOG_STR(QObject::tr("Exiting gracefully"));

    return returnCode;
}
