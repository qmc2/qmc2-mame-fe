#include <qglobal.h>

#if QT_VERSION < 0x050000
#include <QApplication>
#else
#include <QGuiApplication>
#endif
#include <QKeySequence>
#include <QKeyEvent>

#include "keyeventfilter.h"
#include "arcadesettings.h"
#include "tweakedqmlappviewer.h"
#include "macros.h"

extern ArcadeSettings *globalConfig;
extern bool debugKeys;

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
		QString pressedKeySeq = QString(QKeySequence(keySeq).toString().toLatin1());
		QString mappedKeySeq = mKeySequenceMap->mapKeySequence(pressedKeySeq);
		if ( mappedKeySeq != pressedKeySeq ) {
			// emulate a key-event for the mapped key
			if ( debugKeys ) {
				QString debugString = "DEBUG: " + tr("key-sequence '%1' %2 - emulating event for mapped key-sequence '%3'").arg(pressedKeySeq).arg(event->type() == QEvent::KeyPress ? tr("pressed") : tr("released")).arg(mappedKeySeq);
				QMC2_ARCADE_LOG_STR(debugString);
			}
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
			qApp->postEvent(object, new QKeyEvent(event->type(), key, mods));
			// no further event processing
			return true;
		} else {
			// default event processing
			if ( debugKeys ) {
				QString debugString = "DEBUG: " + tr("key-sequence '%1' %2 - default event processing").arg(pressedKeySeq).arg(event->type() == QEvent::KeyPress ? tr("pressed") : tr("released"));
				QMC2_ARCADE_LOG_STR(debugString);
			}
			return QObject::eventFilter(object, event);
		}
	} else {
		// default event processing
		return QObject::eventFilter(object, event);
	}
}
