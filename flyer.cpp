#include "settings.h"
#include "flyer.h"
#include "macros.h"

// external global variables
extern Settings *qmc2Config;
extern bool qmc2UseFlyerFile;
extern bool qmc2ScaledFlyer;

Flyer::Flyer(QWidget *parent)
	: ImageWidget(parent)
{
}

QString Flyer::imageZip()
{
	return qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/FlyerFile").toString();
}

QString Flyer::imageDir()
{
	return cleanDir(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/FlyerDirectory").toString());
}

bool Flyer::useZip()
{
	return qmc2UseFlyerFile;
}

bool Flyer::scaledImage()
{
	return qmc2ScaledFlyer;
}
