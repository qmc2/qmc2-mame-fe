#include "arcade/arcadeview.h"
#include "arcade/arcademenuitem.h"
#include "qmc2main.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;

ArcadeMenuItem::ArcadeMenuItem(QGraphicsItem *parent)
  : ArcadeItem(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ArcadeMenuItem::ArcadeMenuItem(QGraphicsItem *parent = %1)").arg((qulonglong) parent));
#endif

}

ArcadeMenuItem::~ArcadeMenuItem()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ArcadeMenuItem::~ArcadeMenuItem()");
#endif

}

void ArcadeMenuItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ArcadeMenuItem::paint(QPainter *painter = %1, const QStyleOptionGraphicsItem *option = %2, QWidget *widget = %3)").arg((qulonglong) painter).arg((qulonglong) option).arg((qulonglong) widget));
#endif

}
