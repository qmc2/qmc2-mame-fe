#ifndef _TITLE_H_
#define _TITLE_H_

#include "imagewidget.h"

class Title : public ImageWidget
{
	Q_OBJECT 

	public:
		Title(QWidget *parent);
		~Title();

		virtual QString cachePrefix() { return "ttl"; }
		virtual QString imageZip();
		virtual QString imageDir();
		virtual QString imageType() { return tr("title"); }
		virtual int imageTypeNumeric() { return QMC2_IMGTYPE_TITLE; }
		virtual bool useZip();
		virtual bool useSevenZip();
		virtual bool scaledImage();
};

#endif
