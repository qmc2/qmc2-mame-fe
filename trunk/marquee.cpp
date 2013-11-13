#include "settings.h"
#include "marquee.h"
#include "macros.h"

// external global variables
extern Settings *qmc2Config;
extern bool qmc2UseMarqueeFile;
extern bool qmc2ScaledMarquee;

Marquee::Marquee(QWidget *parent)
	: ImageWidget(parent)
{
}

QString Marquee::imageZip()
{
	return qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/MarqueeFile").toString();
}

QString Marquee::imageDir()
{
	return cleanDir(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/MarqueeDirectory").toString());
}

bool Marquee::useZip()
{
	return qmc2UseMarqueeFile;
}

bool Marquee::scaledImage()
{
	return qmc2ScaledMarquee;
}
