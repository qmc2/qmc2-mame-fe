#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include <QMap>
#include <QCache>
#include <QString>

#include "../minizip/unzip.h"

#if QT_VERSION < 0x050000
#include <QDeclarativeImageProvider>

class ImageProvider : public QDeclarativeImageProvider
{
public:
    explicit ImageProvider(QDeclarativeImageProvider::ImageType);
    virtual ~ImageProvider();

    QImage requestImage(const QString &, QSize *, const QSize &);
    QPixmap requestPixmap(const QString &, QSize *, const QSize &);
    QString loadImage(const QString &);

    enum CacheClass { CacheClassImage, CacheClassPixmap };

private:
    QString loadImage(const QString &id, const enum CacheClass cacheClass);
    QString imageTypeToZipFile(QString);
    QString imageTypeToLongName(QString);
    bool isZippedImageType(QString);
    QString imageFolder(QString);
    QStringList mImageTypes;
    QMap<QString, unzFile> mZipFileMap;
    QCache<QString, QImage> mImageCache;
    QCache<QString, QPixmap> mPixmapCache;
};
#else
#include <QQuickImageProvider>

class ImageProvider : public QQuickImageProvider
{
public:
    explicit ImageProvider(QQuickImageProvider::ImageType);
    virtual ~ImageProvider();

    QImage requestImage(const QString &, QSize *, const QSize &);
    QPixmap requestPixmap(const QString &, QSize *, const QSize &);
    QString loadImage(const QString &);

    enum CacheClass { CacheClassImage, CacheClassPixmap };

private:
    QString loadImage(const QString &id, const enum CacheClass cacheClass);
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

#endif
