#ifndef TITLE_H
#define TITLE_H

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
		virtual bool useArchive();
		virtual bool scaledImage();
		virtual QString fallbackSettingsKey() { return QMC2_EMULATOR_PREFIX + "FilesAndDirectories/TitleFallback"; }
};

#endif
