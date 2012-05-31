#include <QSettings>

#include "title.h"
#include "macros.h"

// external global variables
extern QSettings *qmc2Config;
extern bool qmc2UseTitleFile;
extern bool qmc2ScaledTitle;

Title::Title(QWidget *parent)
	: ImageWidget(parent)
{
	// NOP
}

QString Title::imageZip()
{
	return qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/TitleFile").toString();
}

QString Title::imageDir()
{
	QStringList dirList;
	foreach (QString dir, qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/TitleDirectory").toString().split(";", QString::SkipEmptyParts)) {
		if ( !dir.endsWith("/") )
			dir += "/";
		dirList << dir;
	}
	return dirList.join(";");
}

bool Title::useZip()
{
	return qmc2UseTitleFile;
}

bool Title::scaledImage()
{
	return qmc2ScaledTitle;
}
