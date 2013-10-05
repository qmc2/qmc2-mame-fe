#include <QApplication>
#include <QKeySequence>
#include <QKeyEvent>

#include "keyeventfilter.h"
#include "arcadesettings.h"
#include "consolewindow.h"
#include "macros.h"

extern ArcadeSettings *globalConfig;
extern ConsoleWindow *consoleWindow;

KeyEventFilter::KeyEventFilter(KeySequenceMap *keySequenceMap, QObject *parent) :
    QObject(parent)
{
    mKeySequenceMap = keySequenceMap;
}

bool KeyEventFilter::eventFilter(QObject *object, QEvent *event)
{
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    if ( event->spontaneous() && (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) ) {
        // 'native' key-event
        int keySeq = 0;
        if ( keyEvent->modifiers() & Qt::ShiftModifier )
            keySeq += Qt::SHIFT;
        if ( keyEvent->modifiers() & Qt::ControlModifier )
            keySeq += Qt::CTRL;
        if ( keyEvent->modifiers() & Qt::AltModifier )
            keySeq += Qt::ALT;
        if ( keyEvent->modifiers() & Qt::MetaModifier )
            keySeq += Qt::META;
        keySeq += keyEvent->key();
        QString nativeKeySeq = QString(QKeySequence(keySeq).toString().toLatin1());
        QString mappedKeySeq = mKeySequenceMap->mapKeySequence(nativeKeySeq);
        if ( mappedKeySeq != nativeKeySeq ) {
            // emulate a key-event for the mapped key
            QMC2_ARCADE_LOG_STR(QString("DEBUG: key-sequence '%1' %2 - emulating event for mapped key-sequence '%3'").arg(nativeKeySeq).arg(event->type() == QEvent::KeyPress ? "pressed" : "released").arg(mappedKeySeq));
            QKeySequence emulatedKeySequence(mappedKeySeq);
            Qt::KeyboardModifiers mods = Qt::NoModifier;
            int key = emulatedKeySequence[0] | emulatedKeySequence[1] | emulatedKeySequence[2] | emulatedKeySequence[3];
            if ( key & Qt::ShiftModifier ) {
              key -= Qt::ShiftModifier;
              mods |= Qt::ShiftModifier;
            }
            if ( key & Qt::ControlModifier ) {
              key -= Qt::ControlModifier;
              mods |= Qt::ControlModifier;
            }
            if ( key & Qt::AltModifier ) {
              key -= Qt::AltModifier;
              mods |= Qt::AltModifier;
            }
            if ( key & Qt::MetaModifier ) {
              key -= Qt::MetaModifier;
              mods |= Qt::MetaModifier;
            }
            QKeyEvent *emulatedKeyEvent = new QKeyEvent(event->type(), key, mods, QString("QMC2_ARCADE_EMULATED_KEY_EVENT"));
            qApp->postEvent(object, emulatedKeyEvent);
            // no further event processing
            return true;
        } else
            // default event processing
            QMC2_ARCADE_LOG_STR(QString("DEBUG: key-sequence '%1' %2 - default event processing").arg(nativeKeySeq).arg(event->type() == QEvent::KeyPress ? "pressed" : "released"));
            return QObject::eventFilter(object, event);
    } else
        // default event processing
        return QObject::eventFilter(object, event);
}
