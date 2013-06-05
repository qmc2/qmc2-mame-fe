#ifndef WHEELAREA_H
#define WHEELAREA_H

#include <QDeclarativeItem>
#include <QGraphicsSceneWheelEvent>

class WheelArea : public QDeclarativeItem
{
    Q_OBJECT
public:
    explicit WheelArea(QDeclarativeItem *parent = 0) : QDeclarativeItem(parent) {}

protected:
    void wheelEvent(QGraphicsSceneWheelEvent *event) {
        emit wheel(event->delta());        
    }
signals:
//    void wheel(QGraphicsSceneWheelEvent *event);
    void wheel(int delta);
};

#endif
