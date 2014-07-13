#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QApplication>
#include <QTimer>

#include "imageprovider.h"
#include "arcadesettings.h"
#include "consolewindow.h"
#include "macros.h"

extern ArcadeSettings *globalConfig;
extern ConsoleWindow *consoleWindow;
extern int emulatorMode;

#if QT_VERSION < 0x050000
ImageProvider::ImageProvider(QDeclarativeImageProvider::ImageType type, QObject *parent)
    : QObject(parent), QDeclarativeImageProvider(type)
#else
ImageProvider::ImageProvider(QQuickImageProvider::ImageType type, QObject *parent)
    : QObject(parent), QQuickImageProvider(type)
#endif
{
    mImageTypes << "prv" << "fly" << "cab" << "ctl" << "mrq" << "ttl" << "pcb" << "sws" << "ico";
    mImageCache.setMaxCost(QMC2_ARCADE_IMGCACHE_SIZE);
    mPixmapCache.setMaxCost(QMC2_ARCADE_IMGCACHE_SIZE);
    foreach (QString imageType, mImageTypes) {
        mAsyncMap[imageType] = false;
        if ( isZippedImageType(imageType) ) {
            mFileMapZip[imageType] = unzOpen((const char *)imageTypeToFile(imageType).toLocal8Bit());
            if ( !mFileMapZip[imageType] ) {
                QMC2_ARCADE_LOG_STR(QString("WARNING: Can't open %1 ZIP file '%2'").arg(imageTypeToLongName(imageType)).arg(imageTypeToFile(imageType)));
            }
            emit imageDataUpdated(imageType);
        } else if ( isSevenZippedImageType(imageType) ) {
            SevenZipFile *imageFile = new SevenZipFile(imageTypeToFile(imageType));
            if ( imageFile->open() ) {
                connect(imageFile, SIGNAL(dataReady()), this, SLOT(sevenZipDataReady()));
                mFileMap7z[imageType] = imageFile;
            } else {
                QMC2_ARCADE_LOG_STR(QString("WARNING: Can't open %1 7z file '%2'").arg(imageTypeToLongName(imageType)).arg(imageTypeToFile(imageType)));
                delete imageFile;
            }
        } else
            emit imageDataUpdated(imageType);
        QStringList activeFormats = globalConfig->activeImageFormats(imageType);
        if ( activeFormats.isEmpty() )
            mActiveFormatsMap[imageType] << QMC2_ARCADE_IMAGE_FORMAT_INDEX_PNG;
        else for (int i = 0; i < activeFormats.count(); i++)
            mActiveFormatsMap[imageType] << activeFormats[i].toInt();
    }
    // we support all formats for icons in this (hard-coded) order
    mActiveFormatsMap["ico"] << QMC2_ARCADE_IMAGE_FORMAT_INDEX_ICO << QMC2_ARCADE_IMAGE_FORMAT_INDEX_PNG << QMC2_ARCADE_IMAGE_FORMAT_INDEX_BMP << QMC2_ARCADE_IMAGE_FORMAT_INDEX_GIF << QMC2_ARCADE_IMAGE_FORMAT_INDEX_JPG
                             << QMC2_ARCADE_IMAGE_FORMAT_INDEX_PBM << QMC2_ARCADE_IMAGE_FORMAT_INDEX_PGM << QMC2_ARCADE_IMAGE_FORMAT_INDEX_PPM << QMC2_ARCADE_IMAGE_FORMAT_INDEX_TIFF << QMC2_ARCADE_IMAGE_FORMAT_INDEX_XBM
                             << QMC2_ARCADE_IMAGE_FORMAT_INDEX_XPM << QMC2_ARCADE_IMAGE_FORMAT_INDEX_SVG << QMC2_ARCADE_IMAGE_FORMAT_INDEX_TGA;
    mFormatExtensions << "png" << "bmp" << "gif" << "jpg, jpeg" << "pbm" << "pgm" << "ppm" << "tif, tiff" << "xbm" << "xpm" << "svg" << "tga" << "ico";
    mFormatNames << "PNG" << "BMP" << "GIF" << "JPG" << "PBM" << "PGM" << "PPM" << "TIFF" << "XBM" << "XPM" << "SVG" << "TGA" << "ICO";
}

