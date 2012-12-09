#include <QGraphicsObject>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QApplication>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>

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
        rootObject()->setProperty("secondaryImageType", globalConfig->secondaryImageType());
        rootObject()->setProperty("cabinetFlipped", globalConfig->cabinetFlipped());
        rootObject()->setProperty("lastIndex", globalConfig->lastIndex());
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
        globalConfig->setSecondaryImageType(rootObject()->property("secondaryImageType").toString());
        globalConfig->setCabinetFlipped(rootObject()->property("cabinetFlipped").toBool());
        globalConfig->setLastIndex(rootObject()->property("lastIndex").toInt());
    }
}

void TweakedQmlApplicationViewer::switchToFullScreen(bool initially)
{
    QMC2_LOG_STR(tr("Activating full-screen display"));
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
    QMC2_LOG_STR(tr("Activating windowed display"));
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

int TweakedQmlApplicationViewer::romStateCharToInt(char status)
{
    switch ( status ) {
    case 'C':
        return QMC2_ARCADE_ROMSTATE_C;
    case 'M':
        return QMC2_ARCADE_ROMSTATE_M;
    case 'I':
        return QMC2_ARCADE_ROMSTATE_I;
    case 'N':
        return QMC2_ARCADE_ROMSTATE_N;
    case 'U':
    default:
        return QMC2_ARCADE_ROMSTATE_U;
    }
}

void TweakedQmlApplicationViewer::loadGamelist()
{
    QMC2_LOG_STR(tr("Loading and filtering %1").arg(emulatorMode != QMC2_ARCADE_EMUMODE_MESS ? tr("game list") : tr("machine list")));

    QMap<QString, char> rscMap;
    QFile romStateCache(globalConfig->romStateCacheFile());
    if ( romStateCache.exists() ) {
        if ( romStateCache.open(QIODevice::ReadOnly | QIODevice::Text) ) {
            QTextStream tsRomCache(&romStateCache);
            while ( !tsRomCache.atEnd() ) {
                QString line = tsRomCache.readLine();
                if ( !line.isEmpty() && !line.startsWith("#") ) {
                    QStringList words = line.split(" ");
                    rscMap[words[0]] = words[1].at(0).toLatin1();
                }
            }
        } else
            QMC2_LOG_STR(tr("WARNING: Can't open ROM state cache file '%1', please check permissions").arg(globalConfig->romStateCacheFile()));
    } else
        QMC2_LOG_STR(tr("WARNING: The ROM state cache file '%1' doesn't exist, please run main front-end executable to create it").arg(globalConfig->romStateCacheFile()));

    QFile gameListCache(globalConfig->gameListCacheFile());
    if ( gameListCache.exists() ) {
        if ( gameListCache.open(QIODevice::ReadOnly | QIODevice::Text) ) {
            QTextStream tsGameListCache(&gameListCache);
            tsGameListCache.readLine();
            tsGameListCache.readLine();
            // FIXME: add sorting and filtering based on settings made in QMC2!
            while ( !tsGameListCache.atEnd() ) {
                QStringList words = tsGameListCache.readLine().split("\t");
                if ( words[QMC2_ARCADE_GLC_DEVICE] != "1" ) {
                    QString gameId = words[QMC2_ARCADE_GLC_ID];
                    gameList.append(new GameObject(gameId, words[QMC2_ARCADE_GLC_DESCRIPTION], romStateCharToInt(rscMap[gameId])));
                }
            }
        } else
            QMC2_LOG_STR(tr("FATAL: Can't open %1 cache file '%2', please check permissions").arg(emulatorMode != QMC2_ARCADE_EMUMODE_MESS ? tr("game list") : tr("machine list")).arg(globalConfig->gameListCacheFile()));
    } else
        QMC2_LOG_STR(tr("FATAL: The %1 cache file '%2' doesn't exist, please run main front-end executable to create it").arg(emulatorMode != QMC2_ARCADE_EMUMODE_MESS ? tr("game list") : tr("machine list")).arg(globalConfig->gameListCacheFile()));

    // propagate gameList to QML
    rootContext()->setContextProperty("gameListModel", QVariant::fromValue(gameList));
    rootContext()->setContextProperty("gameListModelCount", gameList.count());

    QMC2_LOG_STR(QString(tr("Done (loading and filtering %1)").arg(emulatorMode != QMC2_ARCADE_EMUMODE_MESS ? tr("game list") : tr("machine list")) + " - " + tr("%n non-device set(s) loaded", "", gameList.count())));
}

void TweakedQmlApplicationViewer::launchEmulator(QString id)
{
    QMC2_LOG_STR(tr("Launching emulator for %1 ID '%2'").arg(emulatorMode != QMC2_ARCADE_EMUMODE_MESS ? tr("game") : tr("machine")).arg(id));

    // FIXME
}

int TweakedQmlApplicationViewer::findIndex(QString pattern, int startIndex)
{
    if ( pattern.isEmpty() )
        return startIndex;

    int foundIndex = startIndex;
    bool indexFound = false;

    for (int i = startIndex + 1; i < gameList.count() && !indexFound; i++) {
        QString description = ((GameObject *)gameList[i])->description();
        QString id = ((GameObject *)gameList[i])->id();
        if ( description.indexOf(pattern, 0, Qt::CaseInsensitive) >= 0 || id.indexOf(pattern, 0, Qt::CaseInsensitive) >= 0 ) {
            foundIndex = i;
            indexFound = true;
        }
    }

    for (int i = 0; i < startIndex && !indexFound; i++) {
        QString description = ((GameObject *)gameList[i])->description();
        QString id = ((GameObject *)gameList[i])->id();
        if ( description.indexOf(pattern, 0, Qt::CaseInsensitive) >= 0 || id.indexOf(pattern, 0, Qt::CaseInsensitive) >= 0 ) {
            foundIndex = i;
            indexFound = true;
        }
    }

    return foundIndex;
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
