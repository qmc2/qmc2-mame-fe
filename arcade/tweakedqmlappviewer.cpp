#include <QGraphicsObject>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QApplication>

#include "tweakedqmlappviewer.h"
#include "imageprovider.h"
#include "arcadesettings.h"
#include "gameobject.h"
#include "consolewindow.h"
#include "macros.h"

extern ArcadeSettings *globalConfig;
extern ConsoleWindow *consoleWindow;
extern int emulatorMode;
extern int consoleMode;
extern QStringList emulatorModeNames;

TweakedQmlApplicationViewer::TweakedQmlApplicationViewer(QWidget *parent)
	: QmlApplicationViewer(parent)
{
    numFrames = 0;

    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    engine()->addImageProvider(QLatin1String("qmc2"), new ImageProvider(QDeclarativeImageProvider::Image));

    // this gives access to the viewer object from JavaScript
    rootContext()->setContextProperty("viewer", this);

    loadGamelist();

    connect(&frameCheckTimer, SIGNAL(timeout()), this, SLOT(fpsReady()));
    frameCheckTimer.start(1000);
}

TweakedQmlApplicationViewer::~TweakedQmlApplicationViewer()
{
    saveSettings();
}

void TweakedQmlApplicationViewer::fpsReady()
{
    if ( rootObject() )
        rootObject()->setProperty("fps", numFrames);
    numFrames = 0;
}

void TweakedQmlApplicationViewer::loadSettings()
{
    QMC2_LOG_STR(tr("Loading global and theme-specific settings"));

    // load global arcade settings
    rootObject()->setProperty("version", globalConfig->applicationVersion());

    // load theme-specific arcade settings
    if ( globalConfig->arcadeTheme == "ToxicWaste" ) {
        rootObject()->setProperty("fpsVisible", globalConfig->fpsVisible());
        rootObject()->setProperty("showBackgroundAnimation", globalConfig->showBackgroundAnimation());
        rootObject()->setProperty("fullScreen", globalConfig->fullScreen());
    }

    QMC2_LOG_STR(tr("Ready to launch %1").arg(emulatorMode != QMC2_ARCADE_EMUMODE_MESS ? tr("games") : tr("machines")));
}

void TweakedQmlApplicationViewer::saveSettings()
{
    QMC2_LOG_STR(tr("Saving global and theme-specific settings"));

    // save global arcade settings
    if ( isFullScreen() ) {
        globalConfig->setViewerGeometry(savedGeometry);
        globalConfig->setViewerMaximized(savedMaximized);
    } else {
        globalConfig->setViewerGeometry(saveGeometry());
        globalConfig->setViewerMaximized(isMaximized());
    }

    // save theme-specific arcade settings
    if ( globalConfig->arcadeTheme == "ToxicWaste" ) {
        globalConfig->setFpsVisible(rootObject()->property("fpsVisible").toBool());
        globalConfig->setShowBackgroundAnimation(rootObject()->property("showBackgroundAnimation").toBool());
        globalConfig->setFullScreen(rootObject()->property("fullScreen").toBool());
    }
}

void TweakedQmlApplicationViewer::switchToFullScreen(bool initially)
{
    QMC2_LOG_STR(tr("Activating full-screen mode"));
    if ( initially ) {
        savedGeometry = globalConfig->viewerGeometry();
        savedMaximized = globalConfig->viewerMaximized();
    } else {
        savedGeometry = saveGeometry();
        savedMaximized = isMaximized();
    }
    showFullScreen();
    grabKeyboard();
    raise();
}

void TweakedQmlApplicationViewer::switchToWindowed(bool initially)
{
    QMC2_LOG_STR(tr("Activating windowed mode"));
    if ( initially ) {
        savedGeometry = globalConfig->viewerGeometry();
        savedMaximized = globalConfig->viewerMaximized();
    }
#if defined(QMC2_ARCADE_OS_UNIX)
    hide();
#endif
    restoreGeometry(savedGeometry);
    if ( savedMaximized )
        showMaximized();
    else
        showNormal();
    grabKeyboard();
    raise();
}

QString TweakedQmlApplicationViewer::romStateText(int status)
{
    switch ( status ) {
    case QMC2_ARCADE_ROMSTATE_C:
        return tr("correct");
    case QMC2_ARCADE_ROMSTATE_M:
        return tr("mostly correct");
    case QMC2_ARCADE_ROMSTATE_I:
        return tr("incorrect");
    case QMC2_ARCADE_ROMSTATE_N:
        return tr("not found");
    case QMC2_ARCADE_ROMSTATE_U:
    default:
        return tr("unknown");
    }
}

void TweakedQmlApplicationViewer::loadGamelist()
{
    QMC2_LOG_STR(tr("Loading and filtering %1").arg(emulatorMode != QMC2_ARCADE_EMUMODE_MESS ? tr("game list") : tr("machine list")));

    // FIXME: this is only test-data...
    for (int i = 0; i < 500; i++)
        gameList.append(new GameObject(QString("%1").arg(i + 1), QString("Item %1").arg(i + 1), rand() % 5));

    // propagate gameList to QML
    rootContext()->setContextProperty("gameListModel", QVariant::fromValue(gameList));
    rootContext()->setContextProperty("gameListModelCount", gameList.count());

    QMC2_LOG_STR(QString(tr("Done (loading and filtering %1)").arg(emulatorMode != QMC2_ARCADE_EMUMODE_MESS ? tr("game list") : tr("machine list")) + " - " + tr("%n set(s) loaded", "", gameList.count())));
}

void TweakedQmlApplicationViewer::paintEvent(QPaintEvent *e)
{
    numFrames++;
    QmlApplicationViewer::paintEvent(e);
}

void TweakedQmlApplicationViewer::closeEvent(QCloseEvent *e)
{
    QMC2_LOG_STR(tr("Stopping QML viewer"));

    if ( consoleWindow ) {
        QString consoleMessage(tr("QML viewer stopped - please close the console window to exit"));
        QMC2_LOG_STR(QString("-").repeated(consoleMessage.length()));
        QMC2_LOG_STR(consoleMessage);
        QMC2_LOG_STR(QString("-").repeated(consoleMessage.length()));
        consoleWindow->showNormal();
        consoleWindow->raise();
    }
    e->accept();
}
