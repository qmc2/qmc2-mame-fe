#ifndef _CUSTOMSOFTWAREARTWORK_H_
#define _CUSTOMSOFTWAREARTWORK_H_

#include <QString>

#include "softwareimagewidget.h"

class CustomSoftwareArtwork : public SoftwareImageWidget
{
	Q_OBJECT 

	public:
		CustomSoftwareArtwork(QWidget *parent, QString name, int num);
		~CustomSoftwareArtwork();

		virtual QString cachePrefix() { return m_cachePrefix; }
		virtual QString imageZip();
		virtual QString imageDir();
		virtual QString imageType() { return m_name; }
		virtual int imageTypeNumeric() { return m_num; }
		virtual bool useZip();
		virtual bool useSevenZip();
		virtual bool scaledImage();
		virtual bool customArtwork() { return true; }
		virtual QString fallbackSettingsKey() { return QString("Artwork/%1/Fallback").arg(name()); }

		QString name() { return m_name; }
		void setName(QString name) { m_name = name; }
		int num() { return m_num; }
		void setNum(int num);

	private:
		QString m_name;
		QString m_cachePrefix;
		int m_num;
};

#endif
