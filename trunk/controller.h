#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include "imagewidget.h"

class Controller : public ImageWidget
{
	Q_OBJECT 

	public:
		Controller(QWidget *parent);

		virtual QString cachePrefix() { return "ctl_"; }
		virtual QString imageZip();
		virtual QString imageDir();
		virtual QString imageType() { return tr("controller"); }
		virtual int imageTypeNumeric() { return QMC2_IMGTYPE_CONTROLLER; }
		virtual bool useZip();
		virtual bool scaledImage();
};

#endif
