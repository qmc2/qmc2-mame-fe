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
	QStringList imgFmts = qmc2Config->value(QMC2_FRONTEND_PREFIX + "ActiveImageFormats/ttl", QStringList()).toStringList();
	if ( imgFmts.isEmpty() )
		activeFormats << QMC2_IMAGE_FORMAT_INDEX_PNG;
	else for (int i = 0; i < imgFmts.count(); i++)
		activeFormats << imgFmts[i].toInt();
}

QString Title::imageZip()
{
	return qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/TitleFile").toString();
}

QString Title::imageDir()
{
	return cleanDir(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/TitleDirectory").toString());
}

bool Title::useZip()
{
	return qmc2UseTitleFile;
}

bool Title::scaledImage()
{
	return qmc2ScaledTitle;
}
