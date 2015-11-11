#ifndef _CUSTOMARTWORK_H_
#define _CUSTOMARTWORK_H_

#include <QString>

#include "imagewidget.h"

class CustomArtwork : public ImageWidget
{
	Q_OBJECT 

	public:
		CustomArtwork(QWidget *parent, QString name, int num);
		~CustomArtwork();

		virtual QString cachePrefix() { return m_cachePrefix; }
		virtual QString imageZip();
		virtual QString imageDir();
		virtual QString imageType() { return m_name; }
		virtual int imageTypeNumeric() { return m_num; }
		virtual bool useZip();
		virtual bool useSevenZip();
		virtual bool scaledImage();

	private:
		QString m_name;
		QString m_cachePrefix;
		int m_num;
};

#endif
