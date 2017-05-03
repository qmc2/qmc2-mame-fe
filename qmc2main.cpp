#include <qglobal.h>
#include <QMessageBox>
#include <QCache>
#include <QHeaderView>
#include <QTextStream>
#include <QTextDocument>
#include <QScrollBar>
#include <QProcess>
#include <QTimer>
#include <QTime>
#include <QFile>
#include <QMap>
#include <QMultiMap>
#include <QHash>
#include <QCursor>
#include <QHashIterator>
#include <QStyleFactory>
#include <QBitArray>
#include <QFileInfo>
#include <QFileDialog>
#include <QAction>
#include <QPair>
#include <QClipboard>
#include <QDateTime>
#include <QMutex>
#include <QtWebKit>
#include <QNetworkAccessManager>
#include <QSplashScreen>
#include <QDir>
#include <QTest>
#include <QColorDialog>
#include <QChar>
#if QT_VERSION >= 0x050000
#include <QInputDialog>
#include <QDesktopWidget>
#include <QtWebKitWidgets/QWebFrame>
#endif

#include <algorithm> // std::sort()

#include "macros.h"
#include "qmc2main.h"
#include "options.h"
#include "emuopt.h"
#include "processmanager.h"
#include "machinelist.h"
#include "preview.h"
#include "flyer.h"
#include "cabinet.h"
#include "controller.h"
#include "marquee.h"
#include "title.h"
#include "pcb.h"
#include "docbrowser.h"
#include "about.h"
#include "welcome.h"
#include "imagechecker.h"
#include "samplechecker.h"
#include "romalyzer.h"
#include "romstatusexport.h"
#include "componentsetup.h"
#include "miniwebbrowser.h"
#include "unzip.h"
#include "sevenzipfile.h"
#include "downloaditem.h"
#include "embedder.h"
#include "demomode.h"
#include "softwarelist.h"
#include "softwaresnapshot.h"
#include "toolbarcustomizer.h"
#include "paletteeditor.h"
#include "brusheditor.h"
#include "iconlineedit.h"
#include "cookiejar.h"
#include "networkaccessmanager.h"
#if QMC2_JOYSTICK == 1
#include "joystick.h"
#endif
#include "deviceconfigurator.h"
#if QMC2_USE_PHONON_API
#include "audioeffects.h"
#endif
#if QMC2_MULTIMEDIA_ENABLED
#include <QMediaMetaData>
#endif
#include "toolexec.h"
#if defined(QMC2_OS_UNIX)
#include "keyseqscan.h"
#include "x11_tools.h"
#endif
#if defined(QMC2_YOUTUBE_ENABLED)
#include "youtubevideoplayer.h"
#endif
#if defined(QMC2_OS_WIN)
#include "windows_tools.h"
#endif
#include "htmleditor/htmleditor.h"
#include "arcademodesetup.h"
#include "fileiconprovider.h"
#include "aspectratiolabel.h"
#if defined(QMC2_LIBARCHIVE_ENABLED)
#include "archivefile.h"
#endif
#include "itemselect.h"

#if defined(QMC2_OS_MAC)
#include <ApplicationServices/ApplicationServices.h>
#if defined(verify) // verify() is #defined in AssertMacros.h on OS X
#undef verify
#endif
#include <mach-o/dyld.h>
#endif

// global variables
MainWindow *qmc2MainWindow = 0;
MachineList *qmc2MachineList = 0;
Options *qmc2Options = 0;
Settings *qmc2Config = 0;
EmulatorOptions *qmc2GlobalEmulatorOptions = 0;
EmulatorOptions *qmc2EmulatorOptions = 0;
ProcessManager *qmc2ProcessManager = 0;
Preview *qmc2Preview = 0;
Flyer *qmc2Flyer = 0;
Cabinet *qmc2Cabinet = 0;
Controller *qmc2Controller = 0;
Marquee *qmc2Marquee = 0;
Title *qmc2Title = 0;
PCB *qmc2PCB = 0;
About *qmc2About = 0;
DocBrowser *qmc2DocBrowser = 0;
Welcome *qmc2Welcome = 0;
ImageChecker *qmc2ImageChecker = 0;
SampleChecker *qmc2SampleChecker = 0;
ROMAlyzer *qmc2SystemROMAlyzer = 0;
ROMAlyzer *qmc2SoftwareROMAlyzer = 0;
ROMStatusExporter *qmc2ROMStatusExporter = 0;
ComponentSetup *qmc2ComponentSetup = 0;
ToolBarCustomizer *qmc2ToolBarCustomizer = 0;
PaletteEditor *qmc2PaletteEditor = 0;
SoftwareList *qmc2SoftwareList = 0;
SoftwareSnap *qmc2SoftwareSnap = 0;
SoftwareSnapshot *qmc2SoftwareSnapshot = 0;
QString qmc2DriverName;
DeviceConfigurator *qmc2DeviceConfigurator = 0;
QTreeWidgetItem *qmc2LastDeviceConfigItem = 0;
QTreeWidgetItem *qmc2LastSoftwareListItem = 0;
#if QMC2_USE_PHONON_API
AudioEffectDialog *qmc2AudioEffectDialog = 0;
#endif
#if QMC2_USE_PHONON_API || QMC2_MULTIMEDIA_ENABLED
QString qmc2AudioLastIndividualTrack;
#endif
DemoModeDialog *qmc2DemoModeDialog = 0;
bool qmc2ReloadActive = false;
bool qmc2ImageCheckActive = false;
bool qmc2SampleCheckActive = false;
bool qmc2EarlyReloadActive = false;
bool qmc2VerifyActive = false;
bool qmc2FilterActive = false;
bool qmc2GuiReady = false;
bool qmc2CleaningUp = false;
bool qmc2StartingUp = true;
bool qmc2EarlyStartup = true;
bool qmc2ScaledPreview = true;
bool qmc2ScaledFlyer = true;
bool qmc2ScaledCabinet = true;
bool qmc2ScaledController = true;
bool qmc2ScaledMarquee = true;
bool qmc2ScaledTitle = true;
bool qmc2ScaledPCB = true;
bool qmc2ScaledSoftwareSnapshot = true;
bool qmc2SmoothScaling = false;
bool qmc2RetryLoadingImages = true;
bool qmc2ParentImageFallback = false;
bool qmc2UsePreviewFile = false;
bool qmc2UseFlyerFile = false;
bool qmc2UseCabinetFile = false;
bool qmc2UseControllerFile = false;
bool qmc2UseIconFile = false;
bool qmc2UseMarqueeFile = false;
bool qmc2UseTitleFile = false;
bool qmc2UsePCBFile = false;
bool qmc2UseSoftwareSnapFile = false;
bool qmc2IconsPreloaded = false;
bool qmc2CheckItemVisibility = true;
bool qmc2AutomaticReload = false;
bool qmc2LoadingMachineInfoDB = false;
bool qmc2WidgetsEnabled = true;
bool qmc2ExportingROMStatus = false;
bool qmc2StatesTogglesEnabled = true;
bool qmc2DestroyingArcadeView = false;
bool qmc2IgnoreItemActivation = false;
bool qmc2StartEmbedded = false;
bool qmc2SuppressQtMessages = false;
bool qmc2CriticalSection = false;
bool qmc2UseDefaultEmulator = true;
bool qmc2ForceCacheRefresh = false;
bool qmc2ReconfigureRestartRequested = false;
int qmc2MachineListResponsiveness = 100;
int qmc2UpdateDelay = 10;
QFile *qmc2FrontendLogFile = 0;
QFile *qmc2EmulatorLogFile = 0;
QTextStream qmc2FrontendLogStream;
QTextStream qmc2EmulatorLogStream;
QTranslator *qmc2Translator = 0;
QTranslator *qmc2QtTranslator = 0;
QString qmc2LastFrontendLogMessage;
quint64 qmc2FrontendLogMessageRepeatCount = 0;
QString qmc2LastEmulatorLogMessage;
quint64 qmc2EmulatorLogMessageRepeatCount = 0;
bool qmc2LoadingInterrupted = false;
QTreeWidgetItem *qmc2CurrentItem = 0;
QTreeWidgetItem *qmc2LastConfigItem = 0;
QTreeWidgetItem *qmc2LastMachineInfoItem = 0;
bool qmc2LoadingEmuInfoDB = false;
QTreeWidgetItem *qmc2LastEmuInfoItem = 0;
bool qmc2LoadingSoftwareInfoDB = false;
QTreeWidgetItem *qmc2LastSoftwareInfoItem = 0;
MiniWebBrowser *qmc2ProjectMESSLookup = 0;
QTreeWidgetItem *qmc2LastProjectMESSItem = 0;
QCache<QString, QByteArray> qmc2ProjectMESSCache;
QHash<QString, QTreeWidgetItem *> qmc2CategoryItemHash;
QTreeWidgetItem *qmc2CategoryViewSelectedItem = 0;
QHash<QString, QTreeWidgetItem *> qmc2VersionItemHash;
QTreeWidgetItem *qmc2VersionViewSelectedItem = 0;
MiniWebBrowser *qmc2ProjectMESS = 0;
QTreeWidgetItem *qmc2LastSoftwareNotesItem = 0;
QTreeWidgetItem *qmc2LastSystemNotesItem = 0;
HtmlEditor *qmc2SystemNotesEditor = 0;
HtmlEditor *qmc2SoftwareNotesEditor = 0;
#if defined(QMC2_YOUTUBE_ENABLED)
YouTubeVideoPlayer *qmc2YouTubeWidget = 0;
QTreeWidgetItem *qmc2LastYouTubeItem = 0;
QHash<QString, YouTubeVideoInfo> qmc2YouTubeVideoInfoHash;
bool qmc2YouTubeVideoInfoHashChanged = false;
#endif
QTreeWidgetItem *qmc2HierarchySelectedItem = 0;
QMenu *qmc2EmulatorMenu = 0,
      *qmc2MachineMenu = 0,
      *qmc2FavoritesMenu = 0,
      *qmc2PlayedMenu = 0,
      *qmc2SearchMenu = 0,
      *qmc2ForeignIDsMenu = 0;
QHash<QString, QTreeWidgetItem *> qmc2MachineListItemHash;
QHash<QString, QTreeWidgetItem *> qmc2HierarchyItemHash;
QHash<QString, QString> qmc2ParentHash;
QHash<QString, QIcon> qmc2IconHash;
QHash<QString, QPair<QString, QAction *> > qmc2ShortcutHash;
QHash<QString, QString> qmc2CustomShortcutHash;
QList<QWidget *> qmc2ActiveViews;
QString qmc2DemoMachine;
QStringList qmc2DemoArgs;
int qmc2SortCriteria = QMC2_SORT_BY_DESCRIPTION;
Qt::SortOrder qmc2SortOrder = Qt::AscendingOrder;
bool qmc2SortingActive = false;
QBitArray qmc2Filter;
QMap<QString, unzFile> qmc2IconFileMap;
QMap<QString, SevenZipFile *> qmc2IconFileMap7z;
#if defined(QMC2_LIBARCHIVE_ENABLED)
QMap<QString, ArchiveFile *> qmc2IconArchiveMap;
#endif
MainEventFilter *qmc2MainEventFilter = 0;
QHash<QString, QKeySequence> qmc2QtKeyHash;
#if QMC2_JOYSTICK == 1
QHash<QString, QString> qmc2JoystickFunctionHash;
bool qmc2JoystickIsCalibrating = false;
#endif
bool qmc2ShowMachineName = false;
bool qmc2ShowMachineNameOnlyWhenRequired = true;
QMutex qmc2LogFrontendMutex;
QMutex qmc2LogEmulatorMutex;
QString qmc2FileEditStartPath;
QString qmc2DirectoryEditStartPath;
NetworkAccessManager *qmc2NetworkAccessManager = 0;
int qmc2LastListIndex = 0;
QAbstractItemView::ScrollHint qmc2CursorPositioningMode = QAbstractItemView::PositionAtTop;
QFont *qmc2StartupDefaultFont = 0;
int qmc2SoftwareSnapPosition = 0;
QSplashScreen *qmc2SplashScreen = 0;
int qmc2DefaultLaunchMode = QMC2_LAUNCH_MODE_INDEPENDENT;
QCache<QString, ImagePixmap> qmc2ImagePixmapCache;
QList<QTreeWidgetItem *> qmc2ExpandedMachineListItems;
QPalette qmc2CustomPalette;
QMap<QString, QPalette> qmc2StandardPalettes;
bool qmc2CategoryInfoUsed = false;
bool qmc2VersionInfoUsed = false;
bool qmc2TemplateCheck = false;
QMap<QWidget *, Qt::WindowStates> qmc2AutoMinimizedWidgets;
QTime qmc2StartupTimer;

// game status colors 
QColor MainWindow::qmc2StatusColorGreen = QColor("#00cc00");
QColor MainWindow::qmc2StatusColorYellowGreen = QColor("#799632");
QColor MainWindow::qmc2StatusColorRed = QColor("#f90000");
QColor MainWindow::qmc2StatusColorBlue = QColor("#0000f9");
QColor MainWindow::qmc2StatusColorGrey = QColor("#7f7f7f");
QList<MachineListViewer *> MainWindow::machineListViewers;

// extern global variables
extern QHash<QString, QStringList> systemSoftwareListHash;
extern QHash<QString, QStringList> systemSoftwareFilterHash;
#if defined(QMC2_OS_WIN)
extern QMap<HWND, QString> winWindowMap;
#endif
extern QHash<QString, QString> softwareParentHash;

#define userDataDb	qmc2MachineList->userDataDb()

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent, qmc2TemplateCheck ? Qt::Tool | Qt::FramelessWindowHint : (Qt::WindowFlags)0),
#if QMC2_USE_PHONON_API
	phononAudioPlayer(0),
	phononAudioOutput(0),
#endif
	m_ignoreSelectionChange(false),
	m_lastMlvSender(0)
{
	setUpdatesEnabled(false);
	setVisible(false);

	qmc2Config->setValue(QString(QMC2_FRONTEND_PREFIX + "InstanceRunning"), true);
	qmc2StartupDefaultFont = new QFont(qApp->font());
	desktopGeometry = qApp->desktop()->geometry();
	isActiveState = launchForeignID = negatedMatch = isCreatingSoftList = searchActive = stopSearch = lastPageSoftware = false;
	comboBoxEmuSelector = 0;
	proxyStyle = 0;
	swlDb = 0;
	videoSnapAllowedFormatExtensions << ".mp4" << ".avi";
	FileIconProvider::setCacheSize(QMC2_FILEICONPROVIDER_CACHE_SIZE);

	// remember the default style
	defaultStyle = QApplication::style()->objectName();

	// connections for style/style-sheet/palette setup requests
	connect(this, SIGNAL(styleSetupRequested(QString)), this, SLOT(setupStyle(QString)));
	connect(this, SIGNAL(styleSheetSetupRequested(QString)), this, SLOT(setupStyleSheet(QString)));
	connect(this, SIGNAL(paletteSetupRequested(QString)), this, SLOT(setupPalette(QString)));

	setupUi(this);

#if !defined(QMC2_WIP_ENABLED)
	actionNewFilteredView->setVisible(false);
#endif

	criticalActions << actionRebuildROM << actionRebuildROMTagged;
	rebuildRomActions << actionRebuildROM << actionRebuildROMTagged;

	// palette-editor related
	PaletteEditor::colorNames << "Window" << "WindowText" << "Base" << "AlternateBase" << "Text" << "BrightText" << "Button"
		<< "ButtonText" << "ToolTipBase" << "ToolTipText" << "Light" << "Midlight" << "Dark" << "Mid"
		<< "Shadow" << "Highlight" << "HighlightedText" << "Link" << "LinkVisited";
	BrushEditor::patternNames << "NoBrush" << "SolidPattern" << "Dense1Pattern" << "Dense2Pattern" << "Dense3Pattern" << "Dense4Pattern"
		<< "Dense5Pattern" << "Dense6Pattern" << "Dense7Pattern" << "HorPattern" << "VerPattern" << "CrossPattern"
		<< "BDiagPattern" << "FDiagPattern" << "DiagCrossPattern";
	BrushEditor::gradientTypeNames << "Linear" << "Radial" << "Conical";
	BrushEditor::gradientSpreadNames << "PadSpread" << "RepeatSpread" << "ReflectSpread";

	progressBarSearch->setVisible(false);
	progressBarSearch->setFormat(tr("Searching machines - %p%"));

	qmc2ActiveViews << treeWidgetMachineList << treeWidgetHierarchy << treeWidgetCategoryView << treeWidgetVersionView;

#if defined(QMC2_OS_MAC)
	// we cannot use a native menu-bar on Mac OS X since we have multiple and this conflicts with it
	menuBar()->setNativeMenuBar(false);
#endif

	// enable menu tear-off
	foreach (QMenu *menu, menuBar()->findChildren<QMenu *>())
		menu->setTearOffEnabled(true);

	// toolbar's search combo-box
	comboBoxToolbarSearch = new QComboBox(this);
	comboBoxToolbarSearch->setLineEdit(new IconLineEdit(QIcon(QString::fromUtf8(":/data/img/find.png")), QMC2_ALIGN_LEFT, comboBoxToolbarSearch));
	comboBoxToolbarSearch->setEditable(true);
	comboBoxToolbarSearch->setInsertPolicy(QComboBox::InsertAtTop);
	comboBoxToolbarSearch->lineEdit()->setPlaceholderText(tr("Enter search string"));
	comboBoxToolbarSearch->setToolTip(tr("Search for machines (not case-sensitive)"));
	comboBoxToolbarSearch->setStatusTip(tr("Search for machines"));
	comboBoxToolbarSearch->setToolTip(comboBoxToolbarSearch->toolTip() + " - " + tr("note: the special characters $, (, ), *, +, ., ?, [, ], ^, {, |, } and \\ must be escaped when they are meant literally!"));
	comboBoxToolbarSearch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	widgetActionToolbarSearch = new QWidgetAction(this);
	widgetActionToolbarSearch->setDefaultWidget(comboBoxToolbarSearch);
	widgetActionToolbarSearch->setObjectName("WATS");
	toolbar->addSeparator();
	toolbar->addAction(widgetActionToolbarSearch);
	connect(comboBoxToolbarSearch, SIGNAL(activated(const QString &)), this, SLOT(comboBoxToolbarSearch_activated(const QString &)));
	connect(comboBoxToolbarSearch, SIGNAL(editTextChanged(const QString &)), this, SLOT(comboBoxToolbarSearch_editTextChanged(const QString &)));

	// save splitter widgets at index 0 for later comparison
	hSplitterWidget0 = hSplitter->widget(0);
	vSplitterWidget0 = vSplitter->widget(0);

	// disable the menu-bar's default context menu (may be irritating)
	menuBar()->setContextMenuPolicy(Qt::PreventContextMenu);

	// loading animation
	loadAnimMovie = new QMovie(QString::fromUtf8(":/data/img/loadanim.gif"), QByteArray(), this);
	loadAnimMovie->setCacheMode(QMovie::CacheAll);
	loadAnimMovie->setSpeed(QMC2_LOADANIM_SPEED);
	loadAnimMovie->stop();
	nullMovie = new QMovie();

	// replace loading animation labels with aspect-ratio keeping ones
	gridLayoutMachineListPage->removeWidget(labelLoadingMachineList);
	delete labelLoadingMachineList;
	labelLoadingMachineList = new AspectRatioLabel(this);
	labelLoadingMachineList->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	labelLoadingMachineList->setMovie(loadAnimMovie);
	gridLayoutMachineListPage->addWidget(labelLoadingMachineList, 1, 0);
	labelLoadingMachineList->setVisible(false);

	gridLayoutHierarchyPage->removeWidget(labelLoadingHierarchy);
	delete labelLoadingHierarchy;
	labelLoadingHierarchy = new AspectRatioLabel(this);
	labelLoadingHierarchy->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	labelLoadingHierarchy->setMovie(loadAnimMovie);
	gridLayoutHierarchyPage->addWidget(labelLoadingHierarchy, 1, 0);
	labelLoadingHierarchy->setVisible(false);

	gridLayoutCategoryPage->removeWidget(labelCreatingCategoryView);
	delete labelCreatingCategoryView;
	labelCreatingCategoryView = new AspectRatioLabel(this);
	labelCreatingCategoryView->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	labelCreatingCategoryView->setMovie(loadAnimMovie);
	gridLayoutCategoryPage->addWidget(labelCreatingCategoryView, 1, 0);
	labelCreatingCategoryView->setVisible(false);

	gridLayoutVersionPage->removeWidget(labelCreatingVersionView);
	delete labelCreatingVersionView;
	labelCreatingVersionView = new AspectRatioLabel(this);
	labelCreatingVersionView->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	labelCreatingVersionView->setMovie(loadAnimMovie);
	gridLayoutVersionPage->addWidget(labelCreatingVersionView, 1, 0);
	labelCreatingVersionView->setVisible(false);

	checkBoxAudioFade->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	treeWidgetMachineList->setMouseTracking(true);
	treeWidgetHierarchy->setMouseTracking(true);
	treeWidgetCategoryView->setMouseTracking(true);
	treeWidgetVersionView->setMouseTracking(true);

	labelMachineStatus->setVisible(false);
	labelMachineStatus->setPalette(qmc2StatusColorBlue);

#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
	embedderCornerWidget = new QWidget(tabWidgetEmbeddedEmulators);
	embedderCornerLayout = new QHBoxLayout(embedderCornerWidget);
	embedderCornerLayout->setContentsMargins(0, 0, 0, 0);

#if defined(QMC2_OS_UNIX)
	toolButtonEmbedderAutoPause = new QToolButton(embedderCornerWidget);
	toolButtonEmbedderAutoPause->setIcon(QIcon(QString::fromUtf8(":/data/img/sleep.png")));
	toolButtonEmbedderAutoPause->setToolTip(tr("Toggle automatic pausing of embedded emulators (hold down for menu)"));
	toolButtonEmbedderAutoPause->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
	toolButtonEmbedderAutoPause->setToolButtonStyle(Qt::ToolButtonIconOnly);
	toolButtonEmbedderAutoPause->setAutoRaise(true);
	toolButtonEmbedderAutoPause->setCheckable(true);
	toolButtonEmbedderAutoPause->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/Embedder/AutoPause", false).toBool());
	embedderCornerLayout->addWidget(toolButtonEmbedderAutoPause);

	menuAutoPause = new QMenu(0);
	QString apMenuString = tr("Scan the pause key used by the emulator");
	QAction *apMenuAction = menuAutoPause->addAction(tr("Scan pause key..."));
	apMenuAction->setIcon(QIcon(QString::fromUtf8(":/data/img/keyboard.png")));
	apMenuAction->setToolTip(apMenuString); apMenuAction->setStatusTip(apMenuString);
	connect(apMenuAction, SIGNAL(triggered()), this, SLOT(action_embedderScanPauseKey_triggered()));
	toolButtonEmbedderAutoPause->setMenu(menuAutoPause);
#endif

	toolButtonEmbedderMaximizeToggle = new QToolButton(embedderCornerWidget);
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/Embedder/Maximize", false).toBool() )
		toolButtonEmbedderMaximizeToggle->setIcon(QIcon(QString::fromUtf8(":/data/img/minimize.png")));
	else
		toolButtonEmbedderMaximizeToggle->setIcon(QIcon(QString::fromUtf8(":/data/img/maximize.png")));
	toolButtonEmbedderMaximizeToggle->setToolTip(tr("Toggle maximization of embedded emulator windows"));
	toolButtonEmbedderMaximizeToggle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
	toolButtonEmbedderMaximizeToggle->setToolButtonStyle(Qt::ToolButtonIconOnly);
	toolButtonEmbedderMaximizeToggle->setAutoRaise(true);
	toolButtonEmbedderMaximizeToggle->setCheckable(true);
	toolButtonEmbedderMaximizeToggle->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/Embedder/Maximize", false).toBool());
	connect(toolButtonEmbedderMaximizeToggle, SIGNAL(toggled(bool)), this, SLOT(toolButtonEmbedderMaximizeToggle_toggled(bool)));
	embedderCornerLayout->addWidget(toolButtonEmbedderMaximizeToggle);

	embedderCornerWidget->setLayout(embedderCornerLayout);
	tabWidgetEmbeddedEmulators->setCornerWidget(embedderCornerWidget, Qt::TopRightCorner);

	widgetEmbeddedEmus = tabWidgetMachineList->widget(tabWidgetMachineList->indexOf(tabEmbeddedEmus));
#else
	actionPlayEmbedded->setVisible(false);
	actionPlayEmbeddedTagged->setVisible(false);
#endif
	tabWidgetEmbeddedEmulators->removeTab(0);
	tabWidgetMachineList->removeTab(tabWidgetMachineList->indexOf(tabEmbeddedEmus));

#if !defined(QMC2_YOUTUBE_ENABLED)
	actionClearYouTubeCache->setVisible(false);
#endif
#if QMC2_MULTIMEDIA_ENABLED
	toolButtonAudioSetupEffects->setVisible(false);
#endif
#if QT_VERSION < 0x050000
	lcdNumberSearchResults->setNumDigits(5);
#else
	lcdNumberSearchResults->setDigitCount(5);
#endif

	qmc2ProjectMESSCache.setMaxCost(QMC2_PROJECTMESS_CACHE_SIZE);
	messDevCfgTimer.setSingleShot(true);

	floatToggleButtonSoftwareDetail = new QToolButton(tabWidgetSoftwareDetail);
	floatToggleButtonSoftwareDetail->setCheckable(true);
	floatToggleButtonSoftwareDetail->setChecked(true);
	floatToggleButtonSoftwareDetail->setToolTip(tr("Dock / undock this widget"));
	floatToggleButtonSoftwareDetail->setIcon(QIcon(QString::fromUtf8(":/data/img/dock.png")));
	tabWidgetSoftwareDetail->setCornerWidget(floatToggleButtonSoftwareDetail, Qt::TopRightCorner);

	comboBoxSearch->setToolTip(comboBoxSearch->toolTip() + " - " + tr("note: the special characters $, (, ), *, +, ., ?, [, ], ^, {, |, } and \\ must be escaped when they are meant literally!"));
	setWindowTitle(windowTitle() + " [Qt " + qVersion() + "]");

	// tabs are movable
	QTabBar *tabBar;
	tabWidgetMachineList->setMovable(true);
	tabBar = tabWidgetMachineList->findChild<QTabBar *>();
	if ( tabBar )
		connect(tabBar, SIGNAL(tabMoved(int, int)), this, SLOT(tabWidgetMachineList_tabMoved(int, int)));
	tabWidgetMachineDetail->setMovable(true);
	tabBar = tabWidgetMachineDetail->findChild<QTabBar *>();
	if ( tabBar )
		connect(tabBar, SIGNAL(tabMoved(int, int)), this, SLOT(tabWidgetMachineDetail_tabMoved(int, int)));
	tabWidgetLogsAndEmulators->setMovable(true);
	tabBar = tabWidgetLogsAndEmulators->findChild<QTabBar *>();
	if ( tabBar )
		connect(tabBar, SIGNAL(tabMoved(int, int)), this, SLOT(tabWidgetLogsAndEmulators_tabMoved(int, int)));
	tabWidgetSoftwareDetail->setMovable(true);
	tabBar = tabWidgetSoftwareDetail->findChild<QTabBar *>();
	if ( tabBar )
		connect(tabBar, SIGNAL(tabMoved(int, int)), this, SLOT(tabWidgetSoftwareDetail_tabMoved(int, int)));

	qmc2Options->checkBoxShowMachineName->setText(tr("Show machine/software titles"));
	qmc2Options->checkBoxShowMachineName->setToolTip(tr("Show machine- or software-titles at the bottom of all images"));
	qmc2Options->checkBoxShowMachineNameOnlyWhenRequired->setToolTip(tr("Show machine- or software-titles only when the machine list is not visible due to the current layout"));

	qmc2MachineList = new MachineList(this);
	labelMachineListStatus->setText(qmc2MachineList->status());

	statusBar()->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Statusbar").toBool());

	// machine image widgets
	QHBoxLayout *previewLayout = new QHBoxLayout, 
		    *flyerLayout = new QHBoxLayout,
		    *cabinetLayout = new QHBoxLayout,
		    *controllerLayout = new QHBoxLayout,
		    *marqueeLayout = new QHBoxLayout,
		    *titleLayout = new QHBoxLayout,
		    *pcbLayout = new QHBoxLayout;

	qmc2Preview = new Preview(tabPreview);
	previewLayout->addWidget(qmc2Preview);
	previewLayout->setContentsMargins(0, 0, 0, 0);
	tabPreview->setLayout(previewLayout);
	qmc2Flyer = new Flyer(tabFlyer);
	flyerLayout->addWidget(qmc2Flyer);
	flyerLayout->setContentsMargins(0, 0, 0, 0);
	tabFlyer->setLayout(flyerLayout);
	qmc2Cabinet = new Cabinet(tabCabinet);
	cabinetLayout->addWidget(qmc2Cabinet);
	cabinetLayout->setContentsMargins(0, 0, 0, 0);
	tabCabinet->setLayout(cabinetLayout);
	qmc2Controller = new Controller(tabController);
	controllerLayout->addWidget(qmc2Controller);
	controllerLayout->setContentsMargins(0, 0, 0, 0);
	tabController->setLayout(controllerLayout);
	qmc2Marquee = new Marquee(tabMarquee);
	marqueeLayout->addWidget(qmc2Marquee);
	marqueeLayout->setContentsMargins(0, 0, 0, 0);
	tabMarquee->setLayout(marqueeLayout);
	qmc2Title = new Title(tabTitle);
	titleLayout->addWidget(qmc2Title);
	titleLayout->setContentsMargins(0, 0, 0, 0);
	tabTitle->setLayout(titleLayout);
	qmc2PCB = new PCB(tabPCB);
	pcbLayout->addWidget(qmc2PCB);
	pcbLayout->setContentsMargins(0, 0, 0, 0);
	tabPCB->setLayout(pcbLayout);

	// remove column-width restrictions
	treeWidgetMachineList->header()->setMinimumSectionSize(0);
	treeWidgetHierarchy->header()->setMinimumSectionSize(0);
	treeWidgetCategoryView->header()->setMinimumSectionSize(0);
	treeWidgetVersionView->header()->setMinimumSectionSize(0);

	// restore layout
	if ( !qmc2TemplateCheck ) {
		menuBar()->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ShowMenuBar", true).toBool());
		QSize hSplitterSize = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/hSplitter").toSize();
		QSize vSplitterSize = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/vSplitter").toSize();
		QSize vSplitterSizeSoftwareDetail = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/vSplitterSoftwareDetail").toSize();
		if ( hSplitterSize.width() > 0 || hSplitterSize.height() > 0 )
			hSplitterSizes << hSplitterSize.width() << hSplitterSize.height();
		else
			hSplitterSizes << 100 << 100;
		if ( vSplitterSize.width() > 0 || vSplitterSize.height() > 0 )
			vSplitterSizes << vSplitterSize.width() << vSplitterSize.height();
		else
			vSplitterSizes << 100 << 100;
		if ( vSplitterSizeSoftwareDetail.width() > 0 || vSplitterSizeSoftwareDetail.height() > 0 )
			vSplitterSizesSoftwareDetail << vSplitterSizeSoftwareDetail.width() << vSplitterSizeSoftwareDetail.height();
		else
			vSplitterSizesSoftwareDetail << 100 << 100;
		hSplitter->setSizes(hSplitterSizes);
		vSplitter->setSizes(vSplitterSizes);
		treeWidgetHierarchy->header()->setDefaultAlignment(Qt::AlignLeft);
		treeWidgetMachineList->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/MachineListHeaderState").toByteArray());
		treeWidgetHierarchy->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/HierarchyHeaderState").toByteArray());
		treeWidgetCategoryView->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/CategoryViewHeaderState").toByteArray());
		treeWidgetCategoryView->setColumnHidden(QMC2_MACHINELIST_COLUMN_CATEGORY, true);
		treeWidgetVersionView->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/VersionViewHeaderState").toByteArray());
		treeWidgetVersionView->setColumnHidden(QMC2_MACHINELIST_COLUMN_VERSION, true);
		treeWidgetEmulators->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/EmulatorControlHeaderState").toByteArray());
		actionFullscreenToggle->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Fullscreen", false).toBool());
		tabWidgetMachineList->setTabPosition((QTabWidget::TabPosition)qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MachineList/TabPosition", QTabWidget::North).toInt());
		tabWidgetMachineDetail->setTabPosition((QTabWidget::TabPosition)qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MachineDetail/TabPosition", QTabWidget::North).toInt());
		tabWidgetLogsAndEmulators->setTabPosition((QTabWidget::TabPosition)qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/LogsAndEmulators/TabPosition", QTabWidget::North).toInt());
		tabWidgetSoftwareDetail->setTabPosition((QTabWidget::TabPosition)qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareDetail/TabPosition", QTabWidget::North).toInt());
		floatToggleButtonSoftwareDetail->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/SoftwareDetailDocked", true).toBool());
	} else {
		QList<int> splitterSizes;
		splitterSizes << 100 << 100;
		hSplitter->setSizes(splitterSizes);
		vSplitter->setSizes(splitterSizes);
		hSplitterSizes = splitterSizes;
		vSplitterSizes = splitterSizes;
		vSplitterSizesSoftwareDetail = splitterSizes;
		floatToggleButtonSoftwareDetail->setChecked(true);
	}

	on_actionFullscreenToggle_triggered();
	update_rebuildRomActions_visibility();

	actionSearchInternalBrowser->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "WebSearch/InternalBrowser", false).toBool());
	on_actionSearchInternalBrowser_triggered(actionSearchInternalBrowser->isChecked());
	actionManualInternalViewer->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "PdfViewer/Internal", false).toBool());
	on_actionManualInternalViewer_triggered(actionManualInternalViewer->isChecked());

	// context menus
	QAction *action;
	QString s;

	qmc2EmulatorMenu = new QMenu(0);
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
	s = tr("Embed emulator widget");
	action = qmc2EmulatorMenu->addAction(tr("&Embed"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/embed.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(action_embedEmulator_triggered()));
#endif
	s = tr("Copy emulator command line to clipboard");
	action = qmc2EmulatorMenu->addAction(tr("&Copy command"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/editcopy.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(action_copyEmulatorCommand_triggered()));
	qmc2EmulatorMenu->addSeparator();
	s = tr("Kill selected emulator(s) (sends KILL signal to emulator process(es))");
	action = qmc2EmulatorMenu->addAction(tr("&Kill"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/kill.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(action_killEmulator_triggered()));

	qmc2ForeignIDsMenu = new QMenu(0);
	s = tr("Play selected machine");
	action = qmc2ForeignIDsMenu->addAction(tr("&Play"));
	contextMenuPlayActions.append(action);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/launch.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(on_actionPlay_triggered()));
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
	s = tr("Play selected machine (embedded)");
	action = qmc2ForeignIDsMenu->addAction(tr("Play &embedded"));
	contextMenuPlayActions.append(action);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/embed.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(on_actionPlayEmbedded_triggered()));
#endif

	qmc2MachineMenu = new QMenu(0);
	s = tr("Play selected machine");
	action = qmc2MachineMenu->addAction(tr("&Play"));
	contextMenuPlayActions.append(action);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/launch.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(on_actionPlay_triggered()));
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
	s = tr("Play selected machine (embedded)");
	action = qmc2MachineMenu->addAction(tr("Play &embedded"));
	contextMenuPlayActions.append(action);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/embed.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(on_actionPlayEmbedded_triggered()));
#endif
	s = tr("Add current machine to favorites");
	action = qmc2MachineMenu->addAction(tr("To &favorites"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/favorites.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(on_actionToFavorites_triggered()));
	qmc2MachineMenu->addSeparator();
	s = tr("Check current machine's ROM state");
	action = qmc2MachineMenu->addAction(tr("Check &ROM state"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/rom.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(on_actionCheckCurrentROM_triggered()));
	s = tr("Analyse current machine's ROM set with the ROMAlyzer");
	action = qmc2MachineMenu->addAction(tr("&Analyse ROM..."));
	criticalActions << action;
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/romalyzer.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(on_actionAnalyseCurrentROM_triggered()));
	s = tr("Rebuild current machine's ROM set with the ROMAlyzer");
	action = qmc2MachineMenu->addAction(tr("&Rebuild ROM..."));
	criticalActions << action;
	rebuildRomActions << action;
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/rebuild.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(actionRebuildRom_triggered()));
	qmc2MachineMenu->addSeparator();
	qmc2MachineMenu->addMenu(menuRank);
	qmc2MachineMenu->addMenu(menuSearchWeb);
	qmc2MachineMenu->addMenu(menuManual);

	qmc2SearchMenu = new QMenu(0);
	s = tr("Play selected machine");
	action = qmc2SearchMenu->addAction(tr("&Play"));
	contextMenuPlayActions.append(action);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/launch.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(on_actionPlay_triggered()));
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
	s = tr("Play selected machine (embedded)");
	action = qmc2SearchMenu->addAction(tr("Play &embedded"));
	contextMenuPlayActions.append(action);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/embed.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(on_actionPlayEmbedded_triggered()));
#endif
	s = tr("Add current machine to favorites");
	action = qmc2SearchMenu->addAction(tr("To &favorites"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/favorites.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(on_actionToFavorites_triggered()));
	qmc2SearchMenu->addSeparator();
	s = tr("Check current machine's ROM state");
	action = qmc2SearchMenu->addAction(tr("Check &ROM state"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/rom.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(on_actionCheckCurrentROM_triggered()));
	s = tr("Analyse current machine's ROM set with the ROMAlyzer");
	action = qmc2SearchMenu->addAction(tr("&Analyse ROM..."));
	criticalActions << action;
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/romalyzer.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(on_actionAnalyseCurrentROM_triggered()));
	s = tr("Rebuild current machine's ROM set with the ROMAlyzer");
	action = qmc2SearchMenu->addAction(tr("&Rebuild ROM..."));
	criticalActions << action;
	rebuildRomActions << action;
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/rebuild.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(actionRebuildRom_triggered()));

	qmc2SearchMenu->addSeparator();
	qmc2SearchMenu->addMenu(menuRank);
	qmc2SearchMenu->addMenu(menuSearchWeb);
	qmc2SearchMenu->addMenu(menuManual);

	qmc2FavoritesMenu = new QMenu(0);
	s = tr("Play selected machine");
	action = qmc2FavoritesMenu->addAction(tr("&Play"));
	contextMenuPlayActions.append(action);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/launch.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(on_actionPlay_triggered()));
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
	s = tr("Play selected machine (embedded)");
	action = qmc2FavoritesMenu->addAction(tr("Play &embedded"));
	contextMenuPlayActions.append(action);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/embed.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(on_actionPlayEmbedded_triggered()));
#endif
	qmc2FavoritesMenu->addSeparator();
	s = tr("Check current machine's ROM state");
	action = qmc2FavoritesMenu->addAction(tr("Check &ROM state"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/rom.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(on_actionCheckCurrentROM_triggered()));
	s = tr("Analyse current machine's ROM set with the ROMAlyzer");
	action = qmc2FavoritesMenu->addAction(tr("&Analyse ROM..."));
	criticalActions << action;
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/romalyzer.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(on_actionAnalyseCurrentROM_triggered()));
	s = tr("Rebuild current machine's ROM set with the ROMAlyzer");
	action = qmc2FavoritesMenu->addAction(tr("&Rebuild ROM..."));
	criticalActions << action;
	rebuildRomActions << action;
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/rebuild.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(actionRebuildRom_triggered()));
	qmc2FavoritesMenu->addSeparator();
	qmc2FavoritesMenu->addMenu(menuRank);
	qmc2FavoritesMenu->addMenu(menuSearchWeb);
	qmc2FavoritesMenu->addMenu(menuManual);
	qmc2FavoritesMenu->addSeparator();
	s = tr("Remove from favorites");
	action = qmc2FavoritesMenu->addAction(tr("&Remove"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/editdelete.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(action_removeFromFavorites_triggered()));
	s = tr("Clear all favorites");
	action = qmc2FavoritesMenu->addAction(tr("&Clear"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/broom.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(action_clearAllFavorites_triggered()));
	s = tr("Save favorites now");
	action = qmc2FavoritesMenu->addAction(tr("&Save"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/filesave.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(action_saveFavorites_triggered()));

	qmc2PlayedMenu = new QMenu(0);
	s = tr("Play selected machine");
	action = qmc2PlayedMenu->addAction(tr("&Play"));
	contextMenuPlayActions.append(action);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/launch.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(on_actionPlay_triggered()));
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
	s = tr("Play selected machine (embedded)");
	action = qmc2PlayedMenu->addAction(tr("Play &embedded"));
	contextMenuPlayActions.append(action);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/embed.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(on_actionPlayEmbedded_triggered()));
#endif
	s = tr("Add current machine to favorites");
	action = qmc2PlayedMenu->addAction(tr("To &favorites"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/favorites.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(on_actionToFavorites_triggered()));
	qmc2PlayedMenu->addSeparator();
	s = tr("Check current machine's ROM state");
	action = qmc2PlayedMenu->addAction(tr("Check &ROM state"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/rom.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(on_actionCheckCurrentROM_triggered()));
	s = tr("Analyse current machine's ROM set with the ROMAlyzer");
	action = qmc2PlayedMenu->addAction(tr("&Analyse ROM..."));
	criticalActions << action;
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/romalyzer.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(on_actionAnalyseCurrentROM_triggered()));
	s = tr("Rebuild current machine's ROM set with the ROMAlyzer");
	action = qmc2PlayedMenu->addAction(tr("&Rebuild ROM..."));
	criticalActions << action;
	rebuildRomActions << action;
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/rebuild.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(actionRebuildRom_triggered()));
	qmc2PlayedMenu->addSeparator();
	qmc2PlayedMenu->addMenu(menuRank);
	qmc2PlayedMenu->addMenu(menuSearchWeb);
	qmc2PlayedMenu->addMenu(menuManual);
	qmc2PlayedMenu->addSeparator();
	s = tr("Remove from played");
	action = qmc2PlayedMenu->addAction(tr("&Remove"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/editdelete.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(action_removeFromPlayed_triggered()));
	s = tr("Clear all played");
	action = qmc2PlayedMenu->addAction(tr("&Clear"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/broom.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(action_clearAllPlayed_triggered()));
	s = tr("Save play-history now");
	action = qmc2PlayedMenu->addAction(tr("&Save"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/filesave.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(action_savePlayed_triggered()));

	QMenu *m;

	// tab widget position menus
	menuTabWidgetMachineList = new QMenu(0);
	m = new QMenu(tr("Tab position"), 0);
	m->setIcon(QIcon(QString::fromUtf8(":/data/img/tabwidget.png")));
	menuTabWidgetMachineList->addMenu(m);
	s = tr("Set tab position north");
	action = m->addAction(tr("&North"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/north.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(menuTabWidgetMachineList_North_activated()));
	s = tr("Set tab position south");
	action = m->addAction(tr("&South"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/south.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(menuTabWidgetMachineList_South_activated()));
	s = tr("Set tab position west");
	action = m->addAction(tr("&West"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/west.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(menuTabWidgetMachineList_West_activated()));
	s = tr("Set tab position east");
	action = m->addAction(tr("&East"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/east.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(menuTabWidgetMachineList_East_activated()));
	menuTabWidgetMachineList->addSeparator();
	s = tr("Component setup");
	action = menuTabWidgetMachineList->addAction(tr("Component setup..."));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/work.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(menuTabWidgetMachineList_Setup_activated()));

	menuTabWidgetMachineDetail = new QMenu(0);
	m = new QMenu(tr("Tab position"), 0);
	m->setIcon(QIcon(QString::fromUtf8(":/data/img/tabwidget.png")));
	menuTabWidgetMachineDetail->addMenu(m);
	s = tr("Set tab position north");
	action = m->addAction(tr("&North"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/north.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(menuTabWidgetMachineDetail_North_activated()));
	s = tr("Set tab position south");
	action = m->addAction(tr("&South"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/south.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(menuTabWidgetMachineDetail_South_activated()));
	s = tr("Set tab position west");
	action = m->addAction(tr("&West"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/west.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(menuTabWidgetMachineDetail_West_activated()));
	s = tr("Set tab position east");
	action = m->addAction(tr("&East"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/east.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(menuTabWidgetMachineDetail_East_activated()));
	menuTabWidgetMachineDetail->addSeparator();
	s = tr("Component setup");
	action = menuTabWidgetMachineDetail->addAction(tr("Component setup..."));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/work.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(menuTabWidgetMachineDetail_Setup_activated()));

	menuTabWidgetLogsAndEmulators = new QMenu(0);
	m = new QMenu(tr("Tab position"), 0);
	m->setIcon(QIcon(QString::fromUtf8(":/data/img/tabwidget.png")));
	menuTabWidgetLogsAndEmulators->addMenu(m);
	s = tr("Set tab position north");
	action = m->addAction(tr("&North"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/north.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(menuTabWidgetLogsAndEmulators_North_activated()));
	s = tr("Set tab position south");
	action = m->addAction(tr("&South"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/south.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(menuTabWidgetLogsAndEmulators_South_activated()));
	s = tr("Set tab position west");
	action = m->addAction(tr("&West"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/west.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(menuTabWidgetLogsAndEmulators_West_activated()));
	s = tr("Set tab position east");
	action = m->addAction(tr("&East"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/east.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(menuTabWidgetLogsAndEmulators_East_activated()));
	menuTabWidgetLogsAndEmulators->addSeparator();
	s = tr("Component setup");
	action = menuTabWidgetLogsAndEmulators->addAction(tr("Component setup..."));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/work.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(menuTabWidgetLogsAndEmulators_Setup_activated()));

	menuTabWidgetSoftwareDetail = new QMenu(0);
	m = new QMenu(tr("Tab position"), 0);
	m->setIcon(QIcon(QString::fromUtf8(":/data/img/tabwidget.png")));
	menuTabWidgetSoftwareDetail->addMenu(m);
	s = tr("Set tab position north");
	action = m->addAction(tr("&North"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/north.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(menuTabWidgetSoftwareDetail_North_activated()));
	s = tr("Set tab position south");
	action = m->addAction(tr("&South"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/south.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(menuTabWidgetSoftwareDetail_South_activated()));
	s = tr("Set tab position west");
	action = m->addAction(tr("&West"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/west.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(menuTabWidgetSoftwareDetail_West_activated()));
	s = tr("Set tab position east");
	action = m->addAction(tr("&East"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/east.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(menuTabWidgetSoftwareDetail_East_activated()));
	menuTabWidgetSoftwareDetail->addSeparator();
	s = tr("Component setup");
	action = menuTabWidgetSoftwareDetail->addAction(tr("Component setup..."));
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/work.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(menuTabWidgetSoftwareDetail_Setup_activated()));

	QHeaderView *header;

	menuMachineListHeader = new QMenu(0);
	header = treeWidgetMachineList->header();
	action = menuMachineListHeader->addAction(tr("Machine / Attribute"), this, SLOT(actionMachineListHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_MACHINE);
	action->setChecked(!treeWidgetMachineList->isColumnHidden(QMC2_MACHINELIST_COLUMN_MACHINE));
	action = menuMachineListHeader->addAction(tr("Tag"), this, SLOT(actionMachineListHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_TAG);
	action->setChecked(!treeWidgetMachineList->isColumnHidden(QMC2_MACHINELIST_COLUMN_TAG));
	action = menuMachineListHeader->addAction(tr("Icon / Value"), this, SLOT(actionMachineListHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_ICON);
	action->setChecked(!treeWidgetMachineList->isColumnHidden(QMC2_MACHINELIST_COLUMN_ICON));
	action = menuMachineListHeader->addAction(tr("Year"), this, SLOT(actionMachineListHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_YEAR);
	action->setChecked(!treeWidgetMachineList->isColumnHidden(QMC2_MACHINELIST_COLUMN_YEAR));
	action = menuMachineListHeader->addAction(tr("Manufacturer"), this, SLOT(actionMachineListHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_MANU);
	action->setChecked(!treeWidgetMachineList->isColumnHidden(QMC2_MACHINELIST_COLUMN_MANU));
	action = menuMachineListHeader->addAction(tr("Name"), this, SLOT(actionMachineListHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_NAME);
	action->setChecked(!treeWidgetMachineList->isColumnHidden(QMC2_MACHINELIST_COLUMN_NAME));
	action = menuMachineListHeader->addAction(tr("ROM types"), this, SLOT(actionMachineListHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_RTYPES);
	action->setChecked(!treeWidgetMachineList->isColumnHidden(QMC2_MACHINELIST_COLUMN_RTYPES));
	action = menuMachineListHeader->addAction(tr("Players"), this, SLOT(actionMachineListHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_PLAYERS);
	action->setChecked(!treeWidgetMachineList->isColumnHidden(QMC2_MACHINELIST_COLUMN_PLAYERS));
	action = menuMachineListHeader->addAction(tr("Driver status"), this, SLOT(actionMachineListHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_DRVSTAT);
	action->setChecked(!treeWidgetMachineList->isColumnHidden(QMC2_MACHINELIST_COLUMN_DRVSTAT));
	action = menuMachineListHeader->addAction(tr("Source file"), this, SLOT(actionMachineListHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_SRCFILE);
	action->setChecked(!treeWidgetMachineList->isColumnHidden(QMC2_MACHINELIST_COLUMN_SRCFILE));
	action = menuMachineListHeader->addAction(tr("Rank"), this, SLOT(actionMachineListHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_RANK);
	action->setChecked(!treeWidgetMachineList->isColumnHidden(QMC2_MACHINELIST_COLUMN_RANK));
	action = menuMachineListHeader->addAction(tr("Category"), this, SLOT(actionMachineListHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_CATEGORY);
	action->setChecked(!treeWidgetMachineList->isColumnHidden(QMC2_MACHINELIST_COLUMN_CATEGORY));
	actionMenuMachineListHeaderCategory = action;
	action = menuMachineListHeader->addAction(tr("Version"), this, SLOT(actionMachineListHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_VERSION);
	action->setChecked(!treeWidgetMachineList->isColumnHidden(QMC2_MACHINELIST_COLUMN_VERSION));
	actionMenuMachineListHeaderVersion = action;
	menuMachineListHeader->addSeparator();
	action = menuMachineListHeader->addAction(QIcon(":data/img/reset.png"), tr("Reset"), this, SLOT(actionMachineListHeader_triggered())); action->setData(QMC2_MACHINELIST_RESET);
	header->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(header, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(treeWidgetMachineListHeader_customContextMenuRequested(const QPoint &)));

	menuHierarchyHeader = new QMenu(0);
	header = treeWidgetHierarchy->header();
	action = menuHierarchyHeader->addAction(tr("Machine / Clones"), this, SLOT(actionHierarchyHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_MACHINE);
	action->setChecked(!treeWidgetHierarchy->isColumnHidden(QMC2_MACHINELIST_COLUMN_MACHINE));
	action = menuHierarchyHeader->addAction(tr("Tag"), this, SLOT(actionHierarchyHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_TAG);
	action->setChecked(!treeWidgetHierarchy->isColumnHidden(QMC2_MACHINELIST_COLUMN_TAG));
	action = menuHierarchyHeader->addAction(tr("Icon"), this, SLOT(actionHierarchyHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_ICON);
	action->setChecked(!treeWidgetHierarchy->isColumnHidden(QMC2_MACHINELIST_COLUMN_ICON));
	action = menuHierarchyHeader->addAction(tr("Year"), this, SLOT(actionHierarchyHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_YEAR);
	action->setChecked(!treeWidgetHierarchy->isColumnHidden(QMC2_MACHINELIST_COLUMN_YEAR));
	action = menuHierarchyHeader->addAction(tr("Manufacturer"), this, SLOT(actionHierarchyHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_MANU);
	action->setChecked(!treeWidgetHierarchy->isColumnHidden(QMC2_MACHINELIST_COLUMN_MANU));
	action = menuHierarchyHeader->addAction(tr("Name"), this, SLOT(actionHierarchyHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_NAME);
	action->setChecked(!treeWidgetHierarchy->isColumnHidden(QMC2_MACHINELIST_COLUMN_NAME));
	action = menuHierarchyHeader->addAction(tr("ROM types"), this, SLOT(actionHierarchyHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_RTYPES);
	action->setChecked(!treeWidgetHierarchy->isColumnHidden(QMC2_MACHINELIST_COLUMN_RTYPES));
	action = menuHierarchyHeader->addAction(tr("Players"), this, SLOT(actionHierarchyHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_PLAYERS);
	action->setChecked(!treeWidgetHierarchy->isColumnHidden(QMC2_MACHINELIST_COLUMN_PLAYERS));
	action = menuHierarchyHeader->addAction(tr("Driver status"), this, SLOT(actionHierarchyHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_DRVSTAT);
	action->setChecked(!treeWidgetHierarchy->isColumnHidden(QMC2_MACHINELIST_COLUMN_DRVSTAT));
	action = menuHierarchyHeader->addAction(tr("Source file"), this, SLOT(actionHierarchyHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_SRCFILE);
	action->setChecked(!treeWidgetHierarchy->isColumnHidden(QMC2_MACHINELIST_COLUMN_SRCFILE));
	action = menuHierarchyHeader->addAction(tr("Rank"), this, SLOT(actionHierarchyHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_RANK);
	action->setChecked(!treeWidgetHierarchy->isColumnHidden(QMC2_MACHINELIST_COLUMN_RANK));
	action = menuHierarchyHeader->addAction(tr("Category"), this, SLOT(actionHierarchyHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_CATEGORY);
	action->setChecked(!treeWidgetHierarchy->isColumnHidden(QMC2_MACHINELIST_COLUMN_CATEGORY));
	actionMenuHierarchyHeaderCategory = action;
	action = menuHierarchyHeader->addAction(tr("Version"), this, SLOT(actionHierarchyHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_VERSION);
	action->setChecked(!treeWidgetHierarchy->isColumnHidden(QMC2_MACHINELIST_COLUMN_VERSION));
	actionMenuHierarchyHeaderVersion = action;
	menuHierarchyHeader->addSeparator();
	action = menuHierarchyHeader->addAction(QIcon(":data/img/reset.png"), tr("Reset"), this, SLOT(actionHierarchyHeader_triggered())); action->setData(QMC2_MACHINELIST_RESET);
	header->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(header, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(treeWidgetHierarchyHeader_customContextMenuRequested(const QPoint &)));

	menuCategoryHeader = new QMenu(0);
	header = treeWidgetCategoryView->header();
	action = menuCategoryHeader->addAction(tr("Category / Machine"), this, SLOT(actionCategoryHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_MACHINE);
	action->setChecked(!treeWidgetCategoryView->isColumnHidden(QMC2_MACHINELIST_COLUMN_MACHINE));
	action = menuCategoryHeader->addAction(tr("Tag"), this, SLOT(actionCategoryHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_TAG);
	action->setChecked(!treeWidgetCategoryView->isColumnHidden(QMC2_MACHINELIST_COLUMN_TAG));
	action = menuCategoryHeader->addAction(tr("Icon"), this, SLOT(actionCategoryHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_ICON);
	action->setChecked(!treeWidgetCategoryView->isColumnHidden(QMC2_MACHINELIST_COLUMN_ICON));
	action = menuCategoryHeader->addAction(tr("Year"), this, SLOT(actionCategoryHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_YEAR);
	action->setChecked(!treeWidgetCategoryView->isColumnHidden(QMC2_MACHINELIST_COLUMN_YEAR));
	action = menuCategoryHeader->addAction(tr("Manufacturer"), this, SLOT(actionCategoryHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_MANU);
	action->setChecked(!treeWidgetCategoryView->isColumnHidden(QMC2_MACHINELIST_COLUMN_MANU));
	action = menuCategoryHeader->addAction(tr("Name"), this, SLOT(actionCategoryHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_NAME);
	action->setChecked(!treeWidgetCategoryView->isColumnHidden(QMC2_MACHINELIST_COLUMN_NAME));
	action = menuCategoryHeader->addAction(tr("ROM types"), this, SLOT(actionCategoryHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_RTYPES);
	action->setChecked(!treeWidgetCategoryView->isColumnHidden(QMC2_MACHINELIST_COLUMN_RTYPES));
	action = menuCategoryHeader->addAction(tr("Players"), this, SLOT(actionCategoryHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_PLAYERS);
	action->setChecked(!treeWidgetCategoryView->isColumnHidden(QMC2_MACHINELIST_COLUMN_PLAYERS));
	action = menuCategoryHeader->addAction(tr("Driver status"), this, SLOT(actionCategoryHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_DRVSTAT);
	action->setChecked(!treeWidgetCategoryView->isColumnHidden(QMC2_MACHINELIST_COLUMN_DRVSTAT));
	action = menuCategoryHeader->addAction(tr("Source file"), this, SLOT(actionCategoryHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_SRCFILE);
	action->setChecked(!treeWidgetCategoryView->isColumnHidden(QMC2_MACHINELIST_COLUMN_SRCFILE));
	action = menuCategoryHeader->addAction(tr("Rank"), this, SLOT(actionCategoryHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_RANK);
	action->setChecked(!treeWidgetCategoryView->isColumnHidden(QMC2_MACHINELIST_COLUMN_RANK));
	action = menuCategoryHeader->addAction(tr("Version"), this, SLOT(actionCategoryHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_VERSION);
	action->setChecked(!treeWidgetCategoryView->isColumnHidden(QMC2_MACHINELIST_COLUMN_VERSION));
	actionMenuCategoryHeaderVersion = action;
	menuCategoryHeader->addSeparator();
	action = menuCategoryHeader->addAction(QIcon(":data/img/reset.png"), tr("Reset"), this, SLOT(actionCategoryHeader_triggered())); action->setData(QMC2_MACHINELIST_RESET);
	header->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(header, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(treeWidgetCategoryViewHeader_customContextMenuRequested(const QPoint &)));

	menuVersionHeader = new QMenu(0);
	header = treeWidgetVersionView->header();
	action = menuVersionHeader->addAction(tr("Version / Machine"), this, SLOT(actionVersionHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_MACHINE);
	action->setChecked(!treeWidgetVersionView->isColumnHidden(QMC2_MACHINELIST_COLUMN_MACHINE));
	action = menuVersionHeader->addAction(tr("Tag"), this, SLOT(actionVersionHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_TAG);
	action->setChecked(!treeWidgetVersionView->isColumnHidden(QMC2_MACHINELIST_COLUMN_TAG));
	action = menuVersionHeader->addAction(tr("Icon"), this, SLOT(actionVersionHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_ICON);
	action->setChecked(!treeWidgetVersionView->isColumnHidden(QMC2_MACHINELIST_COLUMN_ICON));
	action = menuVersionHeader->addAction(tr("Year"), this, SLOT(actionVersionHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_YEAR);
	action->setChecked(!treeWidgetVersionView->isColumnHidden(QMC2_MACHINELIST_COLUMN_YEAR));
	action = menuVersionHeader->addAction(tr("Manufacturer"), this, SLOT(actionVersionHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_MANU);
	action->setChecked(!treeWidgetVersionView->isColumnHidden(QMC2_MACHINELIST_COLUMN_MANU));
	action = menuVersionHeader->addAction(tr("Name"), this, SLOT(actionVersionHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_NAME);
	action->setChecked(!treeWidgetVersionView->isColumnHidden(QMC2_MACHINELIST_COLUMN_NAME));
	action = menuVersionHeader->addAction(tr("ROM types"), this, SLOT(actionVersionHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_RTYPES);
	action->setChecked(!treeWidgetVersionView->isColumnHidden(QMC2_MACHINELIST_COLUMN_RTYPES));
	action = menuVersionHeader->addAction(tr("Players"), this, SLOT(actionVersionHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_PLAYERS);
	action->setChecked(!treeWidgetVersionView->isColumnHidden(QMC2_MACHINELIST_COLUMN_PLAYERS));
	action = menuVersionHeader->addAction(tr("Driver status"), this, SLOT(actionVersionHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_DRVSTAT);
	action->setChecked(!treeWidgetVersionView->isColumnHidden(QMC2_MACHINELIST_COLUMN_DRVSTAT));
	action = menuVersionHeader->addAction(tr("Source file"), this, SLOT(actionVersionHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_SRCFILE);
	action->setChecked(!treeWidgetVersionView->isColumnHidden(QMC2_MACHINELIST_COLUMN_SRCFILE));
	action = menuVersionHeader->addAction(tr("Rank"), this, SLOT(actionVersionHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_RANK);
	action->setChecked(!treeWidgetVersionView->isColumnHidden(QMC2_MACHINELIST_COLUMN_RANK));
	action = menuVersionHeader->addAction(tr("Category"), this, SLOT(actionVersionHeader_triggered())); action->setCheckable(true); action->setData(QMC2_MACHINELIST_COLUMN_CATEGORY);
	action->setChecked(!treeWidgetVersionView->isColumnHidden(QMC2_MACHINELIST_COLUMN_CATEGORY));
	actionMenuVersionHeaderCategory = action;
	menuVersionHeader->addSeparator();
	action = menuVersionHeader->addAction(QIcon(":data/img/reset.png"), tr("Reset"), this, SLOT(actionVersionHeader_triggered())); action->setData(QMC2_MACHINELIST_RESET);
	header->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(header, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(treeWidgetVersionViewHeader_customContextMenuRequested(const QPoint &)));

	// other actions
	comboBoxSearch->setLineEdit(new IconLineEdit(QIcon(QString::fromUtf8(":/data/img/find.png")), QMC2_ALIGN_LEFT, comboBoxSearch));
	connect(actionViewFullDetail, SIGNAL(triggered()), this, SLOT(viewFullDetail()));
	connect(actionViewParentClones, SIGNAL(triggered()), this, SLOT(viewParentClones()));
	connect(actionViewByCategory, SIGNAL(triggered()), this, SLOT(viewByCategory()));
	connect(actionViewByVersion, SIGNAL(triggered()), this, SLOT(viewByVersion()));
	connect(&searchTimer, SIGNAL(timeout()), this, SLOT(comboBoxSearch_editTextChanged_delayed()));
	connect(&updateTimer, SIGNAL(timeout()), this, SLOT(treeWidgetMachineList_itemSelectionChanged_delayed()));
	connect(&activityCheckTimer, SIGNAL(timeout()), this, SLOT(checkActivity()));
	activityState = false;
	comboBoxSearch->lineEdit()->setPlaceholderText(tr("Enter search string"));

	// search options menus
	menuSearchOptions = new QMenu(this);
	s = tr("Negate search");
	actionNegateSearch = menuSearchOptions->addAction(s);
	actionNegateSearch->setToolTip(s); actionNegateSearch->setStatusTip(s);
	actionNegateSearch->setIcon(QIcon(QString::fromUtf8(":/data/img/find_negate.png")));
	actionNegateSearch->setCheckable(true);
	bool negated = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/NegateSearch", false).toBool();
	actionNegateSearch->setChecked(negated);
	negateSearchTriggered(negated);
	menuSearchOptions->addSeparator();
	s = tr("Include BIOS sets");
	actionSearchIncludeBiosSets = menuSearchOptions->addAction(s);
	actionSearchIncludeBiosSets->setToolTip(s); actionSearchIncludeBiosSets->setStatusTip(s);
	actionSearchIncludeBiosSets->setCheckable(true);
	bool includeBiosSets = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/SearchIncludeBiosSets", true).toBool();
	actionSearchIncludeBiosSets->setChecked(includeBiosSets);
	connect(actionSearchIncludeBiosSets, SIGNAL(triggered(bool)), this, SLOT(searchIncludeBiosSetsTriggered(bool)));
	s = tr("Include device sets");
	actionSearchIncludeDeviceSets = menuSearchOptions->addAction(s);
	actionSearchIncludeDeviceSets->setToolTip(s); actionSearchIncludeDeviceSets->setStatusTip(s);
	actionSearchIncludeDeviceSets->setCheckable(true);
	bool includeDeviceSets = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/SearchIncludeDeviceSets", true).toBool();
	actionSearchIncludeDeviceSets->setChecked(includeDeviceSets);
	connect(actionSearchIncludeDeviceSets, SIGNAL(triggered(bool)), this, SLOT(searchIncludeDeviceSetsTriggered(bool)));
	connect(actionNegateSearch, SIGNAL(triggered(bool)), this, SLOT(negateSearchTriggered(bool)));
	IconLineEdit *ileSearch = ((IconLineEdit *)comboBoxSearch->lineEdit());
	connect(ileSearch, SIGNAL(returnPressed()), this, SLOT(comboBoxSearch_editTextChanged_delayed()));
	IconLineEdit *ileToolbarSearch = ((IconLineEdit *)comboBoxToolbarSearch->lineEdit());
	connect(ileToolbarSearch, SIGNAL(returnPressed()), this, SLOT(comboBoxToolbarSearch_activated()));
	ileSearch->button()->setPopupMode(QToolButton::InstantPopup);
	ileToolbarSearch->button()->setPopupMode(QToolButton::InstantPopup);
	ileSearch->button()->setMenu(menuSearchOptions);
	ileToolbarSearch->button()->setMenu(menuSearchOptions);

	// rank locking
	RankItemWidget::ranksLocked = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/RanksLocked", false).toBool();
	if ( RankItemWidget::ranksLocked ) {
		actionLockRanks->setIcon(QIcon(QString::fromUtf8(":/data/img/unlock.png")));
		actionLockRanks->setText(tr("Unlock ranks"));
		actionLockRanks->setToolTip(tr("Unlock ranks"));
		actionLockRanks->setStatusTip(tr("Unlock ranks"));
	} else {
		actionLockRanks->setIcon(QIcon(QString::fromUtf8(":/data/img/lock.png")));
		actionLockRanks->setText(tr("Lock ranks"));
		actionLockRanks->setToolTip(tr("Lock ranks"));
		actionLockRanks->setStatusTip(tr("Lock ranks"));
	}
	menuRank_enableActions(!RankItemWidget::ranksLocked);

	// restore toolbar state
	restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/ToolbarState", QByteArray()).toByteArray());
#if defined(QMC2_OS_MAC)
	setUnifiedTitleAndToolBarOnMac(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/UnifiedTitleAndToolBarOnMac", false).toBool());
#endif

#if QMC2_JOYSTICK == 1
	joyIndex = -1;
#endif

	// download manager widget
	checkBoxRemoveFinishedDownloads->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Downloads/RemoveFinished", false).toBool());

	// setup ROM state filter selector menu & toggle actions / short cuts
	qmc2StatesTogglesEnabled = false;
	menuRomStatusFilter = new QMenu(toolButtonSelectRomFilter);
	romStateFilter = new RomStateFilter(this);
	stateFilterAction = new QWidgetAction(menuRomStatusFilter);
	stateFilterAction->setDefaultWidget(romStateFilter);
	menuRomStatusFilter->addAction(stateFilterAction);
	toolButtonSelectRomFilter->setMenu(menuRomStatusFilter);

	// initialize ROM state toggles
	romStateFilter->toolButtonCorrect->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_C]);
	romStateFilter->toolButtonMostlyCorrect->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_M]);
	romStateFilter->toolButtonIncorrect->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_I]);
	romStateFilter->toolButtonNotFound->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_N]);
	romStateFilter->toolButtonUnknown->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_U]);

	// connect header click signals
	connect(treeWidgetMachineList->header(), SIGNAL(sectionClicked(int)), this, SLOT(treeWidgetMachineList_headerSectionClicked(int)));
	connect(treeWidgetHierarchy->header(), SIGNAL(sectionClicked(int)), this, SLOT(treeWidgetHierarchy_headerSectionClicked(int)));
#if QT_VERSION < 0x050000
	treeWidgetMachineList->header()->setClickable(true);
	treeWidgetHierarchy->header()->setClickable(true);
#else
	treeWidgetMachineList->header()->setSectionsClickable(true);
	treeWidgetHierarchy->header()->setSectionsClickable(true);
#endif
	connect(treeWidgetCategoryView->header(), SIGNAL(sectionClicked(int)), this, SLOT(treeWidgetCategoryView_headerSectionClicked(int)));
#if QT_VERSION < 0x050000
	treeWidgetCategoryView->header()->setClickable(true);
#else
	treeWidgetCategoryView->header()->setSectionsClickable(true);
#endif
	connect(treeWidgetVersionView->header(), SIGNAL(sectionClicked(int)), this, SLOT(treeWidgetVersionView_headerSectionClicked(int)));
#if QT_VERSION < 0x050000
	treeWidgetVersionView->header()->setClickable(true);
#else
	treeWidgetVersionView->header()->setSectionsClickable(true);
#endif

	// connections for dock/undock buttons
	connect(floatToggleButtonSoftwareDetail, SIGNAL(toggled(bool)), this, SLOT(floatToggleButtonSoftwareDetail_toggled(bool)));

	// setup the global network access manager
	qmc2NetworkAccessManager = new NetworkAccessManager(0, this);
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "WebBrowser/RestoreCookies", true).toBool() )
		qmc2NetworkAccessManager->setCookieJar(new CookieJar(qmc2NetworkAccessManager));

	// URL replacement regexp
	QString urlChar = QLatin1String("\\+\\-\\w\\./#@&;:=\\?~%_,\\!\\$\\*");
	urlSectionRegExp = QString("[%1]+").arg(urlChar);

	connect(treeWidgetMachineList->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(treeWidgetMachineList_verticalScrollChanged(int)));
	m_mlRankUpdateTimer.setSingleShot(true);
	connect(&m_mlRankUpdateTimer, SIGNAL(timeout()), this, SLOT(treeWidgetMachineList_updateRanks()));
	connect(treeWidgetHierarchy->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(treeWidgetHierarchy_verticalScrollChanged(int)));
	m_hlRankUpdateTimer.setSingleShot(true);
	connect(&m_hlRankUpdateTimer, SIGNAL(timeout()), this, SLOT(treeWidgetHierarchy_updateRanks()));
	connect(treeWidgetCategoryView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(treeWidgetCategoryView_verticalScrollChanged(int)));
	m_clRankUpdateTimer.setSingleShot(true);
	connect(&m_clRankUpdateTimer, SIGNAL(timeout()), this, SLOT(treeWidgetCategoryView_updateRanks()));
	connect(treeWidgetVersionView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(treeWidgetVersionView_verticalScrollChanged(int)));
	m_vlRankUpdateTimer.setSingleShot(true);
	connect(&m_vlRankUpdateTimer, SIGNAL(timeout()), this, SLOT(treeWidgetVersionView_updateRanks()));

	// rank related
	RankItemWidget::rankSingle = QImage(QString::fromUtf8(":/data/img/rank.png"));
	RankItemWidget::rankSingleFlat = QImage(QString::fromUtf8(":/data/img/rank_flat.png"));
	RankItemWidget::rankBackround = QImage(QString::fromUtf8(":/data/img/rank_bg.png"));
	RankItemWidget::rankGradient = QLinearGradient(0, 0, RankItemWidget::rankBackround.width() - 1, 0);
	RankItemWidget::rankGradient.setColorAt(0, QColor(255, 255, 255, 100));
	RankItemWidget::rankGradient.setColorAt(1, Qt::transparent);
	RankItemWidget::useColorRankImage = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/ColorRankImage", false).toBool();
	RankItemWidget::useFlatRankImage = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/FlatRankImage", false).toBool();
	RankItemWidget::rankImageColor = QColor(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/RankImageColor", "#646464").toString());
	if ( RankItemWidget::useColorRankImage )
		on_actionRankImagePlain_triggered(true);
	else if ( RankItemWidget::useFlatRankImage )
		on_actionRankImageFlat_triggered(true);
	else
		on_actionRankImageGradient_triggered(true);
	connect(menuRank, SIGNAL(aboutToShow()), this, SLOT(menuRank_aboutToShow()));

	// prepare to receive MAME output notifications from the process manager
	connect(qmc2ProcessManager, SIGNAL(mameOutputNotifier(int, const QString &, const QString &)), this, SLOT(processOutputNotifier(int, const QString &, const QString &)));

	QTimer::singleShot(0, this, SLOT(init()));
}

MainWindow::~MainWindow()
{
	if ( qmc2StartupDefaultFont )
		delete qmc2StartupDefaultFont;
	delete loadAnimMovie;
	delete nullMovie;
}

void MainWindow::log(int logTarget, const QString &msg)
{
	if ( !qmc2GuiReady )
		return;
	QString message(msg);
	QString timeString(QTime::currentTime().toString("hh:mm:ss.zzz") + ": ");
	switch ( logTarget ) {
		case QMC2_LOG_FRONTEND:
			if ( !qmc2LogFrontendMutex.tryLock(QMC2_LOG_MUTEX_LOCK_TIMEOUT) )
				return;
			if ( message.compare(qmc2LastFrontendLogMessage) == 0 ) {
				qmc2FrontendLogMessageRepeatCount++;
				qmc2LogFrontendMutex.unlock();
				return;
			} else {
				qmc2LastFrontendLogMessage = message;
				if ( qmc2FrontendLogMessageRepeatCount > 0 )
					message = tr("last message repeated %n time(s)", "", qmc2FrontendLogMessageRepeatCount) + '\n' + timeString + qmc2LastFrontendLogMessage;
				qmc2FrontendLogMessageRepeatCount = 0;
			}
			break;
		case QMC2_LOG_EMULATOR:
			if( !qmc2LogEmulatorMutex.tryLock(QMC2_LOG_MUTEX_LOCK_TIMEOUT) )
				return;
			if ( message.compare(qmc2LastEmulatorLogMessage) == 0 ) {
				qmc2EmulatorLogMessageRepeatCount++;
				qmc2LogEmulatorMutex.unlock();
				return;
			} else {
				qmc2LastEmulatorLogMessage = message;
				if ( qmc2EmulatorLogMessageRepeatCount > 0 )
					message = tr("last message repeated %n time(s)", "", qmc2EmulatorLogMessageRepeatCount) + '\n' + timeString + qmc2LastEmulatorLogMessage;
				qmc2EmulatorLogMessageRepeatCount = 0;
			}
			break;
		default:
			return;
	}
	message.prepend(timeString);
	switch ( logTarget ) {
		case QMC2_LOG_FRONTEND:
			textBrowserFrontendLog->appendPlainText(message);
			if ( !qmc2FrontendLogFile ) {
#if defined(QMC2_SDLMAME)
				if ( (qmc2FrontendLogFile = new QFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/LogFile", Options::configPath() + "/qmc2-sdlmame.log").toString(), this)) == 0 ) {
					qmc2LogFrontendMutex.unlock();
					return;
				}
#elif defined(QMC2_MAME)
				if ( (qmc2FrontendLogFile = new QFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/LogFile", Options::configPath() + "/qmc2-mame.log").toString(), this)) == 0 ) {
					qmc2LogFrontendMutex.unlock();
					return;
				}
#endif
			}
			if ( !qmc2FrontendLogFile->isOpen() ) {
				if ( qmc2FrontendLogFile->open(QIODevice::WriteOnly | QIODevice::Text) )
					qmc2FrontendLogStream.setDevice(qmc2FrontendLogFile);
				else {
					qmc2LogFrontendMutex.unlock();
					return;
				}
			}
			qmc2FrontendLogStream << message << '\n';
			qmc2FrontendLogStream.flush();
			qmc2LogFrontendMutex.unlock();
			break;
		case QMC2_LOG_EMULATOR:
			textBrowserEmulatorLog->appendPlainText(message);
			if ( !qmc2EmulatorLogFile ) {
				if ( (qmc2EmulatorLogFile = new QFile(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/LogFile", Options::configPath() + "/mame.log").toString(), this)) == 0 ) {
					qmc2LogEmulatorMutex.unlock();
					return;
				}
			}
			if ( !qmc2EmulatorLogFile->isOpen() ) {
				if ( qmc2EmulatorLogFile->open(QIODevice::WriteOnly | QIODevice::Text) )
					qmc2EmulatorLogStream.setDevice(qmc2EmulatorLogFile);
				else {
					qmc2LogEmulatorMutex.unlock();
					return;
				}
			}
			qmc2EmulatorLogStream << message << '\n';
			qmc2EmulatorLogStream.flush();
			qmc2LogEmulatorMutex.unlock();
			break;
		default:
			break;
	}
}

void MainWindow::logScrollToEnd(int logTarget)
{
	switch ( logTarget ) {
		case QMC2_LOG_FRONTEND:
			textBrowserFrontendLog->horizontalScrollBar()->setValue(textBrowserFrontendLog->horizontalScrollBar()->minimum());
		        textBrowserFrontendLog->verticalScrollBar()->setValue(textBrowserFrontendLog->verticalScrollBar()->maximum());
			break;
		case QMC2_LOG_EMULATOR:
			textBrowserEmulatorLog->horizontalScrollBar()->setValue(textBrowserEmulatorLog->horizontalScrollBar()->minimum());
		        textBrowserEmulatorLog->verticalScrollBar()->setValue(textBrowserEmulatorLog->verticalScrollBar()->maximum());
			break;
		default:
			break;
	}
}

void MainWindow::tabWidgetMachineList_tabMoved(int from, int to)
{
	qmc2ComponentSetup->comboBoxComponents->setCurrentIndex(0);
	ComponentInfo *componentInfo = qmc2ComponentSetup->componentInfoHash().value("Component1");

	if ( !componentInfo )
		return;

	int adjustedFrom = from;
	int adjustedTo = to;
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
	int embedIndex = componentInfo->appliedFeatureList().indexOf(QMC2_EMBED_INDEX);
	if ( embedIndex >= 0 ) {
		if ( tabWidgetMachineList->indexOf(tabEmbeddedEmus) < 0 ) {
			if ( embedIndex <= adjustedFrom )
				adjustedFrom++;
			if ( embedIndex <= adjustedTo )
				adjustedTo++;
		}
	}
#endif
	int foreignIndex = componentInfo->appliedFeatureList().indexOf(QMC2_FOREIGN_INDEX);
	if ( foreignIndex >= 0 ) {
		if ( tabWidgetMachineList->indexOf(tabForeignEmulators) < 0 ) {
			if ( foreignIndex <= adjustedFrom )
				adjustedFrom++;
			if ( foreignIndex <= adjustedTo )
				adjustedTo++;
		}
	}

	int fromFeature = componentInfo->appliedFeatureList().at(adjustedFrom);
	componentInfo->appliedFeatureList().removeAt(adjustedFrom);
	componentInfo->appliedFeatureList().insert(adjustedTo, fromFeature);
	fromFeature = componentInfo->activeFeatureList().at(adjustedFrom);
	componentInfo->activeFeatureList().removeAt(adjustedFrom);
	componentInfo->activeFeatureList().insert(adjustedTo, fromFeature);

	QListWidgetItem *takenItem = qmc2ComponentSetup->listWidgetActiveFeatures->takeItem(adjustedFrom);
	if ( takenItem )
		qmc2ComponentSetup->listWidgetActiveFeatures->insertItem(adjustedTo, takenItem);

	QStringList activeIndexList;
	for (int i = 0; i < componentInfo->appliedFeatureList().count(); i++)
		activeIndexList << QString::number(componentInfo->appliedFeatureList().at(i));

	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/Component1/ActiveFeatures", activeIndexList);
}

void MainWindow::tabWidgetMachineDetail_tabMoved(int from, int to)
{
	qmc2ComponentSetup->comboBoxComponents->setCurrentIndex(1);
	ComponentInfo *componentInfo = qmc2ComponentSetup->componentInfoHash().value("Component2");

	if ( !componentInfo )
		return;

	int fromFeature = componentInfo->appliedFeatureList().at(from);
	componentInfo->appliedFeatureList().removeAt(from);
	componentInfo->appliedFeatureList().insert(to, fromFeature);
	fromFeature = componentInfo->activeFeatureList().at(from);
	componentInfo->activeFeatureList().removeAt(from);
	componentInfo->activeFeatureList().insert(to, fromFeature);

	QListWidgetItem *takenItem = qmc2ComponentSetup->listWidgetActiveFeatures->takeItem(from);
	if ( takenItem )
		qmc2ComponentSetup->listWidgetActiveFeatures->insertItem(to, takenItem);

	QStringList activeIndexList;
	for (int i = 0; i < componentInfo->appliedFeatureList().count(); i++)
		activeIndexList << QString::number(componentInfo->appliedFeatureList().at(i));

	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/Component2/ActiveFeatures", activeIndexList);
}

void MainWindow::tabWidgetLogsAndEmulators_tabMoved(int from, int to)
{
	qmc2ComponentSetup->comboBoxComponents->setCurrentIndex(2);
	ComponentInfo *componentInfo = qmc2ComponentSetup->componentInfoHash().value("Component3");

	if ( !componentInfo )
		return;

	int fromFeature = componentInfo->appliedFeatureList().at(from);
	componentInfo->appliedFeatureList().removeAt(from);
	componentInfo->appliedFeatureList().insert(to, fromFeature);
	fromFeature = componentInfo->activeFeatureList().at(from);
	componentInfo->activeFeatureList().removeAt(from);
	componentInfo->activeFeatureList().insert(to, fromFeature);

	QListWidgetItem *takenItem = qmc2ComponentSetup->listWidgetActiveFeatures->takeItem(from);
	if ( takenItem )
		qmc2ComponentSetup->listWidgetActiveFeatures->insertItem(to, takenItem);

	QStringList activeIndexList;
	for (int i = 0; i < componentInfo->appliedFeatureList().count(); i++)
		activeIndexList << QString::number(componentInfo->appliedFeatureList().at(i));

	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/Component3/ActiveFeatures", activeIndexList);
}

void MainWindow::tabWidgetSoftwareDetail_tabMoved(int from, int to)
{
	qmc2ComponentSetup->comboBoxComponents->setCurrentIndex(3);
	ComponentInfo *componentInfo = qmc2ComponentSetup->componentInfoHash().value("Component4");

	if ( !componentInfo )
		return;

	int fromFeature = componentInfo->appliedFeatureList().at(from);
	componentInfo->appliedFeatureList().removeAt(from);
	componentInfo->appliedFeatureList().insert(to, fromFeature);
	fromFeature = componentInfo->activeFeatureList().at(from);
	componentInfo->activeFeatureList().removeAt(from);
	componentInfo->activeFeatureList().insert(to, fromFeature);

	QListWidgetItem *takenItem = qmc2ComponentSetup->listWidgetActiveFeatures->takeItem(from);
	if ( takenItem )
		qmc2ComponentSetup->listWidgetActiveFeatures->insertItem(to, takenItem);

	QStringList activeIndexList;
	for (int i = 0; i < componentInfo->appliedFeatureList().count(); i++)
		activeIndexList << QString::number(componentInfo->appliedFeatureList().at(i));

	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/Component4/ActiveFeatures", activeIndexList);
}

void MainWindow::on_actionPlayEmbedded_triggered(bool)
{
	qmc2StartEmbedded = true;
	on_actionPlay_triggered();
}

void MainWindow::on_actionPlay_triggered(bool)
{
	if ( qmc2EarlyReloadActive )
		return;

	if ( !qmc2CurrentItem && qmc2DemoMachine.isEmpty() )
		return;

	if ( qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_MACHINE) == MachineList::trWaitingForData )
		return;

	if ( qmc2CriticalSection ) {
		QTimer::singleShot(10, this, SLOT(on_actionPlay_triggered()));
		return;
	}

	QString machineName;

	if ( !qmc2DemoMachine.isEmpty() )
		machineName = qmc2DemoMachine;
	else if ( !qmc2CurrentItem )
		return;
	else
		machineName = qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_NAME);

	ComponentInfo *componentInfo = qmc2ComponentSetup->componentInfoHash().value("Component2");
	if ( !componentInfo )
		return;

	if ( qmc2DemoMachine.isEmpty() ) {
		qmc2LastConfigItem = 0;
		on_tabWidgetMachineDetail_currentChanged(componentInfo->appliedFeatureList().indexOf(QMC2_CONFIG_INDEX));
	}

	// check if a foreign emulator is to be used and if it CAN be used; if yes (both), start it instead of the default emulator...
	qmc2Config->beginGroup(QMC2_EMULATOR_PREFIX + "RegisteredEmulators");
	QStringList registeredEmulators = qmc2Config->childGroups();
	qmc2Config->endGroup();
	bool foreignEmulator = false;
	if ( registeredEmulators.count() > 0 && qmc2DemoMachine.isEmpty() ) {
		if ( !launchForeignID ) {
			if ( tabWidgetMachineList->currentIndex() == tabWidgetMachineList->indexOf(tabForeignEmulators) ) {
				QTreeWidgetItem *item = treeWidgetForeignIDs->currentItem();
				if ( item && item->isSelected() ) {
					QStringList foreignInfo = item->whatsThis(0).split("\t");
					if ( foreignInfo.count() > 2 ) {
						// 0:emuName -- 1:id -- 2:description 
						foreignEmuName = foreignInfo[0];
						foreignID = foreignInfo[1];
						foreignDescription = foreignInfo[2];
						launchForeignID = true;
					}
				}
			}
		}
		if ( launchForeignID ) {
			foreignEmulator = true;
			QString emuCommand = qmc2Config->value(QString(QMC2_EMULATOR_PREFIX + "RegisteredEmulators/%1/Executable").arg(foreignEmuName), QString()).toString();
			QString emuWorkDir = qmc2Config->value(QString(QMC2_EMULATOR_PREFIX + "RegisteredEmulators/%1/WorkingDirectory").arg(foreignEmuName), QString()).toString();
			QString argString = qmc2Config->value(QString(QMC2_EMULATOR_PREFIX + "RegisteredEmulators/%1/Arguments").arg(foreignEmuName), QString()).toString();
			QStringList emuArgs;
			if ( foreignID == tr("N/A") )
				argString.replace("$ID$", "").replace("$DESCRIPTION$", "");
			else
				argString.replace("$ID$", foreignID).replace("$DESCRIPTION$", foreignDescription);
			QRegExp rx("([^\\s]+|[^\\s]*\"[^\"]+\"[^\\s]*)");
			int i = 0;
			while ( (i = rx.indexIn(argString, i)) != -1 ) {
				emuArgs << rx.cap(1).trimmed().remove("\"");
				i += rx.matchedLength();
			}
			qmc2ProcessManager->process(qmc2ProcessManager->start(emuCommand, emuArgs, true, emuWorkDir));
		} else if ( qmc2Config->contains(qmc2EmulatorOptions->settingsGroup + "/SelectedEmulator") ) {
			QString selectedEmulator = qmc2Config->value(qmc2EmulatorOptions->settingsGroup + "/SelectedEmulator").toString();
			if ( !selectedEmulator.isEmpty() && registeredEmulators.contains(selectedEmulator) ) {
				foreignEmulator = true;
				QString emuCommand = qmc2Config->value(QString(QMC2_EMULATOR_PREFIX + "RegisteredEmulators/%1/Executable").arg(selectedEmulator), QString()).toString();
				QString emuWorkDir = qmc2Config->value(QString(QMC2_EMULATOR_PREFIX + "RegisteredEmulators/%1/WorkingDirectory").arg(selectedEmulator), QString()).toString();
				QString argString = qmc2Config->value(QString(QMC2_EMULATOR_PREFIX + "RegisteredEmulators/%1/Arguments").arg(selectedEmulator), QString()).toString();
				QStringList emuArgs;
				argString.replace("$ID$", machineName).replace("$DESCRIPTION$", qmc2MachineListItemHash.value(machineName)->text(QMC2_MACHINELIST_COLUMN_MACHINE));
				QRegExp rx("([^\\s]+|[^\\s]*\"[^\"]+\"[^\\s]*)");
				int i = 0;
				while ( (i = rx.indexIn(argString, i)) != -1 ) {
					emuArgs << rx.cap(1).trimmed().remove("\"");
					i += rx.matchedLength();
				}
				qmc2ProcessManager->process(qmc2ProcessManager->start(emuCommand, emuArgs, true, emuWorkDir));
			}
		}
	}

	qmc2DriverName = machineName;

	if ( foreignEmulator ) {
		qmc2AutoMinimizedWidgets.clear();
		if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/MinimizeOnEmuLaunch", false).toBool() && !qmc2StartEmbedded ) {
			foreach (QWidget *w, qApp->topLevelWidgets()) {
				if ( w->isVisible() ) {
					qmc2AutoMinimizedWidgets[w] = w->windowState();
					w->showMinimized();
				}
			}
		}
		launchForeignID = false;
		return;
	}

	if ( qmc2MachineList->isDevice(machineName) ) {
		log(QMC2_LOG_FRONTEND, tr("sorry, devices cannot run standalone"));
		return;
	}

	launchForeignID = false;
	QStringList args;
	QString sectionTitle;
	EmulatorOptions *emuOptions = qmc2EmulatorOptions;

	EmulatorOptions *demoOpts = 0;
	if ( !qmc2DemoMachine.isEmpty() ) {
		demoOpts = new EmulatorOptions("MAME/Configuration/" + machineName, 0);
		demoOpts->load();
		emuOptions = demoOpts;
	}

	foreach (sectionTitle, emuOptions->optionsMap.uniqueKeys()) {
		int i;
		for (i = 0; i < emuOptions->optionsMap[sectionTitle].count(); i++) {
			EmulatorOption option = emuOptions->optionsMap[sectionTitle][i];
			QString globalOptionKey = QMC2_EMULATOR_PREFIX + "Configuration/Global/" + option.name;
			switch ( option.type ) {
				case QMC2_EMUOPT_TYPE_INT: {
					int  v = option.value.toInt();
					int dv = option.dvalue.toInt();
					int gv = qmc2Config->value(globalOptionKey, dv).toInt();
					if ( !option.valid )
						v = gv;
					if ( v != dv )
						args << QString("-%1").arg(option.name) << QString("%1").arg(v);
					break;
				}

				case QMC2_EMUOPT_TYPE_FLOAT: {
					double  v = option.value.toDouble();
					double dv = option.dvalue.toDouble();
					double gv = qmc2Config->value(globalOptionKey, dv).toDouble();
					if ( !option.valid )
						v = gv;
					if ( v != dv ) {
						QString val;
						val.setNum(v, 'f', option.decimals);
						args << QString("-%1").arg(option.name) << val;
					}
					break;
				}

				case QMC2_EMUOPT_TYPE_FLOAT2: {
					QStringList subValues = option.value.split(",");
					QStringList defaultSubValues = option.dvalue.split(",");
					double v1, v2, dv1, dv2;
					v1 = v2 = dv1 = dv2 = 0;
					if ( subValues.count() > 0 )
						v1 = subValues[0].toDouble();
					if ( subValues.count() > 1 )
						v2 = subValues[1].toDouble();
					if ( defaultSubValues.count() > 0 )
						dv1 = defaultSubValues[0].toDouble();
					if ( defaultSubValues.count() > 1 )
						dv2 = defaultSubValues[1].toDouble();
					if ( v1 != dv1 || v2 != dv2 )
						args << QString("-%1").arg(option.name) << QString("%1,%2").arg(v1).arg(v2);
					break;
				}

				case QMC2_EMUOPT_TYPE_FLOAT3: {
					QStringList subValues = option.value.split(",");
					QStringList defaultSubValues = option.dvalue.split(",");
					double v1, v2, v3, dv1, dv2, dv3;
					v1 = v2 = v3 = dv1 = dv2 = dv3 = 0;
					if ( subValues.count() > 0 )
						v1 = subValues[0].toDouble();
					if ( subValues.count() > 1 )
						v2 = subValues[1].toDouble();
					if ( subValues.count() > 2 )
						v3 = subValues[2].toDouble();
					if ( defaultSubValues.count() > 0 )
						dv1 = defaultSubValues[0].toDouble();
					if ( defaultSubValues.count() > 1 )
						dv2 = defaultSubValues[1].toDouble();
					if ( defaultSubValues.count() > 2 )
						dv3 = defaultSubValues[2].toDouble();
					if ( v1 != dv1 || v2 != dv2 || v3 != dv3 )
						args << QString("-%1").arg(option.name) << QString("%1,%2,%3").arg(v1).arg(v2).arg(v3);
					break;
				}

				case QMC2_EMUOPT_TYPE_BOOL: {
					bool dv = EmulatorOptionDelegate::stringToBool(option.dvalue);
					bool v = EmulatorOptionDelegate::stringToBool(option.value);
					bool gv = qmc2Config->value(globalOptionKey, dv).toBool();
					if ( !option.valid )
						v = gv;
					if ( v != dv ) {
						if ( v )
							args << QString("-%1").arg(option.name);
						else
							args << QString("-no%1").arg(option.name);
					}
					break;
				}

				case QMC2_EMUOPT_TYPE_FILE:
				case QMC2_EMUOPT_TYPE_DIRECTORY: {
					QString  v = option.value;
					QString dv = option.dvalue;
					QString gv = qmc2Config->value(globalOptionKey, dv).toString();
					if ( !option.valid )
						v = gv;
#if defined(QMC2_OS_WIN)
					if ( v != dv )
						args << QString("-%1").arg(option.name) << QDir::toNativeSeparators(QDir::cleanPath(v));
#else
					if ( v != dv )
						args << QString("-%1").arg(option.name) << QDir::toNativeSeparators(v.replace("~", "$HOME"));
#endif
					break;
				}

				case QMC2_EMUOPT_TYPE_STRING:
				default: {
					QString  v = option.value;
					QString dv = option.dvalue;
					QString gv = qmc2Config->value(globalOptionKey, dv).toString();
					if ( !option.valid )
						v = gv;
					if ( v != dv )
						args << QString("-%1").arg(option.name) << v;
					break;
				}
			}
		}
	}

	QStringList extraOpts;

#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
	if ( qmc2StartEmbedded )
		extraOpts << "enable-window" << "disable-maximize" << "enable-keepaspect" << "enable-rotate" << "disable-ror" << "disable-rol";
#endif

	if ( qmc2DemoModeDialog && !qmc2DemoMachine.isEmpty() ) {
		args << "-str" << QString::number(qmc2DemoModeDialog->spinBoxSecondsToRun->value());
		extraOpts << qmc2DemoArgs;
	}

	foreach (QString extraOpt, extraOpts) {
		QStringList optParams = extraOpt.split("-", QString::SkipEmptyParts);
		QString negOpt = "-no" + optParams[1];
		QString posOpt = "-" + optParams[1];
		if ( optParams[0] == "enable" ) {
			if ( args.contains(negOpt) )
				args.removeAll(negOpt);
			if ( !args.contains(posOpt) )
				args << posOpt;
		} else {
			if ( args.contains(posOpt) )
				args.removeAll(posOpt);
			if ( !args.contains(negOpt) )
				args << negOpt;
		}
	}

	if ( qmc2Options->outputNotifiersEnabled() )
		args << "-output" << "console";

	args << machineName;

	QStringList softwareLists, softwareNames;

	if ( qmc2SoftwareList && tabWidgetMachineDetail->currentIndex() == componentInfo->appliedFeatureList().indexOf(QMC2_SOFTWARE_LIST_INDEX) ) {
		QStringList swlArgs = qmc2SoftwareList->arguments(&softwareLists, &softwareNames);
		if ( swlArgs.count() > 1 ) { 
			if ( swlArgs[0] == "-snapname" ) {
				args.removeLast();
				args << swlArgs[0] << swlArgs[1];
				args << machineName;
				for (int a = 2; a < swlArgs.count(); a++)
					args << swlArgs[a];
			} else
				args << swlArgs;
		} else
			args << swlArgs;
	} else if ( qmc2DeviceConfigurator && tabWidgetMachineDetail->currentIndex() == componentInfo->appliedFeatureList().indexOf(QMC2_DEVICE_INDEX) ) {
		switch ( qmc2DeviceConfigurator->tabWidgetDeviceSetup->currentIndex() ) {
			case QMC2_DEVSETUP_TAB_FILECHOOSER: {
				QString instance = qmc2DeviceConfigurator->comboBoxDeviceInstanceChooser->currentText();
				QItemSelectionModel *selectionModel = qmc2DeviceConfigurator->treeViewFileChooser->selectionModel();
				QModelIndexList indexList;
				if ( selectionModel )
					indexList = selectionModel->selectedIndexes();
				if ( indexList.count() > 0 && instance != tr("No devices available") ) {
					QList<QTreeWidgetItem *> allSlotItems;
					for (int i = 0; i < qmc2DeviceConfigurator->treeWidgetSlotOptions->topLevelItemCount(); i++) {
						QTreeWidgetItem *item = qmc2DeviceConfigurator->treeWidgetSlotOptions->topLevelItem(i);
						allSlotItems << item;
						qmc2DeviceConfigurator->insertChildItems(item, allSlotItems);
					}
					foreach (QTreeWidgetItem *item, allSlotItems) {
						QString slotName = item->text(QMC2_SLOTCONFIG_COLUMN_SLOT);
						if ( !slotName.isEmpty() ) {
							QComboBox *cb = (QComboBox *)qmc2DeviceConfigurator->treeWidgetSlotOptions->itemWidget(item, QMC2_SLOTCONFIG_COLUMN_OPTION);
							if ( cb ) {
								int defaultIndex = -1;
								QString biosChoice;
								QComboBox *cbBIOS = (QComboBox *)qmc2DeviceConfigurator->treeWidgetSlotOptions->itemWidget(item, QMC2_SLOTCONFIG_COLUMN_BIOS);
								if ( cbBIOS ) {
									QStringList biosInfoList = cbBIOS->currentText().split(" ", QString::SkipEmptyParts);
									if ( biosInfoList.count() == 3 && biosInfoList[2] == tr("default") )
										biosChoice = tr("N/A");
									else
										biosChoice = biosInfoList[0];
									if ( biosChoice != tr("N/A") )
										biosChoice = ",bios=" + biosChoice;
									else
										biosChoice.clear();
								}
								if ( qmc2DeviceConfigurator->slotPreselectionMap.contains(cb) )
									defaultIndex = qmc2DeviceConfigurator->slotPreselectionMap[cb];
								else if ( qmc2DeviceConfigurator->nestedSlotPreselectionMap.contains(cb) )
									defaultIndex = qmc2DeviceConfigurator->nestedSlotPreselectionMap[cb];
								QString slotDeviceString = QString("%1%2").arg(cb->currentText().split(" ", QString::SkipEmptyParts)[0]).arg(biosChoice);
								if ( cb->currentIndex() > 0 && defaultIndex == 0 )
									args << QString("-%1").arg(slotName) << slotDeviceString;
								else if ( cb->currentIndex() == 0 && defaultIndex > 0 )
									args << QString("-%1").arg(slotName) << "\"\"";
								else if ( cb->currentIndex() > 0 && defaultIndex > 0 && cb->currentIndex() != defaultIndex )
									args << QString("-%1").arg(slotName) << slotDeviceString;
								else if ( !biosChoice.isEmpty() )
									args << QString("-%1").arg(slotName) << slotDeviceString;
							}
						}
					}
					bool doMapping = true;
					if ( qmc2DeviceConfigurator->toolButtonChooserMergeMaps->isChecked() ) {
						QString configName = qmc2DeviceConfigurator->lineEditConfigurationName->text();
						QPair<QStringList, QStringList> valuePair = qmc2DeviceConfigurator->configurationMap[configName];
						for (int i = 0; i < valuePair.first.count(); i++) {
							if ( valuePair.first[i] == instance ) {
								QString file = qmc2DeviceConfigurator->fileModel->absolutePath(indexList[0]);
#if defined(QMC2_OS_WIN)
								args << QString("-%1").arg(instance) << file.replace('/', '\\');
#else
								args << QString("-%1").arg(instance) << file.replace("~", "$HOME");
#endif
								doMapping = false;
							} else {
#if defined(QMC2_OS_WIN)
								args << QString("-%1").arg(valuePair.first[i]) << valuePair.second[i].replace('/', '\\');
#else
								args << QString("-%1").arg(valuePair.first[i]) << valuePair.second[i].replace("~", "$HOME");
#endif
							}
						}
					}
					if ( doMapping ) {
						QString file = qmc2DeviceConfigurator->fileModel->absolutePath(indexList[0]);
#if defined(QMC2_OS_WIN)
						args << QString("-%1").arg(instance) << file.replace('/', '\\');
#else
						args << QString("-%1").arg(instance) << file.replace("~", "$HOME");
#endif
					}
				}
			}
			break;

		default: {
				QString configName = qmc2DeviceConfigurator->lineEditConfigurationName->text();
				if ( configName != tr("Default configuration") ) {
					if ( qmc2DeviceConfigurator->configurationMap.contains(configName) ) {
						QPair<QStringList, QStringList> valuePair = qmc2DeviceConfigurator->slotMap[configName];
						for (int i = 0; i < valuePair.first.count(); i++) {
							QString slotName = valuePair.first[i];
							QString slotOption = valuePair.second[i];
							QTreeWidgetItem *item;
							QComboBox *cb = qmc2DeviceConfigurator->comboBoxByName(slotName, &item);
							if ( cb ) {
								int defaultIndex = -1;
								QString biosChoice;
								if ( item ) {
									QComboBox *cbBIOS = (QComboBox *)qmc2DeviceConfigurator->treeWidgetSlotOptions->itemWidget(item, QMC2_SLOTCONFIG_COLUMN_BIOS);
									if ( cbBIOS ) {
										QStringList biosInfoList = cbBIOS->currentText().split(" ", QString::SkipEmptyParts);
										if ( biosInfoList.count() == 3 && biosInfoList[2] == tr("default") )
											biosChoice = tr("N/A");
										else
											biosChoice = biosInfoList[0];
										if ( biosChoice != tr("N/A") )
											biosChoice = ",bios=" + biosChoice;
										else
											biosChoice.clear();
									}
								}
								if ( qmc2DeviceConfigurator->slotPreselectionMap.contains(cb) )
									defaultIndex = qmc2DeviceConfigurator->slotPreselectionMap[cb];
								else if ( qmc2DeviceConfigurator->nestedSlotPreselectionMap.contains(cb) )
									defaultIndex = qmc2DeviceConfigurator->nestedSlotPreselectionMap[cb];
								QString slotDeviceString = QString("%1%2").arg(cb->currentText().split(" ", QString::SkipEmptyParts)[0]).arg(biosChoice);
								if ( cb->currentIndex() > 0 && defaultIndex == 0 )
									args << QString("-%1").arg(slotName) << slotDeviceString;
								else if ( cb->currentIndex() == 0 && defaultIndex > 0 )
									args << QString("-%1").arg(slotName) << "\"\"";
								else if ( cb->currentIndex() > 0 && defaultIndex > 0 && cb->currentIndex() != defaultIndex )
									args << QString("-%1").arg(slotName) << slotDeviceString;
								else if ( !biosChoice.isEmpty() )
									args << QString("-%1").arg(slotName) << slotDeviceString;
							}
						}
						valuePair = qmc2DeviceConfigurator->configurationMap[configName];
						for (int i = 0; i < valuePair.first.count(); i++)
#if defined(QMC2_OS_WIN)
							args << QString("-%1").arg(valuePair.first[i]) << valuePair.second[i].replace('/', '\\');
#else
							args << QString("-%1").arg(valuePair.first[i]) << valuePair.second[i].replace("~", "$HOME");
#endif
					}
				} else {
					QList<QTreeWidgetItem *> allSlotItems;
					for (int i = 0; i < qmc2DeviceConfigurator->treeWidgetSlotOptions->topLevelItemCount(); i++) {
						QTreeWidgetItem *item = qmc2DeviceConfigurator->treeWidgetSlotOptions->topLevelItem(i);
						allSlotItems << item;
						qmc2DeviceConfigurator->insertChildItems(item, allSlotItems);
					}
					foreach (QTreeWidgetItem *item, allSlotItems) {
						QString slotName = item->text(QMC2_SLOTCONFIG_COLUMN_SLOT);
						if ( !slotName.isEmpty() ) {
							QComboBox *cb = (QComboBox *)qmc2DeviceConfigurator->treeWidgetSlotOptions->itemWidget(item, QMC2_SLOTCONFIG_COLUMN_OPTION);
							if ( cb ) {
								int defaultIndex = -1;
								QString biosChoice;
								QComboBox *cbBIOS = (QComboBox *)qmc2DeviceConfigurator->treeWidgetSlotOptions->itemWidget(item, QMC2_SLOTCONFIG_COLUMN_BIOS);
								if ( cbBIOS ) {
									QStringList biosInfoList = cbBIOS->currentText().split(" ", QString::SkipEmptyParts);
									if ( biosInfoList.count() == 3 && biosInfoList[2] == tr("default") )
										biosChoice = tr("N/A");
									else
										biosChoice = biosInfoList[0];
									if ( biosChoice != tr("N/A") )
										biosChoice = ",bios=" + biosChoice;
									else
										biosChoice.clear();
								}
								if ( qmc2DeviceConfigurator->slotPreselectionMap.contains(cb) )
									defaultIndex = qmc2DeviceConfigurator->slotPreselectionMap[cb];
								else if ( qmc2DeviceConfigurator->nestedSlotPreselectionMap.contains(cb) )
									defaultIndex = qmc2DeviceConfigurator->nestedSlotPreselectionMap[cb];
								QString slotDeviceString = QString("%1%2").arg(cb->currentText().split(" ", QString::SkipEmptyParts)[0]).arg(biosChoice);
								if ( cb->currentIndex() > 0 && defaultIndex == 0 )
									args << QString("-%1").arg(slotName) << slotDeviceString;
								else if ( cb->currentIndex() == 0 && defaultIndex > 0 )
									args << QString("-%1").arg(slotName) << "\"\"";
								else if ( cb->currentIndex() > 0 && defaultIndex > 0 && cb->currentIndex() != defaultIndex )
									args << QString("-%1").arg(slotName) << slotDeviceString;
								else if ( !biosChoice.isEmpty() )
									args << QString("-%1").arg(slotName) << slotDeviceString;
							}
						}
					}
				}
			}
			break;
		}
	}

	QString command(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ExecutableFile", QString()).toString());
	QString workingDirectory(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory", QString()).toString());
	QStringList argsResolved(Settings::stResolve(args));

	// start machine
	qmc2ProcessManager->process(qmc2ProcessManager->start(command, argsResolved, true, workingDirectory, softwareLists, softwareNames));

	qmc2AutoMinimizedWidgets.clear();
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/MinimizeOnEmuLaunch", false).toBool() && !qmc2StartEmbedded ) {
		foreach (QWidget *w, qApp->topLevelWidgets()) {
			if ( w->isVisible() ) {
				qmc2AutoMinimizedWidgets[w] = w->windowState();
				w->showMinimized();
			}
		}
	}

	if ( qmc2DemoMachine.isEmpty() ) {
		// add machine to played list
		listWidgetPlayed->blockSignals(true);
		QTreeWidgetItem *mlItem = qmc2MachineListItemHash.value(machineName);
		QList<QListWidgetItem *> matches = listWidgetPlayed->findItems(mlItem->text(QMC2_MACHINELIST_COLUMN_MACHINE), Qt::MatchExactly);
		QListWidgetItem *playedItem;
		if ( matches.count() > 0 )
			playedItem = listWidgetPlayed->takeItem(listWidgetPlayed->row(matches[0]));
		else {
			playedItem = new QListWidgetItem();
			playedItem->setText(mlItem->text(QMC2_MACHINELIST_COLUMN_MACHINE));
			playedItem->setWhatsThis(mlItem->text(QMC2_MACHINELIST_COLUMN_NAME));
		}
		listWidgetPlayed->insertItem(0, playedItem);
		listWidgetPlayed->setCurrentItem(playedItem);
		listWidgetPlayed->blockSignals(false);
	} else {
		if ( qmc2DemoModeDialog ) {
			QProcess *proc = qmc2ProcessManager->process(qmc2ProcessManager->procCount - 1);
			if ( proc ) {
				connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), qmc2DemoModeDialog, SLOT(emuFinished(int, QProcess::ExitStatus)));
				connect(proc, SIGNAL(started()), qmc2DemoModeDialog, SLOT(emuStarted()));
			}
			if ( demoOpts )
				delete demoOpts;
		}
	}
}

void MainWindow::on_vSplitter_splitterMoved(int pos, int index)
{
	if ( qmc2SystemNotesEditor ) {
		qmc2SystemNotesEditor->move(0, 0);
		qmc2SystemNotesEditor->resize(qmc2SystemNotesEditor->parentWidget()->size());
	}

	QList<int> splitterSizes = vSplitter->sizes();
	switch ( stackedWidgetSpecial->currentIndex() ) {
		case QMC2_SPECIAL_SOFTWARE_PAGE:
			vSplitterSizesSoftwareDetail = splitterSizes;
			break;
		case QMC2_SPECIAL_DEFAULT_PAGE:
		default:
			vSplitterSizes = splitterSizes;
			break;
	}
}

void MainWindow::on_hSplitter_splitterMoved(int pos, int index)
{
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
	if ( tabWidgetMachineList->indexOf(tabEmbeddedEmus) != tabWidgetMachineList->currentIndex() || (tabWidgetMachineList->indexOf(tabEmbeddedEmus) == tabWidgetMachineList->currentIndex() && !toolButtonEmbedderMaximizeToggle->isChecked()) ) {
		hSplitterSizes = hSplitter->sizes();
	} else if ( tabWidgetMachineList->indexOf(tabEmbeddedEmus) == tabWidgetMachineList->currentIndex() && toolButtonEmbedderMaximizeToggle->isChecked() ) {
		toolButtonEmbedderMaximizeToggle->setChecked(false);
		menuBar()->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ShowMenuBar", true).toBool());
		statusBar()->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Statusbar", true).toBool());
		toolbar->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Toolbar", true).toBool());
	}
#endif

	// show / hide game status indicator
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/MachineStatusIndicator").toBool() ) {
		if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/MachineStatusIndicatorOnlyWhenRequired").toBool() ) {
			if ( pos == 0 ) {
				if ( !labelMachineStatus->isVisible() )
					labelMachineStatus->setVisible(true);
			} else {
				if ( labelMachineStatus->isVisible() )
					labelMachineStatus->setVisible(false);
			}
		} else {
			if ( !labelMachineStatus->isVisible() )
				labelMachineStatus->setVisible(true);
		}
	} else
		if ( labelMachineStatus->isVisible() )
			labelMachineStatus->setVisible(false);

	if ( qmc2SystemNotesEditor ) {
		qmc2SystemNotesEditor->move(0, 0);
		qmc2SystemNotesEditor->resize(qmc2SystemNotesEditor->parentWidget()->size());
	}
}

void MainWindow::on_actionToFavorites_triggered(bool)
{
	if ( !qmc2CurrentItem )
		return;

	QString itemText(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_MACHINE));
	QString itemName(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_NAME));

	if ( itemText == MachineList::trWaitingForData )
		return;

	QList<QListWidgetItem *> matches(listWidgetFavorites->findItems(itemText, Qt::MatchExactly));
	if ( matches.count() <= 0 ) {
		QListWidgetItem *item = new QListWidgetItem(listWidgetFavorites);
		item->setText(itemText);
		item->setWhatsThis(itemName);
		listWidgetFavorites->sortItems();
	}
}

void MainWindow::on_actionReload_triggered(bool)
{
	if ( !qmc2StartingUp ) {
		if ( qmc2FilterActive ) {
			log(QMC2_LOG_FRONTEND, tr("please wait for ROM state filter to finish and try again"));
			return;
		}
		if ( qmc2VerifyActive ) {
			log(QMC2_LOG_FRONTEND, tr("please wait for ROM verification to finish and try again"));
			return;
		}
		if ( qmc2ImageCheckActive ) {
			log(QMC2_LOG_FRONTEND, tr("please wait for image check to finish and try again"));
			return;
		}
		if ( qmc2SampleCheckActive ) {
			log(QMC2_LOG_FRONTEND, tr("please wait for sample check to finish and try again"));
			return;
		}
		if ( qmc2SystemROMAlyzer && qmc2SystemROMAlyzer->active() ) {
			log(QMC2_LOG_FRONTEND, tr("please wait for ROMAlyzer to finish the current analysis and try again"));
			return;
		}
		if ( qmc2SoftwareROMAlyzer && qmc2SoftwareROMAlyzer->active() ) {
			log(QMC2_LOG_FRONTEND, tr("please wait for ROMAlyzer to finish the current analysis and try again"));
			return;
		}
	} else if ( QMC2_CLI_OPT_CLEAR_ALL_CACHES ) {
		qmc2ForceCacheRefresh = true;
		qmc2MainWindow->on_actionClearAllEmulatorCaches_triggered(true);
		qmc2ForceCacheRefresh = false;
	}
	if ( qmc2ReloadActive )
		log(QMC2_LOG_FRONTEND, tr("machine list reload is already active"));
	else {
		qmc2LoadingInterrupted = false;
		qmc2MachineList->enableWidgets(false);
		if ( !qmc2LoadingInterrupted && (qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMameHistoryDat", false).toBool() || qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMessSysinfoDat", false).toBool()) )
			loadMachineInfoDB();
		if ( !qmc2LoadingInterrupted && (qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMameInfoDat", false).toBool() || qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMessInfoDat", false).toBool()) )
			loadEmuInfoDB();
		if ( !qmc2LoadingInterrupted && qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProcessSoftwareInfoDB", false).toBool() )
			loadSoftwareInfoDB();
		if ( !qmc2StartingUp && qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveMachineSelection", true).toBool() ) {
			if ( qmc2CurrentItem ) {
				log(QMC2_LOG_FRONTEND, tr("saving machine selection"));
				qmc2Config->setValue(QMC2_EMULATOR_PREFIX + "SelectedMachine", qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_NAME));
			} else
				qmc2Config->remove(QMC2_EMULATOR_PREFIX + "SelectedMachine");
		}
		if ( !qmc2LoadingInterrupted )
			qmc2MachineList->load();
		QTimer::singleShot(0, qmc2MachineList, SLOT(enableWidgets()));
	}
	static bool initialCall = true;
	if ( initialCall )
		QTimer::singleShot(0, this, SLOT(checkRomPath()));
	initialCall = false;
}

void MainWindow::on_actionExitStop_triggered(bool)
{
	close();
}

void MainWindow::on_actionCheckCurrentROM_triggered(bool)
{
	if ( qmc2FilterActive ) {
		log(QMC2_LOG_FRONTEND, tr("please wait for ROM state filter to finish and try again"));
	} else if ( qmc2VerifyActive ) {
		log(QMC2_LOG_FRONTEND, tr("ROM verification already active"));
	} else if ( qmc2ReloadActive ) {
		log(QMC2_LOG_FRONTEND, tr("please wait for reload to finish and try again"));
	} else if ( qmc2ImageCheckActive ) {
		log(QMC2_LOG_FRONTEND, tr("please wait for image check to finish and try again"));
	} else if ( qmc2SampleCheckActive ) {
		log(QMC2_LOG_FRONTEND, tr("please wait for sample check to finish and try again"));
	} else if ( qmc2SystemROMAlyzer && qmc2SystemROMAlyzer->active() ) {
		log(QMC2_LOG_FRONTEND, tr("please wait for ROMAlyzer to finish the current analysis and try again"));
	} else if ( qmc2SoftwareROMAlyzer && qmc2SoftwareROMAlyzer->active() ) {
		log(QMC2_LOG_FRONTEND, tr("please wait for ROMAlyzer to finish the current analysis and try again"));
	} else
		qmc2MachineList->verify(true);
}

void MainWindow::on_actionCheckROMs_triggered(bool)
{
	if ( qmc2FilterActive )
		log(QMC2_LOG_FRONTEND, tr("please wait for ROM state filter to finish and try again"));
	else if ( qmc2VerifyActive )
		log(QMC2_LOG_FRONTEND, tr("ROM verification already active"));
	else if ( qmc2ReloadActive )
		log(QMC2_LOG_FRONTEND, tr("please wait for reload to finish and try again"));
	else if ( qmc2ImageCheckActive )
		log(QMC2_LOG_FRONTEND, tr("please wait for image check to finish and try again"));
	else if ( qmc2SampleCheckActive )
		log(QMC2_LOG_FRONTEND, tr("please wait for sample check to finish and try again"));
	else if ( qmc2SystemROMAlyzer && qmc2SystemROMAlyzer->active() )
		log(QMC2_LOG_FRONTEND, tr("please wait for ROMAlyzer to finish the current analysis and try again"));
	else if ( qmc2SoftwareROMAlyzer && qmc2SoftwareROMAlyzer->active() )
		log(QMC2_LOG_FRONTEND, tr("please wait for ROMAlyzer to finish the current analysis and try again"));
	else {
		if ( !qmc2MachineList->autoRomCheck ) {
			switch ( QMessageBox::question(this,
						tr("Confirm"),
						tr("The ROM verification process may be very time-consuming.\nIt will overwrite existing cached data.\n\nDo you really want to check all ROM states now?"),
						QMessageBox::Yes | QMessageBox::No, QMessageBox::No) ) {
				case QMessageBox::Yes:
					qmc2MachineList->verify();
					break;

				default:
					break;
			}
		} else {
			if ( qmc2AutomaticReload )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("automatic ROM check triggered"));
			qmc2MachineList->verify();
		}
		qmc2MachineList->autoRomCheck = false;
		qmc2AutomaticReload = false;
	}
}

void MainWindow::on_actionExportROMStatus_triggered(bool)
{
	if ( !qmc2ROMStatusExporter )
		qmc2ROMStatusExporter = new ROMStatusExporter(this);

	qmc2ROMStatusExporter->adjustIconSizes();

	if ( qmc2ROMStatusExporter->isHidden() )
		qmc2ROMStatusExporter->show();
	else if ( qmc2ROMStatusExporter->isMinimized() )
		qmc2ROMStatusExporter->showNormal();

	QTimer::singleShot(0, qmc2ROMStatusExporter, SLOT(raise()));
}

void MainWindow::on_actionDemoMode_triggered(bool)
{
	if ( !qmc2DemoModeDialog )
		qmc2DemoModeDialog = new DemoModeDialog(this);

	qmc2DemoModeDialog->adjustIconSizes();

	if ( qmc2DemoModeDialog->isHidden() )
		qmc2DemoModeDialog->show();
	else if ( qmc2DemoModeDialog->isMinimized() )
		qmc2DemoModeDialog->showNormal();

	QTimer::singleShot(0, qmc2DemoModeDialog, SLOT(raise()));
}

void MainWindow::on_actionNewBrowserWindow_triggered(bool)
{
	viewHtml();
}

void MainWindow::viewHtml(QString filePath)
{
	MiniWebBrowser *webBrowser = new MiniWebBrowser(0);
	webBrowser->setAttribute(Qt::WA_DeleteOnClose);
	if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "WebBrowser/Geometry") )
		webBrowser->restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "WebBrowser/Geometry").toByteArray());
	else {
		webBrowser->adjustSize();
		webBrowser->move(QApplication::desktop()->screen()->rect().center() - webBrowser->rect().center());
	}
	connect(webBrowser->webViewBrowser->page(), SIGNAL(windowCloseRequested()), webBrowser, SLOT(close()));
	if ( !filePath.isEmpty() ) {
		QFileInfo fi(filePath);
		if ( fi.isReadable() ) {
#if defined(QMC2_OS_WIN)
			webBrowser->webViewBrowser->load("file:///" + fi.canonicalFilePath());
#else
			webBrowser->webViewBrowser->load("file://" + fi.canonicalFilePath());
#endif
		} else
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: can't load HTML file '%1'").arg(filePath));
	}
	webBrowser->show();
}

void MainWindow::on_actionNewPdfViewer_triggered(bool)
{
	viewPdf();
}

void MainWindow::on_actionNewFilteredView_triggered(bool)
{
	MachineListViewer *mlv = new MachineListViewer;
	machineListViewers.append(mlv);
	connect(mlv, SIGNAL(selectionChanged(const QString &)), this, SLOT(machineListViewer_selectionChanged(const QString &)));
	connect(mlv, SIGNAL(tagChanged(const QString &, bool)), this, SLOT(machineListViewer_tagChanged(const QString &, bool)));
	connect(this, SIGNAL(selectionChanged(const QString &)), mlv, SLOT(mainSelectionChanged(const QString &)));
	mlv->show();
}

void MainWindow::viewPdf(QString filePath)
{
	QString htmlPath = qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/DataDirectory").toString() + "/js/pdfjs/web/viewer.html";
	QFileInfo fi(htmlPath);
	if ( fi.isReadable() ) {
		MiniWebBrowser *webBrowser = new MiniWebBrowser(0, true);
		webBrowser->setAttribute(Qt::WA_DeleteOnClose);
		if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "PdfViewer/Geometry") )
			webBrowser->restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "PdfViewer/Geometry").toByteArray());
		else {
			webBrowser->adjustSize();
			webBrowser->move(QApplication::desktop()->screen()->rect().center() - webBrowser->rect().center());
		}
		connect(webBrowser->webViewBrowser->page(), SIGNAL(windowCloseRequested()), webBrowser, SLOT(close()));
		if ( !filePath.isEmpty() ) {
#if defined(QMC2_OS_WIN)
			webBrowser->webViewBrowser->load("file:///" + fi.canonicalFilePath() + QString("?file=file:///%1").arg(filePath));
#else
			webBrowser->webViewBrowser->load("file://" + fi.canonicalFilePath() + QString("?file=file://%1").arg(filePath));
#endif
		} else {
#if defined(QMC2_OS_WIN)
			webBrowser->webViewBrowser->load("file:///" + fi.canonicalFilePath() + QString("?file="));
#else
			webBrowser->webViewBrowser->load("file://" + fi.canonicalFilePath() + QString("?file="));
#endif
		}
		webBrowser->show();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: can't load PDF viewer from '%1'").arg(htmlPath));
}

void MainWindow::on_actionCheckSamples_triggered(bool)
{
	if ( !qmc2SampleChecker )
		qmc2SampleChecker = new SampleChecker(this);

	if ( qmc2SampleChecker->isHidden() )
		qmc2SampleChecker->show();
	else if ( qmc2SampleChecker->isMinimized() )
		qmc2SampleChecker->showNormal();

	QTimer::singleShot(0, qmc2SampleChecker, SLOT(raise()));
}

void MainWindow::on_actionCheckImagesAndIcons_triggered(bool)
{
	if ( !qmc2ImageChecker )
		qmc2ImageChecker = new ImageChecker(this);

	if ( qmc2ImageChecker->isHidden() )
		qmc2ImageChecker->show();
	else if ( qmc2ImageChecker->isMinimized() )
		qmc2ImageChecker->showNormal();

	QTimer::singleShot(0, qmc2ImageChecker, SLOT(raise()));
}

void MainWindow::on_actionSystemROMAlyzer_triggered(bool)
{
	if ( !qmc2SystemROMAlyzer )
		qmc2SystemROMAlyzer = new ROMAlyzer(0, QMC2_ROMALYZER_MODE_SYSTEM);

	if ( !qmc2SystemROMAlyzer->active() )
		qmc2SystemROMAlyzer->lineEditSets->setText("*");

	if ( qmc2SystemROMAlyzer->isHidden() )
		qmc2SystemROMAlyzer->show();
	else if ( qmc2SystemROMAlyzer->isMinimized() )
		qmc2SystemROMAlyzer->showNormal();

	QTimer::singleShot(0, qmc2SystemROMAlyzer, SLOT(raise()));
}

void MainWindow::on_actionSoftwareROMAlyzer_triggered(bool)
{
	if ( !swlDb ) {
		swlDb = new SoftwareListXmlDatabaseManager(qmc2MainWindow);
		swlDb->setSyncMode(QMC2_DB_SYNC_MODE_OFF);
		swlDb->setJournalMode(QMC2_DB_JOURNAL_MODE_MEMORY);
	}

	if ( !qmc2SoftwareROMAlyzer )
		qmc2SoftwareROMAlyzer = new ROMAlyzer(0, QMC2_ROMALYZER_MODE_SOFTWARE);

	if ( !qmc2SoftwareROMAlyzer->active() ) {
		qmc2SoftwareROMAlyzer->lineEditSoftwareLists->setText("*");
		qmc2SoftwareROMAlyzer->lineEditSets->setText("*");
	}

	if ( qmc2SoftwareROMAlyzer->isHidden() )
		qmc2SoftwareROMAlyzer->show();
	else if ( qmc2SoftwareROMAlyzer->isMinimized() )
		qmc2SoftwareROMAlyzer->showNormal();

	QTimer::singleShot(0, qmc2SoftwareROMAlyzer, SLOT(raise()));
}

void MainWindow::on_actionRunRomTool_triggered(bool)
{
	if ( !qmc2CurrentItem )
		return;

	if ( qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_MACHINE) == MachineList::trWaitingForData )
		return;

	QString command(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/RomTool", QString()).toString());
	QStringList args(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/RomToolArguments", QString()).toString().split(' '));
	QString wd(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/RomToolWorkingDirectory", QString()).toString());
	QString machineId(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_NAME));
	QString gameDescription(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_MACHINE));
	QStringList newArgs;
	foreach (QString argument, args)
		newArgs << argument.replace("$ID$", machineId).replace("$DESCRIPTION$", gameDescription);
	ToolExecutor romTool(this, command, newArgs, wd);
	romTool.exec();
}

void MainWindow::on_actionAnalyseCurrentROM_triggered(bool)
{
	if ( !qmc2CurrentItem )
		return;

	if ( qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_MACHINE) == MachineList::trWaitingForData )
		return;

	if ( qmc2SystemROMAlyzer && qmc2SystemROMAlyzer->active() ) {
		log(QMC2_LOG_FRONTEND, tr("please wait for ROMAlyzer to finish the current analysis and try again"));
		return;
	}

	if ( !qmc2SystemROMAlyzer )
		qmc2SystemROMAlyzer = new ROMAlyzer(0, QMC2_ROMALYZER_MODE_SYSTEM);

	qmc2SystemROMAlyzer->lineEditSets->setText(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_NAME));

	if ( qmc2SystemROMAlyzer->isHidden() )
		qmc2SystemROMAlyzer->show();
	else if ( qmc2SystemROMAlyzer->isMinimized() )
		qmc2SystemROMAlyzer->showNormal();

	if ( qmc2SystemROMAlyzer->tabWidgetAnalysis->currentWidget() != qmc2SystemROMAlyzer->tabReport && qmc2SystemROMAlyzer->tabWidgetAnalysis->currentWidget() != qmc2SystemROMAlyzer->tabLog )
		qmc2SystemROMAlyzer->tabWidgetAnalysis->setCurrentWidget(qmc2SystemROMAlyzer->tabReport);

	QTimer::singleShot(0, qmc2SystemROMAlyzer, SLOT(raise()));
	QTimer::singleShot(0, qmc2SystemROMAlyzer->pushButtonAnalyze, SLOT(click()));
}

void MainWindow::actionRebuildRom_triggered(bool)
{
	if ( !qmc2CurrentItem )
		return;

	if ( qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_MACHINE) == MachineList::trWaitingForData )
		return;

	if ( qmc2SystemROMAlyzer && qmc2SystemROMAlyzer->rebuilderActive() ) {
		log(QMC2_LOG_FRONTEND, tr("please wait for ROMAlyzer to finish the current rebuild and try again"));
		return;
	}

	bool initial = false;
	if ( !qmc2SystemROMAlyzer ) {
		qmc2SystemROMAlyzer = new ROMAlyzer(0, QMC2_ROMALYZER_MODE_SYSTEM);
		initial = true;
	}

	if ( qmc2SystemROMAlyzer->tabWidgetAnalysis->currentWidget() != qmc2SystemROMAlyzer->tabCollectionRebuilder )
		qmc2SystemROMAlyzer->tabWidgetAnalysis->setCurrentWidget(qmc2SystemROMAlyzer->tabCollectionRebuilder);

	if ( qmc2SystemROMAlyzer->isHidden() )
		qmc2SystemROMAlyzer->show();
	else if ( qmc2SystemROMAlyzer->isMinimized() )
		qmc2SystemROMAlyzer->showNormal();

	QTimer::singleShot(0, qmc2SystemROMAlyzer, SLOT(raise()));

	CollectionRebuilder *cr = qmc2SystemROMAlyzer->collectionRebuilder();
	if ( cr ) {
		cr->comboBoxXmlSource->setCurrentIndex(0);
		cr->setIgnoreCheckpoint(true);
		cr->checkBoxFilterExpression->setChecked(true);
		cr->comboBoxFilterSyntax->setCurrentIndex(4);
		cr->comboBoxFilterType->setCurrentIndex(0);
		cr->toolButtonExactMatch->setChecked(true);
		cr->checkBoxFilterStates->setChecked(false);
		cr->lineEditFilterExpression->setText(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_NAME));
		if ( !initial )
			cr->plainTextEditLog->clear();
		QTimer::singleShot(0, cr->pushButtonStartStop, SLOT(click()));
	}
}

void MainWindow::update_rebuildRomActions_visibility()
{
	bool enable = false;
	if ( qmc2SystemROMAlyzer )
		enable = (qmc2SystemROMAlyzer->groupBoxCheckSumDatabase->isChecked() && qmc2SystemROMAlyzer->groupBoxSetRewriter->isChecked());
	else
		enable = (qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/EnableCheckSumDb", false).toBool() && qmc2Config->value(QMC2_FRONTEND_PREFIX + "ROMAlyzer/EnableSetRewriter", false).toBool());
	foreach (QAction *a, rebuildRomActions)
		a->setVisible(enable);
}

void MainWindow::on_actionClearImageCache_triggered(bool)
{
	qmc2ImagePixmapCache.clear();
	log(QMC2_LOG_FRONTEND, tr("image cache cleared"));
}

void MainWindow::on_actionClearIconCache_triggered(bool)
{
	qmc2IconHash.clear();
	qmc2IconsPreloaded = false;
#if defined(QMC2_LIBARCHIVE_ENABLED)
	if ( QMC2_ICON_FILETYPE_ARCHIVE )
		foreach (ArchiveFile *archiveFile, qmc2IconArchiveMap)
			archiveFile->reopen();
#endif
	log(QMC2_LOG_FRONTEND, tr("icon cache cleared"));
}

void MainWindow::on_actionClearProjectMESSCache_triggered(bool)
{
	QString cacheStatus(tr("freed %n byte(s) in %1", "", qmc2ProjectMESSCache.totalCost()).arg(tr("%n entry(s)", "", qmc2ProjectMESSCache.count())));
	qmc2ProjectMESSCache.clear();
	log(QMC2_LOG_FRONTEND, tr("ProjectMESS in-memory cache cleared (%1)").arg(cacheStatus));
}

#if defined(QMC2_YOUTUBE_ENABLED)
void MainWindow::on_actionClearYouTubeCache_triggered(bool)
{
	QDir youTubeCacheDir(qmc2Config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/CacheDirectory").toString());
	quint64 removedBytes = 0, removedFiles = 0;
	if ( youTubeCacheDir.exists() ) {
		QStringList youTubeCacheFiles = youTubeCacheDir.entryList(QStringList("*"));
		foreach (QString youTubeCacheFile, youTubeCacheFiles) {
			QFileInfo fi(youTubeCacheDir.filePath(youTubeCacheFile));
			qint64 fSize = fi.size();
			if ( youTubeCacheDir.remove(youTubeCacheFile) ) {
				removedBytes += fSize;
				removedFiles++;
			}
			qApp->processEvents();
		}
	}
	QString removalInfo(tr("removed %n byte(s) in %1", "", removedBytes).arg(tr("%n file(s)", "", removedFiles)));
	log(QMC2_LOG_FRONTEND, tr("YouTube on-disk cache cleared (%1)").arg(removalInfo));
}
#endif

void MainWindow::on_actionClearROMStateCache_triggered(bool)
{
	if ( !qmc2ForceCacheRefresh ) {
		if ( qmc2ReloadActive ) {
			log(QMC2_LOG_FRONTEND, tr("please wait for reload to finish and try again"));
			return;
		}
		if ( qmc2VerifyActive ) {
			log(QMC2_LOG_FRONTEND, tr("please wait for ROM verification to finish and try again"));
			return;
		}
	}
	QString fileName(qmc2Config->value("MAME/FilesAndDirectories/ROMStateCacheFile", Options::configPath() + "/mame.rsc").toString());
	QFile f(fileName);
	if ( f.exists() ) {
		if ( f.remove() )
			log(QMC2_LOG_FRONTEND, tr("ROM state cache file '%1' removed").arg(fileName));
		else
			log(QMC2_LOG_FRONTEND, tr("WARNING: cannot remove the ROM state cache file '%1', please check permissions").arg(fileName));
	}
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "MachineList/AutoTriggerROMCheck").toBool() ) {
		qmc2MachineList->autoRomCheck = qmc2Config->value(QMC2_FRONTEND_PREFIX + "MachineList/AutoTriggerROMCheck").toBool();
		log(QMC2_LOG_FRONTEND, tr("triggering an automatic ROM check on next reload"));
	}
}

void MainWindow::on_actionClearMachineListCache_triggered(bool)
{
	if ( !qmc2ForceCacheRefresh ) {
		if ( qmc2ReloadActive ) {
			log(QMC2_LOG_FRONTEND, tr("please wait for reload to finish and try again"));
			return;
		}
	}
	QString fileName(qmc2Config->value("MAME/FilesAndDirectories/MachineListCacheFile", Options::configPath() + "/mame.mlc").toString());
	QFile f(fileName);
	if ( f.exists() ) {
		if ( f.remove() )
			log(QMC2_LOG_FRONTEND, tr("machine list cache file '%1' removed").arg(fileName));
		else
			log(QMC2_LOG_FRONTEND, tr("WARNING: cannot remove the machine list cache file '%1', please check permissions").arg(fileName));
	}
	qmc2MachineList->machineListDb()->setLogActive(true);
	qmc2MachineList->machineListDb()->recreateDatabase();
	qmc2MachineList->machineListDb()->setLogActive(false);
}

void MainWindow::on_actionClearXMLCache_triggered(bool)
{
	if ( !qmc2ForceCacheRefresh ) {
		if ( qmc2ReloadActive ) {
			log(QMC2_LOG_FRONTEND, tr("please wait for reload to finish and try again"));
			return;
		}
	}
	qmc2MachineList->xmlDb()->recreateDatabase();
	systemSoftwareListHash.clear();
	systemSoftwareFilterHash.clear();
}

void MainWindow::on_actionClearSlotInfoCache_triggered(bool)
{
	if ( !qmc2ForceCacheRefresh ) {
		if ( qmc2ReloadActive ) {
			log(QMC2_LOG_FRONTEND, tr("please wait for reload to finish and try again"));
			return;
		}
	}
	QString fileName(qmc2Config->value("MAME/FilesAndDirectories/SlotInfoCacheFile", Options::configPath() + "/mame.sic").toString());
	QFile f(fileName);
	if ( f.exists() ) {
		if ( f.remove() )
			log(QMC2_LOG_FRONTEND, tr("slot info cache file '%1' removed").arg(fileName));
		else
			log(QMC2_LOG_FRONTEND, tr("WARNING: cannot remove the slot info cache file '%1', please check permissions").arg(fileName));
	}
}

void MainWindow::on_actionClearSoftwareListCache_triggered(bool)
{
	if ( !qmc2ForceCacheRefresh ) {
		if ( qmc2ReloadActive ) {
			log(QMC2_LOG_FRONTEND, tr("please wait for reload to finish and try again"));
			return;
		}
	}
	if ( !swlDb ) {
		swlDb = new SoftwareListXmlDatabaseManager(qmc2MainWindow);
		swlDb->setSyncMode(QMC2_DB_SYNC_MODE_OFF);
		swlDb->setJournalMode(QMC2_DB_JOURNAL_MODE_MEMORY);
	}
	swlDb->recreateDatabase();
	if ( qmc2ImageChecker )
		QTimer::singleShot(0, qmc2ImageChecker, SLOT(updateCornerWidget()));
}

void MainWindow::on_actionClearAllEmulatorCaches_triggered(bool complete)
{
	actionClearROMStateCache->trigger();
	actionClearMachineListCache->trigger();
	actionClearXMLCache->trigger();
	actionClearSlotInfoCache->trigger();
	actionClearSoftwareListCache->trigger();
	if ( complete )
		qmc2Config->remove(QMC2_EMULATOR_PREFIX + "Cache");
}

void MainWindow::on_actionRecreateTemplateMap_triggered(bool)
{
	if ( qmc2GlobalEmulatorOptions != 0 ) {
		qmc2GlobalEmulatorOptions->createTemplateMap();
		qmc2GlobalEmulatorOptions->clear();
		qmc2GlobalEmulatorOptions->createMap();
		qmc2GlobalEmulatorOptions->load();
	}
}

void MainWindow::on_actionCheckTemplateMap_triggered(bool)
{
	if ( qmc2GlobalEmulatorOptions != 0 )
		qmc2GlobalEmulatorOptions->checkTemplateMap();
}

void MainWindow::on_actionOptions_triggered(bool)
{
	if ( qmc2Options->isHidden() )
		qmc2Options->show();
	else if ( qmc2Options->isMinimized() )
		qmc2Options->showNormal();
	QTimer::singleShot(0, qmc2Options, SLOT(raise()));
}

void MainWindow::on_actionRelaunchSetupWizard_triggered(bool)
{
	qmc2ReconfigureRestartRequested = true;
	QTimer::singleShot(0, this, SLOT(close()));
}

void MainWindow::on_actionFullscreenToggle_triggered(bool)
{
	static quint64 lastFullScreenSwitchTime = 0;

#if defined(QMC2_YOUTUBE_ENABLED)
	if ( qmc2YouTubeWidget )
		if ( qmc2YouTubeWidget->videoWidget()->isFullScreen() ) {
#if QT_VERSION < 0x050000
			qmc2YouTubeWidget->videoWidget()->setFullScreen(false);
#else
			qmc2YouTubeWidget->switchToWindowed();
#endif
			qmc2YouTubeWidget->clearMessage();
			qApp->processEvents();
			if ( windowState() & Qt::WindowFullScreen )
				actionFullscreenToggle->setChecked(true);
			else
				actionFullscreenToggle->setChecked(false);
			return;
		}
#endif

	// avoid switching too quickly...
	quint64 now = QDateTime::currentDateTime().toMSecsSinceEpoch();
	if ( now - lastFullScreenSwitchTime < QMC2_FULLSCREEN_SWITCH_DELAY )
		return;
	lastFullScreenSwitchTime = now;

	if ( !qmc2EarlyStartup ) {
		// saftey checks
		if ( windowState() & Qt::WindowFullScreen )
			actionFullscreenToggle->setChecked(false);
		else
			actionFullscreenToggle->setChecked(true);
	}

	bool feLogScrollBarMaximum = (textBrowserFrontendLog->verticalScrollBar()->value() == textBrowserFrontendLog->verticalScrollBar()->maximum());
	bool emuLogScrollBarMaximum = (textBrowserEmulatorLog->verticalScrollBar()->value() == textBrowserEmulatorLog->verticalScrollBar()->maximum());

	qApp->processEvents();

	if ( actionFullscreenToggle->isChecked() ) {
		if ( isVisible() && !(windowState() & Qt::WindowFullScreen) ) {
			if ( isMaximized() )
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Maximized", true);
			else {
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Geometry", saveGeometry());
#if QT_VERSION < 0x050000
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Position", pos());
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Size", size());
#endif
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Maximized", false);
			}
		} else {
#if QT_VERSION < 0x050000
			if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Size") )
				resize(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Size").toSize());
			if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Position") )
				move(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Position").toPoint());
#endif
			if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Geometry") )
				restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Geometry").toByteArray());
		}
		// we need to tweak full screen startup to ensure that raising the window works
		if ( qmc2EarlyStartup ) {
			setUpdatesEnabled(true);
			showNormal();
			raise();
		}
		showFullScreen();
		if ( qmc2EarlyStartup ) {
			qApp->processEvents();
			setUpdatesEnabled(false);
		}
	} else {
#if defined(QMC2_YOUTUBE_ENABLED)
		bool youTubeWasPlaying = false;
		if ( qmc2YouTubeWidget )
			youTubeWasPlaying = qmc2YouTubeWidget->isPlaying();
#endif
		if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Maximized", false).toBool() )
			showMaximized();
		else {
#if QT_VERSION < 0x050000
			if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Size") )
				resize(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Size").toSize());
			if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Position") )
				move(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Position").toPoint());
#endif
			if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Geometry") )
				restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Geometry").toByteArray());
			showNormal();
		}
#if defined(QMC2_YOUTUBE_ENABLED)
		if ( qmc2YouTubeWidget && youTubeWasPlaying ) {
			qApp->processEvents();
			if ( !qmc2YouTubeWidget->isPlaying() )
				QTimer::singleShot(0, qmc2YouTubeWidget->videoPlayer(), SLOT(play()));
		}
#endif
	}

	activateWindow();
	raise();

	if ( feLogScrollBarMaximum )
		textBrowserFrontendLog->verticalScrollBar()->setValue(textBrowserFrontendLog->verticalScrollBar()->maximum());
	if ( emuLogScrollBarMaximum )
		textBrowserEmulatorLog->verticalScrollBar()->setValue(textBrowserEmulatorLog->verticalScrollBar()->maximum());
}

void MainWindow::on_actionDocumentation_triggered(bool)
{
	if ( !qmc2DocBrowser ) {
		qmc2DocBrowser = new DocBrowser(this);
		qmc2DocBrowser->browser->spinBoxZoom->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/DocBrowser/Zoom", 100).toInt());
		QString searchPath(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/DataDirectory").toString() + "doc/html/" + qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Language", "us").toString());
		QFileInfo fi(searchPath + "/index.html");
		if ( !fi.exists() || !fi.isFile() || fi.isSymLink() ) // fall back to US English if there's no language-specific index file
			searchPath = qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/DataDirectory").toString() + "doc/html/us";
#if defined(QMC2_OS_WIN)
		QDir searchDir(searchPath);
		QUrl docUrl("file:///" + searchDir.absolutePath() + "/index.html");
#else
		QUrl docUrl("file://" + searchPath + "/index.html");
#endif
		qmc2DocBrowser->browser->webViewBrowser->load(docUrl);
	}
	if ( qmc2DocBrowser->isMinimized() )
		qmc2DocBrowser->showNormal();
	else
		qmc2DocBrowser->show();

	qmc2DocBrowser->raise();
}

void MainWindow::on_actionAbout_triggered(bool)
{
	if ( !qmc2About )
		qmc2About = new About(this);
	qmc2About->show();
	qmc2About->raise();
}

void MainWindow::on_actionHomepage_triggered(bool)
{
	QDesktopServices::openUrl(QUrl::fromUserInput("http://qmc2.batcom-it.net/"));
}

void MainWindow::on_actionWiki_triggered(bool)
{
	QDesktopServices::openUrl(QUrl::fromUserInput("http://wiki.batcom-it.net/index.php?title=The_'ultimate'_guide_to_QMC2"));
}

void MainWindow::on_actionForum_triggered(bool)
{
	QDesktopServices::openUrl(QUrl::fromUserInput("http://forums.bannister.org/ubbthreads.php?ubb=postlist&Board=12"));
}

void MainWindow::on_actionBugTracker_triggered(bool)
{
	QDesktopServices::openUrl(QUrl::fromUserInput("http://tracker.batcom-it.net/view_all_bug_page.php?project_id=1"));
}

void MainWindow::on_actionAboutQt_triggered(bool)
{
	QMessageBox::aboutQt(this, tr("About Qt"));
}

void MainWindow::on_actionArcadeSetup_triggered(bool)
{
	ArcadeModeSetup	arcadeModeSetup(this);
	arcadeModeSetup.exec();
}

void MainWindow::on_actionLaunchArcade_triggered(bool)
{
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/ExecutableFile").toString().isEmpty() ) {
		log(QMC2_LOG_FRONTEND, tr("INFORMATION: the arcade mode has to be set up first, launching the respective dialog instead"));
		on_actionArcadeSetup_triggered(false);
		return;
	}

	QStringList args;
	if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/Theme").toString().isEmpty() )
		args << "-theme" << qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/Theme").toString();
	if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/ConsoleType").toString().isEmpty() )
		args << "-console" << qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/ConsoleType").toString();
	if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/GraphicsSystem").toString().isEmpty() )
		args << "-graphicssystem" << qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/GraphicsSystem").toString();
	if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/ConfigurationPath").toString().isEmpty() )
		args << "-config_path" << QDir::toNativeSeparators(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/ConfigurationPath").toString());
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/DebugKeys").toBool() )
		args << "-debugkeys";
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/NoJoy").toBool() )
		args << "-nojoy";
	else {
		if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/Joy").toBool() )
			args << "-joy" << QString::number(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/JoyIndex", 0).toInt());
		if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/DebugJoy").toBool() )
			args << "-debugjoy";
	}

	QString commandString(QDir::toNativeSeparators(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/ExecutableFile").toString()));
	foreach (QString arg, args)
		commandString += ' ' + arg;

	log(QMC2_LOG_FRONTEND, tr("arcade mode: launching QMC2 Arcade, command = '%1'").arg(commandString));

	if ( !QProcess::startDetached(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/ExecutableFile", QString()).toString(), args, qmc2Config->value(QMC2_FRONTEND_PREFIX + "Arcade/WorkingDirectory", QString()).toString()) )
		log(QMC2_LOG_FRONTEND, tr("WARNING: failed launching QMC2 Arcade"));
}

void MainWindow::on_comboBoxSearch_editTextChanged(const QString &text)
{
	comboBoxToolbarSearch->lineEdit()->blockSignals(true);
	int cPos = comboBoxToolbarSearch->lineEdit()->cursorPosition();
	comboBoxToolbarSearch->lineEdit()->setText(text);
	comboBoxToolbarSearch->lineEdit()->setCursorPosition(cPos);
	comboBoxToolbarSearch->lineEdit()->blockSignals(false);
	if ( searchActive )
		stopSearch = true;
	searchTimer.start(QMC2_SEARCH_DELAY);
}

void MainWindow::comboBoxSearch_editTextChanged_delayed()
{
	static QString lastSearchText;
	static bool lastNegatedMatch = false;
	static bool lastIncludeBioses = actionSearchIncludeBiosSets->isChecked();
	static bool lastIncludeDevices = actionSearchIncludeDeviceSets->isChecked();

	searchTimer.stop();

	if ( qmc2CleaningUp )
		return;

	if ( searchActive ) {
		stopSearch = true;
		searchTimer.start(QMC2_SEARCH_DELAY/10);
		return;
	} else
		stopSearch = false;

	if ( treeWidgetMachineList->topLevelItemCount() == 1 )
		if ( treeWidgetMachineList->topLevelItem(0)->text(QMC2_MACHINELIST_COLUMN_MACHINE) == MachineList::trWaitingForData )
			return;

	QString pattern(comboBoxSearch->currentText());
	bool includeBioses = actionSearchIncludeBiosSets->isChecked();
	bool includeDevices = actionSearchIncludeDeviceSets->isChecked();

	if ( pattern.isEmpty() ) {
		listWidgetSearch->clear();
		lastSearchText.clear();
		lastNegatedMatch = negatedMatch;
		lastIncludeBioses = includeBioses;
		lastIncludeDevices = includeDevices;
		qmc2MachineList->numMatchedMachines = listWidgetSearch->count();
		labelMachineListStatus->setText(qmc2MachineList->status());
		lcdNumberSearchResults->display(qmc2MachineList->numMatchedMachines);
		return;
	} else if ( listWidgetSearch->count() == 0 )
		lastSearchText.clear();

	if ( pattern == lastSearchText && lastNegatedMatch == negatedMatch && lastIncludeBioses == includeBioses && lastIncludeDevices == includeDevices )
		return;

	QString patternCopy(pattern);

	// easy pattern match
	int pos = 0;
	QRegExp rxAsterisk("(\\*)");
	while ( (pos = rxAsterisk.indexIn(pattern, pos)) != -1 ) {
		int matchedLength = rxAsterisk.matchedLength();
		if ( pos > 0 ) {
			if ( pattern[pos - 1] != '\\' ) {
				pattern.replace(pos, 1, ".*");
				matchedLength = 2;
			}
		} else {
			pattern.replace(pos, 1, ".*");
			matchedLength = 2;
		}
		pos += matchedLength;
	}

	pos = 0;
	QRegExp rxQuestionMark("(\\?)");
	while ( (pos = rxQuestionMark.indexIn(pattern, pos)) != -1 ) {
		if ( pos > 0 ) {
			if ( pattern[pos - 1] != '\\' )
				pattern.replace(pos, 1, ".");
		} else
			pattern.replace(pos, 1, ".");
		pos += rxQuestionMark.matchedLength();
	}

	pattern.replace(' ', ".* .*").replace(".*^", "").replace("$.*", "");

	listWidgetSearch->clear();

	QRegExp patternRx(QRegExp(pattern, Qt::CaseInsensitive, QRegExp::RegExp2));
	if ( !patternRx.isValid() ) {
		lastSearchText.clear();
		lastNegatedMatch = negatedMatch;
		lastIncludeBioses = includeBioses;
		lastIncludeDevices = includeDevices;
		qmc2MachineList->numMatchedMachines = listWidgetSearch->count();
		labelMachineListStatus->setText(qmc2MachineList->status());
		lcdNumberSearchResults->display(qmc2MachineList->numMatchedMachines);
		return;
	}

	searchActive = true;
	lastSearchText = patternCopy;

	progressBarSearch->setVisible(true);
	progressBarSearch->setRange(0, treeWidgetMachineList->topLevelItemCount());
	progressBarSearch->setValue(0);

	QString currentItemText;
	if ( qmc2CurrentItem )
		currentItemText = qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_MACHINE);

	QList<QTreeWidgetItem *> matches;
	QList<QListWidgetItem *> itemList;
	QListWidgetItem *currentItemPendant = 0;

	for (int i = 0; i < treeWidgetMachineList->topLevelItemCount() && !stopSearch && !qmc2CleaningUp; i++) {
		QTreeWidgetItem *item = treeWidgetMachineList->topLevelItem(i);
		QString itemName = item->text(QMC2_MACHINELIST_COLUMN_NAME);
		if ( !includeBioses && qmc2MachineList->isBios(itemName) )
			continue;
		if ( !includeDevices && qmc2MachineList->isDevice(itemName) )
			continue;
		QString itemText = item->text(QMC2_MACHINELIST_COLUMN_MACHINE);
		bool matched = itemText.indexOf(patternRx) > -1 || itemName.indexOf(patternRx) > -1;
		if ( negatedMatch )
			matched = !matched;
		if ( matched ) {
			matches << item;
			QListWidgetItem *newItem = new QListWidgetItem();
			newItem->setText(itemText);
			newItem->setWhatsThis(itemName);
			itemList << newItem;
			if ( currentItemText == itemText )
				currentItemPendant = newItem;
		}
		progressBarSearch->setValue(progressBarSearch->value() + 1);
		if ( i % QMC2_SEARCH_RESULT_UPDATE == 0 ) {
			foreach (QListWidgetItem *item, itemList)
				listWidgetSearch->addItem(item);
			itemList.clear();
			qmc2MachineList->numMatchedMachines = listWidgetSearch->count();
			labelMachineListStatus->setText(qmc2MachineList->status());
			lcdNumberSearchResults->display(qmc2MachineList->numMatchedMachines);
			if ( currentItemPendant ) {
				listWidgetSearch->setCurrentItem(currentItemPendant, QItemSelectionModel::ClearAndSelect);
				listWidgetSearch->scrollToItem(currentItemPendant, qmc2CursorPositioningMode);
			}
			qApp->processEvents();
		}
	}

	if ( !stopSearch && !qmc2CleaningUp ) {
		foreach (QListWidgetItem *item, itemList)
			listWidgetSearch->addItem(item);
		qmc2MachineList->numMatchedMachines = listWidgetSearch->count();
		labelMachineListStatus->setText(qmc2MachineList->status());
		lcdNumberSearchResults->display(qmc2MachineList->numMatchedMachines);
		if ( currentItemPendant ) {
			listWidgetSearch->setCurrentItem(currentItemPendant, QItemSelectionModel::ClearAndSelect);
			listWidgetSearch->scrollToItem(currentItemPendant, qmc2CursorPositioningMode);
		}
	} else
		lastSearchText.clear();

	lastNegatedMatch = negatedMatch;
	lastIncludeBioses = includeBioses;
	lastIncludeDevices = includeDevices;

	progressBarSearch->setVisible(false);
	progressBarSearch->reset();

	searchActive = false;
}

void MainWindow::on_comboBoxSearch_activated(const QString &text)
{
	if ( tabWidgetMachineList->currentWidget() != tabSearch ) {
		tabWidgetMachineList->blockSignals(true);
		tabWidgetMachineList->setCurrentWidget(tabSearch);
		tabWidgetMachineList->blockSignals(false);
	}
	int index = comboBoxToolbarSearch->findText(text);
	if ( index < 0 ) {
		comboBoxToolbarSearch->insertItem(0, text);
		comboBoxToolbarSearch->setCurrentIndex(0);
		if ( comboBoxToolbarSearch->count() > QMC2_MACHINE_SEARCH_HISTORY_LENGTH )
			comboBoxToolbarSearch->removeItem(QMC2_MACHINE_SEARCH_HISTORY_LENGTH);
	} else {
		comboBoxToolbarSearch->blockSignals(true);
		comboBoxToolbarSearch->removeItem(index);
		comboBoxToolbarSearch->insertItem(0, text);
		comboBoxToolbarSearch->setCurrentIndex(0);
		comboBoxToolbarSearch->blockSignals(false);
	}
	index = comboBoxSearch->findText(text);
	if ( index >= 0 ) {
		comboBoxSearch->removeItem(index);
		comboBoxSearch->insertItem(0, text);
		comboBoxSearch->setCurrentIndex(0);
	}
	if ( comboBoxSearch->count() > QMC2_MACHINE_SEARCH_HISTORY_LENGTH )
		comboBoxSearch->removeItem(QMC2_MACHINE_SEARCH_HISTORY_LENGTH);
	comboBoxSearch_editTextChanged_delayed();
	listWidgetSearch->setFocus();
}

void MainWindow::on_listWidgetSearch_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
	static bool isActive = false;
	if ( isActive )
		return;
	isActive = true;
	QTreeWidgetItem *mlItem = 0;
	if ( current )
		mlItem = qmc2MachineListItemHash.value(current->whatsThis());
	if ( mlItem ) {
		qmc2CheckItemVisibility = false;
		treeWidgetMachineList->clearSelection();
		qmc2CurrentItem = mlItem;
		treeWidgetMachineList->setCurrentItem(mlItem);
	}
	isActive = false;
}

void MainWindow::on_listWidgetSearch_itemPressed(QListWidgetItem *current)
{
	on_listWidgetSearch_currentItemChanged(current, 0);
}

void MainWindow::on_listWidgetSearch_itemSelectionChanged()
{
	QList<QListWidgetItem *> selected(listWidgetSearch->selectedItems());
	if ( !selected.isEmpty() )
		on_listWidgetSearch_currentItemChanged(selected.first(), 0);
}

void MainWindow::on_listWidgetSearch_itemActivated(QListWidgetItem *item)
{
	if ( item == 0 )
		return;

	QList<QTreeWidgetItem *> matchItemList = treeWidgetMachineList->findItems(item->text(), Qt::MatchExactly);
	if ( !matchItemList.isEmpty() ) {
		QTreeWidgetItem *matchItem = matchItemList[0];
		qmc2CheckItemVisibility = false;
		treeWidgetMachineList->clearSelection();
		treeWidgetMachineList->setCurrentItem(matchItem);
		treeWidgetMachineList->scrollToItem(matchItem, qmc2CursorPositioningMode);
		qmc2CurrentItem = matchItem;
		if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "MachineList/PlayOnSublistActivation").toBool() ) {
			if ( qmc2DemoModeDialog )
				if ( qmc2DemoModeDialog->demoModeRunning )
					return;
			switch ( qmc2DefaultLaunchMode ) {
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
				case QMC2_LAUNCH_MODE_EMBEDDED:
					QTimer::singleShot(0, this, SLOT(on_actionPlayEmbedded_triggered()));
					break;
#endif
				case QMC2_LAUNCH_MODE_INDEPENDENT:
				default:
					QTimer::singleShot(0, this, SLOT(on_actionPlay_triggered()));
					break;
			}
		} else {
			tabWidgetMachineList->setCurrentWidget(tabMachineList);
			if ( !qmc2ReloadActive )
				treeWidgetMachineList->expandItem(matchItem);
		}
	} else
		log(QMC2_LOG_FRONTEND, tr("ERROR: no match found (?)"));
}

void MainWindow::on_listWidgetFavorites_itemSelectionChanged()
{
	QList<QListWidgetItem *> selected(listWidgetFavorites->selectedItems());
	if ( !selected.isEmpty() )
		on_listWidgetSearch_currentItemChanged(selected.first(), 0);
}

void MainWindow::on_listWidgetFavorites_itemActivated(QListWidgetItem *item)
{
	on_listWidgetSearch_itemActivated(item);
}

void MainWindow::on_listWidgetPlayed_itemSelectionChanged()
{
	QList<QListWidgetItem *> selected(listWidgetPlayed->selectedItems());
	if ( !selected.isEmpty() )
		on_listWidgetSearch_currentItemChanged(selected.first(), 0);
}

void MainWindow::on_listWidgetPlayed_itemActivated(QListWidgetItem *item)
{
	on_listWidgetSearch_itemActivated(item);
}

void MainWindow::on_tabWidgetMachineList_currentChanged(int currentIndex)
{
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
	static int lastTabWidgetMachineListIndex = -1;
#endif

	if ( !qmc2EarlyStartup ) {
		menuMachineListHeader->hide();
		menuHierarchyHeader->hide();
		menuCategoryHeader->hide();
		menuVersionHeader->hide();
	}

	ComponentInfo *componentInfo = qmc2ComponentSetup->componentInfoHash().value("Component1");
	if ( componentInfo->appliedFeatureList().count() <= currentIndex || currentIndex < 0 )
		return;

#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
	if ( hSplitterSizes.count() > 1 && componentInfo->appliedFeatureList().at(currentIndex) != QMC2_EMBED_INDEX )
		hSplitter->setSizes(hSplitterSizes);
	int embedIndex = componentInfo->appliedFeatureList().indexOf(QMC2_EMBED_INDEX);
	if ( embedIndex >= 0 && embedIndex <= currentIndex )
		if ( tabWidgetMachineList->indexOf(tabEmbeddedEmus) < 0 )
			currentIndex++;
#endif
	int foreignIndex = componentInfo->appliedFeatureList().indexOf(QMC2_FOREIGN_INDEX);
	if ( foreignIndex >= 0 && foreignIndex <= currentIndex )
		if ( tabWidgetMachineList->indexOf(tabForeignEmulators) < 0 )
			currentIndex++;
	switch ( componentInfo->appliedFeatureList().at(currentIndex) ) {
		case QMC2_MACHINELIST_INDEX:
			actionToggleTagCursorDown->setVisible(true);
			actionToggleTagCursorUp->setVisible(true);
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
			if ( lastTabWidgetMachineListIndex >= 0 )
				if ( componentInfo->appliedFeatureList().at(lastTabWidgetMachineListIndex) != QMC2_EMBED_INDEX )
					QTimer::singleShot(0, this, SLOT(scrollToCurrentItem()));
			menuBar()->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ShowMenuBar", true).toBool());
			statusBar()->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Statusbar", true).toBool());
			toolbar->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Toolbar", true).toBool());
#else
			QTimer::singleShot(0, this, SLOT(scrollToCurrentItem()));
#endif
			on_comboBoxViewSelect_currentIndexChanged(comboBoxViewSelect->currentIndex());
			if ( !qmc2MachineList->initialLoad ) {
				switch ( stackedWidgetView->currentIndex() ) {
					case QMC2_VIEWHIERARCHY_INDEX:
						treeWidgetHierarchy->activateWindow();
						treeWidgetHierarchy->setFocus();
						QTimer::singleShot(QMC2_RANK_UPDATE_DELAY, this, SLOT(treeWidgetHierarchy_verticalScrollChanged()));
						break;
					case QMC2_VIEWCATEGORY_INDEX:
						treeWidgetCategoryView->activateWindow();
						treeWidgetCategoryView->setFocus();
						QTimer::singleShot(0, this, SLOT(viewByCategory()));
						break;
					case QMC2_VIEWVERSION_INDEX:
						treeWidgetVersionView->activateWindow();
						treeWidgetVersionView->setFocus();
						QTimer::singleShot(0, this, SLOT(viewByVersion()));
						break;
					case QMC2_VIEWMACHINELIST_INDEX:
					default:
						treeWidgetMachineList->activateWindow();
						treeWidgetMachineList->setFocus();
						QTimer::singleShot(QMC2_RANK_UPDATE_DELAY, this, SLOT(treeWidgetMachineList_verticalScrollChanged()));
						break;
				}
			}
			break;

		case QMC2_SEARCH_INDEX:
			actionToggleTagCursorDown->setVisible(false);
			actionToggleTagCursorUp->setVisible(false);
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
			if ( lastTabWidgetMachineListIndex >= 0 )
				if ( componentInfo->appliedFeatureList().at(lastTabWidgetMachineListIndex) != QMC2_EMBED_INDEX )
					QTimer::singleShot(0, this, SLOT(checkCurrentSearchSelection()));
			menuBar()->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ShowMenuBar", true).toBool());
			statusBar()->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Statusbar", true).toBool());
			toolbar->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Toolbar", true).toBool());
#else
			QTimer::singleShot(0, this, SLOT(checkCurrentSearchSelection()));
#endif
			if ( listWidgetSearch->count() > 0 ) {
				listWidgetSearch->activateWindow();
				listWidgetSearch->setFocus();
			} else {
				comboBoxSearch->activateWindow();
				comboBoxSearch->setFocus();
			}
			break;

		case QMC2_FAVORITES_INDEX:
			actionToggleTagCursorDown->setVisible(false);
			actionToggleTagCursorUp->setVisible(false);
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
			if ( lastTabWidgetMachineListIndex >= 0 )
				if ( componentInfo->appliedFeatureList().at(lastTabWidgetMachineListIndex) != QMC2_EMBED_INDEX )
					QTimer::singleShot(0, this, SLOT(checkCurrentFavoritesSelection()));
			menuBar()->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ShowMenuBar", true).toBool());
			statusBar()->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Statusbar", true).toBool());
			toolbar->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Toolbar", true).toBool());
#else
			QTimer::singleShot(0, this, SLOT(checkCurrentFavoritesSelection()));
#endif
			if ( listWidgetFavorites->count() > 0 ) {
				listWidgetFavorites->activateWindow();
				listWidgetFavorites->setFocus();
			}
			break;

		case QMC2_PLAYED_INDEX:
			actionToggleTagCursorDown->setVisible(false);
			actionToggleTagCursorUp->setVisible(false);
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
			if ( lastTabWidgetMachineListIndex >= 0 )
				if ( componentInfo->appliedFeatureList().at(lastTabWidgetMachineListIndex) != QMC2_EMBED_INDEX )
					QTimer::singleShot(0, this, SLOT(checkCurrentPlayedSelection()));
			menuBar()->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ShowMenuBar", true).toBool());
			statusBar()->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Statusbar", true).toBool());
			toolbar->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Toolbar", true).toBool());
#else
			QTimer::singleShot(0, this, SLOT(checkCurrentPlayedSelection()));
#endif
			if ( listWidgetPlayed->count() > 0 ) {
				listWidgetPlayed->activateWindow();
				listWidgetPlayed->setFocus();
			}
			break;

		case QMC2_FOREIGN_INDEX:
			actionToggleTagCursorDown->setVisible(true);
			actionToggleTagCursorUp->setVisible(true);
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
			menuBar()->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ShowMenuBar", true).toBool());
			statusBar()->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Statusbar", true).toBool());
			toolbar->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Toolbar", true).toBool());
#else
			QTimer::singleShot(0, this, SLOT(scrollToCurrentItem()));
#endif
			if ( treeWidgetForeignIDs->topLevelItemCount() > 0 ) {
				treeWidgetForeignIDs->activateWindow();
				treeWidgetForeignIDs->setFocus();
			}
			break;

#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
		case QMC2_EMBED_INDEX:
			actionToggleTagCursorDown->setVisible(false);
			actionToggleTagCursorUp->setVisible(false);
			if ( toolButtonEmbedderMaximizeToggle->isChecked() ) {
				menuBar()->hide();
				statusBar()->hide();
				toolbar->hide();
				qApp->processEvents();
				hSplitterSizes = hSplitter->sizes();
				QList<int> maximizedSizes;
				if ( hSplitter->widget(0) == hSplitterWidget0 )
					maximizedSizes << desktopGeometry.width() << 0;
				else
					maximizedSizes << 0 << desktopGeometry.width();
				hSplitter->setSizes(maximizedSizes);
			}
			break;
#endif

		default:
			break;
	}

	ImageWidget::updateArtwork();

	// show / hide machine status indicator
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/MachineStatusIndicator").toBool() ) {
		if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/MachineStatusIndicatorOnlyWhenRequired").toBool() ) {
			if ( componentInfo->appliedFeatureList().at(currentIndex) != QMC2_MACHINELIST_INDEX )
				labelMachineStatus->setVisible(true);
			else
				labelMachineStatus->setVisible(false);
		} else
			labelMachineStatus->setVisible(true);
	} else
		labelMachineStatus->setVisible(false);

#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
	lastTabWidgetMachineListIndex = currentIndex;
#endif
}

void MainWindow::on_tabWidgetLogsAndEmulators_currentChanged(int currentIndex)
{
	ComponentInfo *componentInfo = qmc2ComponentSetup->componentInfoHash().value("Component3");
	if ( componentInfo->appliedFeatureList().count() <= currentIndex || currentIndex < 0 )
		return;
	switch ( componentInfo->appliedFeatureList().at(currentIndex) ) {
		case QMC2_FRONTENDLOG_INDEX:
			logScrollToEnd(QMC2_LOG_FRONTEND);
			break;

		case QMC2_EMULATORLOG_INDEX:
			logScrollToEnd(QMC2_LOG_EMULATOR);
			break;

		default:
			break;
	}
}

void MainWindow::on_tabWidgetSoftwareDetail_currentChanged(int currentIndex)
{
	ComponentInfo *componentInfo = qmc2ComponentSetup->componentInfoHash().value("Component4");
	if ( !qmc2SoftwareList )
		return;
	if ( !qmc2SoftwareList->currentItem )
		return;
	if ( !tabWidgetSoftwareDetail->isVisible() )
		return;
	int left, top, right, bottom;
	gridLayout->getContentsMargins(&left, &top, &right, &bottom);
	switch ( componentInfo->appliedFeatureList().at(currentIndex) ) {
		case QMC2_SWINFO_SNAPSHOT_PAGE:
			if ( qmc2SoftwareNotesEditor )
				qmc2SoftwareNotesEditor->hideTearOffMenus();
			if ( !qmc2SoftwareSnapshot ) {
				qmc2SoftwareSnapshot = new SoftwareSnapshot(tabSnapshot);
				QHBoxLayout *layout = new QHBoxLayout;
				layout->addWidget(qmc2SoftwareSnapshot);
				layout->setContentsMargins(0, 0, 0, 0);
				tabSnapshot->setLayout(layout);
			}
			break;
		case QMC2_SWINFO_PROJECTMESS_PAGE:
			if ( qmc2SoftwareNotesEditor )
				qmc2SoftwareNotesEditor->hideTearOffMenus();
			if ( qmc2SoftwareList->currentItem != qmc2LastProjectMESSItem ) {
				if ( !qmc2ProjectMESS ) {
					QVBoxLayout *layout = new QVBoxLayout;
					layout->setContentsMargins(left, top, right, bottom);
					qmc2ProjectMESS = new MiniWebBrowser(tabSoftwareProjectMESS);
					layout->addWidget(qmc2ProjectMESS);
					tabSoftwareProjectMESS->setLayout(layout);
					connect(qmc2ProjectMESS->webViewBrowser, SIGNAL(loadStarted()), this, SLOT(projectMessLoadStarted()));
				}
				QString entryName = qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_NAME);
				QString entryTitle = qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_TITLE);
				QString listName = qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_LIST);
				QString projectMessUrl = qmc2Config->value(QMC2_FRONTEND_PREFIX + "ProjectMESS/BaseURL", QMC2_PROJECT_MESS_BASE_URL).toString().arg(entryName).arg(listName);
				qmc2ProjectMESS->webViewBrowser->setStatusTip(tr("ProjectMESS page for '%1' / '%2'").arg(listName).arg(entryTitle));
				if ( !qmc2ProjectMESSCache.contains(listName + "_" + entryName) ) {
					QColor color = qmc2ProjectMESS->webViewBrowser->palette().color(QPalette::WindowText);
					qmc2ProjectMESS->webViewBrowser->setHtml(
								QString("<html><head></head><body><center><p><font color=\"#%1%2%3\"<b>").arg(color.red()).arg(color.green()).arg(color.blue()) +
									tr("Fetching ProjectMESS page for '%1' / '%2', please wait...").arg(listName).arg(entryTitle) + "</font></b></p><p>" +
									QString("(<a href=\"%1\">%1</a>)").arg(projectMessUrl) + "</p></center></body></html>",
								QUrl(projectMessUrl));
					connect(qmc2ProjectMESS->webViewBrowser, SIGNAL(loadFinished(bool)), this, SLOT(projectMessLoadFinished(bool)));
					qmc2ProjectMESS->webViewBrowser->load(QUrl(projectMessUrl));
				} else {
					// FIXME: There's currently a bug in QWebView::setHtml() so that it executes JavaScript twice.
					qmc2ProjectMESS->webViewBrowser->setHtml(QString(QMC2_UNCOMPRESS(*qmc2ProjectMESSCache[listName + "_" + entryName])), QUrl(projectMessUrl));
					qmc2ProjectMESS->webViewBrowser->load(QUrl(projectMessUrl));
				}
				qmc2ProjectMESS->homeUrl = QUrl(projectMessUrl);
				qmc2LastProjectMESSItem = qmc2SoftwareList->currentItem;
			}
			break;
		case QMC2_SWINFO_NOTES_PAGE:
			if ( qmc2SoftwareList->currentItem != qmc2LastSoftwareNotesItem ) {
				if ( !qmc2SoftwareNotesEditor ) {
					QVBoxLayout *layout = new QVBoxLayout;
					layout->setContentsMargins(left, top, right, bottom);
					qmc2SoftwareNotesEditor = new HtmlEditor("SoftwareNotes", true, tabNotes);
					layout->addWidget(qmc2SoftwareNotesEditor);
					tabNotes->setLayout(layout);
				} else {
					qmc2SoftwareNotesEditor->save();
					qmc2SoftwareNotesEditor->loadedContent.clear();
					qmc2SoftwareNotesEditor->checkRevertStatus();
				}

				QString entryName = qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_NAME);
				QString listName = qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_LIST);
				QString softwareParent = softwareParentHash[listName + ":" + entryName];
				if ( !softwareParent.isEmpty() ) {
					QStringList softwareParentWords = softwareParent.split(":");
					if ( softwareParentWords.count() > 1 )
						softwareParent = softwareParentWords[1];
					else
						softwareParent.clear();
				}

				QString softwareNotesFolder = qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareNotesFolder").toString();
				QString softwareNotesTemplate = qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareNotesTemplate").toString();
				bool useSoftwareNotesTemplate = qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/UseSoftwareNotesTemplate").toBool();
				QString fileName = softwareNotesFolder + qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_LIST) + "/" + qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_NAME) + ".html";
				qmc2SoftwareNotesEditor->setCurrentFileName(fileName);

				qmc2SoftwareNotesEditor->enableFileNewFromTemplateAction(useSoftwareNotesTemplate);

				qmc2SoftwareNotesEditor->templateMap.clear();
#if QT_VERSION < 0x050000
				qmc2SoftwareNotesEditor->templateMap["$SOFTWARE_TITLE$"] = Qt::escape(qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_TITLE));
				qmc2SoftwareNotesEditor->templateMap["$SOFTWARE_PUBLISHER$"] = Qt::escape(qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_PUBLISHER));
				qmc2SoftwareNotesEditor->templateMap["$SOFTWARE_YEAR$"] = Qt::escape(qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_YEAR));
#else
				qmc2SoftwareNotesEditor->templateMap["$SOFTWARE_TITLE$"] = qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_TITLE).toHtmlEscaped();
				qmc2SoftwareNotesEditor->templateMap["$SOFTWARE_PUBLISHER$"] = qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_PUBLISHER).toHtmlEscaped();
				qmc2SoftwareNotesEditor->templateMap["$SOFTWARE_YEAR$"] = qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_YEAR).toHtmlEscaped();
#endif
				qmc2SoftwareNotesEditor->templateMap["$SOFTWARE_NAME$"] = qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_NAME);
				qmc2SoftwareNotesEditor->templateMap["$SOFTWARE_PARENT_ID$"] = softwareParent;
				qmc2SoftwareNotesEditor->templateMap["$SOFTWARE_LIST$"] = qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_LIST);
				qmc2SoftwareNotesEditor->templateMap["$SOFTWARE_SUPPORTED$"] = qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_SUPPORTED);
				qmc2SoftwareNotesEditor->templateMap["$SOFTWARE_SUPPORTED_UT$"] = MachineList::reverseTranslations.value(qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_SUPPORTED));
				qmc2SoftwareNotesEditor->templateMap["$SOFTWARE_STATUS$"] = qmc2SoftwareList->softwareStatus(qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_LIST), qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_NAME), true);
				qmc2SoftwareNotesEditor->templateMap["$SOFTWARE_STATUS_UT$"] = qmc2SoftwareList->softwareStatus(qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_LIST), qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_NAME), false);
	      			qmc2SoftwareNotesEditor->templateMap["$GUI_LANGUAGE$"] = qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Language", "us").toString();
	      			qmc2SoftwareNotesEditor->templateMap["$EMULATOR_VARIANT$"] = QMC2_EMU_NAME_VARIANT;
	      			qmc2SoftwareNotesEditor->templateMap["$EMULATOR_TYPE$"] = QMC2_EMU_NAME;
				if ( !qmc2SoftwareSnapshot ) {
					qmc2SoftwareSnapshot = new SoftwareSnapshot(tabSnapshot);
					QHBoxLayout *layout = new QHBoxLayout;
					layout->addWidget(qmc2SoftwareSnapshot);
					layout->setContentsMargins(0, 0, 0, 0);
					tabSnapshot->setLayout(layout);
					qApp->processEvents();
				}
				QDir dataDir(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/DataDirectory").toString());
				QString ghostPath(QDir::fromNativeSeparators(dataDir.absolutePath() + "/img/ghost.png"));
#if defined(QMC2_OS_WIN)
	      			qmc2SoftwareNotesEditor->templateMap["$GHOST_IMAGE$"] = "file:///" + ghostPath;
				if ( qmc2SoftwareSnapshot->loadImage(listName, entryName) )
					qmc2SoftwareNotesEditor->templateMap["$SOFTWARE_SNAPSHOT$"] = "file:///" + QDir::fromNativeSeparators(qmc2SoftwareSnapshot->absoluteImagePath());
				else
					qmc2SoftwareNotesEditor->templateMap["$SOFTWARE_SNAPSHOT$"] = "file:///" + ghostPath;
#else
	      			qmc2SoftwareNotesEditor->templateMap["$GHOST_IMAGE$"] = "file://" + ghostPath;
				if ( qmc2SoftwareSnapshot->loadImage(listName, entryName) )
					qmc2SoftwareNotesEditor->templateMap["$SOFTWARE_SNAPSHOT$"] = "file://" + QDir::fromNativeSeparators(qmc2SoftwareSnapshot->absoluteImagePath());
				else
					qmc2SoftwareNotesEditor->templateMap["$SOFTWARE_SNAPSHOT$"] = "file://" + ghostPath;
#endif
				QString swInfo = qmc2MachineList->datInfoDb()->softwareInfo(listName, entryName);
				if ( !swInfo.isEmpty() ) {
					qmc2SoftwareNotesEditor->templateMap["$SOFTWARE_INFO$"] = swInfo.replace(QRegExp(QString("((http|https|ftp)://%1)").arg(urlSectionRegExp)), QLatin1String("<a href=\"\\1\">\\1</a>"));
					qmc2SoftwareNotesEditor->templateMap["$SOFTWARE_INFO_STATUS$"] = "OK";
				} else {
					qmc2SoftwareNotesEditor->templateMap["$SOFTWARE_INFO$"] = tr("No data available");
					qmc2SoftwareNotesEditor->templateMap["$SOFTWARE_INFO_STATUS$"] = "NO_DATA";
				}
				qmc2SoftwareNotesEditor->setCurrentTemplateName(softwareNotesTemplate);
				qmc2SoftwareNotesEditor->stopLoading = true;

				if ( QFile::exists(fileName) )
					QTimer::singleShot(25, qmc2SoftwareNotesEditor, SLOT(loadCurrent()));
				else {
					if ( useSoftwareNotesTemplate )
						QTimer::singleShot(25, qmc2SoftwareNotesEditor, SLOT(loadCurrentTemplate()));
					else
						qmc2SoftwareNotesEditor->fileNew();
				}
				qmc2SoftwareNotesEditor->setCurrentFileName(fileName);
				qmc2LastSoftwareNotesItem = qmc2SoftwareList->currentItem;
			}
			break;
		case QMC2_SWINFO_INFO_PAGE:
			if ( qmc2SoftwareList->currentItem != qmc2LastSoftwareInfoItem ) {
				QString listName = qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_LIST);
				QString entryName = qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_NAME);
				QString swInfo = qmc2MachineList->datInfoDb()->softwareInfo(listName, entryName);
				if ( !swInfo.isEmpty() )
					textBrowserSoftwareInfo->setHtml(swInfo.replace(QRegExp(QString("((http|https|ftp)://%1)").arg(urlSectionRegExp)), QLatin1String("<a href=\"\\1\">\\1</a>")));
				else
					textBrowserSoftwareInfo->setHtml("<p>" + tr("No data available") + "</p>");
				qmc2LastSoftwareInfoItem = qmc2SoftwareList->currentItem;
			}
			break;
		default:
			break;
	}
}

void MainWindow::scrollToCurrentItem()
{
	QTreeWidgetItem *ci = 0;
	if ( qmc2CurrentItem )
		ci = qmc2CurrentItem;
	else
		ci = treeWidgetMachineList->currentItem();
	if ( ci ) {
		if ( ci->text(QMC2_MACHINELIST_COLUMN_MACHINE) == MachineList::trWaitingForData )
			return;
		QString machineName(ci->text(QMC2_MACHINELIST_COLUMN_NAME));
		switch ( stackedWidgetView->currentIndex() ) {
			case QMC2_VIEWHIERARCHY_INDEX:
				ci = qmc2HierarchyItemHash.value(machineName);
				if ( ci ) {
					treeWidgetHierarchy->clearSelection();
					treeWidgetHierarchy->setCurrentItem(ci);
					treeWidgetHierarchy->scrollToItem(ci, qmc2CursorPositioningMode);
				}
				break;
			case QMC2_VIEWCATEGORY_INDEX:
				ci = qmc2CategoryItemHash.value(machineName);
				if ( ci ) {
					treeWidgetCategoryView->clearSelection();
					treeWidgetCategoryView->setCurrentItem(ci);
					treeWidgetCategoryView->scrollToItem(ci, qmc2CursorPositioningMode);
				} else if ( qmc2CurrentItem ) {
					// the category view hasn't been loaded (completely) yet, so select the item in the hierarchical view instead to avoid displaying the wrong machine detail temporarily
					// (this will only happen when the selected machine is hidden in the main view and the category view is active inititally)
					ci = qmc2HierarchyItemHash.value(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_NAME));
					if ( ci ) {
						treeWidgetHierarchy->clearSelection();
						treeWidgetHierarchy->setCurrentItem(ci);
						treeWidgetHierarchy->scrollToItem(ci, qmc2CursorPositioningMode);
						treeWidgetHierarchy->horizontalScrollBar()->setValue(0); // hidden views act strangley...
					}
				}
				break;
			case QMC2_VIEWVERSION_INDEX:
				ci = qmc2VersionItemHash.value(machineName);
				if ( ci ) {
					treeWidgetVersionView->clearSelection();
					treeWidgetVersionView->setCurrentItem(ci);
					treeWidgetVersionView->scrollToItem(ci, qmc2CursorPositioningMode);
				} else if ( qmc2CurrentItem ) {
					// the version view hasn't been loaded (completely) yet, so select the item in the hierarchical view instead to avoid displaying the wrong machine detail temporarily
					// (this will only happen when the selected machine is hidden in the main view and the version view is active inititally)
					ci = qmc2HierarchyItemHash.value(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_NAME));
					if ( ci ) {
						treeWidgetHierarchy->clearSelection();
						treeWidgetHierarchy->setCurrentItem(ci);
						treeWidgetHierarchy->scrollToItem(ci, qmc2CursorPositioningMode);
						treeWidgetHierarchy->horizontalScrollBar()->setValue(0); // hidden views act strangley...
					}
				}
				break;
			case QMC2_VIEWMACHINELIST_INDEX:
			default:
				qmc2CheckItemVisibility = false;
				treeWidgetMachineList->clearSelection();
				treeWidgetMachineList->setCurrentItem(ci);
				treeWidgetMachineList->scrollToItem(ci, qmc2CursorPositioningMode);
				break;
		}
		if ( !qmc2ReloadActive && ci )
			ci->setSelected(true);
		QTimer::singleShot(0, this, SLOT(updateUserData()));
	}
}

void MainWindow::checkCurrentSearchSelection()
{
	listWidgetSearch->setCurrentIndex(QModelIndex());
	listWidgetSearch->clearSelection();

	if ( !qmc2CurrentItem )
		return;

	listWidgetSearch->blockSignals(true);
	QList<QListWidgetItem *> searchMatches = listWidgetSearch->findItems(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_MACHINE), Qt::MatchExactly);
	if ( searchMatches.count() > 0 ) {
		QListWidgetItem *matchedItem = searchMatches[0];
		if ( matchedItem != 0 ) {
			listWidgetSearch->setCurrentItem(matchedItem, QItemSelectionModel::ClearAndSelect);
			listWidgetSearch->scrollToItem(matchedItem, qmc2CursorPositioningMode);
			qApp->processEvents();
		}
	}
	listWidgetSearch->blockSignals(false);
}

void MainWindow::checkCurrentFavoritesSelection()
{
	listWidgetFavorites->setCurrentIndex(QModelIndex());
	listWidgetFavorites->clearSelection();

	if ( !qmc2CurrentItem )
		return;

	listWidgetFavorites->blockSignals(true);
	QList<QListWidgetItem *> favoritesMatches = listWidgetFavorites->findItems(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_MACHINE), Qt::MatchExactly);
	if ( favoritesMatches.count() > 0 ) {
		QListWidgetItem *matchedItem = favoritesMatches[0];
		if ( matchedItem != 0 ) {
			listWidgetFavorites->setCurrentItem(matchedItem, QItemSelectionModel::ClearAndSelect);
			listWidgetFavorites->scrollToItem(matchedItem, qmc2CursorPositioningMode);
			qApp->processEvents();
		}
	}
	listWidgetFavorites->setFocus();
	listWidgetFavorites->blockSignals(false);
}

void MainWindow::checkCurrentPlayedSelection()
{
	listWidgetPlayed->setCurrentIndex(QModelIndex());
	listWidgetPlayed->clearSelection();

	if ( !qmc2CurrentItem )
		return;

	listWidgetPlayed->blockSignals(true);
	QList<QListWidgetItem *> playedMatches = listWidgetPlayed->findItems(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_MACHINE), Qt::MatchExactly);
	if ( playedMatches.count() > 0 ) {
		QListWidgetItem *matchedItem = playedMatches[0];
		if ( matchedItem != 0 ) {
			listWidgetPlayed->setCurrentItem(matchedItem, QItemSelectionModel::ClearAndSelect);
			listWidgetPlayed->scrollToItem(matchedItem, qmc2CursorPositioningMode);
			qApp->processEvents();
		}
	}
	listWidgetPlayed->setFocus();
	listWidgetPlayed->blockSignals(false);
}

void MainWindow::softwareLoadInterrupted()
{
	ComponentInfo *componentInfo = qmc2ComponentSetup->componentInfoHash().value("Component2");
	on_tabWidgetMachineDetail_currentChanged(componentInfo->appliedFeatureList().indexOf(QMC2_SOFTWARE_LIST_INDEX));
}

void MainWindow::on_tabWidgetMachineDetail_currentChanged(int currentIndex)
{
	static bool initialCall = true;

	// avoids crashes on critical sections
	if ( qmc2CriticalSection ) {
		retry_tabWidgetMachineDetail_currentIndex = currentIndex;
		QTimer::singleShot(QMC2_CRITSECT_POLLING_TIME, this, SLOT(retry_tabWidgetMachineDetail_currentChanged()));
		return;
	}

	ComponentInfo *componentInfo = qmc2ComponentSetup->componentInfoHash().value("Component2");

	if ( qmc2CleaningUp || componentInfo->appliedFeatureList().isEmpty() || currentIndex == -1 )
		return;

	if ( !qmc2CurrentItem || qmc2EarlyReloadActive ) {
		qmc2LastMachineInfoItem = qmc2LastEmuInfoItem = qmc2LastSoftwareInfoItem = qmc2LastConfigItem = qmc2LastDeviceConfigItem = qmc2LastSoftwareListItem = 0;
		return;
	}

	// paranoia :)
	QTreeWidgetItem *ci = treeWidgetMachineList->currentItem();
	if ( !ci )
		return;
	if ( !ci->isSelected() )
		return;
	while ( ci->parent() )
		ci = ci->parent();
	qmc2CurrentItem = ci;
	if ( qmc2CurrentItem->childCount() <= 0 )
		return;

	// show / hide machine status indicator
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/MachineStatusIndicator").toBool() ) {
		if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/MachineStatusIndicatorOnlyWhenRequired").toBool() ) {
			if ( hSplitter->sizes().first() == 0 || tabWidgetMachineList->indexOf(tabMachineList) != tabWidgetMachineList->currentIndex() )
				labelMachineStatus->setVisible(true);
			else
				labelMachineStatus->setVisible(false);
		} else
			labelMachineStatus->setVisible(true);
	} else
		labelMachineStatus->setVisible(false);

	QString machineName(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_NAME));
	if ( !m_ignoreSelectionChange )
		emit selectionChanged(machineName);
	else foreach (MachineListViewer *v, machineListViewers)
		if ( v != m_lastMlvSender )
			v->mainSelectionChanged(machineName);
	m_ignoreSelectionChange = false;
	m_lastMlvSender = 0;

	// setup status indicator color
	switch ( qmc2MachineList->romState(machineName) ) {
		case 'C':
			labelMachineStatus->setPalette(qmc2StatusColorGreen);
			break;
		case 'M':
			labelMachineStatus->setPalette(qmc2StatusColorYellowGreen);
			break;
		case 'I':
			labelMachineStatus->setPalette(qmc2StatusColorRed);
			break;
		case 'N':
			labelMachineStatus->setPalette(qmc2StatusColorGrey);
			break;
		case 'U':
		default:
			labelMachineStatus->setPalette(qmc2StatusColorBlue);
			break;
	}

	// 'special widgets': switch back to the default page if applicable
	switch ( componentInfo->appliedFeatureList().at(tabWidgetMachineDetail->currentIndex()) )
	{
		case QMC2_SOFTWARE_LIST_INDEX:
			if ( qmc2SoftwareList && qmc2CurrentItem == qmc2LastSoftwareListItem )
				qmc2SoftwareList->on_toolButtonToggleSoftwareInfo_clicked(qmc2SoftwareList->toolButtonToggleSoftwareInfo->isChecked());
			else
				stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);
			break;

		default:
			stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);
			break;
	}

#if defined(QMC2_YOUTUBE_ENABLED)
	// depending on the codec, an unused YT widget may still cause load on the system, so we destroy it when it's no longer required...
	if ( componentInfo->appliedFeatureList().at(currentIndex) != QMC2_YOUTUBE_INDEX ) {
		if ( qmc2YouTubeWidget && qmc2CurrentItem != qmc2LastYouTubeItem ) {
			qmc2YouTubeWidget->saveSettings();
			qmc2YouTubeWidget->forcedExit = true;
			if ( qmc2YouTubeWidget->isPlaying() || qmc2YouTubeWidget->isPaused() )
				qmc2YouTubeWidget->stop();
			QLayout *vbl = tabYouTube->layout();
			if ( vbl )
				delete vbl;
			qmc2YouTubeWidget->close();
			delete qmc2YouTubeWidget;
			qmc2YouTubeWidget = 0;
			qmc2LastYouTubeItem = 0;
		}
	}
#endif

	if ( componentInfo->appliedFeatureList().at(currentIndex) != QMC2_SYSTEM_NOTES_INDEX ) {
		if ( qmc2SystemNotesEditor ) {
			qmc2SystemNotesEditor->hideTearOffMenus();
			qmc2SystemNotesEditor->hide();
		}
	} else if ( qmc2SystemNotesEditor ) {
		qmc2SystemNotesEditor->show();
		qmc2SystemNotesEditor->raise();
	}

	qmc2UseDefaultEmulator = qmc2Config->value(QString(QMC2_EMULATOR_PREFIX + "Configuration/%1/SelectedEmulator").arg(machineName), tr("Default")).toString() == tr("Default");

	int left, top, right, bottom;
	switch ( componentInfo->appliedFeatureList().at(currentIndex) ) {
#if defined(QMC2_YOUTUBE_ENABLED)
		case QMC2_YOUTUBE_INDEX:
			if ( qmc2YouTubeVideoInfoHash.isEmpty() )
				loadYouTubeVideoInfoMap();
			if ( qmc2CurrentItem != qmc2LastYouTubeItem ) {
				tabYouTube->setUpdatesEnabled(false);
				if ( qmc2YouTubeWidget ) {
					qmc2YouTubeWidget->stop();
					qmc2YouTubeWidget->saveSettings();
					qmc2YouTubeWidget->forcedExit = true;
					if ( qmc2YouTubeWidget->isPlaying() || qmc2YouTubeWidget->isPaused() )
						qmc2YouTubeWidget->stop();
					QLayout *vbl = tabYouTube->layout();
					if ( vbl )
						delete vbl;
					qmc2YouTubeWidget->close();
					qmc2YouTubeWidget->deleteLater();
					qmc2YouTubeWidget = 0;
				}
				gridLayout->getContentsMargins(&left, &top, &right, &bottom);
				QVBoxLayout *layout = new QVBoxLayout;
				layout->setContentsMargins(left, top, right, bottom);
				QString setID = qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_NAME);
				qmc2YouTubeWidget = new YouTubeVideoPlayer(setID, qmc2MachineListItemHash.value(setID)->text(QMC2_MACHINELIST_COLUMN_MACHINE), tabYouTube);
				layout->addWidget(qmc2YouTubeWidget);
				tabYouTube->setLayout(layout);
				qmc2YouTubeWidget->show();
				qmc2LastYouTubeItem = qmc2CurrentItem;
				tabYouTube->setUpdatesEnabled(true);
			}
			break;
#endif

		case QMC2_SOFTWARE_LIST_INDEX:
#if defined(QMC2_YOUTUBE_ENABLED)
			if ( qmc2YouTubeWidget )
				qmc2YouTubeWidget->clearMessage();
#endif
			if ( qmc2SoftwareList ) {
				if ( qmc2SoftwareList->isInitialLoad ) {
					QTimer::singleShot(0, this, SLOT(softwareLoadInterrupted()));
					return;
				}
				if ( qmc2SoftwareList->isLoading || isCreatingSoftList ) {
					qmc2SoftwareList->interruptLoad = true;
					qmc2LastSoftwareListItem = 0;
					QTimer::singleShot(0, this, SLOT(softwareLoadInterrupted()));
					return;
				}
			}
			if ( qmc2CurrentItem != qmc2LastSoftwareListItem && !isCreatingSoftList ) {
				tabWidgetMachineDetail->setMovable(false);
				QTreeWidgetItem *ci = qmc2CurrentItem;
				isCreatingSoftList = true;
				tabSoftwareList->setUpdatesEnabled(false);
				if ( qmc2SoftwareList ) {
					qmc2ShortcutHash["F10"].second = 0;
					qmc2SoftwareList->save();
					QLayout *vbl = tabSoftwareList->layout();
					if ( vbl )
						delete vbl;
					delete qmc2SoftwareList;
					qmc2SoftwareList = 0;
				}
				gridLayout->getContentsMargins(&left, &top, &right, &bottom);
				QVBoxLayout *layout = new QVBoxLayout;
				layout->setContentsMargins(left, top, right, bottom);
				qmc2SoftwareList = new SoftwareList(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_NAME), tabSoftwareList);
				layout->addWidget(qmc2SoftwareList);
				tabSoftwareList->setLayout(layout);
				qmc2SoftwareList->show();
				tabSoftwareList->setUpdatesEnabled(true);
				isCreatingSoftList = false;
				tabWidgetMachineDetail->setMovable(true);
				qmc2SoftwareList->load();
				qmc2LastSoftwareListItem = ci;
			}
			break;

		case QMC2_DEVICE_INDEX:
#if defined(QMC2_YOUTUBE_ENABLED)
			if ( qmc2YouTubeWidget )
				qmc2YouTubeWidget->clearMessage();
#endif
			if ( qmc2CurrentItem != qmc2LastDeviceConfigItem ) {
				if ( qmc2DeviceConfigurator ) {
					tabDevices->setUpdatesEnabled(false);
					qmc2DeviceConfigurator->forceQuit = true;
					if ( qmc2DeviceConfigurator->refreshRunning ) {
						QTimer::singleShot(1, this, SLOT(treeWidgetMachineList_itemSelectionChanged_delayed()));
						return;
					}
					qmc2DeviceConfigurator->disconnect();
					qmc2DeviceConfigurator->save();
					qmc2DeviceConfigurator->saveSetup();
					QLayout *vbl = tabDevices->layout();
					if ( vbl )
						delete vbl;
					qmc2DeviceConfigurator->deleteLater();
					qmc2DeviceConfigurator = 0;
				}
				gridLayout->getContentsMargins(&left, &top, &right, &bottom);
				QVBoxLayout *layout = new QVBoxLayout;
				layout->setContentsMargins(left, top, right, bottom);
				qmc2DeviceConfigurator = new DeviceConfigurator(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_NAME), tabDevices);
				connect(&messDevCfgTimer, SIGNAL(timeout()), qmc2DeviceConfigurator, SLOT(load()));
				layout->addWidget(qmc2DeviceConfigurator);
				if ( !tabDevices->layout() )
					tabDevices->setLayout(layout);
				qmc2DeviceConfigurator->show();
				qmc2LastDeviceConfigItem = qmc2CurrentItem;
				messDevCfgTimer.start(QMC2_DEVCONFIG_LOAD_DELAY);
				tabDevices->setUpdatesEnabled(true);
			}
			break;

		case QMC2_PROJECTMESS_INDEX:
#if defined(QMC2_YOUTUBE_ENABLED)
			if ( qmc2YouTubeWidget )
				qmc2YouTubeWidget->clearMessage();
#endif
			if ( qmc2CurrentItem != qmc2LastProjectMESSItem ) {
				tabProjectMESS->setUpdatesEnabled(false);
				if ( qmc2ProjectMESSLookup ) {
					QLayout *vbl = tabProjectMESS->layout();
					if ( vbl )
						delete vbl;
					qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ProjectMESS/Zoom", qmc2ProjectMESSLookup->spinBoxZoom->value());
					delete qmc2ProjectMESSLookup;
					qmc2ProjectMESSLookup = 0;
				}
				gridLayout->getContentsMargins(&left, &top, &right, &bottom);
				QVBoxLayout *layout = new QVBoxLayout;
				layout->setContentsMargins(left, top, right, bottom);
				qmc2ProjectMESSLookup = new MiniWebBrowser(tabProjectMESS);
				qmc2ProjectMESSLookup->spinBoxZoom->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ProjectMESS/Zoom", 100).toInt());
				layout->addWidget(qmc2ProjectMESSLookup);
				tabProjectMESS->setLayout(layout);
				QString projectMessUrl;
				QColor color = qmc2ProjectMESSLookup->webViewBrowser->palette().color(QPalette::WindowText);
				QString machName = qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_NAME);
				qmc2ProjectMESSLookup->webViewBrowser->setStatusTip(tr("ProjectMESS page for system '%1'").arg(machName));
				if ( !qmc2ProjectMESSCache.contains(machName) ) {
					projectMessUrl = QString(QMC2_PROJECTMESS_PATTERN_URL).arg(machName);
					qmc2ProjectMESSLookup->webViewBrowser->setHtml(
							QString("<html><head></head><body><center><p><font color=\"#%1%2%3\"<b>").arg(color.red()).arg(color.green()).arg(color.blue()) +
							tr("Fetching ProjectMESS page for system '%1', please wait...").arg(machName) +
							"</font></b></p><p>" + QString("(<a href=\"%1\">%1</a>)").arg(projectMessUrl) + "</p></center></body></html>",
							QUrl(projectMessUrl));
					qmc2ProjectMESSLookup->webViewBrowser->load(QUrl(projectMessUrl));
				} else {
					projectMessUrl = QString(QMC2_PROJECTMESS_PATTERN_URL).arg(machName);
					qmc2ProjectMESSLookup->webViewBrowser->setHtml(QString(QMC2_UNCOMPRESS(*qmc2ProjectMESSCache[machName])), QUrl(projectMessUrl));
					qmc2ProjectMESSLookup->webViewBrowser->load(QUrl(projectMessUrl));
				}
				qmc2LastProjectMESSItem = qmc2CurrentItem;
				connect(qmc2ProjectMESSLookup->webViewBrowser, SIGNAL(loadFinished(bool)), this, SLOT(projectMessSystemLoadFinished(bool)));
				connect(qmc2ProjectMESSLookup->webViewBrowser, SIGNAL(loadStarted()), this, SLOT(projectMessSystemLoadStarted()));
				tabProjectMESS->setUpdatesEnabled(true);
			}
			break;

		case QMC2_CONFIG_INDEX:
#if defined(QMC2_YOUTUBE_ENABLED)
			if ( qmc2YouTubeWidget )
				qmc2YouTubeWidget->clearMessage();
#endif
			if ( qmc2CurrentItem != qmc2LastConfigItem ) {
				QWidget *configWidget = componentInfo->widget(QMC2_CONFIG_INDEX);

				configWidget->setUpdatesEnabled(false);

				// save & cleanup existing game/machine specific emulator settings
				QString selectedEmulator;
				if ( qmc2EmulatorOptions ) {
					qmc2EmulatorOptions->save();
					selectedEmulator = comboBoxEmuSelector->currentText();
					if ( selectedEmulator == tr("Default") || selectedEmulator.isEmpty() )
						qmc2Config->remove(qmc2EmulatorOptions->settingsGroup + "/SelectedEmulator");
					else
						qmc2Config->setValue(qmc2EmulatorOptions->settingsGroup + "/SelectedEmulator", selectedEmulator);
					QLayout *vbl = configWidget->layout();
					if ( vbl )
						delete vbl;
					delete labelEmuSelector;
					delete comboBoxEmuSelector;
					comboBoxEmuSelector = 0;
					delete qmc2EmulatorOptions;
					delete pushButtonCurrentEmulatorOptionsExportToFile;
					delete pushButtonCurrentEmulatorOptionsImportFromFile;
					qmc2EmulatorOptions = 0;
				}
				gridLayout->getContentsMargins(&left, &top, &right, &bottom);
				QVBoxLayout *layout = new QVBoxLayout;
				layout->setContentsMargins(left, top, right, bottom);
				configWidget->setLayout(layout);

				// emulator selector (default, or one of the registered emulators)
				labelEmuSelector = new QLabel(tr("Emulator for this machine") + ":", configWidget);
				labelEmuSelector->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
				comboBoxEmuSelector = new QComboBox(configWidget);
				comboBoxEmuSelector->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
				qmc2Config->beginGroup(QMC2_EMULATOR_PREFIX + "RegisteredEmulators");
				QStringList registeredEmulators = qmc2Config->childGroups();
				qmc2Config->endGroup();
				registeredEmulators.prepend(tr("Default"));
				comboBoxEmuSelector->insertItems(0, registeredEmulators);
				QHBoxLayout *emuSelectorLayout = new QHBoxLayout();
				emuSelectorLayout->addWidget(labelEmuSelector);
				emuSelectorLayout->addWidget(comboBoxEmuSelector);
				layout->addLayout(emuSelectorLayout);
				connect(comboBoxEmuSelector, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(emuSelector_currentIndexChanged(const QString &)));

				// emulator options
				qmc2EmulatorOptions = new EmulatorOptions(QMC2_EMULATOR_PREFIX + "Configuration/" + machineName, configWidget);
				qmc2EmulatorOptions->load();

				QString defaultChoice;
				QStringList biosSets(getXmlChoices(machineName, "biosset", "name", &defaultChoice));
				QStringList biosDescriptions(getXmlChoices(machineName, "biosset", "description"));
				for (int i = 0; i < biosSets.count(); i++) {
					biosDescriptions[i] = biosSets[i] + " - " + biosDescriptions[i];
					if ( biosSets[i] == defaultChoice )
						biosDescriptions[i] += " / " + tr("default");
				}
				qmc2EmulatorOptions->addChoices("bios", biosSets, biosDescriptions, defaultChoice);

				QStringList ramSizes(getXmlChoices(machineName, "ramoption", QString(), &defaultChoice));
				std::sort(ramSizes.begin(), ramSizes.end(), MainWindow::qStringListLessThan);
				QStringList humanReadableRamSizes;
				for (int i = 0; i < ramSizes.count(); i++) {
					if ( ramSizes[i] == defaultChoice )
						humanReadableRamSizes << ROMAlyzer::humanReadable(ramSizes[i].toULongLong(), 0) + " / " + tr("default");
					else
						humanReadableRamSizes << ROMAlyzer::humanReadable(ramSizes[i].toULongLong(), 0);
				}
				qmc2EmulatorOptions->addChoices("ramsize", ramSizes, humanReadableRamSizes, defaultChoice, false);

				layout->addWidget(qmc2EmulatorOptions);

				// import/export buttons
				QHBoxLayout *buttonLayout = new QHBoxLayout();
				pushButtonCurrentEmulatorOptionsExportToFile = new QPushButton(tr("Export to..."), this);
				pushButtonCurrentEmulatorOptionsExportToFile->setToolTip(QObject::tr("Export machine-specific MAME configuration"));
				pushButtonCurrentEmulatorOptionsExportToFile->setStatusTip(QObject::tr("Export machine-specific MAME configuration"));
				pushButtonCurrentEmulatorOptionsImportFromFile = new QPushButton(QObject::tr("Import from..."), this);
				pushButtonCurrentEmulatorOptionsImportFromFile->setToolTip(QObject::tr("Import machine-specific MAME configuration"));
				pushButtonCurrentEmulatorOptionsImportFromFile->setStatusTip(QObject::tr("Import machine-specific MAME configuration"));
				buttonLayout->addWidget(pushButtonCurrentEmulatorOptionsExportToFile);
				buttonLayout->addWidget(pushButtonCurrentEmulatorOptionsImportFromFile);
				layout->addLayout(buttonLayout);

				// import/export menus
				qmc2MainWindow->selectMenuCurrentEmulatorOptionsExportToFile = new QMenu(qmc2MainWindow->pushButtonCurrentEmulatorOptionsExportToFile);
				connect(qmc2MainWindow->selectMenuCurrentEmulatorOptionsExportToFile->addAction(QIcon(QString::fromUtf8(":/data/img/work.png")), tr("<inipath>/%1.ini").arg(machineName)), SIGNAL(triggered()), qmc2MainWindow, SLOT(pushButtonCurrentEmulatorOptionsExportToFile_clicked()));
				connect(qmc2MainWindow->selectMenuCurrentEmulatorOptionsExportToFile->addAction(QIcon(QString::fromUtf8(":/data/img/fileopen.png")), tr("Select file...")), SIGNAL(triggered()), qmc2MainWindow, SLOT(pushButtonCurrentEmulatorOptionsSelectExportFile_clicked()));
				qmc2MainWindow->pushButtonCurrentEmulatorOptionsExportToFile->setMenu(qmc2MainWindow->selectMenuCurrentEmulatorOptionsExportToFile);
				qmc2MainWindow->selectMenuCurrentEmulatorOptionsImportFromFile = new QMenu(qmc2MainWindow->pushButtonCurrentEmulatorOptionsImportFromFile);
				connect(qmc2MainWindow->selectMenuCurrentEmulatorOptionsImportFromFile->addAction(QIcon(QString::fromUtf8(":/data/img/work.png")), tr("<inipath>/%1.ini").arg(machineName)), SIGNAL(triggered()), qmc2MainWindow, SLOT(pushButtonCurrentEmulatorOptionsImportFromFile_clicked()));
				connect(qmc2MainWindow->selectMenuCurrentEmulatorOptionsImportFromFile->addAction(QIcon(QString::fromUtf8(":/data/img/fileopen.png")), tr("Select file...")), SIGNAL(triggered()), qmc2MainWindow, SLOT(pushButtonCurrentEmulatorOptionsSelectImportFile_clicked()));
				qmc2MainWindow->pushButtonCurrentEmulatorOptionsImportFromFile->setMenu(qmc2MainWindow->selectMenuCurrentEmulatorOptionsImportFromFile);
				qmc2LastConfigItem = qmc2CurrentItem;

				// select the emulator to be used, if applicable
				if ( registeredEmulators.count() > 1 ) {
					if ( qmc2Config->contains(qmc2EmulatorOptions->settingsGroup + "/SelectedEmulator") ) {
						selectedEmulator = qmc2Config->value(qmc2EmulatorOptions->settingsGroup + "/SelectedEmulator").toString();
						if ( !selectedEmulator.isEmpty() && registeredEmulators.contains(selectedEmulator) ) {
							int emuIndex = comboBoxEmuSelector->findText(selectedEmulator);
							if ( emuIndex >= 0 )
								comboBoxEmuSelector->setCurrentIndex(emuIndex);
							else
								comboBoxEmuSelector->setCurrentIndex(0);
						}
					}
				}

				// finally show the widgets...
				labelEmuSelector->show();
				comboBoxEmuSelector->show();
				qmc2EmulatorOptions->show();
				pushButtonCurrentEmulatorOptionsExportToFile->show();
				pushButtonCurrentEmulatorOptionsImportFromFile->show();

				configWidget->setUpdatesEnabled(true);
				configWidget->update();

				qmc2EmulatorOptions->horizontalScrollBar()->setSliderPosition(qmc2EmulatorOptions->horizontalScrollPosition);
				qmc2EmulatorOptions->verticalScrollBar()->setSliderPosition(qmc2EmulatorOptions->verticalScrollPosition);
			}
			break;

		case QMC2_MACHINEINFO_INDEX:
#if defined(QMC2_YOUTUBE_ENABLED)
			if ( qmc2YouTubeWidget )
				qmc2YouTubeWidget->clearMessage();
#endif
			if ( qmc2CurrentItem != qmc2LastMachineInfoItem ) {
				tabGameInfo->setUpdatesEnabled(false);
				QString machineInfoKey(machineName);
				if ( !qmc2MachineList->datInfoDb()->existsMachineInfo(machineInfoKey) ) {
					machineInfoKey = qmc2ParentHash.value(machineName);
					if ( !qmc2MachineList->datInfoDb()->existsMachineInfo(machineInfoKey) )
						machineInfoKey.clear();
				}
				if ( !machineInfoKey.isEmpty() ) {
					QString gameInfoText = qmc2MachineList->datInfoDb()->machineInfo(machineInfoKey);
					if ( !gameInfoText.isEmpty() ) {
						QString emulator = qmc2MachineList->datInfoDb()->machineInfoEmulator(machineInfoKey);
						if ( emulator == "MESS" )
							textBrowserMachineInfo->setHtml(messWikiToHtml(gameInfoText));
						else
							textBrowserMachineInfo->setHtml(gameInfoText.replace(QRegExp(QString("((http|https|ftp)://%1)").arg(urlSectionRegExp)), QLatin1String("<a href=\"\\1\">\\1</a>")));
					} else
						textBrowserMachineInfo->setHtml("<h2>" + qmc2MachineListItemHash.value(machineName)->text(QMC2_MACHINELIST_COLUMN_MACHINE) + "</h2>" + tr("<p>No data available</p>"));
				} else
					textBrowserMachineInfo->setHtml("<h2>" + qmc2MachineListItemHash.value(machineName)->text(QMC2_MACHINELIST_COLUMN_MACHINE) + "</h2>" + tr("<p>No data available</p>"));
				qmc2LastMachineInfoItem = qmc2CurrentItem;
				tabGameInfo->setUpdatesEnabled(true);
			}
			break;

		case QMC2_EMUINFO_INDEX:
#if defined(QMC2_YOUTUBE_ENABLED)
			if ( qmc2YouTubeWidget )
				qmc2YouTubeWidget->clearMessage();
#endif
			if ( qmc2CurrentItem != qmc2LastEmuInfoItem ) {
				tabEmuInfo->setUpdatesEnabled(false);
				QString emuInfoKey(machineName);
				if ( !qmc2MachineList->datInfoDb()->existsEmuInfo(emuInfoKey) ) {
					emuInfoKey = qmc2ParentHash.value(machineName);
					if ( !qmc2MachineList->datInfoDb()->existsEmuInfo(emuInfoKey) )
						emuInfoKey.clear();
				}
				if ( !emuInfoKey.isEmpty() ) {
					QString emuInfoText = qmc2MachineList->datInfoDb()->emuInfo(emuInfoKey);
					if ( !emuInfoText.isEmpty() )
						textBrowserEmuInfo->setHtml(emuInfoText.replace(QRegExp(QString("(\\w+://%1)").arg(urlSectionRegExp)), QLatin1String("<a href=\"\\1\">\\1</a>")));
					else
						textBrowserEmuInfo->setHtml(tr("No data available"));
				} else
					textBrowserEmuInfo->setHtml(tr("No data available"));
				qmc2LastEmuInfoItem = qmc2CurrentItem;
				tabEmuInfo->setUpdatesEnabled(true);
			}
			break;

		case QMC2_SYSTEM_NOTES_INDEX:
#if defined(QMC2_YOUTUBE_ENABLED)
			if ( qmc2YouTubeWidget )
				qmc2YouTubeWidget->clearMessage();
#endif
			if ( qmc2CurrentItem != qmc2LastSystemNotesItem ) {
				qmc2LastSystemNotesItem = qmc2CurrentItem;
				if ( !qmc2SystemNotesEditor ) {
					int tabIndex = tabWidgetMachineDetail->indexOf(componentInfo->widget(QMC2_SYSTEM_NOTES_INDEX));
					tabWidgetMachineDetail->setUpdatesEnabled(false);
					tabWidgetMachineDetail->removeTab(tabIndex);
					qmc2SystemNotesEditor = new HtmlEditor("SystemNotes", true);
					tabWidgetMachineDetail->insertTab(tabIndex, qmc2SystemNotesEditor, QIcon(QString::fromUtf8(":/data/img/notes.png")), tr("&Notes"));
					tabWidgetMachineDetail->setCurrentIndex(tabIndex);
					tabWidgetMachineDetail->setUpdatesEnabled(true);
					qmc2SystemNotesEditor->move(0, 0);
					qmc2SystemNotesEditor->resize(qmc2SystemNotesEditor->parentWidget()->size());
					qmc2SystemNotesEditor->show();
					qmc2SystemNotesEditor->raise();
				} else {
					qmc2SystemNotesEditor->save();
					qmc2SystemNotesEditor->loadedContent.clear();
					qmc2SystemNotesEditor->checkRevertStatus();
				}

				QString systemNotesFolder = qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SystemNotesFolder").toString();
				QString systemNotesTemplate = qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SystemNotesTemplate").toString();
				bool useSystemNotesTemplate = qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/UseSystemNotesTemplate").toBool();
				QString fileName = systemNotesFolder + machineName + ".html";
				qmc2SystemNotesEditor->setCurrentFileName(fileName);
				QString parentSystem = qmc2ParentHash.value(machineName);

				qmc2SystemNotesEditor->enableFileNewFromTemplateAction(useSystemNotesTemplate);

				qmc2SystemNotesEditor->templateMap.clear();
#if QT_VERSION < 0x050000
				qmc2SystemNotesEditor->templateMap["$DESCRIPTION$"] = Qt::escape(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_MACHINE));
				qmc2SystemNotesEditor->templateMap["$MANUFACTURER$"] = Qt::escape(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_MANU));
				qmc2SystemNotesEditor->templateMap["$YEAR$"] = Qt::escape(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_YEAR));
				qmc2SystemNotesEditor->templateMap["$CATEGORY$"] = Qt::escape(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_CATEGORY));
#else
				qmc2SystemNotesEditor->templateMap["$DESCRIPTION$"] = qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_MACHINE).toHtmlEscaped();
				qmc2SystemNotesEditor->templateMap["$MANUFACTURER$"] = qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_MANU).toHtmlEscaped();
				qmc2SystemNotesEditor->templateMap["$YEAR$"] = qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_YEAR).toHtmlEscaped();
				qmc2SystemNotesEditor->templateMap["$CATEGORY$"] = qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_CATEGORY).toHtmlEscaped();
#endif
				qmc2SystemNotesEditor->templateMap["$ID$"] = machineName;
				qmc2SystemNotesEditor->templateMap["$PARENT_ID$"] = parentSystem;
#if QT_VERSION < 0x050000
				qmc2SystemNotesEditor->templateMap["$VERSION$"] = Qt::escape(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_VERSION));
#else
				qmc2SystemNotesEditor->templateMap["$VERSION$"] = qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_VERSION).toHtmlEscaped();
#endif
				qmc2SystemNotesEditor->templateMap["$PLAYERS$"] = qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_PLAYERS);
				qmc2SystemNotesEditor->templateMap["$ROM_TYPES$"] = qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_RTYPES);
				qmc2SystemNotesEditor->templateMap["$DRIVER_STATUS$"] = qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_DRVSTAT);
				qmc2SystemNotesEditor->templateMap["$DRIVER_STATUS_UT$"] = MachineList::reverseTranslations.value(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_DRVSTAT));
				qmc2SystemNotesEditor->templateMap["$ROM_STATUS$"] = qmc2MachineList->romStatus(machineName, true);
				qmc2SystemNotesEditor->templateMap["$ROM_STATUS_UT$"] = qmc2MachineList->romStatus(machineName, false);
				qmc2SystemNotesEditor->templateMap["$IS_BIOS$"] = qmc2MachineList->isBios(machineName) ? "true" : "false";
				qmc2SystemNotesEditor->templateMap["$IS_DEVICE$"] = qmc2MachineList->isDevice(machineName) ? "true" : "false";
				qmc2SystemNotesEditor->templateMap["$GUI_LANGUAGE$"] = qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Language", "us").toString();
				qmc2SystemNotesEditor->templateMap["$EMULATOR_VARIANT$"] = QMC2_EMU_NAME_VARIANT;
				qmc2SystemNotesEditor->templateMap["$EMULATOR_TYPE$"] = QMC2_EMU_NAME;
				qmc2SystemNotesEditor->templateMap["$SOURCE_FILE$"] = qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_SRCFILE);
				QString filePath;
				QDir dataDir(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/DataDirectory").toString());
				QString ghostPath(QDir::fromNativeSeparators(dataDir.absolutePath() + "/img/ghost.png"));
#if defined(QMC2_OS_WIN)
				qmc2SystemNotesEditor->templateMap["$GHOST_IMAGE$"] = "file:///" + ghostPath;
#else
				qmc2SystemNotesEditor->templateMap["$GHOST_IMAGE$"] = "file://" + ghostPath;
#endif
				QString ghostVideoPath = QDir::fromNativeSeparators(dataDir.absolutePath() + "/img/ghost_video.png");
#if defined(QMC2_OS_WIN)
				qmc2SystemNotesEditor->templateMap["$GHOST_VIDEO$"] = "file:///" + ghostVideoPath;
#else
				qmc2SystemNotesEditor->templateMap["$GHOST_VIDEO$"] = "file://" + ghostVideoPath;
#endif
				if ( qmc2Preview ) {
#if defined(QMC2_OS_WIN)
					if ( qmc2Preview->loadImage(machineName, machineName, true, &filePath, false) )
						qmc2SystemNotesEditor->templateMap["$PREVIEW_IMAGE$"] = "file:///" + QDir::fromNativeSeparators(filePath);
					else
						qmc2SystemNotesEditor->templateMap["$PREVIEW_IMAGE$"] = "file:///" + ghostPath;
#else
					if ( qmc2Preview->loadImage(machineName, machineName, true, &filePath, false) )
						qmc2SystemNotesEditor->templateMap["$PREVIEW_IMAGE$"] = "file://" + QDir::fromNativeSeparators(filePath);
					else
						qmc2SystemNotesEditor->templateMap["$PREVIEW_IMAGE$"] = "file://" + ghostPath;
#endif
				}
				if ( qmc2Flyer ) {
#if defined(QMC2_OS_WIN)
					if ( qmc2Flyer->loadImage(machineName, machineName, true, &filePath, false) )
						qmc2SystemNotesEditor->templateMap["$FLYER_IMAGE$"] = "file:///" + QDir::fromNativeSeparators(filePath);
					else
						qmc2SystemNotesEditor->templateMap["$FLYER_IMAGE$"] = "file:///" + ghostPath;
#else
					if ( qmc2Flyer->loadImage(machineName, machineName, true, &filePath, false) )
						qmc2SystemNotesEditor->templateMap["$FLYER_IMAGE$"] = "file://" + QDir::fromNativeSeparators(filePath);
					else
						qmc2SystemNotesEditor->templateMap["$FLYER_IMAGE$"] = "file://" + ghostPath;
#endif
				}
				if ( qmc2Cabinet ) {
#if defined(QMC2_OS_WIN)
					if ( qmc2Cabinet->loadImage(machineName, machineName, true, &filePath, false) )
						qmc2SystemNotesEditor->templateMap["$CABINET_IMAGE$"] = "file:///" + QDir::fromNativeSeparators(filePath);
					else
						qmc2SystemNotesEditor->templateMap["$CABINET_IMAGE$"] = "file:///" + ghostPath;
#else
					if ( qmc2Cabinet->loadImage(machineName, machineName, true, &filePath, false) )
						qmc2SystemNotesEditor->templateMap["$CABINET_IMAGE$"] = "file://" + QDir::fromNativeSeparators(filePath);
					else
						qmc2SystemNotesEditor->templateMap["$CABINET_IMAGE$"] = "file://" + ghostPath;
#endif
				}
				if ( qmc2Controller ) {
#if defined(QMC2_OS_WIN)
					if ( qmc2Controller->loadImage(machineName, machineName, true, &filePath, false) )
						qmc2SystemNotesEditor->templateMap["$CONTROLLER_IMAGE$"] = "file:///" + QDir::fromNativeSeparators(filePath);
					else
						qmc2SystemNotesEditor->templateMap["$CONTROLLER_IMAGE$"] = "file:///" + ghostPath;
#else
					if ( qmc2Controller->loadImage(machineName, machineName, true, &filePath, false) )
						qmc2SystemNotesEditor->templateMap["$CONTROLLER_IMAGE$"] = "file://" + QDir::fromNativeSeparators(filePath);
					else
						qmc2SystemNotesEditor->templateMap["$CONTROLLER_IMAGE$"] = "file://" + ghostPath;
#endif
				}
				if ( qmc2Marquee ) {
#if defined(QMC2_OS_WIN)
					if ( qmc2Marquee->loadImage(machineName, machineName, true, &filePath, false) )
						qmc2SystemNotesEditor->templateMap["$MARQUEE_IMAGE$"] = "file:///" + QDir::fromNativeSeparators(filePath);
					else
						qmc2SystemNotesEditor->templateMap["$MARQUEE_IMAGE$"] = "file:///" + ghostPath;
					qmc2SystemNotesEditor->templateMap["$LOGO_IMAGE$"] = qmc2SystemNotesEditor->templateMap["$MARQUEE_IMAGE$"];
#else
					if ( qmc2Marquee->loadImage(machineName, machineName, true, &filePath, false) )
						qmc2SystemNotesEditor->templateMap["$MARQUEE_IMAGE$"] = "file://" + QDir::fromNativeSeparators(filePath);
					else
						qmc2SystemNotesEditor->templateMap["$MARQUEE_IMAGE$"] = "file://" + ghostPath;
					qmc2SystemNotesEditor->templateMap["$LOGO_IMAGE$"] = qmc2SystemNotesEditor->templateMap["$MARQUEE_IMAGE$"];
#endif
				}
				if ( qmc2Title ) {
#if defined(QMC2_OS_WIN)
					if ( qmc2Title->loadImage(machineName, machineName, true, &filePath, false) )
						qmc2SystemNotesEditor->templateMap["$TITLE_IMAGE$"] = "file:///" + QDir::fromNativeSeparators(filePath);
					else
						qmc2SystemNotesEditor->templateMap["$TITLE_IMAGE$"] = "file:///" + ghostPath;
#else
					if ( qmc2Title->loadImage(machineName, machineName, true, &filePath, false) )
						qmc2SystemNotesEditor->templateMap["$TITLE_IMAGE$"] = "file://" + QDir::fromNativeSeparators(filePath);
					else
						qmc2SystemNotesEditor->templateMap["$TITLE_IMAGE$"] = "file://" + ghostPath;
#endif
				}
				if ( qmc2PCB ) {
#if defined(QMC2_OS_WIN)
					if ( qmc2PCB->loadImage(machineName, machineName, true, &filePath, false) )
						qmc2SystemNotesEditor->templateMap["$PCB_IMAGE$"] = "file:///" + QDir::fromNativeSeparators(filePath);
					else
						qmc2SystemNotesEditor->templateMap["$PCB_IMAGE$"] = "file:///" + ghostPath;
#else
					if ( qmc2PCB->loadImage(machineName, machineName, true, &filePath, false) )
						qmc2SystemNotesEditor->templateMap["$PCB_IMAGE$"] = "file://" + QDir::fromNativeSeparators(filePath);
					else
						qmc2SystemNotesEditor->templateMap["$PCB_IMAGE$"] = "file://" + ghostPath;
#endif
				}
				QString emuInfoKey(machineName);
				if ( !qmc2MachineList->datInfoDb()->existsEmuInfo(emuInfoKey) ) {
					emuInfoKey = parentSystem;
					if ( !qmc2MachineList->datInfoDb()->existsEmuInfo(emuInfoKey) )
						emuInfoKey.clear();
				}
				if ( !emuInfoKey.isEmpty() ) {
					QString emuInfoText = qmc2MachineList->datInfoDb()->emuInfo(emuInfoKey);
					if ( !emuInfoText.isEmpty() ) {
						qmc2SystemNotesEditor->templateMap["$EMU_INFO$"] = emuInfoText.replace(QRegExp(QString("(\\w+://%1)").arg(urlSectionRegExp)), QLatin1String("<a href=\"\\1\">\\1</a>"));
						qmc2SystemNotesEditor->templateMap["$EMU_INFO_STATUS$"] = "OK";
					} else {
						qmc2SystemNotesEditor->templateMap["$EMU_INFO$"] = tr("No data available");
						qmc2SystemNotesEditor->templateMap["$EMU_INFO_STATUS$"] = "NO_DATA";
					}
				} else {
					qmc2SystemNotesEditor->templateMap["$EMU_INFO$"] = tr("No data available");
					qmc2SystemNotesEditor->templateMap["$EMU_INFO_STATUS$"] = "NO_DATA";
				}
				QString videoSnapUrl;
				foreach (QString videoSnapFolder, qmc2Config->value("MAME/FilesAndDirectories/VideoSnapFolder", QMC2_DEFAULT_DATA_PATH + "/vdo/").toString().split(";", QString::SkipEmptyParts)) {
					foreach (QString formatExtension, videoSnapAllowedFormatExtensions) {
						QFileInfo fi(QDir::cleanPath(videoSnapFolder + "/" + machineName + formatExtension));
						if ( fi.exists() && fi.isReadable() ) {
							videoSnapUrl = fi.absoluteFilePath();
#if defined(QMC2_OS_WIN)
							videoSnapUrl.prepend("file:///");
#else
							videoSnapUrl.prepend("file://");
#endif
							break;
						}
					}
					if ( videoSnapUrl.isEmpty() ) { // parent fallback
						if ( qmc2ParentImageFallback && qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/VideoFallback", 0).toInt() == 0 ) {
							QString parentId = qmc2ParentHash.value(machineName);
							if ( !parentId.isEmpty() ) {
								foreach (QString formatExtension, videoSnapAllowedFormatExtensions) {
									QFileInfo fi(QDir::cleanPath(videoSnapFolder + "/" + parentId + formatExtension));
									if ( fi.exists() && fi.isReadable() ) {
										videoSnapUrl = fi.absoluteFilePath();
#if defined(QMC2_OS_WIN)
										videoSnapUrl.prepend("file:///");
#else
										videoSnapUrl.prepend("file://");
#endif
										break;
									}
								}
							}
						}
					}
				}
				qmc2SystemNotesEditor->templateMap["$VIDEO_SNAP_URL$"] = videoSnapUrl;
				QString machineInfoKey(machineName);
				if ( !qmc2MachineList->datInfoDb()->existsMachineInfo(machineInfoKey) ) {
					machineInfoKey = parentSystem;
					if ( !qmc2MachineList->datInfoDb()->existsMachineInfo(machineInfoKey) )
						machineInfoKey.clear();
				}
				if ( !machineInfoKey.isEmpty() ) {
					QString gameInfoText = qmc2MachineList->datInfoDb()->machineInfo(machineInfoKey);
					if ( !gameInfoText.isEmpty() ) {
						QString emulator = qmc2MachineList->datInfoDb()->machineInfoEmulator(machineInfoKey);
						if ( emulator == "MESS" )
							qmc2SystemNotesEditor->templateMap["$GAME_INFO$"] = messWikiToHtml(gameInfoText);
						else
							qmc2SystemNotesEditor->templateMap["$GAME_INFO$"] = gameInfoText.replace(QRegExp(QString("((http|https|ftp)://%1)").arg(urlSectionRegExp)), QLatin1String("<a href=\"\\1\">\\1</a>"));
						qmc2SystemNotesEditor->templateMap["$GAME_INFO_STATUS$"] = "OK";
					} else {
						qmc2SystemNotesEditor->templateMap["$GAME_INFO$"] = tr("No data available");
						qmc2SystemNotesEditor->templateMap["$GAME_INFO_STATUS$"] = "NO_DATA";
					}
				} else {
					qmc2SystemNotesEditor->templateMap["$GAME_INFO$"] = tr("No data available");
					qmc2SystemNotesEditor->templateMap["$GAME_INFO_STATUS$"] = "NO_DATA";
				}
				qmc2SystemNotesEditor->templateMap["$MACHINE_INFO$"] = qmc2SystemNotesEditor->templateMap["$GAME_INFO$"];
				qmc2SystemNotesEditor->templateMap["$MACHINE_INFO_STATUS$"] = qmc2SystemNotesEditor->templateMap["$GAME_INFO_STATUS$"];
				qmc2SystemNotesEditor->setCurrentTemplateName(systemNotesTemplate);
				qmc2SystemNotesEditor->stopLoading = true;
				if ( QFile::exists(fileName) )
					QTimer::singleShot(25, qmc2SystemNotesEditor, SLOT(loadCurrent()));
				else {
					if ( useSystemNotesTemplate )
						QTimer::singleShot(25, qmc2SystemNotesEditor, SLOT(loadCurrentTemplate()));
					else
						qmc2SystemNotesEditor->fileNew();
				}

				qmc2SystemNotesEditor->setCurrentFileName(fileName);
			}
			break;

		default:
#if defined(QMC2_YOUTUBE_ENABLED)
			if ( qmc2YouTubeWidget )
				qmc2YouTubeWidget->clearMessage();
#endif
			// if local emulator options exits and they are no longer needed, close & destroy them...
			if ( qmc2EmulatorOptions ) {
				QWidget *configWidget = componentInfo->widget(QMC2_CONFIG_INDEX);
				QString selectedEmulator = comboBoxEmuSelector->currentText();
				if ( selectedEmulator == tr("Default") || selectedEmulator.isEmpty() )
					qmc2Config->remove(qmc2EmulatorOptions->settingsGroup + "/SelectedEmulator");
				else
					qmc2Config->setValue(qmc2EmulatorOptions->settingsGroup + "/SelectedEmulator", selectedEmulator);
				qmc2EmulatorOptions->save();
				QLayout *vbl = configWidget->layout();
				if ( vbl )
					delete vbl;
				delete labelEmuSelector;
				delete comboBoxEmuSelector;
				comboBoxEmuSelector = 0;
				delete qmc2EmulatorOptions;
				delete pushButtonCurrentEmulatorOptionsExportToFile;
				delete pushButtonCurrentEmulatorOptionsImportFromFile;
				qmc2EmulatorOptions = 0;
			}
			qmc2LastConfigItem = 0;
			break;
	}

	if ( initialCall ) {
		// this *forces* the tab-header to scroll, which it wouldn't do correctly otherwise when the current tab is far right (for whatever reason)
		QTabBar *tabBar = tabWidgetMachineDetail->findChild<QTabBar *>();
		if ( tabBar ) {
			tabWidgetMachineDetail->setUpdatesEnabled(false);
			tabBar->blockSignals(true);
			tabBar->setCurrentIndex(0);
			tabBar->setCurrentIndex(currentIndex);
			tabBar->blockSignals(false);
			tabWidgetMachineDetail->setUpdatesEnabled(true);
		}
	}
	initialCall = false;
}

bool MainWindow::qStringListLessThan(const QString &s1, const QString &s2)
{
	bool n1_ok, n2_ok;
       	quint64 n1 = s1.toULong(&n1_ok);
       	quint64 n2 = s2.toULong(&n2_ok);
	if ( n1_ok && n2_ok )
		return n1 < n2;
	else
		return s1.toLower() < s2.toLower();
}

QStringList &MainWindow::getXmlChoices(const QString &machineName, const QString &optionElement, const QString &optionAttribute, QString *defaultChoice)
{
	static QStringList xmlChoices;
	xmlChoices.clear();
	if ( machineName.isEmpty() )
		return xmlChoices;
	if ( defaultChoice )
		defaultChoice->clear();
	QStringList xmlLines(qmc2MachineList->xmlDb()->xml(machineName).split('\n', QString::SkipEmptyParts));
	QString defaultYes("default=\"yes\"");
	QString defaultOne("default=\"1\"");
	int i = 0;
	while ( i < xmlLines.count() ) {
		QString xmlLine(xmlLines.at(i++).simplified());
		int index = xmlLine.indexOf('<' + optionElement);
		if ( index >= 0 ) {
			if ( optionAttribute.isEmpty() ) {
				index = xmlLine.indexOf('>', index);
				if ( index >= 0 ) {
					xmlLine.remove(0, index + 1);
					bool isDefaultChoice = false;
					if ( defaultChoice && (xmlLine.indexOf(defaultYes) >= 0 || xmlLine.indexOf(defaultOne) >= 0 || defaultChoice->isEmpty()) )
						isDefaultChoice = true;
					xmlLine.replace("</" + optionElement + '>', "");
					QTextDocument doc;
					doc.setHtml(xmlLine);
					xmlLine = doc.toPlainText();
					xmlChoices << xmlLine;
					if ( isDefaultChoice )
						*defaultChoice = xmlLine;
				}
			} else {
				QString prefix(optionAttribute + "=\"");
				index = xmlLine.indexOf(prefix);
				if ( index >= 0 ) {
					xmlLine.remove(0, index + prefix.length());
					index = xmlLine.indexOf('\"');
					if ( index >= 0 ) {
						QTextDocument doc;
						doc.setHtml(xmlLine.left(index));
						QString choice = doc.toPlainText();
						xmlChoices << choice;
						if ( defaultChoice && (xmlLine.indexOf(defaultYes) >= 0 || xmlLine.indexOf(defaultOne) >= 0 || defaultChoice->isEmpty()) )
							*defaultChoice = choice;
					}
				}
			}
		}
	}
	return xmlChoices;
}

void MainWindow::emuSelector_currentIndexChanged(const QString &text)
{
	if ( !qmc2EmulatorOptions )
		return;

	if ( text == tr("Default") ) {
		qmc2UseDefaultEmulator = true;
		qmc2EmulatorOptions->setEnabled(true);
		if ( qmc2SoftwareList )
			qmc2SoftwareList->setEnabled(true);
		if ( qmc2DeviceConfigurator )
			qmc2DeviceConfigurator->setEnabled(true);
		pushButtonCurrentEmulatorOptionsExportToFile->setEnabled(true);
		pushButtonCurrentEmulatorOptionsImportFromFile->setEnabled(true);
	} else {
		qmc2UseDefaultEmulator = false;
		qmc2EmulatorOptions->setEnabled(false);
		if ( qmc2SoftwareList )
			qmc2SoftwareList->setEnabled(false);
		if ( qmc2DeviceConfigurator )
			qmc2DeviceConfigurator->setEnabled(false);
		pushButtonCurrentEmulatorOptionsExportToFile->setEnabled(false);
		pushButtonCurrentEmulatorOptionsImportFromFile->setEnabled(false);
	}

	if ( qmc2UseDefaultEmulator )
		qmc2Config->remove(QString(QMC2_EMULATOR_PREFIX + "Configuration/%1/SelectedEmulator").arg(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_NAME)));
	else
		qmc2Config->setValue(QString(QMC2_EMULATOR_PREFIX + "Configuration/%1/SelectedEmulator").arg(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_NAME)), text);
}

void MainWindow::on_treeWidgetMachineList_itemActivated(QTreeWidgetItem *item, int column)
{
	if ( qmc2DemoModeDialog )
		if ( qmc2DemoModeDialog->demoModeRunning )
			return;

	qmc2StartEmbedded = false;
	if ( !qmc2IgnoreItemActivation ) {
		switch ( qmc2DefaultLaunchMode ) {
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
			case QMC2_LAUNCH_MODE_EMBEDDED:
				on_actionPlayEmbedded_triggered();
				break;
#endif
			case QMC2_LAUNCH_MODE_INDEPENDENT:
			default:
				on_actionPlay_triggered();
				break;
		}
	}
	qmc2IgnoreItemActivation = false;
}

void MainWindow::on_treeWidgetHierarchy_itemActivated(QTreeWidgetItem *item, int column)
{
	if ( qmc2DemoModeDialog )
		if ( qmc2DemoModeDialog->demoModeRunning )
			return;

	qmc2StartEmbedded = false;
	if ( !qmc2IgnoreItemActivation ) {
		switch ( qmc2DefaultLaunchMode ) {
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
			case QMC2_LAUNCH_MODE_EMBEDDED:
				on_actionPlayEmbedded_triggered();
				break;
#endif
			case QMC2_LAUNCH_MODE_INDEPENDENT:
			default:
				on_actionPlay_triggered();
				break;
		}
	}
	qmc2IgnoreItemActivation = false;
}

void MainWindow::on_treeWidgetForeignIDs_itemActivated(QTreeWidgetItem *item, int /*column*/)
{
	if ( qmc2DemoModeDialog )
		if ( qmc2DemoModeDialog->demoModeRunning )
			return;

	qmc2StartEmbedded = false;
	if ( !qmc2IgnoreItemActivation ) {
		QStringList foreignInfo = item->whatsThis(0).split("\t");
		if ( foreignInfo.count() > 2 ) {
			// 0:emuName -- 1:id -- 2:description 
			foreignEmuName = foreignInfo[0];
			foreignID = foreignInfo[1];
			foreignDescription = foreignInfo[2];
			launchForeignID = true;
			qmc2StartEmbedded = false;
			switch ( qmc2DefaultLaunchMode ) {
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
				case QMC2_LAUNCH_MODE_EMBEDDED:
					QTimer::singleShot(0, this, SLOT(on_actionPlayEmbedded_triggered()));
					break;
#endif
				case QMC2_LAUNCH_MODE_INDEPENDENT:
				default:
					QTimer::singleShot(0, this, SLOT(on_actionPlay_triggered()));
					break;
			}
		}
	}
	qmc2IgnoreItemActivation = false;
}

void MainWindow::on_treeWidgetMachineList_itemDoubleClicked(QTreeWidgetItem * /*item*/, int /*column*/)
{
	if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "MachineList/DoubleClickActivation").toBool() )
		qmc2IgnoreItemActivation = true;
}

void MainWindow::on_treeWidgetHierarchy_itemDoubleClicked(QTreeWidgetItem * /*item*/, int /*column*/)
{
	if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "MachineList/DoubleClickActivation").toBool() )
		qmc2IgnoreItemActivation = true;
}

void MainWindow::on_treeWidgetForeignIDs_itemDoubleClicked(QTreeWidgetItem * /*item*/, int /*column*/)
{
	if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "MachineList/DoubleClickActivation").toBool() )
		qmc2IgnoreItemActivation = true;
}

void MainWindow::on_treeWidgetForeignIDs_customContextMenuRequested(const QPoint &p)
{
	QTreeWidgetItem *item = treeWidgetForeignIDs->itemAt(p);
	if ( item ) {
		treeWidgetForeignIDs->setItemSelected(item, true);
		qmc2ForeignIDsMenu->move(adjustedWidgetPosition(treeWidgetForeignIDs->viewport()->mapToGlobal(p), qmc2ForeignIDsMenu));
		qmc2ForeignIDsMenu->show();
	}
}

void MainWindow::machineListViewer_selectionChanged(const QString &id)
{
	m_lastMlvSender = (MachineListViewer *)sender();
	m_ignoreSelectionChange = true;
	qmc2CurrentItem = qmc2MachineListItemHash.value(id);
	treeWidgetMachineList->setCurrentItem(qmc2CurrentItem);
	scrollToCurrentItem();
}

void MainWindow::machineListViewer_tagChanged(const QString &id, bool tagged)
{
	QTreeWidgetItem *item;
	item = qmc2MachineListItemHash.value(id);
	Qt::CheckState cs = (tagged ? Qt::Checked : Qt::Unchecked);
	if ( item )
		item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, cs);
	item = qmc2HierarchyItemHash.value(id);
	if ( item )
		item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, cs);
	item = qmc2CategoryItemHash.value(id);
	if ( item )
		item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, cs);
	item = qmc2VersionItemHash.value(id);
	if ( item )
		item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, cs);
}

void MainWindow::on_treeWidgetMachineList_itemExpanded(QTreeWidgetItem *item)
{
	if ( !item )
		return;

	if ( qmc2ReloadActive ) {
		treeWidgetMachineList->collapseItem(item);
		log(QMC2_LOG_FRONTEND, tr("please wait for reload to finish and try again"));
		return;
	}

	if ( qmc2SortingActive ) {
		treeWidgetMachineList->collapseItem(item);
		log(QMC2_LOG_FRONTEND, tr("please wait for sorting to finish and try again"));
		return;
	}

	if ( item->child(0) ) {
		treeWidgetMachineList->viewport()->update();
		qApp->processEvents();
		if ( item->child(0)->text(QMC2_MACHINELIST_COLUMN_MACHINE) == MachineList::trWaitingForData ) {
			treeWidgetMachineList->viewport()->update();
			qmc2MachineList->parseMachineDetail(item);
			qmc2ExpandedMachineListItems << item;
		}
	}
}

void MainWindow::on_treeWidgetMachineList_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
	// workaround for a Qt bug: when POS1/Home is pressed, QTreeWidget & QTreeView don't correctly select the first VISIBLE item,
	// if the top item is HIDDEN
	if ( current ) {
		if ( qmc2CheckItemVisibility ) {
			if ( current->isHidden() ) {
				int i = treeWidgetMachineList->indexOfTopLevelItem(current);
				if ( i >= 0 ) {
					while ( current->isHidden() && i < treeWidgetMachineList->topLevelItemCount() ) {
						current = treeWidgetMachineList->topLevelItem(++i);
						if ( current == 0 ) 
							break;
					}
					if ( current )
						treeWidgetMachineList->setCurrentItem(current);
				}
			}
		} else
			qmc2CurrentItem = current;
	}
	qmc2CheckItemVisibility = true;
	if ( qmc2UpdateDelay > 0 )
		updateTimer.start(qmc2UpdateDelay);
	else
		treeWidgetMachineList_itemSelectionChanged_delayed();
}

void MainWindow::on_treeWidgetHierarchy_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
	qmc2CheckItemVisibility = false;
	if ( qmc2UpdateDelay > 0 )
		updateTimer.start(qmc2UpdateDelay);
	else
		treeWidgetMachineList_itemSelectionChanged_delayed();
}

void MainWindow::on_treeWidgetMachineList_itemSelectionChanged()
{
	if ( qmc2UpdateDelay > 0 )
		updateTimer.start(qmc2UpdateDelay);
	else
		treeWidgetMachineList_itemSelectionChanged_delayed();
}

void MainWindow::treeWidgetMachineList_itemSelectionChanged_delayed()
{
	updateTimer.stop();
	QList<QTreeWidgetItem *> selected;
	if ( qmc2HierarchySelectedItem != 0 )
		selected.append(qmc2HierarchySelectedItem);
	else
		selected = treeWidgetMachineList->selectedItems();
	if ( selected.isEmpty() )
		if ( treeWidgetMachineList->currentItem() )
			selected.append(treeWidgetMachineList->currentItem());
	if ( !selected.isEmpty() ) {
		QTreeWidgetItem *topLevelItem = selected.at(0);
		while ( topLevelItem->parent() )
			topLevelItem = topLevelItem->parent();
		if ( topLevelItem ) {
			qmc2CurrentItem = topLevelItem;
			ImageWidget::updateArtwork();
			on_tabWidgetMachineDetail_currentChanged(tabWidgetMachineDetail->currentIndex());
		}
	} else
		qmc2CurrentItem = 0;
	qmc2HierarchySelectedItem = 0;
	QTimer::singleShot(0, this, SLOT(checkSystemManualAvailability()));
}

void MainWindow::on_treeWidgetHierarchy_itemSelectionChanged()
{
	qmc2HierarchySelectedItem = 0;
	QList<QTreeWidgetItem *>selected = treeWidgetHierarchy->selectedItems();
	if ( selected.count() > 0 ) {
		if ( selected.at(0)->text(QMC2_MACHINELIST_COLUMN_MACHINE) == MachineList::trWaitingForData )
			return;
		qmc2HierarchySelectedItem = qmc2MachineListItemHash.value(selected.at(0)->text(QMC2_MACHINELIST_COLUMN_NAME));
		qmc2CheckItemVisibility = false;
		treeWidgetMachineList->setCurrentItem(qmc2HierarchySelectedItem);
	}
}

void MainWindow::on_treeWidgetEmulators_customContextMenuRequested(const QPoint &p)
{
	QTreeWidgetItem *item = treeWidgetEmulators->itemAt(p);
	if ( item ) {
		treeWidgetEmulators->setItemSelected(item, true);
		qmc2EmulatorMenu->move(adjustedWidgetPosition(treeWidgetEmulators->viewport()->mapToGlobal(p), qmc2EmulatorMenu));
		qmc2EmulatorMenu->show();
	}
}

#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
void MainWindow::action_embedEmulator_triggered()
{
	qmc2StartEmbedded = false;

	QStringList mL;
	QStringList idList;
#if defined(QMC2_OS_UNIX)
	QStringList statusList;
#endif

	ComponentInfo *componentInfo = qmc2ComponentSetup->componentInfoHash().value("Component1");

	QList<QTreeWidgetItem *> sl = treeWidgetEmulators->selectedItems();
	for (int i = 0; i < sl.count(); i++) {
		QTreeWidgetItem *item = sl.at(i);
		while ( item->parent() )
			item = item->parent();
		mL << item->text(QMC2_EMUCONTROL_COLUMN_MACHINE).split(':').at(0);
		idList << item->text(QMC2_EMUCONTROL_COLUMN_NUMBER);
#if defined(QMC2_OS_UNIX)
		statusList << item->text(QMC2_EMUCONTROL_COLUMN_STATUS);
#endif
	}

	if ( sl.count() == 0 )
		if ( treeWidgetEmulators->currentItem() ) {
			QTreeWidgetItem *item = treeWidgetEmulators->currentItem();
			while ( item->parent() )
				item = item->parent();
			mL << item->text(QMC2_EMUCONTROL_COLUMN_MACHINE).split(':').at(0);
			idList << item->text(QMC2_EMUCONTROL_COLUMN_NUMBER);
#if defined(QMC2_OS_UNIX)
			statusList << item->text(QMC2_EMUCONTROL_COLUMN_STATUS);
#endif
		}

	bool success = true;
	int i;
	for (i = 0; i < mL.count(); i++) {
		QString machineName(mL.at(i));
		QString machineId(idList.at(i));
#if defined(QMC2_OS_UNIX)
		QString machineStatus(statusList.at(i));
#endif

		if ( machineName.isEmpty() || machineId.isEmpty() )
			continue;

		// check if the emulator window is already embedded
		bool found = false;
		int j;
		for (j = 0; j < tabWidgetEmbeddedEmulators->count() && !found; j++)
			found = tabWidgetEmbeddedEmulators->tabText(j).startsWith(QString("#%1 - ").arg(machineId));
		if ( found ) {                                                                  
			log(QMC2_LOG_FRONTEND, tr("emulator #%1 is already embedded").arg(machineId));
			tabWidgetMachineList->setCurrentIndex(tabWidgetMachineList->indexOf(widgetEmbeddedEmus));
			tabWidgetEmbeddedEmulators->setCurrentIndex(j - 1);
			continue;
		}

		QTreeWidgetItem *machineItem = qmc2MachineListItemHash.value(machineName);
#if defined(QMC2_OS_UNIX)
		QList<WId> winIdList;
		int xwininfoRetries = 0;
		while ( winIdList.isEmpty() && xwininfoRetries++ < QMC2_MAX_XWININFO_RETRIES ) {
			WId windowId = x11FindWindowId(QRegExp::escape(QString("MAME: %1").arg(machineItem ? machineItem->text(QMC2_MACHINELIST_COLUMN_MACHINE) : "")), QRegExp::escape(QString("QMC2-MAME-ID-%1").arg(machineId)));
			if ( windowId )
				winIdList << windowId;

			if ( winIdList.isEmpty() && xwininfoRetries <= QMC2_MAX_XWININFO_RETRIES )
				QTest::qWait(QMC2_XWININFO_DELAY);
		}
#elif defined(QMC2_OS_WIN)
		QList<WId> winIdList;
		int wininfoRetries = 0;
		while ( winIdList.isEmpty() && wininfoRetries++ < QMC2_MAX_WININFO_RETRIES ) {
			Q_PID gamePid = qmc2ProcessManager->getPid(machineId.toInt());
			if ( gamePid ) {
				HWND hwnd = winFindWindowHandleOfProcess(gamePid, "MAME:");
				if ( hwnd )
					winIdList << (WId)hwnd;
			}
			if ( winIdList.isEmpty() && wininfoRetries <= QMC2_MAX_WININFO_RETRIES )
				QTest::qWait(QMC2_WININFO_DELAY);
		}
#endif

#if defined(QMC2_OS_UNIX)
		if ( winIdList.count() > 1 )
			log(QMC2_LOG_FRONTEND, tr("WARNING: multiple windows for emulator #%1 found, choosing window ID %2 for embedding").arg(machineId).arg("0x" + QString::number(winIdList[0], 16)));
#elif defined(QMC2_OS_WIN)
		if ( winIdList.count() > 1 )
			log(QMC2_LOG_FRONTEND, tr("WARNING: multiple windows for emulator #%1 found, choosing window ID %2 for embedding").arg(machineId).arg("0x" + QString::number((qulonglong)winIdList[0], 16)));
#endif

		if ( !winIdList.isEmpty() ) {
			int embeddedEmusIndex = tabWidgetMachineList->indexOf(widgetEmbeddedEmus);
			if ( tabWidgetMachineList->currentIndex() != embeddedEmusIndex )
				qmc2LastListIndex = tabWidgetMachineList->currentIndex();
			if ( embeddedEmusIndex < 0 ) {
				int insertIndex = componentInfo->appliedFeatureList().indexOf(QMC2_EMBED_INDEX);
				int foreignIndex = componentInfo->appliedFeatureList().indexOf(QMC2_FOREIGN_INDEX);
				if ( foreignIndex >= 0 && foreignIndex <= insertIndex )
					if ( tabWidgetMachineList->indexOf(tabForeignEmulators) < 0 )
						insertIndex--;
				tabWidgetMachineList->insertTab(insertIndex, widgetEmbeddedEmus, QIcon(QString::fromUtf8(":/data/img/embed.png")), tr("Embedded emulators"));
			}
			tabWidgetMachineList->setCurrentIndex(tabWidgetMachineList->indexOf(widgetEmbeddedEmus));
#if defined(QMC2_OS_UNIX)
			qApp->syncX();
			log(QMC2_LOG_FRONTEND, tr("embedding emulator #%1, window ID = %2").arg(machineId).arg("0x" + QString::number(winIdList[0], 16)));
			Embedder *embedder = new Embedder(machineName, machineId, winIdList[0], (machineStatus == tr("paused")), this, qmc2IconHash.value(machineName));
#elif defined(QMC2_OS_WIN)
			log(QMC2_LOG_FRONTEND, tr("embedding emulator #%1, window ID = %2").arg(machineId).arg("0x" + QString::number((qulonglong)winIdList[0], 16)));
			Embedder *embedder = new Embedder(machineName, machineId, winIdList[0], false, this, qmc2IconHash.value(machineName));
#endif
			connect(embedder, SIGNAL(closing()), this, SLOT(closeEmbeddedEmuTab()));
			if ( machineItem )
				tabWidgetEmbeddedEmulators->addTab(embedder, QString("#%1 - %2").arg(machineId).arg(machineItem->text(QMC2_MACHINELIST_COLUMN_MACHINE)));
			else
				tabWidgetEmbeddedEmulators->addTab(embedder, QString("#%1").arg(machineId));
			tabWidgetEmbeddedEmulators->setCurrentIndex(tabWidgetEmbeddedEmulators->count() - 1);

			// serious hack to access the tab bar without sub-classing from QTabWidget ;)
			QTabBar *tabBar = tabWidgetEmbeddedEmulators->findChild<QTabBar *>();
			if ( tabBar ) {
				// replace the default 'X' icon with something more suitable (the only way I found to accomplish this is by setting a style-sheet)
				tabBar->setStyleSheet("QTabBar::close-button { image: url(:/data/img/release.png); subcontrol-position: right; } QTabBar::close-button:hover { image: url(:/data/img/release_alternate.png); }");

				int index = tabWidgetEmbeddedEmulators->indexOf(embedder);
				tabBar->tabButton(index, QTabBar::RightSide)->setToolTip(tr("Release emulator"));

				QFont f;
				f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
				QFontMetrics fm(f);
				int baseSize = fm.height() + 2;
				QSize optionsButtonSize(2 * baseSize, baseSize + 2);

				QToolButton *optionsButton = new QToolButton(tabBar);
				optionsButton->setFixedSize(optionsButtonSize);
				optionsButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
				optionsButton->setText(" ");
				optionsButton->setIcon(QIcon(QString::fromUtf8(":/data/img/work.png")));
				optionsButton->setToolTip(tr("Toggle embedder options (hold down for menu)"));
				optionsButton->setStatusTip(tr("Toggle embedder options (hold down for menu)"));
				optionsButton->setCheckable(true);
				connect(optionsButton, SIGNAL(toggled(bool)), this, SLOT(embedderOptions_toggled(bool)));

				QMenu *optionsMenu = new QMenu(optionsButton);
				QString s;
				QAction *action;
				s = tr("To favorites");
				action = optionsMenu->addAction(tr("To &favorites"));
				action->setIcon(QIcon(QString::fromUtf8(":/data/img/favorites.png")));
				action->setToolTip(s); action->setStatusTip(s);
				connect(action, SIGNAL(triggered()), this, SLOT(embedderOptionsMenu_ToFavorites_activated()));
				s = tr("Copy emulator command line to clipboard");
				action = optionsMenu->addAction(tr("&Copy command"));
				action->setToolTip(s); action->setStatusTip(s);
				action->setIcon(QIcon(QString::fromUtf8(":/data/img/editcopy.png")));
				connect(action, SIGNAL(triggered()), this, SLOT(embedderOptionsMenu_CopyCommand_activated()));
				optionsMenu->addSeparator();
				s = tr("Kill emulator");
				action = optionsMenu->addAction(tr("&Kill emulator"));
				action->setIcon(QIcon(QString::fromUtf8(":/data/img/kill.png")));
				action->setToolTip(s); action->setStatusTip(s);
				connect(action, SIGNAL(triggered()), this, SLOT(embedderOptionsMenu_KillEmulator_activated()));

				optionsButton->setMenu(optionsMenu);

				tabBar->setTabButton(index, QTabBar::LeftSide, optionsButton);

				QTimer::singleShot(0, embedder, SLOT(embed()));
			}
		} else {
			success = false;
			log(QMC2_LOG_FRONTEND, tr("WARNING: no matching window for emulator #%1 found").arg(machineId));
		}
	}

	sl = treeWidgetEmulators->selectedItems();
	if ( !success && sl.count() > 0 ) {
		switch ( QMessageBox::question(this, tr("Embedding failed"),
					tr("Couldn't find the window ID of one or more\nemulator(s) within a reasonable timeframe.\n\nRetry embedding?"),
					QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) ) {
			case QMessageBox::No:
				break;

			case QMessageBox::Yes:
				sl = treeWidgetEmulators->selectedItems();
				if ( sl.count() > 0 )
					QTimer::singleShot(0, this, SLOT(action_embedEmulator_triggered()));
				else
					QMessageBox::information(this, tr("Information"), tr("Sorry, the emulator meanwhile died a sorrowful death :(."));
				break;

			default:
				break;
		}
	}
}

#if defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000
void MainWindow::action_embedderScanPauseKey_triggered()
{
	qApp->removeEventFilter(qmc2MainEventFilter);

	KeySequenceScanner keySeqScanner(this, true, true);
	keySeqScanner.setWindowTitle(tr("Scanning pause key"));
	keySeqScanner.labelStatus->setText(tr("Scanning pause key"));
	if ( keySeqScanner.exec() == QDialog::Accepted )
  		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Embedder/PauseKey", keySeqScanner.keySequence);

	qApp->installEventFilter(qmc2MainEventFilter);
}
#endif

void MainWindow::embedderOptions_toggled(bool enabled)
{
	// serious hack to access the tab bar ;)
	QToolButton *optionsButton = (QToolButton *)sender();
	QTabBar *tabBar = (QTabBar *)optionsButton->parent();
	Embedder *embedder = (Embedder *)tabWidgetEmbeddedEmulators->widget(tabBar->tabAt(optionsButton->pos()));
	if ( embedder )
		embedder->toggleOptions();
	if ( enabled )
		tabWidgetEmbeddedEmulators->setCurrentIndex(tabWidgetEmbeddedEmulators->indexOf(embedder));
}

void MainWindow::on_tabWidgetEmbeddedEmulators_tabCloseRequested(int index)
{
	tabWidgetMachineList->setUpdatesEnabled(false);
	QWidget *widget = 0;
	if ( index >= 0 ) {
		widget = tabWidgetEmbeddedEmulators->widget(index);
		if ( widget )
			tabWidgetEmbeddedEmulators->removeTab(index);
	}
	if ( tabWidgetEmbeddedEmulators->count() < 1 ) {
		tabWidgetMachineList->removeTab(tabWidgetMachineList->indexOf(tabEmbeddedEmus));
		tabWidgetMachineList->setCurrentIndex(qmc2LastListIndex);
		if ( toolButtonEmbedderMaximizeToggle->isChecked() )
			hSplitter->setSizes(hSplitterSizes);
		menuBar()->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ShowMenuBar", true).toBool());
		statusBar()->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Statusbar", true).toBool());
		toolbar->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Toolbar", true).toBool());
	}
	if ( widget ) {
		widget->close();
		widget->deleteLater();
	}
	tabWidgetMachineList->setUpdatesEnabled(true);
}

void MainWindow::closeEmbeddedEmuTab()
{
	Embedder *embedder = (Embedder *)sender();
	if ( !qmc2CleaningUp )
		on_tabWidgetEmbeddedEmulators_tabCloseRequested(tabWidgetEmbeddedEmulators->indexOf(embedder));
}

void MainWindow::toolButtonEmbedderMaximizeToggle_toggled(bool on)
{
	if ( on ) {
		menuBar()->hide();
		statusBar()->hide();
		toolbar->hide();
		qApp->processEvents();
		hSplitterSizes = hSplitter->sizes();
		QList<int> maximizedSizes;
		if ( hSplitter->widget(0) == hSplitterWidget0 )
			maximizedSizes << desktopGeometry.width() << 0;
		else
			maximizedSizes << 0 << desktopGeometry.width();
		hSplitter->setSizes(maximizedSizes);
		toolButtonEmbedderMaximizeToggle->setIcon(QIcon(QString::fromUtf8(":/data/img/minimize.png")));
	} else {
		hSplitter->setSizes(hSplitterSizes);
		menuBar()->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ShowMenuBar", true).toBool());
		statusBar()->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Statusbar", true).toBool());
		toolbar->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Toolbar", true).toBool());
		toolButtonEmbedderMaximizeToggle->setIcon(QIcon(QString::fromUtf8(":/data/img/maximize.png")));
	}
}

void MainWindow::embedderOptionsMenu_KillEmulator_activated()
{
	// serious hack to access the corresponding embedder ;)
	QAction *action = (QAction *)sender();
	QMenu *menu = (QMenu *)action->parent();
	QToolButton *toolButton = (QToolButton *)menu->parent();
	QTabBar *tabBar = (QTabBar *)toolButton->parent();
	Embedder *embedder = (Embedder *)tabWidgetEmbeddedEmulators->widget(tabBar->tabAt(toolButton->pos()));
	if ( embedder )
		qmc2ProcessManager->kill(embedder->machineId.toInt());
}

void MainWindow::embedderOptionsMenu_ToFavorites_activated()
{
	// serious hack to access the corresponding embedder ;)
	QAction *action = (QAction *)sender();
	QMenu *menu = (QMenu *)action->parent();
	QToolButton *toolButton = (QToolButton *)menu->parent();
	QTabBar *tabBar = (QTabBar *)toolButton->parent();
	Embedder *embedder = (Embedder *)tabWidgetEmbeddedEmulators->widget(tabBar->tabAt(toolButton->pos()));
	if ( embedder ) {
		QString gameDescription = qmc2MachineListItemHash.value(embedder->machineName)->text(QMC2_MACHINELIST_COLUMN_MACHINE);
		QList<QListWidgetItem *> matches = listWidgetFavorites->findItems(gameDescription, Qt::MatchExactly);
		if ( matches.count() <= 0 ) {
			QListWidgetItem *item = new QListWidgetItem(listWidgetFavorites);
			item->setText(gameDescription);
			listWidgetFavorites->sortItems();
		}
	}
}

void MainWindow::embedderOptionsMenu_CopyCommand_activated()
{
	// serious hack to access the corresponding embedder ;)
	QAction *action = (QAction *)sender();
	QMenu *menu = (QMenu *)action->parent();
	QToolButton *toolButton = (QToolButton *)menu->parent();
	QTabBar *tabBar = (QTabBar *)toolButton->parent();
	Embedder *embedder = (Embedder *)tabWidgetEmbeddedEmulators->widget(tabBar->tabAt(toolButton->pos()));
	if ( embedder ) {
		QList<QTreeWidgetItem *> il = treeWidgetEmulators->findItems(embedder->machineId, Qt::MatchExactly, QMC2_EMUCONTROL_COLUMN_ID);
		if ( il.count() > 0 )
			QApplication::clipboard()->setText(il[0]->text(QMC2_EMUCONTROL_COLUMN_COMMAND));
	}
}
#endif

void MainWindow::action_killEmulator_triggered()
{
	QList<QTreeWidgetItem *> sl = treeWidgetEmulators->selectedItems();
	for (int i = 0; i < sl.count(); i++) {
		QTreeWidgetItem *item = sl[i];
		while ( item->parent() ) item = item->parent();
		qmc2ProcessManager->kill(item->text(QMC2_EMUCONTROL_COLUMN_NUMBER).toInt());
	}

	if ( sl.count() == 0 )
		if ( treeWidgetEmulators->currentItem() ) {
			QTreeWidgetItem *item = treeWidgetEmulators->currentItem();
			while ( item->parent() ) item = item->parent();
			qmc2ProcessManager->kill(item->text(QMC2_EMUCONTROL_COLUMN_NUMBER).toInt());
		}
}

void MainWindow::action_copyEmulatorCommand_triggered()
{
	QString commandString;
	QList<QTreeWidgetItem *> sl = treeWidgetEmulators->selectedItems();
	QList<QTreeWidgetItem *> tl;

	// find toplevel items...
	for (int i = 0; i < sl.count(); i++) {
		QTreeWidgetItem *item = sl[i];
		while ( item->parent() )
			item = item->parent();
		if ( !tl.contains(item) )
			tl.append(item);
	}
	// ... and copy their commands
	for (int i = 0; i < tl.count(); i++) {
		if ( i > 0 )
			commandString += "\n";
		commandString += tl[i]->text(QMC2_EMUCONTROL_COLUMN_COMMAND);
	}

	QApplication::clipboard()->setText(commandString);
}

void MainWindow::action_removeFromFavorites_triggered()
{
	QListWidgetItem *i = listWidgetFavorites->currentItem();
	if ( i ) {
		QListWidgetItem *item = listWidgetFavorites->takeItem(listWidgetFavorites->row(i));
		delete item;
	}
}

void MainWindow::action_clearAllFavorites_triggered()
{
	switch ( QMessageBox::question(this, tr("Confirm"),
				tr("Are you sure you want to clear the favorites list?"),
				QMessageBox::Yes | QMessageBox::No, QMessageBox::No) ) {
		case QMessageBox::No:
			return;
			break;

		case QMessageBox::Yes:
			listWidgetFavorites->clear();
			qmc2MachineList->saveFavorites();
			break;

		default:
			break;
	}
}

void MainWindow::action_saveFavorites_triggered()
{
	qmc2MachineList->saveFavorites();
}

void MainWindow::action_removeFromPlayed_triggered()
{
	QListWidgetItem *i = listWidgetPlayed->currentItem();
	if ( i ) {
		QListWidgetItem *item = listWidgetPlayed->takeItem(listWidgetPlayed->row(i));
		delete item;
	}
}

void MainWindow::action_clearAllPlayed_triggered()
{
	switch ( QMessageBox::question(this, tr("Confirm"),
				tr("Are you sure you want to clear the play history?"),
				QMessageBox::Yes | QMessageBox::No, QMessageBox::No) ) {
		case QMessageBox::No:
			return;
			break;

		case QMessageBox::Yes:
			listWidgetPlayed->clear();
			qmc2MachineList->savePlayHistory();
			break;

		default:
			break;
	}
}

void MainWindow::action_savePlayed_triggered()
{
	qmc2MachineList->savePlayHistory();
}

void MainWindow::on_listWidgetSearch_customContextMenuRequested(const QPoint &p)
{
	QListWidgetItem *item = listWidgetSearch->itemAt(p);
	if ( item ) {
		listWidgetSearch->setItemSelected(item, true);
		qmc2SearchMenu->move(adjustedWidgetPosition(listWidgetSearch->viewport()->mapToGlobal(p), qmc2SearchMenu));
		qmc2SearchMenu->show();
	}
}

void MainWindow::on_listWidgetFavorites_customContextMenuRequested(const QPoint &p)
{
	QListWidgetItem *item = listWidgetFavorites->itemAt(p);
	if ( item ) {
		listWidgetFavorites->setItemSelected(item, true);
		qmc2FavoritesMenu->move(adjustedWidgetPosition(listWidgetFavorites->viewport()->mapToGlobal(p), qmc2FavoritesMenu));
		qmc2FavoritesMenu->show();
	}
}

void MainWindow::on_listWidgetPlayed_customContextMenuRequested(const QPoint &p)
{
	QListWidgetItem *item = listWidgetPlayed->itemAt(p);
	if ( item ) {
		listWidgetPlayed->setItemSelected(item, true);
		qmc2PlayedMenu->move(adjustedWidgetPosition(listWidgetPlayed->viewport()->mapToGlobal(p), qmc2PlayedMenu));
		qmc2PlayedMenu->show();
	}
}

void MainWindow::on_treeWidgetMachineList_customContextMenuRequested(const QPoint &p)
{
	QTreeWidgetItem *item = treeWidgetMachineList->itemAt(p);
	if ( !item )
		return;
	if ( item->text(QMC2_MACHINELIST_COLUMN_MACHINE) == MachineList::trWaitingForData )
		return;
	treeWidgetMachineList->setItemSelected(item, true);
	qmc2MachineMenu->move(adjustedWidgetPosition(treeWidgetMachineList->viewport()->mapToGlobal(p), qmc2MachineMenu));
	qmc2MachineMenu->show();
}

void MainWindow::on_treeWidgetHierarchy_customContextMenuRequested(const QPoint &p)
{
	QTreeWidgetItem *item = treeWidgetHierarchy->itemAt(p);
	if ( !item )
		return;
	if ( item->text(QMC2_MACHINELIST_COLUMN_MACHINE) == MachineList::trWaitingForData )
		return;
	if ( item ) {
		treeWidgetHierarchy->setItemSelected(item, true);
		qmc2MachineMenu->move(adjustedWidgetPosition(treeWidgetHierarchy->viewport()->mapToGlobal(p), qmc2MachineMenu));
		qmc2MachineMenu->show();
	}
}

void MainWindow::on_stackedWidgetView_currentChanged(int index)
{
	if ( !qmc2EarlyStartup ) {
		menuMachineListHeader->hide();
		menuHierarchyHeader->hide();
		menuCategoryHeader->hide();
		menuVersionHeader->hide();
	}

	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "GUI/MachineListView", index);
	bool romFilterActive = qmc2Config->value(QMC2_FRONTEND_PREFIX + "RomStateFilter/Enabled", true).toBool();
	switch ( index ) {
		case QMC2_VIEWHIERARCHY_INDEX:
		case QMC2_VIEWCATEGORY_INDEX:
		case QMC2_VIEWVERSION_INDEX:
			toolButtonSelectRomFilter->setVisible(false);
			menuRomStatusFilter->setVisible(false);
			actionTagVisible->setVisible(false);
			actionUntagVisible->setVisible(false);
			actionInvertVisibleTags->setVisible(false);
			break;
		case QMC2_VIEWMACHINELIST_INDEX:
		default:
			toolButtonSelectRomFilter->setVisible(romFilterActive);
			actionTagVisible->setVisible(romFilterActive);
			actionUntagVisible->setVisible(romFilterActive);
			actionInvertVisibleTags->setVisible(romFilterActive);
			break;
	}

	if ( !qmc2CurrentItem )
		return;

	QTreeWidgetItem *ci = treeWidgetMachineList->currentItem();
	if ( !ci )
		return;

	if ( qmc2CurrentItem != ci && qmc2ReloadActive )
		qmc2CurrentItem = ci;

	if ( qmc2CurrentItem->childCount() <= 0 )
		return;

	switch ( index ) {
		case QMC2_VIEWHIERARCHY_INDEX:
			if ( !qmc2ReloadActive ) {
				QString machineName(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_NAME));
				QTreeWidgetItem *hierarchyItem = qmc2HierarchyItemHash.value(machineName);
				treeWidgetHierarchy->clearSelection();
				if ( hierarchyItem ) {
					treeWidgetHierarchy->setCurrentItem(hierarchyItem);
					treeWidgetHierarchy->scrollToItem(hierarchyItem, qmc2CursorPositioningMode);
					hierarchyItem->setSelected(true);
				}
				treeWidgetHierarchy_verticalScrollChanged();
			}
			break;
		case QMC2_VIEWCATEGORY_INDEX:
			if ( !qmc2ReloadActive ) {
				QString machineName(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_NAME));
				QTreeWidgetItem *categoryItem = qmc2CategoryItemHash.value(machineName);
				treeWidgetCategoryView->clearSelection();
				if ( categoryItem ) {
					treeWidgetCategoryView->setCurrentItem(categoryItem);
					treeWidgetCategoryView->scrollToItem(categoryItem, qmc2CursorPositioningMode);
					categoryItem->setSelected(true);
				}
				treeWidgetCategoryView_verticalScrollChanged();
			}
			break;
		case QMC2_VIEWVERSION_INDEX:
			if ( !qmc2ReloadActive ) {
				QString machineName(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_NAME));
				QTreeWidgetItem *versionItem = qmc2VersionItemHash.value(machineName);
				treeWidgetVersionView->clearSelection();
				if ( versionItem ) {
					treeWidgetVersionView->setCurrentItem(versionItem);
					treeWidgetVersionView->scrollToItem(versionItem, qmc2CursorPositioningMode);
					versionItem->setSelected(true);
				}
				treeWidgetVersionView_verticalScrollChanged();
			}
			break;
		case QMC2_VIEWMACHINELIST_INDEX:
		default:
			scrollToCurrentItem();
			treeWidgetMachineList_verticalScrollChanged();
			break;
	}
}

void MainWindow::pushButtonGlobalEmulatorOptionsSelectExportFile_clicked()
{
	QString fileName = qmc2Config->value("MAME/Configuration/Global/inipath", QDir::homePath()).toString().replace("~", QDir::homePath()) + "/mame.ini";
	QString s = QFileDialog::getSaveFileName(qmc2Options, tr("Choose export file"), fileName, tr("All files (*)"), 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		pushButtonGlobalEmulatorOptionsExportToFile_clicked(s);
}

void MainWindow::pushButtonGlobalEmulatorOptionsExportToFile_clicked(QString useFileName)
{
	qmc2GlobalEmulatorOptions->exportToIni(true, useFileName);
}

void MainWindow::pushButtonGlobalEmulatorOptionsSelectImportFile_clicked()
{
	QString fileName = qmc2Config->value("MAME/Configuration/Global/inipath", QDir::homePath()).toString().replace("~", QDir::homePath()) + "/mame.ini";
	QString s = QFileDialog::getOpenFileName(qmc2Options, tr("Choose import file"), fileName, tr("All files (*)"), 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		pushButtonGlobalEmulatorOptionsImportFromFile_clicked(s);
}

void MainWindow::pushButtonGlobalEmulatorOptionsImportFromFile_clicked(QString useFileName)
{
	qmc2GlobalEmulatorOptions->importFromIni(true, useFileName);
}

void MainWindow::pushButtonCurrentEmulatorOptionsSelectExportFile_clicked()
{
	QStringList iniPaths = qmc2Config->value(QMC2_EMULATOR_PREFIX + "Configuration/Global/inipath", QDir::homePath()).toString().split(";");
	QString iniPath;
	if ( iniPaths.count() > 0 )
		iniPath = iniPaths[0].replace("~", QDir::homePath());
	else {
		iniPath = ".";
		log(QMC2_LOG_FRONTEND, tr("WARNING: invalid inipath"));
	}
	QString s = QFileDialog::getSaveFileName(this, tr("Choose export file"), iniPath, tr("All files (*)"), 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		pushButtonCurrentEmulatorOptionsExportToFile_clicked(s);
}

void MainWindow::pushButtonCurrentEmulatorOptionsExportToFile_clicked(QString useFileName)
{
	qmc2EmulatorOptions->exportToIni(false, useFileName);
}

void MainWindow::pushButtonCurrentEmulatorOptionsSelectImportFile_clicked()
{
	if ( !qmc2CurrentItem )
		return;

	QStringList iniPaths = qmc2Config->value(QMC2_EMULATOR_PREFIX + "Configuration/Global/inipath", QDir::homePath()).toString().split(";");
	QString iniPath;
	if ( iniPaths.count() > 0 ) {
		iniPath = iniPaths[0].replace("~", QDir::homePath());
	} else {
		iniPath = ".";
		log(QMC2_LOG_FRONTEND, tr("WARNING: invalid inipath"));
	}
	iniPath += "/" + qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_NAME) + ".ini";
	QString s = QFileDialog::getOpenFileName(this, tr("Choose import file"), iniPath, tr("All files (*)"), 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		pushButtonCurrentEmulatorOptionsImportFromFile_clicked(s);
}

void MainWindow::pushButtonCurrentEmulatorOptionsImportFromFile_clicked(QString useFileName)
{
	qmc2EmulatorOptions->importFromIni(false, useFileName);
}

#if QMC2_JOYSTICK == 1
void MainWindow::mapJoystickFunction(QString function)
{
	if ( qmc2CleaningUp )
		return;

	// don't map joystick functions while calibration or test are active
	if ( qmc2Options->treeWidgetJoystickMappings->isHidden() )
		return;

	QString shortcut = qmc2CustomShortcutHash[qmc2JoystickFunctionHash[function]];

#ifdef QMC2_DEBUG
	log(QMC2_LOG_FRONTEND, QString("DEBUG: matched function = %1").arg(shortcut.isEmpty() ? "none" : shortcut));
#endif

	if ( shortcut.isEmpty() )
		return;
  
	QWidget *focusWidget = QApplication::focusWidget();

	if ( focusWidget ) {
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
		// don't map joystick functions when they are really meant for an embedded emulator
		if ( focusWidget->objectName() == "QMC2_EMBED_CONTAINER" )
			return;
#endif
		QKeySequence keySeq(shortcut);
		int key = 0;
#if QT_VERSION < 0x050000
		for (uint i = 0; i < keySeq.count(); i++)
			key += keySeq[i];
#else
		for (int i = 0; i < keySeq.count(); i++)
			key += keySeq[i];
#endif
		QKeyEvent *emulatedKeyEvent = new QKeyEvent(QEvent::KeyPress, key, Qt::NoModifier);
		qApp->postEvent(focusWidget, emulatedKeyEvent);
	}
}

void MainWindow::joystickAxisValueChanged(int axis, int value)
{
	if ( qmc2Config->value(QString(QMC2_FRONTEND_PREFIX + "Joystick/%1/Axis%2Enabled").arg(joyIndex).arg(axis), true).toBool() ) {
		if ( value != 0 )
			mapJoystickFunction(QString("A%1%2").arg(axis).arg(value < 0 ? "-" : "+"));
	}
}

void MainWindow::joystickButtonValueChanged(int button, bool value)
{
	if ( value )
		mapJoystickFunction(QString("B%1").arg(button));
}

void MainWindow::joystickHatValueChanged(int hat, int value)
{
	if ( value != 0 )
		mapJoystickFunction(QString("H%1:%2").arg(hat).arg(value));
}

void MainWindow::joystickTrackballValueChanged(int trackball, int deltaX, int deltaY)
{
	mapJoystickFunction(QString("T%1:X%2,Y%3").arg(trackball).arg(deltaX < 0 ? "-" : deltaX > 0 ? "+" : "=").arg(deltaY < 0 ? "-" : deltaY > 0 ? "+" : "="));
}
#endif

void MainWindow::closeEvent(QCloseEvent *e)
{
	if ( qmc2CleaningUp ) {
		e->ignore();
		return;
	}

	if ( qmc2ReloadActive || qmc2VerifyActive || qmc2FilterActive || qmc2ImageCheckActive || qmc2SampleCheckActive || (qmc2SystemROMAlyzer && qmc2SystemROMAlyzer->active()) || (qmc2SoftwareROMAlyzer && qmc2SoftwareROMAlyzer->active()) || qmc2LoadingMachineInfoDB || qmc2LoadingSoftwareInfoDB || qmc2LoadingEmuInfoDB ) {
		qmc2LoadingInterrupted = true;
		log(QMC2_LOG_FRONTEND, tr("stopping current processing upon user request"));
		e->ignore();
		return;
	}

	if ( qmc2SoftwareList && qmc2SoftwareList->isLoading ) {
		qmc2SoftwareList->interruptLoad = true;
		log(QMC2_LOG_FRONTEND, tr("stopping current processing upon user request"));
		e->ignore();
		return;
	}

	if ( !qmc2Options->applied ) {
		switch ( QMessageBox::question(this, tr("Confirm"),
					tr("Your configuration changes have not been applied yet.\nReally quit?"),
					QMessageBox::Yes | QMessageBox::No, QMessageBox::No) ) {
			case QMessageBox::No:
				e->ignore();
				return;
				break;

			default:
				break;
		}
	}
 
	bool doKillEmulators = qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/KillEmulatorsOnExit").toBool();
	if ( qmc2ProcessManager->procMap.count() > 0 && !doKillEmulators ) {
		switch ( QMessageBox::question(this, tr("Confirm"),
					tr("There are one or more emulators still running.\nShould they be killed on exit?"),
					QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Cancel) ) {
			case QMessageBox::Yes:
				doKillEmulators = true;
				break;

			case QMessageBox::No:
				doKillEmulators = false;
				break;

			case QMessageBox::Cancel:
				e->ignore();
				return;
				break;

			default:
				break;
		}
	}

	QList<int> indexList;
	int runningDownloads = 0;
	QTreeWidgetItemIterator it(treeWidgetDownloads);
	while ( *it ) {
		indexList << treeWidgetDownloads->indexOfTopLevelItem(*it);
		if ( (*it)->whatsThis(0) == "downloading" || (*it)->whatsThis(0) == "initializing" )
			runningDownloads++;
		it++;
	}
	if ( runningDownloads > 0 ) {
		switch ( QMessageBox::question(this, tr("Confirm"),
					tr("There are one or more running downloads. Quit anyway?"),
					QMessageBox::Yes | QMessageBox::No, QMessageBox::No) ) {
			case QMessageBox::Yes:
				break;

			case QMessageBox::No:
				e->ignore();
				return;

			default:
				break;
		}
	}

	log(QMC2_LOG_FRONTEND, tr("cleaning up"));
	qmc2CleaningUp = true;

	if ( runningDownloads > 0 )
		log(QMC2_LOG_FRONTEND, tr("aborting running downloads"));
	foreach (int i, indexList) {
		DownloadItem *item = (DownloadItem *)treeWidgetDownloads->takeTopLevelItem(i);
		if ( item ) {
			if ( item->whatsThis(0) == "downloading" || item->whatsThis(0) == "initializing" )
				item->itemDownloader->networkReply->abort();
			delete item;
		}
	}

#if defined(QMC2_YOUTUBE_ENABLED)
	if ( !qmc2YouTubeVideoInfoHash.isEmpty() && qmc2YouTubeVideoInfoHashChanged ) {
		log(QMC2_LOG_FRONTEND, tr("saving YouTube video info map"));
		QDir youTubeCacheDir(qmc2Config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/CacheDirectory").toString());
		if ( youTubeCacheDir.exists() ) {
#if defined(QMC2_SDLMAME)
			QFile f(youTubeCacheDir.canonicalPath() + "/qmc2-sdlmame.yti");
#elif defined(QMC2_MAME)
			QFile f(youTubeCacheDir.canonicalPath() + "/qmc2-mame.yti");
#else
			QFile f(youTubeCacheDir.canonicalPath() + "/qmc2.yti");
#endif
			if ( f.open(QIODevice::WriteOnly | QIODevice::Text) ) {
				QTextStream ts(&f);
				ts << "# THIS FILE IS AUTO-GENERATED - PLEASE DO NOT EDIT!\n";
				QHashIterator<QString, YouTubeVideoInfo> it(qmc2YouTubeVideoInfoHash);
				while ( it.hasNext() ) {
					it.next();
					ts << it.key() << "\t" << it.value().author << "\t" << it.value().title << "\n";
				}
				f.close();
				log(QMC2_LOG_FRONTEND, tr("done (saving YouTube video info map)"));
			} else
				log(QMC2_LOG_FRONTEND, tr("failed (saving YouTube video info map)"));
		} else
			log(QMC2_LOG_FRONTEND, tr("failed (saving YouTube video info map)"));
	}
#endif

#if QMC2_USE_PHONON_API
	log(QMC2_LOG_FRONTEND, tr("disconnecting audio source from audio sink"));
	phononAudioPath.disconnect();
	if ( qmc2AudioEffectDialog ) {
		log(QMC2_LOG_FRONTEND, tr("destroying audio effects dialog"));
		qmc2AudioEffectDialog->close();
		delete qmc2AudioEffectDialog;
	}
	if ( phononAudioOutput )
		delete phononAudioOutput;
	if ( phononAudioPlayer )
		delete phononAudioPlayer;
#endif
#if QMC2_MULTIMEDIA_ENABLED
	if ( mediaPlayer ) {
		log(QMC2_LOG_FRONTEND, tr("destroying media player"));
		mediaPlayer->disconnect();
		delete mediaPlayer;
	}
#endif

#if defined(QMC2_YOUTUBE_ENABLED)
	if ( qmc2YouTubeWidget ) {
		log(QMC2_LOG_FRONTEND, tr("destroying YouTube video widget"));
		qmc2YouTubeWidget->saveSettings();
		qmc2YouTubeWidget->forcedExit = true;
		if ( qmc2YouTubeWidget->isPlaying() || qmc2YouTubeWidget->isPaused() )
			qmc2YouTubeWidget->stop();
		qmc2YouTubeWidget->close();
		qmc2YouTubeWidget->deleteLater();
		qmc2YouTubeWidget = 0;
	}
#endif

	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/RanksLocked", RankItemWidget::ranksLocked);

	if ( !qmc2TemplateCheck ) {
		if ( listWidgetFavorites->count() > 0 )
			qmc2MachineList->saveFavorites();

		if ( listWidgetPlayed->count() > 0 )
			qmc2MachineList->savePlayHistory();

		QStringList searchStrings;
		for (int index = 0; index < comboBoxSearch->count(); index++)
			searchStrings << comboBoxSearch->itemText(index);
		if ( searchStrings.isEmpty() )
			qmc2Config->remove(QMC2_FRONTEND_PREFIX + "MachineSearchHistory");
		else
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "MachineSearchHistory", searchStrings);

		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "GUI/MachineListView", comboBoxViewSelect->currentIndex());
		if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveMachineSelection").toBool() ) {
			if ( qmc2CurrentItem ) {
				log(QMC2_LOG_FRONTEND, tr("saving machine selection"));
				qmc2Config->setValue(QMC2_EMULATOR_PREFIX + "SelectedMachine", qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_NAME));
			} else
				qmc2Config->remove(QMC2_EMULATOR_PREFIX + "SelectedMachine");
		}

		QTreeWidgetItem *foreignItem = treeWidgetForeignIDs->currentItem();
		if ( foreignItem && foreignItem->isSelected() ) {
			QTreeWidgetItem *parentItem = foreignItem;
			if ( foreignItem->parent() )
				parentItem = foreignItem->parent();
			QStringList foreignIdState;
			if ( parentItem == foreignItem )
				foreignIdState << QString::number(treeWidgetForeignIDs->indexOfTopLevelItem(parentItem));
			else
				foreignIdState << QString::number(treeWidgetForeignIDs->indexOfTopLevelItem(parentItem)) << QString::number(parentItem->indexOfChild(foreignItem));
			qmc2Config->setValue(QMC2_EMULATOR_PREFIX + "SelectedForeignID", foreignIdState);
		} else
			qmc2Config->remove(QMC2_EMULATOR_PREFIX + "SelectedForeignID");

		log(QMC2_LOG_FRONTEND, tr("saving main widget layout"));
		if ( windowState() & Qt::WindowFullScreen ) {
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Fullscreen", true);
		} else {
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Fullscreen", false);
			if ( isMaximized() ) {
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Maximized", true);
			} else {
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Maximized", false);
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Geometry", saveGeometry());
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Position", pos());
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/Size", size());
			}
		}

		ComponentInfo *componentInfo = qmc2ComponentSetup->componentInfoHash().value("Component1");

#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
		int currentIndex = tabWidgetMachineList->currentIndex();
		int embedIndex = componentInfo->appliedFeatureList().indexOf(QMC2_EMBED_INDEX);
		if ( embedIndex >= 0 && embedIndex <= currentIndex )
			if ( tabWidgetMachineList->indexOf(tabEmbeddedEmus) < 0 )
				currentIndex++;
		int foreignIndex = componentInfo->appliedFeatureList().indexOf(QMC2_FOREIGN_INDEX);
		if ( foreignIndex >= 0 && foreignIndex <= currentIndex )
			if ( tabWidgetMachineList->indexOf(tabForeignEmulators) < 0 )
				currentIndex++;
		if ( toolButtonEmbedderMaximizeToggle->isChecked() && componentInfo->appliedFeatureList().at(currentIndex) == QMC2_EMBED_INDEX )
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/hSplitter", QSize(hSplitterSizes[0], hSplitterSizes[1]));
		else
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/hSplitter", QSize(hSplitter->sizes().at(0), hSplitter->sizes().at(1)));
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/Embedder/Maximize", toolButtonEmbedderMaximizeToggle->isChecked());
#if defined(QMC2_OS_UNIX)
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/Embedder/AutoPause", toolButtonEmbedderAutoPause->isChecked());
#endif
#else
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/hSplitter", QSize(hSplitter->sizes().at(0), hSplitter->sizes().at(1)));
#endif
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/vSplitter", QSize(vSplitterSizes.at(0), vSplitterSizes.at(1)));
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/vSplitterSoftwareDetail", QSize(vSplitterSizesSoftwareDetail.at(0), vSplitterSizesSoftwareDetail.at(1)));
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
		if ( componentInfo->appliedFeatureList().at(currentIndex) == QMC2_EMBED_INDEX )
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/MachineListTab", qmc2LastListIndex);
		else
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/MachineListTab", tabWidgetMachineList->currentIndex());
#else
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/MachineListTab", tabWidgetMachineList->currentIndex());
#endif
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/MachineDetailTab", tabWidgetMachineDetail->currentIndex());
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/LogsAndEmulatorsTab", tabWidgetLogsAndEmulators->currentIndex());
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/SoftwareDetailTab", tabWidgetSoftwareDetail->currentIndex());
		// save toolbar state
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/ToolbarState", saveState());
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/MachineListHeaderState", treeWidgetMachineList->header()->saveState());
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/HierarchyHeaderState", treeWidgetHierarchy->header()->saveState());
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/CategoryViewHeaderState", treeWidgetCategoryView->header()->saveState());
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/VersionViewHeaderState", treeWidgetVersionView->header()->saveState());
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/EmulatorControlHeaderState", treeWidgetEmulators->header()->saveState());

		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/SoftwareDetailDocked", floatToggleButtonSoftwareDetail->isChecked());
		if ( !floatToggleButtonSoftwareDetail->isChecked() )
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/SoftwareDetailGeometry", tabWidgetSoftwareDetail->saveGeometry());

#if QMC2_USE_PHONON_API || QMC2_MULTIMEDIA_ENABLED
		QStringList psl;
		for (int i = 0; i < listWidgetAudioPlaylist->count(); i++)
			psl << listWidgetAudioPlaylist->item(i)->text();
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "AudioPlayer/PlayList", psl);
		if ( listWidgetAudioPlaylist->currentItem() )
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "AudioPlayer/LastTrack", listWidgetAudioPlaylist->currentItem()->text());
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "AudioPlayer/PlayOnStart", checkBoxAudioPlayOnStart->isChecked());
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "AudioPlayer/Shuffle", checkBoxAudioShuffle->isChecked());
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "AudioPlayer/Pause", checkBoxAudioPause->isChecked());
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "AudioPlayer/Fade", checkBoxAudioFade->isChecked());
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "AudioPlayer/Volume", dialAudioVolume->value());
#endif
	}

	// download manager widget
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Downloads/RemoveFinished", checkBoxRemoveFinishedDownloads->isChecked());

	// search box options
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/NegateSearch", actionNegateSearch->isChecked());

	if ( qmc2SystemNotesEditor ) {
		qmc2SystemNotesEditor->stopLoading = true;
		qmc2SystemNotesEditor->save();
		qmc2SystemNotesEditor->close();
		delete qmc2SystemNotesEditor;
		qmc2SystemNotesEditor = 0;
	}

	if ( qmc2SoftwareList ) {
		if ( qmc2SoftwareNotesEditor ) {
			qmc2SoftwareNotesEditor->save();
			qmc2SoftwareNotesEditor->close();
			delete qmc2SoftwareNotesEditor;
		}
		if ( qmc2SoftwareList->fullyLoaded ) {
			log(QMC2_LOG_FRONTEND, tr("saving current machine's favorite software"));
			qmc2SoftwareList->save();
		}
		delete qmc2SoftwareList;
	}
	if ( qmc2DeviceConfigurator ) {
		log(QMC2_LOG_FRONTEND, tr("saving current machine's device configurations"));
		qmc2DeviceConfigurator->save();
		qmc2DeviceConfigurator->saveSetup();
		delete qmc2DeviceConfigurator;
	}

	if ( swlDb ) {
		QString connectionName = swlDb->connectionName();
		delete swlDb;
		QSqlDatabase::removeDatabase(connectionName);
	}

	log(QMC2_LOG_FRONTEND, tr("destroying emulator configuration instances"));
	if ( qmc2EmulatorOptions ) {
		QString selectedEmulator = comboBoxEmuSelector->currentText();
		if ( selectedEmulator == tr("Default") || selectedEmulator.isEmpty() )
			qmc2Config->remove(qmc2EmulatorOptions->settingsGroup + "/SelectedEmulator");
		else
			qmc2Config->setValue(qmc2EmulatorOptions->settingsGroup + "/SelectedEmulator", selectedEmulator);
		qmc2EmulatorOptions->save();
		delete qmc2EmulatorOptions;
	}
	qmc2GlobalEmulatorOptions->saveHeaderState();
	qmc2GlobalEmulatorOptions->setParent(0);
	log(QMC2_LOG_FRONTEND, tr("destroying machine list"));
	delete qmc2MachineList;

	log(QMC2_LOG_FRONTEND, tr("destroying image widgets"));
	if ( qmc2Preview )
		delete qmc2Preview;
	if ( qmc2Flyer )
		delete qmc2Flyer;
	if ( qmc2Cabinet )
		delete qmc2Cabinet;
	if ( qmc2Controller )
		delete qmc2Controller;
	if ( qmc2Marquee )
		delete qmc2Marquee;
	if ( qmc2Title )
		delete qmc2Title;
	if ( qmc2PCB )
		delete qmc2PCB;
	if ( qmc2SoftwareSnapshot )
		delete qmc2SoftwareSnapshot;
	if ( qmc2SoftwareSnap )
		delete qmc2SoftwareSnap;

	log(QMC2_LOG_FRONTEND, tr("destroying open dialogs"));
	if ( qmc2About )
		delete qmc2About;
	if ( qmc2DocBrowser ) {
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/DocBrowser/Zoom", qmc2DocBrowser->browser->spinBoxZoom->value());
		delete qmc2DocBrowser;
	}
	if ( qmc2ProjectMESSLookup )
		delete qmc2ProjectMESSLookup;
	if ( qmc2ImageChecker ) {
		qmc2ImageChecker->close();
		delete qmc2ImageChecker;
	}
	if ( qmc2SampleChecker ) {
		qmc2SampleChecker->close();
		delete qmc2SampleChecker;
	}
	if ( qmc2SystemROMAlyzer ) {
		qmc2SystemROMAlyzer->saveState();
		delete qmc2SystemROMAlyzer;
	}
	if ( qmc2SoftwareROMAlyzer ) {
		qmc2SoftwareROMAlyzer->saveState();
		delete qmc2SoftwareROMAlyzer;
	}
	if ( qmc2ROMStatusExporter ) {
		qmc2ROMStatusExporter->close();
		delete qmc2ROMStatusExporter;
	}
	if ( qmc2ComponentSetup ) {
		qmc2ComponentSetup->close();
		delete qmc2ComponentSetup;
	}
	if ( qmc2ToolBarCustomizer ) {
		qmc2ToolBarCustomizer->close();
		delete qmc2ToolBarCustomizer;
	}
	if ( qmc2DemoModeDialog ) {
		qmc2DemoModeDialog->close();
		delete qmc2DemoModeDialog;
	}

	log(QMC2_LOG_FRONTEND, tr("destroying process manager"));
	if ( qmc2ProcessManager->procMap.count() > 0 ) {
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
		for (int j = 0; j < tabWidgetEmbeddedEmulators->count(); j++) {
			Embedder *embedder = (Embedder *)tabWidgetEmbeddedEmulators->widget(j);
			if ( embedder )
				embedder->release();
		}
#endif
		if ( doKillEmulators ) {
			log(QMC2_LOG_FRONTEND, tr("killing %n running emulator(s) on exit", "", qmc2ProcessManager->procMap.count()));
			delete qmc2ProcessManager;
		} else
			log(QMC2_LOG_FRONTEND, tr("keeping %n running emulator(s) alive", "", qmc2ProcessManager->procMap.count()));
	} else 
		delete qmc2ProcessManager;

	if ( qmc2NetworkAccessManager ) {
		log(QMC2_LOG_FRONTEND, tr("destroying network access manager"));
		delete qmc2NetworkAccessManager;
	}

	log(QMC2_LOG_FRONTEND, tr("so long and thanks for all the fish"));

	qmc2Config->setValue(QString(QMC2_FRONTEND_PREFIX + "InstanceRunning"), false);

#if QT_VERSION < 0x050000
	qInstallMsgHandler(0);
#else
	qInstallMessageHandler(0);
#endif
	delete qmc2MainEventFilter;

	// remove possible left-overs
	QApplication::closeAllWindows();

	delete qmc2Options;
	e->accept();

	// reconfigure restart requested?
	if ( qmc2ReconfigureRestartRequested ) {
		QStringList args(qApp->arguments());
		args << "-r";
		QProcess::startDetached(args.at(0), args);
	}
}

void MainWindow::showEvent(QShowEvent *e)
{
	if ( !qmc2AutoMinimizedWidgets.isEmpty() ) {
		QMapIterator<QWidget *, Qt::WindowStates> it(qmc2AutoMinimizedWidgets);
		while ( it.hasNext() ) {
			it.next();
			it.key()->setWindowState(it.value());
		}
		qmc2AutoMinimizedWidgets.clear();
	}
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
	static bool mainWindowResizeEventActive = false;

	if ( mainWindowResizeEventActive )
		return;
	else
		mainWindowResizeEventActive = true;

	bool refreshViews = true;

	if ( e ) {
		refreshViews = (e->oldSize().height() != e->size().height());
		QMainWindow::resizeEvent(e);
	}

	if ( refreshViews ) {
		if ( tabWidgetMachineList->indexOf(tabMachineList) == tabWidgetMachineList->currentIndex() ) {
			switch ( stackedWidgetView->currentIndex() ) {
				case QMC2_VIEWHIERARCHY_INDEX:
					QTimer::singleShot(QMC2_RANK_UPDATE_DELAY, this, SLOT(treeWidgetHierarchy_verticalScrollChanged()));
					break;
				case QMC2_VIEWCATEGORY_INDEX:
					QTimer::singleShot(QMC2_RANK_UPDATE_DELAY, this, SLOT(treeWidgetCategoryView_verticalScrollChanged()));
					break;
				case QMC2_VIEWVERSION_INDEX:
					QTimer::singleShot(QMC2_RANK_UPDATE_DELAY, this, SLOT(treeWidgetVersionView_verticalScrollChanged()));
					break;
				case QMC2_VIEWMACHINELIST_INDEX:
				default:
					QTimer::singleShot(QMC2_RANK_UPDATE_DELAY, this, SLOT(treeWidgetMachineList_verticalScrollChanged()));
					break;
			}
		}
	}

	mainWindowResizeEventActive = false;
}

void MainWindow::init()
{
	if ( qmc2TemplateCheck )
		return;
 
#if QT_VERSION < 0x050000
#if !(QMC2_USE_PHONON_API)
	tabWidgetLogsAndEmulators->removeTab(tabWidgetLogsAndEmulators->indexOf(tabAudioPlayer));
	menuTools->removeAction(menuAudioPlayer->menuAction());
#else
	audioState = Phonon::StoppedState;
	phononAudioPlayer = new Phonon::MediaObject(this);
	phononAudioOutput = new Phonon::AudioOutput(Phonon::MusicCategory, this);
	phononAudioPath = Phonon::createPath(phononAudioPlayer, phononAudioOutput);
	listWidgetAudioPlaylist->setTextElideMode(Qt::ElideMiddle);
	listWidgetAudioPlaylist->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	QStringList psl(qmc2Config->value(QMC2_FRONTEND_PREFIX + "AudioPlayer/PlayList").toStringList());
	listWidgetAudioPlaylist->addItems(psl);
	int row = psl.indexOf(qmc2Config->value(QMC2_FRONTEND_PREFIX + "AudioPlayer/LastTrack", QString()).toString());
	if ( row >= 0 ) {
		listWidgetAudioPlaylist->setCurrentRow(row);
		listWidgetAudioPlaylist->scrollToItem(listWidgetAudioPlaylist->item(row), qmc2CursorPositioningMode);
		qmc2AudioLastIndividualTrack = psl.at(row);
	}
	checkBoxAudioPlayOnStart->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "AudioPlayer/PlayOnStart", false).toBool());
	checkBoxAudioShuffle->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "AudioPlayer/Shuffle", false).toBool());
	checkBoxAudioPause->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "AudioPlayer/Pause", true).toBool());
	checkBoxAudioFade->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "AudioPlayer/Fade", true).toBool());
	dialAudioVolume->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + "AudioPlayer/Volume", 50).toInt());
	phononAudioOutput->setVolume((qreal)dialAudioVolume->value()/100.0);
	toolButtonAudioPreviousTrack->setDefaultAction(actionAudioPreviousTrack);
	toolButtonAudioNextTrack->setDefaultAction(actionAudioNextTrack);
	toolButtonAudioStopTrack->setDefaultAction(actionAudioStopTrack);
	toolButtonAudioPauseTrack->setDefaultAction(actionAudioPauseTrack);
	toolButtonAudioPlayTrack->setDefaultAction(actionAudioPlayTrack);
	phononAudioPlayer->setTickInterval(1000);
	connect(phononAudioPlayer, SIGNAL(tick(qint64)), this, SLOT(audioTick(qint64)));
	connect(phononAudioPlayer, SIGNAL(totalTimeChanged(qint64)), this, SLOT(audioTotalTimeChanged(qint64)));
	connect(phononAudioPlayer, SIGNAL(finished()), this, SLOT(audioFinished()));
	connect(phononAudioPlayer, SIGNAL(metaDataChanged()), this, SLOT(audioMetaDataChanged()));
	connect(phononAudioPlayer, SIGNAL(bufferStatus(int)), this, SLOT(audioBufferStatus(int)));
	audioFastForwarding = audioFastBackwarding = audioSkippingTracks = false;
	if ( checkBoxAudioPlayOnStart->isChecked() ) {
		audioState = Phonon::PlayingState;
		QTimer::singleShot(0, this, SLOT(on_actionAudioPlayTrack_triggered()));
	} else
		QTimer::singleShot(0, this, SLOT(on_actionAudioStopTrack_triggered()));
#endif
#else
#if !(QMC2_MULTIMEDIA_ENABLED)
	tabWidgetLogsAndEmulators->removeTab(tabWidgetLogsAndEmulators->indexOf(tabAudioPlayer));
	menuTools->removeAction(menuAudioPlayer->menuAction());
#else
	audioState = QMediaPlayer::StoppedState;
	mediaPlayer = new QMediaPlayer(this, QMediaPlayer::StreamPlayback);
	listWidgetAudioPlaylist->setTextElideMode(Qt::ElideMiddle);
	listWidgetAudioPlaylist->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	QStringList psl(qmc2Config->value(QMC2_FRONTEND_PREFIX + "AudioPlayer/PlayList").toStringList());
	listWidgetAudioPlaylist->addItems(psl);
	int row = psl.indexOf(qmc2Config->value(QMC2_FRONTEND_PREFIX + "AudioPlayer/LastTrack", QString()).toString());
	if ( row >= 0 ) {
		listWidgetAudioPlaylist->setCurrentRow(row);
		listWidgetAudioPlaylist->scrollToItem(listWidgetAudioPlaylist->item(row), qmc2CursorPositioningMode);
		qmc2AudioLastIndividualTrack = psl.at(row);
	}
	checkBoxAudioPlayOnStart->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "AudioPlayer/PlayOnStart", false).toBool());
	checkBoxAudioShuffle->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "AudioPlayer/Shuffle", false).toBool());
	checkBoxAudioPause->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "AudioPlayer/Pause", true).toBool());
	checkBoxAudioFade->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "AudioPlayer/Fade", true).toBool());
	dialAudioVolume->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + "AudioPlayer/Volume", 50).toInt());
	mediaPlayer->setVolume(dialAudioVolume->value());
	toolButtonAudioPreviousTrack->setDefaultAction(actionAudioPreviousTrack);
	toolButtonAudioNextTrack->setDefaultAction(actionAudioNextTrack);
	toolButtonAudioStopTrack->setDefaultAction(actionAudioStopTrack);
	toolButtonAudioPauseTrack->setDefaultAction(actionAudioPauseTrack);
	toolButtonAudioPlayTrack->setDefaultAction(actionAudioPlayTrack);
	mediaPlayer->setNotifyInterval(1000);
	connect(mediaPlayer, SIGNAL(positionChanged(qint64)), this, SLOT(audioTick(qint64)));
	connect(mediaPlayer, SIGNAL(durationChanged(qint64)), this, SLOT(audioTotalTimeChanged(qint64)));
	connect(mediaPlayer, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(audioStateChanged(QMediaPlayer::MediaStatus)));
	connect(mediaPlayer, SIGNAL(metaDataChanged()), this, SLOT(audioMetaDataChanged()));
	connect(mediaPlayer, SIGNAL(bufferStatusChanged(int)), this, SLOT(audioBufferStatus(int)));
	audioFastForwarding = audioFastBackwarding = audioSkippingTracks = false;
	if ( checkBoxAudioPlayOnStart->isChecked() ) {
		audioState = QMediaPlayer::PlayingState;
		QTimer::singleShot(0, this, SLOT(on_actionAudioPlayTrack_triggered()));
	} else
		QTimer::singleShot(0, this, SLOT(on_actionAudioStopTrack_triggered()));
#endif
#endif

	// signal setup requests for style, style-sheet and palette
	signalStyleSetupRequested(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Style", "Default").toString());
	signalStyleSheetSetupRequested(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/StyleSheet", QString()).toString());
	signalPaletteSetupRequested(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Style", "Default").toString());

	// recreate all components
	foreach (QString component, qmc2ComponentSetup->components())
		qmc2ComponentSetup->saveComponent(component);

	if ( qmc2SplashScreen ) {
		qmc2SplashScreen->showMessage(tr("Welcome to QMC2 v%1!").arg(XSTR(QMC2_VERSION)), Qt::AlignHCenter | Qt::AlignBottom, Qt::white);
		QTimer::singleShot(QMC2_SPLASH_DURATION, qmc2SplashScreen, SLOT(hide()));
	}

	// tool-bar customization
	qmc2ToolBarCustomizer = new ToolBarCustomizer(qmc2Options);

#if defined(QMC2_OS_MAC)
	qmc2Options->setParent(this, Qt::Dialog);
#endif

#if QMC2_USE_PHONON_API
	QTimer::singleShot(0, this, SLOT(on_toolButtonAudioSetupEffects_clicked()));
#endif

	qmc2GhostImagePixmap.load(":/data/img/ghost.png");
	qmc2GhostImagePixmap.isGhost = true;

	qmc2LastListIndex = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/MachineListTab", 0).toInt();
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
	if ( qmc2LastListIndex == QMC2_EMBED_INDEX )
		qmc2LastListIndex = 0;
#endif
	tabWidgetMachineList->blockSignals(true);
	tabWidgetMachineList->setCurrentIndex(qmc2LastListIndex);
	tabWidgetMachineList->blockSignals(false);
	on_tabWidgetMachineList_currentChanged(qmc2LastListIndex);
	floatToggleButtonSoftwareDetail_toggled(floatToggleButtonSoftwareDetail->isChecked());
	ImageWidget::reloadArtworkFormats();
	qmc2EarlyStartup = false;

	// hide machine list / show loading animation initially
	treeWidgetMachineList->setVisible(false);
	((AspectRatioLabel *)labelLoadingMachineList)->setLabelText(tr("Loading, please wait..."));
	labelLoadingMachineList->setVisible(true);
	treeWidgetHierarchy->setVisible(false);
	((AspectRatioLabel *)labelLoadingHierarchy)->setLabelText(tr("Loading, please wait..."));
	labelLoadingHierarchy->setVisible(true);
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ShowLoadingAnimation", true).toBool() )
		loadAnimMovie->start();

	// restore machine search history
	QStringList machineSearchHistory(qmc2Config->value(QMC2_FRONTEND_PREFIX + "MachineSearchHistory", QStringList()).toStringList());
	machineSearchHistory.removeDuplicates();
	while ( machineSearchHistory.count() > QMC2_MACHINE_SEARCH_HISTORY_LENGTH )
		machineSearchHistory.removeLast();
	comboBoxToolbarSearch->blockSignals(true);
	comboBoxToolbarSearch->addItems(machineSearchHistory);
	comboBoxToolbarSearch->lineEdit()->setText(QString());
	comboBoxToolbarSearch->blockSignals(false);
	comboBoxSearch->blockSignals(true);
	comboBoxSearch->addItems(machineSearchHistory);
	comboBoxSearch->lineEdit()->setText(QString());
	comboBoxSearch->blockSignals(false);

	// disable the system manual actions initially
	checkSystemManualAvailability();

	// process the queued events
	setUpdatesEnabled(true);
	setVisible(true);
	qApp->processEvents();

	// make sure the logs are scrolled to their maxima
	logScrollToEnd(QMC2_LOG_FRONTEND);
	logScrollToEnd(QMC2_LOG_EMULATOR);

	// trigger initial load
	QTimer::singleShot(0, this, SLOT(updateTabWidgets()));
	QTimer::singleShot(0, this, SLOT(on_actionReload_triggered()));

	activityCheckTimer.start(QMC2_ACTIVITY_CHECK_INTERVAL);
}

void MainWindow::setupStyle(QString styleName)
{
	if ( qmc2Options->currentStyleName() == styleName )
		return;
	QStyle *newStyle = 0;
	if ( styleName != "Default" ) {
		if ( QStyleFactory::keys().contains(styleName) )
			newStyle = QStyleFactory::create(styleName);
		else {
			newStyle = QStyleFactory::create(defaultStyle);
			styleName = "Default";
		}
	} else {
		newStyle = QStyleFactory::create(defaultStyle);
		styleName = "Default";
	}
	if ( newStyle ) {
		log(QMC2_LOG_FRONTEND, tr("setting GUI style to '%1'").arg(styleName));
		if ( !qmc2StandardPalettes.contains(styleName) )
			qmc2StandardPalettes[styleName] = newStyle->standardPalette();
		if ( !proxyStyle ) {
			proxyStyle = new ProxyStyle;
			proxyStyle->setBaseStyle(newStyle);
			qApp->setStyle(proxyStyle);
		} else {
			QStyle *oldBaseStyle = proxyStyle->baseStyle();
			proxyStyle->setBaseStyle(newStyle);
			delete oldBaseStyle;
		}
		qmc2Options->setCurrentStyleName(styleName);
	} else
		log(QMC2_LOG_FRONTEND, tr("WARNING: GUI style '%1' not found").arg(styleName));
}

void MainWindow::setupStyleSheet(QString styleSheetName)
{
	static QString oldStyleSheetName;
	if ( !styleSheetName.isEmpty() ) {
		QFile f(styleSheetName);
		if ( f.open(QIODevice::ReadOnly) ) {
			if ( styleSheetName != oldStyleSheetName )
				log(QMC2_LOG_FRONTEND, tr("loading style sheet '%1'").arg(styleSheetName));
			if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/SetWorkDirFromExec", false).toBool() )
				QDir::setCurrent(QFileInfo(f).absolutePath());
			qApp->setStyleSheet(f.readAll());
			f.close();
		} else
			log(QMC2_LOG_FRONTEND, tr("WARNING: can't open style sheet '%1'").arg(styleSheetName));
	} else {
		if ( !qApp->styleSheet().isEmpty() )
			log(QMC2_LOG_FRONTEND, tr("removing current style sheet"));
		qApp->setStyleSheet(QString());
	}
	oldStyleSheetName = styleSheetName;
}

void MainWindow::setupPalette(QString styleName)
{
	static QPalette oldPalette;
	qmc2Options->loadCustomPalette(styleName);
	QPalette newPalette;
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/StandardColorPalette", true).toBool() ) {
		newPalette = qmc2StandardPalettes[styleName];
		if ( oldPalette != newPalette )
			log(QMC2_LOG_FRONTEND, tr("using default color palette for GUI style '%1'").arg(styleName));
	} else {
		newPalette = qmc2CustomPalette;
		if ( oldPalette != newPalette )
			log(QMC2_LOG_FRONTEND, tr("using custom color palette"));
	}
	qApp->setPalette(newPalette);
	// work around for an annoying Qt bug
	menuBar()->setPalette(newPalette);
	toolbar->setPalette(newPalette);
	oldPalette = newPalette;
}

void MainWindow::viewFullDetail()
{
	ComponentInfo *componentInfo = qmc2ComponentSetup->componentInfoHash().value("Component1");
	int index = componentInfo->appliedFeatureList().indexOf(QMC2_MACHINELIST_INDEX);
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
	int embedIndex = componentInfo->appliedFeatureList().indexOf(QMC2_EMBED_INDEX);
	if ( index > 0 && embedIndex >= 0 && embedIndex <= index )
		if ( tabWidgetMachineList->indexOf(tabEmbeddedEmus) < 0 )
			index--;
#endif
	int foreignIndex = componentInfo->appliedFeatureList().indexOf(QMC2_FOREIGN_INDEX);
	if ( index > 0 && foreignIndex >= 0 && foreignIndex <= index )
		if ( tabWidgetMachineList->indexOf(tabForeignEmulators) < 0 )
			index--;
	stackedWidgetView->setCurrentIndex(QMC2_VIEWMACHINELIST_INDEX);
	comboBoxViewSelect->blockSignals(true);
	comboBoxViewSelect->setCurrentIndex(QMC2_VIEWMACHINELIST_INDEX);
	comboBoxViewSelect->blockSignals(false);
	tabWidgetMachineList->setCurrentIndex(index);
	tabWidgetMachineList->setTabIcon(index, QIcon(QString::fromUtf8(":/data/img/flat.png")));
	menuView->setIcon(QIcon(QString::fromUtf8(":/data/img/flat.png")));
	treeWidgetMachineList->setFocus();
	stackedWidgetView->update();
	qApp->processEvents();
}

void MainWindow::viewParentClones()
{
	ComponentInfo *componentInfo = qmc2ComponentSetup->componentInfoHash().value("Component1");
	int index = componentInfo->appliedFeatureList().indexOf(QMC2_MACHINELIST_INDEX);
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
	int embedIndex = componentInfo->appliedFeatureList().indexOf(QMC2_EMBED_INDEX);
	if ( index > 0 && embedIndex >= 0 && embedIndex <= index )
		if ( tabWidgetMachineList->indexOf(tabEmbeddedEmus) < 0 )
			index--;
#endif
	int foreignIndex = componentInfo->appliedFeatureList().indexOf(QMC2_FOREIGN_INDEX);
	if ( index > 0 && foreignIndex >= 0 && foreignIndex <= index )
		if ( tabWidgetMachineList->indexOf(tabForeignEmulators) < 0 )
			index--;
	stackedWidgetView->setCurrentIndex(QMC2_VIEWHIERARCHY_INDEX);
	comboBoxViewSelect->blockSignals(true);
	comboBoxViewSelect->setCurrentIndex(QMC2_VIEWHIERARCHY_INDEX);
	comboBoxViewSelect->blockSignals(false);
	tabWidgetMachineList->setCurrentIndex(index);
	tabWidgetMachineList->setTabIcon(index, QIcon(QString::fromUtf8(":/data/img/clone.png")));
	menuView->setIcon(QIcon(QString::fromUtf8(":/data/img/clone.png")));
	treeWidgetHierarchy->setFocus();
	stackedWidgetView->update();
	qApp->processEvents();
}

void MainWindow::viewByCategory()
{
	ComponentInfo *componentInfo = qmc2ComponentSetup->componentInfoHash().value("Component1");
	int index = componentInfo->appliedFeatureList().indexOf(QMC2_MACHINELIST_INDEX);
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
	int embedIndex = componentInfo->appliedFeatureList().indexOf(QMC2_EMBED_INDEX);
	if ( index > 0 && embedIndex >= 0 && embedIndex <= index )
		if ( tabWidgetMachineList->indexOf(tabEmbeddedEmus) < 0 )
			index--;
#endif
	int foreignIndex = componentInfo->appliedFeatureList().indexOf(QMC2_FOREIGN_INDEX);
	if ( index > 0 && foreignIndex >= 0 && foreignIndex <= index )
		if ( tabWidgetMachineList->indexOf(tabForeignEmulators) < 0 )
			index--;
	stackedWidgetView->setCurrentIndex(QMC2_VIEWCATEGORY_INDEX);
	comboBoxViewSelect->blockSignals(true);
	comboBoxViewSelect->setCurrentIndex(QMC2_VIEWCATEGORY_INDEX);
	comboBoxViewSelect->blockSignals(false);
	tabWidgetMachineList->setCurrentIndex(index);
	tabWidgetMachineList->setTabIcon(index, QIcon(QString::fromUtf8(":/data/img/category.png")));
	menuView->setIcon(QIcon(QString::fromUtf8(":/data/img/category.png")));
	QTreeWidgetItem *item = treeWidgetCategoryView->topLevelItem(0);
	if ( item ) {
		if ( item->text(QMC2_MACHINELIST_COLUMN_MACHINE) == MachineList::trWaitingForData ) {
			treeWidgetCategoryView->setVisible(false);
			((AspectRatioLabel *)labelCreatingCategoryView)->setLabelText(tr("Loading, please wait..."));
			labelCreatingCategoryView->setVisible(true);
			QTimer::singleShot(0, qmc2MachineList, SLOT(createCategoryView()));
		} else
			QTimer::singleShot(QMC2_RANK_UPDATE_DELAY, this, SLOT(treeWidgetCategoryView_verticalScrollChanged()));
	} else {
		treeWidgetCategoryView->setVisible(false);
		((AspectRatioLabel *)labelCreatingCategoryView)->setLabelText(tr("Loading, please wait..."));
		labelCreatingCategoryView->setVisible(true);
		QTimer::singleShot(0, qmc2MachineList, SLOT(createCategoryView()));
	}
	treeWidgetCategoryView->setFocus();
	stackedWidgetView->update();
	qApp->processEvents();
}

void MainWindow::viewByVersion()
{
	ComponentInfo *componentInfo = qmc2ComponentSetup->componentInfoHash().value("Component1");
	int index = componentInfo->appliedFeatureList().indexOf(QMC2_MACHINELIST_INDEX);
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
	int embedIndex = componentInfo->appliedFeatureList().indexOf(QMC2_EMBED_INDEX);
	if ( index > 0 && embedIndex >= 0 && embedIndex <= index )
		if ( tabWidgetMachineList->indexOf(tabEmbeddedEmus) < 0 )
			index--;
#endif
	int foreignIndex = componentInfo->appliedFeatureList().indexOf(QMC2_FOREIGN_INDEX);
	if ( index > 0 && foreignIndex >= 0 && foreignIndex <= index )
		if ( tabWidgetMachineList->indexOf(tabForeignEmulators) < 0 )
			index--;
	stackedWidgetView->setCurrentIndex(QMC2_VIEWVERSION_INDEX);
	comboBoxViewSelect->blockSignals(true);
	comboBoxViewSelect->setCurrentIndex(QMC2_VIEWVERSION_INDEX);
	comboBoxViewSelect->blockSignals(false);
	tabWidgetMachineList->setCurrentIndex(index);
	tabWidgetMachineList->setTabIcon(index, QIcon(QString::fromUtf8(":/data/img/version.png")));
	menuView->setIcon(QIcon(QString::fromUtf8(":/data/img/version.png")));
	QTreeWidgetItem *item = treeWidgetVersionView->topLevelItem(0);
	if ( item ) {
		if ( item->text(QMC2_MACHINELIST_COLUMN_MACHINE) == MachineList::trWaitingForData ) {
			treeWidgetVersionView->setVisible(false);
			((AspectRatioLabel *)labelCreatingVersionView)->setLabelText(tr("Loading, please wait..."));
			labelCreatingVersionView->setVisible(true);
			QTimer::singleShot(0, qmc2MachineList, SLOT(createVersionView()));
		} else
			QTimer::singleShot(QMC2_RANK_UPDATE_DELAY, this, SLOT(treeWidgetVersionView_verticalScrollChanged()));
	} else {
		treeWidgetVersionView->setVisible(false);
		((AspectRatioLabel *)labelCreatingVersionView)->setLabelText(tr("Loading, please wait..."));
		labelCreatingVersionView->setVisible(true);
		QTimer::singleShot(0, qmc2MachineList, SLOT(createVersionView()));
	}
	treeWidgetVersionView->setFocus();
	stackedWidgetView->update();
	qApp->processEvents();
}

bool MainEventFilter::eventFilter(QObject *object, QEvent *event)
{
	switch ( event->type() ) {
		case QEvent::KeyPress:
		case QEvent::KeyRelease: {
			QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
			if ( keyEvent->text() == QString("QMC2_EMULATED_KEY") ) {
				QString emulatedSequence = QKeySequence(keyEvent->key() | keyEvent->modifiers()).toString();
#ifdef QMC2_DEBUG
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: emulated key event, key-sequence = '%1'").arg(emulatedSequence));
#endif
				QPair<QString, QAction *> actionPair = qmc2ShortcutHash[emulatedSequence];
				if ( actionPair.second ) {
					if ( event->type() == QEvent::KeyPress && actionPair.second->isEnabled() )
						actionPair.second->trigger();
					return true;
				} else
					return false;
			}
			int myKeySeq = 0;
			if ( keyEvent->modifiers() & Qt::ShiftModifier )
				myKeySeq += Qt::SHIFT;
			if ( keyEvent->modifiers() & Qt::ControlModifier )
				myKeySeq += Qt::CTRL;
			if ( keyEvent->modifiers() & Qt::AltModifier )
				myKeySeq += Qt::ALT;
			if ( keyEvent->modifiers() & Qt::MetaModifier )
				myKeySeq += Qt::META;
			myKeySeq += keyEvent->key();
			QString pressedKeySeq = QKeySequence(myKeySeq).toString();
#ifdef QMC2_DEBUG
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: current key-sequence = '%1'").arg(pressedKeySeq));
#endif
			QString matchedKeySeq = qmc2CustomShortcutHash.key(pressedKeySeq);
			if ( !matchedKeySeq.isEmpty() ) {
				if ( !qmc2MainWindow->menuBar()->isVisible() ) {
					QPair<QString, QAction *> actionPair = qmc2ShortcutHash[qmc2CustomShortcutHash.key(pressedKeySeq)];
					if ( actionPair.second )
					{
#ifdef QMC2_DEBUG
						qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: '%1' pressed, emulating key event (due to no menu bar)").arg(pressedKeySeq));
#endif
						if ( actionPair.second->isEnabled() )
							actionPair.second->trigger();
						return true;
					}
				}
				if ( matchedKeySeq != pressedKeySeq ) {
#ifdef QMC2_DEBUG
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: emulating key event for '%1'").arg(matchedKeySeq));
#endif
					// emulate a key event for the mapped key
					int key = 0;
					if ( qmc2QtKeyHash.contains(matchedKeySeq) )
						key = qmc2QtKeyHash[matchedKeySeq][0] | qmc2QtKeyHash[matchedKeySeq][1] | qmc2QtKeyHash[matchedKeySeq][2] | qmc2QtKeyHash[matchedKeySeq][3];
					else {
						QKeySequence emulatedKeySequence(matchedKeySeq);
						key = emulatedKeySequence[0] | emulatedKeySequence[1] | emulatedKeySequence[2] | emulatedKeySequence[3];
					}

					Qt::KeyboardModifiers mods = Qt::NoModifier;
					if ( key & Qt::ShiftModifier ) {
						key -= Qt::ShiftModifier;
						mods |= Qt::ShiftModifier;
					}
					if ( key & Qt::ControlModifier ) {
						key -= Qt::ControlModifier;
						mods |= Qt::ControlModifier;
					}
					if ( key & Qt::AltModifier ) {
						key -= Qt::AltModifier;
						mods |= Qt::AltModifier;
					}
					if ( key & Qt::MetaModifier ) {
						key -= Qt::MetaModifier;
						mods |= Qt::MetaModifier;
					}
					QKeyEvent *emulatedKeyEvent = new QKeyEvent(event->type(), key, mods, QString("QMC2_EMULATED_KEY"));
					qApp->postEvent(object, emulatedKeyEvent);
					return true;
				} else {
#ifdef QMC2_DEBUG
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: '%1' pressed, default key event processing").arg(pressedKeySeq));
#endif
					// default key event processing
					return false;
				}
			}
			QHash<QString, QString>::const_iterator it = qmc2CustomShortcutHash.find(pressedKeySeq);
			if ( it != qmc2CustomShortcutHash.end() ) {
				if ( !it.value().isEmpty() ) {
#ifdef QMC2_DEBUG
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: '%1' pressed, key event suppressed").arg(pressedKeySeq));
#endif
					return true;
				} else {
#ifdef QMC2_DEBUG
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: '%1' pressed, default key event processing").arg(pressedKeySeq));
#endif
					return false;
				}
			}
#ifdef QMC2_DEBUG
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: '%1' pressed, default key event processing").arg(pressedKeySeq));
#endif
			return false;
		}
		default:
			// default event processing
			return QObject::eventFilter(object, event);
	}
}

#if defined(QMC2_YOUTUBE_ENABLED)
void MainWindow::loadYouTubeVideoInfoMap()
{
	log(QMC2_LOG_FRONTEND, tr("loading YouTube video info map"));
	QDir youTubeCacheDir(qmc2Config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/CacheDirectory").toString());
	if ( youTubeCacheDir.exists() ) {
#if defined(QMC2_SDLMAME)
		QFile f(youTubeCacheDir.canonicalPath() + "/qmc2-sdlmame.yti");
#elif defined(QMC2_MAME)
		QFile f(youTubeCacheDir.canonicalPath() + "/qmc2-mame.yti");
#else
		QFile f(youTubeCacheDir.canonicalPath() + "/qmc2.yti");
#endif
		if ( f.open(QIODevice::ReadOnly | QIODevice::Text) ) {
			QString oldFormat = progressBarMachineList->format();
			int oldMinimum = progressBarMachineList->minimum();
			int oldMaximum = progressBarMachineList->maximum();
			int oldValue = progressBarMachineList->value();
			if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
				progressBarMachineList->setFormat(tr("YouTube index - %p%"));
			else
				progressBarMachineList->setFormat("%p%");
			QFileInfo fi(f.fileName());
			progressBarMachineList->setRange(0, fi.size());
			progressBarMachineList->setValue(0);
			qmc2YouTubeVideoInfoHash.clear();
			QTextStream ts(&f);
			quint64 viCounter = 0;
			quint64 curLen = 0;
			while ( !ts.atEnd() ) {
				QString line = ts.readLine();
#if defined(QMC2_OS_WIN)
				curLen += line.length() + 2; // + 0x0d 0x0a
#else
				curLen += line.length() + 1; // + 0x0a
#endif
				if ( viCounter++ % QMC2_YOUTUBE_VIDEO_INFO_RSP == 0 )
					progressBarMachineList->setValue(curLen);
				if ( !line.startsWith("#") ) {
					QStringList tokens = line.split("\t");
					if ( tokens.count() > 2 )
						qmc2YouTubeVideoInfoHash[tokens[0]] = YouTubeVideoInfo(tokens[2], tokens[1]);
				}
			}
			progressBarMachineList->reset();
			progressBarMachineList->setFormat(oldFormat);
			progressBarMachineList->setRange(oldMinimum, oldMaximum);
			progressBarMachineList->setValue(oldValue);
		}
	}
	log(QMC2_LOG_FRONTEND, tr("done (loading YouTube video info map)"));
	log(QMC2_LOG_FRONTEND, tr("%n video info record(s) loaded", "", qmc2YouTubeVideoInfoHash.count()));
	qmc2YouTubeVideoInfoHashChanged = false;
}
#endif

void MainWindow::loadMachineInfoDB()
{
	QStringList pathList;
	QStringList emulatorList;
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMameHistoryDat").toBool() ) {
		pathList << qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MameHistoryDat").toString();
		emulatorList << "MAME";
	}
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMessSysinfoDat").toBool() ) {
		pathList << qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MessSysinfoDat").toString();
		if ( !pathList.last().toLower().endsWith("sysinfo.dat") )
			emulatorList << "MAME";
		else
			emulatorList << "MESS";
	}

	if ( qmc2MachineList->datInfoDb()->machineInfoImportRequired(pathList) ) {
		qmc2LoadingMachineInfoDB = true;
		qmc2Options->toolButtonImportGameInfo->setEnabled(false);
		qmc2Options->toolButtonImportMachineInfo->setEnabled(false);
		qmc2MachineList->datInfoDb()->importMachineInfo(pathList, emulatorList);
		qmc2Options->toolButtonImportGameInfo->setEnabled(true);
		qmc2Options->toolButtonImportMachineInfo->setEnabled(true);
		qmc2LoadingMachineInfoDB = false;
	}
}

void MainWindow::loadEmuInfoDB()
{
	QStringList pathList;
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMameInfoDat").toBool() )
		pathList << qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MameInfoDat").toString();
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMessInfoDat").toBool() )
		pathList << qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MessInfoDat").toString();

	if ( qmc2MachineList->datInfoDb()->emuInfoImportRequired(pathList) ) {
		qmc2LoadingEmuInfoDB = true;
		qmc2Options->toolButtonImportMameInfo->setEnabled(false);
		qmc2Options->toolButtonImportMessInfo->setEnabled(false);
		qmc2MachineList->datInfoDb()->importEmuInfo(pathList);
		qmc2Options->toolButtonImportMameInfo->setEnabled(true);
		qmc2Options->toolButtonImportMessInfo->setEnabled(true);
		qmc2LoadingEmuInfoDB = false;
	}
}

void MainWindow::loadSoftwareInfoDB()
{
	QStringList pathList = QStringList() << qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareInfoDB").toString();
	if ( qmc2MachineList->datInfoDb()->softwareInfoImportRequired(pathList) ) {
		qmc2LoadingSoftwareInfoDB = true;
		qmc2Options->toolButtonImportSoftwareInfo->setEnabled(false);
		qmc2MachineList->datInfoDb()->importSoftwareInfo(pathList);
		qmc2Options->toolButtonImportSoftwareInfo->setEnabled(true);
		qmc2LoadingSoftwareInfoDB = false;
	}
}

#if QMC2_USE_PHONON_API || QMC2_MULTIMEDIA_ENABLED
void MainWindow::on_actionAudioPreviousTrack_triggered(bool /*checked*/)
{
	toolButtonAudioPreviousTrack->setDown(true);
	QTimer::singleShot(QMC2_BUTTON_ANIMATION_TIMEOUT, this, SLOT(toolButtonAudioPreviousTrack_resetButton()));
	audioFastForwarding = audioFastBackwarding = false;
	audioSkippingTracks = true;

	if ( listWidgetAudioPlaylist->count() > 0 ) {
		QList<QListWidgetItem *> sl = listWidgetAudioPlaylist->selectedItems();
		QListWidgetItem *ci = 0;
		if ( sl.count() > 0 )
			ci = sl[0];
		if ( !ci )
			ci = listWidgetAudioPlaylist->currentItem();
		int row;
		if ( ci )
			row = listWidgetAudioPlaylist->currentRow() - 1;
		else
			row = listWidgetAudioPlaylist->count() - 1;
		if ( row < 0 )
			row = listWidgetAudioPlaylist->count() - 1;
		listWidgetAudioPlaylist->clearSelection();
		listWidgetAudioPlaylist->setCurrentRow(row);
		ci = listWidgetAudioPlaylist->currentItem();
		switch ( audioState ) {
#if QMC2_MULTIMEDIA_ENABLED
			case QMediaPlayer::PlayingState:
#else
			case Phonon::PlayingState:
#endif
				QTimer::singleShot(0, this, SLOT(on_actionAudioPlayTrack_triggered()));
				break;

			default:
				QTimer::singleShot(0, this, SLOT(on_actionAudioStopTrack_triggered()));
				break;
		}
	}
}

void MainWindow::toolButtonAudioPreviousTrack_resetButton()
{
	toolButtonAudioPreviousTrack->setDown(false);
}

void MainWindow::on_actionAudioNextTrack_triggered(bool /*checked*/)
{
	toolButtonAudioNextTrack->setDown(true);
	QTimer::singleShot(QMC2_BUTTON_ANIMATION_TIMEOUT, this, SLOT(toolButtonAudioNextTrack_resetButton()));
	audioFastForwarding = audioFastBackwarding = false;
	audioSkippingTracks = true;

	if ( listWidgetAudioPlaylist->count() > 0 ) {
		QList<QListWidgetItem *> sl = listWidgetAudioPlaylist->selectedItems();
		QListWidgetItem *ci = 0;
		if ( sl.count() > 0 )
			ci = sl[0];  
		if ( !ci )
			ci = listWidgetAudioPlaylist->currentItem();
		int row;
		if ( ci )
			row = listWidgetAudioPlaylist->currentRow() + 1;
		else
			row = 0;
		if ( row > listWidgetAudioPlaylist->count() - 1 )
			row = 0;
		listWidgetAudioPlaylist->clearSelection();
		listWidgetAudioPlaylist->setCurrentRow(row);
		ci = listWidgetAudioPlaylist->currentItem();
		switch ( audioState ) {
#if QMC2_MULTIMEDIA_ENABLED
			case QMediaPlayer::PlayingState:
#else
			case Phonon::PlayingState:
#endif
				QTimer::singleShot(0, this, SLOT(on_actionAudioPlayTrack_triggered()));
				break;

			default:
				QTimer::singleShot(0, this, SLOT(on_actionAudioStopTrack_triggered()));
				break;
		}
	}
}

void MainWindow::toolButtonAudioNextTrack_resetButton()
{
	toolButtonAudioNextTrack->setDown(false);
}

void MainWindow::on_actionAudioFastBackward_triggered(bool checked)
{
	toolButtonAudioFastBackward->setDown(true);
	QTimer::singleShot(QMC2_BUTTON_ANIMATION_TIMEOUT, this, SLOT(toolButtonAudioFastBackward_resetButton()));

	on_toolButtonAudioFastBackward_clicked(checked);
}

void MainWindow::on_toolButtonAudioFastBackward_clicked(bool /*checked*/)
{
#if QMC2_MULTIMEDIA_ENABLED
	qint64 newTime = mediaPlayer->position();
#else
	qint64 newTime = phononAudioPlayer->currentTime();
#endif
	if ( newTime > 0 ) {
		newTime -= QMC2_AUDIOPLAYER_SEEK_OFFSET;
		audioFastBackwarding = true;
#if QMC2_MULTIMEDIA_ENABLED
		mediaPlayer->setPosition(newTime);
#else
		phononAudioPlayer->seek(newTime);
#endif
		audioTick(newTime);
	}
}

void MainWindow::toolButtonAudioFastBackward_resetButton()
{
	toolButtonAudioFastBackward->setDown(false);
	audioFastForwarding = false;
}

void MainWindow::on_actionAudioFastForward_triggered(bool checked)
{
	toolButtonAudioFastForward->setDown(true);
	QTimer::singleShot(QMC2_BUTTON_ANIMATION_TIMEOUT, this, SLOT(toolButtonAudioFastForward_resetButton()));

	on_toolButtonAudioFastForward_clicked(checked);
}

void MainWindow::on_toolButtonAudioFastForward_clicked(bool /*checked*/)
{
#if QMC2_MULTIMEDIA_ENABLED
	qint64 newTime = mediaPlayer->position();
#else
	qint64 newTime = phononAudioPlayer->currentTime();
#endif
	if ( newTime > 0 ) {
		newTime += QMC2_AUDIOPLAYER_SEEK_OFFSET;
		audioFastForwarding = true;
#if QMC2_MULTIMEDIA_ENABLED
		mediaPlayer->setPosition(newTime);
#else
		phononAudioPlayer->seek(newTime);
#endif
		audioTick(newTime);
	}
}

void MainWindow::toolButtonAudioFastForward_resetButton()
{
	toolButtonAudioFastForward->setDown(false);
	audioFastForwarding = false;
}

void MainWindow::on_actionAudioStopTrack_triggered(bool /*checked*/)
{
	actionAudioStopTrack->setChecked(true);
	actionAudioPauseTrack->setChecked(false);
	actionAudioPlayTrack->setChecked(false);
	audioFastForwarding = audioFastBackwarding = audioSkippingTracks = false;
#if QMC2_MULTIMEDIA_ENABLED
	mediaPlayer->stop();
	audioState = QMediaPlayer::StoppedState;
#else
	phononAudioPlayer->stop();
	audioState = Phonon::StoppedState;
#endif
	progressBarAudioProgress->setFormat(QString());
	progressBarAudioProgress->setRange(0, 100);
	progressBarAudioProgress->setValue(0);
	progressBarAudioProgress->reset();
}

void MainWindow::on_actionAudioPauseTrack_triggered(bool /*checked*/)
{
	actionAudioPauseTrack->setChecked(true);
	actionAudioStopTrack->setChecked(false);
	actionAudioPlayTrack->setChecked(false);
	audioFastForwarding = audioFastBackwarding = audioSkippingTracks = false;
#if QMC2_MULTIMEDIA_ENABLED
	if ( checkBoxAudioFade->isChecked() && audioState == QMediaPlayer::PlayingState )
		audioFade(QMC2_AUDIOPLAYER_FADER_PAUSE);
	else
		mediaPlayer->pause();
	audioState = QMediaPlayer::PausedState;
#else
	if ( checkBoxAudioFade->isChecked() && audioState == Phonon::PlayingState )
		audioFade(QMC2_AUDIOPLAYER_FADER_PAUSE);
	else
		phononAudioPlayer->pause();
	audioState = Phonon::PausedState;
#endif
}

void MainWindow::on_actionAudioPlayTrack_triggered(bool /*checked*/)
{
#if QMC2_MULTIMEDIA_ENABLED
	// if this is a URL media source, force a reconnect to the stream...
	if ( mediaPlayer->currentMedia().canonicalUrl().scheme() != "file" )
		mediaPlayer->setMedia(mediaPlayer->currentMedia().canonicalUrl());

	static QString audioPlayerCurrentTrack;
	audioFastForwarding = audioFastBackwarding = false;
	if ( audioState == QMediaPlayer::PausedState ) {
		if ( qmc2ProcessManager->sentPlaySignal && qmc2ProcessManager->procMap.count() > 0 ) {
			qmc2ProcessManager->musicWasPlaying = true;
		} else if ( checkBoxAudioFade->isChecked() ) {
			audioFade(QMC2_AUDIOPLAYER_FADER_PLAY);
		} else {
			mediaPlayer->play();
			actionAudioPlayTrack->setChecked(true);
			actionAudioStopTrack->setChecked(false);
			actionAudioPauseTrack->setChecked(false);
		}
		qmc2ProcessManager->sentPlaySignal = false;
		audioState = QMediaPlayer::PlayingState;
	} else if ( listWidgetAudioPlaylist->count() > 0 ) {
		QList<QListWidgetItem *> sl = listWidgetAudioPlaylist->selectedItems();
		QListWidgetItem *ci = 0;
		if ( sl.count() > 0 )
			ci = sl[0];
		if ( !ci ) {
			if ( !qmc2AudioLastIndividualTrack.isEmpty() ) {
				audioScrollToCurrentItem();
				ci = listWidgetAudioPlaylist->currentItem();
			}
			if ( !ci ) {
				listWidgetAudioPlaylist->setCurrentRow(0);
				ci = listWidgetAudioPlaylist->currentItem();
			}
		}
		if ( ci->text() != audioPlayerCurrentTrack ) {
			progressBarAudioProgress->reset();
			audioPlayerCurrentTrack = ci->text();
			listWidgetAudioPlaylist->scrollToItem(ci, qmc2CursorPositioningMode);
			if ( QFileInfo(audioPlayerCurrentTrack).exists() )
				mediaPlayer->setMedia(QUrl::fromLocalFile(audioPlayerCurrentTrack));
			else
				mediaPlayer->setMedia(QUrl::fromUserInput(audioPlayerCurrentTrack));
		}
		mediaPlayer->play();
		actionAudioPlayTrack->setChecked(true);
		actionAudioStopTrack->setChecked(false);
		actionAudioPauseTrack->setChecked(false);
		audioState = QMediaPlayer::PlayingState;
	} else
		on_actionAudioStopTrack_triggered(true);
#else
	// if this is a URL media source, force a reconnect to the stream...
	if ( phononAudioPlayer->currentSource().type() == Phonon::MediaSource::Url )
		phononAudioPlayer->setCurrentSource(phononAudioPlayer->currentSource().url());

	static QString audioPlayerCurrentTrack;
	audioFastForwarding = audioFastBackwarding = false;
	if ( audioState == Phonon::PausedState ) {
		if ( qmc2ProcessManager->sentPlaySignal && qmc2ProcessManager->procMap.count() > 0 ) {
			qmc2ProcessManager->musicWasPlaying = true;
		} else if ( checkBoxAudioFade->isChecked() ) {
			audioFade(QMC2_AUDIOPLAYER_FADER_PLAY);
		} else {
			phononAudioPlayer->play();
			actionAudioPlayTrack->setChecked(true);
			actionAudioStopTrack->setChecked(false);
			actionAudioPauseTrack->setChecked(false);
		}
		qmc2ProcessManager->sentPlaySignal = false;
		audioState = Phonon::PlayingState;
	} else if ( listWidgetAudioPlaylist->count() > 0 ) {
		QList<QListWidgetItem *> sl = listWidgetAudioPlaylist->selectedItems();
		QListWidgetItem *ci = 0;
		if ( sl.count() > 0 )
			ci = sl[0];
		if ( !ci ) {
			if ( !qmc2AudioLastIndividualTrack.isEmpty() ) {
				audioScrollToCurrentItem();
				ci = listWidgetAudioPlaylist->currentItem();
			}
			if ( !ci ) {
				listWidgetAudioPlaylist->setCurrentRow(0);
				ci = listWidgetAudioPlaylist->currentItem();
			}
		}
		if ( ci->text() != audioPlayerCurrentTrack ) {
			progressBarAudioProgress->reset();
			audioPlayerCurrentTrack = ci->text();
			listWidgetAudioPlaylist->scrollToItem(ci, qmc2CursorPositioningMode);
			phononAudioPlayer->setCurrentSource(Phonon::MediaSource(audioPlayerCurrentTrack));
		}
		phononAudioPlayer->play();
		actionAudioPlayTrack->setChecked(true);
		actionAudioStopTrack->setChecked(false);
		actionAudioPauseTrack->setChecked(false);
		audioState = Phonon::PlayingState;
	} else
		on_actionAudioStopTrack_triggered(true);
#endif
}

void MainWindow::on_toolButtonAudioAddTracks_clicked()
{
	QStringList sl = QFileDialog::getOpenFileNames(this, tr("Select one or more audio files"), QString(), tr("All files (*)"), 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( sl.count() > 0 )
		listWidgetAudioPlaylist->addItems(sl);
}

void MainWindow::on_toolButtonAudioAddURL_clicked()
{
	bool ok;
	QString streamUrl = QInputDialog::getText(this, tr("Add URL"), tr("Enter valid MP3 stream URL:"), QLineEdit::Normal, "", &ok);
	if ( ok && !streamUrl.isEmpty() )
		listWidgetAudioPlaylist->addItem(streamUrl);
}

void MainWindow::on_toolButtonAudioSetupEffects_clicked()
{
#if !QMC2_MULTIMEDIA_ENABLED
	static bool audioSetupEffectsFirstCall = true;

	if ( !qmc2AudioEffectDialog )
		qmc2AudioEffectDialog = new AudioEffectDialog(this);

	if ( !audioSetupEffectsFirstCall ) {
		qmc2AudioEffectDialog->show();
		qmc2AudioEffectDialog->raise();
	}

	audioSetupEffectsFirstCall = false;
#endif
}

void MainWindow::on_toolButtonAudioRemoveTracks_clicked()
{
	QList<QListWidgetItem *> sl = listWidgetAudioPlaylist->selectedItems();
	foreach (QListWidgetItem *item, sl) {
		item = listWidgetAudioPlaylist->takeItem(listWidgetAudioPlaylist->row(item));
		delete item;
	}
}

void MainWindow::on_listWidgetAudioPlaylist_itemSelectionChanged()
{
	QList<QListWidgetItem *> sl = listWidgetAudioPlaylist->selectedItems();
	if ( sl.count() > 0 )
		toolButtonAudioRemoveTracks->setEnabled(true);
	else
		toolButtonAudioRemoveTracks->setEnabled(false);

	audioFastForwarding = audioFastBackwarding = audioSkippingTracks = false;

	if ( sl.count() == 1 && !audioSkippingTracks && !qmc2EarlyStartup ) {
		QListWidgetItem *ci = listWidgetAudioPlaylist->currentItem();
		switch ( audioState ) {
#if QMC2_MULTIMEDIA_ENABLED
			case QMediaPlayer::PlayingState:
#else
			case Phonon::PlayingState:
#endif
				if ( qmc2AudioLastIndividualTrack != sl[0]->text() && ci == sl[0] )
					QTimer::singleShot(0, this, SLOT(on_actionAudioPlayTrack_triggered()));
				break;

			default:
				QTimer::singleShot(0, this, SLOT(on_actionAudioStopTrack_triggered()));
				break;
		}
		if ( ci ) {
			if ( ci->text() == sl[0]->text() )
				qmc2AudioLastIndividualTrack = sl[0]->text();
		}
	} else if ( sl.count() <= 0 )
		QTimer::singleShot(0, this, SLOT(audioScrollToCurrentItem()));
}

void MainWindow::audioScrollToCurrentItem()
{
	if ( !qmc2AudioLastIndividualTrack.isEmpty() ) {
		QList<QListWidgetItem *> itemList = listWidgetAudioPlaylist->findItems(qmc2AudioLastIndividualTrack, Qt::MatchExactly);
		if ( itemList.count() > 0 ) {
			QListWidgetItem *item = itemList[0];
			listWidgetAudioPlaylist->scrollToItem(item, QAbstractItemView::PositionAtCenter);
			listWidgetAudioPlaylist->setCurrentItem(item, QItemSelectionModel::Clear | QItemSelectionModel::SelectCurrent);
		}
	}
}

void MainWindow::on_actionAudioRaiseVolume_triggered(bool /*checked*/)
{
	dialAudioVolume->setValue(dialAudioVolume->value() + dialAudioVolume->pageStep());
}

void MainWindow::on_actionAudioLowerVolume_triggered(bool /*checked*/)
{
	dialAudioVolume->setValue(dialAudioVolume->value() - dialAudioVolume->pageStep());
}

void MainWindow::on_dialAudioVolume_valueChanged(int value)
{
#if QMC2_MULTIMEDIA_ENABLED
	mediaPlayer->setVolume(value);
#else
	phononAudioOutput->setVolume((qreal)value/100.0);
#endif
}

void MainWindow::audioFinished()
{
	static QStringList shuffleSelectionList;

	if ( audioFastBackwarding )
		QTimer::singleShot(0, this, SLOT(on_actionAudioPreviousTrack_triggered()));
	else if ( audioFastForwarding )
		QTimer::singleShot(0, this, SLOT(on_actionAudioNextTrack_triggered()));
	else if ( checkBoxAudioShuffle->isChecked() ) {
		if ( shuffleSelectionList.count() >= listWidgetAudioPlaylist->count() )
			shuffleSelectionList.clear();
		int newTrackIndex = qrand() % listWidgetAudioPlaylist->count();
		while ( shuffleSelectionList.contains(listWidgetAudioPlaylist->item(newTrackIndex)->text()) ) {
			qApp->processEvents();
			newTrackIndex = qrand() % listWidgetAudioPlaylist->count();
		}
		shuffleSelectionList << listWidgetAudioPlaylist->item(newTrackIndex)->text();
		listWidgetAudioPlaylist->setCurrentRow(newTrackIndex);
		QTimer::singleShot(0, this, SLOT(on_actionAudioPlayTrack_triggered()));
	} else
		QTimer::singleShot(0, this, SLOT(on_actionAudioNextTrack_triggered()));
}

#if QMC2_MULTIMEDIA_ENABLED
void MainWindow::audioStateChanged(QMediaPlayer::MediaStatus mediaStatus)
{
	if ( mediaStatus == QMediaPlayer::EndOfMedia )
		audioFinished();
}
#endif

void MainWindow::audioTick(qint64 newTime)
{
#if QMC2_MULTIMEDIA_ENABLED
	if ( audioState != QMediaPlayer::StoppedState ) {
		progressBarAudioProgress->setFormat(tr("%vs (%ms total)"));
		progressBarAudioProgress->setValue(newTime/1000);
	}
#else
	if ( audioState != Phonon::StoppedState ) {
		progressBarAudioProgress->setFormat(tr("%vs (%ms total)"));
		progressBarAudioProgress->setValue(newTime/1000);
	}
#endif
}

void MainWindow::audioTotalTimeChanged(qint64 newTotalTime)
{
	if ( newTotalTime > 0 ) {
		progressBarAudioProgress->setFormat(tr("%vs (%ms total)"));
		progressBarAudioProgress->setRange(0, newTotalTime/1000);
#if QMC2_MULTIMEDIA_ENABLED
		progressBarAudioProgress->setValue(mediaPlayer->position()/1000);
#else
		progressBarAudioProgress->setValue(phononAudioPlayer->currentTime()/1000);
#endif
	} else {
		progressBarAudioProgress->setRange(0, 100);
		progressBarAudioProgress->setValue(0);
		progressBarAudioProgress->reset();
	}
}

void MainWindow::audioFade(int faderFunction)
{
	int currentVolume = dialAudioVolume->value();
	int updateCounter;
	double vol;
	double volStep = (double)currentVolume / (double)QMC2_AUDIOPLAYER_FADER_TIMEOUT;
	audioFastForwarding = audioFastBackwarding = audioSkippingTracks = false;
	switch ( faderFunction ) {
		case QMC2_AUDIOPLAYER_FADER_PAUSE:
			actionAudioPauseTrack->setChecked(true);
			actionAudioStopTrack->setChecked(false);
			actionAudioPlayTrack->setChecked(false);
			actionAudioPauseTrack->setEnabled(false);
			toolButtonAudioPauseTrack->setEnabled(false);
			actionAudioPlayTrack->setEnabled(false);
			toolButtonAudioPlayTrack->setEnabled(false);
			actionAudioStopTrack->setEnabled(false);
			toolButtonAudioStopTrack->setEnabled(false);
			updateCounter = 0;
			for (vol = currentVolume; vol > 0.0; vol -= volStep) {
				updateCounter++;
				dialAudioVolume->setValue((int)vol);
				if ( updateCounter % 10 == 0 )
					qApp->processEvents();
				QTest::qSleep(1);
			}
#if QMC2_MULTIMEDIA_ENABLED
			mediaPlayer->pause();
			audioState = QMediaPlayer::PausedState;
#else
			phononAudioPlayer->pause();
			audioState = Phonon::PausedState;
#endif
			qApp->processEvents();
			actionAudioPauseTrack->setEnabled(true);
			toolButtonAudioPauseTrack->setEnabled(true);
			actionAudioPlayTrack->setEnabled(true);
			toolButtonAudioPlayTrack->setEnabled(true);
			actionAudioStopTrack->setEnabled(true);
			toolButtonAudioStopTrack->setEnabled(true);
			break;

		case QMC2_AUDIOPLAYER_FADER_PLAY:
			dialAudioVolume->setValue(0);
			actionAudioPauseTrack->setChecked(false);
			actionAudioStopTrack->setChecked(false);
			actionAudioPlayTrack->setChecked(true);
			actionAudioPauseTrack->setEnabled(false);
			toolButtonAudioPauseTrack->setEnabled(false);
			actionAudioPlayTrack->setEnabled(false);
			toolButtonAudioPlayTrack->setEnabled(false);
			actionAudioStopTrack->setEnabled(false);
			toolButtonAudioStopTrack->setEnabled(false);
			qApp->processEvents();
#if QMC2_MULTIMEDIA_ENABLED
			mediaPlayer->play();
			audioState = QMediaPlayer::PlayingState;
#else
			phononAudioPlayer->play();
			audioState = Phonon::PlayingState;
#endif
			updateCounter = 0;
			for (vol = 0; vol <= currentVolume; vol += volStep) {
				updateCounter++;
				dialAudioVolume->setValue((int)vol);
				if ( updateCounter % 10 == 0 )
					qApp->processEvents();
				QTest::qSleep(1);
			}
			qApp->processEvents();
			actionAudioPauseTrack->setEnabled(true);
			toolButtonAudioPauseTrack->setEnabled(true);
			actionAudioPlayTrack->setEnabled(true);
			toolButtonAudioPlayTrack->setEnabled(true);
			actionAudioStopTrack->setEnabled(true);
			toolButtonAudioStopTrack->setEnabled(true);
			break;
	}
	dialAudioVolume->setValue(currentVolume);
}

void MainWindow::audioMetaDataChanged()
{
	static QString lastTrackInfo;
#if QMC2_MULTIMEDIA_ENABLED
	QString titleMetaData(mediaPlayer->metaData(QMediaMetaData::Title).toString());
	QString artistMetaData(mediaPlayer->metaData(QMediaMetaData::ContributingArtist).toStringList().join(" / "));
	QString albumMetaData(mediaPlayer->metaData(QMediaMetaData::AlbumTitle).toString());
	QString genreMetaData(mediaPlayer->metaData(QMediaMetaData::Genre).toString());
#else
	QString titleMetaData(phononAudioPlayer->metaData(Phonon::TitleMetaData).join(", "));
	QString artistMetaData(phononAudioPlayer->metaData(Phonon::ArtistMetaData).join(", "));
	QString albumMetaData(phononAudioPlayer->metaData(Phonon::AlbumMetaData).join(", "));
	QString genreMetaData(phononAudioPlayer->metaData(Phonon::GenreMetaData).join(", "));
#endif
  
	if ( titleMetaData.isEmpty() && artistMetaData.isEmpty() && albumMetaData.isEmpty() && genreMetaData.isEmpty() )
		return;
	QString trackInfo = tr("audio player: track info: title = '%1', artist = '%2', album = '%3', genre = '%4'").arg(titleMetaData).arg(artistMetaData).arg(albumMetaData).arg(genreMetaData);
	if ( trackInfo != lastTrackInfo ) {
		log(QMC2_LOG_FRONTEND, trackInfo);
		lastTrackInfo = trackInfo;
	}
}

void MainWindow::audioBufferStatus(int percentFilled)
{
	progressBarAudioProgress->setRange(0, 100);
	progressBarAudioProgress->setFormat(tr("Buffering %p%"));
	progressBarAudioProgress->setValue(percentFilled);
	if ( percentFilled >= 100 ) {
#if QMC2_MULTIMEDIA_ENABLED
		if ( audioState == QMediaPlayer::StoppedState )
			progressBarAudioProgress->setRange(0, 100);
		else
			progressBarAudioProgress->setRange(0, 0);
#else
		if ( audioState == Phonon::StoppedState )
			progressBarAudioProgress->setRange(0, 100);
		else
			progressBarAudioProgress->setRange(0, 0);
#endif
		progressBarAudioProgress->reset();
	}
}
#else
void MainWindow::on_actionAudioPreviousTrack_triggered(bool) {}
void MainWindow::toolButtonAudioPreviousTrack_resetButton() {}
void MainWindow::on_actionAudioNextTrack_triggered(bool) {}
void MainWindow::toolButtonAudioNextTrack_resetButton() {}
void MainWindow::on_actionAudioFastBackward_triggered(bool) {}
void MainWindow::on_toolButtonAudioFastBackward_clicked(bool) {}
void MainWindow::toolButtonAudioFastBackward_resetButton() {}
void MainWindow::on_actionAudioFastForward_triggered(bool) {}
void MainWindow::on_toolButtonAudioFastForward_clicked(bool) {}
void MainWindow::toolButtonAudioFastForward_resetButton() {}
void MainWindow::on_actionAudioStopTrack_triggered(bool) {}
void MainWindow::on_actionAudioPauseTrack_triggered(bool) {}
void MainWindow::on_actionAudioPlayTrack_triggered(bool) {}
void MainWindow::on_toolButtonAudioAddTracks_clicked() {}
void MainWindow::on_toolButtonAudioAddURL_clicked() {}
void MainWindow::on_toolButtonAudioRemoveTracks_clicked() {}
void MainWindow::on_toolButtonAudioSetupEffects_clicked() {}
void MainWindow::on_listWidgetAudioPlaylist_itemSelectionChanged() {}
void MainWindow::on_dialAudioVolume_valueChanged(int) {}
void MainWindow::on_actionAudioRaiseVolume_triggered(bool) {}
void MainWindow::on_actionAudioLowerVolume_triggered(bool) {}
void MainWindow::audioFinished() {}
void MainWindow::audioTick(qint64) {}
void MainWindow::audioTotalTimeChanged(qint64) {}
void MainWindow::audioFade(int) {}
void MainWindow::audioMetaDataChanged() {}
void MainWindow::audioBufferStatus(int) {}
void MainWindow::audioScrollToCurrentItem() {}
#if QMC2_MULTIMEDIA_ENABLED
void MainWindow::audioStateChanged(QMediaPlayer::MediaStatus) {}
#endif
#endif

void MainWindow::on_checkBoxRemoveFinishedDownloads_stateChanged(int /*state*/)
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Downloads/RemoveFinished", checkBoxRemoveFinishedDownloads->isChecked());
}

void MainWindow::processOutputNotifier(int emuId, const QString &name, const QString &value)
{
	if ( qmc2CleaningUp )
		return;

	QList<QTreeWidgetItem *> il = treeWidgetEmulators->findItems(QString::number(emuId), Qt::MatchExactly, QMC2_EMUCONTROL_COLUMN_ID);
	if ( il.count() > 0 ) {
		if ( name == "pause" ) {
#if defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000
			Embedder *embedder = 0;
			int embedderIndex = -1;
			for (int j = 0; j < tabWidgetEmbeddedEmulators->count() && embedder == 0; j++) {
				if ( tabWidgetEmbeddedEmulators->tabText(j).startsWith(QString("#%1 - ").arg(emuId)) ) {
					embedder = (Embedder *)tabWidgetEmbeddedEmulators->widget(j);
					embedderIndex = j;
				}
			}
			if ( value == "0" ) {
				il.first()->setText(QMC2_EMUCONTROL_COLUMN_STATUS, tr("running"));
				if ( embedder ) {
					embedder->isPaused = false;
					tabWidgetEmbeddedEmulators->setTabIcon(embedderIndex, embedder->iconRunning);
				}
			} else {
				il.first()->setText(QMC2_EMUCONTROL_COLUMN_STATUS, tr("paused"));
				if ( embedder ) {
					embedder->isPaused = true;
					tabWidgetEmbeddedEmulators->setTabIcon(embedderIndex, embedder->iconPaused);
				}
			}
#else
			if ( value == "0" )
				il.first()->setText(QMC2_EMUCONTROL_COLUMN_STATUS, tr("running"));
			else
				il.first()->setText(QMC2_EMUCONTROL_COLUMN_STATUS, tr("paused"));
#endif
		} else if ( name == "led0" ) {
			if ( value == "0" )
				il.first()->setIcon(QMC2_EMUCONTROL_COLUMN_LED0, QIcon(QString::fromUtf8(":/data/img/led_off.png")));
			else
				il.first()->setIcon(QMC2_EMUCONTROL_COLUMN_LED0, QIcon(QString::fromUtf8(":/data/img/led_on.png")));
		} else if ( name == "led1" ) {
			if ( value == "0" )
				il.first()->setIcon(QMC2_EMUCONTROL_COLUMN_LED1, QIcon(QString::fromUtf8(":/data/img/led_off.png")));
			else
				il.first()->setIcon(QMC2_EMUCONTROL_COLUMN_LED1, QIcon(QString::fromUtf8(":/data/img/led_on.png")));
		} else if ( name == "led2" ) {
			if ( value == "0" )
				il.first()->setIcon(QMC2_EMUCONTROL_COLUMN_LED2, QIcon(QString::fromUtf8(":/data/img/led_off.png")));
			else
				il.first()->setIcon(QMC2_EMUCONTROL_COLUMN_LED2, QIcon(QString::fromUtf8(":/data/img/led_on.png")));
		} else {
			// add or refresh dynamic output notifiers
			QTreeWidgetItem *itemFound = 0;
			for (int i = 0; i < il.first()->childCount() && itemFound == 0; i++) {
				QTreeWidgetItem *item = il.first()->child(i);
				if ( item->text(QMC2_EMUCONTROL_COLUMN_MACHINE) == name )
					itemFound = item;
			}
			if ( itemFound != 0 )
				itemFound->setText(QMC2_EMUCONTROL_COLUMN_STATUS, value);
			else {
				itemFound = new QTreeWidgetItem(il[0]);
				itemFound->setText(QMC2_EMUCONTROL_COLUMN_MACHINE, name);
				itemFound->setText(QMC2_EMUCONTROL_COLUMN_STATUS, value);
				if ( il.first()->childCount() == 1 ) {
					// this is a workaround for a minor Qt bug: the root decoration
					// isn't updated correctly on the first child item insertion
					treeWidgetEmulators->setRootIsDecorated(false);
					treeWidgetEmulators->setRootIsDecorated(true);
				}
			}
		}
		treeWidgetEmulators->update();
	}
}

void MainWindow::treeWidgetMachineList_headerSectionClicked(int logicalIndex)
{
	qmc2MainWindow->treeWidgetMachineList->header()->setSortIndicatorShown(false);
	qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(false);
	qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicatorShown(false);
	qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicatorShown(false);

	switch ( logicalIndex ) {
		case QMC2_MACHINELIST_COLUMN_MACHINE:
			if ( qmc2Options->comboBoxSortCriteria->currentIndex() == QMC2_SORTCRITERIA_DESCRIPTION )
				qmc2Options->comboBoxSortOrder->setCurrentIndex(qmc2Options->comboBoxSortOrder->currentIndex() == 0 ? 1 : 0);
			else
				qmc2Options->comboBoxSortCriteria->setCurrentIndex(QMC2_SORTCRITERIA_DESCRIPTION);
			break;

		case QMC2_MACHINELIST_COLUMN_TAG:
			if ( qmc2Options->comboBoxSortCriteria->currentIndex() == QMC2_SORTCRITERIA_TAG )
				qmc2Options->comboBoxSortOrder->setCurrentIndex(qmc2Options->comboBoxSortOrder->currentIndex() == 0 ? 1 : 0);
			else
				qmc2Options->comboBoxSortCriteria->setCurrentIndex(QMC2_SORTCRITERIA_TAG);
			break;

		case QMC2_MACHINELIST_COLUMN_YEAR:
			if ( qmc2Options->comboBoxSortCriteria->currentIndex() == QMC2_SORTCRITERIA_YEAR )
				qmc2Options->comboBoxSortOrder->setCurrentIndex(qmc2Options->comboBoxSortOrder->currentIndex() == 0 ? 1 : 0);
			else
				qmc2Options->comboBoxSortCriteria->setCurrentIndex(QMC2_SORTCRITERIA_YEAR);
			break;

		case QMC2_MACHINELIST_COLUMN_MANU:
			if ( qmc2Options->comboBoxSortCriteria->currentIndex() == QMC2_SORTCRITERIA_MANUFACTURER )
				qmc2Options->comboBoxSortOrder->setCurrentIndex(qmc2Options->comboBoxSortOrder->currentIndex() == 0 ? 1 : 0);
			else
				qmc2Options->comboBoxSortCriteria->setCurrentIndex(QMC2_SORTCRITERIA_MANUFACTURER);
			break;

		case QMC2_MACHINELIST_COLUMN_NAME:
			if ( qmc2Options->comboBoxSortCriteria->currentIndex() == QMC2_SORTCRITERIA_MACHINENAME )
				qmc2Options->comboBoxSortOrder->setCurrentIndex(qmc2Options->comboBoxSortOrder->currentIndex() == 0 ? 1 : 0);
			else
				qmc2Options->comboBoxSortCriteria->setCurrentIndex(QMC2_SORTCRITERIA_MACHINENAME);
			break;

		case QMC2_MACHINELIST_COLUMN_RTYPES:
			if ( qmc2Options->comboBoxSortCriteria->currentIndex() == QMC2_SORTCRITERIA_ROMTYPES )
				qmc2Options->comboBoxSortOrder->setCurrentIndex(qmc2Options->comboBoxSortOrder->currentIndex() == 0 ? 1 : 0);
			else
				qmc2Options->comboBoxSortCriteria->setCurrentIndex(QMC2_SORTCRITERIA_ROMTYPES);
			break;

		case QMC2_MACHINELIST_COLUMN_PLAYERS:
			if ( qmc2Options->comboBoxSortCriteria->currentIndex() == QMC2_SORTCRITERIA_PLAYERS )
				qmc2Options->comboBoxSortOrder->setCurrentIndex(qmc2Options->comboBoxSortOrder->currentIndex() == 0 ? 1 : 0);
			else
				qmc2Options->comboBoxSortCriteria->setCurrentIndex(QMC2_SORTCRITERIA_PLAYERS);
			break;

		case QMC2_MACHINELIST_COLUMN_DRVSTAT:
			if ( qmc2Options->comboBoxSortCriteria->currentIndex() == QMC2_SORTCRITERIA_DRVSTAT )
				qmc2Options->comboBoxSortOrder->setCurrentIndex(qmc2Options->comboBoxSortOrder->currentIndex() == 0 ? 1 : 0);
			else
				qmc2Options->comboBoxSortCriteria->setCurrentIndex(QMC2_SORTCRITERIA_DRVSTAT);
			break;

		case QMC2_MACHINELIST_COLUMN_SRCFILE:
			if ( qmc2Options->comboBoxSortCriteria->currentIndex() == QMC2_SORTCRITERIA_SRCFILE )
				qmc2Options->comboBoxSortOrder->setCurrentIndex(qmc2Options->comboBoxSortOrder->currentIndex() == 0 ? 1 : 0);
			else
				qmc2Options->comboBoxSortCriteria->setCurrentIndex(QMC2_SORTCRITERIA_SRCFILE);
			break;

		case QMC2_MACHINELIST_COLUMN_RANK:
			if ( qmc2Options->comboBoxSortCriteria->currentIndex() == QMC2_SORTCRITERIA_RANK )
				qmc2Options->comboBoxSortOrder->setCurrentIndex(qmc2Options->comboBoxSortOrder->currentIndex() == 0 ? 1 : 0);
			else
				qmc2Options->comboBoxSortCriteria->setCurrentIndex(QMC2_SORTCRITERIA_RANK);
			break;

		case QMC2_MACHINELIST_COLUMN_CATEGORY:
			if ( qmc2Options->comboBoxSortCriteria->currentIndex() == QMC2_SORTCRITERIA_CATEGORY )
				qmc2Options->comboBoxSortOrder->setCurrentIndex(qmc2Options->comboBoxSortOrder->currentIndex() == 0 ? 1 : 0);
			else
				qmc2Options->comboBoxSortCriteria->setCurrentIndex(QMC2_SORTCRITERIA_CATEGORY);
			break;

		case QMC2_MACHINELIST_COLUMN_VERSION:
			if ( qmc2Options->comboBoxSortCriteria->currentIndex() == QMC2_SORTCRITERIA_VERSION )
				qmc2Options->comboBoxSortOrder->setCurrentIndex(qmc2Options->comboBoxSortOrder->currentIndex() == 0 ? 1 : 0);
			else
				qmc2Options->comboBoxSortCriteria->setCurrentIndex(QMC2_SORTCRITERIA_VERSION);
			break;

		default:
			break;
	}

	QTimer::singleShot(0, qmc2Options, SLOT(on_pushButtonApply_clicked()));
}

void MainWindow::menuTabWidgetMachineList_North_activated()
{
	tabWidgetMachineList->setTabPosition(QTabWidget::North);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MachineList/TabPosition", QTabWidget::North);
}

void MainWindow::menuTabWidgetMachineList_South_activated()
{
	tabWidgetMachineList->setTabPosition(QTabWidget::South);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MachineList/TabPosition", QTabWidget::South);
}

void MainWindow::menuTabWidgetMachineList_West_activated()
{
	tabWidgetMachineList->setTabPosition(QTabWidget::West);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MachineList/TabPosition", QTabWidget::West);
}

void MainWindow::menuTabWidgetMachineList_East_activated()
{
	tabWidgetMachineList->setTabPosition(QTabWidget::East);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MachineList/TabPosition", QTabWidget::East);
}

void MainWindow::menuTabWidgetMachineList_Setup_activated()
{
	if ( !qmc2ComponentSetup )
		return;
 
	qmc2ComponentSetup->adjustIconSizes();

	qmc2ComponentSetup->setParent(this);
	qmc2ComponentSetup->setWindowFlags(Qt::Dialog);
	qmc2ComponentSetup->comboBoxComponents->setCurrentIndex(0);

	if ( qmc2ComponentSetup->isHidden() )
		qmc2ComponentSetup->show();
	else if ( qmc2ComponentSetup->isMinimized() )
		qmc2ComponentSetup->showNormal();

	QTimer::singleShot(0, qmc2ComponentSetup, SLOT(raise()));
}


void MainWindow::on_tabWidgetMachineList_customContextMenuRequested(const QPoint &p)
{
	QTabBar *tabBar = tabWidgetMachineList->findChild<QTabBar *>();
	if ( tabBar ) {
		if ( tabBar->geometry().contains(p) ) {
			while ( menuTabWidgetMachineList->actions().count() > 3 ) {
				QAction *a = menuTabWidgetMachineList->actions().at(2);
				menuTabWidgetMachineList->removeAction(a);
				a->disconnect();
				delete a;
			}
			QAction *insAct = menuTabWidgetMachineList->actions().last();
			for (int i = 0; i < tabBar->count(); i++) {
				QAction *a = new QAction(tabBar->tabIcon(i), tabBar->tabText(i).replace("&", ""), menuTabWidgetMachineList);
				a->setData(i);
				connect(a, SIGNAL(triggered(bool)), this, SLOT(tabWidgetMachineList_actionSwitchTab_triggered(bool)));
				menuTabWidgetMachineList->insertAction(insAct, a);
			}
			QAction *a = new QAction(menuTabWidgetMachineList);
			a->setSeparator(true);
			menuTabWidgetMachineList->insertAction(menuTabWidgetMachineList->actions().last(), a);
			menuTabWidgetMachineList->move(adjustedWidgetPosition(tabWidgetMachineList->mapToGlobal(p), menuTabWidgetMachineList));
			menuTabWidgetMachineList->show();
		}
	}
}

void MainWindow::tabWidgetMachineList_actionSwitchTab_triggered(bool)
{
	QAction *a = (QAction *)sender();
	tabWidgetMachineList->setCurrentIndex(a->data().toInt());
}

void MainWindow::menuTabWidgetMachineDetail_North_activated()
{
	tabWidgetMachineDetail->setTabPosition(QTabWidget::North);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MachineDetail/TabPosition", QTabWidget::North);
}

void MainWindow::menuTabWidgetMachineDetail_South_activated()
{
	tabWidgetMachineDetail->setTabPosition(QTabWidget::South);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MachineDetail/TabPosition", QTabWidget::South);
}

void MainWindow::menuTabWidgetMachineDetail_West_activated()
{
	tabWidgetMachineDetail->setTabPosition(QTabWidget::West);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MachineDetail/TabPosition", QTabWidget::West);
}

void MainWindow::menuTabWidgetMachineDetail_East_activated()
{
	tabWidgetMachineDetail->setTabPosition(QTabWidget::East);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MachineDetail/TabPosition", QTabWidget::East);
}

void MainWindow::menuTabWidgetMachineDetail_Setup_activated()
{
	if ( !qmc2ComponentSetup )
		return;
 
	qmc2ComponentSetup->adjustIconSizes();

	qmc2ComponentSetup->setParent(this);
	qmc2ComponentSetup->setWindowFlags(Qt::Dialog);
	qmc2ComponentSetup->comboBoxComponents->setCurrentIndex(1);

	if ( qmc2ComponentSetup->isHidden() )
		qmc2ComponentSetup->show();
	else if ( qmc2ComponentSetup->isMinimized() )
		qmc2ComponentSetup->showNormal();

	QTimer::singleShot(0, qmc2ComponentSetup, SLOT(raise()));
}

void MainWindow::on_tabWidgetMachineDetail_customContextMenuRequested(const QPoint &p)
{
	QTabBar *tabBar = tabWidgetMachineDetail->findChild<QTabBar *>();
	if ( tabBar ) {
		if ( tabBar->geometry().contains(p) ) {
			while ( menuTabWidgetMachineDetail->actions().count() > 3 ) {
				QAction *a = menuTabWidgetMachineDetail->actions().at(2);
				menuTabWidgetMachineDetail->removeAction(a);
				a->disconnect();
				delete a;
			}
			QAction *insAct = menuTabWidgetMachineDetail->actions().last();
			for (int i = 0; i < tabBar->count(); i++) {
				QAction *a = new QAction(tabBar->tabIcon(i), tabBar->tabText(i).replace("&", ""), menuTabWidgetMachineDetail);
				a->setData(i);
				connect(a, SIGNAL(triggered(bool)), this, SLOT(tabWidgetMachineDetail_actionSwitchTab_triggered(bool)));
				menuTabWidgetMachineDetail->insertAction(insAct, a);
			}
			QAction *a = new QAction(menuTabWidgetMachineDetail);
			a->setSeparator(true);
			menuTabWidgetMachineDetail->insertAction(menuTabWidgetMachineDetail->actions().last(), a);
			menuTabWidgetMachineDetail->move(adjustedWidgetPosition(tabWidgetMachineDetail->mapToGlobal(p), menuTabWidgetMachineDetail));
			menuTabWidgetMachineDetail->show();
		}
	}
}

void MainWindow::tabWidgetMachineDetail_actionSwitchTab_triggered(bool)
{
	QAction *a = (QAction *)sender();
	tabWidgetMachineDetail->setCurrentIndex(a->data().toInt());
}

void MainWindow::menuTabWidgetLogsAndEmulators_North_activated()
{
	tabWidgetLogsAndEmulators->setTabPosition(QTabWidget::North);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/LogsAndEmulators/TabPosition", QTabWidget::North);
}

void MainWindow::menuTabWidgetLogsAndEmulators_South_activated()
{
	tabWidgetLogsAndEmulators->setTabPosition(QTabWidget::South);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/LogsAndEmulators/TabPosition", QTabWidget::South);
}

void MainWindow::menuTabWidgetLogsAndEmulators_West_activated()
{
	tabWidgetLogsAndEmulators->setTabPosition(QTabWidget::West);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/LogsAndEmulators/TabPosition", QTabWidget::West);
}

void MainWindow::menuTabWidgetLogsAndEmulators_East_activated()
{
	tabWidgetLogsAndEmulators->setTabPosition(QTabWidget::East);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/LogsAndEmulators/TabPosition", QTabWidget::East);
}

void MainWindow::menuTabWidgetLogsAndEmulators_Setup_activated()
{
	if ( !qmc2ComponentSetup )
		return;
 
	qmc2ComponentSetup->adjustIconSizes();

	// reparent detail setup dialog to the widget it was called from
	qmc2ComponentSetup->setParent(this);
	qmc2ComponentSetup->setWindowFlags(Qt::Dialog);
	qmc2ComponentSetup->comboBoxComponents->setCurrentIndex(2);

	if ( qmc2ComponentSetup->isHidden() )
		qmc2ComponentSetup->show();
	else if ( qmc2ComponentSetup->isMinimized() )
		qmc2ComponentSetup->showNormal();

	QTimer::singleShot(0, qmc2ComponentSetup, SLOT(raise()));
}

void MainWindow::on_tabWidgetLogsAndEmulators_customContextMenuRequested(const QPoint &p)
{
	QTabBar *tabBar = tabWidgetLogsAndEmulators->findChild<QTabBar *>();
	if ( tabBar ) {
		if ( tabBar->geometry().contains(p) ) {
			while ( menuTabWidgetLogsAndEmulators->actions().count() > 3 ) {
				QAction *a = menuTabWidgetLogsAndEmulators->actions().at(2);
				menuTabWidgetLogsAndEmulators->removeAction(a);
				a->disconnect();
				delete a;
			}
			QAction *insAct = menuTabWidgetLogsAndEmulators->actions().last();
			for (int i = 0; i < tabBar->count(); i++) {
				QAction *a = new QAction(tabBar->tabIcon(i), tabBar->tabText(i).replace("&", ""), menuTabWidgetLogsAndEmulators);
				a->setData(i);
				connect(a, SIGNAL(triggered(bool)), this, SLOT(tabWidgetLogsAndEmulators_actionSwitchTab_triggered(bool)));
				menuTabWidgetLogsAndEmulators->insertAction(insAct, a);
			}
			QAction *a = new QAction(menuTabWidgetLogsAndEmulators);
			a->setSeparator(true);
			menuTabWidgetLogsAndEmulators->insertAction(menuTabWidgetLogsAndEmulators->actions().last(), a);
			menuTabWidgetLogsAndEmulators->move(adjustedWidgetPosition(tabWidgetLogsAndEmulators->mapToGlobal(p), menuTabWidgetLogsAndEmulators));
			menuTabWidgetLogsAndEmulators->show();
		}
	}
}

void MainWindow::tabWidgetLogsAndEmulators_actionSwitchTab_triggered(bool)
{
	QAction *a = (QAction *)sender();
	tabWidgetLogsAndEmulators->setCurrentIndex(a->data().toInt());
}

void MainWindow::menuTabWidgetSoftwareDetail_North_activated()
{
	tabWidgetSoftwareDetail->setTabPosition(QTabWidget::North);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareDetail/TabPosition", QTabWidget::North);
}

void MainWindow::menuTabWidgetSoftwareDetail_South_activated()
{
	tabWidgetSoftwareDetail->setTabPosition(QTabWidget::South);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareDetail/TabPosition", QTabWidget::South);
}

void MainWindow::menuTabWidgetSoftwareDetail_West_activated()
{
	tabWidgetSoftwareDetail->setTabPosition(QTabWidget::West);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareDetail/TabPosition", QTabWidget::West);
}

void MainWindow::menuTabWidgetSoftwareDetail_East_activated()
{
	tabWidgetSoftwareDetail->setTabPosition(QTabWidget::East);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareDetail/TabPosition", QTabWidget::East);
}

void MainWindow::menuTabWidgetSoftwareDetail_Setup_activated()
{
	if ( !qmc2ComponentSetup )
		return;
 
	qmc2ComponentSetup->adjustIconSizes();

	qmc2ComponentSetup->setParent(this);
	qmc2ComponentSetup->setWindowFlags(Qt::Dialog);
	qmc2ComponentSetup->comboBoxComponents->setCurrentIndex(3);

	if ( qmc2ComponentSetup->isHidden() )
		qmc2ComponentSetup->show();
	else if ( qmc2ComponentSetup->isMinimized() )
		qmc2ComponentSetup->showNormal();

	QTimer::singleShot(0, qmc2ComponentSetup, SLOT(raise()));
}

void MainWindow::on_tabWidgetSoftwareDetail_customContextMenuRequested(const QPoint &p)
{
	QTabBar *tabBar = tabWidgetSoftwareDetail->findChild<QTabBar *>();
	if ( tabBar ) {
		if ( tabBar->geometry().contains(p) ) {
			while ( menuTabWidgetSoftwareDetail->actions().count() > 3 ) {
				QAction *a = menuTabWidgetSoftwareDetail->actions().at(2);
				menuTabWidgetSoftwareDetail->removeAction(a);
				a->disconnect();
				delete a;
			}
			QAction *insAct = menuTabWidgetSoftwareDetail->actions().last();
			for (int i = 0; i < tabBar->count(); i++) {
				QAction *a = new QAction(tabBar->tabIcon(i), tabBar->tabText(i).replace("&", ""), menuTabWidgetSoftwareDetail);
				a->setData(i);
				connect(a, SIGNAL(triggered(bool)), this, SLOT(tabWidgetSoftwareDetail_actionSwitchTab_triggered(bool)));
				menuTabWidgetSoftwareDetail->insertAction(insAct, a);
			}
			QAction *a = new QAction(menuTabWidgetSoftwareDetail);
			a->setSeparator(true);
			menuTabWidgetSoftwareDetail->insertAction(menuTabWidgetSoftwareDetail->actions().last(), a);
			menuTabWidgetSoftwareDetail->move(adjustedWidgetPosition(tabWidgetSoftwareDetail->mapToGlobal(p), menuTabWidgetSoftwareDetail));
			menuTabWidgetSoftwareDetail->show();
		}
	}
}

void MainWindow::tabWidgetSoftwareDetail_actionSwitchTab_triggered(bool)
{
	QAction *a = (QAction *)sender();
	tabWidgetSoftwareDetail->setCurrentIndex(a->data().toInt());
}

void MainWindow::treeWidgetHierarchy_headerSectionClicked(int logicalIndex)
{
	treeWidgetMachineList_headerSectionClicked(logicalIndex);
}

void MainWindow::treeWidgetCategoryView_headerSectionClicked(int logicalIndex)
{
	treeWidgetMachineList_headerSectionClicked(logicalIndex);
}

void MainWindow::on_treeWidgetCategoryView_itemActivated(QTreeWidgetItem *item, int column)
{
	if ( qmc2DemoModeDialog )
		if ( qmc2DemoModeDialog->demoModeRunning )
			return;
	if ( !item )
		return;
	if ( item->text(QMC2_MACHINELIST_COLUMN_NAME).isEmpty() )
		return;
	qmc2StartEmbedded = false;
	if ( !qmc2IgnoreItemActivation ) {
		switch ( qmc2DefaultLaunchMode ) {
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
			case QMC2_LAUNCH_MODE_EMBEDDED:
				on_actionPlayEmbedded_triggered();
				break;
#endif
			case QMC2_LAUNCH_MODE_INDEPENDENT:
			default:
				on_actionPlay_triggered();
				break;
		}
	}
	qmc2IgnoreItemActivation = false;
}

void MainWindow::on_treeWidgetCategoryView_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
	if ( !item )
		return;
	if ( item->text(QMC2_MACHINELIST_COLUMN_NAME).isEmpty() )
		return;
	if ( qmc2DemoModeDialog )
		if ( qmc2DemoModeDialog->demoModeRunning )
			qmc2IgnoreItemActivation = true;
	if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "MachineList/DoubleClickActivation").toBool() )
		qmc2IgnoreItemActivation = true;
}

void MainWindow::on_treeWidgetCategoryView_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
	if ( !current )
		return;
	if ( current->text(QMC2_MACHINELIST_COLUMN_NAME).isEmpty() )
		return;
	qmc2CheckItemVisibility = false;
	if ( qmc2UpdateDelay > 0 )
		updateTimer.start(qmc2UpdateDelay);
	else
		treeWidgetMachineList_itemSelectionChanged_delayed();
}

void MainWindow::on_treeWidgetCategoryView_itemSelectionChanged()
{
	qmc2CategoryViewSelectedItem = 0;
	QList<QTreeWidgetItem *>selected = treeWidgetCategoryView->selectedItems();
	if ( selected.count() > 0 ) {
		QTreeWidgetItem *item = selected.at(0);
		if ( item->text(QMC2_MACHINELIST_COLUMN_MACHINE) == MachineList::trWaitingForData || item->text(QMC2_MACHINELIST_COLUMN_NAME).isEmpty() )
			return;
		qmc2CategoryViewSelectedItem = qmc2MachineListItemHash.value(item->text(QMC2_MACHINELIST_COLUMN_NAME));
		qmc2CheckItemVisibility = false;
		treeWidgetMachineList->setCurrentItem(qmc2CategoryViewSelectedItem);
	}
}

void MainWindow::on_treeWidgetCategoryView_customContextMenuRequested(const QPoint &p)
{
	QTreeWidgetItem *item = treeWidgetCategoryView->itemAt(p);
	if ( !item )
		return;
	if ( item->text(QMC2_MACHINELIST_COLUMN_MACHINE) == MachineList::trWaitingForData || item->text(QMC2_MACHINELIST_COLUMN_NAME).isEmpty() )
		return;
	treeWidgetCategoryView->setItemSelected(item, true);
	qmc2MachineMenu->move(adjustedWidgetPosition(treeWidgetCategoryView->viewport()->mapToGlobal(p), qmc2MachineMenu));
	qmc2MachineMenu->show();
}

void MainWindow::treeWidgetVersionView_headerSectionClicked(int logicalIndex)
{
	treeWidgetMachineList_headerSectionClicked(logicalIndex);
}

void MainWindow::on_treeWidgetVersionView_itemActivated(QTreeWidgetItem *item, int column)
{
	if ( qmc2DemoModeDialog )
		if ( qmc2DemoModeDialog->demoModeRunning )
			return;
	if ( !item )
		return;
	if ( item->text(QMC2_MACHINELIST_COLUMN_NAME).isEmpty() )
		return;
	qmc2StartEmbedded = false;
	if ( !qmc2IgnoreItemActivation ) {
		switch ( qmc2DefaultLaunchMode ) {
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
			case QMC2_LAUNCH_MODE_EMBEDDED:
				on_actionPlayEmbedded_triggered();
				break;
#endif
			case QMC2_LAUNCH_MODE_INDEPENDENT:
			default:
				on_actionPlay_triggered();
				break;
		}
	}
	qmc2IgnoreItemActivation = false;
}

void MainWindow::on_treeWidgetVersionView_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
	if ( !item )
		return;
	if ( item->text(QMC2_MACHINELIST_COLUMN_NAME).isEmpty() )
		return;
	if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "MachineList/DoubleClickActivation").toBool() )
		qmc2IgnoreItemActivation = true;
}

void MainWindow::on_treeWidgetVersionView_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
	if ( !current )
		return;
	if ( current->text(QMC2_MACHINELIST_COLUMN_NAME).isEmpty() )
		return;
	qmc2CheckItemVisibility = false;
	if ( qmc2UpdateDelay > 0 )
		updateTimer.start(qmc2UpdateDelay);
	else
		treeWidgetMachineList_itemSelectionChanged_delayed();
}

void MainWindow::on_treeWidgetVersionView_itemSelectionChanged()
{
	qmc2VersionViewSelectedItem = 0;
	QList<QTreeWidgetItem *>selected = treeWidgetVersionView->selectedItems();
	if ( selected.count() > 0 ) {
		QTreeWidgetItem *item = selected.at(0);
		if ( item->text(QMC2_MACHINELIST_COLUMN_MACHINE) == MachineList::trWaitingForData || item->text(QMC2_MACHINELIST_COLUMN_NAME).isEmpty() )
			return;
		qmc2VersionViewSelectedItem = qmc2MachineListItemHash.value(item->text(QMC2_MACHINELIST_COLUMN_NAME));
		qmc2CheckItemVisibility = false;
		treeWidgetMachineList->setCurrentItem(qmc2VersionViewSelectedItem);
	}
}

void MainWindow::on_treeWidgetVersionView_customContextMenuRequested(const QPoint &p)
{
	QTreeWidgetItem *item = treeWidgetVersionView->itemAt(p);
	if ( !item )
		return;
	if ( item->text(QMC2_MACHINELIST_COLUMN_MACHINE) == MachineList::trWaitingForData || item->text(QMC2_MACHINELIST_COLUMN_NAME).isEmpty() )
		return;
	treeWidgetVersionView->setItemSelected(item, true);
	qmc2MachineMenu->move(adjustedWidgetPosition(treeWidgetVersionView->viewport()->mapToGlobal(p), qmc2MachineMenu));
	qmc2MachineMenu->show();
}

void MainWindow::on_comboBoxViewSelect_currentIndexChanged(int index)
{
	if ( tabWidgetMachineList->indexOf(tabMachineList) != tabWidgetMachineList->currentIndex() )
		return;

	switch ( index ) {
		case QMC2_VIEWHIERARCHY_INDEX:
			toolButtonSelectRomFilter->setVisible(false);
			menuRomStatusFilter->setVisible(false);
			actionTagVisible->setVisible(false);
			actionUntagVisible->setVisible(false);
			actionInvertVisibleTags->setVisible(false);
			viewParentClones();
			break;
		case QMC2_VIEWCATEGORY_INDEX:
			toolButtonSelectRomFilter->setVisible(false);
			menuRomStatusFilter->setVisible(false);
			actionTagVisible->setVisible(false);
			actionUntagVisible->setVisible(false);
			actionInvertVisibleTags->setVisible(false);
			viewByCategory();
			break;
		case QMC2_VIEWVERSION_INDEX:
			toolButtonSelectRomFilter->setVisible(false);
			menuRomStatusFilter->setVisible(false);
			actionTagVisible->setVisible(false);
			actionUntagVisible->setVisible(false);
			actionInvertVisibleTags->setVisible(false);
			viewByVersion();
			break;
		case QMC2_VIEWMACHINELIST_INDEX:
		default: {
			bool romFilterActive = qmc2Config->value(QMC2_FRONTEND_PREFIX + "RomStateFilter/Enabled", true).toBool();
			toolButtonSelectRomFilter->setVisible(romFilterActive);
			actionTagVisible->setVisible(romFilterActive);
			actionUntagVisible->setVisible(romFilterActive);
			actionInvertVisibleTags->setVisible(romFilterActive);
			viewFullDetail();
			break;
		}
	}
}

void MainWindow::projectMessLoadStarted()
{
}

void MainWindow::projectMessLoadFinished(bool ok)
{
	if ( qmc2SoftwareList->currentItem && qmc2ProjectMESS && ok ) {
		// store compressed page to in-memory cache
		QString cacheKey = qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_LIST) + "_" + qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_NAME);
		if ( qmc2ProjectMESSCache.contains(cacheKey) )
			qmc2ProjectMESSCache.remove(cacheKey);
		QByteArray data = QMC2_COMPRESS(qmc2ProjectMESS->webViewBrowser->page()->mainFrame()->toHtml().toUtf8());
		qmc2ProjectMESSCache.insert(cacheKey, new QByteArray(data), data.size());
	}

	// we only want to know this ONCE
	disconnect(qmc2ProjectMESS->webViewBrowser, SIGNAL(loadFinished(bool)), this, SLOT(projectMessLoadFinished(bool)));
}

void MainWindow::projectMessSystemLoadStarted()
{
	// NOP
}

void MainWindow::projectMessSystemLoadFinished(bool ok)
{
	if ( ok ) {
		QByteArray projectMessData = QMC2_COMPRESS(qmc2ProjectMESSLookup->webViewBrowser->page()->mainFrame()->toHtml().toUtf8());
    		QString machName = qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_NAME);
		if ( qmc2ProjectMESSCache.contains(machName) )
			qmc2ProjectMESSCache.remove(machName);
		qmc2ProjectMESSCache.insert(machName, new QByteArray(projectMessData), projectMessData.size());
	}
}

void MainWindow::startDownload(QWidget *forParent, QNetworkReply *reply, QString saveAsName, QString savePath)
{
	if ( !reply )
		return;

	QFileInfo fi(reply->url().path());
	QString proposedName = fi.baseName();
	if ( !saveAsName.isEmpty() )
		proposedName = saveAsName;
	else if ( !fi.completeSuffix().isEmpty() )
		proposedName += "." + fi.completeSuffix();

	QString filePath;

	if ( !saveAsName.isEmpty() && !savePath.isEmpty() ) {
		if ( !savePath.endsWith("/") )
			savePath.append("/");
		filePath = savePath + saveAsName;
	} else {
		if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "WebBrowser/LastStoragePath") )
			proposedName.prepend(qmc2Config->value(QMC2_FRONTEND_PREFIX + "WebBrowser/LastStoragePath").toString());
		filePath = QFileDialog::getSaveFileName(forParent, tr("Choose file to store download"), proposedName, tr("All files (*)"), 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	}

	if ( !filePath.isEmpty() ) {
		DownloadItem *downloadItem = new DownloadItem(reply, filePath, treeWidgetDownloads);
		treeWidgetDownloads->scrollToItem(downloadItem);
		QFileInfo fiFilePath(filePath);
		QString storagePath = fiFilePath.absolutePath();
		if ( !storagePath.endsWith("/") )
			storagePath.append("/");
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "WebBrowser/LastStoragePath", storagePath);
	} else
		reply->close();
}

void MainWindow::on_pushButtonClearFinishedDownloads_clicked()
{
	static bool downloadCleanupActive = false;

	if ( downloadCleanupActive )
		return;

	downloadCleanupActive = true;

	QList<int> indexList;

	QTreeWidgetItemIterator it(treeWidgetDownloads);
	while ( *it ) {
		if ( (*it)->whatsThis(0) == "finished" || (*it)->whatsThis(0) == "aborted" )
			indexList << treeWidgetDownloads->indexOfTopLevelItem(*it);
		it++;
	}

	for (int i = indexList.count(); i >= 0; i--) {
		DownloadItem *item = (DownloadItem *)treeWidgetDownloads->takeTopLevelItem(indexList[i]);
		if ( item )
			delete item;
		if ( i % 10 == 0 ) qApp->processEvents();
	}

	downloadCleanupActive = false;
}

void MainWindow::on_pushButtonReloadSelectedDownloads_clicked()
{
	QTreeWidgetItemIterator it(treeWidgetDownloads);
	while ( *it ) {
		if ( (*it)->isSelected() )
			if ( (*it)->whatsThis(0) != "downloading" && (*it)->whatsThis(0) != "initializing" )
				((DownloadItem*)(*it))->itemDownloader->reload();
		it++;
	}
}

void MainWindow::on_pushButtonStopSelectedDownloads_clicked()
{
	QTreeWidgetItemIterator it(treeWidgetDownloads);
	while ( *it ) {
		if ( (*it)->isSelected() ) {
			if ( (*it)->whatsThis(0) == "downloading" )
				((DownloadItem*)(*it))->itemDownloader->networkReply->abort();
		}
		it++;
	}
}

void MainWindow::checkActivity()
{
	// resync timer (as far as possible)
	activityCheckTimer.start(QMC2_ACTIVITY_CHECK_INTERVAL);

	if ( qmc2ReloadActive || qmc2VerifyActive || qmc2FilterActive || qmc2ImageCheckActive || qmc2SampleCheckActive || (qmc2SystemROMAlyzer && qmc2SystemROMAlyzer->active()) || (qmc2SoftwareROMAlyzer && qmc2SoftwareROMAlyzer->active()) || qmc2LoadingMachineInfoDB || qmc2LoadingSoftwareInfoDB || qmc2LoadingEmuInfoDB || (qmc2SoftwareList && qmc2SoftwareList->isLoading) ) {
		activityState = !activityState;
		if ( activityState )
			actionExitStop->setIcon(QIcon(QString::fromUtf8(":/data/img/activity_green.png")));
		else
			actionExitStop->setIcon(QIcon(QString::fromUtf8(":/data/img/activity_red.png")));
		toolbar->update();
		isActiveState = true;
	} else {
		if ( isActiveState ) {
			actionExitStop->setIcon(QIcon(QString::fromUtf8(":/data/img/exit.png")));
			toolbar->update();
			activityState = false;
		}
		isActiveState = false;
	}

	if ( qmc2SystemNotesEditor ) {
		qmc2SystemNotesEditor->move(0, 0);
		qmc2SystemNotesEditor->resize(qmc2SystemNotesEditor->parentWidget()->size());
	}

	if ( menuRank->isVisible() || menuRank->isTearOffMenuVisible() )
		QTimer::singleShot(0, this, SLOT(menuRank_aboutToShow()));
}

int MainWindow::sortCriteriaLogicalIndex()
{
	switch ( qmc2SortCriteria ) {
		case QMC2_SORT_BY_TAG:
			return QMC2_MACHINELIST_COLUMN_TAG;
		case QMC2_SORT_BY_YEAR:
			return QMC2_MACHINELIST_COLUMN_YEAR;
		case QMC2_SORT_BY_MANUFACTURER:
			return QMC2_MACHINELIST_COLUMN_MANU;
		case QMC2_SORT_BY_NAME:
			return QMC2_MACHINELIST_COLUMN_NAME;
		case QMC2_SORT_BY_ROMTYPES:
			return QMC2_MACHINELIST_COLUMN_RTYPES;
		case QMC2_SORT_BY_PLAYERS:
			return QMC2_MACHINELIST_COLUMN_PLAYERS;
		case QMC2_SORT_BY_DRVSTAT:
			return QMC2_MACHINELIST_COLUMN_DRVSTAT;
		case QMC2_SORT_BY_SRCFILE:
			return QMC2_MACHINELIST_COLUMN_SRCFILE;
		case QMC2_SORT_BY_RANK:
			return QMC2_MACHINELIST_COLUMN_RANK;
		case QMC2_SORT_BY_CATEGORY:
			return QMC2_MACHINELIST_COLUMN_CATEGORY;
		case QMC2_SORT_BY_VERSION:
			return QMC2_MACHINELIST_COLUMN_VERSION;
		case QMC2_SORT_BY_DESCRIPTION:
		case QMC2_SORT_BY_ROM_STATE:
		default:
			return QMC2_MACHINELIST_COLUMN_MACHINE;
	}
}

QPoint MainWindow::adjustedWidgetPosition(QPoint p, QWidget *w)
{
	static QPoint adjustedPosition;
	static QList<QWidget *> adjustedWidgets;

	if ( !adjustedWidgets.contains(w) ) {
		w->move(p);
		w->show();
		adjustedWidgets.append(w);
	}

	adjustedPosition = p;
	if ( p.x() + w->width() > desktopGeometry.width() )
		adjustedPosition.setX(desktopGeometry.width() - w->width());
	if ( p.y() + w->height() > desktopGeometry.height() )
		adjustedPosition.setY(desktopGeometry.height() - w->height());

	return adjustedPosition;
}

void MainWindow::enableContextMenuPlayActions(bool enable)
{
	foreach(QAction *action, contextMenuPlayActions)
		action->setEnabled(enable);
}

void MainWindow::treeWidgetMachineListHeader_customContextMenuRequested(const QPoint &p)
{
	menuMachineListHeader->move(adjustedWidgetPosition(treeWidgetMachineList->header()->viewport()->mapToGlobal(p), menuMachineListHeader));
	menuMachineListHeader->show();
}

void MainWindow::actionMachineListHeader_triggered()
{
	QAction *action = (QAction *)sender();
	int visibleColumns = 0;
	for (int i = 0; i < treeWidgetMachineList->columnCount(); i++)
		if ( !treeWidgetMachineList->isColumnHidden(i) )
			visibleColumns++;
	if ( action ) {
		if ( action->data().toInt() == QMC2_MACHINELIST_RESET ) {
			for (int i = 0; i < treeWidgetMachineList->columnCount(); i++) {
				if ( i == QMC2_MACHINELIST_COLUMN_CATEGORY ) {
				       	if ( qmc2CategoryInfoUsed )
						treeWidgetMachineList->setColumnHidden(i, false);
				} else if ( i == QMC2_MACHINELIST_COLUMN_VERSION ) {
				       	if ( qmc2VersionInfoUsed )
						treeWidgetMachineList->setColumnHidden(i, false);
				} else
					treeWidgetMachineList->setColumnHidden(i, false);
			}
			treeWidgetMachineList->header()->resizeSections(QHeaderView::Stretch);
			foreach (QAction *a, menuMachineListHeader->actions())
				if ( a->isCheckable() ) {
					a->blockSignals(true);
					a->setChecked(true);
					a->blockSignals(false);
				}
			return;
		}
		bool visibility = true;
		if ( action->isChecked() )
			treeWidgetMachineList->setColumnHidden(action->data().toInt(), false);
		else if ( visibleColumns > 1 ) {
			treeWidgetMachineList->setColumnHidden(action->data().toInt(), true);
			visibility = false;
		}
		action->blockSignals(true);
		action->setChecked(visibility);
		action->blockSignals(false);
		QTimer::singleShot(0, this, SLOT(updateUserData()));
	}
}

void MainWindow::treeWidgetHierarchyHeader_customContextMenuRequested(const QPoint &p)
{
	menuHierarchyHeader->move(adjustedWidgetPosition(treeWidgetHierarchy->header()->viewport()->mapToGlobal(p), menuHierarchyHeader));
	menuHierarchyHeader->show();
}

void MainWindow::actionHierarchyHeader_triggered()
{
	QAction *action = (QAction *)sender();
	int visibleColumns = 0;
	for (int i = 0; i < treeWidgetHierarchy->columnCount(); i++)
		if ( !treeWidgetHierarchy->isColumnHidden(i) )
			visibleColumns++;
	if ( action ) {
		if ( action->data().toInt() == QMC2_MACHINELIST_RESET ) {
			for (int i = 0; i < treeWidgetHierarchy->columnCount(); i++) {
				if ( i == QMC2_MACHINELIST_COLUMN_CATEGORY ) {
				       	if ( qmc2CategoryInfoUsed )
						treeWidgetHierarchy->setColumnHidden(i, false);
				} else if ( i == QMC2_MACHINELIST_COLUMN_VERSION ) {
				       	if ( qmc2VersionInfoUsed )
						treeWidgetHierarchy->setColumnHidden(i, false);
				} else
					treeWidgetHierarchy->setColumnHidden(i, false);
			}
			treeWidgetHierarchy->header()->resizeSections(QHeaderView::Stretch);
			foreach (QAction *a, menuHierarchyHeader->actions())
				if ( a->isCheckable() ) {
					a->blockSignals(true);
					a->setChecked(true);
					a->blockSignals(false);
				}
			return;
		}
		bool visibility = true;
		if ( action->isChecked() )
			treeWidgetHierarchy->setColumnHidden(action->data().toInt(), false);
		else if ( visibleColumns > 1 ) {
			treeWidgetHierarchy->setColumnHidden(action->data().toInt(), true);
			visibility = false;
		}
		action->blockSignals(true);
		action->setChecked(visibility);
		action->blockSignals(false);
		QTimer::singleShot(0, this, SLOT(updateUserData()));
	}
}

void MainWindow::on_treeWidgetMachineList_itemEntered(QTreeWidgetItem *item, int column)
{
	if ( column == QMC2_MACHINELIST_COLUMN_TAG && item->parent() == 0 ) {
		if ( qApp->mouseButtons() == Qt::LeftButton && qApp->activeWindow() ) {
			QPoint cPos = treeWidgetMachineList->viewport()->mapFromGlobal(QCursor::pos());
			if ( treeWidgetMachineList->itemAt(cPos) == item ) {
				Qt::CheckState cs = (item->checkState(column) == Qt::Unchecked ? Qt::Checked : Qt::Unchecked);
				bool wasTagged = (cs != Qt::Checked);
				item->setCheckState(column, cs);
				QString machineName(item->text(QMC2_MACHINELIST_COLUMN_NAME));
				item = qmc2HierarchyItemHash.value(machineName);
				if ( item )
					item->setCheckState(column, cs);
				item = qmc2CategoryItemHash.value(machineName);
				if ( item )
					item->setCheckState(column, cs);
				item = qmc2VersionItemHash.value(machineName);
				if ( item )
					item->setCheckState(column, cs);
				if ( wasTagged )
					qmc2MachineList->numTaggedSets--;
				else
					qmc2MachineList->numTaggedSets++;
				foreach (MachineListViewer *v, machineListViewers)
					v->mainTagChanged(machineName, !wasTagged);
				labelMachineListStatus->setText(qmc2MachineList->status());
			}
		}
	}
}

void MainWindow::on_treeWidgetMachineList_itemPressed(QTreeWidgetItem *item, int column)
{
	on_treeWidgetMachineList_itemEntered(item, column);
}

void MainWindow::on_treeWidgetHierarchy_itemEntered(QTreeWidgetItem *item, int column)
{
	if ( column == QMC2_MACHINELIST_COLUMN_TAG ) {
		if ( qApp->mouseButtons() == Qt::LeftButton && qApp->activeWindow() ) {
			QPoint cPos = treeWidgetHierarchy->viewport()->mapFromGlobal(QCursor::pos());
			if ( treeWidgetHierarchy->itemAt(cPos) == item ) {
				Qt::CheckState cs = (item->checkState(column) == Qt::Unchecked ? Qt::Checked : Qt::Unchecked);
				bool wasTagged = (cs != Qt::Checked);
				item->setCheckState(column, cs);
				QString machineName(item->text(QMC2_MACHINELIST_COLUMN_NAME));
				item = qmc2MachineListItemHash.value(machineName);
				if ( item )
					item->setCheckState(column, cs);
				item = qmc2CategoryItemHash.value(machineName);
				if ( item )
					item->setCheckState(column, cs);
				item = qmc2VersionItemHash.value(machineName);
				if ( item )
					item->setCheckState(column, cs);
				if ( wasTagged )
					qmc2MachineList->numTaggedSets--;
				else
					qmc2MachineList->numTaggedSets++;
				foreach (MachineListViewer *v, machineListViewers)
					v->mainTagChanged(machineName, !wasTagged);
				labelMachineListStatus->setText(qmc2MachineList->status());
			}
		}
	}
}

void MainWindow::on_treeWidgetHierarchy_itemPressed(QTreeWidgetItem *item, int column)
{
	on_treeWidgetHierarchy_itemEntered(item, column);
}

void MainWindow::on_treeWidgetCategoryView_itemEntered(QTreeWidgetItem *item, int column)
{
	if ( column == QMC2_MACHINELIST_COLUMN_TAG ) {
		if ( item->parent() ) {
			if ( qApp->mouseButtons() == Qt::LeftButton && qApp->activeWindow() ) {
				QPoint cPos = treeWidgetCategoryView->viewport()->mapFromGlobal(QCursor::pos());
				if ( treeWidgetCategoryView->itemAt(cPos) == item ) {
					Qt::CheckState cs = (item->checkState(column) == Qt::Unchecked ? Qt::Checked : Qt::Unchecked);
					bool wasTagged = (cs != Qt::Checked);
					item->setCheckState(column, cs);
					QString machineName(item->text(QMC2_MACHINELIST_COLUMN_NAME));
					item = qmc2MachineListItemHash.value(machineName);
					if ( item )
						item->setCheckState(column, cs);
					item = qmc2HierarchyItemHash.value(machineName);
					if ( item )
						item->setCheckState(column, cs);
					item = qmc2VersionItemHash.value(machineName);
					if ( item )
						item->setCheckState(column, cs);
					if ( wasTagged )
						qmc2MachineList->numTaggedSets--;
					else
						qmc2MachineList->numTaggedSets++;
					foreach (MachineListViewer *v, machineListViewers)
						v->mainTagChanged(machineName, !wasTagged);
					labelMachineListStatus->setText(qmc2MachineList->status());
				}
			}
		}
	}
}

void MainWindow::on_treeWidgetCategoryView_itemPressed(QTreeWidgetItem *item, int column)
{
	on_treeWidgetCategoryView_itemEntered(item, column);
}

void MainWindow::on_treeWidgetVersionView_itemEntered(QTreeWidgetItem *item, int column)
{
	if ( column == QMC2_MACHINELIST_COLUMN_TAG ) {
		if ( item->parent() ) {
			if ( qApp->mouseButtons() == Qt::LeftButton && qApp->activeWindow() ) {
				QPoint cPos = treeWidgetVersionView->viewport()->mapFromGlobal(QCursor::pos());
				if ( treeWidgetVersionView->itemAt(cPos) == item ) {
					Qt::CheckState cs = (item->checkState(column) == Qt::Unchecked ? Qt::Checked : Qt::Unchecked);
					bool wasTagged = (cs != Qt::Checked);
					item->setCheckState(column, cs);
					QString machineName(item->text(QMC2_MACHINELIST_COLUMN_NAME));
					item = qmc2MachineListItemHash.value(machineName);
					if ( item )
						item->setCheckState(column, cs);
					item = qmc2HierarchyItemHash.value(machineName);
					if ( item )
						item->setCheckState(column, cs);
					item = qmc2CategoryItemHash.value(machineName);
					if ( item )
						item->setCheckState(column, cs);
					if ( wasTagged )
						qmc2MachineList->numTaggedSets--;
					else
						qmc2MachineList->numTaggedSets++;
					foreach (MachineListViewer *v, machineListViewers)
						v->mainTagChanged(machineName, !wasTagged);
					labelMachineListStatus->setText(qmc2MachineList->status());
				}
			}
		}
	}
}

void MainWindow::on_treeWidgetVersionView_itemPressed(QTreeWidgetItem *item, int column)
{
	on_treeWidgetVersionView_itemEntered(item, column);
}

void MainWindow::treeWidgetCategoryViewHeader_customContextMenuRequested(const QPoint &p)
{
	menuCategoryHeader->move(adjustedWidgetPosition(treeWidgetCategoryView->header()->viewport()->mapToGlobal(p), menuCategoryHeader));
	menuCategoryHeader->show();
}

void MainWindow::actionCategoryHeader_triggered()
{
	QAction *action = (QAction *)sender();
	int visibleColumns = 0;
	for (int i = 0; i < treeWidgetCategoryView->columnCount(); i++) if ( !treeWidgetCategoryView->isColumnHidden(i) ) visibleColumns++;
	if ( action ) {
		if ( action->data().toInt() == QMC2_MACHINELIST_RESET ) {
			treeWidgetCategoryView->setColumnHidden(QMC2_MACHINELIST_COLUMN_CATEGORY, true);
			for (int i = 0; i < treeWidgetCategoryView->columnCount(); i++) {
				if ( i == QMC2_MACHINELIST_COLUMN_VERSION ) {
				       	if ( qmc2VersionInfoUsed )
						treeWidgetCategoryView->setColumnHidden(i, false);
				} else if ( i != QMC2_MACHINELIST_COLUMN_CATEGORY )
					treeWidgetCategoryView->setColumnHidden(i, false);
			}
			treeWidgetCategoryView->header()->resizeSections(QHeaderView::Stretch);
			foreach (QAction *a, menuCategoryHeader->actions())
				if ( a->isCheckable() ) {
					a->blockSignals(true);
					a->setChecked(true);
					a->blockSignals(false);
				}
			return;
		}
		bool visibility = true;
		if ( action->isChecked() )
			treeWidgetCategoryView->setColumnHidden(action->data().toInt(), false);
		else if ( visibleColumns > 1 ) {
			treeWidgetCategoryView->setColumnHidden(action->data().toInt(), true);
			visibility = false;
		}
		action->blockSignals(true);
		action->setChecked(visibility);
		action->blockSignals(false);
		QTimer::singleShot(0, this, SLOT(updateUserData()));
	}
}

void MainWindow::treeWidgetVersionViewHeader_customContextMenuRequested(const QPoint &p)
{
	menuVersionHeader->move(adjustedWidgetPosition(treeWidgetVersionView->header()->viewport()->mapToGlobal(p), menuVersionHeader));
	menuVersionHeader->show();
}

void MainWindow::actionVersionHeader_triggered()
{
	QAction *action = (QAction *)sender();
	int visibleColumns = 0;
	for (int i = 0; i < treeWidgetVersionView->columnCount(); i++) if ( !treeWidgetVersionView->isColumnHidden(i) ) visibleColumns++;
	if ( action ) {
		if ( action->data().toInt() == QMC2_MACHINELIST_RESET ) {
			treeWidgetVersionView->setColumnHidden(QMC2_MACHINELIST_COLUMN_VERSION, true);
			for (int i = 0; i < treeWidgetVersionView->columnCount(); i++) {
				if ( i == QMC2_MACHINELIST_COLUMN_CATEGORY ) {
				       	if ( qmc2CategoryInfoUsed )
						treeWidgetVersionView->setColumnHidden(i, false);
				} else if ( i != QMC2_MACHINELIST_COLUMN_VERSION )
					treeWidgetVersionView->setColumnHidden(i, false);
			}
			treeWidgetVersionView->header()->resizeSections(QHeaderView::Stretch);
			foreach (QAction *a, menuVersionHeader->actions())
				if ( a->isCheckable() ) {
					a->blockSignals(true);
					a->setChecked(true);
					a->blockSignals(false);
				}
			return;
		}
		bool visibility = true;
		if ( action->isChecked() )
			treeWidgetVersionView->setColumnHidden(action->data().toInt(), false);
		else if ( visibleColumns > 1 ) {
			treeWidgetVersionView->setColumnHidden(action->data().toInt(), true);
			visibility = false;
		}
		action->blockSignals(true);
		action->setChecked(visibility);
		action->blockSignals(false);
		QTimer::singleShot(0, this, SLOT(updateUserData()));
	}
}

void MainWindow::on_actionPlayTagged_triggered(bool)
{
	progressBarMachineList->reset();
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
		progressBarMachineList->setFormat(tr("Play tagged - %p%"));
	else
		progressBarMachineList->setFormat("%p%");
	progressBarMachineList->setRange(0, qmc2MachineList->numMachines);
	qApp->processEvents();
	int count = 0;
	QTreeWidgetItem *oldCurrentItem = qmc2CurrentItem;
	QHashIterator<QString, QTreeWidgetItem *> it(qmc2MachineListItemHash);
	QTreeWidgetItem *item;
	while ( it.hasNext() ) {
		progressBarMachineList->setValue(count++);
		it.next();
		item = qmc2MachineListItemHash.value(it.key());
		if ( item ) {
			if ( item->checkState(QMC2_MACHINELIST_COLUMN_TAG) == Qt::Checked ) {
				qApp->processEvents();
				qmc2CurrentItem = item;
				on_actionPlay_triggered();
			}
		}
	}
	qmc2CurrentItem = oldCurrentItem;
	progressBarMachineList->reset();
}

void MainWindow::on_actionPlayEmbeddedTagged_triggered(bool)
{
	progressBarMachineList->reset();
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
		progressBarMachineList->setFormat(tr("Play tagged - %p%"));
	else
		progressBarMachineList->setFormat("%p%");
	progressBarMachineList->setRange(0, qmc2MachineList->numMachines);
	qApp->processEvents();
	int count = 0;
	QTreeWidgetItem *oldCurrentItem = qmc2CurrentItem;
	QHashIterator<QString, QTreeWidgetItem *> it(qmc2MachineListItemHash);
	QTreeWidgetItem *item;
	while ( it.hasNext() ) {
		progressBarMachineList->setValue(count++);
		it.next();
		item = qmc2MachineListItemHash.value(it.key());
		if ( item ) {
			if ( item->checkState(QMC2_MACHINELIST_COLUMN_TAG) == Qt::Checked ) {
				qApp->processEvents();
				qmc2CurrentItem = item;
				qmc2StartEmbedded = true;
				on_actionPlay_triggered();
				QTest::qWait(2*QMC2_EMBED_DELAY);
			}
		}
	}
	qmc2CurrentItem = oldCurrentItem;
	progressBarMachineList->reset();
}

void MainWindow::on_actionToFavoritesTagged_triggered(bool)
{
	progressBarMachineList->reset();
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
		progressBarMachineList->setFormat(tr("Add favorites - %p%"));
	else
		progressBarMachineList->setFormat("%p%");
	progressBarMachineList->setRange(0, qmc2MachineList->numMachines);
	qApp->processEvents();
	int count = 0;
	QTreeWidgetItem *oldCurrentItem = qmc2CurrentItem;
	QHashIterator<QString, QTreeWidgetItem *> it(qmc2MachineListItemHash);
	QTreeWidgetItem *item;
	while ( it.hasNext() ) {
		progressBarMachineList->setValue(count++);
		it.next();
		item = qmc2MachineListItemHash.value(it.key());
		if ( item ) {
			if ( item->checkState(QMC2_MACHINELIST_COLUMN_TAG) == Qt::Checked ) {
				qApp->processEvents();
				qmc2CurrentItem = item;
				on_actionToFavorites_triggered();
			}
		}
	}
	qmc2CurrentItem = oldCurrentItem;
	progressBarMachineList->reset();
}

void MainWindow::on_actionCheckROMStateTagged_triggered(bool)
{
	if ( isActiveState ) {
		log(QMC2_LOG_FRONTEND, tr("please wait for current activity to finish and try again (this batch-mode operation can only run exclusively)"));
		return;
	}
	QHashIterator<QString, QTreeWidgetItem *> it(qmc2MachineListItemHash);
	QTreeWidgetItem *item;
	qmc2LoadingInterrupted = false;
	while ( it.hasNext() && !qmc2LoadingInterrupted ) {
		it.next();
		item = qmc2MachineListItemHash.value(it.key());
		if ( item ) {
			if ( item->checkState(QMC2_MACHINELIST_COLUMN_TAG) == Qt::Checked ) {
				qApp->processEvents();
				qmc2CurrentItem = item;
				on_actionCheckCurrentROM_triggered();
				while ( qmc2VerifyActive && !qmc2LoadingInterrupted )
					QTest::qWait(QMC2_TAGGEDROMCHECK_DELAY);
			}
		}
	}
}

void MainWindow::on_actionAnalyseROMTagged_triggered(bool)
{
	if ( qmc2MachineList->numTaggedSets <= 0 )
		return;
	if ( isActiveState ) {
		log(QMC2_LOG_FRONTEND, tr("please wait for current activity to finish and try again (this batch-mode operation can only run exclusively)"));
		return;
	}
	QStringList setsToAnalyze;
	QHashIterator<QString, QTreeWidgetItem *> it(qmc2MachineListItemHash);
	QTreeWidgetItem *item;
	while ( it.hasNext() ) {
		it.next();
		item = qmc2MachineListItemHash.value(it.key());
		if ( item )
			if ( item->checkState(QMC2_MACHINELIST_COLUMN_TAG) == Qt::Checked )
				setsToAnalyze << item->text(QMC2_MACHINELIST_COLUMN_NAME);
	}
	std::sort(setsToAnalyze.begin(), setsToAnalyze.end());
	if ( !qmc2SystemROMAlyzer )
		qmc2SystemROMAlyzer = new ROMAlyzer(0, QMC2_ROMALYZER_MODE_SYSTEM);
	qmc2SystemROMAlyzer->quickSearch = true;
	qmc2SystemROMAlyzer->lineEditSets->setText(setsToAnalyze.join(" "));
	if ( qmc2SystemROMAlyzer->isHidden() )
		qmc2SystemROMAlyzer->show();
	else if ( qmc2SystemROMAlyzer->isMinimized() )
		qmc2SystemROMAlyzer->showNormal();
	if ( qmc2SystemROMAlyzer->tabWidgetAnalysis->currentWidget() != qmc2SystemROMAlyzer->tabReport && qmc2SystemROMAlyzer->tabWidgetAnalysis->currentWidget() != qmc2SystemROMAlyzer->tabLog )
		qmc2SystemROMAlyzer->tabWidgetAnalysis->setCurrentWidget(qmc2SystemROMAlyzer->tabReport);
	QTimer::singleShot(0, qmc2SystemROMAlyzer, SLOT(raise()));
	QTimer::singleShot(0, qmc2SystemROMAlyzer->pushButtonAnalyze, SLOT(animateClick()));
}

void MainWindow::on_actionRunRomToolTagged_triggered(bool)
{
	if ( isActiveState ) {
		log(QMC2_LOG_FRONTEND, tr("please wait for current activity to finish and try again (this batch-mode operation can only run exclusively)"));
		return;
	}
	progressBarMachineList->reset();
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
		progressBarMachineList->setFormat(tr("ROM tool tagged - %p%"));
	else
		progressBarMachineList->setFormat("%p%");
	progressBarMachineList->setRange(0, qmc2MachineList->numMachines);
	qApp->processEvents();
	int count = 0;
	QTreeWidgetItem *oldCurrentItem = qmc2CurrentItem;
	QHashIterator<QString, QTreeWidgetItem *> it(qmc2MachineListItemHash);
	QTreeWidgetItem *item;
	while ( it.hasNext() && !qmc2LoadingInterrupted ) {
		progressBarMachineList->setValue(count++);
		it.next();
		item = qmc2MachineListItemHash.value(it.key());
		if ( item ) {
			if ( item->checkState(QMC2_MACHINELIST_COLUMN_TAG) == Qt::Checked ) {
				qApp->processEvents();
				qmc2CurrentItem = item;
				on_actionRunRomTool_triggered();
			}
		}
	}
	qmc2CurrentItem = oldCurrentItem;
	progressBarMachineList->reset();
}

void MainWindow::on_actionSetTag_triggered(bool)
{
	if ( !qmc2CurrentItem )
		return;
	bool wasUntagged = false;
	QString machineName(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_NAME));
	QTreeWidgetItem *item = qmc2MachineListItemHash.value(machineName);
	if ( item ) {
		wasUntagged = (item->checkState(QMC2_MACHINELIST_COLUMN_TAG) == Qt::Unchecked);
		item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, Qt::Checked);
	}
	item = qmc2HierarchyItemHash.value(machineName);
	if ( item )
		item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, Qt::Checked);
	item = qmc2CategoryItemHash.value(machineName);
	if ( item )
		item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, Qt::Checked);
	item = qmc2VersionItemHash.value(machineName);
	if ( item )
		item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, Qt::Checked);
	if ( wasUntagged ) {
		qmc2MachineList->numTaggedSets++;
		labelMachineListStatus->setText(qmc2MachineList->status());
	}
	foreach (MachineListViewer *v, machineListViewers)
		v->mainTagChanged(machineName, true);
}

void MainWindow::on_actionUnsetTag_triggered(bool)
{
	if ( !qmc2CurrentItem )
		return;
	bool wasTagged = false;
	QString machineName(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_NAME));
	QTreeWidgetItem *item = qmc2MachineListItemHash.value(machineName);
	if ( item ) {
		wasTagged = (item->checkState(QMC2_MACHINELIST_COLUMN_TAG) == Qt::Checked);
		item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, Qt::Unchecked);
	}
	item = qmc2HierarchyItemHash.value(machineName);
	if ( item )
		item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, Qt::Unchecked);
	item = qmc2CategoryItemHash.value(machineName);
	if ( item )
		item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, Qt::Unchecked);
	item = qmc2VersionItemHash.value(machineName);
	if ( item )
		item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, Qt::Unchecked);
	if ( wasTagged ) {
		qmc2MachineList->numTaggedSets--;
		labelMachineListStatus->setText(qmc2MachineList->status());
	}
	foreach (MachineListViewer *v, machineListViewers)
		v->mainTagChanged(machineName, false);
}

void MainWindow::on_actionToggleTag_triggered(bool)
{
	if ( !qmc2CurrentItem )
		return;
	QString machineName(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_NAME));
	QTreeWidgetItem *item = qmc2MachineListItemHash.value(machineName);
	if ( item ) {
		Qt::CheckState cs = (item->checkState(QMC2_MACHINELIST_COLUMN_TAG) == Qt::Unchecked ? Qt::Checked : Qt::Unchecked);
		bool wasTagged = (cs != Qt::Checked);
		item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, cs);
		item = qmc2HierarchyItemHash.value(machineName);
		if ( item )
			item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, cs);
		item = qmc2CategoryItemHash.value(machineName);
		if ( item )
			item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, cs);
		item = qmc2VersionItemHash.value(machineName);
		if ( item )
			item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, cs);
		if ( wasTagged )
			qmc2MachineList->numTaggedSets--;
		else
			qmc2MachineList->numTaggedSets++;
		labelMachineListStatus->setText(qmc2MachineList->status());
		foreach (MachineListViewer *v, machineListViewers)
			v->mainTagChanged(machineName, !wasTagged);
	}
}

void MainWindow::on_actionToggleTagCursorDown_triggered(bool)
{
	if ( !qmc2CurrentItem )
		return;
	QWidget *focusWidget = qApp->focusWidget();
	if ( qmc2ActiveViews.contains(focusWidget) ) {
		bool doToggle = true;
		if ( focusWidget == treeWidgetMachineList ) {
			QTreeWidgetItem *item = treeWidgetMachineList->selectedItems()[0];
			if ( item->parent() )
				doToggle = false;
			while ( item->parent() )
				item = item->parent();
			qmc2CurrentItem = item;
		} else if ( focusWidget == treeWidgetCategoryView ) {
			QTreeWidgetItem *item = treeWidgetCategoryView->selectedItems()[0];
			if ( !item->parent() )
				doToggle = false;
		} else if ( focusWidget == treeWidgetVersionView ) {
			QTreeWidgetItem *item = treeWidgetVersionView->selectedItems()[0];
			if ( !item->parent() )
				doToggle = false;
		}
		if ( doToggle )
			on_actionToggleTag_triggered();
		QKeyEvent *pressEv = new QKeyEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
		QKeyEvent *releaseEv = new QKeyEvent(QEvent::KeyRelease, Qt::Key_Down, Qt::NoModifier);
		qApp->postEvent(focusWidget, pressEv);
		qApp->postEvent(focusWidget, releaseEv);
	}
}

void MainWindow::on_actionToggleTagCursorUp_triggered(bool)
{
	if ( !qmc2CurrentItem )
		return;
	QWidget *focusWidget = qApp->focusWidget();
	if ( qmc2ActiveViews.contains(focusWidget) ) {
		bool doToggle = true;
		if ( focusWidget == treeWidgetMachineList ) {
			QTreeWidgetItem *item = treeWidgetMachineList->selectedItems()[0];
			if ( item->parent() )
				doToggle = false;
			while ( item->parent() )
				item = item->parent();
			qmc2CurrentItem = item;
		} else if ( focusWidget == treeWidgetCategoryView ) {
			QTreeWidgetItem *item = treeWidgetCategoryView->selectedItems()[0];
			if ( !item->parent() )
				doToggle = false;
		} else if ( focusWidget == treeWidgetVersionView ) {
			QTreeWidgetItem *item = treeWidgetVersionView->selectedItems()[0];
			if ( !item->parent() )
				doToggle = false;
		}
		if ( doToggle )
			on_actionToggleTag_triggered();
		QKeyEvent *pressEv = new QKeyEvent(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
		QKeyEvent *releaseEv = new QKeyEvent(QEvent::KeyRelease, Qt::Key_Up, Qt::NoModifier);
		qApp->postEvent(focusWidget, pressEv);
		qApp->postEvent(focusWidget, releaseEv);
	}
}

void MainWindow::showLoadAnim(QString text, bool enable)
{
	if ( enable ) {
		treeWidgetMachineList->setVisible(false);
		((AspectRatioLabel *)labelLoadingMachineList)->setLabelText(text);
		labelLoadingMachineList->setVisible(true);
		treeWidgetHierarchy->setVisible(false);
		((AspectRatioLabel *)labelLoadingHierarchy)->setLabelText(text);
		labelLoadingHierarchy->setVisible(true);
		treeWidgetCategoryView->setVisible(false);
		((AspectRatioLabel *)labelCreatingCategoryView)->setLabelText(text);
		labelCreatingCategoryView->setVisible(true);
		treeWidgetVersionView->setVisible(false);
		((AspectRatioLabel *)labelCreatingVersionView)->setLabelText(text);
		labelCreatingVersionView->setVisible(true);
		if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ShowLoadingAnimation", true).toBool() )
			loadAnimMovie->start();
	} else {
		loadAnimMovie->setPaused(true);
		treeWidgetMachineList->setVisible(true);
		labelLoadingMachineList->setVisible(false);
		treeWidgetHierarchy->setVisible(true);
		labelLoadingHierarchy->setVisible(false);
		treeWidgetCategoryView->setVisible(true);
		labelCreatingCategoryView->setVisible(false);
		treeWidgetVersionView->setVisible(true);
		labelCreatingVersionView->setVisible(false);
	}
	qApp->processEvents();
}

void MainWindow::on_actionTagAll_triggered(bool)
{
	if ( qmc2ReloadActive )
		return;
	progressBarMachineList->reset();
	QString oldFormat = progressBarMachineList->format();
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
		progressBarMachineList->setFormat(tr("Tag - %p%"));
	else
		progressBarMachineList->setFormat("%p%");
	progressBarMachineList->setRange(0, qmc2MachineList->numTotalMachines + qmc2MachineList->deviceSets.count());
	showLoadAnim(tr("Tagging, please wait..."));
	QHashIterator<QString, QTreeWidgetItem *> it(qmc2MachineListItemHash);
	QTreeWidgetItem *item;
	QString id;
	int taggingResponse = qmc2MachineListItemHash.count() / QMC2_TAGGING_UPDATES;
	int count = 0;
	while ( it.hasNext() ) {
		it.next();
		id = it.key();
		item = qmc2MachineListItemHash.value(id);
		if ( !item )
			continue;
		if ( item )
			item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, Qt::Checked);
		item = qmc2HierarchyItemHash.value(id);
		if ( item )
			item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, Qt::Checked);
		item = qmc2CategoryItemHash.value(id);
		if ( item )
			item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, Qt::Checked);
		item = qmc2VersionItemHash.value(id);
		if ( item )
			item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, Qt::Checked);
		qmc2MachineList->numTaggedSets++;
		foreach (MachineListViewer *v, machineListViewers)
			v->mainTagChanged(id, true);
		if ( count++ % taggingResponse == 0 ) {
			progressBarMachineList->setValue(count);
			labelMachineListStatus->setText(qmc2MachineList->status());
			qApp->processEvents();
		}
	}
	qmc2MachineList->numTaggedSets = qmc2MachineList->numMachines;
	progressBarMachineList->setValue(count);
	labelMachineListStatus->setText(qmc2MachineList->status());
	hideLoadAnim();
	progressBarMachineList->setFormat(oldFormat);
	progressBarMachineList->reset();
}

void MainWindow::on_actionUntagAll_triggered(bool)
{
	if ( qmc2ReloadActive )
		return;
	progressBarMachineList->reset();
	QString oldFormat = progressBarMachineList->format();
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
		progressBarMachineList->setFormat(tr("Untag - %p%"));
	else
		progressBarMachineList->setFormat("%p%");
	progressBarMachineList->setRange(0, qmc2MachineList->numTotalMachines + qmc2MachineList->deviceSets.count());
	showLoadAnim(tr("Untagging, please wait..."));
	QHashIterator<QString, QTreeWidgetItem *> it(qmc2MachineListItemHash);
	QTreeWidgetItem *item;
	QString id;
	int taggingResponse = qmc2MachineListItemHash.count() / QMC2_TAGGING_UPDATES;
	int count = 0;
	while ( it.hasNext() ) {
		it.next();
		id = it.key();
		item = qmc2MachineListItemHash.value(id);
		if ( !item )
			continue;
		if ( item )
			item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, Qt::Unchecked);
		item = qmc2HierarchyItemHash.value(id);
		if ( item )
			item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, Qt::Unchecked);
		item = qmc2CategoryItemHash.value(id);
		if ( item )
			item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, Qt::Unchecked);
		item = qmc2VersionItemHash.value(id);
		if ( item )
			item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, Qt::Unchecked);
		qmc2MachineList->numTaggedSets--;
		foreach (MachineListViewer *v, machineListViewers)
			v->mainTagChanged(id, false);
		if ( count++ % taggingResponse == 0 ) {
			progressBarMachineList->setValue(count);
			labelMachineListStatus->setText(qmc2MachineList->status());
			qApp->processEvents();
		}
	}
	qmc2MachineList->numTaggedSets = 0;
	progressBarMachineList->setValue(count);
	labelMachineListStatus->setText(qmc2MachineList->status());
	hideLoadAnim();
	progressBarMachineList->setFormat(oldFormat);
	progressBarMachineList->reset();
}

void MainWindow::on_actionInvertTags_triggered(bool)
{
	if ( qmc2ReloadActive )
		return;
	progressBarMachineList->reset();
	QString oldFormat = progressBarMachineList->format();
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
		progressBarMachineList->setFormat(tr("Invert tag - %p%"));
	else
		progressBarMachineList->setFormat("%p%");
	progressBarMachineList->setRange(0, qmc2MachineList->numTotalMachines + qmc2MachineList->deviceSets.count());
	showLoadAnim(tr("Inverting tags, please wait..."));
	QHashIterator<QString, QTreeWidgetItem *> it(qmc2MachineListItemHash);
	QTreeWidgetItem *item;
	QString id;
	int taggingResponse = qmc2MachineListItemHash.count() / QMC2_TAGGING_UPDATES;
	int count = 0;
	while ( it.hasNext() ) {
		it.next();
		id = it.key();
		item = qmc2MachineListItemHash.value(id);
		if ( !item )
			continue;
		Qt::CheckState cs = (item->checkState(QMC2_MACHINELIST_COLUMN_TAG) == Qt::Unchecked ? Qt::Checked : Qt::Unchecked);
		bool wasTagged = (cs != Qt::Checked);
		item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, cs);
		item = qmc2HierarchyItemHash.value(id);
		if ( item )
			item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, cs);
		item = qmc2CategoryItemHash.value(id);
		if ( item )
			item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, cs);
		item = qmc2VersionItemHash.value(id);
		if ( item )
			item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, cs);
		if ( wasTagged )
			qmc2MachineList->numTaggedSets--;
		else
			qmc2MachineList->numTaggedSets++;
		foreach (MachineListViewer *v, machineListViewers)
			v->mainTagChanged(id, !wasTagged);
		if ( count++ % taggingResponse == 0 ) {
			progressBarMachineList->setValue(count);
			labelMachineListStatus->setText(qmc2MachineList->status());
			qApp->processEvents();
		}
	}
	progressBarMachineList->setValue(count);
	labelMachineListStatus->setText(qmc2MachineList->status());
	hideLoadAnim();
	progressBarMachineList->setFormat(oldFormat);
	progressBarMachineList->reset();
}

void MainWindow::on_actionTagVisible_triggered(bool)
{
	if ( qmc2ReloadActive )
		return;
	progressBarMachineList->reset();
	QString oldFormat = progressBarMachineList->format();
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
		progressBarMachineList->setFormat(tr("Tag - %p%"));
	else
		progressBarMachineList->setFormat("%p%");
	progressBarMachineList->setRange(0, qmc2MachineList->numTotalMachines + qmc2MachineList->deviceSets.count());
	showLoadAnim(tr("Tagging, please wait..."));
	QHashIterator<QString, QTreeWidgetItem *> it(qmc2MachineListItemHash);
	QTreeWidgetItem *item;
	QString id;
	int taggingResponse = qmc2MachineListItemHash.count() / QMC2_TAGGING_UPDATES;
	int count = 0;
	while ( it.hasNext() ) {
		it.next();
		id = it.key();
		item = qmc2MachineListItemHash.value(id);
		if ( !item )
			continue;
		if ( item->isHidden() )
			continue;
		item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, Qt::Checked);
		item = qmc2HierarchyItemHash.value(id);
		if ( item )
			item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, Qt::Checked);
		item = qmc2CategoryItemHash.value(id);
		if ( item )
			item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, Qt::Checked);
		item = qmc2VersionItemHash.value(id);
		if ( item )
			item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, Qt::Checked);
		qmc2MachineList->numTaggedSets++;
		foreach (MachineListViewer *v, machineListViewers)
			v->mainTagChanged(id, true);
		if ( count++ % taggingResponse == 0 ) {
			progressBarMachineList->setValue(count);
			labelMachineListStatus->setText(qmc2MachineList->status());
			qApp->processEvents();
		}
	}
	progressBarMachineList->setValue(count);
	labelMachineListStatus->setText(qmc2MachineList->status());
	hideLoadAnim();
	progressBarMachineList->setFormat(oldFormat);
	progressBarMachineList->reset();
}

void MainWindow::on_actionUntagVisible_triggered(bool)
{
	if ( qmc2ReloadActive )
		return;
	progressBarMachineList->reset();
	QString oldFormat = progressBarMachineList->format();
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
		progressBarMachineList->setFormat(tr("Untag - %p%"));
	else
		progressBarMachineList->setFormat("%p%");
	progressBarMachineList->setRange(0, qmc2MachineList->numTotalMachines + qmc2MachineList->deviceSets.count());
	showLoadAnim(tr("Untagging, please wait..."));
	QHashIterator<QString, QTreeWidgetItem *> it(qmc2MachineListItemHash);
	QTreeWidgetItem *item;
	QString id;
	int taggingResponse = qmc2MachineListItemHash.count() / QMC2_TAGGING_UPDATES;
	int count = 0;
	while ( it.hasNext() ) {
		it.next();
		id = it.key();
		item = qmc2MachineListItemHash.value(id);
		if ( !item )
			continue;
		if ( item->isHidden() )
			continue;
		item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, Qt::Unchecked);
		item = qmc2HierarchyItemHash.value(id);
		if ( item )
			item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, Qt::Unchecked);
		item = qmc2CategoryItemHash.value(id);
		if ( item )
			item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, Qt::Unchecked);
		item = qmc2VersionItemHash.value(id);
		if ( item )
			item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, Qt::Unchecked);
		qmc2MachineList->numTaggedSets--;
		foreach (MachineListViewer *v, machineListViewers)
			v->mainTagChanged(id, false);
		if ( count++ % taggingResponse == 0 ) {
			progressBarMachineList->setValue(count);
			labelMachineListStatus->setText(qmc2MachineList->status());
			qApp->processEvents();
		}
	}
	progressBarMachineList->setValue(count);
	labelMachineListStatus->setText(qmc2MachineList->status());
	hideLoadAnim();
	progressBarMachineList->setFormat(oldFormat);
	progressBarMachineList->reset();
}

void MainWindow::on_actionInvertVisibleTags_triggered(bool)
{
	if ( qmc2ReloadActive )
		return;
	progressBarMachineList->reset();
	QString oldFormat = progressBarMachineList->format();
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
		progressBarMachineList->setFormat(tr("Invert tag - %p%"));
	else
		progressBarMachineList->setFormat("%p%");
	progressBarMachineList->setRange(0, qmc2MachineList->numTotalMachines + qmc2MachineList->deviceSets.count());
	showLoadAnim(tr("Inverting tags, please wait..."));
	QHashIterator<QString, QTreeWidgetItem *> it(qmc2MachineListItemHash);
	QTreeWidgetItem *item;
	QString id;
	int taggingResponse = qmc2MachineListItemHash.count() / QMC2_TAGGING_UPDATES;
	int count = 0;
	while ( it.hasNext() ) {
		it.next();
		id = it.key();
		item = qmc2MachineListItemHash.value(id);
		if ( !item )
			continue;
		if ( item->isHidden() )
			continue;
		Qt::CheckState cs = (item->checkState(QMC2_MACHINELIST_COLUMN_TAG) == Qt::Unchecked ? Qt::Checked : Qt::Unchecked);
		bool wasTagged = (cs != Qt::Checked);
		item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, cs);
		item = qmc2HierarchyItemHash.value(id);
		if ( item )
			item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, cs);
		item = qmc2CategoryItemHash.value(id);
		if ( item )
			item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, cs);
		item = qmc2VersionItemHash.value(id);
		if ( item )
			item->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, cs);
		if ( wasTagged )
			qmc2MachineList->numTaggedSets--;
		else
			qmc2MachineList->numTaggedSets++;
		foreach (MachineListViewer *v, machineListViewers)
			v->mainTagChanged(id, !wasTagged);
		if ( count++ % taggingResponse == 0 ) {
			progressBarMachineList->setValue(count);
			labelMachineListStatus->setText(qmc2MachineList->status());
			qApp->processEvents();
		}
	}
	progressBarMachineList->setValue(count);
	labelMachineListStatus->setText(qmc2MachineList->status());
	hideLoadAnim();
	progressBarMachineList->setFormat(oldFormat);
	progressBarMachineList->reset();
}

void MainWindow::commonWebSearch(QString baseUrl, QTreeWidgetItem *item)
{
	if ( !item )
		return;
	QString manu(item->text(QMC2_MACHINELIST_COLUMN_MANU));
	QString searchPattern;
	if ( manu.compare(MachineList::trQuestionMark) != 0 )
		searchPattern = item->text(QMC2_MACHINELIST_COLUMN_MACHINE) + " " + manu;
	else
		searchPattern = item->text(QMC2_MACHINELIST_COLUMN_MACHINE);
	searchPattern = searchPattern.replace(QRegExp("\\((.*)\\)"), "\\1").replace(QRegExp("[\\\\,\\.\\;\\:\\'\\/\\(\\)\\[\\]\\{\\}]"), " ").replace("&", "\%26").replace(" - ", " ").simplified().trimmed();
	QStringList wordList(searchPattern.split(" ", QString::SkipEmptyParts));
	wordList.removeAll("<unknown>");
	wordList.removeAll("<generic>");
	wordList.removeDuplicates();
	QString url(wordList.join("+"));
	url.prepend(baseUrl);
	if ( actionSearchInternalBrowser->isChecked() ) {
		MiniWebBrowser *webBrowser = new MiniWebBrowser(0);
		webBrowser->homeUrl = QUrl::fromUserInput(url);
		webBrowser->setAttribute(Qt::WA_DeleteOnClose);
		if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "WebBrowser/Geometry") )
			webBrowser->restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "WebBrowser/Geometry").toByteArray());
		else {
			webBrowser->adjustSize();
			webBrowser->move(QApplication::desktop()->screen()->rect().center() - webBrowser->rect().center());
		}
		connect(webBrowser->webViewBrowser->page(), SIGNAL(windowCloseRequested()), webBrowser, SLOT(close()));
		webBrowser->show();
		webBrowser->webViewBrowser->load(webBrowser->homeUrl);
	} else
		QDesktopServices::openUrl(QUrl::fromUserInput(url));
}

void MainWindow::on_actionSearchDuckDuckGo_triggered(bool)
{
	commonWebSearch("http://duckduckgo.com/?q=", qmc2CurrentItem);
}

void MainWindow::on_actionSearchGoogle_triggered(bool)
{
	commonWebSearch("http://www.google.com/search?q=", qmc2CurrentItem);
}

void MainWindow::on_actionSearchWikipedia_triggered(bool)
{
	commonWebSearch("http://en.wikipedia.org/wiki/Special:Search?search=", qmc2CurrentItem);
}

void MainWindow::on_actionSearchYandex_triggered(bool)
{
	commonWebSearch("http://www.yandex.com/yandsearch?text=", qmc2CurrentItem);
}

void MainWindow::on_actionSearchInternalBrowser_triggered(bool checked)
{
	if ( checked )
		actionSearchInternalBrowser->setText(tr("Internal browser"));
	else
		actionSearchInternalBrowser->setText(tr("External browser"));
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "WebSearch/InternalBrowser", checked);
}

void MainWindow::on_actionManualInternalViewer_triggered(bool checked)
{
	if ( checked )
		actionManualInternalViewer->setText(tr("Internal PDF viewer"));
	else
		actionManualInternalViewer->setText(tr("External PDF viewer"));
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "PdfViewer/Internal", checked);
}

void MainWindow::on_actionManualOpenInViewer_triggered(bool)
{
	if ( !qmc2CurrentItem )
		return;
	QStringList manualPaths(userDataDb->systemManualPaths(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_NAME)));
	if ( manualPaths.isEmpty() ) {
		QString parentName(qmc2ParentHash.value(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_NAME)));
		if ( !parentName.isEmpty() )
			manualPaths = userDataDb->systemManualPaths(parentName);
	}
	if ( manualPaths.count() > 1 ) {
		ItemSelector itemSelector(this, manualPaths);
		itemSelector.setWindowTitle(tr("Manual selection"));
		itemSelector.labelMessage->setText(tr("Multiple PDF manuals exist. Select the ones you want to open:"));
		itemSelector.listWidgetItems->setSelectionMode(QAbstractItemView::ExtendedSelection);
		if ( itemSelector.exec() != QDialog::Rejected ) {
			QList<QListWidgetItem *> itemList(itemSelector.listWidgetItems->selectedItems());
			for (int i = 0; i < itemList.count(); i++) {
				QFileInfo fi(itemList.at(i)->text());
				if ( actionManualInternalViewer->isChecked() )
					viewPdf(fi.absoluteFilePath());
				else
					QDesktopServices::openUrl(QUrl::fromUserInput(fi.absoluteFilePath()));
			}
		}
	} else if ( manualPaths.count() > 0 ) {
		QFileInfo fi(manualPaths.first());
		if ( actionManualInternalViewer->isChecked() )
			viewPdf(fi.absoluteFilePath());
		else
			QDesktopServices::openUrl(QUrl::fromUserInput(fi.absoluteFilePath()));
	}
}

void MainWindow::checkSystemManualAvailability()
{
	if ( !qmc2CurrentItem ) {
		actionManualOpenInViewer->setEnabled(false);
		actionManualOpenInViewer->setIcon(QIcon(QString::fromUtf8(":/data/img/no_book.png")));
		return;
	}
	bool enable = !userDataDb->systemManualPaths(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_NAME)).isEmpty();
	if ( !enable ) {
		QString parentName(qmc2ParentHash.value(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_NAME)));
		if ( !parentName.isEmpty() )
			enable = !userDataDb->systemManualPaths(parentName).isEmpty();
	}
	actionManualOpenInViewer->setEnabled(enable);
	if ( enable )
		actionManualOpenInViewer->setIcon(QIcon(QString::fromUtf8(":/data/img/book.png")));
	else
		actionManualOpenInViewer->setIcon(QIcon(QString::fromUtf8(":/data/img/no_book.png")));
}

void MainWindow::on_actionRebuildROMTagged_triggered(bool)
{
	if ( qmc2MachineList->numTaggedSets <= 0 )
		return;

	if ( isActiveState ) {
		log(QMC2_LOG_FRONTEND, tr("please wait for current activity to finish and try again (this batch-mode operation can only run exclusively)"));
		return;
	}

	if ( qmc2SystemROMAlyzer && qmc2SystemROMAlyzer->rebuilderActive() ) {
		log(QMC2_LOG_FRONTEND, tr("please wait for ROMAlyzer to finish the current rebuild and try again"));
		return;
	}

	QStringList setsToRebuild;
	QHashIterator<QString, QTreeWidgetItem *> it(qmc2MachineListItemHash);
	QTreeWidgetItem *item;
	while ( it.hasNext() ) {
		it.next();
		item = qmc2MachineListItemHash.value(it.key());
		if ( item )
			if ( item->checkState(QMC2_MACHINELIST_COLUMN_TAG) == Qt::Checked )
				setsToRebuild << item->text(QMC2_MACHINELIST_COLUMN_NAME);
	}

	std::sort(setsToRebuild.begin(), setsToRebuild.end());

	bool initial = false;
	if ( !qmc2SystemROMAlyzer ) {
		qmc2SystemROMAlyzer = new ROMAlyzer(0, QMC2_ROMALYZER_MODE_SYSTEM);
		initial = true;
	}

	if ( qmc2SystemROMAlyzer->tabWidgetAnalysis->currentWidget() != qmc2SystemROMAlyzer->tabCollectionRebuilder )
		qmc2SystemROMAlyzer->tabWidgetAnalysis->setCurrentWidget(qmc2SystemROMAlyzer->tabCollectionRebuilder);

	if ( qmc2SystemROMAlyzer->isHidden() )
		qmc2SystemROMAlyzer->show();
	else if ( qmc2SystemROMAlyzer->isMinimized() )
		qmc2SystemROMAlyzer->showNormal();

	QTimer::singleShot(0, qmc2SystemROMAlyzer, SLOT(raise()));

	CollectionRebuilder *cr = qmc2SystemROMAlyzer->collectionRebuilder();
	if ( cr ) {
		cr->comboBoxXmlSource->setCurrentIndex(0);
		cr->setIgnoreCheckpoint(true);
		cr->checkBoxFilterExpression->setChecked(true);
		cr->comboBoxFilterSyntax->setCurrentIndex(0);
		cr->comboBoxFilterType->setCurrentIndex(0);
		cr->toolButtonExactMatch->setChecked(true);
		cr->checkBoxFilterStates->setChecked(false);
		cr->lineEditFilterExpression->setText(setsToRebuild.join("|"));
		if ( !initial )
			cr->plainTextEditLog->clear();
		QTimer::singleShot(0, cr->pushButtonStartStop, SLOT(click()));
	}
}

// note: - this routine is far from "elegant" but basically works (there may be minor conversion "bugs", though, depending on the quality of the wiki source data)
//       - if someone knows a CLEAN wiki2html converter that's not "bloated" and written in C/C++ (and legally redistributable open source code), please let us know!
QString &MainWindow::messWikiToHtml(QString &wikiText)
{
	int ulLevel = 0;
	int olLevel = 0;
	bool tableOpen = false;
	bool preOn = false;
	bool codeOn = false;
	int preCounter = 0;
	QStringList wikiLines = wikiText.split("<p>");
	wikiText.clear();
	foreach (QString wikiLine, wikiLines) {
		QString wikiLineTrimmed = wikiLine.trimmed();
		if ( wikiLine.indexOf(QRegExp("\\s*<code>")) == 0 ) {
			codeOn = true;
			continue;
		} 
		if ( wikiLine.indexOf(QRegExp("\\s*</code>")) == 0 )
			codeOn = false;
		bool listDetected = ( (wikiLineTrimmed.startsWith("* ") && wikiLine[wikiLine.indexOf("*") + 2] != ' ') || wikiLineTrimmed.startsWith("- ") );
		if ( wikiLine == "  * " || wikiLine == "  - " || wikiLine == "  *" || wikiLine == "  -" ) continue; // this is an "artifact"... ignore :)
		if ( !listDetected && (wikiLine.startsWith("  ") || codeOn) ) {
			if ( tableOpen ) { wikiText += "</table><p>"; tableOpen = false; }
			if ( ulLevel > 0 ) { for (int i = 0; i < ulLevel; i++) wikiText += "</ul>"; ulLevel = 0; wikiText += "<p>"; }
			if ( olLevel > 0 ) { for (int i = 0; i < olLevel; i++) wikiText += "</ol>"; olLevel = 0; wikiText += "<p>"; }
			if ( wikiLine == "  "  && preCounter == 0 ) {
				preCounter++;
				wikiText += "\n";
				continue;
			}
			if ( !preOn ) {
				wikiText += "<p><table border=\"1\"><tr><td><pre>";
				preOn = true;
			}
			if ( wikiLine == "  " ) {
				wikiText += "\n";
				continue;
			}
		} else if ( preOn ) {
			preCounter++;
			wikiText += "</pre></td></tr></table><p>";
			preOn = codeOn = false;
		}
		int listDepth = 0;
		if ( listDetected ) listDepth = wikiLine.indexOf(QRegExp("[\\-\\*]")) / 2;
		if ( !preOn ) {
			wikiLine = wikiLineTrimmed;
			preCounter = 0;
		}
		if ( wikiLine.isEmpty() )
			continue;
		wikiLine.replace("<", "&lt;").replace(">", "&gt;");
		if ( wikiLine.startsWith("//") && wikiLine.endsWith("//") ) {
			wikiLine.replace(0, 2, "<i>");
			wikiLine.replace(wikiLine.length() - 2, 2, "</i>");
		}
		foreach (QString snippet, wikiLine.split("//")) {
			if ( snippet.indexOf(QRegExp("^.*(http:|https:|ftp:)$")) < 0 )
				wikiLine.replace(QString("//%1//").arg(snippet), QString("<i>%1</i>").arg(snippet));
		}
		wikiLine.replace(QRegExp("\\*\\*(.*)\\*\\*"), "<b>\\1</b>");
		wikiLine.replace(QRegExp("__(.*)__"), "<u>\\1</u>");
		wikiLine.replace(QRegExp("\\[\\[wp>([^\\]]*)\\]\\]"), QLatin1String("\\1 -- http://en.wikipedia.org/wiki/\\1"));
		foreach (QString snippet, wikiLine.split("[[")) {
			if ( snippet.indexOf(QRegExp("\\]\\]|\\|")) > 0 ) {
				QStringList subSnippets = snippet.split(QRegExp("\\]\\]|\\|"));
				wikiLine.replace(QString("[[%1]]").arg(snippet), subSnippets[0]);
			}
		}
		wikiLine.replace(QRegExp(QString("((http|https|ftp)://%1)").arg(urlSectionRegExp)), QLatin1String("<a href=\"\\1\">\\1</a>"));
		if ( wikiLine.startsWith("&lt;h2&gt;======") && wikiLine.endsWith("======&lt;/h2&gt;") ) {
			if ( tableOpen ) { wikiText += "</table><p>"; tableOpen = false; }
			if ( ulLevel > 0 ) { for (int i = 0; i < ulLevel; i++) wikiText += "</ul>"; ulLevel = 0; wikiText += "<p>"; }
			if ( olLevel > 0 ) { for (int i = 0; i < olLevel; i++) wikiText += "</ol>"; olLevel = 0; wikiText += "<p>"; }
			wikiText += "<h2>" + wikiLine.mid(16, wikiLine.length() - 33) + "</h2>";
		} else if ( wikiLine.startsWith("=====") && wikiLine.endsWith("=====") ) {
			if ( tableOpen ) { wikiText += "</table><p>"; tableOpen = false; }
			if ( ulLevel > 0 ) { for (int i = 0; i < ulLevel; i++) wikiText += "</ul>"; ulLevel = 0; wikiText += "<p>"; }
			if ( olLevel > 0 ) { for (int i = 0; i < olLevel; i++) wikiText += "</ol>"; olLevel = 0; wikiText += "<p>"; }
			wikiText += "<h3>" + wikiLine.mid(6, wikiLine.length() - 12) + "</h3>";
		} else if ( wikiLine.startsWith("====") && wikiLine.endsWith("====") ) {
			if ( tableOpen ) { wikiText += "</table><p>"; tableOpen = false; }
			if ( ulLevel > 0 ) { for (int i = 0; i < ulLevel; i++) wikiText += "</ul>"; ulLevel = 0; wikiText += "<p>"; }
			if ( olLevel > 0 ) { for (int i = 0; i < olLevel; i++) wikiText += "</ol>"; olLevel = 0; wikiText += "<p>"; }
			wikiText += "<h4>" + wikiLine.mid(5, wikiLine.length() - 10) + "</h4>";
		} else if ( wikiLine.startsWith("===") && wikiLine.endsWith("===") ) {
			if ( tableOpen ) { wikiText += "</table><p>"; tableOpen = false; }
			if ( ulLevel > 0 ) { for (int i = 0; i < ulLevel; i++) wikiText += "</ul>"; ulLevel = 0; wikiText += "<p>"; }
			if ( olLevel > 0 ) { for (int i = 0; i < olLevel; i++) wikiText += "</ol>"; olLevel = 0; wikiText += "<p>"; }
			wikiText += "<b>" + wikiLine.mid(4, wikiLine.length() - 8) + "</b>";
		} else if ( wikiLine.startsWith("==") && wikiLine.endsWith("==") ) {
			if ( tableOpen ) { wikiText += "</table><p>"; tableOpen = false; }
			if ( ulLevel > 0 ) { for (int i = 0; i < ulLevel; i++) wikiText += "</ul>"; ulLevel = 0; wikiText += "<p>"; }
			if ( olLevel > 0 ) { for (int i = 0; i < olLevel; i++) wikiText += "</ol>"; olLevel = 0; wikiText += "<p>"; }
			wikiText += "<b>" + wikiLine.mid(3, wikiLine.length() - 6) + "</b>";
		} else if ( wikiLine.indexOf(QRegExp("\\* \\S")) == 0 ) {
			if ( tableOpen ) { wikiText += "</table><p>"; tableOpen = false; }
			if ( olLevel > 0 ) { for (int i = 0; i < olLevel; i++) wikiText += "</ol>"; olLevel = 0; wikiText += "<p>"; }
			if ( listDepth > ulLevel ) {
				wikiText += "<ul style=\"list-style-type:square;\">";
				ulLevel++;
			} else if ( listDepth < ulLevel ) {
				wikiText += "</ul>";
				ulLevel--;
			}
			wikiText += "<li>" + wikiLine.mid(2) + "</li>";
		} else if ( wikiLine.indexOf(QRegExp("\\- \\S")) == 0 ) {
			if ( tableOpen ) { wikiText += "</table><p>"; tableOpen = false; }
			if ( ulLevel > 0 ) { for (int i = 0; i < ulLevel; i++) wikiText += "</ul>"; ulLevel = 0; wikiText += "<p>"; }
			if ( listDepth > olLevel ) {
				wikiText += "<ol style=\"list-style-type:decimal;\">";
				olLevel++;
			} else if ( listDepth < olLevel ) {
				wikiText += "</ol>";
				olLevel--;
			}
			wikiText += "<li>" + wikiLine.mid(2) + "</li>";
		} else if ( ( wikiLine.startsWith("| ") && wikiLine.endsWith(" |") ) || ( wikiLine.startsWith("^ ") && wikiLine.endsWith(" |") ) ) {
			if ( ulLevel > 0 ) { for (int i = 0; i < ulLevel; i++) wikiText += "</ul>"; ulLevel = 0; wikiText += "<p>"; }
			if ( olLevel > 0 ) { for (int i = 0; i < olLevel; i++) wikiText += "</ol>"; olLevel = 0; wikiText += "<p>"; }
			if ( !tableOpen ) { wikiText += "<p><table border=\"1\">"; tableOpen = true; }
			wikiText += "<tr>";
			foreach (QString cell, wikiLine.split(QRegExp("\\^|\\|"), QString::SkipEmptyParts)) wikiText += "<td>" + cell + "</td>";
			wikiText += "</tr>";
		} else if ( wikiLine.startsWith("^ ") && wikiLine.endsWith(" ^") ) {
			if ( ulLevel > 0 ) { for (int i = 0; i < ulLevel; i++) wikiText += "</ul>"; ulLevel = 0; wikiText += "<p>"; }
			if ( olLevel > 0 ) { for (int i = 0; i < olLevel; i++) wikiText += "</ol>"; olLevel = 0; wikiText += "<p>"; }
			if ( !tableOpen ) { wikiText += "<p><table border=\"1\">"; tableOpen = true; }
			wikiText += "<tr>";
			foreach (QString cell, wikiLine.split("^", QString::SkipEmptyParts)) wikiText += "<td><b>" + cell + "</b></td>";
			wikiText += "</tr>";
		} else {
			if ( tableOpen ) { wikiText += "</table><p>"; tableOpen = false; }
			if ( ulLevel > 0 ) { for (int i = 0; i < ulLevel; i++) wikiText += "</ul>"; ulLevel = 0; wikiText += "<p>"; }
			if ( olLevel > 0 ) { for (int i = 0; i < olLevel; i++) wikiText += "</ol>"; olLevel = 0; wikiText += "<p>"; }
			if ( preOn ) {
				wikiText += wikiLine.mid(2) + "\n";
			} else if ( codeOn ) {
				wikiText += wikiLine + "\n";
			} else
				wikiText += "<p>" + wikiLine + "</p>";
		}
	}

	if ( ulLevel > 0 )
		for (int i = 0; i < ulLevel; i++) wikiText += "</ul>";
	else if ( olLevel > 0 )
		for (int i = 0; i < olLevel; i++) wikiText += "</ol>";

	return wikiText;
}

void MainWindow::floatToggleButtonSoftwareDetail_toggled(bool checked)
{
	if ( qmc2EarlyStartup )
		return;

	if ( checked ) {
		if ( tabWidgetSoftwareDetail->parent() == this ) {
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/SoftwareDetailGeometry", tabWidgetSoftwareDetail->saveGeometry());
			stackedWidgetSpecial->insertWidget(QMC2_SPECIAL_SOFTWARE_PAGE, tabWidgetSoftwareDetail);
			stackedWidgetSpecial->setCurrentIndex(QMC2_SPECIAL_SOFTWARE_PAGE);
			adjustSplitter(vSplitter, tabWidgetLogsAndEmulators, vSplitterSizesSoftwareDetail, true);
			lastPageSoftware = true;
		}
	} else {
		stackedWidgetSpecial->setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);
		stackedWidgetSpecial->removeWidget(tabWidgetSoftwareDetail);
		tabWidgetSoftwareDetail->setParent(this);
		tabWidgetSoftwareDetail->setAttribute(Qt::WA_ShowWithoutActivating);
		tabWidgetSoftwareDetail->setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
		tabWidgetSoftwareDetail->setWindowIcon(qApp->windowIcon());
		tabWidgetSoftwareDetail->setWindowTitle(tr("Software detail"));
		if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/SoftwareDetailGeometry") )
			tabWidgetSoftwareDetail->restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/SoftwareDetailGeometry").toByteArray());
		if ( qmc2SoftwareList ) {
			tabWidgetSoftwareDetail->showNormal();
			tabWidgetSoftwareDetail->raise();
		}
		adjustSplitter(vSplitter, tabWidgetLogsAndEmulators, vSplitterSizes, false);
		if ( lastPageSoftware ) {
			ComponentInfo *componentInfo = qmc2ComponentSetup->componentInfoHash().value("Component2");
			switch ( componentInfo->appliedFeatureList().at(tabWidgetLogsAndEmulators->currentIndex()) ) {
				case QMC2_FRONTENDLOG_INDEX:
					logScrollToEnd(QMC2_LOG_FRONTEND);
					break;
				case QMC2_EMULATORLOG_INDEX:
					logScrollToEnd(QMC2_LOG_EMULATOR);
					break;
			}
		}
		lastPageSoftware = false;
	}
}

void MainWindow::adjustSplitter(QSplitter *splitter, QTabWidget *tabWidget, QList<int> &sizes, bool enable)
{
	if ( tabWidget->count() > 0 || enable ) {
		splitter->handle(1)->setEnabled(true);
		if ( splitter->orientation() == Qt::Horizontal )
			splitter->handle(1)->setFixedWidth(4);
		else
			splitter->handle(1)->setFixedHeight(4);
		splitter->setSizes(sizes);
		splitter->setHandleWidth(4);
	} else {
		splitter->handle(1)->setEnabled(false);
		if ( splitter->orientation() == Qt::Horizontal )
			splitter->handle(1)->setFixedWidth(0);
		else
			splitter->handle(1)->setFixedHeight(0);
		splitter->setSizes(sizes);
		splitter->setHandleWidth(1);
	}
}

void MainWindow::stackedWidgetSpecial_setCurrentIndex(int index)
{
	switch ( index ) {
		case QMC2_SPECIAL_DEFAULT_PAGE:
			stackedWidgetSpecial->setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);
			if ( tabWidgetSoftwareDetail->parent() == this )
				tabWidgetSoftwareDetail->hide();
			if ( qmc2SoftwareNotesEditor )
				qmc2SoftwareNotesEditor->hideTearOffMenus();
			adjustSplitter(vSplitter, tabWidgetLogsAndEmulators, vSplitterSizes, false);
			if ( lastPageSoftware ) {
				ComponentInfo *componentInfo = qmc2ComponentSetup->componentInfoHash().value("Component2");
				switch ( componentInfo->appliedFeatureList().at(tabWidgetLogsAndEmulators->currentIndex()) ) {
					case QMC2_FRONTENDLOG_INDEX:
						logScrollToEnd(QMC2_LOG_FRONTEND);
						break;
					case QMC2_EMULATORLOG_INDEX:
						logScrollToEnd(QMC2_LOG_EMULATOR);
						break;
				}
			}
			lastPageSoftware = false;
			break;

		case QMC2_SPECIAL_SOFTWARE_PAGE:
			if ( qmc2SoftwareList ) {
				if ( !floatToggleButtonSoftwareDetail->isChecked() ) {
					stackedWidgetSpecial->setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);
					if ( tabWidgetSoftwareDetail->parent() != this ) {
						stackedWidgetSpecial->removeWidget(tabWidgetSoftwareDetail);
						tabWidgetSoftwareDetail->setParent(this);
						tabWidgetSoftwareDetail->setAttribute(Qt::WA_ShowWithoutActivating);
						tabWidgetSoftwareDetail->setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
						tabWidgetSoftwareDetail->setWindowIcon(qApp->windowIcon());
						tabWidgetSoftwareDetail->setWindowTitle(tr("Software detail"));
						if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/SoftwareDetailGeometry") )
							tabWidgetSoftwareDetail->restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/SoftwareDetailGeometry").toByteArray());

					}
					adjustSplitter(vSplitter, tabWidgetLogsAndEmulators, vSplitterSizes, false);
					qmc2SoftwareList->detailUpdateTimer.start(qmc2UpdateDelay);
					tabWidgetSoftwareDetail->showNormal();
					tabWidgetSoftwareDetail->raise();
					lastPageSoftware = false;
				} else {
					stackedWidgetSpecial->setCurrentIndex(QMC2_SPECIAL_SOFTWARE_PAGE);
					adjustSplitter(vSplitter, tabWidgetLogsAndEmulators, vSplitterSizesSoftwareDetail, true);
					lastPageSoftware = true;
				}
			} else {
				stackedWidgetSpecial->setCurrentIndex(QMC2_SPECIAL_SOFTWARE_PAGE);
				adjustSplitter(vSplitter, tabWidgetLogsAndEmulators, vSplitterSizesSoftwareDetail, true);
				lastPageSoftware = true;
			}
			break;

		default:
			stackedWidgetSpecial->setCurrentIndex(index);
			adjustSplitter(vSplitter, tabWidgetLogsAndEmulators, vSplitterSizes, false);
			lastPageSoftware = false;
			break;
	}
}

void MainWindow::comboBoxToolbarSearch_activated(const QString &text)
{
	comboBoxSearch->lineEdit()->setText(text);
	int index = comboBoxSearch->findText(text);
	if ( index < 0 ) {
		comboBoxSearch->insertItem(0, text);
		comboBoxSearch->setCurrentIndex(0);
		if ( comboBoxSearch->count() > QMC2_MACHINE_SEARCH_HISTORY_LENGTH )
			comboBoxSearch->removeItem(QMC2_MACHINE_SEARCH_HISTORY_LENGTH);
	} else {
		comboBoxSearch->blockSignals(true);
		comboBoxSearch->removeItem(index);
		comboBoxSearch->insertItem(0, text);
		comboBoxSearch->setCurrentIndex(0);
		comboBoxSearch->blockSignals(false);
	}
	index = comboBoxToolbarSearch->findText(text);
	if ( index >= 0 ) {
		comboBoxToolbarSearch->removeItem(index);
		comboBoxToolbarSearch->insertItem(0, text);
		comboBoxToolbarSearch->setCurrentIndex(0);
	}
	if ( comboBoxToolbarSearch->count() > QMC2_MACHINE_SEARCH_HISTORY_LENGTH )
		comboBoxToolbarSearch->removeItem(QMC2_MACHINE_SEARCH_HISTORY_LENGTH);
	if ( tabWidgetMachineList->currentWidget() != tabSearch ) {
		tabWidgetMachineList->blockSignals(true);
		tabWidgetMachineList->setCurrentWidget(tabSearch);
		tabWidgetMachineList->blockSignals(false);
	}
	comboBoxSearch_editTextChanged_delayed();
	listWidgetSearch->setFocus();
}

void MainWindow::comboBoxToolbarSearch_editTextChanged(const QString &text)
{
	comboBoxSearch->lineEdit()->setText(text);
}

void MainWindow::treeWidgetMachineList_verticalScrollChanged(int)
{
	if ( !treeWidgetMachineList->isColumnHidden(QMC2_MACHINELIST_COLUMN_RANK) )
		m_mlRankUpdateTimer.start(QMC2_RANK_UPDATE_DELAY);
}

void MainWindow::treeWidgetMachineList_updateRanks()
{
  	if ( !qmc2CurrentItem || qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_MACHINE) == MachineList::trWaitingForData )
		return;

	QTreeWidget *treeWidget = treeWidgetMachineList;

	QTreeWidgetItem *startItem = treeWidget->itemAt(treeWidget->viewport()->rect().topLeft());
	QPoint bottomPoint = treeWidget->viewport()->rect().bottomLeft();
	QTreeWidgetItem *endItem = treeWidget->itemAt(bottomPoint);
	QFontMetrics fm = treeWidget->fontMetrics();
	if ( startItem ) {
		while ( !endItem && endItem != startItem ) {
			bottomPoint.setY(bottomPoint.y() - fm.height());
			endItem = treeWidget->itemAt(bottomPoint);
		}
	}

	if ( startItem && endItem ) {
		while ( startItem->parent() )
			startItem = startItem->parent();
		while ( endItem->parent() )
			endItem = endItem->parent();
		int startIndex = treeWidget->indexOfTopLevelItem(startItem);
		if ( startIndex > 0 )
			startIndex--;
		int endIndex = treeWidget->indexOfTopLevelItem(endItem);
		if ( endIndex + 1 < treeWidget->topLevelItemCount() )
			endIndex++;
		int minWidth = 0;
		for (int i = startIndex; i <= endIndex && !m_mlRankUpdateTimer.isActive(); i++) {
			QTreeWidgetItem *item = treeWidget->topLevelItem(i);
			if ( item->isHidden() )
				continue;
			RankItemWidget *riw = (RankItemWidget *)treeWidget->itemWidget(item, QMC2_MACHINELIST_COLUMN_RANK);
			if ( riw ) {
				riw->updateSize(&fm);
				if ( minWidth == 0 )
					minWidth = riw->width();
				if ( riw->rank() > 0 )
					riw->updateRankImage();
			} else
				treeWidget->setItemWidget(item, QMC2_MACHINELIST_COLUMN_RANK, new RankItemWidget(item));
		}
		if ( treeWidget->columnWidth(QMC2_MACHINELIST_COLUMN_RANK) < minWidth )
			treeWidget->resizeColumnToContents(QMC2_MACHINELIST_COLUMN_RANK);
	}
}

void MainWindow::treeWidgetHierarchy_verticalScrollChanged(int)
{
	if ( !treeWidgetHierarchy->isColumnHidden(QMC2_MACHINELIST_COLUMN_RANK) )
		m_hlRankUpdateTimer.start(QMC2_RANK_UPDATE_DELAY);
}

void MainWindow::treeWidgetHierarchy_updateRanks()
{
  	if ( !qmc2CurrentItem || qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_MACHINE) == MachineList::trWaitingForData )
		return;

	QTreeWidget *treeWidget = treeWidgetHierarchy;

	QTreeWidgetItem *startItem = treeWidget->itemAt(treeWidget->viewport()->rect().topLeft());
	QPoint bottomPoint = treeWidget->viewport()->rect().bottomLeft();
	QTreeWidgetItem *endItem = treeWidget->itemAt(bottomPoint);
	QFontMetrics fm = treeWidget->fontMetrics();
	if ( startItem ) {
		while ( !endItem && endItem != startItem ) {
			bottomPoint.setY(bottomPoint.y() - fm.height());
			endItem = treeWidget->itemAt(bottomPoint);
		}
	}

	if ( startItem && endItem ) {
		QTreeWidgetItem *item;
		item = treeWidget->itemBelow(endItem);
		if ( item )
			endItem = item;
		item = treeWidget->itemAbove(startItem);
		if ( item )
			startItem = item;
		item = startItem;
		QFontMetrics fm = treeWidget->fontMetrics();
		int minWidth = 0;
		while ( item && item != endItem && !m_hlRankUpdateTimer.isActive() ) {
			RankItemWidget *riw = (RankItemWidget *)treeWidget->itemWidget(item, QMC2_MACHINELIST_COLUMN_RANK);
			if ( riw ) {
				riw->updateSize(&fm);
				if ( minWidth == 0 )
					minWidth = riw->width();
				if ( riw->rank() > 0 )
					riw->updateRankImage();
			} else
				treeWidget->setItemWidget(item, QMC2_MACHINELIST_COLUMN_RANK, new RankItemWidget(item));
			item = treeWidget->itemBelow(item);
		}
		if ( item == endItem && !m_hlRankUpdateTimer.isActive() ) {
			RankItemWidget *riw = (RankItemWidget *)treeWidget->itemWidget(item, QMC2_MACHINELIST_COLUMN_RANK);
			if ( riw ) {
				riw->updateSize(&fm);
				if ( minWidth == 0 )
					minWidth = riw->width();
				if ( riw->rank() > 0 )
					riw->updateRankImage();
			} else
				treeWidget->setItemWidget(item, QMC2_MACHINELIST_COLUMN_RANK, new RankItemWidget(item));
		}
		if ( treeWidget->columnWidth(QMC2_MACHINELIST_COLUMN_RANK) < minWidth )
			treeWidget->resizeColumnToContents(QMC2_MACHINELIST_COLUMN_RANK);
	}
}

void MainWindow::on_treeWidgetHierarchy_itemExpanded(QTreeWidgetItem * /*item*/)
{
	treeWidgetHierarchy_verticalScrollChanged();
}

void MainWindow::treeWidgetCategoryView_verticalScrollChanged(int)
{
	if ( !treeWidgetCategoryView->isColumnHidden(QMC2_MACHINELIST_COLUMN_RANK) )
		m_clRankUpdateTimer.start(QMC2_RANK_UPDATE_DELAY);
}

void MainWindow::treeWidgetCategoryView_updateRanks()
{
  	if ( !qmc2CurrentItem || qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_MACHINE) == MachineList::trWaitingForData )
		return;

	QTreeWidget *treeWidget = treeWidgetCategoryView;

	QTreeWidgetItem *startItem = treeWidget->itemAt(treeWidget->viewport()->rect().topLeft());
	QPoint bottomPoint = treeWidget->viewport()->rect().bottomLeft();
	QTreeWidgetItem *endItem = treeWidget->itemAt(bottomPoint);
	QFontMetrics fm = treeWidget->fontMetrics();
	if ( startItem ) {
		while ( !endItem && endItem != startItem ) {
			bottomPoint.setY(bottomPoint.y() - fm.height());
			endItem = treeWidget->itemAt(bottomPoint);
		}
	}

	if ( startItem && endItem ) {
		QTreeWidgetItem *item;
		item = treeWidget->itemBelow(endItem);
		if ( item )
			endItem = item;
		item = treeWidget->itemAbove(startItem);
		if ( item )
			startItem = item;
		item = startItem;
		QFontMetrics fm = treeWidget->fontMetrics();
		int minWidth = 0;
		while ( item && item != endItem && !m_clRankUpdateTimer.isActive() ) {
			if ( item->parent() ) {
				RankItemWidget *riw = (RankItemWidget *)treeWidget->itemWidget(item, QMC2_MACHINELIST_COLUMN_RANK);
				if ( riw ) {
					riw->updateSize(&fm);
					if ( minWidth == 0 )
						minWidth = riw->width();
					if ( riw->rank() > 0 )
						riw->updateRankImage();
				} else
					treeWidget->setItemWidget(item, QMC2_MACHINELIST_COLUMN_RANK, new RankItemWidget(item));
			}
			item = treeWidget->itemBelow(item);
		}
		if ( item == endItem && !m_clRankUpdateTimer.isActive() ) {
			if ( item->parent() ) {
				RankItemWidget *riw = (RankItemWidget *)treeWidget->itemWidget(item, QMC2_MACHINELIST_COLUMN_RANK);
				if ( riw ) {
					riw->updateSize(&fm);
					if ( minWidth == 0 )
						minWidth = riw->width();
					if ( riw->rank() > 0 )
						riw->updateRankImage();
				} else
					treeWidget->setItemWidget(item, QMC2_MACHINELIST_COLUMN_RANK, new RankItemWidget(item));
			}
		}
		if ( treeWidget->columnWidth(QMC2_MACHINELIST_COLUMN_RANK) < minWidth )
			treeWidget->resizeColumnToContents(QMC2_MACHINELIST_COLUMN_RANK);
	}
}

void MainWindow::on_treeWidgetCategoryView_itemExpanded(QTreeWidgetItem * /*item*/)
{
	treeWidgetCategoryView_verticalScrollChanged();
}

void MainWindow::treeWidgetVersionView_verticalScrollChanged(int)
{
	if ( !treeWidgetVersionView->isColumnHidden(QMC2_MACHINELIST_COLUMN_RANK) )
		m_vlRankUpdateTimer.start(QMC2_RANK_UPDATE_DELAY);
}

void MainWindow::treeWidgetVersionView_updateRanks()
{
  	if ( !qmc2CurrentItem || qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_MACHINE) == MachineList::trWaitingForData )
		return;

	QTreeWidget *treeWidget = treeWidgetVersionView;

	QTreeWidgetItem *startItem = treeWidget->itemAt(treeWidget->viewport()->rect().topLeft());
	QPoint bottomPoint = treeWidget->viewport()->rect().bottomLeft();
	QTreeWidgetItem *endItem = treeWidget->itemAt(bottomPoint);
	QFontMetrics fm = treeWidget->fontMetrics();
	if ( startItem ) {
		while ( !endItem && endItem != startItem ) {
			bottomPoint.setY(bottomPoint.y() - fm.height());
			endItem = treeWidget->itemAt(bottomPoint);
		}
	}

	if ( startItem && endItem ) {
		QTreeWidgetItem *item;
		item = treeWidget->itemBelow(endItem);
		if ( item )
			endItem = item;
		item = treeWidget->itemAbove(startItem);
		if ( item )
			startItem = item;
		item = startItem;
		QFontMetrics fm = treeWidget->fontMetrics();
		int minWidth = 0;
		while ( item && item != endItem && !m_clRankUpdateTimer.isActive() ) {
			if ( item->parent() ) {
				RankItemWidget *riw = (RankItemWidget *)treeWidget->itemWidget(item, QMC2_MACHINELIST_COLUMN_RANK);
				if ( riw ) {
					riw->updateSize(&fm);
					if ( minWidth == 0 )
						minWidth = riw->width();
					if ( riw->rank() > 0 )
						riw->updateRankImage();
				} else
					treeWidget->setItemWidget(item, QMC2_MACHINELIST_COLUMN_RANK, new RankItemWidget(item));
			}
			item = treeWidget->itemBelow(item);
		}
		if ( item == endItem && !m_clRankUpdateTimer.isActive() ) {
			if ( item->parent() ) {
				RankItemWidget *riw = (RankItemWidget *)treeWidget->itemWidget(item, QMC2_MACHINELIST_COLUMN_RANK);
				if ( riw ) {
					riw->updateSize(&fm);
					if ( minWidth == 0 )
						minWidth = riw->width();
					if ( riw->rank() > 0 )
						riw->updateRankImage();
				} else
					treeWidget->setItemWidget(item, QMC2_MACHINELIST_COLUMN_RANK, new RankItemWidget(item));
			}
		}
		if ( treeWidget->columnWidth(QMC2_MACHINELIST_COLUMN_RANK) < minWidth )
			treeWidget->resizeColumnToContents(QMC2_MACHINELIST_COLUMN_RANK);
	}
}

void MainWindow::on_treeWidgetVersionView_itemExpanded(QTreeWidgetItem *item)
{
	treeWidgetVersionView_verticalScrollChanged();
}

void MainWindow::on_actionIncreaseRank_triggered(bool)
{
	if ( !qmc2CurrentItem || qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_MACHINE) == MachineList::trWaitingForData )
		return;
	RankItemWidget *riw = getCurrentRankItemWidget();
	if ( riw )
		riw->increaseRank();
}

void MainWindow::on_actionDecreaseRank_triggered(bool)
{
	if ( !qmc2CurrentItem || qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_MACHINE) == MachineList::trWaitingForData )
		return;
	RankItemWidget *riw = getCurrentRankItemWidget();
	if ( riw )
		riw->decreaseRank();
}

void MainWindow::updateUserData()
{
	if ( tabWidgetMachineList->indexOf(tabMachineList) == tabWidgetMachineList->currentIndex() ) {
		switch ( stackedWidgetView->currentIndex() ) {
			case QMC2_VIEWHIERARCHY_INDEX:
				treeWidgetHierarchy_verticalScrollChanged();
				break;
			case QMC2_VIEWCATEGORY_INDEX:
				treeWidgetCategoryView_verticalScrollChanged();
				break;
			case QMC2_VIEWVERSION_INDEX:
				treeWidgetVersionView_verticalScrollChanged();
				break;
			case QMC2_VIEWMACHINELIST_INDEX:
			default:
				treeWidgetMachineList_verticalScrollChanged();
				break;
		}
	}
	foreach (MachineListViewer *v, machineListViewers)
		QTimer::singleShot(0, v, SLOT(treeViewUpdateRanks()));
}

void MainWindow::on_actionRankImageGradient_triggered(bool checked)
{
	if ( checked ) {
		actionRankImageGradient->setChecked(true);
		actionRankImageFlat->setChecked(false);
		actionRankImagePlain->setChecked(false);
		RankItemWidget::useFlatRankImage = false;
		RankItemWidget::useColorRankImage = false;
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/FlatRankImage", RankItemWidget::useFlatRankImage);
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/ColorRankImage", RankItemWidget::useColorRankImage);
		menuRank->setIcon(RankItemWidget::gradientRankIcon());
		actionRankImageGradient->setIcon(RankItemWidget::gradientRankIcon());
		actionRankImageFlat->setIcon(RankItemWidget::flatRankIcon());
		actionRankImagePlain->setIcon(RankItemWidget::colorRankIcon());
		QTimer::singleShot(0, this, SLOT(updateUserData()));
	}
}

void MainWindow::on_actionRankImageFlat_triggered(bool checked)
{
	if ( checked ) {
		actionRankImageGradient->setChecked(false);
		actionRankImageFlat->setChecked(true);
		actionRankImagePlain->setChecked(false);
		RankItemWidget::useFlatRankImage = true;
		RankItemWidget::useColorRankImage = false;
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/FlatRankImage", RankItemWidget::useFlatRankImage);
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/ColorRankImage", RankItemWidget::useColorRankImage);
		menuRank->setIcon(RankItemWidget::flatRankIcon());
		actionRankImageGradient->setIcon(RankItemWidget::gradientRankIcon());
		actionRankImageFlat->setIcon(RankItemWidget::flatRankIcon());
		actionRankImagePlain->setIcon(RankItemWidget::colorRankIcon());
		QTimer::singleShot(0, this, SLOT(updateUserData()));
	}
}

void MainWindow::on_actionRankImagePlain_triggered(bool checked)
{
	if ( checked ) {
		actionRankImageGradient->setChecked(false);
		actionRankImageFlat->setChecked(false);
		actionRankImagePlain->setChecked(true);
		RankItemWidget::useFlatRankImage = true;
		RankItemWidget::useColorRankImage = true;
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/FlatRankImage", RankItemWidget::useFlatRankImage);
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/ColorRankImage", RankItemWidget::useColorRankImage);
		menuRank->setIcon(RankItemWidget::colorRankIcon());
		actionRankImageGradient->setIcon(RankItemWidget::gradientRankIcon());
		actionRankImageFlat->setIcon(RankItemWidget::flatRankIcon());
		actionRankImagePlain->setIcon(RankItemWidget::colorRankIcon());
		QTimer::singleShot(0, this, SLOT(updateUserData()));
	}
}

void MainWindow::on_actionRankImageColor_triggered(bool)
{
	QColor color = QColorDialog::getColor(RankItemWidget::rankImageColor, this, tr("Choose overlay color"), QColorDialog::DontUseNativeDialog | QColorDialog::ShowAlphaChannel);
	if ( color.isValid() ) {
		RankItemWidget::rankImageColor = color;
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/RankImageColor", RankItemWidget::rankImageColor.name());
		menuRank->setIcon(RankItemWidget::currentRankIcon());
		actionRankImageGradient->setIcon(RankItemWidget::gradientRankIcon());
		actionRankImageFlat->setIcon(RankItemWidget::flatRankIcon());
		actionRankImagePlain->setIcon(RankItemWidget::colorRankIcon());
		QTimer::singleShot(0, this, SLOT(updateUserData()));
	}
}

void MainWindow::on_actionSetRank0_triggered(bool)
{
	RankItemWidget *riw = getCurrentRankItemWidget();
	if ( riw )
		riw->setRankComplete(0);
}

void MainWindow::on_actionSetRank1_triggered(bool)
{
	RankItemWidget *riw = getCurrentRankItemWidget();
	if ( riw )
		riw->setRankComplete(1);
}

void MainWindow::on_actionSetRank2_triggered(bool)
{
	RankItemWidget *riw = getCurrentRankItemWidget();
	if ( riw )
		riw->setRankComplete(2);
}

void MainWindow::on_actionSetRank3_triggered(bool)
{
	RankItemWidget *riw = getCurrentRankItemWidget();
	if ( riw )
		riw->setRankComplete(3);
}

void MainWindow::on_actionSetRank4_triggered(bool)
{
	RankItemWidget *riw = getCurrentRankItemWidget();
	if ( riw )
		riw->setRankComplete(4);
}

void MainWindow::on_actionSetRank5_triggered(bool)
{
	RankItemWidget *riw = getCurrentRankItemWidget();
	if ( riw )
		riw->setRankComplete(5);
}

void MainWindow::on_actionTaggedIncreaseRank_triggered(bool)
{
	QList<RankItemWidget *> *riwList = getTaggedRankItemWidgets();
	foreach (RankItemWidget *riw, *riwList)
		riw->increaseRank();
}

void MainWindow::on_actionTaggedDecreaseRank_triggered(bool)
{
	QList<RankItemWidget *> *riwList = getTaggedRankItemWidgets();
	foreach (RankItemWidget *riw, *riwList)
		riw->decreaseRank();
}

void MainWindow::on_actionTaggedSetRank0_triggered(bool)
{
	QList<RankItemWidget *> *riwList = getTaggedRankItemWidgets();
	foreach (RankItemWidget *riw, *riwList)
		riw->setRankComplete(0);
}

void MainWindow::on_actionTaggedSetRank1_triggered(bool)
{
	QList<RankItemWidget *> *riwList = getTaggedRankItemWidgets();
	foreach (RankItemWidget *riw, *riwList)
		riw->setRankComplete(1);
}

void MainWindow::on_actionTaggedSetRank2_triggered(bool)
{
	QList<RankItemWidget *> *riwList = getTaggedRankItemWidgets();
	foreach (RankItemWidget *riw, *riwList)
		riw->setRankComplete(2);
}

void MainWindow::on_actionTaggedSetRank3_triggered(bool)
{
	QList<RankItemWidget *> *riwList = getTaggedRankItemWidgets();
	foreach (RankItemWidget *riw, *riwList)
		riw->setRankComplete(3);
}

void MainWindow::on_actionTaggedSetRank4_triggered(bool)
{
	QList<RankItemWidget *> *riwList = getTaggedRankItemWidgets();
	foreach (RankItemWidget *riw, *riwList)
		riw->setRankComplete(4);
}

void MainWindow::on_actionTaggedSetRank5_triggered(bool)
{
	QList<RankItemWidget *> *riwList = getTaggedRankItemWidgets();
	foreach (RankItemWidget *riw, *riwList)
		riw->setRankComplete(5);
}

void MainWindow::on_actionLockRanks_triggered(bool)
{
	RankItemWidget::ranksLocked = !RankItemWidget::ranksLocked;
	if ( RankItemWidget::ranksLocked ) {
		actionLockRanks->setIcon(QIcon(QString::fromUtf8(":/data/img/unlock.png")));
		actionLockRanks->setText(tr("Unlock ranks"));
		actionLockRanks->setToolTip(tr("Unlock ranks"));
		actionLockRanks->setStatusTip(tr("Unlock ranks"));
	} else {
		actionLockRanks->setIcon(QIcon(QString::fromUtf8(":/data/img/lock.png")));
		actionLockRanks->setText(tr("Lock ranks"));
		actionLockRanks->setToolTip(tr("Lock ranks"));
		actionLockRanks->setStatusTip(tr("Lock ranks"));
	}
	menuRank_enableActions(!RankItemWidget::ranksLocked);
}

void MainWindow::menuRank_enableActions(bool enable)
{
	menuSetRank->menuAction()->setEnabled(enable);
	menuTaggedSetRank->menuAction()->setEnabled(enable);
	actionIncreaseRank->setEnabled(enable);
	actionTaggedIncreaseRank->setEnabled(enable);
	actionDecreaseRank->setEnabled(enable);
	actionTaggedDecreaseRank->setEnabled(enable);
}

void MainWindow::menuRank_aboutToShow()
{
	bool taggedItemsVisible = (qmc2MachineList->numTaggedSets > 0 && (menuGame->isVisible() || menuRank->isTearOffMenuVisible()));
	menuTaggedSetRank->menuAction()->setVisible(taggedItemsVisible);
	actionTaggedIncreaseRank->setVisible(taggedItemsVisible);
	actionTaggedDecreaseRank->setVisible(taggedItemsVisible);
}

RankItemWidget *MainWindow::getCurrentRankItemWidget()
{
	QTreeWidget *treeWidget;
	QTreeWidgetItem *item;
	switch ( stackedWidgetView->currentIndex() ) {
		case QMC2_VIEWHIERARCHY_INDEX:
			treeWidget = treeWidgetHierarchy;
			item = qmc2HierarchyItemHash.value(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_NAME));
			break;
		case QMC2_VIEWCATEGORY_INDEX:
			treeWidget = treeWidgetCategoryView;
			item = qmc2CategoryItemHash.value(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_NAME));
			break;
		case QMC2_VIEWVERSION_INDEX:
			treeWidget = treeWidgetVersionView;
			item = qmc2VersionItemHash.value(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_NAME));
			break;
		case QMC2_VIEWMACHINELIST_INDEX:
		default:
			treeWidget = treeWidgetMachineList;
			item = qmc2CurrentItem;
			break;
	}

	return (RankItemWidget *)treeWidget->itemWidget(item, QMC2_MACHINELIST_COLUMN_RANK);
}

QList<RankItemWidget *> *MainWindow::getTaggedRankItemWidgets()
{
	static QList<RankItemWidget *> taggedRiwList;

	taggedRiwList.clear();

	QHashIterator<QString, QTreeWidgetItem *> it(qmc2MachineListItemHash);
	QTreeWidgetItem *item;
	while ( it.hasNext() ) {
		it.next();
		item = qmc2MachineListItemHash.value(it.key());
		if ( item ) {
			if ( item->checkState(QMC2_MACHINELIST_COLUMN_TAG) == Qt::Checked )
				taggedRiwList << (RankItemWidget *)treeWidgetMachineList->itemWidget(item, QMC2_MACHINELIST_COLUMN_RANK);
		}
	}

	return &taggedRiwList;
}

void MainWindow::checkRomPath()
{
	if ( !qmc2Config->value(QMC2_EMULATOR_PREFIX + "CheckRomPath", true).toBool() )
		return;

	QString myRomPath;

	if ( qmc2Config->contains(QMC2_EMULATOR_PREFIX + "Configuration/Global/rompath") ) {
		QStringList romPaths;
		foreach (QString romPath, qmc2Config->value(QMC2_EMULATOR_PREFIX + "Configuration/Global/rompath").toString().split(";", QString::SkipEmptyParts)) {
			QDir romDir(romPath);
			if ( romDir.isRelative() ) {
				if ( qmc2Config->contains(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory") ) {
					QString workPath = qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory", QString()).toString();
					if ( !workPath.isEmpty() )
						romPaths << QDir::cleanPath(workPath + "/" + romPath);
					else
						romPaths << romPath;
				}
			} else
				romPaths << romPath;
		}
		myRomPath = romPaths.join(";");
	} else if ( qmc2Config->contains(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory") )
		myRomPath = QDir::cleanPath(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory", QString()).toString() + "/roms");
	else
		myRomPath = QDir::cleanPath(QDir::currentPath() + "/roms");

	bool allRomPathsOk = true;
	QStringList pathsToCheck;

	foreach (QString romPath, myRomPath.split(";", QString::SkipEmptyParts)) {
		QDir romDir(romPath);
		if ( !romDir.exists() ) {
			log(QMC2_LOG_FRONTEND, tr("WARNING: ROM path '%1' doesn't exist").arg(romPath));
			allRomPathsOk = false;
			pathsToCheck << romPath;
		} else if ( !romDir.isReadable() ) {
			log(QMC2_LOG_FRONTEND, tr("WARNING: ROM path '%1' isn't accessible").arg(romPath));;
			allRomPathsOk = false;
			pathsToCheck << romPath;
		}
	}

	if ( !allRomPathsOk ) {
		QString message;
		if ( pathsToCheck.count() == 1 ) {
			message = tr("The ROM path '%1' doesn't exist or isn't accessible.\n\n"
				     "Please check the 'rompath' option in the global emulator configuration to fix this, otherwise ROMs will probably not be available to the emulator!").arg(pathsToCheck[0]);
		} else {
			message = tr("The ROM paths\n\n%1\n\ndon't exist or aren't accessible.\n\n"
				     "Please check the 'rompath' option in the global emulator configuration to fix this, otherwise ROMs will probably not be available to the emulator!").arg(pathsToCheck.join("\n"));
		}
		switch ( QMessageBox::warning(this, tr("Check ROM path"), message, QMessageBox::Ok | QMessageBox::Ignore, QMessageBox::Ok) ) {
			case QMessageBox::Ignore:
				qmc2Config->setValue(QMC2_EMULATOR_PREFIX + "CheckRomPath", false);
				break;
			case QMessageBox::Ok:
			default:
				break;
		}
	}
}

void MainWindow::negateSearchTriggered(bool negate)
{
	negatedMatch = negate;
	IconLineEdit *ileSearch = ((IconLineEdit *)comboBoxSearch->lineEdit());
	IconLineEdit *ileToolbarSearch = ((IconLineEdit *)comboBoxToolbarSearch->lineEdit());
	if ( negatedMatch ) {
		ileSearch->button()->setIcon(QIcon(QString::fromUtf8(":/data/img/find_negate.png")));
		ileToolbarSearch->button()->setIcon(QIcon(QString::fromUtf8(":/data/img/find_negate.png")));
	} else {
		ileSearch->button()->setIcon(QIcon(QString::fromUtf8(":/data/img/find.png")));
		ileToolbarSearch->button()->setIcon(QIcon(QString::fromUtf8(":/data/img/find.png")));
	}
	actionNegateSearch->setChecked(negatedMatch);
	searchTimer.start(QMC2_SEARCH_DELAY);
}

void MainWindow::searchIncludeBiosSetsTriggered(bool includeBioses)
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/SearchIncludeBiosSets", includeBioses);
	searchTimer.start(QMC2_SEARCH_DELAY);
}

void MainWindow::searchIncludeDeviceSetsTriggered(bool includeDevices)
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/SearchIncludeDeviceSets", includeDevices);
	searchTimer.start(QMC2_SEARCH_DELAY);
}

void MainWindow::updateTabWidgets()
{
	tabWidgetMachineDetail->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/MachineDetailTab", 0).toInt());
	tabWidgetLogsAndEmulators->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/LogsAndEmulatorsTab", 0).toInt());
	on_tabWidgetLogsAndEmulators_currentChanged(tabWidgetLogsAndEmulators->currentIndex());
	tabWidgetMachineList->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/MachineListTab", 0).toInt());
	tabWidgetSoftwareDetail->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/SoftwareDetailTab", 0).toInt());
}

#if QT_VERSION < 0x050000
void myQtMessageHandler(QtMsgType type, const char *msg)
#else
void myQtMessageHandler(QtMsgType type, const QMessageLogContext &, const QString &msg)
#endif
{
	if ( qmc2SuppressQtMessages )
		return;
	QString msgString;
	switch ( type ) {
		case QtDebugMsg:
			msgString = "QtDebugMsg: " + QString(msg);
			break;
		case QtWarningMsg:
			msgString = "QtWarningMsg: " + QString(msg);
			break;
		case QtCriticalMsg:
			msgString = "QtCriticalMsg: " + QString(msg);
			break;
		case QtFatalMsg:
			msgString = "QtFatalMsg: " + QString(msg);
			break;
		default:
			return;
	}
	if ( qmc2GuiReady )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, msgString);
	else {
		printf("%s: %s\n", QTime::currentTime().toString("hh:mm:ss.zzz").toUtf8().constData(), msgString.toUtf8().constData());
		fflush(stdout);
	}
}

void MainWindow::processGlobalEmuConfig()
{
	log(QMC2_LOG_FRONTEND, tr("processing global emulator configuration"));
	int left, top, right, bottom;
	gridLayout->getContentsMargins(&left, &top, &right, &bottom);
	QVBoxLayout *layout = new QVBoxLayout;
	qmc2GlobalEmulatorOptions = new EmulatorOptions(QMC2_EMULATOR_PREFIX + "Configuration/Global", qmc2Options->tabGlobalConfiguration);
	qmc2GlobalEmulatorOptions->load();
	layout->addWidget(qmc2GlobalEmulatorOptions);
	layout->setContentsMargins(left, top, right, bottom);
	QHBoxLayout *buttonLayout = new QHBoxLayout();
	pushButtonGlobalEmulatorOptionsExportToFile = new QPushButton(tr("Export to..."), qmc2Options);
	pushButtonGlobalEmulatorOptionsExportToFile->setToolTip(tr("Export global MAME configuration"));
	pushButtonGlobalEmulatorOptionsExportToFile->setToolTip(tr("Export global MAME configuration"));
	pushButtonGlobalEmulatorOptionsImportFromFile = new QPushButton(tr("Import from..."), qmc2Options);
	pushButtonGlobalEmulatorOptionsImportFromFile->setToolTip(tr("Import global MAME configuration"));
	pushButtonGlobalEmulatorOptionsImportFromFile->setStatusTip(tr("Import global MAME configuration"));
	buttonLayout->addWidget(pushButtonGlobalEmulatorOptionsExportToFile);
	buttonLayout->addWidget(pushButtonGlobalEmulatorOptionsImportFromFile);
	layout->addLayout(buttonLayout);
	qmc2Options->checkBoxEnableOutputNotifiers = new QCheckBox(qmc2Options);
	qmc2Options->checkBoxEnableOutputNotifiers->setText(tr("Enable processing of MAME output notifiers (this forces '-output console')"));
	qmc2Options->checkBoxEnableOutputNotifiers->setToolTip(tr("Enable processing of MAME output notifiers (this forces '-output console')"));
	qmc2Options->checkBoxEnableOutputNotifiers->setChecked(qmc2Config->value(QMC2_EMULATOR_PREFIX + "EnableOutputNotifiers", true).toBool());
	qmc2Options->setEnableOutputNotifiers(qmc2Options->checkBoxEnableOutputNotifiers->isChecked());
	layout->addWidget(qmc2Options->checkBoxEnableOutputNotifiers);
	qmc2Options->tabGlobalConfiguration->setLayout(layout);
	selectMenuGlobalEmulatorOptionsExportToFile = new QMenu(pushButtonGlobalEmulatorOptionsExportToFile);
	connect(selectMenuGlobalEmulatorOptionsExportToFile->addAction(QIcon(QString::fromUtf8(":/data/img/work.png")), tr("<inipath>/mame.ini")), SIGNAL(triggered()), this, SLOT(pushButtonGlobalEmulatorOptionsExportToFile_clicked()));
	connect(selectMenuGlobalEmulatorOptionsExportToFile->addAction(QIcon(QString::fromUtf8(":/data/img/fileopen.png")), tr("Select file...")), SIGNAL(triggered()), this, SLOT(pushButtonGlobalEmulatorOptionsSelectExportFile_clicked()));
	pushButtonGlobalEmulatorOptionsExportToFile->setMenu(selectMenuGlobalEmulatorOptionsExportToFile);
	selectMenuGlobalEmulatorOptionsImportFromFile = new QMenu(pushButtonGlobalEmulatorOptionsImportFromFile);
	connect(selectMenuGlobalEmulatorOptionsImportFromFile->addAction(QIcon(QString::fromUtf8(":/data/img/work.png")), tr("<inipath>/mame.ini")), SIGNAL(triggered()), this, SLOT(pushButtonGlobalEmulatorOptionsImportFromFile_clicked()));
	connect(selectMenuGlobalEmulatorOptionsImportFromFile->addAction(QIcon(QString::fromUtf8(":/data/img/fileopen.png")), tr("Select file...")), SIGNAL(triggered()), this, SLOT(pushButtonGlobalEmulatorOptionsSelectImportFile_clicked()));
	pushButtonGlobalEmulatorOptionsImportFromFile->setMenu(selectMenuGlobalEmulatorOptionsImportFromFile);
}

void MainWindow::prepareShortcuts()
{
	// shortcuts
	qmc2ShortcutHash["Ctrl+1"].second = actionCheckROMs;
	qmc2ShortcutHash["Ctrl+2"].second = actionCheckSamples;
	qmc2ShortcutHash["Ctrl+3"].second = actionCheckImagesAndIcons;
	qmc2ShortcutHash["Ctrl+B"].second = actionAbout;
	qmc2ShortcutHash["Ctrl+D"].second = actionAnalyseCurrentROM;
	qmc2ShortcutHash["Ctrl+Shift+D"].second = actionAnalyseROMTagged;
	qmc2ShortcutHash["Ctrl+E"].second = actionExportROMStatus;
	qmc2ShortcutHash["Ctrl+J"].second = actionToFavorites;
	qmc2ShortcutHash["Ctrl+Shift+J"].second = actionToFavoritesTagged;
	qmc2ShortcutHash["Ctrl+H"].second = actionDocumentation;
	qmc2ShortcutHash["Ctrl+I"].second = actionClearImageCache;
	qmc2ShortcutHash["Ctrl+Shift+A"].second = actionArcadeSetup;
	qmc2ShortcutHash["Ctrl+M"].second = actionClearProjectMESSCache;
	qmc2ShortcutHash["Ctrl+N"].second = actionClearIconCache;
#if defined(QMC2_OS_MAC)
	qmc2ShortcutHash["Ctrl+,"].second = actionOptions;
#else
	qmc2ShortcutHash["Ctrl+O"].second = actionOptions;
#endif
	qmc2ShortcutHash["Ctrl+P"].second = actionPlay;
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
	qmc2ShortcutHash["Ctrl+Shift+P"].second = actionPlayEmbedded;
#endif
#if !defined(QMC2_OS_MAC)
	qmc2ShortcutHash["Ctrl+Q"].second = actionAboutQt;
#endif
	qmc2ShortcutHash["Ctrl+R"].second = actionReload;
	qmc2ShortcutHash["Ctrl+S"].second = actionCheckCurrentROM;
	qmc2ShortcutHash["Ctrl+Shift+S"].second = actionCheckROMStateTagged;
	qmc2ShortcutHash["Ctrl+T"].second = actionRecreateTemplateMap;
	qmc2ShortcutHash["Ctrl+Shift+C"].second = actionCheckTemplateMap;
#if defined(QMC2_OS_MAC)
	qmc2ShortcutHash["Ctrl+Q"].second = actionExitStop;
#else
	qmc2ShortcutHash["Ctrl+X"].second = actionExitStop;
#endif
#if defined(QMC2_YOUTUBE_ENABLED)
	qmc2ShortcutHash["Ctrl+Y"].second = actionClearYouTubeCache;
#endif
	qmc2ShortcutHash["Ctrl+Z"].second = actionSystemROMAlyzer;
	qmc2ShortcutHash["Ctrl+W"].second = actionSoftwareROMAlyzer;
	qmc2ShortcutHash["Ctrl+Alt+C"].second = romStateFilter->actionShowCorrect;
	qmc2ShortcutHash["Ctrl+Alt+M"].second = romStateFilter->actionShowMostlyCorrect;
	qmc2ShortcutHash["Ctrl+Alt+I"].second = romStateFilter->actionShowIncorrect;
	qmc2ShortcutHash["Ctrl+Alt+N"].second = romStateFilter->actionShowNotFound;
	qmc2ShortcutHash["Ctrl+Alt+U"].second = romStateFilter->actionShowUnknown;
	qmc2ShortcutHash["Ctrl+Shift+T"].second = actionSetTag;
	qmc2ShortcutHash["Ctrl+Shift+U"].second = actionUnsetTag;
	qmc2ShortcutHash["Ctrl+Shift+G"].second = actionToggleTag;
	qmc2ShortcutHash["Shift+Down"].second = actionToggleTagCursorDown;
	qmc2ShortcutHash["Shift+Up"].second = actionToggleTagCursorUp;
	qmc2ShortcutHash["Ctrl+Shift+L"].second = actionTagAll;
	qmc2ShortcutHash["Ctrl+Shift+N"].second = actionUntagAll;
	qmc2ShortcutHash["Ctrl+Shift+I"].second = actionInvertTags;
	qmc2ShortcutHash["Ctrl+Shift+X"].second = actionTagVisible;
	qmc2ShortcutHash["Ctrl+Shift+Y"].second = actionUntagVisible;
	qmc2ShortcutHash["Ctrl+Shift+Z"].second = actionInvertVisibleTags;
	qmc2ShortcutHash["F2"].second = actionRebuildROM;
	qmc2ShortcutHash["Ctrl+Shift+F2"].second = actionRebuildROMTagged;
	qmc2ShortcutHash["F5"].second = actionViewFullDetail;
	qmc2ShortcutHash["F6"].second = actionViewParentClones;
	qmc2ShortcutHash["F7"].second = actionViewByCategory;
	qmc2ShortcutHash["F8"].second = actionViewByVersion;
	qmc2ShortcutHash["Shift+F9"].second = actionRunRomTool;
	qmc2ShortcutHash["Ctrl+Shift+F9"].second = actionRunRomToolTagged;
	qmc2ShortcutHash["F10"].second = 0; // for "check software-states"
	qmc2ShortcutHash["F11"].second = actionFullscreenToggle;
	qmc2ShortcutHash["F12"].second = actionLaunchArcade;
	qmc2ShortcutHash["Shift+F5"].second = 0; // for "software-list view flat"
	qmc2ShortcutHash["Shift+F6"].second = 0; // for "software-list view tree"
#if QMC2_USE_PHONON_API
	qmc2ShortcutHash["Ctrl+Alt+Left"].second = actionAudioPreviousTrack;
	qmc2ShortcutHash["Ctrl+Alt+Right"].second = actionAudioNextTrack;
	qmc2ShortcutHash["Ctrl+Alt+B"].second = actionAudioFastBackward;
	qmc2ShortcutHash["Ctrl+Alt+F"].second = actionAudioFastForward;
	qmc2ShortcutHash["Ctrl+Alt+S"].second = actionAudioStopTrack;
	qmc2ShortcutHash["Ctrl+Alt+#"].second = actionAudioPauseTrack;
	qmc2ShortcutHash["Ctrl+Alt+P"].second = actionAudioPlayTrack;
	qmc2ShortcutHash["Ctrl+Alt+PgUp"].second = actionAudioRaiseVolume;
	qmc2ShortcutHash["Ctrl+Alt+PgDown"].second = actionAudioLowerVolume;
#endif
	qmc2ShortcutHash["Alt+PgUp"].second = actionIncreaseRank;
	qmc2ShortcutHash["Alt+PgDown"].second = actionDecreaseRank;

	// special keys
	qmc2QtKeyHash.insert("+", QKeySequence("+", QKeySequence::PortableText));
	qmc2QtKeyHash.insert("-", QKeySequence("-", QKeySequence::PortableText));
	qmc2QtKeyHash.insert("Down", QKeySequence("Down", QKeySequence::PortableText));
	qmc2QtKeyHash.insert("End", QKeySequence("End", QKeySequence::PortableText));
	qmc2QtKeyHash.insert("Esc", QKeySequence("Esc", QKeySequence::PortableText));
	qmc2QtKeyHash.insert("Left", QKeySequence("Left", QKeySequence::PortableText));
	qmc2QtKeyHash.insert("Home", QKeySequence("Home", QKeySequence::PortableText));
	qmc2QtKeyHash.insert("PgDown", QKeySequence("PgDown", QKeySequence::PortableText));
	qmc2QtKeyHash.insert("PgUp", QKeySequence("PgUp", QKeySequence::PortableText));
	qmc2QtKeyHash.insert("Return", QKeySequence("Return", QKeySequence::PortableText));
	qmc2QtKeyHash.insert("Enter", QKeySequence("Enter", QKeySequence::PortableText));
	qmc2QtKeyHash.insert("Right", QKeySequence("Right", QKeySequence::PortableText));
	qmc2QtKeyHash.insert("Tab", QKeySequence("Tab", QKeySequence::PortableText));
	qmc2QtKeyHash.insert("Up", QKeySequence("Up", QKeySequence::PortableText));
#if defined(QMC2_OS_MAC)
	qmc2QtKeyHash.insert("Ctrl+O", QKeySequence("Ctrl+O", QKeySequence::PortableText));
#endif

	QTimer::singleShot(0, qmc2Options, SLOT(setupShortcutActions()));
}

#if defined(QMC2_MINGW)
#undef main
#endif

int main(int argc, char **argv)
{
#if defined(QMC2_OS_MAC)
	// this hack ensures that we're using the bundled plugins rather than the ones from a Qt SDK installation
	char exec_path[4096];
	uint32_t exec_path_size = sizeof(exec_path);
	if ( _NSGetExecutablePath(exec_path, &exec_path_size) == 0 ) {
		QFileInfo fi(exec_path);
		QCoreApplication::addLibraryPath(fi.absoluteDir().absolutePath() + "/../PlugIns");
	}
#endif

	qsrand(QDateTime::currentDateTime().toTime_t());

	// install message handler
#if QT_VERSION < 0x050000
	qInstallMsgHandler(myQtMessageHandler);
#else
	qInstallMessageHandler(myQtMessageHandler);
#endif

	// prepare application and resources
	QApplication qmc2App(argc, argv);

	if ( QMC2_CLI_OPT_HELP ) {
#if defined(QMC2_OS_WIN)
		winAllocConsole();
#endif
		printf("Usage: %s [-config_path <config_path>] [-cc] [-tc] [-r] [-h|-?|-help] [qt_arguments]\n\n"
		       "-config_path    Use specified configuration path (default: %s)\n"
		       "-cc             Clear all emulator caches before starting up\n"
		       "-tc             Check the emulator configuration template and exit\n"
		       "-r		Reconfigure (runs the setup wizard before starting)\n"
		       "-h|-?|-help     Show this help text and exit\n", argv[0], QString(QMC2_DOT_PATH).toUtf8().constData());
#if defined(QMC2_OS_WIN)
		winFreeConsole();
#endif
		return 1;
	}

	Q_INIT_RESOURCE(qmc2);

	qmc2App.setWindowIcon(QIcon(QString::fromUtf8(":/data/img/mame.png")));
	QPixmap splashPixmap(QString::fromUtf8(":/data/img/qmc2_mame_splash.png"));

	qmc2TemplateCheck = QMC2_CLI_OPT_TEMPLATE_CHECK;

	// check configuration
	int checkReturn = QDialog::Accepted;
	qmc2Welcome = new Welcome(0);
	if ( !qmc2Welcome->checkOkay ) {
		checkReturn = qmc2Welcome->exec();
		qmc2TemplateCheck = false;
	}
	if ( checkReturn != QDialog::Accepted )
		return 1;

	bool showSplashScreen = qmc2Welcome->startupConfig->value(QMC2_FRONTEND_PREFIX + "GUI/ShowSplashScreen", true).toBool() && !qmc2TemplateCheck;
	QFont splashFont(qApp->font());
	if ( !qmc2Welcome->startupConfig->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString().isEmpty() )
		splashFont.fromString(qmc2Welcome->startupConfig->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	delete qmc2Welcome;

	// setup splash screen
	if ( showSplashScreen ) {
		qmc2SplashScreen = new QSplashScreen(splashPixmap);
		splashFont.setBold(true);
		qmc2SplashScreen->setFont(splashFont);
		qmc2SplashScreen->setAttribute(Qt::WA_ShowWithoutActivating);
		qmc2SplashScreen->setMask(splashPixmap.mask());
		qmc2SplashScreen->setWindowOpacity(0.8);
		qmc2SplashScreen->showMessage(QObject::tr("Setting up the GUI, please wait..."), Qt::AlignHCenter | Qt::AlignBottom, Qt::white);
		qmc2SplashScreen->show();
	}

	// setup event filter
	qmc2MainEventFilter = new MainEventFilter(0);
	qmc2App.installEventFilter(qmc2MainEventFilter);

	// create mandatory objects
	qmc2Options = new Options(0);
	qmc2Config = qmc2Options->config;
	qmc2ProcessManager = new ProcessManager(0);
	qmc2MainWindow = new MainWindow(0);
	qmc2ComponentSetup = new ComponentSetup(qmc2MainWindow);

	// finalize initial setup
	qmc2Options->apply();
	qmc2GuiReady = true;
	qmc2MainWindow->prepareShortcuts();

#if QT_VERSION < 0x050000
	// this effectively enables support for unicode characters in C strings
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#endif

	// create & show the greeting string
	QString greeting(QObject::tr("M.A.M.E. Catalog / Launcher II v") + QString(XSTR(QMC2_VERSION)) +
#if QMC2_SVN_REV > 0
			   ", " + QObject::tr("SVN r%1").arg(QMC2_SVN_REV) +
#endif
			   " (Qt " + qVersion() + ", " + QMC2_EMU_NAME_VARIANT + ", " + QMC2_OS_NAME + " " + QMC2_MACHINE_ARCHITECTURE + ")");
	qmc2StartupTimer.start();
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, greeting);

#if QMC2_USE_PHONON_API
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QObject::tr("Phonon features enabled - using Phonon v%1").arg(Phonon::phononVersion()));
#endif

#if QMC2_JOYSTICK == 1
#if SDL_MAJOR_VERSION == 1
	const SDL_version *sdlVersion = SDL_Linked_Version();
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QObject::tr("SDL joystick support enabled - using SDL v%1.%2.%3").arg(sdlVersion->major).arg(sdlVersion->minor).arg(sdlVersion->patch));
#elif SDL_MAJOR_VERSION == 2
	SDL_version sdlVersion;
	SDL_GetVersion(&sdlVersion);
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QObject::tr("SDL joystick support enabled - using SDL v%1.%2.%3").arg(sdlVersion.major).arg(sdlVersion.minor).arg(sdlVersion.patch));
#endif
#endif

	// process global emulator configuration and create import/export popup menus
	qmc2MainWindow->processGlobalEmuConfig();

	int retCode = 0;
	if ( qmc2TemplateCheck ) {
#if defined(QMC2_OS_WIN)
		winAllocConsole();
#endif
		qmc2GlobalEmulatorOptions->checkTemplateMap();
#if defined(QMC2_OS_WIN)
		winFreeConsole();
#endif
		qmc2Options->applied = true;
		qmc2MainWindow->close();
	} else {
		QTimer::singleShot(0, qmc2Options, SLOT(checkShortcuts()));
#if QMC2_JOYSTICK == 1
		QTimer::singleShot(0, qmc2Options, SLOT(checkJoystickMappings()));
#endif
		qmc2Options->on_pushButtonApply_clicked();
		// finally run the application
		retCode = qmc2App.exec();
	}

	if ( qmc2SplashScreen )
		qmc2SplashScreen->deleteLater();

	// wait for all application threads to finish
	QThreadPool::globalInstance()->waitForDone();

	return retCode;
}
