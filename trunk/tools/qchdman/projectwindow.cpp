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

    closeOk = true;
    projectWidget = new ProjectWidget(this);
    setWidget(projectWidget);
}

ProjectWindow::~ProjectWindow()
{
}

void ProjectWindow::closeEvent(QCloseEvent *e)
{
    closeOk = true;

    if ( projectWidget->chdmanProc ) {
        if ( projectWidget->chdmanProc->state() == QProcess::Running && mainWindow->closeOk ) {
            switch ( QMessageBox::question(this, tr("Confirm"),
                                           tr("Project '%1' is currently running.\nClosing its window will kill the external process!\n\nProceed?").arg(windowTitle()),
                                           QMessageBox::Yes | QMessageBox::No, QMessageBox::No) ) {
            case QMessageBox::Yes:
                projectWidget->chdmanProc->kill();
                projectWidget->chdmanProc->waitForFinished();
                break;
            case QMessageBox::No:
            default:
                closeOk = false;
                mainWindow->closeOk = false;
                break;
            }
        } else
            closeOk = mainWindow->closeOk;
    }

    if ( closeOk ) {
        e->accept();
        deleteLater();
    } else
        e->ignore();
}
