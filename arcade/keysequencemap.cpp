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
    if ( mKeySequenceMap.contains(nativeKeySeq) )
        return mKeySequenceMap[nativeKeySeq];
    else
        return nativeKeySeq;
}
