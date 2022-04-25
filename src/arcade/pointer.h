#ifndef POINTER_H
#define POINTER_H

#include <QQuickItem>
#include <QCursor>

class CursorShapeArea : public QQuickItem
{
	Q_OBJECT
	Q_PROPERTY(Qt::CursorShape cursorShape READ cursorShape WRITE setCursorShape NOTIFY cursorShapeChanged)

public:
	explicit CursorShapeArea(QQuickItem *parent = 0);
	Q_INVOKABLE Qt::CursorShape cursorShape() const;
	Q_INVOKABLE void setCursorShape(Qt::CursorShape cursorShape);

private:
	int m_currentShape;

signals:
	void cursorShapeChanged();
};

#endif // CURSORSHAPEAREA_H
