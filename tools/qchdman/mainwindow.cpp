#include <QtGui>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "projectwindow.h"
#include "projectwidget.h"
#include "macros.h"
#include "qchdmansettings.h"

extern QtChdmanGuiSettings *globalConfig;
extern quint64 runningProjects;
extern quint64 runningScripts;

QStringList MainWindow::projectTypes;
QMap<QString, QList<DiskGeometry> > MainWindow::hardDiskTemplates;

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
    aboutDialog = new AboutDialog(this);

    restoreGeometry(globalConfig->mainWindowGeometry());
    restoreState(globalConfig->mainWindowState());

    setWindowTitle(QCHDMAN_APP_TITLE + " " + QCHDMAN_APP_VERSION + " [Qt " + qVersion() + "]");
    nextProjectID = 0;

    if ( projectTypes.isEmpty() )
        projectTypes << "Info" << "Verify" << "Copy" << "CreateRaw" << "CreateHD" << "CreateCD" << "CreateLD" << "ExtractRaw" << "ExtractHD" << "ExtractCD" << "ExtractLD" << "DumpMeta" << "AddMeta" << "DelMeta";

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

    recentScripts = globalConfig->mainWindowRecentScripts();
    foreach (QString script, recentScripts) {
        QFile f(script);
        if ( f.exists() )
            ui->menuProjectRecentScripts->addAction(script, this, SLOT(loadRecentScript()));
    }

    // sub-window icons
    iconMap[QCHDMAN_PRJ_INFO] = QIcon(":/images/info.png");
    iconMap[QCHDMAN_PRJ_VERIFY] = QIcon(":/images/verify.png");
    iconMap[QCHDMAN_PRJ_COPY] = QIcon(":/images/copy.png");
    iconMap[QCHDMAN_PRJ_CREATE_RAW] = QIcon(":/images/createraw.png");
    iconMap[QCHDMAN_PRJ_CREATE_HD] = QIcon(":/images/createhd.png");
    iconMap[QCHDMAN_PRJ_CREATE_CD] = QIcon(":/images/createcd.png");
    iconMap[QCHDMAN_PRJ_CREATE_LD] = QIcon(":/images/createld.png");
    iconMap[QCHDMAN_PRJ_EXTRACT_RAW] = QIcon(":/images/extractraw.png");
    iconMap[QCHDMAN_PRJ_EXTRACT_HD] = QIcon(":/images/extracthd.png");
    iconMap[QCHDMAN_PRJ_EXTRACT_CD] = QIcon(":/images/extractcd.png");
    iconMap[QCHDMAN_PRJ_EXTRACT_LD] = QIcon(":/images/extractld.png");
    iconMap[QCHDMAN_PRJ_DUMP_META] = QIcon(":/images/dumpmeta.png");
    iconMap[QCHDMAN_PRJ_ADD_META] = QIcon(":/images/addmeta.png");
    iconMap[QCHDMAN_PRJ_DEL_META] = QIcon(":/images/delmeta.png");

    // compression codecs
    compressionTypes["avhu"] = tr("avhu (A/V Huffman)");
    compressionTypes["cdfl"] = tr("cdfl (CD FLAC)");
    compressionTypes["cdlz"] = tr("cdlz (CD LZMA)");
    compressionTypes["cdzl"] = tr("cdzl (CD Deflate)");
    compressionTypes["flac"] = tr("flac (FLAC)");
    compressionTypes["huff"] = tr("huff (Huffman)");
    compressionTypes["lzma"] = tr("lzma (LZMA)");
    compressionTypes["zlib"] = tr("zlib (Deflate)");

    // morph & clone types
    copyTypes[QCHDMAN_PRJ_INFO]
            << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_CHECKBOX;
    copyTypes[QCHDMAN_PRJ_VERIFY]
            << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_LINEEDIT;
    copyTypes[QCHDMAN_PRJ_COPY]
            << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_LINEEDIT
            << QCHDMAN_TYPE_CHECKBOX << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_SPINBOX
            << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_COMBOBOX;
    copyTypes[QCHDMAN_PRJ_CREATE_RAW]
            << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_LINEEDIT
            << QCHDMAN_TYPE_CHECKBOX << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_SPINBOX
            << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_COMBOBOX
            << QCHDMAN_TYPE_SPINBOX;
    copyTypes[QCHDMAN_PRJ_CREATE_HD]
            << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_LINEEDIT
            << QCHDMAN_TYPE_CHECKBOX << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_SPINBOX
            << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_COMBOBOX
            << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_SPINBOX
            << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_SPINBOX;
    copyTypes[QCHDMAN_PRJ_CREATE_CD]
            << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_LINEEDIT
            << QCHDMAN_TYPE_CHECKBOX << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE
            << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_COMBOBOX;
    copyTypes[QCHDMAN_PRJ_CREATE_LD]
            << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_LINEEDIT
            << QCHDMAN_TYPE_CHECKBOX << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE
            << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_COMBOBOX
            << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE
            << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_SPINBOX;
    copyTypes[QCHDMAN_PRJ_EXTRACT_RAW]
            << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_NONE
            << QCHDMAN_TYPE_CHECKBOX << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_SPINBOX
            << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_SPINBOX;
    copyTypes[QCHDMAN_PRJ_EXTRACT_HD]
            << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_NONE
            << QCHDMAN_TYPE_CHECKBOX << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_SPINBOX
            << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_SPINBOX;
    copyTypes[QCHDMAN_PRJ_EXTRACT_CD]
            << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_LINEEDIT
            << QCHDMAN_TYPE_CHECKBOX;
    copyTypes[QCHDMAN_PRJ_EXTRACT_LD]
            << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_NONE
            << QCHDMAN_TYPE_CHECKBOX << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE
            << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE
            << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE
            << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_SPINBOX;
    copyTypes[QCHDMAN_PRJ_DUMP_META]
            << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE
            << QCHDMAN_TYPE_CHECKBOX << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE
            << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE
            << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE
            << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE
            << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_SPINBOX;
    copyTypes[QCHDMAN_PRJ_ADD_META]
            << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE
            << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE
            << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE
            << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE
            << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE
            << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_SPINBOX << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_LINEEDIT
            << QCHDMAN_TYPE_CHECKBOX;
    copyTypes[QCHDMAN_PRJ_DEL_META]
            << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE
            << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE
            << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE
            << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE
            << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE << QCHDMAN_TYPE_NONE
            << QCHDMAN_TYPE_LINEEDIT << QCHDMAN_TYPE_SPINBOX;

    // hard disk templates
    hardDiskTemplates["Alps Electric"]
            << DiskGeometry("DRND-10A", 615, 2, 17, 256)
            << DiskGeometry("DRND-20A", 615, 4, 17, 256)
            << DiskGeometry("RPO-20A", 615, 4, 17, 512);

    hardDiskTemplates["Ampex"]
            << DiskGeometry("Pyxis-7", 320, 2, 17, 512)
            << DiskGeometry("Pyxis-13", 320, 4, 17, 512)
            << DiskGeometry("Pyxis-20", 320, 6, 17, 512)
            << DiskGeometry("Pyxis-27", 320, 8, 17, 512);

    hardDiskTemplates["Amstrad"]
            << DiskGeometry("DRMD2OA12A", 615, 4, 17, 512)
            << DiskGeometry("SRD 30400-50", 822, 2, 51, 512)
            << DiskGeometry("SRD 3O8OC-50", 964, 10, 17, 512);

    hardDiskTemplates["APS Technologies"]
            << DiskGeometry("MS 4.0", 3124, 19, 144, 512)
            << DiskGeometry("Q 10800", 2864, 16, 46, 512)
            << DiskGeometry("Q 2.0", 3850, 10, 109, 512)
            << DiskGeometry("ST 2000(W)", 6311, 4, 175, 512)
            << DiskGeometry("ST 23000(W)", 6880, 28, 237, 512)
            << DiskGeometry("ST 4200", 3992, 19, 110, 512)
            << DiskGeometry("ST 9.0", 2925, 27, 133, 512);

    hardDiskTemplates["Blue Disk"]
            << DiskGeometry("CD 1241-ISA", 976, 8, 31, 512)
            << DiskGeometry("CD 1501-ISA", 989, 8, 37, 512)
            << DiskGeometry("CD 2401-ISA", 977, 8, 59, 512)
            << DiskGeometry("CD 3251-ISA", 1024, 12, 51, 512)
            << DiskGeometry("CD 421-ISA", 976, 4, 21, 512)
            << DiskGeometry("CD 5101", 977, 14, 72, 512);

    hardDiskTemplates["Conner"]
            << DiskGeometry("CP3024", 636, 2, 33, 512)
            << DiskGeometry("CP349", 788, 4, 26, 512)
            << DiskGeometry("CFA 2161A", 1023, 64, 63, 512)
            << DiskGeometry("CFS 1275A", 2479, 16, 63, 512)
            << DiskGeometry("CFS 1621A", 3146, 16, 63, 512)
            << DiskGeometry("CFP 1080S(E)", 3658, 6, 96, 512)
            << DiskGeometry("CFP 4217S", 6028, 10, 147, 512)
            << DiskGeometry("CFP 9117S", 6028, 20, 158, 512)
            << DiskGeometry("CP3184", 832, 6, 33, 512)
            << DiskGeometry("CP3104", 776, 8, 33, 512)
            << DiskGeometry("CFS210A", 685, 16, 38, 512);

    hardDiskTemplates["Data General"]
            << DiskGeometry("6301", 640, 7, 17, 512)
            << DiskGeometry("6338", 1024, 8, 17, 512)
            << DiskGeometry("6339", 965, 15, 17, 512);

    hardDiskTemplates["DMA"]
            << DiskGeometry("306", 612, 2, 17, 256);

    hardDiskTemplates["Hitachi"]
            << DiskGeometry("DK226A-21", 4188, 16, 63, 512)
            << DiskGeometry("DK227A-41", 7944, 16, 63, 512);

    hardDiskTemplates["JVC"]
            << DiskGeometry("JD 3812M0Z0", 436, 2, 48, 512)
            << DiskGeometry("JD E3848HA", 436, 4, 48, 512);

    hardDiskTemplates["JTC"]
            << DiskGeometry("100", 131, 4, 17, 512)
            << DiskGeometry("1006", 436, 2, 17, 512)
            << DiskGeometry("1010", 436, 4, 17, 512);

    hardDiskTemplates["Magtron"]
            << DiskGeometry("MT 5760", 1632, 15, 54, 512);

    hardDiskTemplates["Maxtor"]
            << DiskGeometry("72025AP", 3936, 16, 63, 512)
            << DiskGeometry("72577AV", 4996, 16, 63, 512)
            << DiskGeometry("82625A6", 5100, 16, 63, 512)
            << DiskGeometry("83209A5", 6218, 16, 63, 512)
            << DiskGeometry("83500A4", 7237, 16, 63, 512)
            << DiskGeometry("84000A6", 7763, 16, 63, 512)
            << DiskGeometry("85250A6", 10856, 16, 63, 512)
            << DiskGeometry("86480A8", 13392, 16, 63, 512)
            << DiskGeometry("87000A8", 14475, 16, 63, 512)
            << DiskGeometry("88400D8", 16278, 16, 63, 512)
            << DiskGeometry("MXT 1240S", 2512, 15, 67, 512)
            << DiskGeometry("XT-1065", 918, 7, 17, 512)
            << DiskGeometry("XT-1085", 1024, 8, 17, 512)
            << DiskGeometry("XT-1105", 918, 11, 17, 512)
            << DiskGeometry("XT-2085", 1224, 7, 33, 256)
            << DiskGeometry("XT-2190", 1224, 15, 33, 256)
            << DiskGeometry("XT-4170E/S", 1224, 7, 36, 512)
            << DiskGeometry("XT-4280S", 1224, 11, 36, 512)
            << DiskGeometry("XT-8380S", 1632, 8, 54, 512)
            << DiskGeometry("XT-8760S", 1632, 15, 54, 512);

    hardDiskTemplates["Olivetti"]
            << DiskGeometry("HD 362", 612, 4, 17, 512)
            << DiskGeometry("HD 561-1", 180, 2, 17, 512)
            << DiskGeometry("HD 561-2", 180, 4, 17, 512)
            << DiskGeometry("HD 561-3", 180, 6, 17, 512)
            << DiskGeometry("HD 662/11", 612, 2, 17, 512)
            << DiskGeometry("HD 662/12", 612, 4, 17, 512)
            << DiskGeometry("HD 674", 820, 6, 17, 512);

    hardDiskTemplates["Quantum"]
            << DiskGeometry("ProDrive 40 S", 834, 3, 35, 512) // SCSI
            << DiskGeometry("ProDrive 105 S", 1019, 6, 35, 512) // SCSI
            << DiskGeometry("ProDrive 170 AT", 1123, 7, 43, 512)
            << DiskGeometry("ProDrive 210 AT", 1156, 7, 56, 512)
            << DiskGeometry("ProDrive 425 AT", 1021, 16, 51, 512)
            << DiskGeometry("ProDrive LPS 525 AT", 1017, 16, 63, 512);

    hardDiskTemplates["Samsung"]
            << DiskGeometry("SP 1366D", 26464, 16, 63, 512)
            << DiskGeometry("SV 1296A/D", 25038, 16, 63, 512)
            << DiskGeometry("SV 1533D", 29660, 16, 63, 512)
            << DiskGeometry("SV 1705D", 33187, 16, 63, 512)
            << DiskGeometry("SV 1824D", 35324, 16, 63, 512)
            << DiskGeometry("SV 2044D", 39546, 16, 63, 512);

    hardDiskTemplates["Seagate"]
            << DiskGeometry("ST-506", 153, 4, 32, 256)
            << DiskGeometry("ST-412", 306, 4, 32, 256)
            << DiskGeometry("ST-225", 615, 4, 17, 512) // same specs as Western Digital WD262
            << DiskGeometry("ST-251", 820, 6, 17, 512)
            << DiskGeometry("ST-9100A", 748, 14, 16, 512)
            << DiskGeometry("ST-3243A", 1024, 12, 34, 512)
            << DiskGeometry("ST-9655AG", 1024, 16, 64, 512)
            << DiskGeometry("ST-423451N", 6876, 28, 237, 512);

    hardDiskTemplates["Texas Instruments"]
            << DiskGeometry("TI-5", 153, 4, 17, 512)
            << DiskGeometry("525-122", 306, 4, 17, 512);

    hardDiskTemplates["Western Digital"]
            << DiskGeometry("WD262", 615, 4, 17, 512) // same specs as Seagate ST-225
            << DiskGeometry("WD95038X", 615, 6, 17, 512)
            << DiskGeometry("WDAC140", 980, 5, 17, 512)
            << DiskGeometry("WDAC280", 980, 10, 17, 512)
            << DiskGeometry("WDAC1170", 1010, 6, 55, 512)
            << DiskGeometry("WDAC2340", 1010, 12, 55, 512)
            << DiskGeometry("WDAC2700", 1416, 16, 63, 512)
            << DiskGeometry("WDAC11000", 2046, 16, 63, 512);

    hardDiskTemplates["Applied Engineering"]
            << DiskGeometry("Vulcan 40MB", 960, 5, 17, 512)
            << DiskGeometry("Vulcan 80MB v1", 1067, 5, 29, 512)
            << DiskGeometry("Vulcan 80MB v2", 954, 10, 17, 512)
            << DiskGeometry("Vulcan 100MB", 863, 7, 34, 512)
            << DiskGeometry("Vulcan 140MB", 1068, 9, 29, 512);

    statisticsLabel = new QLabel(this);
    statusBar()->addPermanentWidget(statisticsLabel);
    updateStatus();
    connect(&statusTimer, SIGNAL(timeout()), this, SLOT(updateStatus()));
    statusTimer.start(QCHDMAN_STATUS_INTERVAL);

    QTimer::singleShot(0, preferencesDialog, SLOT(applySettings()));

    // check CHDMAN binary setting
    QFileInfo chdmanFileInfo(globalConfig->preferencesChdmanBinary());
    if ( globalConfig->preferencesChdmanBinary().isEmpty() || !chdmanFileInfo.isExecutable() )
        QTimer::singleShot(100, preferencesDialog, SLOT(initialSetup()));
}

