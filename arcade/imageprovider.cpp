#include <QImage>
#include <QPixmap>

#include "imageprovider.h"
#include "arcadesettings.h"
#include "consolewindow.h"
#include "macros.h"

extern ArcadeSettings *globalConfig;
extern ConsoleWindow *consoleWindow;

#if QT_VERSION < 0x050000
ImageProvider::ImageProvider(QDeclarativeImageProvider::ImageType type)
    : QDeclarativeImageProvider(type)
#else
ImageProvider::ImageProvider(QQuickImageProvider::ImageType type)
    : QQuickImageProvider(type)
#endif
{
    mImageTypes << "prv" << "fly" << "cab" << "ctl" << "mrq" << "ttl" << "pcb";
    mImageCache.setMaxCost(QMC2_ARCADE_IMGCACHE_SIZE);
    mPixmapCache.setMaxCost(QMC2_ARCADE_IMGCACHE_SIZE);
    foreach (QString imageType, mImageTypes) {
        if ( isZippedImageType(imageType) ) {
            mZipFileMap[imageType] = unzOpen((const char *)imageTypeToZipFile(imageType).toLocal8Bit());
            if ( !mZipFileMap[imageType] ) {
                QMC2_ARCADE_LOG_STR(QString("WARNING: Can't open %1 ZIP file '%2'").arg(imageTypeToLongName(imageType)).arg(imageTypeToZipFile(imageType)));
            }
        }
        QStringList activeFormats = globalConfig->activeImageFormats(imageType);
        if ( activeFormats.isEmpty() )
            mActiveFormatsMap[imageType] << QMC2_ARCADE_IMAGE_FORMAT_INDEX_PNG;
        else for (int i = 0; i < activeFormats.count(); i++)
            mActiveFormatsMap[imageType]<< activeFormats[i].toInt();
    }
    mFormatExtensions << "png" << "bmp" << "gif" << "jpg:jpeg" << "pbm" << "pgm" << "ppm" << "tif:tiff" << "xbm" << "xpm" << "svg" << "tga";
}

ImageProvider::~ImageProvider()
{
    foreach (unzFile zipFile, mZipFileMap)
        unzClose(zipFile);
}

QImage ImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    QImage image, result;
    QString cacheKey = "";
    if (id != "")
       cacheKey = loadImage(id, CacheClassImage);
    if (cacheKey != "")
       image = *mImageCache.object(cacheKey);
    else
       image.load(QLatin1String(":/images/ghost.png"));
    if ( requestedSize.isValid() )
        result = image.scaled(requestedSize, Qt::KeepAspectRatio);
    else
        result = image;
    if ( size )
        *size = result.size();
    return result;
}

QPixmap ImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    QPixmap image, result;
    QString cacheKey = "";
    if (id != "")
       cacheKey = loadImage(id, CacheClassPixmap);
    if (cacheKey != "")
       image = *mPixmapCache.object(cacheKey);
    else
       image.load(QLatin1String(":/images/ghost.png"));
    if ( requestedSize.isValid() )
        result = image.scaled(requestedSize, Qt::KeepAspectRatio);
    else
        result = image;
    if ( size )
        *size = result.size();
    return result;
}

QString ImageProvider::loadImage(const QString &id)
{
    if (id == "")
        return "";

    QString cacheKey;
    cacheKey = loadImage(id, CacheClassImage);
    if (cacheKey != "") return cacheKey;
    cacheKey = loadImage(id, CacheClassPixmap);
    if (cacheKey != "") return cacheKey;

    return "";
}

