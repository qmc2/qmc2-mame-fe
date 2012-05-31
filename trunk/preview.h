#ifndef _PREVIEW_H_
#define _PREVIEW_H_

#include "imagewidget.h"

class Preview : public ImageWidget
{
	Q_OBJECT 

	public:
		Preview(QWidget *parent);

	protected:
		virtual QString cachePrefix() { return "prv_"; }
		virtual QString imageZip();
		virtual QString imageDir();
		virtual QString imageType() { return tr("preview"); }
		virtual bool useZip();
		virtual bool scaledImage();
};

#endif
