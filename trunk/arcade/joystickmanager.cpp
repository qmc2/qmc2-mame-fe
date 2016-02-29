#if defined(QMC2_ARCADE_ENABLE_JOYSTICK)

#include <qglobal.h>

#if QT_VERSION >= 0x050000
#include <QGuiApplication>
#include <QWindow>
#include <QTest>
#else
#include <QApplication>
#endif

#include "joystickmanager.h"
#include "arcadesettings.h"
#include "consolewindow.h"
#include "macros.h"

extern ArcadeSettings *globalConfig;
extern ConsoleWindow *consoleWindow;
extern bool debugJoy;
extern QStringList argumentList;
extern int consoleMode;

JoystickManager::JoystickManager(JoyFunctionMap *joyFuncMap, int joyIndex, QObject *parent) :
	QObject(parent)
{
	mJoyIndex = joyIndex;
	mJoystick = NULL;
	mJoyFunctionMap = joyFuncMap;

	if ( !QMC2_ARCADE_CLI_NO_JOY ) {
		if ( QMC2_ARCADE_CLI_JOY_VAL )
			openJoystick(QMC2_ARCADE_CLI_JOY);
		else
			openJoystick();
	}
}

JoystickManager::~JoystickManager()
{
	closeJoystick();
}

void JoystickManager::mapJoystickFunction(QString joystickFunction)
{
#if QT_VERSION < 0x050000
	QWidget *focusInstance = QApplication::focusWidget();
#else
	QWindow *focusInstance = QApplication::focusWindow();
#endif

	if ( focusInstance ) {
		QString mappedKeySequence = mJoyFunctionMap->mapJoyFunction(joystickFunction);
		if ( mappedKeySequence != joystickFunction ) {
			// emulate a key-event for the mapped joystick-function
			if ( debugJoy ) {
				QString debugString = "DEBUG: " + tr("joystick-function '%1' triggered - emulating event for mapped key-sequence '%2'").arg(joystickFunction).arg(mappedKeySequence);
				QMC2_ARCADE_LOG_STR(debugString);
			}
			QKeySequence emulatedKeySequence(mappedKeySequence);
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
#if QT_VERSION < 0x050000
			qApp->postEvent(focusInstance, new QKeyEvent(QKeyEvent::KeyPress, key, mods));
#else
			QTest::sendKeyEvent(QTest::Press, focusInstance, (Qt::Key) key, mappedKeySequence, mods);
#endif
		}
	}
}

void JoystickManager::axisValueChanged(int axis, int value)
{
	if ( globalConfig->joystickAxisEnabled(mJoyIndex, axis) )
		if ( value != 0 )
			mapJoystickFunction(QString("A%1%2").arg(axis).arg(value < 0 ? "-" : "+"));
}

void JoystickManager::buttonValueChanged(int button, bool value)
{
	if ( value )
		mapJoystickFunction(QString("B%1").arg(button));
}

void JoystickManager::hatValueChanged(int hat, int value)
{
	if ( value != 0 )
		mapJoystickFunction(QString("H%1:%2").arg(hat).arg(value));
}

void JoystickManager::trackballValueChanged(int trackball, int deltaX, int deltaY)
{
	mapJoystickFunction(QString("T%1:X%2,Y%3").arg(trackball).arg(deltaX < 0 ? "-" : deltaX > 0 ? "+" : "=").arg(deltaY < 0 ? "-" : deltaY > 0 ? "+" : "="));
}

void JoystickManager::openJoystick(int joystickIndex)
{
	if ( joystickIndex == -1 )
		mJoyIndex = globalConfig->joystickIndex();
	else
		mJoyIndex = joystickIndex;

	if ( !mJoystick )
		mJoystick = new Joystick(this, globalConfig->joystickEventTimeout(), globalConfig->joystickAutoRepeat(), globalConfig->joystickAutoRepeatTimeout());

	mJoystick->open(mJoyIndex);

	connect(mJoystick, SIGNAL(axisValueChanged(int,int)), this, SLOT(axisValueChanged(int,int)));
	connect(mJoystick, SIGNAL(buttonValueChanged(int,bool)), this, SLOT(buttonValueChanged(int,bool)));
	connect(mJoystick, SIGNAL(hatValueChanged(int,int)), this, SLOT(hatValueChanged(int,int)));
	connect(mJoystick, SIGNAL(trackballValueChanged(int,int,int)), this, SLOT(trackballValueChanged(int,int,int)));
}

void JoystickManager::closeJoystick()
{
	if ( mJoystick ) {
		mJoystick->disconnect();
		mJoystick->close();
		delete mJoystick;
		mJoystick = NULL;
	}
}

#endif
