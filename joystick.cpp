#if QMC2_JOYSTICK == 1

#include "joystick.h"
#include "qmc2main.h"
#include "macros.h"

// external global variables
extern MainWindow *qmc2MainWindow;

Joystick::Joystick(QObject *parent, int joystickEventTimeout, bool doAutoRepeat, int repeatDelay)
  : QObject(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Joystick::Joystick(QObject *parent = %1, int joystickEventTimeout = %2, bool doAutoRepeat = %3, int repeatDelay = %4)").arg((qulonglong)parent).arg(joystickEventTimeout).arg(doAutoRepeat).arg(repeatDelay));
#endif

  if ( SDL_Init(SDL_INIT_JOYSTICK) == 0 ) {
    int i;
    for (i = 0; i < SDL_NumJoysticks(); i++)
      joystickNames.append(SDL_JoystickName(i));
    connect(&joystickTimer, SIGNAL(timeout()), this, SLOT(processEvents()));
  } else {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: couldn't initialize SDL joystick support"));
  }

  joystick = NULL;
  numAxes = numButtons = numHats = numTrackballs = 0;
  autoRepeat = doAutoRepeat;
  autoRepeatDelay = repeatDelay;
  eventTimeout = joystickEventTimeout;
}

Joystick::~Joystick()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Joystick::~Joystick()");
#endif

  if ( isOpen() )
    close();

  SDL_Quit();
}

bool Joystick::open(int stick)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Joystick::open(int stick = %1)").arg(stick));
#endif

  if ( isOpen() )
    close();

  joystick = SDL_JoystickOpen(stick);
  if ( joystick ) {
    numAxes = SDL_JoystickNumAxes(joystick);
    numButtons = SDL_JoystickNumButtons(joystick);
    numHats = SDL_JoystickNumHats(joystick);
    numTrackballs = SDL_JoystickNumBalls(joystick);
#ifdef QMC2_DEBUG
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SDL joystick #%1 opened; axes = %2, buttons = %3, hats = %4, trackballs = %5").
                        arg(stick).arg(numAxes).arg(numButtons).arg(numHats).arg(numTrackballs));
#endif
    joystickTimer.start(eventTimeout);
    return true;
  } else {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: couldn't open SDL joystick #%1").arg(stick));
    return false;
  }
}

void Joystick::close()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Joystick::close()");
#endif

  joystickTimer.stop();
  if ( joystick )
    SDL_JoystickClose(joystick);
  joystick = NULL;
  numAxes = numButtons = numHats = numTrackballs = 0;
}

void Joystick::processEvents()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Joystick::processEvents()");
#endif

  if ( !isOpen() )
    return;

  SDL_JoystickUpdate();

  int i;
  for (i = 0; i < numAxes; i++) {
    Sint16 moved = SDL_JoystickGetAxis(joystick, i);
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
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Joystick::getAxisValue(int axis = %1)").arg(axis));
#endif

  if ( isOpen() ) {
    SDL_JoystickUpdate();
    return SDL_JoystickGetAxis(joystick, axis);
  } else
    return 0;
}

#endif
