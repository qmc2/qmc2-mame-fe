#include <QStringList>

#include "keysequencemap.h"

KeySequenceMap::KeySequenceMap(QStringList keySequences, QObject *parent) :
    QObject(parent)
{
    mNativeKeySequences = keySequences;
    loadKeySequenceMap();
}

void KeySequenceMap::loadKeySequenceMap()
{
    // FIXME
}

QString KeySequenceMap::mapKeySequence(QString nativeKeySeq)
{
    // FIXME
    return QString();
}
