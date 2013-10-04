#include <QKeySequence>
#include <QKeyEvent>

#include "keyeventfilter.h"
#include "arcadesettings.h"
#include "consolewindow.h"
#include "macros.h"

extern ArcadeSettings *globalConfig;
extern ConsoleWindow *consoleWindow;

KeyEventFilter::KeyEventFilter(QObject *parent) :
    QObject(parent)
{
}

bool KeyEventFilter::eventFilter(QObject *, QEvent *event)
{
    if ( event->spontaneous() && (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) ) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        int myKeySeq = 0;
        if ( keyEvent->modifiers() & Qt::ShiftModifier )
            myKeySeq += Qt::SHIFT;
        if ( keyEvent->modifiers() & Qt::ControlModifier )
            myKeySeq += Qt::CTRL;
        if ( keyEvent->modifiers() & Qt::AltModifier )
            myKeySeq += Qt::ALT;
        if ( keyEvent->modifiers() & Qt::MetaModifier )
            myKeySeq += Qt::META;
        myKeySeq += keyEvent->key();
        QString pressedKeySeq = QKeySequence(myKeySeq).toString();
        //QMC2_ARCADE_LOG_STR(QString("DEBUG: Key-sequence '%1' %2").arg(pressedKeySeq).arg(event->type() == QEvent::KeyPress ? "pressed" : "released"));
        //return true;
    }
    return false; // default key processing
}