MainWindow::~MainWindow()
{
    delete ui;
    globalConfig->deleteLater();
}

QMdiArea *MainWindow::mdiArea()
{
    return ui->mdiArea;
}

void MainWindow::on_actionProjectNew_triggered(bool)
{
    createProjectWindow(QCHDMAN_MDI_PROJECT);
}

void MainWindow::on_actionProjectNewScript_triggered(bool checked)
{
    createProjectWindow(QCHDMAN_MDI_SCRIPT);
}

void MainWindow::on_actionProjectLoad_triggered(bool)
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choose project file"), QString(), tr("Project files (*.prj)") + ";;" + tr("All files (*)"), 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
    if ( !fileName.isNull() ) {
        ProjectWindow *projectWindow = new ProjectWindow(fileName, QCHDMAN_MDI_PROJECT, ui->mdiArea);
        projectWindow->show();
        if ( globalConfig->mainWindowViewMode() == QCHDMAN_VIEWMODE_WINDOWED )
            if ( globalConfig->preferencesMaximizeWindows() )
                projectWindow->showMaximized();
        projectWindow->projectWidget->load(fileName);
    }
}

void MainWindow::on_actionProjectLoadScript_triggered(bool checked)
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choose script file"), QString(), tr("Script files (*.scr)") + ";;" + tr("All files (*)"), 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
    if ( !fileName.isNull() ) {
        ProjectWindow *projectWindow = new ProjectWindow(fileName, QCHDMAN_MDI_SCRIPT, ui->mdiArea);
        projectWindow->show();
        if ( globalConfig->mainWindowViewMode() == QCHDMAN_VIEWMODE_WINDOWED )
            if ( globalConfig->preferencesMaximizeWindows() )
                projectWindow->showMaximized();
        projectWindow->scriptWidget->load(fileName);
    }
}

