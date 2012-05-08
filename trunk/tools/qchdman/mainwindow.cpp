 #include <QtGui>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "projectwindow.h"
#include "projectwidget.h"
#include "macros.h"
#include "settings.h"

extern Settings *globalConfig;
extern quint64 runningProjects;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    closeOk = true;
    forceQuit = false;

#if QT_VERSION >= 0x040800
    ui->mdiArea->setTabsMovable(true);
    ui->mdiArea->setTabsClosable(true);
#endif

    preferencesDialog = new PreferencesDialog(this);

    restoreGeometry(globalConfig->mainWindowGeometry());
    restoreState(globalConfig->mainWindowState());

    setWindowTitle(QCHDMAN_APP_TITLE + " " + QCHDMAN_APP_VERSION);
    nextProjectID = 0;

    if ( globalConfig->mainWindowViewMode() == QCHDMAN_VIEWMODE_WINDOWED )
        on_actionWindowViewModeWindowed_triggered();
    else
        on_actionWindowViewModeTabbed_triggered();

    // check CHDMAN binary setting
    QFileInfo chdmanFileInfo(globalConfig->preferencesChdmanBinary());   
    if ( globalConfig->preferencesChdmanBinary().isEmpty() || !chdmanFileInfo.isExecutable() )
        QTimer::singleShot(100, preferencesDialog, SLOT(initialSetup()));

    statisticsLabel = new QLabel;
    statusBar()->addPermanentWidget(statisticsLabel);
    updateStatus();
    connect(&statusTimer, SIGNAL(timeout()), this, SLOT(updateStatus()));
    statusTimer.start(QCHDMAN_STATUS_INTERVAL);

    QTimer::singleShot(0, preferencesDialog, SLOT(applySettings()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionProjectNew_triggered(bool)
{
    ProjectWindow *projectWindow = new ProjectWindow(QString(), ui->mdiArea);
    projectWindow->show();
    if ( globalConfig->mainWindowViewMode() == QCHDMAN_VIEWMODE_WINDOWED )
        if ( globalConfig->preferencesMaximizeWindows() )
            projectWindow->showMaximized();
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
    applySettings();
}

void MainWindow::on_actionWindowViewModeTabbed_triggered(bool)
{
    ui->actionWindowViewModeTabbed->setChecked(true);
    ui->actionWindowViewModeWindowed->setChecked(false);
    globalConfig->setMainWindowViewMode(QCHDMAN_VIEWMODE_TABBED);
    ui->mdiArea->setViewMode(QCHDMAN_VIEWMODE_TABBED);
    ui->actionWindowCascade->setEnabled(false);
    ui->actionWindowTile->setEnabled(false);
    applySettings();
}

void MainWindow::on_actionHelpAbout_triggered(bool)
{
}

void MainWindow::on_actionHelpAboutQt_triggered(bool)
{
    QApplication::aboutQt();
}

void MainWindow::updateStatus()
{
    statisticsLabel->setText(" " + tr("Running projects: %1").arg(runningProjects) + " ");
}

void MainWindow::applySettings()
{
    qApp->processEvents();
    QFont f;
    f.fromString(globalConfig->preferencesLogFont());
    f.setPointSize(globalConfig->preferencesLogFontSize());
    foreach (QMdiSubWindow *w, ui->mdiArea->subWindowList()) {
        ProjectWidget *pW = (ProjectWidget *)w->widget();
        if ( pW ) {
            pW->setLogFont(f);
            pW->on_comboBoxProjectType_currentIndexChanged(-1);
        }
    }
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    closeOk = true;
    forceQuit = false;

    if ( runningProjects > 0 ) {
        switch ( QMessageBox::question(this, tr("Confirm"),
                                       runningProjects == 1 ?
                                       tr("There is 1 project currently running.\nClosing its window will kill the external process!\n\nProceed?") :
                                       tr("There are %1 projects currently running.\nClosing their windows will kill the external processes!\n\nProceed?").arg(runningProjects),
                                       QMessageBox::Yes | QMessageBox::No, QMessageBox::No) ) {
        case QMessageBox::Yes:
            forceQuit = true;
            break;
        case QMessageBox::No:
        default:
            closeOk = false;
            QTimer::singleShot(100, this, SLOT(resetCloseFlag()));
            e->ignore();
            return;
            break;
        }
    }

    ui->mdiArea->closeAllSubWindows();
    qApp->processEvents();

    QList<QMdiSubWindow *>subWindowList = ui->mdiArea->subWindowList();

    if ( subWindowList.isEmpty() && closeOk ) {
        globalConfig->setMainWindowState(saveState());
        globalConfig->setMainWindowGeometry(saveGeometry());

        if ( preferencesDialog )
            delete preferencesDialog;

        e->accept();
        delete globalConfig;
    } else
        e->ignore();
}
