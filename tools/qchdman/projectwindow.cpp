#include <QtGui>
#include <QFileDialog>
#include <QMessageBox>

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
    closeOk = true;

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
    closeOk = true;

    if ( mainWindow->forceQuit ) {
        projectWidget->chdmanProc->kill();
        projectWidget->chdmanProc->waitForFinished();
        e->accept();
        deleteLater();
        return;
    }

    if ( !mainWindow->closeOk ) {
        e->ignore();
        return;
    }

    if ( projectWidget->chdmanProc ) {
        if ( projectWidget->chdmanProc->state() == QProcess::Running ) {
            switch ( QMessageBox::question(this, tr("Confirm"),
                                           tr("Project '%1' is currently running.\n\nClosing its window will kill the external process!\n\nProceed?").arg(windowTitle()),
                                           QMessageBox::Yes | QMessageBox::No, QMessageBox::No) ) {
            case QMessageBox::Yes:
                projectWidget->chdmanProc->kill();
                projectWidget->chdmanProc->waitForFinished();
                break;
            case QMessageBox::No:
            default:
                closeOk = false;
                mainWindow->closeOk = false;
                QTimer::singleShot(100, mainWindow, SLOT(resetCloseFlag()));
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
