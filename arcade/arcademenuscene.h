#ifndef _ARCADEMENUSCENE_H_
#define _ARCADEMENUSCENE_H_

#include "arcade/arcadescene.h"

#define QMC2_MENUSCENE_ANIMATION_TIMEOUT	10

class ArcadeMenuScene : public ArcadeScene
{
  Q_OBJECT

  public:
    QGraphicsPixmapItem *foregroundImageItem;
    QTimer animationTimer;

    ArcadeMenuScene(QObject *parent = 0);
    ~ArcadeMenuScene();

    void rescaleContent();

  public slots:
    void animationStep();
    void startAnimation();
    void stopAnimation();

  protected:
    virtual void drawBackground(QPainter *, const QRectF &);
    virtual void drawForeground(QPainter *, const QRectF &);
    virtual void drawItems(QPainter *, int, QGraphicsItem *[], const QStyleOptionGraphicsItem [], QWidget *);
};

#endif
