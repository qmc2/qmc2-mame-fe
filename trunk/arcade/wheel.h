#ifndef WHEELAREA_H
#define WHEELAREA_H

#include <qglobal.h>

#if QT_VERSION < 0x050000 // we use the standard MouseArea's wheel events for Qt version >= 5.0.0

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

#endif

#endif
