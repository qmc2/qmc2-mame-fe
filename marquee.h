#ifndef _MARQUEE_H_
#define _MARQUEE_H_

#include "imagewidget.h"

class Marquee : public ImageWidget
{
	Q_OBJECT 

	public:
		Marquee(QWidget *parent);
		~Marquee();

		virtual QString cachePrefix() { return "mrq"; }
		virtual QString imageZip();
		virtual QString imageDir();
		virtual QString imageType() { return tr("marquee"); }
		virtual int imageTypeNumeric() { return QMC2_IMGTYPE_MARQUEE; }
		virtual bool useZip();
		virtual bool useSevenZip();
		virtual bool useArchive();
		virtual bool scaledImage();
		virtual QString fallbackSettingsKey() { return QMC2_EMULATOR_PREFIX + "FilesAndDirectories/MarqueeFallback"; }
};

#endif
