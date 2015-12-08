#include <QUuid>

#include "settings.h"
#include "customsoftwareartwork.h"
#include "qmc2main.h"
#include "macros.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;

CustomSoftwareArtwork::CustomSoftwareArtwork(QWidget *parent, QString name, int num)
	: SoftwareImageWidget(parent)
{
	setName(name);
	setNum(num);
	m_cachePrefix = QUuid::createUuid().toString();
	artworkHash.insert(imageTypeNumeric(), this);
	if ( !parent )
		hide();
}

CustomSoftwareArtwork::~CustomSoftwareArtwork()
{
	artworkHash.remove(imageTypeNumeric());
}

void CustomSoftwareArtwork::setNum(int num)
{
	m_num = QMC2_IMGTYPE_USER + num;
}

QString CustomSoftwareArtwork::imageZip()
{
	return qmc2Config->value(QString("Artwork/%1/Archive").arg(name()), QString()).toString();
}

QString CustomSoftwareArtwork::imageDir()
{
	return qmc2Config->value(QString("Artwork/%1/Folder").arg(name()), QString()).toString();
}

bool CustomSoftwareArtwork::useZip()
{
	return qmc2Config->value(QString("Artwork/%1/Type").arg(name()), 0).toInt() == QMC2_AW_INDEX_TYPE_ARCHIVE && qmc2Config->value(QString("Artwork/%1/Format").arg(name()), 0).toInt() == QMC2_AW_INDEX_FORMAT_ZIP;
}

bool CustomSoftwareArtwork::useSevenZip()
{
	return qmc2Config->value(QString("Artwork/%1/Type").arg(name()), 0).toInt() == QMC2_AW_INDEX_TYPE_ARCHIVE && qmc2Config->value(QString("Artwork/%1/Format").arg(name()), 0).toInt() == QMC2_AW_INDEX_FORMAT_7Z;
}

bool CustomSoftwareArtwork::useArchive()
{
	return qmc2Config->value(QString("Artwork/%1/Type").arg(name()), 0).toInt() == QMC2_AW_INDEX_TYPE_ARCHIVE && qmc2Config->value(QString("Artwork/%1/Format").arg(name()), 0).toInt() == QMC2_AW_INDEX_FORMAT_ARCHIVE;
}

bool CustomSoftwareArtwork::scaledImage()
{
	return qmc2Config->value(QString("Artwork/%1/Scaled").arg(name()), 0).toInt() == QMC2_AW_INDEX_SCALED_ON;
}
