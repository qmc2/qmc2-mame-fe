#include "keysequencemap.h"
#include "arcadesettings.h"
#include "consolewindow.h"
#include "macros.h"

extern ArcadeSettings *globalConfig;
extern ConsoleWindow *consoleWindow;

KeySequenceMap::KeySequenceMap(QStringList keySequences, QObject *parent) :
    QObject(parent)
{
    mNativeKeySequences = keySequences;
    loadKeySequenceMap();
}

void KeySequenceMap::loadKeySequenceMap()
{
    globalConfig->beginGroup(globalConfig->keySequenceMapBaseKey());
    foreach (QString key, globalConfig->childKeys()) {
        if ( mNativeKeySequences.contains(key) )
            mKeySequenceMap[globalConfig->value(key, key).toString()] = key;
        else
            globalConfig->remove(key);
    }
    globalConfig->endGroup();
}

QString KeySequenceMap::mapKeySequence(QString nativeKeySeq)
{
    if ( mKeySequenceMap.contains(nativeKeySeq) )
        return mKeySequenceMap[nativeKeySeq];
    else
        return nativeKeySeq;
}
