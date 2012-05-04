#include <QtGui>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "projectwindow.h"
#include "macros.h"
#include "settings.h"

extern Settings *globalConfig;
extern quint64 runningProjects;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    preferencesDialog = NULL;

    restoreGeometry(globalConfig->mainWindowGeometry());
    restoreState(globalConfig->mainWindowState());

    setWindowTitle(QCHDMAN_TITLE + " " + QCHDMAN_VERSION);
    nextProjectID = 0;

    if ( globalConfig->mainWindowViewMode() == QCHDMAN_VIEWMODE_WINDOWED )
        on_actionWindowViewModeWindowed_triggered();
    else
        on_actionWindowViewModeTabbed_triggered();

    // check CHDMAN binary setting
    QFileInfo chdmanFileInfo(globalConfig->preferencesChdmanBinary());
    if ( globalConfig->preferencesChdmanBinary().isEmpty() || !chdmanFileInfo.isExecutable() ) {
        preferencesDialog = new PreferencesDialog(0);
        QTimer::singleShot(100, preferencesDialog, SLOT(initialSetup()));
    }

    statisticsLabel = new QLabel;
    statisticsLabel->setFrameStyle(QFrame::Raised | QFrame::StyledPanel);
    statusBar()->addPermanentWidget(statisticsLabel);
    updateStatus();
    connect(&statusTimer, SIGNAL(timeout()), this, SLOT(updateStatus()));
    statusTimer.start(QCHDMAN_STATUS_INTERVAL);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionProjectNew_triggered(bool)
{
    ProjectWindow *projectWindow = new ProjectWindow(QString(), ui->mdiArea);
    projectWindow->show();
}

void MainWindow::on_actionProjectLoad_triggered(bool)
{
}

void MainWindow::on_actionProjectSave_triggered(bool)
{
}

void MainWindow::on_actionProjectSaveAs_triggered(bool)
{
}

void MainWindow::on_actionProjectPreferences_triggered(bool)
{
    if ( !preferencesDialog )
        preferencesDialog = new PreferencesDialog(0);
    preferencesDialog->exec();
}

void MainWindow::on_actionProjectExit_triggered(bool)
{
    QTimer::singleShot(0, this, SLOT(close()));
}

void MainWindow::on_actionWindowNext_triggered(bool)
{
    ui->mdiArea->activateNextSubWindow();
}

void MainWindow::on_actionWindowPrevious_triggered(bool)
{
    ui->mdiArea->activatePreviousSubWindow();
}

void MainWindow::on_actionWindowTile_triggered(bool)
{
    ui->mdiArea->tileSubWindows();
}

void MainWindow::on_actionWindowCascade_triggered(bool)
{
    ui->mdiArea->cascadeSubWindows();
}

void MainWindow::on_actionWindowClose_triggered(bool)
{
    ui->mdiArea->closeActiveSubWindow();
}

void MainWindow::on_actionWindowCloseAll_triggered(bool)
{
    ui->mdiArea->closeAllSubWindows();
}

void MainWindow::on_actionWindowViewModeWindowed_triggered(bool)
{
    ui->actionWindowViewModeWindowed->setChecked(true);
    ui->actionWindowViewModeTabbed->setChecked(false);
    globalConfig->setMainWindowViewMode(QCHDMAN_VIEWMODE_WINDOWED);
    ui->mdiArea->setViewMode(QCHDMAN_VIEWMODE_WINDOWED);
    ui->actionWindowCascade->setEnabled(true);
    ui->actionWindowTile->setEnabled(true);
}

void MainWindow::on_actionWindowViewModeTabbed_triggered(bool)
{
    ui->actionWindowViewModeTabbed->setChecked(true);
    ui->actionWindowViewModeWindowed->setChecked(false);
    globalConfig->setMainWindowViewMode(QCHDMAN_VIEWMODE_TABBED);
    ui->mdiArea->setViewMode(QCHDMAN_VIEWMODE_TABBED);
    ui->actionWindowCascade->setEnabled(false);
    ui->actionWindowTile->setEnabled(false);
}

void MainWindow::on_actionHelpAbout_triggered(bool)
{
}

void MainWindow::on_actionHelpAboutQt_triggered(bool)
{
}

void MainWindow::updateStatus()
{
    statisticsLabel->setText(tr("Running projects: %1").arg(runningProjects));
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    globalConfig->setMainWindowState(saveState());
    globalConfig->setMainWindowGeometry(saveGeometry());

    if ( preferencesDialog )
        delete preferencesDialog;

    delete globalConfig;

    QMainWindow::closeEvent(e);
}
