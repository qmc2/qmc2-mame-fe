#include "softwaresnapshot.h"
#include "settings.h"
#include "macros.h"

extern bool qmc2ScaledSoftwareSnapshot;
extern bool qmc2UseSoftwareSnapFile;
extern Settings *qmc2Config;

SoftwareSnapshot::SoftwareSnapshot(QWidget *parent)
	: SoftwareImageWidget(parent)
{
	artworkHash.insert(imageTypeNumeric(), this);
}

SoftwareSnapshot::~SoftwareSnapshot()
{
	artworkHash.remove(imageTypeNumeric());
}

QString SoftwareSnapshot::imageZip()
{
	return qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareSnapFile", QString()).toString();
}

QString SoftwareSnapshot::imageDir()
{
	return cleanDir(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareSnapDirectory", QString()).toString());
}

bool SoftwareSnapshot::useZip()
{
	return qmc2UseSoftwareSnapFile && qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareSnapFileType").toInt() == QMC2_IMG_FILETYPE_ZIP;
}

bool SoftwareSnapshot::useSevenZip()
{
	return qmc2UseSoftwareSnapFile && qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareSnapFileType").toInt() == QMC2_IMG_FILETYPE_7Z;
}

bool SoftwareSnapshot::useArchive()
{
	return qmc2UseSoftwareSnapFile && qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareSnapFileType").toInt() == QMC2_IMG_FILETYPE_ARCHIVE;
}

bool SoftwareSnapshot::scaledImage()
{
	return qmc2ScaledSoftwareSnapshot;
}
