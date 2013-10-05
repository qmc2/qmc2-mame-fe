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
    // FIXME: loads nothing
}

QString KeySequenceMap::mapKeySequence(QString nativeKeySeq)
{
    // FIXME: maps nothing
    return nativeKeySeq;
}
