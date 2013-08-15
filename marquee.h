#ifndef _MARQUEE_H_
#define _MARQUEE_H_

#include "imagewidget.h"

class Marquee : public ImageWidget
{
	Q_OBJECT 

	public:
		Marquee(QWidget *parent);

		virtual QString cachePrefix() { return "mrq"; }
		virtual QString imageZip();
		virtual QString imageDir();
#if defined(QMC2_EMUTYPE_MESS)
		virtual QString imageType() { return tr("logo"); }
#else
		virtual QString imageType() { return tr("marquee"); }
#endif
		virtual int imageTypeNumeric() { return QMC2_IMGTYPE_MARQUEE; }
		virtual bool useZip();
		virtual bool scaledImage();
};

#endif
