#include <QSettings>

#include "cabinet.h"
#include "macros.h"

// external global variables
extern QSettings *qmc2Config;
extern bool qmc2UseCabinetFile;
extern bool qmc2ScaledCabinet;

Cabinet::Cabinet(QWidget *parent)
	: ImageWidget(parent)
{
	QStringList imgFmts = qmc2Config->value(QMC2_FRONTEND_PREFIX + "ActiveImageFormats/cab", QStringList()).toStringList();
	if ( imgFmts.isEmpty() )
		activeFormats << QMC2_IMAGE_FORMAT_INDEX_PNG;
	else for (int i = 0; i < imgFmts.count(); i++)
		activeFormats << imgFmts[i].toInt();
}

QString Cabinet::imageZip()
{
	return qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/CabinetFile").toString();
}

QString Cabinet::imageDir()
{
	return cleanDir(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/CabinetDirectory").toString());
}

bool Cabinet::useZip()
{
	return qmc2UseCabinetFile;
}

bool Cabinet::scaledImage()
{
	return qmc2ScaledCabinet;
}
