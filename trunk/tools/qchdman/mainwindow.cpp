#include <QtGui>
#include <QFileDialog>
#include <QMessageBox>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "projectwindow.h"
#include "projectwidget.h"
#include "aboutdialog.h"
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

    projectTypes << "Info" << "Verify" << "Copy" << "CreateRaw" << "CreateHD" << "CreateCD" << "CreateLD" << "ExtractRaw" << "ExtractHD" << "ExtractCD" << "ExtractLD";

    if ( globalConfig->mainWindowViewMode() == QCHDMAN_VIEWMODE_WINDOWED )
        on_actionWindowViewModeWindowed_triggered();
    else
        on_actionWindowViewModeTabbed_triggered();

    recentFiles = globalConfig->mainWindowRecentFiles();
    foreach (QString file, recentFiles) {
        QFile f(file);
        if ( f.exists() )
            ui->menuProjectRecent->addAction(file, this, SLOT(loadRecentFile()));
    }

    // check CHDMAN binary setting
    QFileInfo chdmanFileInfo(globalConfig->preferencesChdmanBinary());   
    if ( globalConfig->preferencesChdmanBinary().isEmpty() || !chdmanFileInfo.isExecutable() )
        QTimer::singleShot(100, preferencesDialog, SLOT(initialSetup()));

    statisticsLabel = new QLabel;
    statusBar()->addPermanentWidget(statisticsLabel);
    updateStatus();
    connect(&statusTimer, SIGNAL(timeout()), this, SLOT(updateStatus()));
    statusTimer.start(QCHDMAN_STATUS_INTERVAL);

    // hard disk templates
    hardDiskTemplates["Conner"]
            << DiskGeometry("CP3024 (20 MB)", 636, 2, 33, 512)
            << DiskGeometry("CP349 (40 MB)", 788, 4, 26, 512)
            << DiskGeometry("CP3184 (80 MB)", 832, 6, 33, 512)
            << DiskGeometry("CP3104 (100 MB)", 776, 8, 33, 512)
            << DiskGeometry("CFS210A (203 MB)", 685, 16, 38, 512);

    hardDiskTemplates["Hitachi"]
            << DiskGeometry("DK226A-21 (2.0 GB)", 4188, 16, 63, 512)
            << DiskGeometry("DK227A-41 (3.8 GB)", 7944, 16, 63, 512);

    hardDiskTemplates["Western Digital"]
            << DiskGeometry("WD95038X (32 MB)", 615, 6, 17, 512)
            << DiskGeometry("WDAC140 (41 MB)", 980, 5, 17, 512)
            << DiskGeometry("WDAC280 (81 MB)", 980, 10, 17, 512)
            << DiskGeometry("WDAC1170 (163 MB)", 1010, 6, 55, 512)
            << DiskGeometry("WDAC2340 (325 MB)", 1010, 12, 55, 512)
            << DiskGeometry("WDAC2700 (697 MB)", 1416, 16, 63, 512)
            << DiskGeometry("WDAC11000 (1 GB)", 2046, 16, 63, 512);

    hardDiskTemplates["Seagate"]
            << DiskGeometry("ST-506 (5 MB)", 153, 4, 32, 256)
            << DiskGeometry("ST-412 (10 MB)", 306, 4, 32, 256)
            << DiskGeometry("ST-225 (20 MB)", 615, 4, 17, 512)
            //<< DiskGeometry("ST-262 (21 MB)", 615, 4, 17, 512)
            << DiskGeometry("ST-251 (41 MB)", 820, 6, 17, 512)
            << DiskGeometry("ST-9100A (82 MB)", 748, 14, 16, 512)
            << DiskGeometry("ST-3243A (204 MB)", 1024, 12, 34, 512)
            << DiskGeometry("ST-9655AG (512 MB)", 1024, 16, 64, 512);

    hardDiskTemplates["Maxtor"]
            << DiskGeometry("XT-2085 (69 MB)", 1224, 7, 33, 256)
            << DiskGeometry("XT-2190 (148 MB)", 1224, 15, 33, 256);

    hardDiskTemplates["JTC"]
            << DiskGeometry("100 (5 MB)", 131, 4, 17, 512)
            << DiskGeometry("1006 (7 MB)", 436, 2, 17, 512)
            << DiskGeometry("1010 (14 MB)", 436, 4, 17, 512);

    hardDiskTemplates["JVC"]
            << DiskGeometry("JD 3812M0Z0 (21 MB)", 436, 2, 48, 512)
            << DiskGeometry("JD E3848HA (43 MB)", 436, 4, 48, 512);

    hardDiskTemplates["Magtron"]
            << DiskGeometry("MT 5760 (677 MB)", 1632, 15, 54, 512);

    hardDiskTemplates["Texas Instruments"]
            << DiskGeometry("TI-5 (5 MB)", 153, 4, 17, 512)
            << DiskGeometry("525-122 (10 MB)", 306, 4, 17, 512);

    QTimer::singleShot(0, preferencesDialog, SLOT(applySettings()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

QMdiArea *MainWindow::mdiArea()
{
    return ui->mdiArea;
}

void MainWindow::on_actionProjectNew_triggered(bool)
{
    createProjectWindow();
}

void MainWindow::on_actionProjectLoad_triggered(bool)
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choose file"), QString(), tr("All files (*)"), 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
    if ( !fileName.isNull() ) {
        ProjectWindow *projectWindow = new ProjectWindow(fileName, QCHDMAN_MDI_PROJECT, ui->mdiArea);
        projectWindow->show();
        if ( globalConfig->mainWindowViewMode() == QCHDMAN_VIEWMODE_WINDOWED )
            if ( globalConfig->preferencesMaximizeWindows() )
                projectWindow->showMaximized();
        projectWindow->projectWidget->load(fileName);
    }
}

void MainWindow::on_actionProjectSave_triggered(bool)
{
    ProjectWindow *projectWindow = (ProjectWindow *)ui->mdiArea->activeSubWindow();
    if ( projectWindow ) {
        if ( projectWindow->subWindowType == QCHDMAN_MDI_PROJECT ) {
            projectWindow->projectWidget->save();
        } else if ( projectWindow->subWindowType == QCHDMAN_MDI_JOB ) {
            // FIXME
        }
    }
}

void MainWindow::on_actionProjectSaveAs_triggered(bool)
{
    ProjectWindow *projectWindow = (ProjectWindow *)ui->mdiArea->activeSubWindow();
    if ( projectWindow ) {
        if ( projectWindow->subWindowType == QCHDMAN_MDI_PROJECT ) {
            projectWindow->projectWidget->askFileName = true;
            projectWindow->projectWidget->saveAs();
            projectWindow->projectWidget->askFileName = false;
        } else if ( projectWindow->subWindowType == QCHDMAN_MDI_JOB ) {
            // FIXME
        }
    }
}

void MainWindow::on_actionProjectSaveAll_triggered(bool)
{
    foreach (QMdiSubWindow *w, ui->mdiArea->subWindowList()) {
        ProjectWindow *projectWindow = (ProjectWindow *)w;
        if ( projectWindow->subWindowType == QCHDMAN_MDI_PROJECT ) {
            ProjectWidget *projectWidget = (ProjectWidget *)projectWindow->widget();
            projectWidget->save();
        } else if ( projectWindow->subWindowType == QCHDMAN_MDI_JOB ) {
            // FIXME
        }
    }
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
    QTimer::singleShot(0, this, SLOT(updateSubWindows()));
}

void MainWindow::on_actionWindowCascade_triggered(bool)
{
    ui->mdiArea->cascadeSubWindows();
    QTimer::singleShot(0, this, SLOT(updateSubWindows()));
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
    if ( !ui->mdiArea->subWindowList().isEmpty() ) {
        ui->actionWindowCascade->setEnabled(true);
        ui->actionWindowTile->setEnabled(true);
    }
    applySettings();
    foreach (QMdiSubWindow *w, ui->mdiArea->subWindowList()) {
        ProjectWindow *projectWindow = (ProjectWindow *)w;
        if ( globalConfig->preferencesMaximizeWindows() ) {
            projectWindow->showMaximized();
            qApp->processEvents();
        }
        if ( projectWindow->subWindowType == QCHDMAN_MDI_PROJECT ) {
            projectWindow->projectWidget->on_comboBoxProjectType_currentIndexChanged(-1);
        } else if ( projectWindow->subWindowType == QCHDMAN_MDI_JOB ) {
            // FIXME
        }
    }
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
    foreach (QMdiSubWindow *w, ui->mdiArea->subWindowList()) {
        ProjectWindow *projectWindow = (ProjectWindow *)w;
        if ( projectWindow->subWindowType == QCHDMAN_MDI_PROJECT ) {
            projectWindow->projectWidget->on_comboBoxProjectType_currentIndexChanged(-1);
        } else if ( projectWindow->subWindowType == QCHDMAN_MDI_JOB ) {
            // FIXME
        }
    }
}

void MainWindow::on_actionHelpAbout_triggered(bool)
{
    AboutDialog aboutDialog(this);
    aboutDialog.exec();
}

void MainWindow::on_actionHelpAboutQt_triggered(bool)
{
    QApplication::aboutQt();
}

ProjectWindow *MainWindow::createProjectWindow()
{
    ProjectWindow *projectWindow = new ProjectWindow(QString(), QCHDMAN_MDI_PROJECT, ui->mdiArea);

    projectWindow->show();
    if ( globalConfig->mainWindowViewMode() == QCHDMAN_VIEWMODE_WINDOWED )
        if ( globalConfig->preferencesMaximizeWindows() )
            projectWindow->showMaximized();

    return projectWindow;
}

QString MainWindow::humanReadable(qreal value)
{
    qreal humanReadableValue;
    QLocale locale;

#if __WORDSIZE == 64
    if ( (qreal)value / (qreal)QCHDMAN_ONE_KILOBYTE < (qreal)QCHDMAN_ONE_KILOBYTE ) {
        humanReadableValue = (qreal)value / (qreal)QCHDMAN_ONE_KILOBYTE;
        humanReadableString = locale.toString(humanReadableValue, 'f', 2) + QString(tr(" KB"));
    } else if ( (qreal)value / (qreal)QCHDMAN_ONE_MEGABYTE < (qreal)QCHDMAN_ONE_KILOBYTE ) {
        humanReadableValue = (qreal)value / (qreal)QCHDMAN_ONE_MEGABYTE;
        humanReadableString = locale.toString(humanReadableValue, 'f', 2) + QString(tr(" MB"));
    } else if ( (qreal)value / (qreal)QCHDMAN_ONE_GIGABYTE < (qreal)QCHDMAN_ONE_KILOBYTE ) {
        humanReadableValue = (qreal)value / (qreal)QCHDMAN_ONE_GIGABYTE;
        humanReadableString = locale.toString(humanReadableValue, 'f', 2) + QString(tr(" GB"));
    } else {
        humanReadableValue = (qreal)value / (qreal)QCHDMAN_ONE_TERABYTE;
        humanReadableString = locale.toString(humanReadableValue, 'f', 2) + QString(tr(" TB"));
    }
#else
    if ( (qreal)value / (qreal)QCHDMAN_ONE_KILOBYTE < (qreal)QCHDMAN_ONE_KILOBYTE ) {
        humanReadableValue = (qreal)value / (qreal)QCHDMAN_ONE_KILOBYTE;
        humanReadableString = locale.toString(humanReadableValue, 'f', 2) + QString(tr(" KB"));
    } else if ( (qreal)value / (qreal)QCHDMAN_ONE_MEGABYTE < (qreal)QCHDMAN_ONE_KILOBYTE ) {
        humanReadableValue = (qreal)value / (qreal)QCHDMAN_ONE_MEGABYTE;
        humanReadableString = locale.toString(humanReadableValue, 'f', 2) + QString(tr(" MB"));
    } else {
        humanReadableValue = (qreal)value / (qreal)QCHDMAN_ONE_GIGABYTE;
        humanReadableString = locale.toString(humanReadableValue, 'f', 2) + QString(tr(" GB"));
    }
#endif

    return humanReadableString;
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
        ProjectWindow *projectWindow = (ProjectWindow *)w;
        if ( projectWindow->subWindowType == QCHDMAN_MDI_PROJECT ) {
            ProjectWidget *projectWidget = (ProjectWidget *)projectWindow->widget();
            if ( projectWidget ) {
                projectWidget->setLogFont(f);
                projectWidget->on_comboBoxProjectType_currentIndexChanged(-1);
                projectWidget->needsTabbedUiAdjustment = true;
                projectWidget->needsWindowedUiAdjustment = true;
            }
        } else if ( projectWindow->subWindowType == QCHDMAN_MDI_JOB ) {
            // FIXME
        }
    }
}

