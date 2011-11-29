#include "arcade/arcadeview.h"
#include "arcade/arcadesettings.h"
#include "arcade/arcademenuscene.h"
#include "qmc2main.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;
extern ArcadeSettings *arcadeSettings;

ArcadeMenuScene::ArcadeMenuScene(QObject *parent)
  : ArcadeScene(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ArcadeMenuScene::ArcadeMenuScene(QObject *parent = %1)").arg((qulonglong) parent));
#endif

  QPixmap backgroundPixmap(QString::fromUtf8(":/data/img/arcadebackground.png"));
  setBackgroundBrush(backgroundPixmap.scaled((int)width(), (int)height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

  foregroundImageItem = new QGraphicsPixmapItem();
  QPixmap foregroundPixmap(QString::fromUtf8(":/data/img/arcadeforeground.png"));
  foregroundImageItem->setPixmap(foregroundPixmap.scaled((int)width(), (int)height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
  foregroundImageItem->setPos(0, 0);
  foregroundImageItem->setZValue(QMC2_SCENE_FOREGROUND_Z);
  addItem(foregroundImageItem);

  centerX = width() / 2;
  centerY = height() / 2;

  connect(&animationTimer, SIGNAL(timeout()), this, SLOT(animationStep()));
}

ArcadeMenuScene::~ArcadeMenuScene()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ArcadeMenuScene::~ArcadeMenuScene()");
#endif

}

void ArcadeMenuScene::rescaleContent()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ArcadeMenuScene::rescaleContent()");
#endif

  // first call baseclass' rescaleContent() (also sets scaling factors)
  ArcadeScene::rescaleContent();

  QPixmap backgroundPixmap(QString::fromUtf8(":/data/img/arcadebackground.png"));
  setBackgroundBrush(backgroundPixmap.scaled((int)width(), (int)height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

  QPixmap foregroundPixmap(QString::fromUtf8(":/data/img/arcadeforeground.png"));
  foregroundImageItem->setPixmap(foregroundPixmap.scaled((int)width(), (int)height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
  foregroundImageItem->setPos(0, 0);

  centerX = width() / 2;
  centerY = height() / 2;
}

void ArcadeMenuScene::drawBackground(QPainter *painter, const QRectF &rect)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ArcadeMenuScene::drawBackground(QPainter *painter = %1, const QRectF &rect = ...)").arg((qulonglong) painter));
#endif

  // for now, just call the baseclass handler
  ArcadeScene::drawBackground(painter, rect);
}

void ArcadeMenuScene::drawForeground(QPainter *painter, const QRectF &rect)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ArcadeMenuScene::drawForeground(QPainter *painter = %1, const QRectF &rect = ...)").arg((qulonglong) painter));
#endif

  // for now, just call the baseclass handler
  ArcadeScene::drawForeground(painter, rect);
}

void ArcadeMenuScene::drawItems(QPainter *painter, int numItems, QGraphicsItem *items[], const QStyleOptionGraphicsItem options[], QWidget *widget)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ArcadeMenuScene::drawItems(QPainter *painter = %1, int numItems = %2, QGraphicsItem *items[] = ..., const QStyleOptionGraphicsItem options[] = ..., QWidget *widget = %3)").arg((qulonglong) painter).arg(numItems).arg((qulonglong) widget));
#endif

  // for now, just call the baseclass handler
  ArcadeScene::drawItems(painter, numItems, items, options, widget);
}

void ArcadeMenuScene::animationStep()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ArcadeMenuScene::animationStep()");
#endif

}

void ArcadeMenuScene::startAnimation()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ArcadeMenuScene::startAnimation()");
#endif

  animationTimer.start(QMC2_MENUSCENE_ANIMATION_TIMEOUT);
  animationPaused = false;
}

void ArcadeMenuScene::stopAnimation()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ArcadeMenuScene::stopAnimation()");
#endif

  animationTimer.stop();
  animationPaused = true;
}
