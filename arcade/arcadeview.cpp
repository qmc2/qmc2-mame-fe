#include <QApplication>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QSettings>
#include <QLocale>
#include <QtDebug>
#include <QMutex>
#ifdef Q_WS_X11
#include <QX11Info>  
#endif
#if defined(QT_NO_OPENGL)
#undef QMC2_ARCADE_OPENGL
#define QMC2_ARCADE_OPENGL	0
#endif
#if QMC2_ARCADE_OPENGL == 1
#include <QGLWidget>
#endif
#include "arcade/arcadeview.h"
#include "arcade/arcadescene.h"
#include "arcade/arcadesettings.h"
#include "arcade/arcadesetupdialog.h"
#include "qmc2main.h"

// extern global variables
extern MainWindow *qmc2MainWindow;
extern QTreeWidgetItem *qmc2CurrentItem;
extern ArcadeSetupDialog *qmc2ArcadeSetupDialog;
extern QSettings *qmc2Config;
extern bool qmc2CleaningUp;

QWaitCondition arcadeScreenshotSaverWaitCondition;
ArcadeSettings *arcadeSettings = NULL;
ArcadeView *arcadeView = NULL;
QMutex arcadeScreenshotMutex;
QImage *arcadeScreenshotImage = NULL;
QQueue<QString> arcadeMessageQueue;
bool exitArcade = false;

ArcadeView::ArcadeView(QWidget *parent)
  : QGraphicsView(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ArcadeView::ArcadeView(QWidget *parent = %1)").arg((qulonglong)parent));
#endif

  arcadeScreenshotSaverThread = NULL;

  arcadeView = this;
  arcadeSettings = new ArcadeSettings(this);

  // window title
  setWindowTitle(tr("QMC2 - ArcadeView"));

  // replace some odd QGraphicsView default settings
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setFrameStyle(QFrame::NoFrame);
  setCacheMode(QGraphicsView::CacheBackground);
  setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);

  // graphics init
  setupGraphicsMode();

  // scene init
  setInteractive(true);
  menuScene = new ArcadeMenuScene(this);
  setScene(menuScene);

  // connections
  connect(this, SIGNAL(resized(QSize)), this, SLOT(rescale(QSize)));
  reconnectSceneSignals();

  // switch to fullscreen if applicable
  if ( arcadeSettings->graphicsFullscreen() )
    QTimer::singleShot(0, this, SLOT(toggleFullscreen()));

  // delayed init
  QTimer::singleShot(0, this, SLOT(delayedInit()));
}

ArcadeView::~ArcadeView()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ArcadeView::~ArcadeView()");
#endif

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: Cleaning up"));

  if ( arcadeScreenshotSaverThread )
    delete arcadeScreenshotSaverThread;
  if ( menuScene )
    delete menuScene;
  if ( arcadeSettings )
    delete arcadeSettings;
}

void ArcadeView::reconnectSceneSignals()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ArcadeView::reconnectSceneSignals()");
#endif

  ArcadeScene *currentScene = (ArcadeScene *)scene();
  disconnect(currentScene);
  connect(this, SIGNAL(viewportAdjusted(QSize)), currentScene, SLOT(resizeScene(QSize)));
  connect(this, SIGNAL(switchedToFullscreenMode()), currentScene, SLOT(setupFullscreenMode()));
  connect(this, SIGNAL(switchedToWindowedMode()), currentScene, SLOT(setupWindowedMode()));
}

void ArcadeView::toggleFullscreen()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ArcadeView::toggleFullscreen()");
#endif

  ((ArcadeScene *)scene())->sceneClipRect = QRectF(0, 0, width(), height());

  if ( isFullScreen() ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: Switching to windowed mode"));
    showNormal();
    emit switchedToWindowedMode();
    qmc2Config->setValue(QMC2_ARCADE_PREFIX + "Graphics/Fullscreen", false);
    if ( qmc2ArcadeSetupDialog )
      qmc2ArcadeSetupDialog->checkBoxGraphicsFullscreen->setChecked(false);
  } else {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: Switching to full screen mode"));
    if ( arcadeSettings->graphicsSwitchResolution() ) {
      // <<< FIXME: find available resolutions and switch to nearest fit >>>
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: Resolution switching is not yet supported"));
    }
    showFullScreen();
    emit switchedToFullscreenMode();
    qmc2Config->setValue(QMC2_ARCADE_PREFIX + "Graphics/Fullscreen", true);
    if ( qmc2ArcadeSetupDialog )
      qmc2ArcadeSetupDialog->checkBoxGraphicsFullscreen->setChecked(true);
  }
}

