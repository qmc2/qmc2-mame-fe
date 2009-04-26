#include "arcade/arcadeview.h"
#include "arcade/arcadeitem.h"
#include "qmc2main.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;

ArcadeItem::ArcadeItem(QGraphicsItem *parent)
  : QGraphicsItem(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ArcadeItem::ArcadeItem(QGraphicsItem *parent = %1)").arg((qulonglong) parent));
#endif

  scaleFactorX = scaleFactorY = 1.0;
}

ArcadeItem::~ArcadeItem()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ArcadeItem::~ArcadeItem()");
#endif

}

void ArcadeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ArcadeItem::paint(QPainter *painter = %1, const QStyleOptionGraphicsItem *option = %2, QWidget *widget = %3)").arg((qulonglong) painter).arg((qulonglong) option).arg((qulonglong) widget));
#endif

}

void ArcadeItem::toggleAnimation()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ArcadeItem::toggleAnimation()");
#endif

  if ( animationPaused )
    startAnimation();
  else
    stopAnimation();
}
