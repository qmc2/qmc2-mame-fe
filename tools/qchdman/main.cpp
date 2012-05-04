#include <QtGui/QApplication>
#include <QDir>
#include "mainwindow.h"
#include "settings.h"
#include "macros.h"

quint64 runningProjects = 0;
MainWindow *mW = NULL;
Settings *globalConfig = NULL;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName(QCHDMAN_ORG_NAME);
    QCoreApplication::setOrganizationDomain(QCHDMAN_ORG_DOMAIN);
    QCoreApplication::setApplicationName(QCHDMAN_APP_NAME);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, QCHDMAN_DYN_DOT_PATH);
    globalConfig = new Settings();

    MainWindow w;
    w.show();
    mW = &w;
    return a.exec();
}
