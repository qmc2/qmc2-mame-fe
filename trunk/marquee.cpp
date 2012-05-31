#include <QSettings>

#include "marquee.h"
#include "macros.h"

// external global variables
extern QSettings *qmc2Config;
extern bool qmc2UseMarqueeFile;
extern bool qmc2ScaledMarquee;

Marquee::Marquee(QWidget *parent)
	: ImageWidget(parent)
{
	// NOP
}

QString Marquee::imageZip()
{
	return qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/MarqueeFile").toString();
}

QString Marquee::imageDir()
{
	QStringList dirList;
	foreach (QString dir, qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/MarqueeDirectory").toString().split(";", QString::SkipEmptyParts)) {
		if ( !dir.endsWith("/") )
			dir += "/";
		dirList << dir;
	}
	return dirList.join(";");
}

bool Marquee::useZip()
{
	return qmc2UseMarqueeFile;
}

bool Marquee::scaledImage()
{
	return qmc2ScaledMarquee;
}
