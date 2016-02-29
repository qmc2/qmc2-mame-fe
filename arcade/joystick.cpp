#if defined(QMC2_ARCADE_ENABLE_JOYSTICK)

#include <QRegExp>

#include "joystick.h"
#include "arcadesettings.h"
#include "consolewindow.h"
#include "macros.h"

extern ArcadeSettings *globalConfig;
extern ConsoleWindow *consoleWindow;

Joystick::Joystick(QObject *parent, int joystickEventTimeout, bool doAutoRepeat, int repeatDelay)
	: QObject(parent)
{
	if ( SDL_Init(SDL_INIT_JOYSTICK) == 0 ) {
		QRegExp rx("(\\b.*\\b)\\1");
		for (int i = 0; i < SDL_NumJoysticks(); i++) {
#if SDL_MAJOR_VERSION == 1
			QString jsName = SDL_JoystickName(i);
#elif SDL_MAJOR_VERSION == 2
			SDL_Joystick *js = SDL_JoystickOpen(i);
			QString jsName = SDL_JoystickName(js);
			SDL_JoystickClose(js);
#endif
			jsName.replace(rx, "\\1"); // remove consecutive duplicate words in the joystick name (i. e. "Logitech Logitech Extreme 3D" becomes "Logitech Extreme 3D")
			joystickNames.append(jsName);
		}
		connect(&joystickTimer, SIGNAL(timeout()), this, SLOT(processEvents()));
	} else
		QMC2_ARCADE_LOG_STR(tr("ERROR: couldn't initialize SDL joystick support"));

	joystick = NULL;
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
	if ( isOpen() )
		close();

	joystick = SDL_JoystickOpen(stick);
	if ( joystick ) {
		numAxes = SDL_JoystickNumAxes(joystick);
		numButtons = SDL_JoystickNumButtons(joystick);
		numHats = SDL_JoystickNumHats(joystick);
		numTrackballs = SDL_JoystickNumBalls(joystick);
		QMC2_ARCADE_LOG_STR(tr("SDL joystick #%1 opened: name = %2, axes = %3, buttons = %4, hats = %5, trackballs = %6").arg(stick).arg(joystickNames[stick]).arg(numAxes).arg(numButtons).arg(numHats).arg(numTrackballs));
		joystickTimer.start(eventTimeout);
		jsIndex = stick;
		deadzones.clear();
		sensitivities.clear();
		for (int axis = 0; axis < numAxes; axis++) {
			deadzones[axis] = globalConfig->joystickDeadzone(jsIndex, axis);
			sensitivities[axis] = globalConfig->joystickSensitivity(jsIndex, axis);
		}
		return true;
	} else {
		jsIndex = -1;
		return false;
	}
}

void Joystick::close()
{
	joystickTimer.stop();
	if ( joystick ) {
		SDL_JoystickClose(joystick);
		QMC2_ARCADE_LOG_STR(tr("SDL joystick #%1 closed").arg(jsIndex));
	}
	joystick = NULL;
	jsIndex = -1;
	numAxes = numButtons = numHats = numTrackballs = 0;
}

void Joystick::processEvents()
{
	if ( !isOpen() )
		return;

	SDL_JoystickUpdate();

	int i;
	for (i = 0; i < numAxes; i++) {
		Sint16 moved = normalizeAxisValue(SDL_JoystickGetAxis(joystick, i), i);
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
	for (i = 0; i < numButtons; i++) {
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
	for (i = 0; i < numHats; i++) {
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
	for (i = 0; i < numTrackballs; i++) {
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
		return normalizeAxisValue(SDL_JoystickGetAxis(joystick, axis), axis);
	} else
		return 0;
}

Sint16 Joystick::normalizeAxisValue(Sint16 rawValue, int axis)
{
	int max = globalConfig->joystickAxisMaximum(jsIndex, axis);
	int min = globalConfig->joystickAxisMinimum(jsIndex, axis);
	if ( max > min ) {
		rawValue -= (max + min) / 2;
		return 65535.0 * (double)rawValue / (double)(max - min);
	} else
		return 0;
}

#endif
