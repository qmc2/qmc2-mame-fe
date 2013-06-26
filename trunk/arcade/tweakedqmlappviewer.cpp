#include <QGraphicsObject>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QApplication>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QPaintEngine>
#include <QDesktopServices>
#include <QUrl>

#include "tweakedqmlappviewer.h"
#include "arcadesettings.h"
#include "gameobject.h"
#include "consolewindow.h"
#include "macros.h"
#include "wheel.h"
#include "pointer.h"

extern ArcadeSettings *globalConfig;
extern ConsoleWindow *consoleWindow;
extern int emulatorMode;
extern int consoleMode;
extern QStringList emulatorModeNames;
extern QStringList mameThemes;
extern QStringList messThemes;
extern QStringList umeThemes;
extern QStringList consoleModes;
extern QStringList graphicsSystems;

TweakedQmlApplicationViewer::TweakedQmlApplicationViewer(QWidget *parent)
	: QmlApplicationViewer(parent)
{
    initialised = false;
    numFrames = 0;
    windowModeSwitching = false;

    cliParams << "theme" << "graphicssystem" << "console" << "language";
    switch ( emulatorMode ) {
    case QMC2_ARCADE_EMUMODE_MAME:
        cliAllowedParameterValues["theme"] = mameThemes;
        break;
    case QMC2_ARCADE_EMUMODE_MESS:
        cliAllowedParameterValues["theme"] = messThemes;
        break;
    case QMC2_ARCADE_EMUMODE_UME:
        cliAllowedParameterValues["theme"] = umeThemes;
        break;
    }
    cliAllowedParameterValues["graphicssystem"] = graphicsSystems;
    cliAllowedParameterValues["console"] = consoleModes;
    cliAllowedParameterValues["language"] = globalConfig->languageMap.keys();
    cliParameterDescriptions["theme"] = tr("Theme");
    cliParameterDescriptions["graphicssystem"] = tr("Graphics system");
    cliParameterDescriptions["console"] = tr("Console mode");
    cliParameterDescriptions["language"] = tr("Language");

    qmlRegisterType<WheelArea>("Wheel", 1, 0, "WheelArea");
    qmlRegisterType<CursorShapeArea>("Pointer", 1, 0, "CursorShapeArea");

    processManager = new ProcessManager(this);
    processManager->createTemplateList();
    connect(processManager, SIGNAL(emulatorStarted(int)), this, SIGNAL(emulatorStarted(int)));
    connect(processManager, SIGNAL(emulatorFinished(int)), this, SIGNAL(emulatorFinished(int)));

    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    imageProvider = new ImageProvider(QDeclarativeImageProvider::Image);
    engine()->addImageProvider(QLatin1String("qmc2"), imageProvider);

    infoProvider = new InfoProvider();

    engine()->addImportPath(QDir::fromNativeSeparators(XSTR(QMC2_ARCADE_QML_IMPORT_PATH)));
    rootContext()->setContextProperty("viewer", this);

    // theme-specific initialisation
    if ( globalConfig->arcadeTheme == "ToxicWaste" ) {
        loadGamelist();
    } else if ( globalConfig->arcadeTheme == "darkone" ) {
        // propagate empty gameList to QML
        rootContext()->setContextProperty("gameListModel", QVariant::fromValue(gameList));
        rootContext()->setContextProperty("gameListModelCount", gameList.count());
    }

    connect(&frameCheckTimer, SIGNAL(timeout()), this, SLOT(fpsReady()));
    frameCheckTimer.start(1000);
}

