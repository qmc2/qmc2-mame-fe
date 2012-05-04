#include <QtGui>

#include "projectwindow.h"
#include "projectwidget.h"
#include "mainwindow.h"
#include "macros.h"
#include "settings.h"

extern Settings *globalConfig;
extern MainWindow *mW;

ProjectWindow::ProjectWindow(QString pn, QWidget *parent) :
    QMdiSubWindow(parent)
{
    firstShowEvent = true;

    if ( pn.isEmpty() )
        projectName = tr("Noname-%1").arg(mW->nextProjectID++);
    else
        projectName = pn;

    setWindowTitle(projectName);

    projectWidget = new ProjectWidget(this);
    setWidget(projectWidget);
}

ProjectWindow::~ProjectWindow()
{
}

void ProjectWindow::showEvent(QShowEvent *e)
{
    if ( firstShowEvent && globalConfig->mainWindowViewMode() == QCHDMAN_VIEWMODE_WINDOWED ) {
        adjustSize();
        firstShowEvent = false;
    }

    QMdiSubWindow::showEvent(e);
}

void ProjectWindow::hideEvent(QHideEvent *e)
{
    QMdiSubWindow::hideEvent(e);
}

void ProjectWindow::closeEvent(QCloseEvent *e)
{
    globalConfig->setProjectWidgetSplitterSizes(projectWidget->splitterSizes());

    if ( projectWidget->chdmanProc ) {
        if ( projectWidget->chdmanProc->state() == QProcess::Running ) {
             projectWidget->chdmanProc->kill();
             projectWidget->chdmanProc->waitForFinished();
        }
    }

    QMdiSubWindow::closeEvent(e);

    deleteLater();
}
