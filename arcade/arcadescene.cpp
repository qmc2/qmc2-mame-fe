#include "arcade/arcadesettings.h"
#include "arcade/arcadescene.h"
#include "qmc2main.h"
#include "options.h"
#if QT_VERSION >= 0x050000
#include <QDesktopWidget>
#include <QGraphicsProxyWidget>
#endif

extern ArcadeSettings *arcadeSettings;
extern MainWindow *qmc2MainWindow;

extern bool exitArcade;
QMutex arcadeStatusMutex;

ArcadeScene::ArcadeScene(QObject *parent)
  : QGraphicsScene(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ArcadeScene::ArcadeScene(QObject *parent = %1)").arg((qulonglong) parent));
#endif

  setSceneRect(0, 0, arcadeSettings->graphicsVirtualResolution().width(), arcadeSettings->graphicsVirtualResolution().height());
  sceneClipRect = sceneRect();
  setItemIndexMethod(QGraphicsScene::NoIndex);

  scaleFactorX = width() / arcadeSettings->graphicsVirtualResolution().width();
  scaleFactorY = height() / arcadeSettings->graphicsVirtualResolution().height();
  centerX = width() / 2;
  centerY = height() / 2;

  sceneFont.fromString(arcadeSettings->miscellaneousFont());
  QFontMetrics fontMetrics(sceneFont);
  double fmHeight = scaleY(fontMetrics.height());

  fpsShown = arcadeSettings->miscellaneousShowFPS();

  fpsTextItem = new QGraphicsSimpleTextItem();
  fpsTextItem->setFont(sceneFont);
  fpsTextItem->setText(tr("FPS: --"));
  fpsTextItem->setBrush(QColor(Qt::white));
  fpsTextItem->scale(scaleFactorX, scaleFactorY);
  fpsTextItem->setPos(scaleX(2.0), height() - scaleY(2.0) - fmHeight);
  fpsTextItem->setZValue(QMC2_SCENE_LOG_Z);
  fpsTextItem->setVisible(fpsShown);
  addItem(fpsTextItem);

  fpsBackgroundItem = new QGraphicsRectItem();
  QRectF r = fpsTextItem->boundingRect();
  fpsBackgroundItem->setRect(scaleX(r.x()), scaleY(r.y()), scaleX(r.width()), scaleY(r.height()));
  fpsBackgroundItem->setPos(scaleX(2.0), height() - scaleY(2.0) - fmHeight);
  fpsBackgroundItem->setBrush(QColor(0, 0, 0, 128));
  fpsBackgroundItem->setZValue(QMC2_SCENE_LOG_Z - 1);
  fpsBackgroundItem->setVisible(fpsShown);
  addItem(fpsBackgroundItem);

  statusTextItem = new QGraphicsSimpleTextItem();
  statusTextItem->setFont(sceneFont);
  statusTextItem->setText("");
  statusTextItem->setBrush(QColor(Qt::white));
  statusTextItem->scale(scaleFactorX, scaleFactorY);
  statusTextItem->setPos((width() - scaleX(statusTextItem->boundingRect().width())) / 2, height() - scaleY(2.0) - fmHeight);
  statusTextItem->setZValue(QMC2_SCENE_LOG_Z);
  addItem(statusTextItem);

  statusBackgroundItem = new QGraphicsRectItem();
  r = statusTextItem->boundingRect();
  statusBackgroundItem->setRect(scaleX(r.x()), scaleY(r.y()), scaleX(r.width()), scaleY(r.height()));
  statusBackgroundItem->setPos((width() - scaleX(statusTextItem->boundingRect().width())) / 2, height() - scaleY(2.0) - fmHeight);
  statusBackgroundItem->setBrush(QColor(0, 0, 0, 128));
  statusBackgroundItem->setZValue(QMC2_SCENE_LOG_Z - 1);
  statusBackgroundItem->setVisible(false);
  addItem(statusBackgroundItem);

  progressBar = new QProgressBar();
  progressBarProxy = addWidget(progressBar);
  progressBarProxy->scale(scaleFactorX, scaleFactorY);
  progressBarProxy->setPos((width() - scaleX(progressBarProxy->boundingRect().width())) / 2, height() - scaleY(2.0) - fmHeight * 2);
  progressBarProxy->setVisible(false);
  progressBarProxy->setZValue(QMC2_SCENE_LOG_Z);

  connect(this, SIGNAL(changed(const QList<QRectF> &)), this, SLOT(hasChanged(const QList<QRectF> &)));
  connect(&frameCounterTimer, SIGNAL(timeout()), this, SLOT(updateFrameCounters()));
  frameCounterTimer.start(QMC2_SCENE_FRAMECOUNTER_TIMEOUT);

  consoleShown = false;
  animationPaused = false;
  frames = 0;
  frameTime.start();
}

ArcadeScene::~ArcadeScene()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ArcadeScene::~ArcadeScene()");
#endif

}

