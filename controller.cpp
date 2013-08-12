#include <QSettings>

#include "controller.h"
#include "macros.h"

// external global variables
extern QSettings *qmc2Config;
extern bool qmc2UseControllerFile;
extern bool qmc2ScaledController;

Controller::Controller(QWidget *parent)
	: ImageWidget(parent)
{
	QStringList imgFmts = qmc2Config->value(QMC2_FRONTEND_PREFIX + "ActiveImageFormats/ctl", QStringList()).toStringList();
	if ( imgFmts.isEmpty() )
		activeFormats << QMC2_IMAGE_FORMAT_INDEX_PNG;
	else for (int i = 0; i < imgFmts.count(); i++)
		activeFormats << imgFmts[i].toInt();
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
	return qmc2UseControllerFile;
}

bool Controller::scaledImage()
{
	return qmc2ScaledController;
}
