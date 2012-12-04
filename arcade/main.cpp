#include <QApplication>
#include <QDir>

#include "arcadesettings.h"
#include "tweakedqmlappviewer.h"
#include "macros.h"

ArcadeSettings *globalConfig = NULL;

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QScopedPointer<QApplication> app(createApplication(argc, argv));

    QCoreApplication::setOrganizationName(QMC2_ARCADE_ORG_NAME);
    QCoreApplication::setOrganizationDomain(QMC2_ARCADE_ORG_DOMAIN);
    QCoreApplication::setApplicationName(QMC2_ARCADE_APP_NAME);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, QMC2_ARCADE_DYN_DOT_PATH);

    globalConfig = new ArcadeSettings("ToxicWaste");
    globalConfig->setApplicationVersion(QMC2_ARCADE_APP_VERSION);

    TweakedQmlApplicationViewer *viewer = new TweakedQmlApplicationViewer();
    viewer->setWindowTitle(QMC2_ARCADE_APP_TITLE);
    viewer->setOrientation(QmlApplicationViewer::ScreenOrientationAuto);

    if ( globalConfig->fullScreen() )
        viewer->switchToFullScreen();
    else
        viewer->switchToWindowed();

    viewer->setMainQmlFile(QLatin1String("qml/ToxicWaste/ToxicWaste.qml"));

    int returnCode = app->exec();

    delete viewer;
    delete globalConfig;

    return returnCode;
}
