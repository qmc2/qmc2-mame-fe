#include "pointer.h"

#if QT_VERSION < 0x050000
CursorShapeArea::CursorShapeArea(QDeclarativeItem *parent)
	: QDeclarativeItem(parent), m_currentShape(-1)
{
	// NOP
}

Qt::CursorShape CursorShapeArea::cursorShape() const
{
	return cursor().shape();
}

void CursorShapeArea::setCursorShape(Qt::CursorShape cursorShape)
{
	if ( m_currentShape == (int) cursorShape )
		return;

	setCursor(cursorShape);
	emit cursorShapeChanged();
	m_currentShape = cursorShape;
}
#else
CursorShapeArea::CursorShapeArea(QQuickItem *parent)
	: QQuickItem(parent), m_currentShape(-1)
{
	// NOP
}

Qt::CursorShape CursorShapeArea::cursorShape() const
{
	return cursor().shape();
}

void CursorShapeArea::setCursorShape(Qt::CursorShape cursorShape)
{
	if ( m_currentShape == (int) cursorShape )
		return;

	setCursor(cursorShape);
	emit cursorShapeChanged();
	m_currentShape = cursorShape;
}
#endif
