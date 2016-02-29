#if defined(QMC2_ARCADE_ENABLE_JOYSTICK)

#ifndef JOYSTICKMANAGER_H
#define JOYSTICKMANAGER_H

#include <QObject>
#include <QString>

#include "joystick.h"
#include "joyfunctionmap.h"

class JoystickManager : public QObject
{
	Q_OBJECT

public:
	explicit JoystickManager(JoyFunctionMap *joyFuncMap, int joyIndex = -1, QObject *parent = 0);
	virtual ~JoystickManager();

	int joyIndex() { return mJoyIndex; }
	Joystick *joystick() { return mJoystick; }

	void mapJoystickFunction(QString);

public slots:
	void axisValueChanged(int, int);
	void buttonValueChanged(int, bool);
	void hatValueChanged(int, int);
	void trackballValueChanged(int, int, int);

private:
	int mJoyIndex;
	Joystick *mJoystick;
	JoyFunctionMap *mJoyFunctionMap;

	void openJoystick(int joystickIndex = -1);
	void closeJoystick();
};

#endif

#endif
