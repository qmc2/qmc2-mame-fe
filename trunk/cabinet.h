#ifndef _CABINET_H_
#define _CABINET_H_

#include "imagewidget.h"

class Cabinet : public ImageWidget
{
	Q_OBJECT 

	public:
		Cabinet(QWidget *parent);

		virtual QString cachePrefix() { return "cab"; }
		virtual QString imageZip();
		virtual QString imageDir();
		virtual QString imageType() { return tr("cabinet"); }
		virtual int imageTypeNumeric() { return QMC2_IMGTYPE_CABINET; }
		virtual bool useZip();
		virtual bool scaledImage();
};

#endif
