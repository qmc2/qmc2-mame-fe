#ifndef KEYPRESSFILTER_H
#define KEYPRESSFILTER_H

#include <QObject>
#include "keysequencemap.h"

class KeyEventFilter : public QObject
{
	Q_OBJECT

public:
	explicit KeyEventFilter(KeySequenceMap *keySequenceMap, QObject *parent = 0);

protected:
	bool eventFilter(QObject *object, QEvent *event);

private:
	KeySequenceMap *mKeySequenceMap;
};

#endif // KEYPRESSFILTER_H
