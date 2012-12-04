#include <QGraphicsObject>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>

#include "tweakedqmlappviewer.h"
#include "imageprovider.h"
#include "arcadesettings.h"

//#include <stdio.h>

extern ArcadeSettings *globalConfig;

TweakedQmlApplicationViewer::TweakedQmlApplicationViewer(QWidget *parent)
	: QmlApplicationViewer(parent)
{
    numFrames = 0;
    initialSwitch = true;

    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    engine()->addImageProvider(QLatin1String("qmc2"), new ImageProvider(QDeclarativeImageProvider::Image));

    // this gives access to the viewer object from JavaScript
    rootContext()->setContextProperty("viewer", this);

    connect(&frameCheckTimer, SIGNAL(timeout()), this, SLOT(fpsReady()));
    frameCheckTimer.start(1000);
}

TweakedQmlApplicationViewer::~TweakedQmlApplicationViewer()
{
    saveSettings();
}

void TweakedQmlApplicationViewer::fpsReady()
{
    rootObject()->setProperty("fps", numFrames);
    numFrames = 0;
}

void TweakedQmlApplicationViewer::loadSettings()
{
    // load global arcade settings
    rootObject()->setProperty("version", globalConfig->applicationVersion());

    // load theme-specific arcade settings
    if ( globalConfig->arcadeTheme == "ToxicWaste" ) {
        rootObject()->setProperty("fpsVisible", globalConfig->fpsVisible());
        rootObject()->setProperty("showBackgroundAnimation", globalConfig->showBackgroundAnimation());
        rootObject()->setProperty("fullScreen", globalConfig->fullScreen());
    }
}

void TweakedQmlApplicationViewer::saveSettings()
{
    // save global arcade settings
    if ( isFullScreen() )
        globalConfig->setViewerGeometry(savedGeometry);
    else
        globalConfig->setViewerGeometry(saveGeometry());

    // save theme-specific arcade settings
    if ( globalConfig->arcadeTheme == "ToxicWaste" ) {
        globalConfig->setFpsVisible(rootObject()->property("fpsVisible").toBool());
        globalConfig->setShowBackgroundAnimation(rootObject()->property("showBackgroundAnimation").toBool());
        globalConfig->setFullScreen(rootObject()->property("fullScreen").toBool());
    }
}

void TweakedQmlApplicationViewer::switchToFullScreen()
{
    if ( initialSwitch )
        savedGeometry = globalConfig->viewerGeometry();
    else
        savedGeometry = saveGeometry();
    showFullScreen();
    initialSwitch = false;
}

void TweakedQmlApplicationViewer::switchToWindowed()
{
    if ( initialSwitch )
        savedGeometry = globalConfig->viewerGeometry();
    restoreGeometry(savedGeometry);
    showNormal();
    initialSwitch = false;
}

void TweakedQmlApplicationViewer::paintEvent(QPaintEvent *e)
{
    numFrames++;
    QmlApplicationViewer::paintEvent(e);
}
