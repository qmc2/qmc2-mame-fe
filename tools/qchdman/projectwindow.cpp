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
    if ( pn.isEmpty() )
        projectName = tr("Noname-%1").arg(mW->nextProjectID++);
    else
        projectName = pn;

    setWindowTitle(projectName);

    projectWidget = new ProjectWidget(this);
    layout()->addWidget(projectWidget);
}

ProjectWindow::~ProjectWindow()
{
}

void ProjectWindow::showEvent(QShowEvent *e)
{
    QMdiSubWindow::showEvent(e);
}

void ProjectWindow::hideEvent(QHideEvent *e)
{
    QMdiSubWindow::hideEvent(e);
}

void ProjectWindow::closeEvent(QCloseEvent *e)
{
    globalConfig->setProjectWidgetSplitterSizes(projectWidget->splitterSizes());

    QMdiSubWindow::closeEvent(e);

    deleteLater();
}
