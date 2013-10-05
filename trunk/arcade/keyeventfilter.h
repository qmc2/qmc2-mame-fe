#ifndef KEYPRESSFILTER_H
#define KEYPRESSFILTER_H

#include <QObject>
#include "keysequencemap.h"

class KeyEventFilter : public QObject
{
    Q_OBJECT

public:
    explicit KeyEventFilter(KeySequenceMap * keyMap, QObject *parent = 0);

protected:
    bool eventFilter(QObject *, QEvent *event);

private:
    KeySequenceMap *mKeyMap;
};

#endif // KEYPRESSFILTER_H
