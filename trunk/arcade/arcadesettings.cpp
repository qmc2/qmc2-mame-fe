#include "arcadesettings.h"
#include "macros.h"

extern int emulatorMode;

ArcadeSettings::ArcadeSettings(QString theme)
    : QSettings(QSettings::IniFormat, QSettings::UserScope, QMC2_ARCADE_APP_NAME)
{
    arcadeTheme = theme;
    switch ( emulatorMode ) {
    case QMC2_ARCADE_EMUMODE_MAME:
#if defined(QMC2_OS_WIN)
        frontEndPrefix = "Frontend/qmc2-mame";
#else
        frontEndPrefix = "Frontend/qmc2-sdlmame";
#endif
        emulatorPrefix = "MAME";
        break;
    case QMC2_ARCADE_EMUMODE_MESS:
#if defined(QMC2_OS_WIN)
        frontEndPrefix = "Frontend/qmc2-mess";
#else
        frontEndPrefix = "Frontend/qmc2-sdlmess";
#endif
        emulatorPrefix = "MESS";
        break;
    case QMC2_ARCADE_EMUMODE_UME:
#if defined(QMC2_OS_WIN)
        frontEndPrefix = "Frontend/qmc2-ume";
#else
        frontEndPrefix = "Frontend/qmc2-sdlume";
#endif
        emulatorPrefix = "UME";
        break;
    }
}

ArcadeSettings::~ArcadeSettings()
{
    sync();
}

void ArcadeSettings::setApplicationVersion(QString version)
{
    setValue("Arcade/Version", version);
}

QString ArcadeSettings::applicationVersion()
{
    return value("Arcade/Version", QMC2_ARCADE_APP_VERSION).toString();
}

void ArcadeSettings::setViewerGeometry(QByteArray geom)
{
    setValue("Arcade/ViewerGeometry", geom);
}

QByteArray ArcadeSettings::viewerGeometry()
{
    return value("Arcade/ViewerGeometry", QByteArray()).toByteArray();
}

void ArcadeSettings::setViewerMaximized(bool maximized)
{
    setValue("Arcade/ViewerMaximized", maximized);
}

bool ArcadeSettings::viewerMaximized()
{
    return value("Arcade/ViewerMaximized", false).toBool();
}

void ArcadeSettings::setConsoleGeometry(QByteArray geom)
{
    setValue("Arcade/ConsoleGeometry", geom);
}

QByteArray ArcadeSettings::consoleGeometry()
{
    return value("Arcade/ConsoleGeometry", QByteArray()).toByteArray();
}

void ArcadeSettings::setFpsVisible(bool visible)
{
    setValue(QString("Arcade/%1/fpsVisible").arg(arcadeTheme), visible);
}

bool ArcadeSettings::fpsVisible()
{
    return value(QString("Arcade/%1/fpsVisible").arg(arcadeTheme), false).toBool();
}

void ArcadeSettings::setShowBackgroundAnimation(bool show)
{
    setValue(QString("Arcade/%1/showBackgroundAnimation").arg(arcadeTheme), show);
}

bool ArcadeSettings::showBackgroundAnimation()
{
    return value(QString("Arcade/%1/showBackgroundAnimation").arg(arcadeTheme), false).toBool();
}

void ArcadeSettings::setFullScreen(bool fullScreen)
{
    setValue(QString("Arcade/%1/fullScreen").arg(arcadeTheme), fullScreen);
}

bool ArcadeSettings::fullScreen()
{
    return value(QString("Arcade/%1/fullScreen").arg(arcadeTheme), false).toBool();
}

QString ArcadeSettings::gameListCacheFile()
{
    return value(QString("%1/FilesAndDirectories/GamelistCacheFile").arg(emulatorPrefix)).toString();
}

QString ArcadeSettings::romStateCacheFile()
{
    return value(QString("%1/FilesAndDirectories/ROMStateCacheFile").arg(emulatorPrefix)).toString();
}

bool ArcadeSettings::previewsZipped()
{
    return value(QString("%1/FilesAndDirectories/UsePreviewFile").arg(emulatorPrefix)).toBool();
}

QString ArcadeSettings::previewZipFile()
{
    return value(QString("%1/FilesAndDirectories/PreviewFile").arg(emulatorPrefix)).toString();
}

QString ArcadeSettings::previewFolder()
{
    return value(QString("%1/FilesAndDirectories/PreviewDirectory").arg(emulatorPrefix)).toString();
}
