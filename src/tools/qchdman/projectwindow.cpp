#include <QtGui>
#include <QMessageBox>
#include <QMenu>

#include "projectwindow.h"
#include "projectwidget.h"
#include "scriptwidget.h"
#include "mainwindow.h"
#include "macros.h"
#include "qchdmansettings.h"

extern QtChdmanGuiSettings *globalConfig;
extern MainWindow *mainWindow;

ProjectWindow::ProjectWindow(QString pn, int type, QWidget *parent) :
	QMdiSubWindow(parent)
{
	closeOk = true;
	subWindowType = type;

	if ( pn.isEmpty() )
		projectName = tr("Noname-%1").arg(mainWindow->nextProjectID++);
	else
		projectName = pn;

	if ( subWindowType == QCHDMAN_MDI_PROJECT ) {
		projectWidget = new ProjectWidget(this);
		setWidget(projectWidget);
		setWindowTitle(projectName);
		scriptWidget = NULL;
	} else if ( subWindowType == QCHDMAN_MDI_SCRIPT ) {
		scriptWidget = new ScriptWidget(this);
		setWidget(scriptWidget);
		setWindowTitle(projectName);
		projectWidget = NULL;
	}

	connect(this, SIGNAL(windowStateChanged(Qt::WindowStates, Qt::WindowStates)), this, SLOT(myWindowStateChanged(Qt::WindowStates, Qt::WindowStates)));
	connect(systemMenu(), SIGNAL(triggered(QAction *)), this, SLOT(systemMenuAction(QAction *)));
	mainWindow->enableActions();
}

ProjectWindow::~ProjectWindow()
{
	int windowCount = mainWindow->mdiArea()->subWindowList().count();
	if ( windowCount == 1 )
		mainWindow->disableActions();
	else if ( windowCount == 2 )
		mainWindow->disableActionsRequiringTwo();
}

void ProjectWindow::setProjectName(QString newName)
{
	projectName = newName;
	setWindowTitle(projectName);
}

void ProjectWindow::myWindowStateChanged(Qt::WindowStates oldState, Qt::WindowStates newState)
{
	if ( subWindowType == QCHDMAN_MDI_PROJECT ) {
		if ( newState == Qt::WindowNoState && oldState == Qt::WindowActive)
			return;

		if ( newState == Qt::WindowActive && oldState == Qt::WindowNoState )
			return;

		projectWidget->on_comboBoxProjectType_currentIndexChanged(-1);
	}
}

void ProjectWindow::systemMenuAction(QAction *action)
{
	// this works arounds a known Qt bug when a QMdiArea is in tabbed mode and the restore system-menu action is triggered... we must switch to windowed-mode in this case!
	if ( globalConfig->mainWindowViewMode() == QCHDMAN_VIEWMODE_TABBED ) {
		if ( !systemMenu()->actions().isEmpty() )
			if ( action == systemMenu()->actions().first() )
				QTimer::singleShot(0, mainWindow, SLOT(on_actionWindowViewModeWindowed_triggered()));
	}
}

void ProjectWindow::closeEvent(QCloseEvent *e)
{
	closeOk = true;

	if ( subWindowType == QCHDMAN_MDI_PROJECT ) {
		if ( mainWindow->forceQuit ) {
			if ( projectWidget->chdmanProc ) {
				while ( projectWidget->chdmanProc->state() == QProcess::Running && !projectWidget->chdmanProc->waitForFinished(QCHDMAN_KILL_WAIT) ) {
					projectWidget->chdmanProc->kill();
					qApp->processEvents();
				}
			}
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
					while ( projectWidget->chdmanProc->state() == QProcess::Running && !projectWidget->chdmanProc->waitForFinished(QCHDMAN_KILL_WAIT) ) {
						projectWidget->chdmanProc->kill();
						qApp->processEvents();
					}
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
	} else if ( subWindowType == QCHDMAN_MDI_SCRIPT ) {
		if ( mainWindow->forceQuit ) {
			scriptWidget->saveSettings();
			scriptWidget->engine()->stopScript();
			e->accept();
			deleteLater();
			return;
		}

		if ( !mainWindow->closeOk ) {
			e->ignore();
			return;
		}

		if ( scriptWidget->isRunning ) {
			switch ( QMessageBox::question(this, tr("Confirm"),
						       tr("Script '%1' is currently running.\n\nProceed?").arg(windowTitle()),
						       QMessageBox::Yes | QMessageBox::No, QMessageBox::No) ) {
			case QMessageBox::Yes:
				scriptWidget->engine()->stopScript();
				closeOk = true;
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

		if ( closeOk )
			scriptWidget->saveSettings();
	}

	if ( closeOk ) {
		e->accept();
		deleteLater();
	} else
		e->ignore();
}
