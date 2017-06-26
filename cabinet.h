#ifndef CABINET_H
#define CABINET_H

#include "imagewidget.h"

class Cabinet : public ImageWidget
{
	Q_OBJECT 

	public:
		Cabinet(QWidget *parent);
		~Cabinet();

		virtual QString cachePrefix() { return "cab"; }
		virtual QString imageZip();
		virtual QString imageDir();
		virtual QString imageType() { return tr("cabinet"); }
		virtual int imageTypeNumeric() { return QMC2_IMGTYPE_CABINET; }
		virtual bool useZip();
		virtual bool useSevenZip();
		virtual bool useArchive();
		virtual bool scaledImage();
		virtual QString fallbackSettingsKey() { return QMC2_EMULATOR_PREFIX + "FilesAndDirectories/CabinetFallback"; }
};

#endif