TweakedQmlApplicationViewer::~TweakedQmlApplicationViewer()
{
    if (initialised)
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
    QMC2_ARCADE_LOG_STR(tr("Loading global and theme-specific settings"));

    // load global arcade settings
    rootObject()->setProperty("version", globalConfig->applicationVersion());

    // load theme-specific arcade settings
    if ( globalConfig->arcadeTheme == "ToxicWaste" ) {
        rootObject()->setProperty("fpsVisible", globalConfig->fpsVisible());
        rootObject()->setProperty("showBackgroundAnimation", globalConfig->showBackgroundAnimation());
        rootObject()->setProperty("showShaderEffect", globalConfig->showShaderEffect());
        rootObject()->setProperty("animateInForeground", globalConfig->animateInForeground());
        rootObject()->setProperty("fullScreen", globalConfig->fullScreen());
        rootObject()->setProperty("secondaryImageType", globalConfig->secondaryImageType());
        rootObject()->setProperty("cabinetFlipped", globalConfig->cabinetFlipped());
        rootObject()->setProperty("lastIndex", globalConfig->lastIndex() < gameList.count() ? globalConfig->lastIndex() : 0);
        rootObject()->setProperty("menuHidden", globalConfig->menuHidden());
        rootObject()->setProperty("confirmQuit", globalConfig->confirmQuit());
        rootObject()->setProperty("gameCardPage", globalConfig->gameCardPage());
        rootObject()->setProperty("autoPositionOverlay", globalConfig->autoPositionOverlay());
        rootObject()->setProperty("overlayScale", std::max(0.0, std::min(10.0, globalConfig->overlayScale())));
        rootObject()->setProperty("overlayOffsetX", globalConfig->overlayOffsetX());
        rootObject()->setProperty("overlayOffsetY", globalConfig->overlayOffsetY());
        rootObject()->setProperty("overlayOpacity", globalConfig->overlayOpacity());
        rootObject()->setProperty("backgroundOpacity", globalConfig->backgroundOpacity());
        rootObject()->setProperty("gameListOpacity", globalConfig->gameListOpacity());
    } else if ( globalConfig->arcadeTheme == "darkone" ) {
        rootObject()->setProperty("lastIndex", globalConfig->lastIndex());
        rootObject()->setProperty("dataTypePrimary", globalConfig->dataTypePrimary());
        rootObject()->setProperty("dataTypeSecondary", globalConfig->dataTypeSecondary());
        rootObject()->setProperty("fullScreen", globalConfig->fullScreen());
        rootObject()->setProperty("listHidden", globalConfig->listHidden());
        rootObject()->setProperty("toolbarHidden", globalConfig->toolbarHidden());
        rootObject()->setProperty("fpsVisible", globalConfig->fpsVisible());
        rootObject()->setProperty("sortByName", globalConfig->sortByName());
        rootObject()->setProperty("backLight", globalConfig->backLight());
        rootObject()->setProperty("toolbarAutoHide", globalConfig->toolbarAutoHide());
        rootObject()->setProperty("launchFlash", globalConfig->launchFlash());
        rootObject()->setProperty("launchZoom", globalConfig->launchZoom());
        rootObject()->setProperty("overlayScale", std::max(0.33, globalConfig->overlayScale()));
        rootObject()->setProperty("lightTimeout", std::max(5.0, globalConfig->lightTimeout()));
        rootObject()->setProperty("colourScheme", globalConfig->colourScheme());
    }
    initialised = true;
}

