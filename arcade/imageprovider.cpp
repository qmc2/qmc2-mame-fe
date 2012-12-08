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
