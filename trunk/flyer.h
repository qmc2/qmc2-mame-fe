#ifndef _FLYER_H_
#define _FLYER_H_

#include "imagewidget.h"

class Flyer : public ImageWidget
{
	Q_OBJECT 

	public:
		Flyer(QWidget *parent);

		virtual QString cachePrefix() { return "fly"; }
		virtual QString imageZip();
		virtual QString imageDir();
		virtual QString imageType() { return tr("flyer"); }
		virtual int imageTypeNumeric() { return QMC2_IMGTYPE_FLYER; }
		virtual bool useZip();
		virtual bool useSevenZip();
		virtual bool scaledImage();
};

#endif