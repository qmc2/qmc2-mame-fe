#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include <QDeclarativeImageProvider>
#include <QMap>
#include <QCache>
#include <QString>

#include "../minizip/unzip.h"

class ImageProvider : public QDeclarativeImageProvider
{
public:
    explicit ImageProvider(QDeclarativeImageProvider::ImageType);
    virtual ~ImageProvider();

    QImage requestImage(const QString &, QSize *, const QSize &);
    QPixmap requestPixmap(const QString &, QSize *, const QSize &);

private:
    QImage loadZippedImage(QString, QString);
    QPixmap loadZippedPixmap(QString, QString);
    QString imageTypeToZipFile(QString);
    QString imageTypeToLongName(QString);
    bool isZippedImageType(QString);
    QString imageFolder(QString);
    QStringList mImageTypes;
    QMap<QString, unzFile> mZipFileMap;
    QCache<QString, QImage> mImageCache;
    QCache<QString, QPixmap> mPixmapCache;
};

#endif
