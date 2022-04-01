#if defined(QMC2_ARCADE_ENABLE_JOYSTICK)

#include "joyfunctionmap.h"
#include "arcadesettings.h"
#include "macros.h"

extern ArcadeSettings *globalConfig;

JoyFunctionMap::JoyFunctionMap(QStringList keySequences, QObject *parent) :
	QObject(parent)
{
	setKeySequences(keySequences);
	loadJoyFunctionMap();
}

void JoyFunctionMap::setKeySequences(QStringList keySequences)
{
	mNativeKeySequences = keySequences;
}

void JoyFunctionMap::loadJoyFunctionMap()
{
	mJoyFunctionMap.clear();
	globalConfig->beginGroup(globalConfig->joyFunctionMapBaseKey());
	foreach (QString key, globalConfig->childKeys()) {
		if ( mNativeKeySequences.contains(key) )
			mJoyFunctionMap[globalConfig->value(key, key).toString()] = key;
		else
			globalConfig->remove(key);
	}
	globalConfig->endGroup();
}

QString JoyFunctionMap::mapJoyFunction(QString joyFunction)
{
	if ( mJoyFunctionMap.contains(joyFunction) )
		return mJoyFunctionMap[joyFunction];
	else
		return joyFunction;
}

#endif
