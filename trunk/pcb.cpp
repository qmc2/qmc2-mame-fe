#include <QSettings>

#include "pcb.h"
#include "macros.h"

// external global variables
extern QSettings *qmc2Config;
extern bool qmc2UsePCBFile;
extern bool qmc2ScaledPCB;

PCB::PCB(QWidget *parent)
	: ImageWidget(parent)
{
	// NOP
}

QString PCB::imageZip()
{
	return qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/PCBFile").toString();
}

QString PCB::imageDir()
{
	QStringList dirList;
	foreach (QString dir, qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/PCBDirectory").toString().split(";", QString::SkipEmptyParts)) {
		if ( !dir.endsWith("/") )
			dir += "/";
		dirList << dir;
	}
	return dirList.join(";");
}

bool PCB::useZip()
{
	return qmc2UsePCBFile;
}

bool PCB::scaledImage()
{
	return qmc2ScaledPCB;
}
