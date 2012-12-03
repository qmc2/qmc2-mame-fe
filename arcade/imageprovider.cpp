#include <QImage>
#include <QPixmap>

#include "imageprovider.h"

ImageProvider::ImageProvider(QDeclarativeImageProvider::ImageType type)
    : QDeclarativeImageProvider(type)
{
}

ImageProvider::~ImageProvider()
{
}

QImage ImageProvider::requestImage(const QString &, QSize *, const QSize &)
{
    // FIXME
    return QImage();
}

QPixmap ImageProvider::requestPixmap(const QString &, QSize *, const QSize &)
{
    return QPixmap();
}
