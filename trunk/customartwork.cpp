#include "settings.h"
#include "customartwork.h"
#include "macros.h"

// external global variables
extern Settings *qmc2Config;

CustomArtwork::CustomArtwork(QWidget *parent, QString name)
	: ImageWidget(parent)
{
	m_name = name;
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
	return qmc2Config->value(QString("Artwork/%1/Type").arg(m_name), 0).toInt() == QMC2_ADDITIONALARTWORK_INDEX_TYPE_ARCHIVE; // FIXME
}

bool CustomArtwork::useSevenZip()
{
	return qmc2Config->value(QString("Artwork/%1/Type").arg(m_name), 0).toInt() == QMC2_ADDITIONALARTWORK_INDEX_TYPE_ARCHIVE; // FIXME
}


bool CustomArtwork::scaledImage()
{
	return qmc2Config->value(QString("Artwork/%1/Scaled").arg(m_name), 0).toInt() == QMC2_ADDITIONALARTWORK_INDEX_SCALED_ON;
}
