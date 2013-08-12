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
	QStringList imgFmts = qmc2Config->value(QMC2_FRONTEND_PREFIX + "ActiveImageFormats/fly", QStringList()).toStringList();
	if ( imgFmts.isEmpty() )
		activeFormats << QMC2_IMAGE_FORMAT_INDEX_PNG;
	else for (int i = 0; i < imgFmts.count(); i++)
		activeFormats << imgFmts[i].toInt();
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
