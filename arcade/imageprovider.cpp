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
        QString fileName;
        if ( imageType == "prv" ) {
            if ( globalConfig->previewsZipped() )
                return loadZippedImage(imageType, gameId);
            else {
                foreach (QString folder, globalConfig->previewFolder().split(";", QString::SkipEmptyParts)) {
                    fileName = QFileInfo(folder + "/" + gameId + ".png").absoluteFilePath();
                    if ( !image.load(fileName) )
                        image.load(QLatin1String(":/images/ghost.png"));
                    else
                        break;
                }
            }
        } else if ( imageType == "fly" ) {
            if ( globalConfig->flyersZipped() )
                return loadZippedImage(imageType, gameId);
            else {
                foreach (QString folder, globalConfig->flyerFolder().split(";", QString::SkipEmptyParts)) {
                    fileName = QFileInfo(folder + "/" + gameId + ".png").absoluteFilePath();
                    if ( !image.load(fileName) )
                        image.load(QLatin1String(":/images/ghost.png"));
                    else
                        break;
                }
            }
        } else if ( imageType == "cab" ) {
            if ( globalConfig->cabinetsZipped() )
                return loadZippedImage(imageType, gameId);
            else {
                foreach (QString folder, globalConfig->cabinetFolder().split(";", QString::SkipEmptyParts)) {
                    fileName = QFileInfo(folder + "/" + gameId + ".png").absoluteFilePath();
                    if ( !image.load(fileName) )
                        image.load(QLatin1String(":/images/ghost.png"));
                    else
                        break;
                }
            }
        } else if ( imageType == "ctl" ) {
            if ( globalConfig->controllersZipped() )
                return loadZippedImage(imageType, gameId);
            else {
                foreach (QString folder, globalConfig->controllerFolder().split(";", QString::SkipEmptyParts)) {
                    fileName = QFileInfo(folder + "/" + gameId + ".png").absoluteFilePath();
                    if ( !image.load(fileName) )
                        image.load(QLatin1String(":/images/ghost.png"));
                    else
                        break;
                }
            }
        } else if ( imageType == "mrq" ) {
            if ( globalConfig->marqueesZipped() )
                return loadZippedImage(imageType, gameId);
            else {
                foreach (QString folder, globalConfig->marqueeFolder().split(";", QString::SkipEmptyParts)) {
                    fileName = QFileInfo(folder + "/" + gameId + ".png").absoluteFilePath();
                    if ( !image.load(fileName) )
                        image.load(QLatin1String(":/images/ghost.png"));
                    else
                        break;
                }
            }
        } else if ( imageType == "ttl" ) {
            if ( globalConfig->titlesZipped() )
                return loadZippedImage(imageType, gameId);
            else {
                foreach (QString folder, globalConfig->titleFolder().split(";", QString::SkipEmptyParts)) {
                    fileName = QFileInfo(folder + "/" + gameId + ".png").absoluteFilePath();
                    if ( !image.load(fileName) )
                        image.load(QLatin1String(":/images/ghost.png"));
                    else
                        break;
                }
            }
        } else if ( imageType == "pcb" ) {
            if ( globalConfig->pcbsZipped() )
                return loadZippedImage(imageType, gameId);
            else {
                foreach (QString folder, globalConfig->pcbFolder().split(";", QString::SkipEmptyParts)) {
                    fileName = QFileInfo(folder + "/" + gameId + ".png").absoluteFilePath();
                    if ( !image.load(fileName) )
                        image.load(QLatin1String(":/images/ghost.png"));
                    else
                        break;
                }
            }
        } else {
            QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: ImageProvider::requestImage(): invalid image type '%1' requested").arg(imageType));
            image.load(QLatin1String(":/images/ghost.png"));
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

QPixmap ImageProvider::requestPixmap(const QString &/*id*/, QSize */*size*/, const QSize &/*requestedSize*/)
{
    // FIXME (however, this isn't used anyway)
    QPixmap result;
    return result;
}

QImage ImageProvider::loadZippedImage(QString imageType, QString id)
{
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
        }
        zippedImage.loadFromData(imageData, "PNG");
    } else
        zippedImage.load(QLatin1String(":/images/ghost.png"));
    return zippedImage;
}

QString ImageProvider::imageTypeToZipFile(QString type)
{
    if ( type == "prv" )
        return globalConfig->previewZipFile();
    else if ( type == "fly" )
        return globalConfig->flyerZipFile();
    else if ( type == "cab" )
        return globalConfig->cabinetZipFile();
    else if ( type == "ctl" )
        return globalConfig->controllerZipFile();
    else if ( type == "mrq" )
        return globalConfig->marqueeZipFile();
    else if ( type == "ttl" )
        return globalConfig->titleZipFile();
    else if ( type == "pcb" )
        return globalConfig->pcbZipFile();
    else
        return QString();
}

QString ImageProvider::imageTypeToLongName(QString type)
{
    if ( type == "prv" )
        return QObject::tr("preview");
    else if ( type == "fly" )
        return QObject::tr("flyer");
    else if ( type == "cab" )
        return QObject::tr("cabinet");
    else if ( type == "ctl" )
        return QObject::tr("controller");
    else if ( type == "mrq" )
        return QObject::tr("marquee");
    else if ( type == "ttl" )
        return QObject::tr("title");
    else if ( type == "pcb" )
        return QObject::tr("PCB");
    else
        return QString();
}

bool ImageProvider::isZippedImageType(QString type)
{
    if ( type == "prv" )
        return globalConfig->previewsZipped();
    else if ( type == "fly" )
        return globalConfig->flyersZipped();
    else if ( type == "cab" )
        return globalConfig->cabinetsZipped();
    else if ( type == "ctl" )
        return globalConfig->controllersZipped();
    else if ( type == "mrq" )
        return globalConfig->marqueesZipped();
    else if ( type == "ttl" )
        return globalConfig->titlesZipped();
    else if ( type == "pcb" )
        return globalConfig->pcbsZipped();
    else
        return false;
}