void ArcadeView::setupWindowSize(QSize size)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ArcadeView::setupWindowSize(QSize size = %1x%2)").arg(size.width()).arg(size.height()));
#endif

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: Setting window size to %1x%2").arg(size.width()).arg(size.height()));
  resize(size);
}

void ArcadeView::setupWindowPosition(QPoint point)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ArcadeView::setupWindowPosition(QPoint point = %1, %2)").arg(point.x()).arg(point.y()));
#endif

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: Setting window position to %1, %2").arg(point.x()).arg(point.y()));
  move(point);
}

void ArcadeView::setupGraphicsMode()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ArcadeView::setupGraphicsMode()");
#endif

#if QMC2_ARCADE_OPENGL == 1
  if ( arcadeSettings->graphicsUseOpenGL() ) {
    // check for OpenGL support
    if ( !QGLFormat::hasOpenGL() ) {
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: This system does not appear to support OpenGL -- reverting to non-OpenGL / software renderer"));
      arcadeSettings->setGraphicsUseOpenGL(false);
    }
  }
  if ( arcadeSettings->graphicsUseOpenGL() ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: Using OpenGL renderer"));
    QGLWidget *widget = new QGLWidget();

    if ( widget->format().swapInterval() == -1 && arcadeSettings->openGLSyncToScreen() ) {
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: This system does not appear to support vertical syncing -- disabling SyncToScreen"));
      arcadeSettings->setOpenGLSyncToScreen(false);
    }
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: OpenGL: SyncToScreen: %1").arg(arcadeSettings->openGLSyncToScreen() ? tr("on") : tr("off")));
    widget->format().setSwapInterval(arcadeSettings->openGLSyncToScreen() ? 1 : 0);
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: OpenGL: DoubleBuffer: %1").arg(arcadeSettings->openGLDoubleBuffer() ? tr("on") : tr("off")));
    widget->format().setDoubleBuffer(arcadeSettings->openGLDoubleBuffer());
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: OpenGL: DepthBuffer: %1").arg(arcadeSettings->openGLDepthBuffer() ? tr("on") : tr("off")));
    widget->format().setDepth(arcadeSettings->openGLDepthBuffer());
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: OpenGL: RGBA: %1").arg(arcadeSettings->openGLRGBA() ? tr("on") : tr("off")));
    widget->format().setRgba(arcadeSettings->openGLRGBA());
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: OpenGL: AlphaChannel: %1").arg(arcadeSettings->openGLAlphaChannel() ? tr("on") : tr("off")));
    widget->format().setAlpha(arcadeSettings->openGLAlphaChannel());
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: OpenGL: AccumulatorBuffer: %1").arg(arcadeSettings->openGLAccumulatorBuffer() ? tr("on") : tr("off")));
    widget->format().setAccum(arcadeSettings->openGLAccumulatorBuffer());
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: OpenGL: StencilBuffer: %1").arg(arcadeSettings->openGLStencilBuffer() ? tr("on") : tr("off")));
    widget->format().setStencil(arcadeSettings->openGLStencilBuffer());
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: OpenGL: Stereo: %1").arg(arcadeSettings->openGLStereo() ? tr("on") : tr("off")));
    widget->format().setStereo(arcadeSettings->openGLStereo());
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: OpenGL: DirectRendering: %1").arg(arcadeSettings->openGLDirectRendering() ? tr("on") : tr("off")));
    widget->format().setDirectRendering(arcadeSettings->openGLDirectRendering());
    if ( !QGLFormat::hasOpenGLOverlays() && arcadeSettings->openGLOverlay() ) {
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: This system does not appear to support OpenGL overlays -- disabling OpenGL overlays"));
      arcadeSettings->setOpenGLOverlay(false);
    }
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: OpenGL: Overlay: %1").arg(arcadeSettings->openGLOverlay() ? tr("on") : tr("off")));
    widget->format().setOverlay(arcadeSettings->openGLOverlay());
    if ( widget->format().samples() == -1 && arcadeSettings->openGLMultiSample() ) {
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: This system does not appear to support OpenGL multi sampling -- disabling MultiSample"));
      arcadeSettings->setOpenGLMultiSample(false);
    }
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: OpenGL: MultiSample: %1").arg(arcadeSettings->openGLMultiSample() ? tr("on") : tr("off")));
    widget->format().setSampleBuffers(arcadeSettings->openGLMultiSample());
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: OpenGL: AntiAliasing: %1").arg(arcadeSettings->openGLAntiAliasing() ? tr("on") : tr("off")));
    if ( arcadeSettings->openGLAntiAliasing() )
      setRenderHint(QPainter::HighQualityAntialiasing);

    widget->setAutoFillBackground(false);
    setViewport(widget);
    setCacheMode(QGraphicsView::CacheNone);
  } else
