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
    qchdmanApplication.setWindowIcon(QIcon(":/images/qchdman.png"));

    QCoreApplication::setOrganizationName(QCHDMAN_ORG_NAME);
    QCoreApplication::setOrganizationDomain(QCHDMAN_ORG_DOMAIN);
    QCoreApplication::setApplicationName(QCHDMAN_APP_NAME);

    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, QCHDMAN_DYN_DOT_PATH);
    globalConfig = new Settings();
    globalConfig->setApplicationVersion(QCHDMAN_APP_VERSION);

    // FIXME: load a Qt translation as well
    QString language = globalConfig->preferencesLanguage();
    if ( !globalConfig->languageMap.contains(language) )
        language = "us";
    QTranslator qchdmanTranslator;
    if ( qchdmanTranslator.load(QString("qchdman_%1").arg(language), ":/translations") )
        qchdmanApplication.installTranslator(&qchdmanTranslator);

    MainWindow w;
    mainWindow = &w;
    mainWindow->show();

    return qchdmanApplication.exec();
}
