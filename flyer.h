#ifndef FLYER_H
#define FLYER_H

#include "imagewidget.h"

class Flyer : public ImageWidget
{
	Q_OBJECT 

	public:
		Flyer(QWidget *parent);
		~Flyer();

		virtual QString cachePrefix() { return "fly"; }
		virtual QString imageZip();
		virtual QString imageDir();
		virtual QString imageType() { return tr("flyer"); }
		virtual int imageTypeNumeric() { return QMC2_IMGTYPE_FLYER; }
		virtual bool useZip();
		virtual bool useSevenZip();
		virtual bool useArchive();
		virtual bool scaledImage();
		virtual QString fallbackSettingsKey() { return QMC2_EMULATOR_PREFIX + "FilesAndDirectories/FlyerFallback"; }
};

#endif
