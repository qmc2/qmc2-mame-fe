#include "settings.h"
#include "pcb.h"
#include "macros.h"

// external global variables
extern Settings *qmc2Config;
extern bool qmc2UsePCBFile;
extern bool qmc2ScaledPCB;

PCB::PCB(QWidget *parent)
	: ImageWidget(parent)
{
	artworkHash.insert(imageTypeNumeric(), this);
}

PCB::~PCB()
{
	artworkHash.remove(imageTypeNumeric());
}

QString PCB::imageZip()
{
	return qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/PCBFile").toString();
}

QString PCB::imageDir()
{
	return cleanDir(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/PCBDirectory").toString());
}

bool PCB::useZip()
{
	return qmc2UsePCBFile && qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/PCBFileType").toInt() == QMC2_IMG_FILETYPE_ZIP;
}

bool PCB::useSevenZip()
{
	return qmc2UsePCBFile && qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/PCBFileType").toInt() == QMC2_IMG_FILETYPE_7Z;
}

bool PCB::scaledImage()
{
	return qmc2ScaledPCB;
}
