#include <QtGui>
#include <QApplication>

#include "mainwindow.h"
#include "settings.h"
#include "macros.h"

quint64 runningProjects = 0;
MainWindow *mainWindow = NULL;
Settings *globalConfig = NULL;

int main(int argc, char *argv[])
{
    QApplication qchdmanApplication(argc, argv);

    QCoreApplication::setOrganizationName(QCHDMAN_ORG_NAME);
    QCoreApplication::setOrganizationDomain(QCHDMAN_ORG_DOMAIN);
    QCoreApplication::setApplicationName(QCHDMAN_APP_NAME);

    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, QCHDMAN_DYN_DOT_PATH);
    globalConfig = new Settings();
    globalConfig->setApplicationVersion(QCHDMAN_APP_VERSION);

    MainWindow w;
    mainWindow = &w;
    mainWindow->show();

    return qchdmanApplication.exec();
}
