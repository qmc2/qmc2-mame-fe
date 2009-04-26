#ifndef _ARCADEMENUITEM_H_
#define _ARCADEMENUITEM_H_

#include "arcade/arcadeitem.h"

class ArcadeMenuItem : public ArcadeItem
{
  Q_OBJECT

  public:
    ArcadeMenuItem(QGraphicsItem *parent = 0);
    ~ArcadeMenuItem();

  protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option = 0, QWidget *widget = 0);
};

#endif
