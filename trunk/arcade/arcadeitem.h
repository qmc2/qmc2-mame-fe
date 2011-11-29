#ifndef _ARCADEITEM_H_
#define _ARCADEITEM_H_

#include <QtGui>

class ArcadeItem : public QObject, public QGraphicsItem
{
  Q_OBJECT
#if QT_VERSION >= 0x040600
  Q_INTERFACES(QGraphicsItem)
#endif

  public:
    bool animationPaused;
    double scaleFactorX;
    double scaleFactorY;

    ArcadeItem(QGraphicsItem *parent = 0);
    ~ArcadeItem();

  public slots:
    virtual void startAnimation() { animationPaused = false; }
    virtual void stopAnimation() { animationPaused = true; }
    virtual void toggleAnimation();
    virtual void setScale(double x = 1.0, double y = 1.0) { scaleFactorX = x; scaleFactorY = y; }

  protected:
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option = 0, QWidget *widget = 0);
};

#endif