#endif
  {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: Using software renderer"));
  }

#ifdef Q_WS_X11
  int myScreen = QX11Info::appScreen();
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: X11: Screen number: %1").arg(myScreen));
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: X11: Color depth: %1").arg(QX11Info::appDepth(myScreen)));
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: X11: DPI-X: %1").arg(QX11Info::appDpiX(myScreen)));
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: X11: DPI-Y: %1").arg(QX11Info::appDpiY(myScreen)));
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: X11: Compositing manager: %1").arg(QX11Info::isCompositingManagerRunning() ? tr("running") : tr("not running")));
#endif

  QRect screenGeom = QApplication::desktop()->screenGeometry();
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: Screen geometry: %1x%2").arg(screenGeom.width()).arg(screenGeom.height()));
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: Virtual resolution: %1x%2").arg(arcadeSettings->graphicsVirtualResolution().width()).arg(arcadeSettings->graphicsVirtualResolution().height()));
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: Selected aspect ratio: %1:%2").arg(arcadeSettings->graphicsAspectRatio().width()).arg(arcadeSettings->graphicsAspectRatio().height()));
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: Scene rotation angle: %1 degrees").arg(arcadeSettings->graphicsRotationAngle(), 0, 'f', 2));
  if ( (double)arcadeSettings->graphicsVirtualResolution().width() / (double)arcadeSettings->graphicsVirtualResolution().height() !=
       (double)arcadeSettings->graphicsAspectRatio().width() / (double)arcadeSettings->graphicsAspectRatio().height())
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: Virtual resolution doesn't fit aspect ratio -- scene coordinates may be stretched or compressed"));
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, arcadeSettings->graphicsKeepAspect() ? tr("ArcadeView: Aspect ratio will be maintained") : tr("ArcadeView: Aspect ratio will not be maintained"));
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: FPS counter display %1").arg(arcadeSettings->miscellaneousShowFPS() ? tr("activated") : tr("deactivated")));
  if ( arcadeSettings->graphicsPrimitiveAntiAliasing() )
    setRenderHint(QPainter::Antialiasing);
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: Primitive antialiasing %1").arg(arcadeSettings->graphicsPrimitiveAntiAliasing() ? tr("activated") : tr("deactivated")));

  setupWindowSize(arcadeSettings->graphicsWindowSize());
  if ( arcadeSettings->graphicsCenterOnScreen() ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: Centering window on screen"));
    QPoint centerPos((screenGeom.width() - width()) / 2, (screenGeom.height() - height()) / 2);
    setupWindowPosition(centerPos);
  } else {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: Restoring saved window position"));
    setupWindowPosition(arcadeSettings->graphicsWindowPosition());
  }
}

void ArcadeView::keyPressEvent(QKeyEvent *event)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ArcadeView::keyPressEvent(QKeyEvent *event = %1)").arg((qulonglong)event));
#endif

  QKeyEvent *emulatedEvent = new QKeyEvent(event->type(), event->key(), event->modifiers());
  qApp->postEvent(qmc2MainWindow->treeWidgetGamelist, emulatedEvent);
  qApp->processEvents();
  if ( qmc2MainWindow->stackedWidgetView->currentIndex() == QMC2_VIEWHIERARCHY_INDEX ) {
    if ( qmc2CurrentItem )
      qmc2CurrentItem->setExpanded(false);
    qmc2MainWindow->scrollToCurrentItem();
  }
}

void ArcadeView::takeScreenshot()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ArcadeView::takeScreenshot()");
#endif

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: Rendering screen shot"));

  arcadeScreenshotMutex.lock();
  arcadeScreenshotImage = new QImage(size(), QImage::Format_ARGB32);
  QPainter screenshotPainter(arcadeScreenshotImage);
  render(&screenshotPainter);
  screenshotPainter.end();
  arcadeScreenshotMutex.unlock();

  ((ArcadeScene *)scene())->setStatus(tr("Saving screen shot"));
  arcadeScreenshotSaverWaitCondition.wakeAll();
}

void ArcadeView::sizeAdjust()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ArcadeView::sizeAdjust()");
#endif

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ArcadeView: Adjusting window size to %1x%2 to maintain the aspect ratio").arg(sizeAdjusted.width()).arg(sizeAdjusted.height()));
  setGeometry(geometry().x(), geometry().y(), sizeAdjusted.width(), sizeAdjusted.height());
}

