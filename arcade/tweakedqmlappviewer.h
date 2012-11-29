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

    explicit TweakedQmlApplicationViewer(QWidget *parent = 0);
    virtual ~TweakedQmlApplicationViewer();

public slots:
    void fpsReady();

protected:
    void paintEvent(QPaintEvent *);
};

#endif
