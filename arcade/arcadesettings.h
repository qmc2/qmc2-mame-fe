#ifndef ARCADESETTINGS_H
#define ARCADESETTINGS_H

#include <QSettings>
#include <QSize>

class ArcadeSettings : public QSettings
{
    Q_OBJECT
public:
    QString arcadeTheme;

    ArcadeSettings(QString);
    ~ArcadeSettings();
    
signals:
    
public slots:
    void setApplicationVersion(QString version);
    QString applicationVersion();
    void setViewerGeometry(QByteArray geom);
    QByteArray viewerGeometry();

    void setFpsVisible(bool);
    bool fpsVisible();
    void setShowBackgroundAnimation(bool);
    bool showBackgroundAnimation();
    void setFullScreen(bool);
    bool fullScreen();
};

#endif
