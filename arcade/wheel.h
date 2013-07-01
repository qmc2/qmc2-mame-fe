#ifndef WHEELAREA_H
#define WHEELAREA_H

#include <Qt>

#if QT_VERSION < 0x050000
#include <QDeclarativeItem>
#include <QGraphicsSceneWheelEvent>

class WheelArea : public QDeclarativeItem
{
    Q_OBJECT

public:
    explicit WheelArea(QDeclarativeItem *parent = 0) : QDeclarativeItem(parent) {}

protected:
    void wheelEvent(QGraphicsSceneWheelEvent *event)
    {
        emit wheel(event->delta());        
    }

signals:
    void wheel(int delta);
};
#else
#include <QQuickItem>
#include <QGraphicsSceneWheelEvent>

class WheelArea : public QQuickItem
{
    Q_OBJECT

public:
    explicit WheelArea(QQuickItem *parent = 0) : QQuickItem(parent) {}

protected:
    void wheelEvent(QGraphicsSceneWheelEvent *event)
    {
        emit wheel(event->delta());
    }

signals:
    void wheel(int delta);
};
#endif

#endif
