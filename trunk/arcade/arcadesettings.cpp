#include "arcadesettings.h"
#include "macros.h"

extern int emulatorMode;

ArcadeSettings::ArcadeSettings(QString theme)
    : QSettings(QSettings::IniFormat, QSettings::UserScope, QMC2_ARCADE_APP_NAME)
{
    arcadeTheme = theme;
    switch ( emulatorMode ) {
    case QMC2_ARCADE_EMUMODE_MAME:
#if defined(QMC2_ARCADE_OS_WIN)
        frontEndPrefix = "Frontend/qmc2-mame";
#else
        frontEndPrefix = "Frontend/qmc2-sdlmame";
#endif
        emulatorPrefix = "MAME";
        break;
    case QMC2_ARCADE_EMUMODE_MESS:
#if defined(QMC2_ARCADE_OS_WIN)
        frontEndPrefix = "Frontend/qmc2-mess";
#else
        frontEndPrefix = "Frontend/qmc2-sdlmess";
#endif
        emulatorPrefix = "MESS";
        break;
    case QMC2_ARCADE_EMUMODE_UME:
#if defined(QMC2_ARCADE_OS_WIN)
        frontEndPrefix = "Frontend/qmc2-ume";
#else
        frontEndPrefix = "Frontend/qmc2-sdlume";
#endif
        emulatorPrefix = "UME";
        break;
    }
    languageMap["de"] = QLocale::German;
    languageMap["es"] = QLocale::Spanish;
    languageMap["fr"] = QLocale::French;
    languageMap["el"] = QLocale::Greek;
    languageMap["it"] = QLocale::Italian;
    languageMap["pl"] = QLocale::Polish;
    languageMap["pt"] = QLocale::Portuguese;
    languageMap["ro"] = QLocale::Romanian;
    languageMap["sv"] = QLocale::Swedish;
    languageMap["us"] = QLocale::English;
}

ArcadeSettings::~ArcadeSettings()
{
    sync();
}

QString ArcadeSettings::languageToString(QLocale::Language lang)
{
    QString langStr = languageMap.key(lang);
    if ( !langStr.isEmpty() )
        return langStr;
    else
        return "us";
}

QLocale::Language ArcadeSettings::languageFromString(QString lang)
{
    if ( languageMap.contains(lang) )
        return languageMap[lang];
    else
        return QLocale::English;
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

void ArcadeSettings::setSecondaryImageType(QString imageType)
{
    setValue(QString("Arcade/%1/secondaryImageType").arg(arcadeTheme), imageType);
}

QString ArcadeSettings::secondaryImageType()
{
    return value(QString("Arcade/%1/secondaryImageType").arg(arcadeTheme), "preview").toString();
}

void ArcadeSettings::setCabinetFlipped(bool flipped)
{
    setValue(QString("Arcade/%1/cabinetFlipped").arg(arcadeTheme), flipped);
}

bool ArcadeSettings::cabinetFlipped()
{
    return value(QString("Arcade/%1/cabinetFlipped").arg(arcadeTheme), false).toBool();
}

void ArcadeSettings::setLastIndex(int index)
{
    setValue(QString("Arcade/%1/lastIndex").arg(emulatorPrefix), index);
}

int ArcadeSettings::lastIndex()
{
    return value(QString("Arcade/%1/lastIndex").arg(emulatorPrefix), 0).toInt();
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

bool ArcadeSettings::flyersZipped()
{
    return value(QString("%1/FilesAndDirectories/UseFlyerFile").arg(emulatorPrefix)).toBool();
}

QString ArcadeSettings::flyerZipFile()
{
    return value(QString("%1/FilesAndDirectories/FlyerFile").arg(emulatorPrefix)).toString();
}

QString ArcadeSettings::flyerFolder()
{
    return value(QString("%1/FilesAndDirectories/FlyerDirectory").arg(emulatorPrefix)).toString();
}

bool ArcadeSettings::cabinetsZipped()
{
    return value(QString("%1/FilesAndDirectories/UseCabinetFile").arg(emulatorPrefix)).toBool();
}

QString ArcadeSettings::cabinetZipFile()
{
    return value(QString("%1/FilesAndDirectories/CabinetFile").arg(emulatorPrefix)).toString();
}

QString ArcadeSettings::cabinetFolder()
{
    return value(QString("%1/FilesAndDirectories/CabinetDirectory").arg(emulatorPrefix)).toString();
}

bool ArcadeSettings::controllersZipped()
{
    return value(QString("%1/FilesAndDirectories/UseControllerFile").arg(emulatorPrefix)).toBool();
}

QString ArcadeSettings::controllerZipFile()
{
    return value(QString("%1/FilesAndDirectories/ControllerFile").arg(emulatorPrefix)).toString();
}

QString ArcadeSettings::controllerFolder()
{
    return value(QString("%1/FilesAndDirectories/ControllerDirectory").arg(emulatorPrefix)).toString();
}

bool ArcadeSettings::marqueesZipped()
{
    return value(QString("%1/FilesAndDirectories/UseMarqueeFile").arg(emulatorPrefix)).toBool();
}

QString ArcadeSettings::marqueeZipFile()
{
    return value(QString("%1/FilesAndDirectories/MarqueeFile").arg(emulatorPrefix)).toString();
}

QString ArcadeSettings::marqueeFolder()
{
    return value(QString("%1/FilesAndDirectories/MarqueeDirectory").arg(emulatorPrefix)).toString();
}

bool ArcadeSettings::titlesZipped()
{
    return value(QString("%1/FilesAndDirectories/UseTitleFile").arg(emulatorPrefix)).toBool();
}

QString ArcadeSettings::titleZipFile()
{
    return value(QString("%1/FilesAndDirectories/TitleFile").arg(emulatorPrefix)).toString();
}

QString ArcadeSettings::titleFolder()
{
    return value(QString("%1/FilesAndDirectories/TitleDirectory").arg(emulatorPrefix)).toString();
}

bool ArcadeSettings::pcbsZipped()
{
    return value(QString("%1/FilesAndDirectories/UsePCBFile").arg(emulatorPrefix)).toBool();
}

QString ArcadeSettings::pcbZipFile()
{
    return value(QString("%1/FilesAndDirectories/PCBFile").arg(emulatorPrefix)).toString();
}

QString ArcadeSettings::pcbFolder()
{
    return value(QString("%1/FilesAndDirectories/PCBDirectory").arg(emulatorPrefix)).toString();
}

QString ArcadeSettings::optionsTemplateFile()
{
    return value(QString("%1/FilesAndDirectories/OptionsTemplateFile").arg(emulatorPrefix)).toString();
}

QString ArcadeSettings::emulatorExecutablePath()
{
    return value(QString("%1/FilesAndDirectories/ExecutableFile").arg(emulatorPrefix)).toString();
}

QString ArcadeSettings::emulatorWorkingDirectory()
{
    return value(QString("%1/FilesAndDirectories/WorkingDirectory").arg(emulatorPrefix)).toString();
}

QString ArcadeSettings::language()
{
    return value(QString("%1/GUI/Language").arg(frontEndPrefix)).toString();
}