//don't cache missing
QString ImageProvider::loadImage(const QString &id, const enum CacheClass cacheClass)
{
    if (id == "")
        return "";

    QString validCacheKey = "";
    QStringList idWords = id.split("/", QString::SkipEmptyParts);
    if ( idWords.count() == 0 ) {
        QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: ImageProvider::loadImage(): invalid image ID '%1' requested").arg(id));
    } else {
        QString imageType = idWords[0]; 
        QString gameId = idWords[1];
        QString cacheKey = imageType + "-" + gameId;
        switch (cacheClass) {
            case CacheClassImage :
                if ( mImageCache.contains(cacheKey) )
                    validCacheKey = cacheKey;
                else {
                    QImage image;
                    if ( isZippedImageType(imageType) ) {
                        QString imageFileName = gameId + ".png";
                        unzFile imageFile = mZipFileMap[imageType];
                        if ( imageFile && unzLocateFile(imageFile, (const char *)imageFileName.toLocal8Bit(), 0) != UNZ_OK ) {
                            QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: ImageProvider::loadImage(): unable to load image file '%1' from ZIP").arg(imageFileName));
                        } else {
                            QByteArray imageData;
                            char imageBuffer[QMC2_ARCADE_ZIP_BUFSIZE];
                            if ( unzOpenCurrentFile(imageFile) != UNZ_OK ) {
                                QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: ImageProvider::loadImage(): unable to load image file '%1' from ZIP").arg(imageFileName));
                            } else {
                                int len = 0;
                                while ( (len = unzReadCurrentFile(imageFile, &imageBuffer, QMC2_ARCADE_ZIP_BUFSIZE)) > 0 ) {
                                    for (int i = 0; i < len; i++)
                                        imageData += imageBuffer[i];
                                }
                                unzCloseCurrentFile(imageFile);
                                if ( image.loadFromData(imageData, "PNG") )  {
                                    mImageCache.insert(cacheKey, new QImage(image));
                                    validCacheKey = cacheKey;
                                }
                            }
                        }
                    } else {
                        QStringList imageFolders = imageFolder(imageType).split(";", QString::SkipEmptyParts);
                        if ( imageFolders.isEmpty() ) {
                            QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: ImageProvider::loadImage(): invalid image type '%1' requested").arg(imageType));
                        } else {
                            foreach (QString folder, imageFolders) {
                                QString fileName = QFileInfo(folder + "/" + gameId + ".png").absoluteFilePath();
                                QImage image;
                                if ( image.load(fileName) ) {
                                     mImageCache.insert(cacheKey, new QImage(image));
                                     validCacheKey = cacheKey;
                                }
                            }
                        }
                    }
                }        
                break;
            case CacheClassPixmap :
                if ( mPixmapCache.contains(cacheKey) )
                    validCacheKey = cacheKey;
                else {
                    QPixmap image;
                    if ( isZippedImageType(imageType) ) {
                        QString imageFileName = gameId + ".png";
                        unzFile imageFile = mZipFileMap[imageType];
                        if ( imageFile && unzLocateFile(imageFile, (const char *)imageFileName.toLocal8Bit(), 0) != UNZ_OK ) {
                            QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: ImageProvider::loadImage(): unable to load image file '%1' from ZIP").arg(imageFileName));
                        } else {
                            QByteArray imageData;
                            char imageBuffer[QMC2_ARCADE_ZIP_BUFSIZE];
                            if ( unzOpenCurrentFile(imageFile) != UNZ_OK ) {
                                QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: ImageProvider::loadImage(): unable to load image file '%1' from ZIP").arg(imageFileName));
                            } else {
                                int len = 0;
                                while ( (len = unzReadCurrentFile(imageFile, &imageBuffer, QMC2_ARCADE_ZIP_BUFSIZE)) > 0 ) {
                                    for (int i = 0; i < len; i++)
                                        imageData += imageBuffer[i];
                                }
                                unzCloseCurrentFile(imageFile);
                                if ( image.loadFromData(imageData, "PNG") )  {
                                    mPixmapCache.insert(cacheKey, new QPixmap(image));
                                    validCacheKey = cacheKey;
                                }
                            }
                        }
                    } else {
                        QStringList imageFolders = imageFolder(imageType).split(";", QString::SkipEmptyParts);
                        if ( imageFolders.isEmpty() ) {
                            QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: ImageProvider::loadImage(): invalid image type '%1' requested").arg(imageType));
                        } else {
                            foreach (QString folder, imageFolders) {
                                QString fileName = QFileInfo(folder + "/" + gameId + ".png").absoluteFilePath();
                                QPixmap image;
                                if ( image.load(fileName) ) {
                                     mPixmapCache.insert(cacheKey, new QPixmap(image));
                                     validCacheKey = cacheKey;
                                }
                            }
                        }
                    }
                }
                break;
        }
    }
    return validCacheKey;
}

QString ImageProvider::imageTypeToZipFile(QString type)
{
    switch ( mImageTypes.indexOf(type) ) {
    case 0:
        return globalConfig->previewZipFile();
    case 1:
        return globalConfig->flyerZipFile();
    case 2:
        return globalConfig->cabinetZipFile();
    case 3:
        return globalConfig->controllerZipFile();
    case 4:
        return globalConfig->marqueeZipFile();
    case 5:
        return globalConfig->titleZipFile();
    case 6:
        return globalConfig->pcbZipFile();
    default:
        return QString();
    }
}

QString ImageProvider::imageTypeToLongName(QString type)
{
    switch ( mImageTypes.indexOf(type) ) {
    case 0:
        return QObject::tr("preview");
    case 1:
        return QObject::tr("flyer");
    case 2:
        return QObject::tr("cabinet");
    case 3:
        return QObject::tr("controller");
    case 4:
        return QObject::tr("marquee");
    case 5:
        return QObject::tr("title");
    case 6:
        return QObject::tr("PCB");
    default:
        return QString();
    }
}

bool ImageProvider::isZippedImageType(QString type)
{
    switch ( mImageTypes.indexOf(type) ) {
    case 0:
        return globalConfig->previewsZipped();
    case 1:
        return globalConfig->flyersZipped();
    case 2:
        return globalConfig->cabinetsZipped();
    case 3:
        return globalConfig->controllersZipped();
    case 4:
        return globalConfig->marqueesZipped();
    case 5:
        return globalConfig->titlesZipped();
    case 6:
        return globalConfig->pcbsZipped();
    default:
        return false;
    }
}

QString ImageProvider::imageFolder(QString type)
{
    switch ( mImageTypes.indexOf(type) ) {
    case 0:
        return globalConfig->previewFolder();
    case 1:
        return globalConfig->flyerFolder();
    case 2:
        return globalConfig->cabinetFolder();
    case 3:
        return globalConfig->controllerFolder();
    case 4:
        return globalConfig->marqueeFolder();
    case 5:
        return globalConfig->titleFolder();
    case 6:
        return globalConfig->pcbFolder();
    default:
        return QString();
    }
}
