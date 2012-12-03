#include <QGraphicsObject>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>

#include "tweakedqmlappviewer.h"
#include "imageprovider.h"

TweakedQmlApplicationViewer::TweakedQmlApplicationViewer(QWidget *parent)
	: QmlApplicationViewer(parent)
{
    numFrames = 0;
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    rootContext()->setContextProperty("viewer", this);
    engine()->addImageProvider(QLatin1String("qmc2"), new ImageProvider(QDeclarativeImageProvider::Image));

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
