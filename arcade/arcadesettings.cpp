#include "arcadesettings.h"
#include "macros.h"

extern int emulatorMode;

ArcadeSettings::ArcadeSettings(QString theme)
    : Settings(QSettings::IniFormat, QSettings::UserScope, QMC2_ARCADE_APP_NAME)
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

    // theme-specific default values (add only if different from global default)
    m_themeDefaults["darkone"]["sortByName"] = true;
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

QString ArcadeSettings::keySequenceMapBaseKey()
{
    return QString("Arcade/%1/keySequenceMap").arg(arcadeTheme);
}

#if defined(QMC2_ARCADE_ENABLE_JOYSTICK)
QString ArcadeSettings::joyFunctionMapBaseKey()
{
    return QString("Arcade/%1/joyFunctionMap").arg(arcadeTheme);
}
#endif

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

void ArcadeSettings::setUseFilteredList(bool use)
{
    setValue(QString("Arcade/%1/UseFilteredList").arg(emulatorPrefix), use);
}

bool ArcadeSettings::useFilteredList()
{
    return value(QString("Arcade/%1/UseFilteredList").arg(emulatorPrefix), false).toBool();
}

void ArcadeSettings::setFilteredListFile(QString fileName)
{
    setValue(QString("Arcade/%1/FilteredListFile").arg(emulatorPrefix), fileName);
}

QString ArcadeSettings::filteredListFile()
{
    return value(QString("Arcade/%1/FilteredListFile").arg(emulatorPrefix), QString()).toString();
}

void ArcadeSettings::setDefaultTheme(QString theme)
{
    setValue(QString("%1/Arcade/Theme").arg(frontEndPrefix), theme);
}

QString ArcadeSettings::defaultTheme()
{
    return value(QString("%1/Arcade/Theme").arg(frontEndPrefix), "ToxicWaste").toString();
}

void ArcadeSettings::setDefaultConsoleType(QString consoleType)
{
    setValue(QString("%1/Arcade/ConsoleType").arg(frontEndPrefix), consoleType);
}

QString ArcadeSettings::defaultConsoleType()
{
    return value(QString("%1/Arcade/ConsoleType").arg(frontEndPrefix), "terminal").toString();
}

#if QT_VERSION < 0x050000
void ArcadeSettings::setDefaultGraphicsSystem(QString graphicsSystem)
{
    setValue(QString("%1/Arcade/GraphicsSystem").arg(frontEndPrefix), graphicsSystem);
}

QString ArcadeSettings::defaultGraphicsSystem()
{
    return value(QString("%1/Arcade/GraphicsSystem").arg(frontEndPrefix), "raster").toString();
}
#endif

void ArcadeSettings::setDefaultLanguage(QString lang)
{
    setValue(QString("%1/GUI/Language").arg(frontEndPrefix), lang);
}

QString ArcadeSettings::defaultLanguage()
{
    return value(QString("%1/GUI/Language").arg(frontEndPrefix), "us").toString();
}

void ArcadeSettings::setDefaultFont(QString font)
{
    setValue(QString("%1/GUI/Font").arg(frontEndPrefix), font);
}

