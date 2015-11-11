#include <QUuid>

#include "settings.h"
#include "customartwork.h"
#include "macros.h"

// external global variables
extern Settings *qmc2Config;

CustomArtwork::CustomArtwork(QWidget *parent, QString name, int num)
	: ImageWidget(parent)
{
	m_name = name;
	m_num = QMC2_IMGTYPE_USER + num;
	m_cachePrefix = QUuid::createUuid().toString();
	artworkHash.insert(imageTypeNumeric(), this);
}

CustomArtwork::~CustomArtwork()
{
	artworkHash.remove(imageTypeNumeric());
}

QString CustomArtwork::imageZip()
{
	return qmc2Config->value(qmc2Config->value(QString("Artwork/%1/Archive").arg(m_name), QString()).toString(), QString()).toString();
}

QString CustomArtwork::imageDir()
{
	return qmc2Config->value(qmc2Config->value(QString("Artwork/%1/Folder").arg(m_name), QString()).toString(), QString()).toString();
}

bool CustomArtwork::useZip()
{
	return qmc2Config->value(QString("Artwork/%1/Type").arg(m_name), 0).toInt() == QMC2_ADDITIONALARTWORK_INDEX_TYPE_ARCHIVE && qmc2Config->value(QString("Artwork/%1/Format").arg(m_name), 0).toInt() == QMC2_ADDITIONALARTWORK_INDEX_FORMAT_ZIP;
}

bool CustomArtwork::useSevenZip()
{
	return qmc2Config->value(QString("Artwork/%1/Type").arg(m_name), 0).toInt() == QMC2_ADDITIONALARTWORK_INDEX_TYPE_ARCHIVE && qmc2Config->value(QString("Artwork/%1/Format").arg(m_name), 0).toInt() == QMC2_ADDITIONALARTWORK_INDEX_FORMAT_7Z;
}


bool CustomArtwork::scaledImage()
{
	return qmc2Config->value(QString("Artwork/%1/Scaled").arg(m_name), 0).toInt() == QMC2_ADDITIONALARTWORK_INDEX_SCALED_ON;
}
