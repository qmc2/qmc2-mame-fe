#include "settings.h"
#include "title.h"
#include "macros.h"

// external global variables
extern Settings *qmc2Config;
extern bool qmc2UseTitleFile;
extern bool qmc2ScaledTitle;

Title::Title(QWidget *parent)
	: ImageWidget(parent)
{
	artworkHash.insert(imageTypeNumeric(), this);
}

Title::~Title()
{
	artworkHash.remove(imageTypeNumeric());
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
	return qmc2UseTitleFile && qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/TitleFileType").toInt() == QMC2_IMG_FILETYPE_ZIP;
}

bool Title::useSevenZip()
{
	return qmc2UseTitleFile && qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/TitleFileType").toInt() == QMC2_IMG_FILETYPE_7Z;
}

bool Title::scaledImage()
{
	return qmc2ScaledTitle;
}
