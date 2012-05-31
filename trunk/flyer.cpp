#include <QSettings>

#include "flyer.h"
#include "macros.h"

// external global variables
extern QSettings *qmc2Config;
extern bool qmc2UseFlyerFile;
extern bool qmc2ScaledFlyer;

Flyer::Flyer(QWidget *parent)
	: ImageWidget(parent)
{
	// NOP
}

QString Flyer::imageZip()
{
	return qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/FlyerFile").toString();
}

QString Flyer::imageDir()
{
	QStringList dirList;
	foreach (QString dir, qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/FlyerDirectory").toString().split(";", QString::SkipEmptyParts)) {
		if ( !dir.endsWith("/") )
			dir += "/";
		dirList << dir;
	}
	return dirList.join(";");
}

bool Flyer::useZip()
{
	return qmc2UseFlyerFile;
}

bool Flyer::scaledImage()
{
	return qmc2ScaledFlyer;
}
