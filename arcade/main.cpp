#include <QApplication>
#include "tweakedqmlappviewer.h"

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QScopedPointer<QApplication> app(createApplication(argc, argv));

    TweakedQmlApplicationViewer viewer;
    viewer.setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    viewer.setMainQmlFile(QLatin1String("qml/ToxicWaste/ToxicWaste.qml"));
    viewer.showExpanded();

    return app->exec();
}
