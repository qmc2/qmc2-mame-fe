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
