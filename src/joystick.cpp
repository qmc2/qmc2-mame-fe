#if QMC2_JOYSTICK == 1

#include <QRegExp>

#include "settings.h"
#include "joystick.h"
#include "qmc2main.h"
#include "macros.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;
extern bool qmc2JoystickIsCalibrating;

Joystick::Joystick(QObject *parent, int joystickEventTimeout, bool doAutoRepeat, int repeatDelay) :
	QObject(parent)
{
	if ( SDL_Init(SDL_INIT_JOYSTICK) == 0 ) {
		QRegExp rx("(\\b.*\\b)\\1");
		for (int i = 0; i < SDL_NumJoysticks(); i++) {
#if SDL_MAJOR_VERSION == 1
			QString jsName(SDL_JoystickName(i));
#elif SDL_MAJOR_VERSION == 2
			SDL_Joystick *js = SDL_JoystickOpen(i);
			QString jsName(SDL_JoystickName(js));
			SDL_JoystickClose(js);
#endif
			jsName.replace(rx, "\\1"); // remove consecutive duplicate words in the joystick name (i. e. "Logitech Logitech Extreme 3D" becomes "Logitech Extreme 3D")
			joystickNames.append(jsName);
		}
		connect(&joystickTimer, SIGNAL(timeout()), this, SLOT(processEvents()));
	}
	joystick = 0;
	jsIndex = -1;
	numAxes = numButtons = numHats = numTrackballs = 0;
	autoRepeat = doAutoRepeat;
	autoRepeatDelay = repeatDelay;
	eventTimeout = joystickEventTimeout;
}

Joystick::~Joystick()
{
	if ( isOpen() )
		close();
	SDL_Quit();
}

bool Joystick::open(int stick)
{
	static bool firstOpenCall = true;
	bool doLog = !firstOpenCall;
	firstOpenCall = false;
	if ( isOpen() )
		close();
	joystick = SDL_JoystickOpen(stick);
	if ( joystick ) {
		numAxes = SDL_JoystickNumAxes(joystick);
		numButtons = SDL_JoystickNumButtons(joystick);
		numHats = SDL_JoystickNumHats(joystick);
		numTrackballs = SDL_JoystickNumBalls(joystick);
		if ( doLog )
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("SDL joystick #%1 opened: name = %2, axes = %3, buttons = %4, hats = %5, trackballs = %6")
								  .arg(stick).arg(joystickNames[stick]).arg(numAxes).arg(numButtons).arg(numHats).arg(numTrackballs));
		joystickTimer.start(eventTimeout);
		jsIndex = stick;
		return true;
	} else {
		jsIndex = -1;
		return false;
	}
}

void Joystick::close()
{
	static bool firstCloseCall = true;
	bool doLog = !firstCloseCall;
	firstCloseCall = false;
	joystickTimer.stop();
	if ( joystick ) {
		SDL_JoystickClose(joystick);
		if ( doLog )
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("SDL joystick #%1 closed").arg(jsIndex));
	}
	joystick = 0;
	jsIndex = -1;
	numAxes = numButtons = numHats = numTrackballs = 0;
}

void Joystick::processEvents()
{
	if ( !isOpen() )
		return;
	SDL_JoystickUpdate();
	for (int i = 0; i < numAxes; i++) {
		Sint16 moved = SDL_JoystickGetAxis(joystick, i);
		if ( !qmc2JoystickIsCalibrating )
			moved = normalizeAxisValue(moved, i);
		if ( abs(moved) >= deadzones[i] ) {
			if ( (moved != axes[i]) ) {
				int deltaMoved = abs(axes[i] - moved);
				if ( deltaMoved >= sensitivities[i] )
					emit axisValueChanged(i, moved);
				axes[i] = moved;
				axisRepeatTimers[i].restart();
			} else if (autoRepeat && moved != 0) {
				if ( axisRepeatTimers[i].elapsed() >= autoRepeatDelay ) {
					emit axisValueChanged(i, moved);
					axes[i] = moved;
				}
			} else
				axisRepeatTimers[i].restart();
		} else
			emit axisValueChanged(i, 0);
	}
	for (int i = 0; i < numButtons; i++) {
		Uint8 changed = SDL_JoystickGetButton(joystick, i);
		if ( (changed != buttons[i]) ) {
			emit buttonValueChanged(i, (bool) changed);
			buttons[i] = changed;
			buttonRepeatTimers[i].restart();
		} else if (autoRepeat && changed != 0) {
			if ( buttonRepeatTimers[i].elapsed() >= autoRepeatDelay ) {
				emit buttonValueChanged(i, (bool) changed);
				buttons[i] = changed;
			}
		} else
			buttonRepeatTimers[i].restart();
	}
	for (int i = 0; i < numHats; i++) {
		Uint8 changed = SDL_JoystickGetHat(joystick, i);
		if ( (changed != hats[i]) ) {
			emit hatValueChanged(i, changed);
			hats[i] = changed;
			hatRepeatTimers[i].restart();
		} else if (autoRepeat && changed != 0) {
			if ( hatRepeatTimers[i].elapsed() >= autoRepeatDelay ) {
				emit hatValueChanged(i, changed);
				hats[i] = changed;
			}
		} else
			hatRepeatTimers[i].restart();
	}
	for (int i = 0; i < numTrackballs; i++) {
		int dx, dy;
		SDL_JoystickGetBall(joystick, i, &dx, &dy);
		if ( dx != 0 || dy != 0 )
			emit trackballValueChanged(i, dx, dy);
	}
}

int Joystick::getAxisValue(int axis)
{
	if ( isOpen() ) {
		SDL_JoystickUpdate();
		if ( !qmc2JoystickIsCalibrating )
			return normalizeAxisValue(SDL_JoystickGetAxis(joystick, axis), axis);
		else
			return SDL_JoystickGetAxis(joystick, axis);
	} else
		return 0;
}

Sint16 Joystick::normalizeAxisValue(Sint16 rawValue, int axis)
{
	int max = qmc2Config->value(QString(QMC2_FRONTEND_PREFIX + "Joystick/%1/Axis%2Maximum").arg(jsIndex).arg(axis), 32767).toInt();
	int min = qmc2Config->value(QString(QMC2_FRONTEND_PREFIX + "Joystick/%1/Axis%2Minimum").arg(jsIndex).arg(axis), -32768).toInt();
	if ( max > min ) {
		rawValue -= (max + min) / 2;
		return 65535.0 * (double)rawValue / (double)(max - min);
	} else
		return 0;
}

#endif