void ArcadeScene::rescaleContent()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ArcadeScene::rescaleContent()");
#endif

  if ( views()[0]->isFullScreen() ) {
    if ( arcadeSettings->graphicsUseWindowResolutionInFullScreenMode() ) {
      setSceneRect(sceneClipRect);
    } else {
      setSceneRect(0, 0, views()[0]->width(), views()[0]->height());
    }
  } else {
    setSceneRect(0, 0, views()[0]->width(), views()[0]->height());
  }

  scaleFactorX = width() / arcadeSettings->graphicsVirtualResolution().width();
  scaleFactorY = height() / arcadeSettings->graphicsVirtualResolution().height();
  centerX = width() / 2;
  centerY = height() / 2;

  QFontMetrics fontMetrics(sceneFont);
  double fmHeight = scaleY(fontMetrics.height());

  fpsTextItem->resetMatrix();
  fpsTextItem->scale(scaleFactorX, scaleFactorY);
  fpsTextItem->setPos(scaleX(2.0), height() - scaleY(2.0) - fmHeight);
  QRectF r = fpsTextItem->boundingRect();
  fpsBackgroundItem->setRect(scaleX(r.x()), scaleY(r.y()), scaleX(r.width()), scaleY(r.height()));
  fpsBackgroundItem->setPos(scaleX(2.0), height() - scaleY(2.0) - fmHeight);
  statusTextItem->resetMatrix();
  statusTextItem->scale(scaleFactorX, scaleFactorY);
  statusTextItem->setPos((width() - scaleX(statusTextItem->boundingRect().width())) / 2, height() - scaleY(2.0) - fmHeight);
  r = statusTextItem->boundingRect();
  statusBackgroundItem->setRect(scaleX(r.x()), scaleY(r.y()), scaleX(r.width()), scaleY(r.height()));
  statusBackgroundItem->setPos((width() - scaleX(statusTextItem->boundingRect().width())) / 2, height() - scaleY(2.0) - fmHeight);
  progressBarProxy->resetMatrix();
  progressBarProxy->scale(scaleFactorX, scaleFactorY);
  progressBarProxy->setPos((width() - scaleX(progressBarProxy->boundingRect().width())) / 2, height() - scaleY(2.0) - fmHeight * 2);
}

void ArcadeScene::drawBackground(QPainter *painter, const QRectF &rect)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ArcadeScene::drawBackground(QPainter *painter = %1, const QRectF &rect = ...)").arg((qulonglong) painter));
#endif

  // setup rotation angle
  painter->setWorldTransform(QTransform().translate(centerX, centerY).rotate(arcadeSettings->graphicsRotationAngle()).translate(-centerX, -centerY));

  // fill background on "window resolution in full screen" case
  if ( views()[0]->isFullScreen() ) {
    if ( arcadeSettings->graphicsUseWindowResolutionInFullScreenMode() ) {
      painter->fillRect(rect, Qt::black);
      painter->setClipRect(sceneClipRect);
    }
  }

  // ... and call the baseclass handler
  QGraphicsScene::drawBackground(painter, rect);
}

void ArcadeScene::drawForeground(QPainter *painter, const QRectF &rect)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ArcadeScene::drawForeground(QPainter *painter = %1, const QRectF &rect = ...)").arg((qulonglong) painter));
#endif

  // setup rotation angle
  painter->setWorldTransform(QTransform().translate(centerX, centerY).rotate(arcadeSettings->graphicsRotationAngle()).translate(-centerX, -centerY));

  // ... and call the baseclass handler
  QGraphicsScene::drawForeground(painter, rect);
}

void ArcadeScene::drawItems(QPainter *painter, int numItems, QGraphicsItem *items[], const QStyleOptionGraphicsItem options[], QWidget *widget)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ArcadeScene::drawItems(QPainter *painter = %1, int numItems = %2, QGraphicsItem *items[] = ..., const QStyleOptionGraphicsItem options[] = ..., QWidget *widget = %3)").arg((qulonglong) painter).arg(numItems).arg((qulonglong) widget));
#endif

  // setup rotation angle
  painter->setWorldTransform(QTransform().translate(centerX, centerY).rotate(arcadeSettings->graphicsRotationAngle()).translate(-centerX, -centerY));

  // ... and call the baseclass handler
  QGraphicsScene::drawItems(painter, numItems, items, options, widget);
}

void ArcadeScene::setStatus(QString message)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ArcadeScene::setStatus(QString message = %1)").arg(message));
#endif

  arcadeStatusMutex.lock();
  messageList.append(message);
  QFontMetrics fontMetrics(sceneFont);
  double fmHeight = scaleY(fontMetrics.height());
  statusTextItem->setText(message);
  statusTextItem->setPos((width() - scaleX(statusTextItem->boundingRect().width())) / 2, height() - scaleY(2.0) - fmHeight); 
  statusTextItem->setVisible(true);
  QRectF r = statusTextItem->boundingRect();
  statusBackgroundItem->setRect(scaleX(r.x()), scaleY(r.y()), scaleX(r.width()), scaleY(r.height()));
  statusBackgroundItem->setPos((width() - scaleX(statusTextItem->boundingRect().width())) / 2, height() - scaleY(2.0) - fmHeight);
  statusBackgroundItem->setVisible(true);
  arcadeStatusMutex.unlock();
}

