#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include "imagewidget.h"

class Controller : public ImageWidget
{
	Q_OBJECT 

	public:
		Controller(QWidget *parent);

	protected:
		virtual QString cachePrefix() { return "ctl_"; }
		virtual QString imageZip();
		virtual QString imageDir();
		virtual QString imageType() { return tr("controller"); }
		virtual bool useZip();
		virtual bool scaledImage();
};

#endif