void MainWindow::updateSubWindows()
{
    foreach (QMdiSubWindow *w, ui->mdiArea->subWindowList()) {
        ProjectWindow *projectWindow = (ProjectWindow *)w;
        if ( projectWindow->subWindowType == QCHDMAN_MDI_PROJECT ) {
            ProjectWidget *projectWidget = (ProjectWidget *)projectWindow->widget();
            projectWidget->on_comboBoxProjectType_currentIndexChanged(-1);
            projectWidget->needsTabbedUiAdjustment = true;
            projectWidget->needsWindowedUiAdjustment = true;
        } else if ( projectWindow->subWindowType == QCHDMAN_MDI_JOB ) {
            // FIXME
        }
    }
}

void MainWindow::addRecentFile(const QString &fileName)
{
    if ( !fileName.isEmpty() ) {
        recentFiles.removeAll(fileName);
        recentFiles.insert(0, fileName);
        if ( recentFiles.count() > QCHDMAN_MAX_RECENT_FILES )
            recentFiles.removeAt(recentFiles.count() - 1);
        ui->menuProjectRecent->clear();
        foreach (QString file, recentFiles) {
            QFile f(file);
            if ( f.exists() )
                ui->menuProjectRecent->addAction(file, this, SLOT(loadRecentFile()));
        }
        globalConfig->setMainWindowRecentFiles(recentFiles);
    }
}

