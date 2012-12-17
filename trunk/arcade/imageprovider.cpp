#include <QImage>
#include <QPixmap>

#include "imageprovider.h"
#include "arcadesettings.h"
#include "consolewindow.h"
#include "macros.h"

extern ArcadeSettings *globalConfig;
extern ConsoleWindow *consoleWindow;

ImageProvider::ImageProvider(QDeclarativeImageProvider::ImageType type)
    : QDeclarativeImageProvider(type)
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
    }
}

ImageProvider::~ImageProvider()
{
    foreach (unzFile zipFile, mZipFileMap)
        unzClose(zipFile);
}

QImage ImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    QImage image, result;
    QStringList idWords = id.split("/", QString::SkipEmptyParts);
    if ( idWords.count() > 1 ) {
        QString imageType = idWords[0];
        QString gameId = idWords[1];
        QString cacheKey = imageType + "-" + gameId;
        if ( mImageCache.contains(cacheKey) )
            image = *mImageCache.object(cacheKey);
        else if ( isZippedImageType(imageType) )
            image = loadZippedImage(imageType, gameId);
        else {
            QStringList imageFolders = imageFolder(imageType).split(";", QString::SkipEmptyParts);
            if ( !imageFolders.isEmpty() ) {
                foreach (QString folder, imageFolders) {
                    QString fileName = QFileInfo(folder + "/" + gameId + ".png").absoluteFilePath();
                    if ( !image.load(fileName) )
                        image.load(QLatin1String(":/images/ghost.png"));
                    else
                        break;
                }
                mImageCache.insert(cacheKey, new QImage(image));
            } else {
                QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: ImageProvider::requestImage(): invalid image type '%1' requested").arg(imageType));
                image.load(QLatin1String(":/images/ghost.png"));
            }
        }
    } else {
        QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: ImageProvider::requestImage(): invalid image ID '%1' requested").arg(id));
        image.load(QLatin1String(":/images/ghost.png"));
    }
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
    QStringList idWords = id.split("/", QString::SkipEmptyParts);
    if ( idWords.count() > 1 ) {
        QString imageType = idWords[0];
        QString gameId = idWords[1];
        QString cacheKey = imageType + "-" + gameId;
        if ( mPixmapCache.contains(cacheKey) )
            image = *mPixmapCache.object(cacheKey);
        else if ( isZippedImageType(imageType) )
            image = loadZippedPixmap(imageType, gameId);
        else {
            QStringList imageFolders = imageFolder(imageType).split(";", QString::SkipEmptyParts);
            if ( !imageFolders.isEmpty() ) {
                foreach (QString folder, imageFolders) {
                    QString fileName = QFileInfo(folder + "/" + gameId + ".png").absoluteFilePath();
                    if ( !image.load(fileName) )
                        image.load(QLatin1String(":/images/ghost.png"));
                    else
                        break;
                }
                mPixmapCache.insert(cacheKey, new QPixmap(image));
            } else {
                QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: ImageProvider::requestImage(): invalid image type '%1' requested").arg(imageType));
                image.load(QLatin1String(":/images/ghost.png"));
            }
        }
    } else {
        QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: ImageProvider::requestImage(): invalid image ID '%1' requested").arg(id));
        image.load(QLatin1String(":/images/ghost.png"));
    }
    if ( requestedSize.isValid() )
        result = image.scaled(requestedSize, Qt::KeepAspectRatio);
    else
        result = image;
    if ( size )
        *size = result.size();
    return result;
}

QImage ImageProvider::loadZippedImage(QString imageType, QString id)
{
    QString cacheKey = imageType + "-" + id;
    if ( mImageCache.contains(cacheKey) )
        return *mImageCache.object(cacheKey);
    QImage zippedImage;
    QString imageFileName = id + ".png";
    unzFile imageFile = mZipFileMap[imageType];
    if ( imageFile && unzLocateFile(imageFile, (const char *)imageFileName.toLocal8Bit(), 0) == UNZ_OK ) {
        QByteArray imageData;
        char imageBuffer[QMC2_ARCADE_ZIP_BUFSIZE];
        if ( unzOpenCurrentFile(imageFile) == UNZ_OK ) {
            int len = 0;
            while ( (len = unzReadCurrentFile(imageFile, &imageBuffer, QMC2_ARCADE_ZIP_BUFSIZE)) > 0 ) {
                for (int i = 0; i < len; i++)
                    imageData += imageBuffer[i];
            }
            unzCloseCurrentFile(imageFile);
            zippedImage.loadFromData(imageData, "PNG");
        } else
            zippedImage.load(QLatin1String(":/images/ghost.png"));
        mImageCache.insert(cacheKey, new QImage(zippedImage));
    } else
        zippedImage.load(QLatin1String(":/images/ghost.png"));
    return zippedImage;
}

QPixmap ImageProvider::loadZippedPixmap(QString imageType, QString id)
{
    QString cacheKey = imageType + "-" + id;
    if ( mPixmapCache.contains(cacheKey) )
        return *mPixmapCache.object(cacheKey);
    QPixmap zippedImage;
    QString imageFileName = id + ".png";
    unzFile imageFile = mZipFileMap[imageType];
    if ( imageFile && unzLocateFile(imageFile, (const char *)imageFileName.toLocal8Bit(), 0) == UNZ_OK ) {
        QByteArray imageData;
        char imageBuffer[QMC2_ARCADE_ZIP_BUFSIZE];
        if ( unzOpenCurrentFile(imageFile) == UNZ_OK ) {
            int len = 0;
            while ( (len = unzReadCurrentFile(imageFile, &imageBuffer, QMC2_ARCADE_ZIP_BUFSIZE)) > 0 ) {
                for (int i = 0; i < len; i++)
                    imageData += imageBuffer[i];
            }
            unzCloseCurrentFile(imageFile);
            zippedImage.loadFromData(imageData, "PNG");
        } else
            zippedImage.load(QLatin1String(":/images/ghost.png"));
        mPixmapCache.insert(cacheKey, new QPixmap(zippedImage));
    } else
        zippedImage.load(QLatin1String(":/images/ghost.png"));
    return zippedImage;
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
