#include <QtCore>
#include <QDebug>

#include "syncer.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if ( argc > 3 ) {
        Syncer syncer(argv[1], argv[2], argv[3]);
        QTimer::singleShot(0, &syncer, SLOT(syncTemplates()));
        return a.exec();
    }

    qDebug() << "Usage:" << argv[0] << "<sourceTemplate> <targetTemplate> <language>";
    return 1;
}
