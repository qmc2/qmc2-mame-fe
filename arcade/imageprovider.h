#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include <QDeclarativeImageProvider>

class ImageProvider : public QDeclarativeImageProvider
{
public:
    ImageProvider(QDeclarativeImageProvider::ImageType);
    ~ImageProvider();

    QImage requestImage(const QString &, QSize *, const QSize &);
    QPixmap requestPixmap(const QString &, QSize *, const QSize &);
};

#endif
