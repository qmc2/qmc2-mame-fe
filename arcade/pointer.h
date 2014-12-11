#ifndef CURSORSHAPEAREA_H
#define CURSORSHAPEAREA_H

#include <qglobal.h>

#if QT_VERSION < 0x050000
#include <QDeclarativeItem>

class CursorShapeArea : public QDeclarativeItem
{
    Q_OBJECT
    Q_PROPERTY(Qt::CursorShape cursorShape READ cursorShape WRITE setCursorShape NOTIFY cursorShapeChanged)

public:
    explicit CursorShapeArea(QDeclarativeItem *parent = 0);
    Q_INVOKABLE Qt::CursorShape cursorShape() const;
    Q_INVOKABLE void setCursorShape(Qt::CursorShape cursorShape);

private:
    int m_currentShape;

signals:
    void cursorShapeChanged();
};
#else
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
#endif

#endif // CURSORSHAPEAREA_H
