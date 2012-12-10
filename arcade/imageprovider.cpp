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
                QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: ImageProvider::requestImage(): zipped previews are not supported yet"));
                image.load(QLatin1String(":/images/ghost.png"));
            } else {
                foreach (QString folder, globalConfig->previewFolder().split(";", QString::SkipEmptyParts)) {
                    fileName = QFileInfo(folder + "/" + gameId + ".png").absoluteFilePath();
                    if ( !image.load(fileName) )
                        image.load(QLatin1String(":/images/ghost.png"));
                    else
                        break;
                }
            }
        } else if ( imageType == "fly" ) {
            if ( globalConfig->flyersZipped() ) {
                // FIXME: add support for zipped flyers
                QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: ImageProvider::requestImage(): zipped flyers are not supported yet"));
                image.load(QLatin1String(":/images/ghost.png"));
            } else {
                foreach (QString folder, globalConfig->flyerFolder().split(";", QString::SkipEmptyParts)) {
                    fileName = QFileInfo(folder + "/" + gameId + ".png").absoluteFilePath();
                    if ( !image.load(fileName) )
                        image.load(QLatin1String(":/images/ghost.png"));
                    else
                        break;
                }
            }
        } else if ( imageType == "cab" ) {
            if ( globalConfig->cabinetsZipped() ) {
                // FIXME: add support for zipped cabinets
                QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: ImageProvider::requestImage(): zipped cabinets are not supported yet"));
                image.load(QLatin1String(":/images/ghost.png"));
            } else {
                foreach (QString folder, globalConfig->cabinetFolder().split(";", QString::SkipEmptyParts)) {
                    fileName = QFileInfo(folder + "/" + gameId + ".png").absoluteFilePath();
                    if ( !image.load(fileName) )
                        image.load(QLatin1String(":/images/ghost.png"));
                    else
                        break;
                }
            }
        } else if ( imageType == "ctl" ) {
            if ( globalConfig->controllersZipped() ) {
                // FIXME: add support for zipped controllers
                QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: ImageProvider::requestImage(): zipped controllers are not supported yet"));
                image.load(QLatin1String(":/images/ghost.png"));
            } else {
                foreach (QString folder, globalConfig->controllerFolder().split(";", QString::SkipEmptyParts)) {
                    fileName = QFileInfo(folder + "/" + gameId + ".png").absoluteFilePath();
                    if ( !image.load(fileName) )
                        image.load(QLatin1String(":/images/ghost.png"));
                    else
                        break;
                }
            }
        } else if ( imageType == "mrq" ) {
            if ( globalConfig->marqueesZipped() ) {
                // FIXME: add support for zipped marquees
                QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: ImageProvider::requestImage(): zipped marquees are not supported yet"));
                image.load(QLatin1String(":/images/ghost.png"));
            } else {
                foreach (QString folder, globalConfig->marqueeFolder().split(";", QString::SkipEmptyParts)) {
                    fileName = QFileInfo(folder + "/" + gameId + ".png").absoluteFilePath();
                    if ( !image.load(fileName) )
                        image.load(QLatin1String(":/images/ghost.png"));
                    else
                        break;
                }
            }
        } else if ( imageType == "ttl" ) {
            if ( globalConfig->titlesZipped() ) {
                // FIXME: add support for zipped titles
                QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: ImageProvider::requestImage(): zipped titles are not supported yet"));
                image.load(QLatin1String(":/images/ghost.png"));
            } else {
                foreach (QString folder, globalConfig->titleFolder().split(";", QString::SkipEmptyParts)) {
                    fileName = QFileInfo(folder + "/" + gameId + ".png").absoluteFilePath();
                    if ( !image.load(fileName) )
                        image.load(QLatin1String(":/images/ghost.png"));
                    else
                        break;
                }
            }
        } else if ( imageType == "pcb" ) {
            if ( globalConfig->pcbsZipped() ) {
                // FIXME: add support for zipped PCBs
                QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: ImageProvider::requestImage(): zipped PCBs are not supported yet"));
                image.load(QLatin1String(":/images/ghost.png"));
            } else {
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