void TweakedQmlApplicationViewer::saveSettings()
{
    QMC2_ARCADE_LOG_STR(tr("Saving global and theme-specific settings"));

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
        globalConfig->setShowShaderEffect(rootObject()->property("showShaderEffect").toBool());
        globalConfig->setAnimateInForeground(rootObject()->property("animateInForeground").toBool());
        globalConfig->setFullScreen(rootObject()->property("fullScreen").toBool());
        globalConfig->setSecondaryImageType(rootObject()->property("secondaryImageType").toString());
        globalConfig->setCabinetFlipped(rootObject()->property("cabinetFlipped").toBool());
        globalConfig->setLastIndex(rootObject()->property("lastIndex").toInt());
        globalConfig->setMenuHidden(rootObject()->property("menuHidden").toBool());
        globalConfig->setConfirmQuit(rootObject()->property("confirmQuit").toBool());
        globalConfig->setGameCardPage(rootObject()->property("gameCardPage").toInt());
        globalConfig->setAutoPositionOverlay(rootObject()->property("autoPositionOverlay").toBool());
        globalConfig->setOverlayScale(rootObject()->property("overlayScale").toDouble());
        globalConfig->setOverlayOffsetX(rootObject()->property("overlayOffsetX").toDouble());
        globalConfig->setOverlayOffsetY(rootObject()->property("overlayOffsetY").toDouble());
        globalConfig->setOverlayOpacity(rootObject()->property("overlayOpacity").toDouble());
        globalConfig->setBackgroundOpacity(rootObject()->property("backgroundOpacity").toDouble());
        globalConfig->setGameListOpacity(rootObject()->property("gameListOpacity").toDouble());
    } else if ( globalConfig->arcadeTheme == "darkone" ) {
        globalConfig->setLastIndex(rootObject()->property("lastIndex").toInt());
        globalConfig->setDataTypePrimary(rootObject()->property("dataTypePrimary").toString());
        globalConfig->setDataTypeSecondary(rootObject()->property("dataTypeSecondary").toString());
        globalConfig->setToolbarHidden(rootObject()->property("toolbarHidden").toBool());
        globalConfig->setListHidden(rootObject()->property("listHidden").toBool());
        globalConfig->setFullScreen(rootObject()->property("fullScreen").toBool());
        globalConfig->setFpsVisible(rootObject()->property("fpsVisible").toBool());
        globalConfig->setSortByName(rootObject()->property("sortByName").toBool());
        globalConfig->setBackLight(rootObject()->property("backLight").toBool());
        globalConfig->setToolbarAutoHide(rootObject()->property("toolbarAutoHide").toBool());
        globalConfig->setLaunchFlash(rootObject()->property("launchFlash").toBool());
        globalConfig->setLaunchZoom(rootObject()->property("launchZoom").toBool());
        globalConfig->setOverlayScale(rootObject()->property("overlayScale").toDouble());
        globalConfig->setLightTimeout(rootObject()->property("lightTimeout").toDouble());
        globalConfig->setColourScheme(rootObject()->property("colourScheme").toString());
    }
}

void TweakedQmlApplicationViewer::goFullScreen()
{
    showFullScreen();
    raise();
    qApp->processEvents();
    windowModeSwitching = false;
}

void TweakedQmlApplicationViewer::switchToFullScreen(bool initially)
{
    if ( windowModeSwitching )
        return;
    windowModeSwitching = true;
    QMC2_ARCADE_LOG_STR(tr("Activating full-screen display"));
    if ( initially ) {
        savedGeometry = globalConfig->viewerGeometry();
        savedMaximized = globalConfig->viewerMaximized();
    } else {
        savedGeometry = saveGeometry();
        savedMaximized = isMaximized();
    }
#if defined(QMC2_ARCADE_OS_UNIX)
    hide();
    qApp->processEvents();
    QTimer::singleShot(100, this, SLOT(goFullScreen()));
#else
    showFullScreen();
    windowModeSwitching = false;
#endif
}

