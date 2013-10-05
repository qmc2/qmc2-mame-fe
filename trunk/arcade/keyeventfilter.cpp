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

bool KeyEventFilter::eventFilter(QObject *, QEvent *event)
{
    if ( event->spontaneous() && (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) ) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
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
        QString pressedKeySeq = QString(QKeySequence(keySeq).toString().toLatin1());
        QMC2_ARCADE_LOG_STR(QString("DEBUG: Key-sequence '%1' %2").arg(pressedKeySeq).arg(event->type() == QEvent::KeyPress ? "pressed" : "released"));
        //return true;
    }

    // default key processing
    return false;
}
