#ifndef ARCADESETTINGS_H
#define ARCADESETTINGS_H

#include <qglobal.h>
#include <QLocale>
#include <QSize>
#include <QStringList>

#include "../settings.h"

class ArcadeSettings : public Settings
{
    Q_OBJECT

public:
    QString arcadeTheme;
    QString frontEndPrefix;
    QString emulatorPrefix;
    QMap<QString, QLocale::Language> languageMap;

    explicit ArcadeSettings(QString theme = QString());
    virtual ~ArcadeSettings();
    
    QString languageToString(QLocale::Language);
    QLocale::Language languageFromString(QString);
    QString keySequenceMapBaseKey();
#if defined(QMC2_ARCADE_ENABLE_JOYSTICK)
    QString joyFunctionMapBaseKey();
#endif

signals:
    
public slots:
    // global settings
    void setApplicationVersion(QString);
    QString applicationVersion();
    void setViewerGeometry(QByteArray);
    QByteArray viewerGeometry();
    void setViewerMaximized(bool);
    bool viewerMaximized();
    void setConsoleGeometry(QByteArray);
    QByteArray consoleGeometry();
    void setUseFilteredList(bool);
    bool useFilteredList();
    void setFilteredListFile(QString);
    QString filteredListFile();

    // default settings for command line arguments
    void setDefaultTheme(QString);
    QString defaultTheme();
    void setDefaultConsoleType(QString);
    QString defaultConsoleType();
#if QT_VERSION < 0x050000
    void setDefaultGraphicsSystem(QString);
    QString defaultGraphicsSystem();
#endif
    void setDefaultLanguage(QString);
    QString defaultLanguage();
    void setDefaultFont(QString);
    QString defaultFont();

    // theme-specific settings (general)
    void setFpsVisible(bool);
    bool fpsVisible();
    void setFullScreen(bool);
    bool fullScreen();
    void setLastIndex(int);
    int lastIndex();
    void setOverlayScale(double);
    double overlayScale();

    // ToxicWaste
    void setShowBackgroundAnimation(bool);
    bool showBackgroundAnimation();
    void setAnimateInForeground(bool);
    bool animateInForeground();
    void setSecondaryImageType(QString);
    QString secondaryImageType();
    void setCabinetFlipped(bool);
    bool cabinetFlipped();
    void setMenuHidden(bool);
    bool menuHidden();
    void setShowShaderEffect(bool);
    bool showShaderEffect();
    void setConfirmQuit(bool);
    bool confirmQuit();
    void setGameCardPage(int);
    int gameCardPage();
    void setAutoPositionOverlay(bool);
    bool autoPositionOverlay();
    void setOverlayOffsetX(double);
    double overlayOffsetX();
    void setOverlayOffsetY(double);
    double overlayOffsetY();
    void setOverlayOpacity(double);
    double overlayOpacity();
    void setBackgroundOpacity(double);
    double backgroundOpacity();
    void setGameListOpacity(double);
    double gameListOpacity();
    void setCabinetImageType(QString);
    QString cabinetImageType();
    void setAutoStopAnimations(bool);
    bool autoStopAnimations();

    // darkone
    void setToolbarHidden(bool);
    bool toolbarHidden();
    void setListHidden(bool);
    bool listHidden();
    void setSortByName(bool);
    bool sortByName();
    void setScreenLight(bool);
    bool screenLight();
    void setScreenLightOpacity(double);
    double screenLightOpacity();
    void setBackLight(bool);
    bool backLight();
    void setBackLightOpacity(double);
    double backLightOpacity();
    void setToolbarAutoHide(bool);
    bool toolbarAutoHide();
    void setLaunchFlash(bool);
    bool launchFlash();
    void setLaunchZoom(bool);
    bool launchZoom();
    void setDataTypePrimary(QString);
    QString dataTypePrimary();
    void setDataTypeSecondary(QString);
    QString dataTypeSecondary();
    void setLightTimeout(double);
    double lightTimeout();
    void setColourScheme(QString);
    QString colourScheme();

    // main frontend / emulator settings (from QMC2, read-only)
    QString gameListCacheFile();
    QString romStateCacheFile();
    bool previewsZipped();
    bool previewsSevenZipped();
    QString previewFile();
    QString previewFolder();
    bool flyersZipped();
    bool flyersSevenZipped();
    QString flyerFile();
    QString flyerFolder();
    bool cabinetsZipped();
    bool cabinetsSevenZipped();
    QString cabinetFile();
    QString cabinetFolder();
    bool controllersZipped();
    bool controllersSevenZipped();
    QString controllerFile();
    QString controllerFolder();
    bool marqueesZipped();
    bool marqueesSevenZipped();
    QString marqueeFile();
    QString marqueeFolder();
    bool titlesZipped();
    bool titlesSevenZipped();
    QString titleFile();
    QString titleFolder();
    bool pcbsZipped();
    bool pcbsSevenZipped();
    QString pcbFile();
    QString pcbFolder();
    bool swSnapsZipped();
    bool swSnapsSevenZipped();
    QString swSnapFile();
    QString swSnapFolder();
    bool iconsZipped();
    bool iconsSevenZipped();
    QString iconFile();
    QString iconFolder();
    QString optionsTemplateFile();
    QString emulatorExecutablePath();
    QString emulatorWorkingDirectory();
    QString mameHistoryDat();
    bool compressMameHistoryDat();
    QString messSysinfoDat();
    bool compressMessSysinfoDat();
    QString mameInfoDat();
    bool compressMameInfoDat();
    QString messInfoDat();
    bool compressMessInfoDat();
    QStringList activeImageFormats(QString);
#if defined(QMC2_ARCADE_ENABLE_JOYSTICK)
    int joystickAxisMinimum(int, int);
    int joystickAxisMaximum(int, int);
    bool joystickAxisEnabled(int, int);
    int joystickIndex();
    bool joystickEnabled();
    int joystickEventTimeout();
    bool joystickAutoRepeat();
    int joystickAutoRepeatTimeout();
    int joystickDeadzone(int, int);
    int joystickSensitivity(int, int);
#endif

private:
    QMap<QString, QMap<QString, QVariant> > m_themeDefaults;
};

#endif
