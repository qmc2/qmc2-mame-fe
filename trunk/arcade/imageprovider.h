#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include <QMap>
#include <QCache>
#include <QList>
#include <QString>

#define QMC2_ARCADE_IMAGE_FORMAT_INDEX_PNG     0
#define QMC2_ARCADE_IMAGE_FORMAT_INDEX_BMP     1
#define QMC2_ARCADE_IMAGE_FORMAT_INDEX_GIF     2
#define QMC2_ARCADE_IMAGE_FORMAT_INDEX_JPG     3
#define QMC2_ARCADE_IMAGE_FORMAT_INDEX_PBM     4
#define QMC2_ARCADE_IMAGE_FORMAT_INDEX_PGM     5
#define QMC2_ARCADE_IMAGE_FORMAT_INDEX_PPM     6
#define QMC2_ARCADE_IMAGE_FORMAT_INDEX_TIFF    7
#define QMC2_ARCADE_IMAGE_FORMAT_INDEX_XBM     8
#define QMC2_ARCADE_IMAGE_FORMAT_INDEX_XPM     9
#define QMC2_ARCADE_IMAGE_FORMAT_INDEX_SVG     10
#define QMC2_ARCADE_IMAGE_FORMAT_INDEX_TGA     11

#include "../minizip/unzip.h"
#include "../sevenzipfile.h"

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
    QMap<QString, SevenZipFile *> mZipFileMap7z;
    QCache<QString, QImage> mImageCache;
    QCache<QString, QPixmap> mPixmapCache;
    QMap<QString, QList<int> > mActiveFormatsMap;
    QStringList mFormatExtensions;
    QStringList mFormatNames;
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
    QMap<QString, SevenZipFile *> mZipFileMap7z;
    QCache<QString, QImage> mImageCache;
    QCache<QString, QPixmap> mPixmapCache;
    QMap<QString, QList<int> > mActiveFormatsMap;
    QStringList mFormatExtensions;
    QStringList mFormatNames;
};
#endif

#endif
