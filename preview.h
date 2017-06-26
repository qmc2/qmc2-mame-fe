#ifndef PREVIEW_H
#define PREVIEW_H

#include "imagewidget.h"

class Preview : public ImageWidget
{
	Q_OBJECT 

	public:
		Preview(QWidget *parent);
		~Preview();

		virtual QString cachePrefix() { return "prv"; }
		virtual QString imageZip();
		virtual QString imageDir();
		virtual QString imageType() { return tr("preview"); }
		virtual int imageTypeNumeric() { return QMC2_IMGTYPE_PREVIEW; }
		virtual bool useZip();
		virtual bool useSevenZip();
		virtual bool useArchive();
		virtual bool scaledImage();
		virtual QString fallbackSettingsKey() { return QMC2_EMULATOR_PREFIX + "FilesAndDirectories/PreviewFallback"; }
};

#endif
