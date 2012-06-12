#ifndef _MARQUEE_H_
#define _MARQUEE_H_

#include "imagewidget.h"

class Marquee : public ImageWidget
{
	Q_OBJECT 

	public:
		Marquee(QWidget *parent);

		virtual QString cachePrefix() { return "mrq_"; }
		virtual QString imageZip();
		virtual QString imageDir();
		virtual QString imageType() { return tr("marquee"); }
		virtual bool useZip();
		virtual bool scaledImage();
};

#endif
