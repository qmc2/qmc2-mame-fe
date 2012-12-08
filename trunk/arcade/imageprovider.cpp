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
}

ImageProvider::~ImageProvider()
{
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
            if ( globalConfig->previewsZipped() ) {
                // FIXME: add support for zipped previews
                QMC2_LOG_STR(QObject::tr("WARNING: ImageProvider::requestImage(): zipped previews are not supported yet"));
                image.load(QLatin1String(":/images/ghost.png"));
            } else {
                fileName = QFileInfo(globalConfig->previewFolder() + "/" + gameId + ".png").absoluteFilePath();
                if ( !image.load(fileName) )
                    image.load(QLatin1String(":/images/ghost.png"));
            }
        } else if ( imageType == "fly" ) {
            if ( globalConfig->flyersZipped() ) {
                // FIXME: add support for zipped flyers
                QMC2_LOG_STR(QObject::tr("WARNING: ImageProvider::requestImage(): zipped flyers are not supported yet"));
                image.load(QLatin1String(":/images/ghost.png"));
            } else {
                fileName = QFileInfo(globalConfig->flyerFolder() + "/" + gameId + ".png").absoluteFilePath();
                if ( !image.load(fileName) )
                    image.load(QLatin1String(":/images/ghost.png"));
            }
        } else if ( imageType == "cab" ) {
            if ( globalConfig->cabinetsZipped() ) {
                // FIXME: add support for zipped cabinets
                QMC2_LOG_STR(QObject::tr("WARNING: ImageProvider::requestImage(): zipped cabinets are not supported yet"));
                image.load(QLatin1String(":/images/ghost.png"));
            } else {
                fileName = QFileInfo(globalConfig->cabinetFolder() + "/" + gameId + ".png").absoluteFilePath();
                if ( !image.load(fileName) )
                    image.load(QLatin1String(":/images/ghost.png"));
            }
        } else if ( imageType == "ctl" ) {
            if ( globalConfig->controllersZipped() ) {
                // FIXME: add support for zipped controllers
                QMC2_LOG_STR(QObject::tr("WARNING: ImageProvider::requestImage(): zipped controllers are not supported yet"));
                image.load(QLatin1String(":/images/ghost.png"));
            } else {
                fileName = QFileInfo(globalConfig->controllerFolder() + "/" + gameId + ".png").absoluteFilePath();
                if ( !image.load(fileName) )
                    image.load(QLatin1String(":/images/ghost.png"));
            }
        } else if ( imageType == "mrq" ) {
            if ( globalConfig->marqueesZipped() ) {
                // FIXME: add support for zipped marquees
                QMC2_LOG_STR(QObject::tr("WARNING: ImageProvider::requestImage(): zipped marquees are not supported yet"));
                image.load(QLatin1String(":/images/ghost.png"));
            } else {
                fileName = QFileInfo(globalConfig->marqueeFolder() + "/" + gameId + ".png").absoluteFilePath();
                if ( !image.load(fileName) )
                    image.load(QLatin1String(":/images/ghost.png"));
            }
        } else if ( imageType == "ttl" ) {
            if ( globalConfig->titlesZipped() ) {
                // FIXME: add support for zipped titles
                QMC2_LOG_STR(QObject::tr("WARNING: ImageProvider::requestImage(): zipped titles are not supported yet"));
                image.load(QLatin1String(":/images/ghost.png"));
            } else {
                fileName = QFileInfo(globalConfig->titleFolder() + "/" + gameId + ".png").absoluteFilePath();
                if ( !image.load(fileName) )
                    image.load(QLatin1String(":/images/ghost.png"));
            }
        } else if ( imageType == "pcb" ) {
            if ( globalConfig->pcbsZipped() ) {
                // FIXME: add support for zipped PCBs
                QMC2_LOG_STR(QObject::tr("WARNING: ImageProvider::requestImage(): zipped PCBs are not supported yet"));
                image.load(QLatin1String(":/images/ghost.png"));
            } else {
                fileName = QFileInfo(globalConfig->pcbFolder() + "/" + gameId + ".png").absoluteFilePath();
                if ( !image.load(fileName) )
                    image.load(QLatin1String(":/images/ghost.png"));
            }
        } else {
            QMC2_LOG_STR(QObject::tr("WARNING: ImageProvider::requestImage(): invalid image type '%1' requested").arg(imageType));
            image.load(QLatin1String(":/images/ghost.png"));
        }
    } else {
        QMC2_LOG_STR(QObject::tr("WARNING: ImageProvider::requestImage(): invalid image ID '%1' requested").arg(id));
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
    QPixmap result;
    return result;
}