void MainWindow::on_actionProjectSave_triggered(bool)
{
    ProjectWindow *projectWindow = (ProjectWindow *)ui->mdiArea->activeSubWindow();
    if ( projectWindow ) {
        if ( projectWindow->subWindowType == QCHDMAN_MDI_PROJECT )
            projectWindow->projectWidget->save();
        else if ( projectWindow->subWindowType == QCHDMAN_MDI_SCRIPT )
            projectWindow->scriptWidget->save();
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
        } else if ( projectWindow->subWindowType == QCHDMAN_MDI_SCRIPT ) {
            projectWindow->scriptWidget->askFileName = true;
            projectWindow->scriptWidget->saveAs();
            projectWindow->scriptWidget->askFileName = false;
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
        } else if ( projectWindow->subWindowType == QCHDMAN_MDI_SCRIPT ) {
            ScriptWidget *scriptWidget = (ScriptWidget *)projectWindow->widget();
            scriptWidget->save();
        }
    }
}

void MainWindow::on_actionProjectPreferences_triggered(bool)
{
    preferencesDialog->show();
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
        if ( projectWindow->subWindowType == QCHDMAN_MDI_PROJECT )
            projectWindow->projectWidget->on_comboBoxProjectType_currentIndexChanged(-1);
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
        if ( projectWindow->subWindowType == QCHDMAN_MDI_PROJECT )
            projectWindow->projectWidget->on_comboBoxProjectType_currentIndexChanged(-1);
    }
}

