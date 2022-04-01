#include "settings.h"
#include "controller.h"
#include "macros.h"

// external global variables
extern Settings *qmc2Config;
extern bool qmc2UseControllerFile;
extern bool qmc2ScaledController;

Controller::Controller(QWidget *parent)
	: ImageWidget(parent)
{
	artworkHash.insert(imageTypeNumeric(), this);
}

Controller::~Controller()
{
	artworkHash.remove(imageTypeNumeric());
}

QString Controller::imageZip()
{
	return qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ControllerFile").toString();
}

QString Controller::imageDir()
{
	return cleanDir(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ControllerDirectory").toString());
}

bool Controller::useZip()
{
	return qmc2UseControllerFile && qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ControllerFileType").toInt() == QMC2_IMG_FILETYPE_ZIP;
}

bool Controller::useSevenZip()
{
	return qmc2UseControllerFile && qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ControllerFileType").toInt() == QMC2_IMG_FILETYPE_7Z;
}

bool Controller::useArchive()
{
	return qmc2UseControllerFile && qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ControllerFileType").toInt() == QMC2_IMG_FILETYPE_ARCHIVE;
}

bool Controller::scaledImage()
{
	return qmc2ScaledController;
}
