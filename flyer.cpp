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
	artworkHash.insert(imageTypeNumeric(), this);
}

Flyer::~Flyer()
{
	artworkHash.remove(imageTypeNumeric());
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
	return qmc2UseFlyerFile && qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/FlyerFileType").toInt() == QMC2_IMG_FILETYPE_ZIP;
}

bool Flyer::useSevenZip()
{
	return qmc2UseFlyerFile && qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/FlyerFileType").toInt() == QMC2_IMG_FILETYPE_7Z;
}

bool Flyer::scaledImage()
{
	return qmc2ScaledFlyer;
}
