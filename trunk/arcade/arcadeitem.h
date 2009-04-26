#ifndef _ARCADEITEM_H_
#define _ARCADEITEM_H_

#include <QtGui>

class ArcadeItem : public QObject, public QGraphicsItem
{
  Q_OBJECT

  public:
    bool animationPaused;
    double scaleFactorX;
    double scaleFactorY;

    ArcadeItem(QGraphicsItem *parent = 0);
    ~ArcadeItem();

  public slots:
    virtual void startAnimation() { animationPaused = FALSE; }
    virtual void stopAnimation() { animationPaused = TRUE; }
    virtual void toggleAnimation();
    virtual void setScale(double x = 1.0, double y = 1.0) { scaleFactorX = x; scaleFactorY = y; }

  protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option = 0, QWidget *widget = 0);
};

#endif