void MainWindow::on_actionHelpAbout_triggered(bool)
{
    aboutDialog->show();
}

void MainWindow::on_actionHelpAboutQt_triggered(bool)
{
    QApplication::aboutQt();
}

void MainWindow::on_actionHelpWiki_triggered(bool)
{
    QDesktopServices::openUrl(QUrl::fromUserInput("http://wiki.batcom-it.net/index.php?title=Qt_CHDMAN_GUI"));
}

void MainWindow::on_actionHelpForum_triggered(bool)
{
    QDesktopServices::openUrl(QUrl::fromUserInput("http://forums.bannister.org/ubbthreads.php?ubb=postlist&Board=12"));
}

void MainWindow::on_actionHelpBugTracker_triggered(bool checked)
{
    QDesktopServices::openUrl(QUrl::fromUserInput("http://tracker.batcom-it.net/view_all_bug_page.php?project_id=1"));
}

ProjectWindow *MainWindow::createProjectWindow(int type)
{
    ProjectWindow *projectWindow = new ProjectWindow(QString(), type, ui->mdiArea);

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
    statisticsLabel->setText(" " + tr("Running scripts / projects: %1 / %2").arg(runningScripts).arg(runningProjects) + " ");
}

void MainWindow::applySettings()
{
    qApp->processEvents();
    QFont logFont;
    logFont.fromString(globalConfig->preferencesLogFont());
    logFont.setPointSize(globalConfig->preferencesLogFontSize());
    foreach (QMdiSubWindow *w, ui->mdiArea->subWindowList()) {
        ProjectWindow *projectWindow = (ProjectWindow *)w;
        if ( projectWindow->subWindowType == QCHDMAN_MDI_PROJECT ) {
            ProjectWidget *projectWidget = (ProjectWidget *)projectWindow->widget();
            if ( projectWidget ) {
                projectWidget->setLogFont(logFont);
                projectWidget->on_comboBoxProjectType_currentIndexChanged(-1);
                projectWidget->needsTabbedUiAdjustment = true;
                projectWidget->needsWindowedUiAdjustment = true;
            }
        } else if ( projectWindow->subWindowType == QCHDMAN_MDI_SCRIPT ) {
            ScriptWidget *scriptWidget = (ScriptWidget *)projectWindow->widget();
            if ( scriptWidget )
                scriptWidget->adjustFonts();
        }
    }
    preferredCHDInputFolder = globalConfig->preferencesPreferredCHDInputPath();
    preferredInputFolder = globalConfig->preferencesPreferredInputPath();
    preferredCHDOutputFolder = globalConfig->preferencesPreferredCHDOutputPath();
    preferredOutputFolder = globalConfig->preferencesPreferredOutputPath();
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

void MainWindow::addRecentScript(const QString &fileName)
{
    if ( !fileName.isEmpty() ) {
        recentScripts.removeAll(fileName);
        recentScripts.insert(0, fileName);
        if ( recentScripts.count() > QCHDMAN_MAX_RECENT_FILES )
            recentScripts.removeAt(recentScripts.count() - 1);
        ui->menuProjectRecentScripts->clear();
        foreach (QString file, recentScripts) {
            QFile f(file);
            if ( f.exists() )
                ui->menuProjectRecentScripts->addAction(file, this, SLOT(loadRecentScript()));
        }
        globalConfig->setMainWindowRecentScripts(recentScripts);
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

void MainWindow::loadRecentScript()
{
    QAction *action = (QAction *)sender();
    QFile f(action->text());
    if ( !f.exists() ) {
        statusBar()->showMessage(tr("Script '%1' doesn't exist"), QCHDMAN_STATUS_MSGTIME);
        return;
    }
    ProjectWindow *projectWindow = new ProjectWindow(action->text(), QCHDMAN_MDI_SCRIPT, ui->mdiArea);
    projectWindow->show();
    if ( globalConfig->mainWindowViewMode() == QCHDMAN_VIEWMODE_WINDOWED )
        if ( globalConfig->preferencesMaximizeWindows() )
            projectWindow->showMaximized();
    projectWindow->scriptWidget->load(action->text());
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
        }
    } else {
        if ( projectWindow->subWindowType == QCHDMAN_MDI_PROJECT ) {
            if ( projectWindow->projectWidget->needsWindowedUiAdjustment ) {
                projectWindow->projectWidget->on_comboBoxProjectType_currentIndexChanged(-1);
                projectWindow->projectWidget->needsWindowedUiAdjustment = false;
            }
            projectWindow->projectWidget->needsTabbedUiAdjustment = true;
        } else if ( projectWindow->subWindowType == QCHDMAN_MDI_SCRIPT ) {
            // NOP
        }
    }
}

void MainWindow::resizeEvent(QResizeEvent *)
{
    foreach (QMdiSubWindow *w, ui->mdiArea->subWindowList()) {
        ProjectWindow *projectWindow = (ProjectWindow *)w;
        if ( projectWindow->subWindowType == QCHDMAN_MDI_PROJECT ) {
            QTimer::singleShot(0, (ProjectWidget *)w->widget(), SLOT(triggerUpdate()));
        } else if ( projectWindow->subWindowType == QCHDMAN_MDI_SCRIPT ) {
            // NOP
        }
    }
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    closeOk = true;
    forceQuit = false;

    if ( runningScripts > 0 ) {
        switch ( QMessageBox::question(this, tr("Confirm"),
                                       tr("There are %n script(s) currently running.\n\nProceed?", "", runningScripts),
                                       QMessageBox::Yes | QMessageBox::No, QMessageBox::No) ) {
        case QMessageBox::Yes:
            forceQuit = true;
            break;
        case QMessageBox::No:
        default:
            closeOk = false;
            forceQuit = false;
            QTimer::singleShot(100, this, SLOT(resetCloseFlag()));
            e->ignore();
            return;
            break;
        }
    }

    int runningIndependentProjects = 0;
    foreach (QMdiSubWindow *w, ui->mdiArea->subWindowList()) {
        ProjectWindow *pw = (ProjectWindow *)w;
        if ( pw->subWindowType == QCHDMAN_MDI_PROJECT )
            if ( !pw->projectWidget->isScriptElement && pw->projectWidget->status == QCHDMAN_PRJSTAT_RUNNING )
                runningIndependentProjects++;
    }

    if ( runningIndependentProjects > 0 ) {
        switch ( QMessageBox::question(this, tr("Confirm"),
                                       runningIndependentProjects == 1 ?
                                       tr("There is 1 project currently running.\n\nClosing its window will kill the external process!\n\nProceed?") :
                                       tr("There are %1 projects currently running.\n\nClosing their windows will kill the external processes!\n\nProceed?").arg(runningIndependentProjects),
                                       QMessageBox::Yes | QMessageBox::No, QMessageBox::No) ) {
        case QMessageBox::Yes:
            forceQuit = true;
            break;
        case QMessageBox::No:
        default:
            closeOk = false;
            forceQuit = false;
            QTimer::singleShot(100, this, SLOT(resetCloseFlag()));
            e->ignore();
            return;
            break;
        }
    }

    if ( closeOk ) {
        foreach (QMdiSubWindow *w, ui->mdiArea->subWindowList()) {
            if ( ((ProjectWindow *)w)->subWindowType == QCHDMAN_MDI_PROJECT ) {
                ProjectWidget *projectWidget = (ProjectWidget *)w->widget();
                if ( projectWidget ) {
                    if ( projectWidget->chdmanProc && projectWidget->chdmanProc->state() == QProcess::Running )
                        projectWidget->chdmanProc->terminate();
                }
            }
            QTimer::singleShot(0, w, SLOT(close()));
        }

        globalConfig->setMainWindowState(saveState());
        globalConfig->setMainWindowGeometry(saveGeometry());

        if ( preferencesDialog )
            delete preferencesDialog;

        e->accept();
    } else
        e->ignore();

    qApp->processEvents();
}
