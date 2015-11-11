#include "settings.h"
#include "customartwork.h"
#include "macros.h"

// external global variables
extern Settings *qmc2Config;

CustomArtwork::CustomArtwork(QWidget *parent)
	: ImageWidget(parent)
{
}

QString CustomArtwork::imageZip()
{
	// FIXME
	return QString();
}

QString CustomArtwork::imageDir()
{
	// FIXME
	return QString();
}

bool CustomArtwork::useZip()
{
	// FIXME
	return false;
}

bool CustomArtwork::useSevenZip()
{
	// FIXME
	return false;
}


bool CustomArtwork::scaledImage()
{
	// FIXME
	return true;
}
