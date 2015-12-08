#include <QUuid>

#include "settings.h"
#include "customartwork.h"
#include "qmc2main.h"
#include "macros.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;

CustomArtwork::CustomArtwork(QWidget *parent, QString name, int num)
	: ImageWidget(parent)
{
	setName(name);
	setNum(num);
	m_cachePrefix = QUuid::createUuid().toString();
	artworkHash.insert(imageTypeNumeric(), this);
	if ( !parent )
		hide();
}

CustomArtwork::~CustomArtwork()
{
	artworkHash.remove(imageTypeNumeric());
}

void CustomArtwork::setNum(int num)
{
	m_num = QMC2_IMGTYPE_USER + num;
}

QString CustomArtwork::imageZip()
{
	return qmc2Config->value(QString("Artwork/%1/Archive").arg(name()), QString()).toString();
}

QString CustomArtwork::imageDir()
{
	return qmc2Config->value(QString("Artwork/%1/Folder").arg(name()), QString()).toString();
}

bool CustomArtwork::useZip()
{
	return qmc2Config->value(QString("Artwork/%1/Type").arg(name()), 0).toInt() == QMC2_AW_INDEX_TYPE_ARCHIVE && qmc2Config->value(QString("Artwork/%1/Format").arg(name()), 0).toInt() == QMC2_AW_INDEX_FORMAT_ZIP;
}

bool CustomArtwork::useSevenZip()
{
	return qmc2Config->value(QString("Artwork/%1/Type").arg(name()), 0).toInt() == QMC2_AW_INDEX_TYPE_ARCHIVE && qmc2Config->value(QString("Artwork/%1/Format").arg(name()), 0).toInt() == QMC2_AW_INDEX_FORMAT_7Z;
}

bool CustomArtwork::useArchive()
{
	return qmc2Config->value(QString("Artwork/%1/Type").arg(name()), 0).toInt() == QMC2_AW_INDEX_TYPE_ARCHIVE && qmc2Config->value(QString("Artwork/%1/Format").arg(name()), 0).toInt() == QMC2_AW_INDEX_FORMAT_ARCHIVE;
}

bool CustomArtwork::scaledImage()
{
	return qmc2Config->value(QString("Artwork/%1/Scaled").arg(name()), 0).toInt() == QMC2_AW_INDEX_SCALED_ON;
}