void MainWindow::loadRecentFile()
{
    QAction *action = (QAction *)sender();
    QFile f(action->text());
    if ( !f.exists() ) {
        statusBar()->showMessage(tr("Project '%1' doesn't exist"), QCHDMAN_STATUS_MSGTIME);
        return;
    }
    ProjectWindow *projectWindow = new ProjectWindow(action->text(), QCHDMAN_MDI_PROJECT, ui->mdiArea);
    projectWindow->show();
    if ( globalConfig->mainWindowViewMode() == QCHDMAN_VIEWMODE_WINDOWED )
        if ( globalConfig->preferencesMaximizeWindows() )
            projectWindow->showMaximized();
    projectWindow->projectWidget->load(action->text());
}

void MainWindow::enableActions(bool enable)
{
    ui->actionProjectSave->setEnabled(enable);
    ui->actionProjectSaveAs->setEnabled(enable);
    ui->actionProjectSaveAll->setEnabled(enable);
    if ( ui->mdiArea->subWindowList().count() > 0 || !enable ) {
        ui->actionWindowNext->setEnabled(enable);
        ui->actionWindowPrevious->setEnabled(enable);
    }
    if ( globalConfig->mainWindowViewMode() == QCHDMAN_VIEWMODE_WINDOWED ) {
        ui->actionWindowTile->setEnabled(enable);
        ui->actionWindowCascade->setEnabled(enable);
    }
    ui->actionWindowClose->setEnabled(enable);
    ui->actionWindowCloseAll->setEnabled(enable);
}

