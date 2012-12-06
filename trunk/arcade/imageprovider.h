#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include <QDeclarativeImageProvider>

class ImageProvider : public QDeclarativeImageProvider
{
public:
    explicit ImageProvider(QDeclarativeImageProvider::ImageType);
    virtual ~ImageProvider();

    QImage requestImage(const QString &, QSize *, const QSize &);
    QPixmap requestPixmap(const QString &, QSize *, const QSize &);
};

#endif