void TweakedQmlApplicationViewer::switchToWindowed(bool initially)
{
    if ( windowModeSwitching )
        return;
    windowModeSwitching = true;
    QMC2_ARCADE_LOG_STR(tr("Activating windowed display"));
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
    raise();
    windowModeSwitching = false;
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
    QString gameListCachePath;
    gameList.clear();

    if ( globalConfig->useFilteredList() ) {
        gameListCachePath = QFileInfo(globalConfig->filteredListFile()).absoluteFilePath();
        if ( !QFileInfo(gameListCachePath).exists() || !QFileInfo(gameListCachePath).isReadable() ) {
            QMC2_ARCADE_LOG_STR(tr("WARNING: filtered list file '%1' doesn't exist or isn't accessible, falling back to the full %2").
                                arg(gameListCachePath).
                                arg(emulatorMode != QMC2_ARCADE_EMUMODE_MESS ? tr("game list") : tr("machine list")));
            gameListCachePath = QFileInfo(globalConfig->gameListCacheFile()).absoluteFilePath();
        } 
    } else
        gameListCachePath = QFileInfo(globalConfig->gameListCacheFile()).absoluteFilePath();

    QMap<QString, char> rscMap;

    QMC2_ARCADE_LOG_STR(tr("Loading %1 from '%2'").
                        arg(emulatorMode != QMC2_ARCADE_EMUMODE_MESS ? tr("game list") : tr("machine list")).
                        arg(QDir::toNativeSeparators(gameListCachePath)));

    QString romStateCachePath = QFileInfo(globalConfig->romStateCacheFile()).absoluteFilePath();
    QFile romStateCache(romStateCachePath);
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
            QMC2_ARCADE_LOG_STR(tr("WARNING: Can't open ROM state cache file '%1', please check permissions").
                         arg(QDir::toNativeSeparators(romStateCachePath)));
    } else
        QMC2_ARCADE_LOG_STR(tr("WARNING: The ROM state cache file '%1' doesn't exist, please run main front-end executable to create it").
                     arg(QDir::toNativeSeparators(romStateCachePath)));

    QFile gameListCache(gameListCachePath);
    if ( gameListCache.exists() ) {
        if ( gameListCache.open(QIODevice::ReadOnly | QIODevice::Text) ) {
            QTextStream tsGameListCache(&gameListCache);
            tsGameListCache.readLine();
            tsGameListCache.readLine();
            while ( !tsGameListCache.atEnd() ) {
                QStringList words = tsGameListCache.readLine().split("\t");
                if ( words[QMC2_ARCADE_GLC_DEVICE] != "1" ) {
                    QString gameId = words[QMC2_ARCADE_GLC_ID];
                    gameList.append(new GameObject(gameId, words[QMC2_ARCADE_GLC_DESCRIPTION], romStateCharToInt(rscMap[gameId])));
                }
            }
        } else
            QMC2_ARCADE_LOG_STR(tr("FATAL: Can't open %1 cache file '%2', please check permissions").
                         arg(emulatorMode != QMC2_ARCADE_EMUMODE_MESS ? tr("game list") : tr("machine list")).
                         arg(QDir::toNativeSeparators(gameListCachePath)));
    } else
        QMC2_ARCADE_LOG_STR(tr("FATAL: The %1 cache file '%2' doesn't exist, please run main front-end executable to create it").
                     arg(emulatorMode != QMC2_ARCADE_EMUMODE_MESS ? tr("game list") : tr("machine list")).
                     arg(QDir::toNativeSeparators(gameListCachePath)));

    if ( globalConfig->sortByName() )
        qSort(gameList.begin(), gameList.end(), GameObject::lessThan);

    // propagate gameList to QML
    rootContext()->setContextProperty("gameListModel", QVariant::fromValue(gameList));
    rootContext()->setContextProperty("gameListModelCount", gameList.count());

    QMC2_ARCADE_LOG_STR(QString(tr("Done (loading %1 from '%2')").
                         arg(emulatorMode != QMC2_ARCADE_EMUMODE_MESS ? tr("game list") : tr("machine list")) + " - " + tr("%n non-device set(s) loaded", "", gameList.count())).
                         arg(QDir::toNativeSeparators(gameListCachePath)));
}

void TweakedQmlApplicationViewer::launchEmulator(QString id)
{
    QMC2_ARCADE_LOG_STR(tr("Starting emulator #%1 for %2 ID '%3'").arg(processManager->highestProcessID()).arg(emulatorMode != QMC2_ARCADE_EMUMODE_MESS ? tr("game") : tr("machine")).arg(id));
    processManager->startEmulator(id);
}

QString TweakedQmlApplicationViewer::loadImage(const QString &id)
{
    return imageProvider->loadImage(id);
}

QString TweakedQmlApplicationViewer::requestInfo(const QString &id, const QString &infoClass)
{
    QString info("");
    if (infoClass == "gameinfo")
        info = infoProvider->requestInfo(id, InfoProvider::InfoClassGame);
    else if (infoClass == "emuinfo")
        info = infoProvider->requestInfo(id, InfoProvider::InfoClassEmu);
#ifdef QMC2_DEBUG
    else
        info = QString("DEBUG: TweakedQmlApplicationViewer::requestInfo() unsupported info class '%1'").arg(infoClass);
#endif

    return info;
}