void MainWindow::disableActionsRequiringTwo()
{
    ui->actionWindowNext->setEnabled(false);
    ui->actionWindowPrevious->setEnabled(false);
}

void MainWindow::on_mdiArea_subWindowActivated(QMdiSubWindow *w)
{
    if ( !w )
        return;

    ProjectWindow *projectWindow = (ProjectWindow *)w;
    if ( globalConfig->mainWindowViewMode() == QCHDMAN_VIEWMODE_TABBED ) {
        if ( projectWindow->subWindowType == QCHDMAN_MDI_PROJECT ) {
            if ( projectWindow->projectWidget->needsTabbedUiAdjustment ) {
                projectWindow->projectWidget->on_comboBoxProjectType_currentIndexChanged(-1);
                projectWindow->projectWidget->needsTabbedUiAdjustment = false;
            }
            projectWindow->projectWidget->needsWindowedUiAdjustment = true;
        } else if ( projectWindow->subWindowType == QCHDMAN_MDI_JOB ) {
            // FIXME
        }
    } else {
        if ( projectWindow->subWindowType == QCHDMAN_MDI_PROJECT ) {
            if ( projectWindow->projectWidget->needsWindowedUiAdjustment ) {
                projectWindow->projectWidget->on_comboBoxProjectType_currentIndexChanged(-1);
                projectWindow->projectWidget->needsWindowedUiAdjustment = false;
            }
            projectWindow->projectWidget->needsTabbedUiAdjustment = true;
        } else if ( projectWindow->subWindowType == QCHDMAN_MDI_JOB ) {
            // FIXME
        }
    }
}

void MainWindow::resizeEvent(QResizeEvent *)
{
    foreach (QMdiSubWindow *w, ui->mdiArea->subWindowList()) {
        ProjectWindow *projectWindow = (ProjectWindow *)w;
        if ( projectWindow->subWindowType == QCHDMAN_MDI_PROJECT ) {
            QTimer::singleShot(0, (ProjectWidget *)w->widget(), SLOT(triggerUpdate()));
        } else if ( projectWindow->subWindowType == QCHDMAN_MDI_JOB ) {
            // FIXME
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
                                       tr("There is 1 project currently running.\n\nClosing its window will kill the external process!\n\nProceed?") :
                                       tr("There are %1 projects currently running.\n\nClosing their windows will kill the external processes!\n\nProceed?").arg(runningProjects),
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
