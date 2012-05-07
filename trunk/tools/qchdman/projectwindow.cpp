#include <QtGui>

#include "projectwindow.h"
#include "projectwidget.h"
#include "mainwindow.h"
#include "macros.h"
#include "settings.h"

extern Settings *globalConfig;
extern MainWindow *mainWindow;

ProjectWindow::ProjectWindow(QString pn, QWidget *parent) :
    QMdiSubWindow(parent)
{
    if ( pn.isEmpty() )
        projectName = tr("Noname-%1").arg(mainWindow->nextProjectID++);
    else
        projectName = pn;

    setWindowTitle(projectName);

    projectWidget = new ProjectWidget(this);
    setWidget(projectWidget);
}

ProjectWindow::~ProjectWindow()
{
}

void ProjectWindow::closeEvent(QCloseEvent *e)
{
    if ( projectWidget->chdmanProc ) {
        if ( projectWidget->chdmanProc->state() == QProcess::Running ) {
             projectWidget->chdmanProc->kill();
             projectWidget->chdmanProc->waitForFinished();
        }
    }

    QMdiSubWindow::closeEvent(e);

    deleteLater();
}
