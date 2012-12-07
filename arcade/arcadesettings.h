#ifndef ARCADESETTINGS_H
#define ARCADESETTINGS_H

#include <QSettings>
#include <QSize>

class ArcadeSettings : public QSettings
{
    Q_OBJECT
public:
    QString arcadeTheme;

    explicit ArcadeSettings(QString);
    virtual ~ArcadeSettings();
    
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
};

#endif
