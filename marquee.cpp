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
	QStringList imgFmts = qmc2Config->value(QMC2_FRONTEND_PREFIX + "ActiveImageFormats/mrq", QStringList()).toStringList();
	if ( imgFmts.isEmpty() )
		activeFormats << QMC2_IMAGE_FORMAT_INDEX_PNG;
	else for (int i = 0; i < imgFmts.count(); i++)
		activeFormats << imgFmts[i].toInt();
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