int TweakedQmlApplicationViewer::findIndex(QString pattern, int startIndex)
{
    if ( pattern.isEmpty() )
        return startIndex;

    int foundIndex = startIndex;
    bool indexFound = false;

    QRegExp wildcard(pattern, Qt::CaseInsensitive, QRegExp::Wildcard);
    QRegExp regexp(pattern, Qt::CaseInsensitive, QRegExp::RegExp);

    for (int i = startIndex + 1; i < gameList.count() && !indexFound; i++) {
        QString description = ((GameObject *)gameList[i])->description();
        QString id = ((GameObject *)gameList[i])->id();
        if ( description.indexOf(wildcard, 0) >= 0 || id.indexOf(wildcard, 0) >= 0 ) {
            foundIndex = i;
            indexFound = true;
        } else if ( regexp.indexIn(description, 0) >= 0 || regexp.indexIn(id, 0) >= 0 ) {
            foundIndex = i;
            indexFound = true;
        }
    }

    for (int i = 0; i < startIndex && !indexFound; i++) {
        QString description = ((GameObject *)gameList[i])->description();
        QString id = ((GameObject *)gameList[i])->id();
        if ( description.indexOf(wildcard, 0) >= 0 || id.indexOf(wildcard, 0) >= 0 ) {
            foundIndex = i;
            indexFound = true;
        } else if ( regexp.indexIn(description, 0) >= 0 || regexp.indexIn(id, 0) >= 0 ) {
            foundIndex = i;
            indexFound = true;
        }
    }

    return foundIndex;
}

void TweakedQmlApplicationViewer::log(QString message)
{
    QMC2_ARCADE_LOG_STR(message);
}

QStringList TweakedQmlApplicationViewer::cliParamNames()
{
    return cliAllowedParameterValues.keys();
}

QString TweakedQmlApplicationViewer::cliParamDescription(QString param)
{
    return cliParameterDescriptions[param];
}

QString TweakedQmlApplicationViewer::cliParamValue(QString param)
{
    switch ( cliParams.indexOf(param) ) {
    case QMC2_ARCADE_PARAM_THEME:
        return globalConfig->defaultTheme();
    case QMC2_ARCADE_PARAM_GRASYS:
        return globalConfig->defaultGraphicsSystem();
    case QMC2_ARCADE_PARAM_CONSOLE:
        return globalConfig->defaultConsoleType();
    case QMC2_ARCADE_PARAM_LANGUAGE:
        return globalConfig->defaultLanguage();
    default:
        return QString();
    }
}

QStringList TweakedQmlApplicationViewer::cliParamAllowedValues(QString param)
{
    return cliAllowedParameterValues[param];
}

void TweakedQmlApplicationViewer::setCliParamValue(QString param, QString value)
{
    switch ( cliParams.indexOf(param) ) {
    case QMC2_ARCADE_PARAM_THEME:
        globalConfig->setDefaultTheme(value);
        break;
    case QMC2_ARCADE_PARAM_GRASYS:
        globalConfig->setDefaultGraphicsSystem(value);
        break;
    case QMC2_ARCADE_PARAM_CONSOLE:
        globalConfig->setDefaultConsoleType(value);
        break;
    case QMC2_ARCADE_PARAM_LANGUAGE:
        globalConfig->setDefaultLanguage(value);
        break;
    }
}

void TweakedQmlApplicationViewer::linkActivated(QString link)
{
    QDesktopServices::openUrl(QUrl::fromUserInput(link));
}

void TweakedQmlApplicationViewer::paintEvent(QPaintEvent *e)
{
    numFrames++;
    QmlApplicationViewer::paintEvent(e);
}

void TweakedQmlApplicationViewer::closeEvent(QCloseEvent *e)
{
    QMC2_ARCADE_LOG_STR(tr("Stopping QML viewer"));

    if ( consoleWindow ) {
        QString consoleMessage(tr("QML viewer stopped - please close the console window to exit"));
        QMC2_ARCADE_LOG_STR(QString("-").repeated(consoleMessage.length()));
        QMC2_ARCADE_LOG_STR(consoleMessage);
        QMC2_ARCADE_LOG_STR(QString("-").repeated(consoleMessage.length()));
        consoleWindow->showNormal();
        consoleWindow->raise();
    }
    e->accept();
}