ImageProvider::~ImageProvider()
{
    foreach (unzFile zipFile, mFileMapZip)
        unzClose(zipFile);

    foreach (SevenZipFile *sevenZipFile, mFileMap7z) {
        sevenZipFile->close();
        delete sevenZipFile;
    }
}

QImage ImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    QImage image, result;
    QString cacheKey, cachePrefix;

    if ( !id.isEmpty() ) {
       cacheKey = loadImage(id, CacheClassImage);
       cachePrefix = id.split("/", QString::SkipEmptyParts)[0];
    }

    if ( !cacheKey.isEmpty() ) {
        if ( mAsyncMap[cachePrefix] ) {
            if ( cachePrefix == "ico" ) {
                image = QImage(QSize(1, 1), QImage::Format_ARGB32);
                image.fill(Qt::transparent);
            } else {
                image.load(QLatin1String(":/images/ghost.png"));
                QPainter p;
                QString message = tr("Decompressing archive, please wait...");
                p.begin(&image);
                p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);
                QFont f(qApp->font());
                f.setWeight(QFont::Bold);
                f.setPointSize(f.pointSize() * 2);
                QFontMetrics fm(f);
                int adjustment = fm.height() / 2;
                p.setFont(f);
                QRect outerRect = p.boundingRect(image.rect(), Qt::AlignCenter | Qt::TextWordWrap, message).adjusted(-adjustment, -adjustment, adjustment, adjustment);
                QPainterPath pp;
                pp.addRoundedRect(outerRect, 5, 5);
                p.fillPath(pp, QBrush(QColor(0, 0, 0, 128), Qt::SolidPattern));
                p.setPen(QColor(255, 255, 0, 255));
                p.drawText(image.rect(), Qt::AlignCenter | Qt::TextWordWrap, message);
                p.end();
            }
        } else if ( mImageCache.contains(cacheKey) ) {
            image = *mImageCache.object(cacheKey);
        } else {
            cacheKey = loadImage(id, CacheClassImage);
            if ( !cacheKey.isEmpty() )
                image = *mImageCache.object(cacheKey);
            else if ( cachePrefix == "ico" ) {
                image = QImage(QSize(1, 1), QImage::Format_ARGB32);
                image.fill(Qt::transparent);
            } else
                image.load(QLatin1String(":/images/ghost.png"));
        }
    } else {
        if ( cachePrefix == "ico" ) {
            image = QImage(QSize(1, 1), QImage::Format_ARGB32);
            image.fill(Qt::transparent);
        } else
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
    QString cacheKey, cachePrefix;

    if ( !id.isEmpty() ) {
       cacheKey = loadImage(id, CacheClassPixmap);
       cachePrefix = id.split("/", QString::SkipEmptyParts)[0];
    }

    if ( !cacheKey.isEmpty() ) {
        if ( mAsyncMap[cachePrefix] ) {
            if ( cachePrefix == "ico" ) {
                image = QPixmap(QSize(1, 1));
                image.fill(Qt::transparent);
            } else {
                image.load(QLatin1String(":/images/ghost.png"));
                QPainter p;
                QString message = tr("Decompressing archive, please wait...");
                p.begin(&image);
                p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);
                QFont f(qApp->font());
                f.setWeight(QFont::Bold);
                f.setPointSize(f.pointSize() * 2);
                QFontMetrics fm(f);
                int adjustment = fm.height() / 2;
                p.setFont(f);
                QRect outerRect = p.boundingRect(image.rect(), Qt::AlignCenter | Qt::TextWordWrap, message).adjusted(-adjustment, -adjustment, adjustment, adjustment);
                QPainterPath pp;
                pp.addRoundedRect(outerRect, 5, 5);
                p.fillPath(pp, QBrush(QColor(0, 0, 0, 128), Qt::SolidPattern));
                p.setPen(QColor(255, 255, 0, 255));
                p.drawText(image.rect(), Qt::AlignCenter | Qt::TextWordWrap, message);
                p.end();
            }
        } else if ( mPixmapCache.contains(cacheKey) ) {
            image = *mPixmapCache.object(cacheKey);
        } else {
            cacheKey = loadImage(id, CacheClassPixmap);
            if ( !cacheKey.isEmpty() )
                image = *mPixmapCache.object(cacheKey);
            else if ( cachePrefix == "ico" ) {
                image = QPixmap(QSize(1, 1));
                image.fill(Qt::transparent);
            } else
                image.load(QLatin1String(":/images/ghost.png"));
        }
    } else {
        if ( cachePrefix == "ico" ) {
            image = QPixmap(QSize(1, 1));
            image.fill(Qt::transparent);
        } else
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

void ImageProvider::sevenZipDataReady()
{
    SevenZipFile *sevenZipFile = (SevenZipFile *)sender();
    if ( sevenZipFile ) {
        QString cachePrefix = sevenZipFile->userData().split("/", QString::SkipEmptyParts)[0];
        mAsyncMap[cachePrefix] = false;
        emit imageDataUpdated(cachePrefix);
    }
}

QString ImageProvider::loadImage(const QString &id)
{
    if ( id.isEmpty() )
        return QString();

    QString cacheKey;
    cacheKey = loadImage(id, CacheClassImage);
    if ( !cacheKey.isEmpty() )
        return cacheKey;

    cacheKey = loadImage(id, CacheClassPixmap);
    if ( !cacheKey.isEmpty() )
        return cacheKey;

    return QString();
}

QString ImageProvider::loadImage(const QString &id, const enum CacheClass cacheClass)
{
    if ( id.isEmpty() )
        return QString();

    QString validCacheKey;
    QStringList idWords = id.split("/", QString::SkipEmptyParts);
    if ( idWords.count() < 2 ) {
        if ( idWords.count() > 0 ) {
            if ( idWords[0] == "ghost" ) // allows using "image://qmc2/ghost" to retrieve the Ghostbusters image
                return QString();
            else
                QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: ImageProvider::loadImage(): invalid image ID '%1' requested").arg(id));
        } else
            QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: ImageProvider::loadImage(): invalid image ID '%1' requested").arg(id));
    } else {
        QString imageType = idWords[0];
        QString gameId = idWords[1];
        QString cacheKey = imageType + "/" + gameId;
        switch ( cacheClass ) {
            case CacheClassImage:
            if ( mImageCache.contains(cacheKey) )
                validCacheKey = cacheKey;
            else {
                QImage image;
                if ( isZippedImageType(imageType) ) {
                    unzFile imageFile = mFileMapZip[imageType];
                    foreach (int format, mActiveFormatsMap[imageType]) {
                        QString formatName = mFormatNames[format];
                        foreach (QString extension, mFormatExtensions[format].split(", ", QString::SkipEmptyParts)) {
                            QString imageFileName = gameId + "." + extension;
                            if ( imageFile && unzLocateFile(imageFile, imageFileName.toLocal8Bit().constData(), 0) == UNZ_OK ) {
                                QByteArray imageData;
                                char imageBuffer[QMC2_ARCADE_ZIP_BUFSIZE];
                                if ( unzOpenCurrentFile(imageFile) != UNZ_OK ) {
                                    QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: ImageProvider::loadImage(): unable to load image file '%1' from ZIP").arg(imageFileName));
				} else {
                                    int len = 0;
                                    while ( (len = unzReadCurrentFile(imageFile, &imageBuffer, QMC2_ARCADE_ZIP_BUFSIZE)) > 0 )
                                        imageData.append(imageBuffer, len);
                                    unzCloseCurrentFile(imageFile);
                                    if ( image.loadFromData(imageData, formatName.toLocal8Bit().constData()) )  {
                                        mImageCache.insert(cacheKey, new QImage(image));
                                        validCacheKey = cacheKey;
                                    }
                                }
                            }
                            if ( !validCacheKey.isEmpty() )
                                break;
                        }
                        if ( !validCacheKey.isEmpty() )
                            break;
                    }
                } else if ( isSevenZippedImageType(imageType) ) {
                    SevenZipFile *sevenZipFile = mFileMap7z[imageType];
                    foreach (int format, mActiveFormatsMap[imageType]) {
                        QString formatName = mFormatNames[format];
                        foreach (QString extension, mFormatExtensions[format].split(", ", QString::SkipEmptyParts)) {
                            QString imageFileName = gameId + "." + extension;
                            int index = sevenZipFile->indexOfName(imageFileName);
                            if ( index >= 0 ) {
                                QByteArray imageData;
                                bool async = true;
                                int readLength = sevenZipFile->read(index, &imageData, &async);
                                if ( readLength == 0 && async ) {
                                    validCacheKey = cacheKey;
                                    sevenZipFile->setUserData(cacheKey);
                                } else if ( image.loadFromData(imageData, formatName.toLocal8Bit().constData()) ) {
                                    mImageCache.insert(cacheKey, new QImage(image));
                                    validCacheKey = cacheKey;
                                }
                                mAsyncMap[imageType] = async;
                            }
                            if ( !validCacheKey.isEmpty() )
                                break;
                        }
                        if ( !validCacheKey.isEmpty() )
                            break;
                    }
                } else {
                    QStringList imageFolders = imageFolder(imageType).split(";", QString::SkipEmptyParts);
                    if ( imageFolders.isEmpty() ) {
                        QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: ImageProvider::loadImage(): invalid image type '%1' requested").arg(imageType));
                    } else {
                        foreach (QString folder, imageFolders) {
                            foreach (int format, mActiveFormatsMap[imageType]) {
                                foreach (QString extension, mFormatExtensions[format].split(", ", QString::SkipEmptyParts)) {
                                    QString fileName = QFileInfo(folder + "/" + gameId + "." + extension).absoluteFilePath();
                                    QImage image;
                                    if ( image.load(fileName) ) {
                                         mImageCache.insert(cacheKey, new QImage(image));
                                         validCacheKey = cacheKey;
                                    }
                                    if ( !validCacheKey.isEmpty() )
                                        break;
                                }
                                if ( !validCacheKey.isEmpty() )
                                    break;
                            }
                            if ( !validCacheKey.isEmpty() )
                                break;
                        }
                    }
                }
            }
            break;

            case CacheClassPixmap:
            if ( mPixmapCache.contains(cacheKey) )
                validCacheKey = cacheKey;
            else {
                QPixmap image;
                if ( isZippedImageType(imageType) ) {
                    unzFile imageFile = mFileMapZip[imageType];
                    foreach (int format, mActiveFormatsMap[imageType]) {
                        QString formatName = mFormatNames[format];
                        foreach (QString extension, mFormatExtensions[format].split(", ", QString::SkipEmptyParts)) {
                            QString imageFileName = gameId + "." + extension;
                            if ( imageFile && unzLocateFile(imageFile, imageFileName.toLocal8Bit().constData(), 0) == UNZ_OK ) {
                                QByteArray imageData;
                                char imageBuffer[QMC2_ARCADE_ZIP_BUFSIZE];
                                if ( unzOpenCurrentFile(imageFile) != UNZ_OK ) {
                                    QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: ImageProvider::loadImage(): unable to load image file '%1' from ZIP").arg(imageFileName));
				} else {
                                    int len = 0;
                                    while ( (len = unzReadCurrentFile(imageFile, &imageBuffer, QMC2_ARCADE_ZIP_BUFSIZE)) > 0 )
                                        imageData.append(imageBuffer, len);
                                    unzCloseCurrentFile(imageFile);
                                    if ( image.loadFromData(imageData, formatName.toLocal8Bit().constData()) )  {
                                        mPixmapCache.insert(cacheKey, new QPixmap(image));
                                        validCacheKey = cacheKey;
                                    }
                                }
                            }
                            if ( !validCacheKey.isEmpty() )
                                break;
                        }
                        if ( !validCacheKey.isEmpty() )
                            break;
                    }
                } else if ( isSevenZippedImageType(imageType) ) {
                    SevenZipFile *sevenZipFile = mFileMap7z[imageType];
                    foreach (int format, mActiveFormatsMap[imageType]) {
                        QString formatName = mFormatNames[format];
                        foreach (QString extension, mFormatExtensions[format].split(", ", QString::SkipEmptyParts)) {
                            QString imageFileName = gameId + "." + extension;
                            int index = sevenZipFile->indexOfName(imageFileName);
                            if ( index >= 0 ) {
                                QByteArray imageData;
                                bool async = true;
                                int readLength = sevenZipFile->read(index, &imageData, &async);
                                if ( readLength == 0 && async ) {
                                    validCacheKey = cacheKey;
                                    sevenZipFile->setUserData(cacheKey);
                                } else if ( image.loadFromData(imageData, formatName.toLocal8Bit().constData()) ) {
                                    mPixmapCache.insert(cacheKey, new QPixmap(image));
                                    validCacheKey = cacheKey;
                                }
                                mAsyncMap[imageType] = async;
                            }
                            if ( !validCacheKey.isEmpty() )
                                break;
                        }
                        if ( !validCacheKey.isEmpty() )
                            break;
                    }
                } else {
                    QStringList imageFolders = imageFolder(imageType).split(";", QString::SkipEmptyParts);
                    if ( imageFolders.isEmpty() ) {
                        QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: ImageProvider::loadImage(): invalid image type '%1' requested").arg(imageType));
                    } else {
                        foreach (QString folder, imageFolders) {
                            foreach (int format, mActiveFormatsMap[imageType]) {
                                foreach (QString extension, mFormatExtensions[format].split(", ", QString::SkipEmptyParts)) {
                                    QString fileName = QFileInfo(folder + "/" + gameId + "." + extension).absoluteFilePath();
                                    QPixmap image;
                                    if ( image.load(fileName) ) {
                                         mPixmapCache.insert(cacheKey, new QPixmap(image));
                                         validCacheKey = cacheKey;
                                    }
                                    if ( !validCacheKey.isEmpty() )
                                        break;
                                }
                                if ( !validCacheKey.isEmpty() )
                                    break;
                            }
                            if ( !validCacheKey.isEmpty() )
                                break;
                        }
                    }
                }
            }
            break;
        }
    }
    return validCacheKey;
}

QString ImageProvider::imageTypeToFile(QString type)
{
    switch ( mImageTypes.indexOf(type) ) {
    case QMC2_ARCADE_IMGTYPE_PREVIEW:
        return globalConfig->previewFile();
    case QMC2_ARCADE_IMGTYPE_FLYER:
        return globalConfig->flyerFile();
    case QMC2_ARCADE_IMGTYPE_CABINET:
        return globalConfig->cabinetFile();
    case QMC2_ARCADE_IMGTYPE_CONTROLLER:
        return globalConfig->controllerFile();
    case QMC2_ARCADE_IMGTYPE_MARQUEE:
        return globalConfig->marqueeFile();
    case QMC2_ARCADE_IMGTYPE_TITLE:
        return globalConfig->titleFile();
    case QMC2_ARCADE_IMGTYPE_PCB:
        return globalConfig->pcbFile();
    case QMC2_ARCADE_IMGTYPE_SWSNAP:
        return globalConfig->swSnapFile();
    case QMC2_ARCADE_IMGTYPE_ICON:
        return globalConfig->iconFile();
    default:
        return QString();
    }
}

QString ImageProvider::imageTypeToLongName(QString type)
{
    switch ( mImageTypes.indexOf(type) ) {
    case QMC2_ARCADE_IMGTYPE_PREVIEW:
        return QObject::tr("preview");
    case QMC2_ARCADE_IMGTYPE_FLYER:
        return QObject::tr("flyer");
    case QMC2_ARCADE_IMGTYPE_CABINET:
        return QObject::tr("cabinet");
    case QMC2_ARCADE_IMGTYPE_CONTROLLER:
        return QObject::tr("controller");
    case QMC2_ARCADE_IMGTYPE_MARQUEE:
        if ( emulatorMode == QMC2_ARCADE_EMUMODE_MESS )
            return QObject::tr("logo");
        else
            return QObject::tr("marquee");
    case QMC2_ARCADE_IMGTYPE_TITLE:
        return QObject::tr("title");
    case QMC2_ARCADE_IMGTYPE_PCB:
        return QObject::tr("PCB");
    case QMC2_ARCADE_IMGTYPE_SWSNAP:
        return QObject::tr("software snapshot");
    case QMC2_ARCADE_IMGTYPE_ICON:
        return QObject::tr("icon");
    default:
        return QString();
    }
}

bool ImageProvider::isZippedImageType(QString type)
{
    switch ( mImageTypes.indexOf(type) ) {
    case QMC2_ARCADE_IMGTYPE_PREVIEW:
        return globalConfig->previewsZipped();
    case QMC2_ARCADE_IMGTYPE_FLYER:
        return globalConfig->flyersZipped();
    case QMC2_ARCADE_IMGTYPE_CABINET:
        return globalConfig->cabinetsZipped();
    case QMC2_ARCADE_IMGTYPE_CONTROLLER:
        return globalConfig->controllersZipped();
    case QMC2_ARCADE_IMGTYPE_MARQUEE:
        return globalConfig->marqueesZipped();
    case QMC2_ARCADE_IMGTYPE_TITLE:
        return globalConfig->titlesZipped();
    case QMC2_ARCADE_IMGTYPE_PCB:
        return globalConfig->pcbsZipped();
    case QMC2_ARCADE_IMGTYPE_SWSNAP:
        return globalConfig->swSnapsZipped();
    case QMC2_ARCADE_IMGTYPE_ICON:
        return globalConfig->iconsZipped();
    default:
        return false;
    }
}

bool ImageProvider::isSevenZippedImageType(QString type)
{
    switch ( mImageTypes.indexOf(type) ) {
    case QMC2_ARCADE_IMGTYPE_PREVIEW:
        return globalConfig->previewsSevenZipped();
    case QMC2_ARCADE_IMGTYPE_FLYER:
        return globalConfig->flyersSevenZipped();
    case QMC2_ARCADE_IMGTYPE_CABINET:
        return globalConfig->cabinetsSevenZipped();
    case QMC2_ARCADE_IMGTYPE_CONTROLLER:
        return globalConfig->controllersSevenZipped();
    case QMC2_ARCADE_IMGTYPE_MARQUEE:
        return globalConfig->marqueesSevenZipped();
    case QMC2_ARCADE_IMGTYPE_TITLE:
        return globalConfig->titlesSevenZipped();
    case QMC2_ARCADE_IMGTYPE_PCB:
        return globalConfig->pcbsSevenZipped();
    case QMC2_ARCADE_IMGTYPE_SWSNAP:
        return globalConfig->swSnapsSevenZipped();
    case QMC2_ARCADE_IMGTYPE_ICON:
        return globalConfig->iconsSevenZipped();
    default:
        return false;
    }
}

QString ImageProvider::imageFolder(QString type)
{
    switch ( mImageTypes.indexOf(type) ) {
    case QMC2_ARCADE_IMGTYPE_PREVIEW:
        return globalConfig->previewFolder();
    case QMC2_ARCADE_IMGTYPE_FLYER:
        return globalConfig->flyerFolder();
    case QMC2_ARCADE_IMGTYPE_CABINET:
        return globalConfig->cabinetFolder();
    case QMC2_ARCADE_IMGTYPE_CONTROLLER:
        return globalConfig->controllerFolder();
    case QMC2_ARCADE_IMGTYPE_MARQUEE:
        return globalConfig->marqueeFolder();
    case QMC2_ARCADE_IMGTYPE_TITLE:
        return globalConfig->titleFolder();
    case QMC2_ARCADE_IMGTYPE_PCB:
        return globalConfig->pcbFolder();
    case QMC2_ARCADE_IMGTYPE_SWSNAP:
        return globalConfig->swSnapFolder();
    case QMC2_ARCADE_IMGTYPE_ICON:
        return globalConfig->iconFolder();
    default:
        return QString();
    }
}
