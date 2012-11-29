#include <QGraphicsObject>
#include "tweakedqmlappviewer.h"

TweakedQmlApplicationViewer::TweakedQmlApplicationViewer(QWidget *parent)
	: QmlApplicationViewer(parent)
{
    numFrames = 0;
    connect(&frameCheckTimer, SIGNAL(timeout()), this, SLOT(fpsReady()));
    frameCheckTimer.start(1000);
}

TweakedQmlApplicationViewer::~TweakedQmlApplicationViewer()
{
}

void TweakedQmlApplicationViewer::fpsReady()
{
    rootObject()->setProperty("fps", numFrames);
    numFrames = 0;
}

void TweakedQmlApplicationViewer::paintEvent(QPaintEvent *e)
{
    numFrames++;
	QmlApplicationViewer::paintEvent(e);
}