void ArcadeView::rescale(QSize size)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ArcadeView::rescale(QSize size = %1x%2)").arg(size.width()).arg(size.height()));
#endif

  if ( arcadeSettings->graphicsKeepAspect() ) {
    QSize newSize(arcadeSettings->graphicsAspectRatio());
    if ( size.width() < sizeAdjusted.width() || size.height() < sizeAdjusted.height() ) {
      newSize.scale(size, Qt::KeepAspectRatio);
    } else {
      newSize.scale(size, Qt::KeepAspectRatioByExpanding);
    }
    if ( newSize != size ) {
      sizeAdjusted = newSize;
      QTimer::singleShot(QMC2_MAIN_SIZEADJUST_TIMEOUT, this, SLOT(sizeAdjust()));
      return;
    }
  }

  sizeAdjusted = size;
  emit viewportAdjusted(sizeAdjusted);
}

void ArcadeView::delayedInit()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ArcadeView::delayedInit()");
#endif
 
  // create and start the screen shot saver thread
  arcadeScreenshotSaverThread = new ArcadeScreenshotSaverThread();
  connect(arcadeScreenshotSaverThread, SIGNAL(log(const QString &)), this, SLOT(logMsg(const QString &)));
}

void ArcadeView::logMsg(const QString &msg)
{
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, msg);
}

void ArcadeView::resizeEvent(QResizeEvent *event)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ArcadeView::resizeEvent(QResizeEvent *event = %1)").arg((qulonglong)event));
#endif

  if ( !isFullScreen() && isVisible() ) {
    if ( qmc2ArcadeSetupDialog ) {
      qmc2ArcadeSetupDialog->spinBoxGraphicsWindowWidth->setValue(width());
      qmc2ArcadeSetupDialog->spinBoxGraphicsWindowHeight->setValue(height());
    }
    if ( !qmc2CleaningUp )
      qmc2Config->setValue(QMC2_ARCADE_PREFIX + "Graphics/WindowSize", size());
  }

  emit resized(event->size());
}

void ArcadeView::moveEvent(QMoveEvent *event)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ArcadeView::moveEvent(QMoveEvent *event = %1)").arg((qulonglong)event));
#endif

  // first call the baseclass handler
  QGraphicsView::moveEvent(event);

  qApp->processEvents();

  if ( !isFullScreen() && isVisible() ) {
    if ( qmc2ArcadeSetupDialog ) {
      qmc2ArcadeSetupDialog->spinBoxGraphicsWindowPosX->setValue(x());
      qmc2ArcadeSetupDialog->spinBoxGraphicsWindowPosY->setValue(y());
    }
    if ( !qmc2CleaningUp )
      qmc2Config->setValue(QMC2_ARCADE_PREFIX + "Graphics/WindowPosition", pos());
  }
}

void ArcadeView::showEvent(QShowEvent *event)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ArcadeView::showEvent(QShowEvent *event = %1)").arg((qulonglong)event));
#endif

  static bool firstCall = true;

  exitArcade = false;

  if ( arcadeScreenshotSaverThread && !firstCall )
    arcadeScreenshotSaverThread->start();

  if ( !isFullScreen() ) {
    if ( qmc2ArcadeSetupDialog )
    {
      qmc2ArcadeSetupDialog->spinBoxGraphicsWindowPosX->setValue(x());
      qmc2ArcadeSetupDialog->spinBoxGraphicsWindowPosY->setValue(y());
      qmc2ArcadeSetupDialog->spinBoxGraphicsWindowWidth->setValue(width());
      qmc2ArcadeSetupDialog->spinBoxGraphicsWindowHeight->setValue(height());
    }

    if ( !qmc2CleaningUp ) {
      qmc2Config->setValue(QMC2_ARCADE_PREFIX + "Graphics/WindowSize", size());
      qmc2Config->setValue(QMC2_ARCADE_PREFIX + "Graphics/WindowPosition", pos());
    }
  }

  firstCall = false;

  // call the baseclass handler
  QGraphicsView::showEvent(event);
}

void ArcadeView::closeEvent(QCloseEvent *event)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ArcadeView::closeEvent(QCloseEvent *event = %1)").arg((qulonglong)event));
#endif

  exitArcade = true;
  arcadeScreenshotSaverWaitCondition.wakeAll();
  arcadeScreenshotSaverThread->quit();
  arcadeScreenshotSaverThread->wait();

  // call the baseclass handler
  if ( event != NULL )
    QGraphicsView::closeEvent(event);
  qmc2MainWindow->actionArcadeToggle->setChecked(false);
}

void ArcadeView::hideEvent(QHideEvent *event)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ArcadeView::hideEvent(QHideEvent *event = %1)").arg((qulonglong)event));
#endif

  // for now, just call the baseclass handler
  QGraphicsView::hideEvent(event);
}
