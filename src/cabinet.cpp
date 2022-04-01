#include "settings.h"
#include "cabinet.h"
#include "macros.h"

// external global variables
extern Settings *qmc2Config;
extern bool qmc2UseCabinetFile;
extern bool qmc2ScaledCabinet;

Cabinet::Cabinet(QWidget *parent)
	: ImageWidget(parent)
{
	artworkHash.insert(imageTypeNumeric(), this);
}

Cabinet::~Cabinet()
{
	artworkHash.remove(imageTypeNumeric());
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
	return qmc2UseCabinetFile && qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/CabinetFileType").toInt() == QMC2_IMG_FILETYPE_ZIP;
}

bool Cabinet::useSevenZip()
{
	return qmc2UseCabinetFile && qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/CabinetFileType").toInt() == QMC2_IMG_FILETYPE_7Z;
}

bool Cabinet::useArchive()
{
	return qmc2UseCabinetFile && qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/CabinetFileType").toInt() == QMC2_IMG_FILETYPE_ARCHIVE;
}

bool Cabinet::scaledImage()
{
	return qmc2ScaledCabinet;
}
