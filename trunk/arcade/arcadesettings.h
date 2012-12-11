#ifndef ARCADESETTINGS_H
#define ARCADESETTINGS_H

#include <QSettings>
#include <QLocale>
#include <QSize>

class ArcadeSettings : public QSettings
{
    Q_OBJECT

public:
    QString arcadeTheme;
    QString frontEndPrefix;
    QString emulatorPrefix;
    QMap<QString, QLocale::Language> languageMap;

    explicit ArcadeSettings(QString);
    virtual ~ArcadeSettings();
    
    QString languageToString(QLocale::Language);
    QLocale::Language languageFromString(QString);

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

    // theme-specific settings
    void setFpsVisible(bool);
    bool fpsVisible();
    void setShowBackgroundAnimation(bool);
    bool showBackgroundAnimation();
    void setFullScreen(bool);
    bool fullScreen();
    void setSecondaryImageType(QString);
    QString secondaryImageType();
    void setCabinetFlipped(bool);
    bool cabinetFlipped();
    void setLastIndex(int);
    int lastIndex();

    // main frontend / emulator settings (from QMC2, read-only)
    QString gameListCacheFile();
    QString romStateCacheFile();
    bool previewsZipped();
    QString previewZipFile();
    QString previewFolder();
    bool flyersZipped();
    QString flyerZipFile();
    QString flyerFolder();
    bool cabinetsZipped();
    QString cabinetZipFile();
    QString cabinetFolder();
    bool controllersZipped();
    QString controllerZipFile();
    QString controllerFolder();
    bool marqueesZipped();
    QString marqueeZipFile();
    QString marqueeFolder();
    bool titlesZipped();
    QString titleZipFile();
    QString titleFolder();
    bool pcbsZipped();
    QString pcbZipFile();
    QString pcbFolder();
    QString optionsTemplateFile();
    QString emulatorExecutablePath();
    QString emulatorWorkingDirectory();
    QString language();
};

#endif
