#include <QSettings>

#include "preview.h"
#include "macros.h"

// external global variables
extern QSettings *qmc2Config;
extern bool qmc2UsePreviewFile;
extern bool qmc2ScaledPreview;

Preview::Preview(QWidget *parent)
	: ImageWidget(parent)
{
	// NOP
}

QString Preview::imageZip()
{
	return qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/PreviewFile").toString();
}

QString Preview::imageDir()
{
	QStringList dirList;
	foreach (QString dir, qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/PreviewDirectory").toString().split(";", QString::SkipEmptyParts)) {
		if ( !dir.endsWith("/") )
			dir += "/";
		dirList << dir;
	}
	return dirList.join(";");
}

bool Preview::useZip()
{
	return qmc2UsePreviewFile;
}

bool Preview::scaledImage()
{
	return qmc2ScaledPreview;
}
