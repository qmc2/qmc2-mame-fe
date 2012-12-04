#ifndef TWEAKEDQMLAPPVIEWER_H
#define TWEAKEDQMLAPPVIEWER_H

#include "qmlapplicationviewer.h"
#include <QTimer>

class TweakedQmlApplicationViewer : public QmlApplicationViewer
{
    Q_OBJECT

public:
    int numFrames;
    QTimer frameCheckTimer;
    QByteArray savedGeometry;
    bool initialSwitch;

    explicit TweakedQmlApplicationViewer(QWidget *parent = 0);
    virtual ~TweakedQmlApplicationViewer();

public slots:
    void fpsReady();
    void loadSettings();
    void saveSettings();
    void switchToFullScreen();
    void switchToWindowed();

protected:
    void paintEvent(QPaintEvent *);
};

#endif