QString ArcadeSettings::defaultFont()
{
    return value(QString("%1/GUI/Font").arg(frontEndPrefix)).toString();
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

void ArcadeSettings::setAnimateInForeground(bool foreground)
{
    setValue(QString("Arcade/%1/animateInForeground").arg(arcadeTheme), foreground);
}

bool ArcadeSettings::animateInForeground()
{
    return value(QString("Arcade/%1/animateInForeground").arg(arcadeTheme), false).toBool();
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

void ArcadeSettings::setMenuHidden(bool hidden)
{
    setValue(QString("Arcade/%1/menuHidden").arg(arcadeTheme), hidden);
}

bool ArcadeSettings::menuHidden()
{
    return value(QString("Arcade/%1/menuHidden").arg(arcadeTheme), false).toBool();
}

void ArcadeSettings::setShowShaderEffect(bool show)
{
    setValue(QString("Arcade/%1/showShaderEffect").arg(arcadeTheme), show);
}

bool ArcadeSettings::showShaderEffect()
{
    return value(QString("Arcade/%1/showShaderEffect").arg(arcadeTheme), false).toBool();
}

void ArcadeSettings::setConfirmQuit(bool confirm)
{
    setValue(QString("Arcade/%1/confirmQuit").arg(arcadeTheme), confirm);
}

bool ArcadeSettings::confirmQuit()
{
    return value(QString("Arcade/%1/confirmQuit").arg(arcadeTheme), true).toBool();
}

void ArcadeSettings::setGameCardPage(int page)
{
    setValue(QString("Arcade/%1/gameCardPage").arg(arcadeTheme), page);
}

int ArcadeSettings::gameCardPage()
{
    return value(QString("Arcade/%1/gameCardPage").arg(arcadeTheme), 0).toInt();
}

void ArcadeSettings::setAutoPositionOverlay(bool autoPosition)
{
    setValue(QString("Arcade/%1/autoPositionOverlay").arg(arcadeTheme), autoPosition);
}

bool ArcadeSettings::autoPositionOverlay()
{
    return value(QString("Arcade/%1/autoPositionOverlay").arg(arcadeTheme), true).toBool();
}

void ArcadeSettings::setOverlayOffsetX(double offset)
{
    setValue(QString("Arcade/%1/overlayOffsetX").arg(arcadeTheme), offset);
}

double ArcadeSettings::overlayOffsetX()
{
    return value(QString("Arcade/%1/overlayOffsetX").arg(arcadeTheme), 0).toDouble();
}

void ArcadeSettings::setOverlayOffsetY(double offset)
{
    setValue(QString("Arcade/%1/overlayOffsetY").arg(arcadeTheme), offset);
}

double ArcadeSettings::overlayOffsetY()
{
    return value(QString("Arcade/%1/overlayOffsetY").arg(arcadeTheme), 0).toDouble();
}

void ArcadeSettings::setOverlayOpacity(double opacity)
{
    setValue(QString("Arcade/%1/overlayOpacity").arg(arcadeTheme), opacity);
}

double ArcadeSettings::overlayOpacity()
{
    return value(QString("Arcade/%1/overlayOpacity").arg(arcadeTheme), 1).toDouble();
}

void ArcadeSettings::setBackgroundOpacity(double opacity)
{
    setValue(QString("Arcade/%1/backgroundOpacity").arg(arcadeTheme), opacity);
}

double ArcadeSettings::backgroundOpacity()
{
    return value(QString("Arcade/%1/backgroundOpacity").arg(arcadeTheme), 1).toDouble();
}

void ArcadeSettings::setGameListOpacity(double opacity)
{
    setValue(QString("Arcade/%1/gameListOpacity").arg(arcadeTheme), opacity);
}

double ArcadeSettings::gameListOpacity()
{
    return value(QString("Arcade/%1/gameListOpacity").arg(arcadeTheme), 1).toDouble();
}

void ArcadeSettings::setCabinetImageType(QString imageType)
{
    setValue(QString("Arcade/%1/cabinetImageType").arg(arcadeTheme), imageType);
}

QString ArcadeSettings::cabinetImageType()
{
    return value(QString("Arcade/%1/cabinetImageType").arg(arcadeTheme), "preview").toString();
}

void ArcadeSettings::setAutoStopAnimations(bool autoStop)
{
    setValue(QString("Arcade/%1/autoStopAnimations").arg(arcadeTheme), autoStop);
}

bool ArcadeSettings::autoStopAnimations()
{
    return value(QString("Arcade/%1/autoStopAnimations").arg(arcadeTheme), true).toBool();
}

void ArcadeSettings::setToolbarHidden(bool hidden)
{
    setValue(QString("Arcade/%1/toolbarHidden").arg(arcadeTheme), hidden);
}

bool ArcadeSettings::toolbarHidden()
{
    return value(QString("Arcade/%1/toolbarHidden").arg(arcadeTheme), false).toBool();
}

void ArcadeSettings::setListHidden(bool hidden)
{
    setValue(QString("Arcade/%1/listHidden").arg(arcadeTheme), hidden);
}

bool ArcadeSettings::listHidden()
{
    return value(QString("Arcade/%1/listHidden").arg(arcadeTheme), false).toBool();
}

void ArcadeSettings::setSortByName(bool sortByName)
{
    setValue(QString("Arcade/%1/sortByName").arg(arcadeTheme), sortByName);
}

bool ArcadeSettings::sortByName()
{
    return value(QString("Arcade/%1/sortByName").arg(arcadeTheme), m_themeDefaults[arcadeTheme]["sortByName"].toBool()).toBool();
}

void ArcadeSettings::setToolbarAutoHide(bool toolbarAutoHide)
{
    setValue(QString("Arcade/%1/toolbarAutoHide").arg(arcadeTheme), toolbarAutoHide);
}
bool ArcadeSettings::toolbarAutoHide()
{
    return value(QString("Arcade/%1/toolbarAutoHide").arg(arcadeTheme), true).toBool();
}

void ArcadeSettings::setScreenLight(bool screenLight)
{
    setValue(QString("Arcade/%1/screenLight").arg(arcadeTheme), screenLight);
}
bool ArcadeSettings::screenLight()
{
    return value(QString("Arcade/%1/screenLight").arg(arcadeTheme), false).toBool();
}

void ArcadeSettings::setScreenLightOpacity(double screenLightOpacity)
{
    setValue(QString("Arcade/%1/screenLightOpacity").arg(arcadeTheme), screenLightOpacity);
}
double ArcadeSettings::screenLightOpacity()
{
    return value(QString("Arcade/%1/screenLightOpacity").arg(arcadeTheme), 0.5).toDouble();
}

void ArcadeSettings::setBackLight(bool backLight)
{
    setValue(QString("Arcade/%1/backLight").arg(arcadeTheme), backLight);
}
bool ArcadeSettings::backLight()
{
    return value(QString("Arcade/%1/backLight").arg(arcadeTheme), false).toBool();
}

void ArcadeSettings::setBackLightOpacity(double backLightOpacity)
{
    setValue(QString("Arcade/%1/backLightOpacity").arg(arcadeTheme), backLightOpacity);
}
double ArcadeSettings::backLightOpacity()
{
    return value(QString("Arcade/%1/backLightOpacity").arg(arcadeTheme), 0.75).toDouble();
}

void ArcadeSettings::setLaunchFlash(bool launchFlash)
{
    setValue(QString("Arcade/%1/launchFlash").arg(arcadeTheme), launchFlash);
}
bool ArcadeSettings::launchFlash()
{
    return value(QString("Arcade/%1/launchFlash").arg(arcadeTheme), true).toBool();
}

void ArcadeSettings::setLaunchZoom(bool launchZoom)
{
    setValue(QString("Arcade/%1/launchZoom").arg(arcadeTheme), launchZoom);
}
bool ArcadeSettings::launchZoom()
{
    return value(QString("Arcade/%1/launchZoom").arg(arcadeTheme), true).toBool();
}

void ArcadeSettings::setOverlayScale(double overlayScale)
{
    setValue(QString("Arcade/%1/overlayScale").arg(arcadeTheme), overlayScale);
}
double ArcadeSettings::overlayScale()
{
    return value(QString("Arcade/%1/overlayScale").arg(arcadeTheme), 1.0).toDouble();
}

void ArcadeSettings::setLightTimeout(double lightTimeout)
{
    setValue(QString("Arcade/%1/lightTimeout").arg(arcadeTheme), lightTimeout);
}
double ArcadeSettings::lightTimeout()
{
    return value(QString("Arcade/%1/lightTimeout").arg(arcadeTheme), 60).toDouble();
}

void ArcadeSettings::setDataTypePrimary(QString dataTypePrimary)
{
    setValue(QString("Arcade/%1/dataTypePrimary").arg(arcadeTheme), dataTypePrimary);
}
QString ArcadeSettings::dataTypePrimary()
{
    return value(QString("Arcade/%1/dataTypePrimary").arg(arcadeTheme), "title").toString();
}

void ArcadeSettings::setDataTypeSecondary(QString dataTypeSecondary)
{
    setValue(QString("Arcade/%1/dataTypeSecondary").arg(arcadeTheme), dataTypeSecondary);
}
QString ArcadeSettings::dataTypeSecondary()
{
    return value(QString("Arcade/%1/dataTypeSecondary").arg(arcadeTheme), "preview").toString();
}

void ArcadeSettings::setColourScheme(QString colourScheme)
{
    setValue(QString("Arcade/%1/colourScheme").arg(arcadeTheme), colourScheme);
}
QString ArcadeSettings::colourScheme()
{
    return value(QString("Arcade/%1/colourScheme").arg(arcadeTheme), "dark").toString();
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
    return value(QString("%1/FilesAndDirectories/UsePreviewFile").arg(emulatorPrefix)).toBool() && (value(QString("%1/FilesAndDirectories/PreviewFileType").arg(emulatorPrefix)).toInt() == QMC2_ARCADE_IMG_FILETYPE_ZIP);
}

bool ArcadeSettings::previewsSevenZipped()
{
    return value(QString("%1/FilesAndDirectories/UsePreviewFile").arg(emulatorPrefix)).toBool() && (value(QString("%1/FilesAndDirectories/PreviewFileType").arg(emulatorPrefix)).toInt() == QMC2_ARCADE_IMG_FILETYPE_7Z);
}

QString ArcadeSettings::previewFile()
{
    return value(QString("%1/FilesAndDirectories/PreviewFile").arg(emulatorPrefix)).toString();
}

QString ArcadeSettings::previewFolder()
{
    return value(QString("%1/FilesAndDirectories/PreviewDirectory").arg(emulatorPrefix)).toString();
}

bool ArcadeSettings::flyersZipped()
{
    return value(QString("%1/FilesAndDirectories/UseFlyerFile").arg(emulatorPrefix)).toBool() && (value(QString("%1/FilesAndDirectories/FlyerFileType").arg(emulatorPrefix)).toInt() == QMC2_ARCADE_IMG_FILETYPE_ZIP);
}

bool ArcadeSettings::flyersSevenZipped()
{
    return value(QString("%1/FilesAndDirectories/UseFlyerFile").arg(emulatorPrefix)).toBool() && (value(QString("%1/FilesAndDirectories/FlyerFileType").arg(emulatorPrefix)).toInt() == QMC2_ARCADE_IMG_FILETYPE_7Z);
}

QString ArcadeSettings::flyerFile()
{
    return value(QString("%1/FilesAndDirectories/FlyerFile").arg(emulatorPrefix)).toString();
}

QString ArcadeSettings::flyerFolder()
{
    return value(QString("%1/FilesAndDirectories/FlyerDirectory").arg(emulatorPrefix)).toString();
}

bool ArcadeSettings::cabinetsZipped()
{
    return value(QString("%1/FilesAndDirectories/UseCabinetFile").arg(emulatorPrefix)).toBool() && (value(QString("%1/FilesAndDirectories/CabinetFileType").arg(emulatorPrefix)).toInt() == QMC2_ARCADE_IMG_FILETYPE_ZIP);
}

bool ArcadeSettings::cabinetsSevenZipped()
{
    return value(QString("%1/FilesAndDirectories/UseCabinetFile").arg(emulatorPrefix)).toBool() && (value(QString("%1/FilesAndDirectories/CabinetFileType").arg(emulatorPrefix)).toInt() == QMC2_ARCADE_IMG_FILETYPE_7Z);
}

QString ArcadeSettings::cabinetFile()
{
    return value(QString("%1/FilesAndDirectories/CabinetFile").arg(emulatorPrefix)).toString();
}

QString ArcadeSettings::cabinetFolder()
{
    return value(QString("%1/FilesAndDirectories/CabinetDirectory").arg(emulatorPrefix)).toString();
}

bool ArcadeSettings::controllersZipped()
{
    return value(QString("%1/FilesAndDirectories/UseControllerFile").arg(emulatorPrefix)).toBool() && (value(QString("%1/FilesAndDirectories/ControllerFileType").arg(emulatorPrefix)).toInt() == QMC2_ARCADE_IMG_FILETYPE_ZIP);
}

bool ArcadeSettings::controllersSevenZipped()
{
    return value(QString("%1/FilesAndDirectories/UseControllerFile").arg(emulatorPrefix)).toBool() && (value(QString("%1/FilesAndDirectories/ControllerFileType").arg(emulatorPrefix)).toInt() == QMC2_ARCADE_IMG_FILETYPE_7Z);
}

QString ArcadeSettings::controllerFile()
{
    return value(QString("%1/FilesAndDirectories/ControllerFile").arg(emulatorPrefix)).toString();
}

QString ArcadeSettings::controllerFolder()
{
    return value(QString("%1/FilesAndDirectories/ControllerDirectory").arg(emulatorPrefix)).toString();
}

bool ArcadeSettings::marqueesZipped()
{
    return value(QString("%1/FilesAndDirectories/UseMarqueeFile").arg(emulatorPrefix)).toBool() && (value(QString("%1/FilesAndDirectories/MarqueeFileType").arg(emulatorPrefix)).toInt() == QMC2_ARCADE_IMG_FILETYPE_ZIP);
}

bool ArcadeSettings::marqueesSevenZipped()
{
    return value(QString("%1/FilesAndDirectories/UseMarqueeFile").arg(emulatorPrefix)).toBool() && (value(QString("%1/FilesAndDirectories/MarqueeFileType").arg(emulatorPrefix)).toInt() == QMC2_ARCADE_IMG_FILETYPE_7Z);
}

QString ArcadeSettings::marqueeFile()
{
    return value(QString("%1/FilesAndDirectories/MarqueeFile").arg(emulatorPrefix)).toString();
}

QString ArcadeSettings::marqueeFolder()
{
    return value(QString("%1/FilesAndDirectories/MarqueeDirectory").arg(emulatorPrefix)).toString();
}

bool ArcadeSettings::titlesZipped()
{
    return value(QString("%1/FilesAndDirectories/UseTitleFile").arg(emulatorPrefix)).toBool() && (value(QString("%1/FilesAndDirectories/TitleFileType").arg(emulatorPrefix)).toInt() == QMC2_ARCADE_IMG_FILETYPE_ZIP);
}

bool ArcadeSettings::titlesSevenZipped()
{
    return value(QString("%1/FilesAndDirectories/UseTitleFile").arg(emulatorPrefix)).toBool() && (value(QString("%1/FilesAndDirectories/TitleFileType").arg(emulatorPrefix)).toInt() == QMC2_ARCADE_IMG_FILETYPE_7Z);
}

QString ArcadeSettings::titleFile()
{
    return value(QString("%1/FilesAndDirectories/TitleFile").arg(emulatorPrefix)).toString();
}

QString ArcadeSettings::titleFolder()
{
    return value(QString("%1/FilesAndDirectories/TitleDirectory").arg(emulatorPrefix)).toString();
}

bool ArcadeSettings::pcbsZipped()
{
    return value(QString("%1/FilesAndDirectories/UsePCBFile").arg(emulatorPrefix)).toBool() && (value(QString("%1/FilesAndDirectories/PCBFileType").arg(emulatorPrefix)).toInt() == QMC2_ARCADE_IMG_FILETYPE_ZIP);
}

bool ArcadeSettings::pcbsSevenZipped()
{
    return value(QString("%1/FilesAndDirectories/UsePCBFile").arg(emulatorPrefix)).toBool() && (value(QString("%1/FilesAndDirectories/PCBFileType").arg(emulatorPrefix)).toInt() == QMC2_ARCADE_IMG_FILETYPE_7Z);
}

QString ArcadeSettings::pcbFile()
{
    return value(QString("%1/FilesAndDirectories/PCBFile").arg(emulatorPrefix)).toString();
}

QString ArcadeSettings::pcbFolder()
{
    return value(QString("%1/FilesAndDirectories/PCBDirectory").arg(emulatorPrefix)).toString();
}

bool ArcadeSettings::swSnapsZipped()
{
    return value(QString("%1/FilesAndDirectories/UseSoftwareSnapFile").arg(emulatorPrefix)).toBool() && (value(QString("%1/FilesAndDirectories/SoftwareSnapFileType").arg(emulatorPrefix)).toInt() == QMC2_ARCADE_IMG_FILETYPE_ZIP);
}

bool ArcadeSettings::swSnapsSevenZipped()
{
    return value(QString("%1/FilesAndDirectories/UseSoftwareSnapFile").arg(emulatorPrefix)).toBool() && (value(QString("%1/FilesAndDirectories/SoftwareSnapFileType").arg(emulatorPrefix)).toInt() == QMC2_ARCADE_IMG_FILETYPE_7Z);
}

QString ArcadeSettings::swSnapFile()
{
    return value(QString("%1/FilesAndDirectories/SoftwareSnapFile").arg(emulatorPrefix)).toString();
}

QString ArcadeSettings::swSnapFolder()
{
    return value(QString("%1/FilesAndDirectories/SoftwareSnapDirectory").arg(emulatorPrefix)).toString();
}

bool ArcadeSettings::iconsZipped()
{
    return value(QString("%1/FilesAndDirectories/UseIconFile").arg(emulatorPrefix)).toBool() && (value(QString("%1/FilesAndDirectories/IconFileType").arg(emulatorPrefix)).toInt() == QMC2_ARCADE_IMG_FILETYPE_ZIP);
}

bool ArcadeSettings::iconsSevenZipped()
{
    return value(QString("%1/FilesAndDirectories/UseIconFile").arg(emulatorPrefix)).toBool() && (value(QString("%1/FilesAndDirectories/IconFileType").arg(emulatorPrefix)).toInt() == QMC2_ARCADE_IMG_FILETYPE_7Z);
}

QString ArcadeSettings::iconFile()
{
    return value(QString("%1/FilesAndDirectories/IconFile").arg(emulatorPrefix)).toString();
}

QString ArcadeSettings::iconFolder()
{
    return value(QString("%1/FilesAndDirectories/IconDirectory").arg(emulatorPrefix)).toString();
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

QString ArcadeSettings::mameHistoryDat()
{
    return value(QString("%1/FilesAndDirectories/MameHistoryDat").arg(frontEndPrefix)).toString();
}

bool ArcadeSettings::compressMameHistoryDat()
{
    return value(QString("%1/GUI/CompressMameHistoryDat").arg(frontEndPrefix)).toBool();
}

QString ArcadeSettings::messSysinfoDat()
{
    return value(QString("%1/FilesAndDirectories/MessSysinfoDat").arg(frontEndPrefix)).toString();
}

bool ArcadeSettings::compressMessSysinfoDat()
{
    return value(QString("%1/GUI/CompressMessSysinfoDat").arg(frontEndPrefix)).toBool();
}

QString ArcadeSettings::mameInfoDat()
{
    return value(QString("%1/FilesAndDirectories/MameInfoDat").arg(frontEndPrefix)).toString();
}

bool ArcadeSettings::compressMameInfoDat()
{
    return value(QString("%1/FilesAndDirectories/CompressMameInfoDat").arg(frontEndPrefix)).toBool();
}

QString ArcadeSettings::messInfoDat()
{
    return value(QString("%1/FilesAndDirectories/MessInfoDat").arg(frontEndPrefix)).toString();
}

bool ArcadeSettings::compressMessInfoDat()
{
    return value(QString("%1/FilesAndDirectories/CompressMessInfoDat").arg(frontEndPrefix)).toBool();
}

QStringList ArcadeSettings::activeImageFormats(QString imageType)
{
    return value(QString("%1/ActiveImageFormats/%2").arg(frontEndPrefix).arg(imageType)).toStringList();
}

bool ArcadeSettings::parentImageFallback()
{
    return value(QString("%1/GUI/ParentImageFallback").arg(frontEndPrefix)).toBool();
}

#if defined(QMC2_ARCADE_ENABLE_JOYSTICK)
int ArcadeSettings::joystickAxisMinimum(int joystickIndex, int axis)
{
    return value(QString("%1/Joystick/%2/Axis%3Minimum").arg(frontEndPrefix).arg(joystickIndex).arg(axis), -32767).toInt();
}

int ArcadeSettings::joystickAxisMaximum(int joystickIndex, int axis)
{
    return value(QString("%1/Joystick/%2/Axis%3Maximum").arg(frontEndPrefix).arg(joystickIndex).arg(axis), 32768).toInt();
}

bool ArcadeSettings::joystickAxisEnabled(int joystickIndex, int axis)
{
    return value(QString("%1/Joystick/%2/Axis%3Enabled").arg(frontEndPrefix).arg(joystickIndex).arg(axis), true).toBool();
}

int ArcadeSettings::joystickIndex()
{
    return value(QString("%1/Joystick/Index").arg(frontEndPrefix), 0).toInt();
}

bool ArcadeSettings::joystickEnabled()
{
    return value(QString("%1/Joystick/EnableJoystickControl").arg(frontEndPrefix), false).toBool();
}

int ArcadeSettings::joystickEventTimeout()
{
    return value(QString("%1/Joystick/EventTimeout").arg(frontEndPrefix), 25).toInt();
}

bool ArcadeSettings::joystickAutoRepeat()
{
    return value(QString("%1/Joystick/AutoRepeat").arg(frontEndPrefix), true).toBool();
}

int ArcadeSettings::joystickAutoRepeatTimeout()
{
    return value(QString("%1/Joystick/AutoRepeatTimeout").arg(frontEndPrefix), 250).toInt();
}

int ArcadeSettings::joystickDeadzone(int joystickIndex, int axis)
{
    return value(QString("%1/Joystick/%2/Axis%3Deadzone").arg(frontEndPrefix).arg(joystickIndex).arg(axis), 0).toInt();
}

int ArcadeSettings::joystickSensitivity(int joystickIndex, int axis)
{
    return value(QString("%1/Joystick/%2/Axis%3Sensitivity").arg(frontEndPrefix).arg(joystickIndex).arg(axis), 0).toInt();
}
#endif
