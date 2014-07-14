#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include <qglobal.h>
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
#define QMC2_ARCADE_IMAGE_FORMAT_INDEX_ICO     12

#include "../minizip/unzip.h"
#include "../sevenzipfile.h"

#if QT_VERSION < 0x050000
#include <QDeclarativeImageProvider>

class ImageProvider : public QObject, public QDeclarativeImageProvider
{
    Q_OBJECT

public:
    explicit ImageProvider(QDeclarativeImageProvider::ImageType, QObject *parent = 0);
    virtual ~ImageProvider();

    QImage requestImage(const QString &, QSize *, const QSize &);
    QPixmap requestPixmap(const QString &, QSize *, const QSize &);
    QString loadImage(const QString &);

    enum CacheClass { CacheClassImage, CacheClassPixmap };

public slots:
    void sevenZipDataReady();

signals:
    void imageDataUpdated(QString);

private:
    QString loadImage(const QString &id, const enum CacheClass cacheClass);
    QString imageTypeToFile(QString);
    QString imageTypeToLongName(QString);
    bool isZippedImageType(QString);
    bool isSevenZippedImageType(QString);
    QString imageFolder(QString);
    bool isAsync(QString);

    QStringList mImageTypes;
    QMap<QString, QString> mFileTypeMap;
    QMap<QString, unzFile> mFileMapZip;
    QMap<QString, SevenZipFile *> mFileMap7z;
    QCache<QString, QImage> mImageCache;
    QCache<QString, QPixmap> mPixmapCache;
    QMap<QString, QList<int> > mActiveFormatsMap;
    QStringList mFormatExtensions;
    QStringList mFormatNames;
    QMap<QString, bool> mAsyncMap;
};
#else
#include <QQuickImageProvider>

class ImageProvider : public QObject, public QQuickImageProvider
{
    Q_OBJECT

public:
    explicit ImageProvider(QQuickImageProvider::ImageType, QObject *parent = 0);
    virtual ~ImageProvider();

    QImage requestImage(const QString &, QSize *, const QSize &);
    QPixmap requestPixmap(const QString &, QSize *, const QSize &);
    QString loadImage(const QString &);

    enum CacheClass { CacheClassImage, CacheClassPixmap };

public slots:
    void sevenZipDataReady();

signals:
    void imageDataUpdated(QString);

private:
    QString loadImage(const QString &id, const enum CacheClass cacheClass);
    QString imageTypeToFile(QString);
    QString imageTypeToLongName(QString);
    bool isZippedImageType(QString);
    bool isSevenZippedImageType(QString);
    QString imageFolder(QString);
    bool isAsync(QString);

    QStringList mImageTypes;
    QMap<QString, QString> mFileTypeMap;
    QMap<QString, unzFile> mFileMapZip;
    QMap<QString, SevenZipFile *> mFileMap7z;
    QCache<QString, QImage> mImageCache;
    QCache<QString, QPixmap> mPixmapCache;
    QMap<QString, QList<int> > mActiveFormatsMap;
    QStringList mFormatExtensions;
    QStringList mFormatNames;
    QMap<QString, bool> mAsyncMap;
};
#endif

#endif