void ArcadeScene::clearStatus(QString message)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ArcadeScene::clearStatus(QString message = %1)").arg(message));
#endif

  arcadeStatusMutex.lock();
  if ( message.isEmpty() )
    messageList.removeLast();
  else
    messageList.removeAll(message);

  if ( messageList.isEmpty() ) {
    statusTextItem->setVisible(false);
    statusBackgroundItem->setVisible(false);
  } else {
    QFontMetrics fontMetrics(sceneFont);
    double fmHeight = scaleY(fontMetrics.height());
    statusTextItem->setText(messageList.last());
    statusTextItem->setPos((width() - scaleX(statusTextItem->boundingRect().width())) / 2, height() - scaleY(2.0) - fmHeight); 
    statusTextItem->setVisible(true);
    QRectF r = statusTextItem->boundingRect();
    statusBackgroundItem->setRect(scaleX(r.x()), scaleY(r.y()), scaleX(r.width()), scaleY(r.height()));
    statusBackgroundItem->setPos((width() - scaleX(statusTextItem->boundingRect().width())) / 2, height() - scaleY(2.0) - fmHeight);
    statusBackgroundItem->setVisible(true);
  }
  arcadeStatusMutex.unlock();
}

void ArcadeScene::setProgress(int progress)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ArcadeScene::setProgress(int progress = %1)").arg(progress));
#endif

  if ( progress < 0 ) {
    progressBarProxy->setVisible(false);
    progressBar->reset();
  } else {
    progressBarProxy->setVisible(true);
    progressBar->setValue(progress);
  }
  qApp->processEvents();
}

void ArcadeScene::updateFrameCounters()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ArcadeScene::updateFrameCounters()");
#endif

  int msecs = frameTime.restart();
  double avgFps = (double)frames / ((double)msecs / (double)1000.0);
#if defined(Q_WS_WIN)
  fpsTextItem->setText(tr("FPS: %1").arg(floor(avgFps + 0.5)));
#else
  fpsTextItem->setText(tr("FPS: %1").arg(round(avgFps)));
#endif

  sceneFont.fromString(arcadeSettings->miscellaneousFont());
  QFontMetrics fontMetrics(sceneFont);
  double fmHeight = scaleY(fontMetrics.height());
  QRectF r = fpsTextItem->boundingRect();
  fpsBackgroundItem->setRect(scaleX(r.x()), scaleY(r.y()), scaleX(r.width()), scaleY(r.height()));
  fpsBackgroundItem->setPos(scaleX(2.0), height() - scaleY(2.0) - fmHeight);

  frames = 0;
}

void ArcadeScene::hasChanged(const QList<QRectF> &changedRegions)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ArcadeScene::hasChanged(const QList<QRectF> &changedRegions = ...)");
#endif

  frames++;
}

void ArcadeScene::toggleConsole()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ArcadeScene::toggleConsole()");
#endif

  if ( consoleShown ) {
    consoleShown = false;
  } else {
    consoleShown = true;
  }
}

void ArcadeScene::toggleAnimation()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ArcadeScene::toggleAnimation()");
#endif

  if ( animationPaused ) {
    startAnimation();
    clearStatus(tr("Paused"));
  } else {
    stopAnimation();
    setStatus(tr("Paused"));
  }
}

void ArcadeScene::toggleFps()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ArcadeScene::toggleFps()");
#endif

  if ( fpsShown )
    hideFps();
  else
    showFps();
}

void ArcadeScene::showFps()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ArcadeScene::showFps()");
#endif

  fpsShown = true;
  fpsBackgroundItem->setVisible(fpsShown);
  fpsTextItem->setVisible(fpsShown);
}

void ArcadeScene::hideFps()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ArcadeScene::hideFps()");
#endif

  fpsShown = false;
  fpsBackgroundItem->setVisible(fpsShown);
  fpsTextItem->setVisible(fpsShown);
}

void ArcadeScene::updateScene()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ArcadeScene::updateScene()");
#endif

  update();
}

void ArcadeScene::resizeScene(QSize size)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ArcadeScene::resizeScene(QSize size = %1x%2)").arg(size.width()).arg(size.height()));
#endif

  setSceneRect(0, 0, size.width(), size.height());
  rescaleContent();
}

void ArcadeScene::setupWindowedMode()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ArcadeScene::setupWindowedMode()");
#endif

  rescaleContent();
}

void ArcadeScene::setupFullscreenMode()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ArcadeScene::setupFullscreenMode()");
#endif

}
