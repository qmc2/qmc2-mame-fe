#ifndef _FLYER_H_
#define _FLYER_H_

#include "imagewidget.h"

class Flyer : public ImageWidget
{
	Q_OBJECT 

	public:
		Flyer(QWidget *parent);

	protected:
		virtual QString cachePrefix() { return "fly_"; }
		virtual QString imageZip();
		virtual QString imageDir();
		virtual QString imageType() { return tr("flyer"); }
		virtual bool useZip();
		virtual bool scaledImage();
};

#endif
