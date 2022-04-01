#include "settings.h"
#include "preview.h"
#include "macros.h"

// external global variables
extern Settings *qmc2Config;
extern bool qmc2UsePreviewFile;
extern bool qmc2ScaledPreview;

Preview::Preview(QWidget *parent)
	: ImageWidget(parent)
{
	artworkHash.insert(imageTypeNumeric(), this);
}

Preview::~Preview()
{
	artworkHash.remove(imageTypeNumeric());
}

QString Preview::imageZip()
{
	return qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/PreviewFile").toString();
}

QString Preview::imageDir()
{
	return cleanDir(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/PreviewDirectory").toString());
}

bool Preview::useZip()
{
	return qmc2UsePreviewFile && qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/PreviewFileType").toInt() == QMC2_IMG_FILETYPE_ZIP;
}

bool Preview::useSevenZip()
{
	return qmc2UsePreviewFile && qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/PreviewFileType").toInt() == QMC2_IMG_FILETYPE_7Z;
}

bool Preview::useArchive()
{
	return qmc2UsePreviewFile && qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/PreviewFileType").toInt() == QMC2_IMG_FILETYPE_ARCHIVE;
}

bool Preview::scaledImage()
{
	return qmc2ScaledPreview;
}
