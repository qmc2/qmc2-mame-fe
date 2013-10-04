#ifndef KEYPRESSFILTER_H
#define KEYPRESSFILTER_H

#include <QObject>

class KeyEventFilter : public QObject
{
    Q_OBJECT
public:
    explicit KeyEventFilter(QObject *parent = 0);

protected:
    bool eventFilter(QObject *, QEvent *event);
};

#endif // KEYPRESSFILTER_H
