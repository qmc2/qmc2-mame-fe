#include <QTranslator>
#include <QFileInfo>
#include <QFileDialog>
#include <QFontDialog>
#include <QMessageBox>
#include <QTimer>
#include <QMap>
#include <QMultiMap>
#include <QHash>
#include <QHashIterator>
#include <QStyleFactory>
#include <QHeaderView>
#include <QBitArray>
#include <QAction>
#include <QPair>
#include <QNetworkProxy>
#include <QScrollBar>
#include <QInputDialog>
#include <QSplashScreen>
#include <QNetworkAccessManager>
#include <QWebSettings>
#include <QCache>

#include "options.h"
#include "emuopt.h"
#include "preview.h"
#include "flyer.h"
#include "cabinet.h"
#include "controller.h"
#include "marquee.h"
#include "title.h"
#include "pcb.h"
#include "qmc2main.h"
#include "machinelist.h"
#include "imagechecker.h"
#include "macros.h"
#include "unzip.h"
#include "keyseqscan.h"
#include "romalyzer.h"
#include "romstatusexport.h"
#include "docbrowser.h"
#include "componentsetup.h"
#include "toolbarcustomizer.h"
#include "paletteeditor.h"
#include "iconlineedit.h"
#include "imagewidget.h"
#include "cookiejar.h"
#include "cookiemanager.h"
#include "networkaccessmanager.h"
#include "additionalartworksetup.h"
#include "imageformatsetup.h"
#if QMC2_JOYSTICK == 1
#include "joystick.h"
#include "joyfuncscan.h"
#endif
#include "deviceconfigurator.h"
#include "softwarelist.h"
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
#include "embedder.h"
#include "embedderopt.h"
#endif
#if QMC2_USE_PHONON_API
#include "audioeffects.h"
#endif
#if defined(QMC2_YOUTUBE_ENABLED)
#include "youtubevideoplayer.h"
#endif
#include "htmleditor/htmleditor.h"
#include "customidsetup.h"
#include "samplechecker.h"
#include "rankitemwidget.h"
#include "componentsetup.h"
#include "cryptedbytearray.h"
#include "individualfallbacksettings.h"
#include "catverinioptimizer.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern Options *qmc2Options;
extern bool qmc2GuiReady;
extern bool qmc2CleaningUp;
extern bool qmc2StartingUp;
extern bool qmc2EarlyStartup;
extern bool qmc2ReloadActive;
extern bool qmc2ScaledPreview;
extern bool qmc2ScaledFlyer;
extern bool qmc2ScaledCabinet;
extern bool qmc2ScaledController;
extern bool qmc2ScaledMarquee;
extern bool qmc2ScaledTitle;
extern bool qmc2ScaledPCB;
extern bool qmc2ScaledSoftwareSnapshot;
extern bool qmc2SmoothScaling;
extern bool qmc2RetryLoadingImages;
extern bool qmc2ParentImageFallback;
extern bool qmc2VerifyActive;
extern bool qmc2ImageCheckActive;
extern bool qmc2SampleCheckActive;
extern bool qmc2UsePreviewFile;
extern bool qmc2UseFlyerFile;
extern bool qmc2UseIconFile;
extern bool qmc2UseCabinetFile;
extern bool qmc2UseControllerFile;
extern bool qmc2UseMarqueeFile;
extern bool qmc2UseTitleFile;
extern bool qmc2UsePCBFile;
extern bool qmc2UseSoftwareSnapFile;
extern bool qmc2AutomaticReload;
extern bool qmc2SuppressQtMessages;
extern bool qmc2ShowMachineName;
extern bool qmc2ShowMachineNameOnlyWhenRequired;
extern bool qmc2StatesTogglesEnabled;
extern int qmc2MachineListResponsiveness;
extern int qmc2UpdateDelay;
extern EmulatorOptions *qmc2GlobalEmulatorOptions;
extern EmulatorOptions *qmc2EmulatorOptions;
extern Preview *qmc2Preview;
extern Flyer *qmc2Flyer;
extern Cabinet *qmc2Cabinet;
extern Controller *qmc2Controller;
extern Marquee *qmc2Marquee;
extern Title *qmc2Title;
extern PCB *qmc2PCB;
extern SoftwareSnap *qmc2SoftwareSnap;
extern MachineList *qmc2MachineList;
extern ImageChecker *qmc2ImageChecker;
extern ROMAlyzer *qmc2SystemROMAlyzer;
extern ROMAlyzer *qmc2SoftwareROMAlyzer;
extern ROMStatusExporter *qmc2ROMStatusExporter;
extern DocBrowser *qmc2DocBrowser;
extern int qmc2SortCriteria;
extern Qt::SortOrder qmc2SortOrder;
extern Settings *qmc2Config;
extern QBitArray qmc2Filter;
extern QMap<QString, unzFile> qmc2IconFileMap;
extern QMap<QString, SevenZipFile *> qmc2IconFileMap7z;
#if defined(QMC2_LIBARCHIVE_ENABLED)
extern QMap<QString, ArchiveFile *> qmc2IconArchiveMap;
#endif
extern QHash<QString, QPair<QString, QAction *> > qmc2ShortcutHash;
extern QHash<QString, QString> qmc2CustomShortcutHash;
extern MainEventFilter *qmc2MainEventFilter;
extern QHash<QString, QKeySequence> qmc2QtKeyHash;
extern ComponentSetup *qmc2ComponentSetup;
extern ToolBarCustomizer *qmc2ToolBarCustomizer;
extern PaletteEditor *qmc2PaletteEditor;
#if QMC2_JOYSTICK == 1
extern QHash<QString, QString> qmc2JoystickFunctionHash;
extern bool qmc2JoystickIsCalibrating;
#endif
extern DeviceConfigurator *qmc2DeviceConfigurator;
extern MiniWebBrowser *qmc2ProjectMESS;
extern SoftwareList *qmc2SoftwareList;
#if QMC2_USE_PHONON_API
extern AudioEffectDialog *qmc2AudioEffectDialog;
#endif
#if defined(QMC2_YOUTUBE_ENABLED)
extern YouTubeVideoPlayer *qmc2YouTubeWidget;
#endif
extern QAbstractItemView::ScrollHint qmc2CursorPositioningMode;
extern QFont *qmc2StartupDefaultFont;
extern int qmc2SoftwareSnapPosition;
extern int qmc2DefaultLaunchMode;
extern HtmlEditor *qmc2SystemNotesEditor;
extern HtmlEditor *qmc2SoftwareNotesEditor;
extern QSplashScreen *qmc2SplashScreen;
extern QCache<QString, ImagePixmap> qmc2ImagePixmapCache;
extern QList<QTreeWidgetItem *> qmc2ExpandedMachineListItems;
extern bool qmc2SortingActive;
extern SampleChecker *qmc2SampleChecker;
extern NetworkAccessManager *qmc2NetworkAccessManager;
extern QPalette qmc2CustomPalette;
extern QMap<QString, QPalette> qmc2StandardPalettes;
extern bool qmc2CategoryInfoUsed;
extern bool qmc2VersionInfoUsed;
extern bool qmc2LoadingSoftwareInfoDB;
extern bool qmc2LoadingEmuInfoDB;
extern bool qmc2LoadingMachineInfoDB;

QBrush Options::greenBrush(QColor(0, 255, 0));
QBrush Options::yellowBrush(QColor(255, 255, 0));
QBrush Options::blueBrush(QColor(0, 0, 255));
QBrush Options::redBrush(QColor(255, 0, 0));
QBrush Options::greyBrush(QColor(128, 128, 128));
QBrush Options::lightgreyBrush(QColor(200, 200, 200));

Options::Options(QWidget *parent) :
#if defined(QMC2_OS_WIN)
	QDialog(parent, Qt::Dialog)
#else
	QDialog(parent, Qt::Dialog | Qt::SubWindow)
#endif
{
	qmc2Filter.resize(QMC2_ROMSTATE_COUNT);

	QCoreApplication::setOrganizationName(QMC2_ORGANIZATION_NAME);
	QCoreApplication::setOrganizationDomain(QMC2_ORGANIZATION_DOMAIN);
	QCoreApplication::setApplicationName(QMC2_VARIANT_NAME);

#if !defined(QMC2_OS_WIN)
	QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, QMC2_SYSCONF_PATH);
#endif
	QString userScopePath = configPath();
	QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, userScopePath);
	QDir userScopeDir(userScopePath);
	if ( !userScopeDir.exists() )
		userScopeDir.mkdir(userScopePath);

	config = new Settings(QSettings::IniFormat, QSettings::UserScope, "qmc2");

	QWebSettings::enablePersistentStorage(userScopePath);

	setupUi(this);

#if !defined(QMC2_LIBARCHIVE_ENABLED)
	comboBoxPreviewFileType->removeItem(QMC2_IMG_FILETYPE_ARCHIVE);
	comboBoxFlyerFileType->removeItem(QMC2_IMG_FILETYPE_ARCHIVE);
	comboBoxCabinetFileType->removeItem(QMC2_IMG_FILETYPE_ARCHIVE);
	comboBoxControllerFileType->removeItem(QMC2_IMG_FILETYPE_ARCHIVE);
	comboBoxMarqueeFileType->removeItem(QMC2_IMG_FILETYPE_ARCHIVE);
	comboBoxTitleFileType->removeItem(QMC2_IMG_FILETYPE_ARCHIVE);
	comboBoxPCBFileType->removeItem(QMC2_IMG_FILETYPE_ARCHIVE);
	comboBoxIconFileType->removeItem(QMC2_IMG_FILETYPE_ARCHIVE);
	comboBoxSoftwareSnapFileType->removeItem(QMC2_IMG_FILETYPE_ARCHIVE);
#endif

	cancelClicked = false;

	setStandardWorkDir(QDir::currentPath());

#if !defined(QMC2_OS_MAC)
	checkBoxUnifiedTitleAndToolBarOnMac->setVisible(false);
#endif

#if !defined(QMC2_OS_UNIX) && !defined(QMC2_OS_WIN)
	labelDefaultLaunchMode->setVisible(false);
	comboBoxDefaultLaunchMode->setVisible(false);
#endif

#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
	checkBoxMinimizeOnEmuLaunch->setToolTip(tr("Minimize when launching (non-embedded) emulators?"));
#endif

	comboBoxSortCriteria->insertItem(QMC2_SORTCRITERIA_CATEGORY, tr("Category"));
	comboBoxSortCriteria->insertItem(QMC2_SORTCRITERIA_VERSION, tr("Version"));

	// shortcuts
	qmc2ShortcutHash["Ctrl+1"] = QPair<QString, QAction *>(tr("Check all ROM states"), 0);
	qmc2ShortcutHash["Ctrl+2"] = QPair<QString, QAction *>(tr("Check all sample sets"), 0);
	qmc2ShortcutHash["Ctrl+3"] = QPair<QString, QAction *>(tr("Check images and icons"), 0);
	qmc2ShortcutHash["Ctrl+B"] = QPair<QString, QAction *>(tr("About QMC2"), 0);
	qmc2ShortcutHash["Ctrl+D"] = QPair<QString, QAction *>(tr("Analyze current machine"), 0);
	qmc2ShortcutHash["Ctrl+Shift+D"] = QPair<QString, QAction *>(tr("Analyze tagged sets"), 0);
	qmc2ShortcutHash["Ctrl+E"] = QPair<QString, QAction *>(tr("Export ROM Status"), 0);
	qmc2ShortcutHash["Ctrl+J"] = QPair<QString, QAction *>(tr("Copy machine to favorites"), 0);
	qmc2ShortcutHash["Ctrl+Shift+J"] = QPair<QString, QAction *>(tr("Copy tagged sets to favorites"), 0);
	qmc2ShortcutHash["Ctrl+H"] = QPair<QString, QAction *>(tr("Online documentation"), 0);
	qmc2ShortcutHash["Ctrl+I"] = QPair<QString, QAction *>(tr("Clear image cache"), 0);
	qmc2ShortcutHash["Ctrl+Shift+A"] = QPair<QString, QAction *>(tr("Setup arcade mode"), 0);
	qmc2ShortcutHash["Ctrl+M"] = QPair<QString, QAction *>(tr("Clear ProjectMESS cache"), 0);
	qmc2ShortcutHash["Ctrl+N"] = QPair<QString, QAction *>(tr("Clear icon cache"), 0);
#if defined(QMC2_OS_MAC)
	qmc2ShortcutHash["Ctrl+,"] = QPair<QString, QAction *>(tr("Open options dialog"), 0);
#else
	qmc2ShortcutHash["Ctrl+O"] = QPair<QString, QAction *>(tr("Open options dialog"), 0);
#endif
	qmc2ShortcutHash["Ctrl+P"] = QPair<QString, QAction *>(tr("Play (independent)"), 0);
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
	qmc2ShortcutHash["Ctrl+Shift+P"] = QPair<QString, QAction *>(tr("Play (embedded)"), 0);
#endif
#if !defined(QMC2_OS_MAC)
	qmc2ShortcutHash["Ctrl+Q"] = QPair<QString, QAction *>(tr("About Qt"), 0);
#endif
	qmc2ShortcutHash["Ctrl+R"] = QPair<QString, QAction *>(tr("Reload machine list"), 0);
	qmc2ShortcutHash["Ctrl+S"] = QPair<QString, QAction *>(tr("Check machine's ROM state"), 0);
	qmc2ShortcutHash["Ctrl+Shift+S"] = QPair<QString, QAction *>(tr("Check states of tagged ROMs"), 0);
	qmc2ShortcutHash["Ctrl+T"] = QPair<QString, QAction *>(tr("Recreate template map"), 0);
	qmc2ShortcutHash["Ctrl+Shift+C"] = QPair<QString, QAction *>(tr("Check template map"), 0);
#if defined(QMC2_OS_MAC)
	qmc2ShortcutHash["Ctrl+Q"] = QPair<QString, QAction *>(tr("Stop processing / exit QMC2"), 0);
#else
	qmc2ShortcutHash["Ctrl+X"] = QPair<QString, QAction *>(tr("Stop processing / exit QMC2"), 0);
#endif
#if defined(QMC2_YOUTUBE_ENABLED)
	qmc2ShortcutHash["Ctrl+Y"] = QPair<QString, QAction *>(tr("Clear YouTube cache"), 0);
#endif
	qmc2ShortcutHash["Ctrl+Z"] = QPair<QString, QAction *>(tr("Open ROMAlyzer (system mode)"), 0);
	qmc2ShortcutHash["Ctrl+W"] = QPair<QString, QAction *>(tr("Open ROMAlyzer (software mode)"), 0);
	qmc2ShortcutHash["Ctrl+Alt+C"] = QPair<QString, QAction *>(tr("Toggle ROM state C"), 0);
	qmc2ShortcutHash["Ctrl+Alt+M"] = QPair<QString, QAction *>(tr("Toggle ROM state M"), 0);
	qmc2ShortcutHash["Ctrl+Alt+I"] = QPair<QString, QAction *>(tr("Toggle ROM state I"), 0);
	qmc2ShortcutHash["Ctrl+Alt+N"] = QPair<QString, QAction *>(tr("Toggle ROM state N"), 0);
	qmc2ShortcutHash["Ctrl+Alt+U"] = QPair<QString, QAction *>(tr("Toggle ROM state U"), 0);
	qmc2ShortcutHash["Ctrl+Shift+T"] = QPair<QString, QAction *>(tr("Tag current set"), 0);
	qmc2ShortcutHash["Ctrl+Shift+U"] = QPair<QString, QAction *>(tr("Untag current set"), 0);
	qmc2ShortcutHash["Ctrl+Shift+G"] = QPair<QString, QAction *>(tr("Toggle tag mark"), 0);
	qmc2ShortcutHash["Shift+Down"] = QPair<QString, QAction *>(tr("Toggle tag / cursor down"), 0);
	qmc2ShortcutHash["Shift+Up"] = QPair<QString, QAction *>(tr("Toggle tag / cursor up"), 0);
	qmc2ShortcutHash["Ctrl+Shift+L"] = QPair<QString, QAction *>(tr("Tag all sets"), 0);
	qmc2ShortcutHash["Ctrl+Shift+N"] = QPair<QString, QAction *>(tr("Untag all sets"), 0);
	qmc2ShortcutHash["Ctrl+Shift+I"] = QPair<QString, QAction *>(tr("Invert all tags"), 0);
	qmc2ShortcutHash["Ctrl+Shift+X"] = QPair<QString, QAction *>(tr("Tag visible sets"), 0);
	qmc2ShortcutHash["Ctrl+Shift+Y"] = QPair<QString, QAction *>(tr("Untag visible sets"), 0);
	qmc2ShortcutHash["Ctrl+Shift+Z"] = QPair<QString, QAction *>(tr("Invert visible tags"), 0);
	qmc2ShortcutHash["F2"] = QPair<QString, QAction *>(tr("Rebuild current machine"), 0);
	qmc2ShortcutHash["Ctrl+Shift+F2"] = QPair<QString, QAction *>(tr("Rebuild tagged machines"), 0);
	qmc2ShortcutHash["F5"] = QPair<QString, QAction *>(tr("Full detail view"), 0);
	qmc2ShortcutHash["F6"] = QPair<QString, QAction *>(tr("Hierarchical view"), 0);
	qmc2ShortcutHash["F7"] = QPair<QString, QAction *>(tr("Category view"), 0);
	qmc2ShortcutHash["F8"] = QPair<QString, QAction *>(tr("Version view"), 0);
	qmc2ShortcutHash["Shift+F9"] = QPair<QString, QAction *>(tr("Run external ROM tool"), 0);
	qmc2ShortcutHash["Ctrl+Shift+F9"] = QPair<QString, QAction *>(tr("Run ROM tool for tagged sets"), 0);
	qmc2ShortcutHash["F10"] = QPair<QString, QAction *>(tr("Check software-states"), 0);
	qmc2ShortcutHash["F11"] = QPair<QString, QAction *>(tr("Toggle full screen"), 0);
	qmc2ShortcutHash["F12"] = QPair<QString, QAction *>(tr("Launch arcade mode"), 0);
	qmc2ShortcutHash["Shift+F5"] = QPair<QString, QAction *>(tr("Software-list view-mode flat"), 0);
	qmc2ShortcutHash["Shift+F6"] = QPair<QString, QAction *>(tr("Software-list view-mode tree"), 0);
#if QMC2_USE_PHONON_API || QMC2_MULTIMEDIA_ENABLED
	qmc2ShortcutHash["Ctrl+Alt+Left"] = QPair<QString, QAction *>(tr("Previous track (audio player)"), 0);
	qmc2ShortcutHash["Ctrl+Alt+Right"] = QPair<QString, QAction *>(tr("Next track (audio player)"), 0);
	qmc2ShortcutHash["Ctrl+Alt+B"] = QPair<QString, QAction *>(tr("Fast backward (audio player)"), 0);
	qmc2ShortcutHash["Ctrl+Alt+F"] = QPair<QString, QAction *>(tr("Fast forward (audio player)"), 0);
	qmc2ShortcutHash["Ctrl+Alt+S"] = QPair<QString, QAction *>(tr("Stop track (audio player)"), 0);
	qmc2ShortcutHash["Ctrl+Alt+#"] = QPair<QString, QAction *>(tr("Pause track (audio player)"), 0);
	qmc2ShortcutHash["Ctrl+Alt+P"] = QPair<QString, QAction *>(tr("Play track (audio player)"), 0);
	qmc2ShortcutHash["Ctrl+Alt+PgUp"] = QPair<QString, QAction *>(tr("Raise volume (audio player)"), 0);
	qmc2ShortcutHash["Ctrl+Alt+PgDown"] = QPair<QString, QAction *>(tr("Lower volume (audio player)"), 0);
#endif
	qmc2ShortcutHash["Alt+PgUp"] = QPair<QString, QAction *>(tr("Increase rank"), 0);
	qmc2ShortcutHash["Alt+PgDown"] = QPair<QString, QAction *>(tr("Decrease rank"), 0);

	// special keys
	qmc2ShortcutHash["+"] = QPair<QString, QAction *>(tr("Plus (+)"), 0);
	qmc2ShortcutHash["-"] = QPair<QString, QAction *>(tr("Minus (-)"), 0);
	qmc2ShortcutHash["Down"] = QPair<QString, QAction *>(tr("Cursor down"), 0);
	qmc2ShortcutHash["End"] = QPair<QString, QAction *>(tr("End"), 0);
	qmc2ShortcutHash["Enter"] = QPair<QString, QAction *>(tr("Enter key"), 0);
	qmc2ShortcutHash["Esc"] = QPair<QString, QAction *>(tr("Escape"), 0);
	qmc2ShortcutHash["Left"] = QPair<QString, QAction *>(tr("Cursor left"), 0);
	qmc2ShortcutHash["Home"] = QPair<QString, QAction *>(tr("Home"), 0);
	qmc2ShortcutHash["PgDown"] = QPair<QString, QAction *>(tr("Page down"), 0);
	qmc2ShortcutHash["PgUp"] = QPair<QString, QAction *>(tr("Page up"), 0);
	qmc2ShortcutHash["Return"] = QPair<QString, QAction *>(tr("Return key"), 0);
	qmc2ShortcutHash["Right"] = QPair<QString, QAction *>(tr("Cursor right"), 0);
	qmc2ShortcutHash["Tab"] = QPair<QString, QAction *>(tr("Tabulator"), 0);
	qmc2ShortcutHash["Up"] = QPair<QString, QAction *>(tr("Cursor up"), 0);
#if defined(QMC2_OS_MAC)
	qmc2ShortcutHash["Ctrl+O"] = QPair<QString, QAction *>(tr("Activate item"), 0);
#endif

	if ( !config->isWritable() )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: configuration is not writeable, please check access permissions for ") + config->fileName());

	// font reset actions
	QMenu *fontButtonMenu = new QMenu(0);
	QString s = tr("Reset to default font");
	QAction *action = fontButtonMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	toolButtonBrowseFont->setMenu(fontButtonMenu);
	connect(action, SIGNAL(triggered()), lineEditFont, SLOT(clear()));

	QMenu *logFontButtonMenu = new QMenu(0);
	s = tr("Reset to default font");
	action = logFontButtonMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	toolButtonBrowseLogFont->setMenu(logFontButtonMenu);
	connect(action, SIGNAL(triggered()), lineEditLogFont, SLOT(clear()));

	// style-sheet reset action
	QMenu *styleSheetButtonMenu = new QMenu(0);
	s = tr("No style sheet");
	action = styleSheetButtonMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	toolButtonBrowseStyleSheet->setMenu(styleSheetButtonMenu);
	connect(action, SIGNAL(triggered()), lineEditStyleSheet, SLOT(clear()));

	lineEditStyleSheet->setPlaceholderText(tr("No style sheet"));
	lineEditFont->setPlaceholderText(tr("Default"));
	lineEditLogFont->setPlaceholderText(tr("Default"));

#if QMC2_JOYSTICK != 1
	tabWidgetFrontendSettings->removeTab(tabWidgetFrontendSettings->indexOf(tabFrontendJoystick));
#else
	joystick = 0;
	joystickCalibrationWidget = 0;
	joystickTestWidget = 0;
	scrollArea = new QScrollArea(frameCalibrationAndTest);
	scrollArea->hide();
	scrollArea->setWidgetResizable(true);
#endif

	lineEditPreviewDirectory->setToolTip(lineEditPreviewDirectory->toolTip() + " - " + tr("use semicolon (;) to separate multiple folders"));
	lineEditPreviewFile->setToolTip(lineEditPreviewFile->toolTip() + " - " + tr("use semicolon (;) to separate multiple files"));
	lineEditFlyerDirectory->setToolTip(lineEditFlyerDirectory->toolTip() + " - " + tr("use semicolon (;) to separate multiple folders"));
	lineEditFlyerFile->setToolTip(lineEditFlyerFile->toolTip() + " - " + tr("use semicolon (;) to separate multiple files"));
	lineEditIconDirectory->setToolTip(lineEditIconDirectory->toolTip() + " - " + tr("use semicolon (;) to separate multiple folders"));
	lineEditIconFile->setToolTip(lineEditIconFile->toolTip() + " - " + tr("use semicolon (;) to separate multiple files"));
	lineEditCabinetDirectory->setToolTip(lineEditCabinetDirectory->toolTip() + " - " + tr("use semicolon (;) to separate multiple folders"));
	lineEditCabinetFile->setToolTip(lineEditCabinetFile->toolTip() + " - " + tr("use semicolon (;) to separate multiple files"));
	lineEditControllerDirectory->setToolTip(lineEditControllerDirectory->toolTip() + " - " + tr("use semicolon (;) to separate multiple folders"));
	lineEditControllerFile->setToolTip(lineEditControllerFile->toolTip() + " - " + tr("use semicolon (;) to separate multiple files"));
	lineEditMarqueeDirectory->setToolTip(lineEditMarqueeDirectory->toolTip() + " - " + tr("use semicolon (;) to separate multiple folders"));
	lineEditMarqueeFile->setToolTip(lineEditMarqueeFile->toolTip() + " - " + tr("use semicolon (;) to separate multiple files"));
	lineEditTitleDirectory->setToolTip(lineEditTitleDirectory->toolTip() + " - " + tr("use semicolon (;) to separate multiple folders"));
	lineEditTitleFile->setToolTip(lineEditTitleFile->toolTip() + " - " + tr("use semicolon (;) to separate multiple files"));
	lineEditPCBDirectory->setToolTip(lineEditPCBDirectory->toolTip() + " - " + tr("use semicolon (;) to separate multiple folders"));
	lineEditPCBFile->setToolTip(lineEditPCBFile->toolTip() + " - " + tr("use semicolon (;) to separate multiple files"));
	lineEditSoftwareSnapDirectory->setToolTip(lineEditSoftwareSnapDirectory->toolTip() + " - " + tr("use semicolon (;) to separate multiple folders"));
	lineEditSoftwareSnapFile->setToolTip(lineEditSoftwareSnapFile->toolTip() + " - " + tr("use semicolon (;) to separate multiple files"));
	lineEditVideoSnapFolder->setToolTip(lineEditVideoSnapFolder->toolTip() + " - " + tr("use semicolon (;) to separate multiple folders"));

	checkPlaceholderStatus();
	restoreCurrentConfig();
}

Options::~Options()
{
	config->setValue(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/FrontendTab", tabWidgetFrontendSettings->currentIndex());
	config->setValue(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/MAMETab", tabWidgetGlobalMAMESetup->currentIndex());
	config->setValue(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/OptionsTab", tabWidgetOptions->currentIndex());
	config->setValue(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/ShortcutsHeaderState", treeWidgetShortcuts->header()->saveState());
#if QMC2_JOYSTICK == 1
	config->setValue(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/JoyMapHeaderState", treeWidgetJoystickMappings->header()->saveState());
#endif
	config->setValue(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/RegisteredEmulatorsHeaderState", tableWidgetRegisteredEmulators->horizontalHeader()->saveState());

#if QMC2_JOYSTICK == 1
	if ( joystick )
		delete joystick;
#endif

	delete config;
	close();
}

void Options::apply()
{
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
	if ( qmc2MainWindow->tabWidgetMachineList->currentIndex() != QMC2_EMBED_INDEX || !qmc2MainWindow->toolButtonEmbedderMaximizeToggle->isChecked() ) {
		qmc2MainWindow->statusBar()->setVisible(config->value(QMC2_FRONTEND_PREFIX + "GUI/Statusbar", true).toBool());
		qmc2MainWindow->toolbar->setVisible(config->value(QMC2_FRONTEND_PREFIX + "GUI/Toolbar", true).toBool());
	}
#else
	qmc2MainWindow->statusBar()->setVisible(config->value(QMC2_FRONTEND_PREFIX + "GUI/Statusbar", true).toBool());
	qmc2MainWindow->toolbar->setVisible(config->value(QMC2_FRONTEND_PREFIX + "GUI/Toolbar", true).toBool());
#endif
	QFont f;
	if ( qmc2StartupDefaultFont )
		f = *qmc2StartupDefaultFont;
	if ( !config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString().isEmpty() )
		f.fromString(config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	qApp->setFont(f);
	QFontMetrics fm(f);
	QSize iconSize(fm.height() - 2, fm.height() - 2);
	QSize iconSizeMiddle = iconSize + QSize(2, 2);
	QSize iconSizeLarge = iconSize + QSize(4, 4);
	foreach (QWidget *widget, QApplication::allWidgets()) {
		widget->setFont(f);
		if ( widget->objectName() == "MiniWebBrowser" )
			QTimer::singleShot(0, (MiniWebBrowser *)widget, SLOT(adjustIconSizes()));
	}
	if ( qmc2SplashScreen ) {
		QFont splashFont = f;
		splashFont.setBold(true);
		qmc2SplashScreen->setFont(splashFont);
	}
	QFont logFont = f;
	if ( !config->value(QMC2_FRONTEND_PREFIX + "GUI/LogFont").toString().isEmpty() )
		logFont.fromString(config->value(QMC2_FRONTEND_PREFIX + "GUI/LogFont").toString());
	qmc2MainWindow->textBrowserFrontendLog->setFont(logFont);
	qmc2MainWindow->textBrowserEmulatorLog->setFont(logFont);
	lineEditLogFont->setFont(logFont);
	((IconLineEdit *)qmc2MainWindow->comboBoxSearch->lineEdit())->setIconSize(iconSizeMiddle);
	((IconLineEdit *)qmc2MainWindow->comboBoxToolbarSearch->lineEdit())->setIconSize(iconSizeMiddle);
	qmc2MainWindow->treeWidgetMachineList->setIconSize(iconSizeMiddle);
	qmc2MainWindow->treeWidgetHierarchy->setIconSize(iconSizeMiddle);
	qmc2MainWindow->treeWidgetForeignIDs->setIconSize(iconSizeMiddle);
	qmc2MainWindow->treeWidgetEmulators->setIconSize(iconSizeMiddle);
	qmc2MainWindow->floatToggleButtonSoftwareDetail->setIconSize(iconSizeMiddle);
	pushButtonApply->setIconSize(iconSize);
	pushButtonRestore->setIconSize(iconSize);
	pushButtonDefault->setIconSize(iconSize);
	pushButtonOk->setIconSize(iconSize);
	pushButtonCancel->setIconSize(iconSize);
	toolButtonBrowseStyleSheet->setIconSize(iconSize);
	toolButtonBrowseFont->setIconSize(iconSize);
	toolButtonBrowseLogFont->setIconSize(iconSize);
	toolButtonBrowseTemporaryFile->setIconSize(iconSize);
	toolButtonBrowseFrontendLogFile->setIconSize(iconSize);
	toolButtonBrowseFavoritesFile->setIconSize(iconSize);
	toolButtonBrowseHistoryFile->setIconSize(iconSize);
	toolButtonBrowseMachineListCacheFile->setIconSize(iconSize);
	toolButtonBrowseROMStateCacheFile->setIconSize(iconSize);
	toolButtonBrowseSlotInfoCacheFile->setIconSize(iconSize);
	toolButtonBrowseDataDirectory->setIconSize(iconSize);
	toolButtonBrowseDatInfoDatabase->setIconSize(iconSize);
	toolButtonBrowseMameHistoryDat->setIconSize(iconSize);
	toolButtonBrowseMessSysinfoDat->setIconSize(iconSize);
	toolButtonBrowseMameInfoDat->setIconSize(iconSize);
	toolButtonBrowseMessInfoDat->setIconSize(iconSize);
	toolButtonBrowseSoftwareInfoDB->setIconSize(iconSize);
	toolButtonImportGameInfo->setIconSize(iconSize);
	toolButtonImportMachineInfo->setIconSize(iconSize);
	toolButtonImportMameInfo->setIconSize(iconSize);
	toolButtonImportMessInfo->setIconSize(iconSize);
	toolButtonImportSoftwareInfo->setIconSize(iconSize);
	toolButtonOptimizeCatverIni->setIconSize(iconSize);
	qmc2MainWindow->treeWidgetCategoryView->setIconSize(iconSizeMiddle);
	checkBoxUseCategoryIni->setIconSize(iconSize);
	toolButtonBrowseCategoryIniFile->setIconSize(iconSize);
	qmc2MainWindow->treeWidgetVersionView->setIconSize(iconSizeMiddle);
	checkBoxUseCatverIni->setIconSize(iconSize);
	toolButtonBrowseCatverIniFile->setIconSize(iconSize);
	toolButtonBrowsePreviewDirectory->setIconSize(iconSize);
	toolButtonBrowsePreviewFile->setIconSize(iconSize);
	comboBoxPreviewFileType->setIconSize(iconSize);
	toolButtonBrowseFlyerDirectory->setIconSize(iconSize);
	toolButtonBrowseFlyerFile->setIconSize(iconSize);
	comboBoxFlyerFileType->setIconSize(iconSize);
	toolButtonBrowseIconDirectory->setIconSize(iconSize);
	toolButtonBrowseIconFile->setIconSize(iconSize);
	comboBoxIconFileType->setIconSize(iconSize);
	toolButtonBrowseCabinetDirectory->setIconSize(iconSize);
	toolButtonBrowseCabinetFile->setIconSize(iconSize);
	comboBoxCabinetFileType->setIconSize(iconSize);
	toolButtonBrowseControllerDirectory->setIconSize(iconSize);
	toolButtonBrowseControllerFile->setIconSize(iconSize);
	comboBoxControllerFileType->setIconSize(iconSize);
	toolButtonBrowseMarqueeDirectory->setIconSize(iconSize);
	toolButtonBrowseMarqueeFile->setIconSize(iconSize);
	comboBoxMarqueeFileType->setIconSize(iconSize);
	toolButtonBrowseTitleDirectory->setIconSize(iconSize);
	toolButtonBrowseTitleFile->setIconSize(iconSize);
	comboBoxTitleFileType->setIconSize(iconSize);
	toolButtonBrowsePCBDirectory->setIconSize(iconSize);
	toolButtonBrowsePCBFile->setIconSize(iconSize);
	comboBoxPCBFileType->setIconSize(iconSize);
	toolButtonBrowseSoftwareSnapDirectory->setIconSize(iconSize);
	toolButtonBrowseSoftwareSnapFile->setIconSize(iconSize);
	comboBoxSoftwareSnapFileType->setIconSize(iconSize);
	toolButtonBrowseSoftwareNotesFolder->setIconSize(iconSize);
	toolButtonBrowseSoftwareNotesTemplate->setIconSize(iconSize);
	toolButtonBrowseVideoSnapFolder->setIconSize(iconSize);
	toolButtonBrowseSystemNotesFolder->setIconSize(iconSize);
	toolButtonBrowseSystemNotesTemplate->setIconSize(iconSize);
	toolButtonShowC->setIconSize(iconSize);
	toolButtonShowM->setIconSize(iconSize);
	toolButtonShowI->setIconSize(iconSize);
	toolButtonShowN->setIconSize(iconSize);
	toolButtonShowU->setIconSize(iconSize);
	checkBoxRomStateFilter->setIconSize(iconSize);
	comboBoxSortOrder->setIconSize(iconSize);
	checkBoxShowROMStatusIcons->setIconSize(iconSize);
	checkBoxShowDeviceSets->setIconSize(iconSize);
	checkBoxShowBiosSets->setIconSize(iconSize);
	toolButtonBrowseExecutableFile->setIconSize(iconSize);
	toolButtonBrowseWorkingDirectory->setIconSize(iconSize);
	toolButtonBrowseEmulatorLogFile->setIconSize(iconSize);
	toolButtonBrowseOptionsTemplateFile->setIconSize(iconSize);
	toolButtonBrowseXmlCacheDatabase->setIconSize(iconSize);
	toolButtonBrowseUserDataDatabase->setIconSize(iconSize);
	toolButtonBrowseMachineListDatabase->setIconSize(iconSize);
	toolButtonCleanupUserDataDatabase->setIconSize(iconSize);
	toolButtonClearUserDataDatabase->setIconSize(iconSize);
	toolButtonBrowseCookieDatabase->setIconSize(iconSize);
	toolButtonBrowseZipTool->setIconSize(iconSize);
	toolButtonBrowseSevenZipTool->setIconSize(iconSize);
	toolButtonBrowseRomTool->setIconSize(iconSize);
	toolButtonBrowseRomToolWorkingDirectory->setIconSize(iconSize);
	pushButtonRedefineKeySequence->setIconSize(iconSize);
	pushButtonResetShortcut->setIconSize(iconSize);
	toolButtonAddEmulator->setIconSize(iconSize);
	toolButtonSaveEmulator->setIconSize(iconSize);
	toolButtonRemoveEmulator->setIconSize(iconSize);
	toolButtonBrowseAdditionalEmulatorExecutable->setIconSize(iconSize);
	toolButtonBrowseAdditionalEmulatorWorkingDirectory->setIconSize(iconSize);
	QPixmap exitPixmap = QPixmap(QString::fromUtf8(":/data/img/exit.png")).scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	QPixmap reloadPixmap = QPixmap(QString::fromUtf8(":/data/img/reload.png")).scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	labelLanguagePic->setPixmap(exitPixmap);
	labelLegend1Pic->setPixmap(exitPixmap);
	labelLegend3Pic->setPixmap(reloadPixmap);
	labelExecutableFilePic->setPixmap(reloadPixmap);
	labelLegend4Pic->setPixmap(reloadPixmap);
	labelLegend5Pic->setPixmap(reloadPixmap);
	tableWidgetRegisteredEmulators->resizeRowsToContents();
	for (int i = 0; i < tableWidgetRegisteredEmulators->rowCount(); i++) {
		QToolButton *tb = (QToolButton *)tableWidgetRegisteredEmulators->cellWidget(i, QMC2_ADDTLEMUS_COLUMN_ICON);
		if ( tb )
			tb->setIconSize(iconSizeMiddle);
	}
	// global web-browser fonts
	QWebSettings::globalSettings()->setFontFamily(QWebSettings::StandardFont, qApp->font().family());
	QWebSettings::globalSettings()->setFontFamily(QWebSettings::SerifFont, qApp->font().family());
	QWebSettings::globalSettings()->setFontFamily(QWebSettings::SansSerifFont, qApp->font().family());
	QWebSettings::globalSettings()->setFontFamily(QWebSettings::FantasyFont, qApp->font().family());
	QWebSettings::globalSettings()->setFontFamily(QWebSettings::CursiveFont, qApp->font().family());
	QWebSettings::globalSettings()->setFontFamily(QWebSettings::FixedFont, logFont.family());
	QWebSettings::globalSettings()->setFontSize(QWebSettings::DefaultFontSize, qApp->font().pointSize() + 1);
	QWebSettings::globalSettings()->setFontSize(QWebSettings::DefaultFixedFontSize, logFont.pointSize() + 1);
#if QMC2_JOYSTICK == 1
	pushButtonRescanJoysticks->setIconSize(iconSize);
	pushButtonRemapJoystickFunction->setIconSize(iconSize);
	pushButtonRemoveJoystickMapping->setIconSize(iconSize);
#endif
	if ( qmc2SystemROMAlyzer ) {
		qmc2SystemROMAlyzer->textBrowserLog->setFont(logFont);
		QTimer::singleShot(0, qmc2SystemROMAlyzer, SLOT(adjustIconSizes()));
		if ( qmc2SystemROMAlyzer->checkSumScannerLog() )
			qmc2SystemROMAlyzer->checkSumScannerLog()->plainTextEditLog->setFont(logFont);
		if ( qmc2SystemROMAlyzer->collectionRebuilder() ) {
			qmc2SystemROMAlyzer->collectionRebuilder()->plainTextEditLog->setFont(logFont);
			QTimer::singleShot(0, qmc2SystemROMAlyzer->collectionRebuilder(), SLOT(adjustIconSizes()));
		}
	}
	if ( qmc2SoftwareROMAlyzer ) {
		qmc2SoftwareROMAlyzer->textBrowserLog->setFont(logFont);
		QTimer::singleShot(0, qmc2SoftwareROMAlyzer, SLOT(adjustIconSizes()));
		if ( qmc2SoftwareROMAlyzer->checkSumScannerLog() )
			qmc2SoftwareROMAlyzer->checkSumScannerLog()->plainTextEditLog->setFont(logFont);
		if ( qmc2SoftwareROMAlyzer->collectionRebuilder() ) {
			qmc2SoftwareROMAlyzer->collectionRebuilder()->plainTextEditLog->setFont(logFont);
			QTimer::singleShot(0, qmc2SoftwareROMAlyzer->collectionRebuilder(), SLOT(adjustIconSizes()));
		}
	}
	if ( qmc2ImageChecker )
		qmc2ImageChecker->adjustIconSizes();
	if ( qmc2SampleChecker )
		QTimer::singleShot(0, qmc2SampleChecker, SLOT(adjustIconSizes()));
#if QMC2_USE_PHONON_API || QMC2_MULTIMEDIA_ENABLED
	qmc2MainWindow->toolButtonAudioPreviousTrack->setIconSize(iconSize);
	qmc2MainWindow->toolButtonAudioNextTrack->setIconSize(iconSize);
	qmc2MainWindow->toolButtonAudioFastBackward->setIconSize(iconSize);
	qmc2MainWindow->toolButtonAudioFastForward->setIconSize(iconSize);
	qmc2MainWindow->toolButtonAudioStopTrack->setIconSize(iconSize);
	qmc2MainWindow->toolButtonAudioPauseTrack->setIconSize(iconSize);
	qmc2MainWindow->toolButtonAudioPlayTrack->setIconSize(iconSize);
	qmc2MainWindow->toolButtonAudioAddTracks->setIconSize(iconSize);
	qmc2MainWindow->toolButtonAudioAddURL->setIconSize(iconSize);
	qmc2MainWindow->toolButtonAudioRemoveTracks->setIconSize(iconSize);
	qmc2MainWindow->toolButtonAudioSetupEffects->setIconSize(iconSize);
#endif
#if QMC2_USE_PHONON_API
	if ( qmc2AudioEffectDialog )
		QTimer::singleShot(0, qmc2AudioEffectDialog, SLOT(adjustIconSizes()));
#endif
#if defined(QMC2_YOUTUBE_ENABLED)
	if ( qmc2YouTubeWidget )
		QTimer::singleShot(0, qmc2YouTubeWidget, SLOT(adjustIconSizes()));
#endif
	if ( qmc2ROMStatusExporter )
		QTimer::singleShot(0, qmc2ROMStatusExporter, SLOT(adjustIconSizes()));
	toolButtonBrowseSoftwareListCacheDb->setIconSize(iconSize);
	toolButtonBrowseSoftwareStateCache->setIconSize(iconSize);
	toolButtonBrowseGeneralSoftwareFolder->setIconSize(iconSize);
	if ( qmc2DeviceConfigurator ) {
		qmc2DeviceConfigurator->toolButtonConfiguration->setIconSize(iconSize);
		qmc2DeviceConfigurator->toolButtonNewConfiguration->setIconSize(iconSize);
		qmc2DeviceConfigurator->toolButtonCloneConfiguration->setIconSize(iconSize);
		qmc2DeviceConfigurator->toolButtonSaveConfiguration->setIconSize(iconSize);
		qmc2DeviceConfigurator->toolButtonRemoveConfiguration->setIconSize(iconSize);
		qmc2DeviceConfigurator->toolButtonChooserPlay->setIconSize(iconSize);
		qmc2DeviceConfigurator->toolButtonChooserPlayEmbedded->setIconSize(iconSize);
		qmc2DeviceConfigurator->toolButtonChooserReload->setIconSize(iconSize);
		qmc2DeviceConfigurator->toolButtonChooserClearFilterPattern->setIconSize(iconSize);
		qmc2DeviceConfigurator->toolButtonChooserAutoSelect->setIconSize(iconSize);
		qmc2DeviceConfigurator->toolButtonChooserFilter->setIconSize(iconSize);
		qmc2DeviceConfigurator->toolButtonChooserProcessZIPs->setIconSize(iconSize);
		qmc2DeviceConfigurator->toolButtonChooserMergeMaps->setIconSize(iconSize);
		qmc2DeviceConfigurator->toolButtonChooserSaveConfiguration->setIconSize(iconSize);
		qmc2DeviceConfigurator->comboBoxDeviceInstanceChooser->setIconSize(iconSize);
		qmc2DeviceConfigurator->treeWidgetDeviceSetup->setIconSize(iconSize);
		qmc2DeviceConfigurator->treeWidgetSlotOptions->setIconSize(iconSize);
		((IconLineEdit *)qmc2DeviceConfigurator->comboBoxChooserFilterPattern->lineEdit())->setIconSize(iconSizeMiddle);
	}
	if ( qmc2SoftwareList ) {
		qmc2SoftwareList->toolButtonExport->setIconSize(iconSize);
		qmc2SoftwareList->toolButtonAddToFavorites->setIconSize(iconSize);
		qmc2SoftwareList->toolButtonRemoveFromFavorites->setIconSize(iconSize);
		qmc2SoftwareList->toolButtonFavoritesOptions->setIconSize(iconSize);
		qmc2SoftwareList->toolButtonPlay->setIconSize(iconSize);
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
		qmc2SoftwareList->toolButtonPlayEmbedded->setIconSize(iconSize);
#endif
		qmc2SoftwareList->toolButtonToggleSoftwareInfo->setIconSize(iconSize);
		qmc2SoftwareList->toolButtonCompatFilterToggle->setIconSize(iconSize);
		qmc2SoftwareList->toolButtonReload->setIconSize(iconSize);
		qmc2SoftwareList->toolButtonToggleSnapnameAdjustment->setIconSize(iconSize);
		qmc2SoftwareList->toolButtonSoftwareStates->setIconSize(iconSize);
		qmc2SoftwareList->treeWidgetKnownSoftware->setIconSize(iconSizeMiddle);
		qmc2SoftwareList->treeWidgetFavoriteSoftware->setIconSize(iconSizeMiddle);
		qmc2SoftwareList->treeWidgetSearchResults->setIconSize(iconSizeMiddle);
		if ( qmc2SoftwareList->viewTree() )
			qmc2SoftwareList->toolBoxSoftwareList->setItemIcon(QMC2_SWLIST_KNOWN_SW_PAGE, QIcon(QPixmap(QString::fromUtf8(":/data/img/view_tree.png")).scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
		else
			qmc2SoftwareList->toolBoxSoftwareList->setItemIcon(QMC2_SWLIST_KNOWN_SW_PAGE, QIcon(QPixmap(QString::fromUtf8(":/data/img/view_detail.png")).scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
		qmc2SoftwareList->toolBoxSoftwareList->setItemIcon(QMC2_SWLIST_FAVORITES_PAGE, QIcon(QPixmap(QString::fromUtf8(":/data/img/favorites.png")).scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
		qmc2SoftwareList->toolBoxSoftwareList->setItemIcon(QMC2_SWLIST_SEARCH_PAGE, QIcon(QPixmap(QString::fromUtf8(":/data/img/hint.png")).scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
		((IconLineEdit *)qmc2SoftwareList->comboBoxSearch->lineEdit())->setIconSize(iconSizeMiddle);
		if ( qmc2SoftwareList->exporter )
			QTimer::singleShot(0, qmc2SoftwareList->exporter, SLOT(adjustIconSizes()));
		if ( qmc2SoftwareNotesEditor )
			qmc2SoftwareNotesEditor->adjustIconSizes();
		if ( qmc2SystemNotesEditor )
			qmc2SystemNotesEditor->adjustIconSizes();
	}
	qmc2MainWindow->pushButtonClearFinishedDownloads->setIconSize(iconSize);
	qmc2MainWindow->pushButtonReloadSelectedDownloads->setIconSize(iconSize);
	qmc2MainWindow->pushButtonStopSelectedDownloads->setIconSize(iconSize);
	qmc2MainWindow->treeWidgetDownloads->setIconSize(iconSize);
	qmc2MainWindow->toolButtonSelectRomFilter->setIconSize(iconSize);
	qmc2MainWindow->comboBoxViewSelect->setIconSize(iconSize);
	QTabBar *tabBar = qmc2MainWindow->tabWidgetMachineList->findChild<QTabBar *>();
	if ( tabBar )
		tabBar->setIconSize(iconSizeMiddle);
	tabBar = qmc2MainWindow->tabWidgetMachineDetail->findChild<QTabBar *>();
	if ( tabBar )
		tabBar->setIconSize(iconSizeMiddle);
	tabBar = qmc2MainWindow->tabWidgetLogsAndEmulators->findChild<QTabBar *>();
	if ( tabBar )
		tabBar->setIconSize(iconSizeMiddle);
	tabBar = qmc2MainWindow->tabWidgetSoftwareDetail->findChild<QTabBar *>();
	if ( tabBar )
		tabBar->setIconSize(iconSizeMiddle);
	qmc2MainWindow->toolbar->setIconSize(iconSizeLarge);
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
	for (int i = 0; i < qmc2MainWindow->tabWidgetEmbeddedEmulators->count(); i++) {
		Embedder *embedder = (Embedder *)qmc2MainWindow->tabWidgetEmbeddedEmulators->widget(i);
		embedder->adjustIconSizes();
		if ( embedder->embedderOptions )
			embedder->embedderOptions->adjustIconSizes();
	}
	qmc2MainWindow->toolButtonEmbedderMaximizeToggle->setIconSize(iconSizeLarge);
#if defined(QMC2_OS_UNIX)
	qmc2MainWindow->toolButtonEmbedderAutoPause->setIconSize(iconSizeLarge);
#endif
#endif
	if ( qmc2ComponentSetup )
		if ( qmc2ComponentSetup->isVisible() )
			QTimer::singleShot(0, qmc2ComponentSetup, SLOT(adjustIconSizes()));
	if ( qmc2ToolBarCustomizer )
		if ( qmc2ToolBarCustomizer->isVisible() )
			QTimer::singleShot(0, qmc2ToolBarCustomizer, SLOT(adjustIconSizes()));
	if ( qmc2GlobalEmulatorOptions )
		QTimer::singleShot(0, qmc2GlobalEmulatorOptions, SLOT(adjustIconSizes()));
	if ( qmc2EmulatorOptions )
		QTimer::singleShot(0, qmc2EmulatorOptions, SLOT(adjustIconSizes()));
	QTimer::singleShot(0, qmc2MainWindow, SLOT(updateUserData()));

	// we get an X error / Qt warning here upon qApp->processEvents(), but it seems safe to ignore it
	qmc2SuppressQtMessages = true;
	qApp->processEvents();
	qmc2SuppressQtMessages = config->value(QMC2_FRONTEND_PREFIX + "GUI/SuppressQtMessages", false).toBool();;
	applied = true;
}

void Options::on_pushButtonOk_clicked()
{
	on_pushButtonApply_clicked();
}

void Options::on_pushButtonCancel_clicked()
{
	cancelClicked = true;
	restoreCurrentConfig();
}

void Options::on_pushButtonRestore_clicked()
{
	restoreCurrentConfig();
}

void Options::on_pushButtonApply_clicked()
{
	static int oldCacheSize = 0;
	static bool initialCall = true;
	QString s;
	int i;
	bool needRestart = false,
	     needResort = false,
	     needRecreateTemplateMap = false,
	     needFilter = false,
	     needReopenPreviewFile = false,
	     needReopenFlyerFile = false,
	     needReopenIconFile = false,
	     needReopenCabinetFile = false,
	     needReopenControllerFile = false,
	     needReopenMarqueeFile = false,
	     needReopenTitleFile = false,
	     needReopenPCBFile = false,
	     needReopenSoftwareSnapFile = false,
	     needReload = false,
	     needManualReload = false,
	     needChangeCookieJar = false;

	pushButtonApply->setEnabled(false);
	pushButtonRestore->setEnabled(false);
	pushButtonDefault->setEnabled(false);
	pushButtonOk->setEnabled(false);
	pushButtonCancel->setEnabled(false);

	if ( qmc2EarlyStartup )
		initialCall = true;

	// General
	config->setValue("Version", QString(XSTR(QMC2_VERSION)));
#if QMC2_SVN_REV > 0
	config->setValue("SVN_Revision", QMC2_SVN_REV);
#else
	config->remove("SVN_Revision");
#endif

	// Frontend

	// GUI
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/Toolbar", checkBoxToolbar->isChecked());
#if defined(QMC2_OS_MAC)
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/UnifiedTitleAndToolBarOnMac", checkBoxUnifiedTitleAndToolBarOnMac->isChecked());
	qmc2MainWindow->setUnifiedTitleAndToolBarOnMac(checkBoxUnifiedTitleAndToolBarOnMac->isChecked());
#endif
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/SaveMachineSelection", checkBoxSaveMachineSelection->isChecked());
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/RestoreMachineSelection", checkBoxRestoreMachineSelection->isChecked());
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/Statusbar", checkBoxStatusbar->isChecked());
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/StandardColorPalette", checkBoxStandardColorPalette->isChecked());
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts", checkBoxProgressTexts->isChecked());

	bool b;

	bool invalidateGameInfoDB = false;
	b = checkBoxProcessMameHistoryDat->isChecked();
	needManualReload |= (config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMameHistoryDat").toBool() != b);
	invalidateGameInfoDB |= (config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMameHistoryDat").toBool() != b);
	config->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMameHistoryDat", checkBoxProcessMameHistoryDat->isChecked());
	b = checkBoxProcessMessSysinfoDat->isChecked();
	needManualReload |= (config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMessSysinfoDat").toBool() != b);
	invalidateGameInfoDB |= (config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMessSysinfoDat").toBool() != b);
	config->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMessSysinfoDat", checkBoxProcessMessSysinfoDat->isChecked());

	bool invalidateEmuInfoDB = false;
	b = checkBoxProcessMameInfoDat->isChecked();
	needManualReload |= (config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMameInfoDat").toBool() != b);
	invalidateEmuInfoDB |= (config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMameInfoDat").toBool() != b);
	config->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMameInfoDat", checkBoxProcessMameInfoDat->isChecked());
	b = checkBoxProcessMessInfoDat->isChecked();
	needManualReload |= (config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMessInfoDat").toBool() != b);
	invalidateEmuInfoDB |= (config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMessInfoDat").toBool() != b);
	config->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMessInfoDat", checkBoxProcessMessInfoDat->isChecked());
	b = checkBoxProcessSoftwareInfoDB->isChecked();
	needManualReload |= (config->value(QMC2_FRONTEND_PREFIX + "GUI/ProcessSoftwareInfoDB").toBool() != b);
	bool invalidateSoftwareInfoDB = (config->value(QMC2_FRONTEND_PREFIX + "GUI/ProcessSoftwareInfoDB").toBool() != b);
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/ProcessSoftwareInfoDB", checkBoxProcessSoftwareInfoDB->isChecked());
	qmc2ScaledPreview = checkBoxScaledPreview->isChecked();
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/ScaledPreview", qmc2ScaledPreview);
	qmc2ScaledFlyer = checkBoxScaledFlyer->isChecked();
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/ScaledFlyer", qmc2ScaledFlyer);
	qmc2ScaledCabinet = checkBoxScaledCabinet->isChecked();
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/ScaledCabinet", qmc2ScaledCabinet);
	qmc2ScaledController = checkBoxScaledController->isChecked();
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/ScaledController", qmc2ScaledController);
	qmc2ScaledMarquee = checkBoxScaledMarquee->isChecked();
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/ScaledMarquee", qmc2ScaledMarquee);
	qmc2ScaledTitle = checkBoxScaledTitle->isChecked();
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/ScaledTitle", qmc2ScaledTitle);
	qmc2ScaledPCB = checkBoxScaledPCB->isChecked();
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/ScaledPCB", qmc2ScaledPCB);
	qmc2ScaledSoftwareSnapshot = checkBoxScaledSoftwareSnapshot->isChecked();
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/ScaledSoftwareSnapshot", qmc2ScaledSoftwareSnapshot);
	qmc2SmoothScaling = checkBoxSmoothScaling->isChecked();
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/SmoothScaling", qmc2SmoothScaling);
	qmc2RetryLoadingImages = checkBoxRetryLoadingImages->isChecked();
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/RetryLoadingImages", qmc2RetryLoadingImages);
	qmc2ParentImageFallback = checkBoxParentImageFallback->isChecked();
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/ParentImageFallback", qmc2ParentImageFallback);
	s = comboBoxLanguage->currentText().left(2).toLower();
	needRestart |= (config->value(QMC2_FRONTEND_PREFIX + "GUI/Language", "us").toString() != s);
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/Language", s);

#if QMC2_JOYSTICK == 1
	if ( joystickTestWidget )
		joystickTestWidget->cleanupPalette();
#endif
	if ( !lineEditFont->text().isEmpty() )
		config->setValue(QMC2_FRONTEND_PREFIX + "GUI/Font", lineEditFont->text());
	else
		config->remove(QMC2_FRONTEND_PREFIX + "GUI/Font");
	if ( !lineEditLogFont->text().isEmpty() )
		config->setValue(QMC2_FRONTEND_PREFIX + "GUI/LogFont", lineEditLogFont->text());
	else
		config->remove(QMC2_FRONTEND_PREFIX + "GUI/LogFont");
	if ( spinBoxPixmapCacheSize->value() != oldCacheSize ) {
		oldCacheSize = spinBoxPixmapCacheSize->value();
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("image cache size set to %1 MB").arg(oldCacheSize));
	}
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/PixmapCacheSize", oldCacheSize);
	qmc2ImagePixmapCache.setMaxCost(oldCacheSize * QMC2_1M);
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/MinimizeOnEmuLaunch", checkBoxMinimizeOnEmuLaunch->isChecked());
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/KillEmulatorsOnExit", checkBoxKillEmulatorsOnExit->isChecked());
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/OneEmulatorOnly", checkBoxOneEmulatorOnly->isChecked());
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/ShowMenuBar", checkBoxShowMenuBar->isChecked());
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/CheckSingleInstance", checkBoxCheckSingleInstance->isChecked());
	qmc2SuppressQtMessages = checkBoxSuppressQtMessages->isChecked();
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/SuppressQtMessages", qmc2SuppressQtMessages);
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/ShowSplashScreen", checkBoxShowSplashScreen->isChecked());
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/ShowLoadingAnimation", checkBoxShowLoadingAnimation->isChecked());
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/SetWorkDirFromExec", checkBoxSetWorkDirFromExec->isChecked());
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/MachineStatusIndicator", checkBoxMachineStatusIndicator->isChecked());
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/MachineStatusIndicatorOnlyWhenRequired", checkBoxMachineStatusIndicatorOnlyWhenRequired->isChecked());
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/ShowMachineName", checkBoxShowMachineName->isChecked());
	qmc2ShowMachineName = checkBoxShowMachineName->isChecked();
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/ShowMachineNameOnlyWhenRequired", checkBoxShowMachineNameOnlyWhenRequired->isChecked());
	qmc2ShowMachineNameOnlyWhenRequired = checkBoxShowMachineNameOnlyWhenRequired->isChecked();
	// show / hide game status indicator
	if ( config->value(QMC2_FRONTEND_PREFIX + "GUI/MachineStatusIndicator").toBool() ) {
		if ( config->value(QMC2_FRONTEND_PREFIX + "GUI/MachineStatusIndicatorOnlyWhenRequired").toBool() ) {
			if ( qmc2MainWindow->hSplitter->sizes()[0] == 0 || qmc2MainWindow->tabWidgetMachineList->currentIndex() != QMC2_MACHINELIST_INDEX )
				qmc2MainWindow->labelMachineStatus->setVisible(true);
			else
				qmc2MainWindow->labelMachineStatus->setVisible(false);
		} else
			qmc2MainWindow->labelMachineStatus->setVisible(true);
	} else
		qmc2MainWindow->labelMachineStatus->setVisible(false);
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/FrontendLogSize", spinBoxFrontendLogSize->value());
	qmc2MainWindow->textBrowserFrontendLog->setMaximumBlockCount(spinBoxFrontendLogSize->value());
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/EmulatorLogSize", spinBoxEmulatorLogSize->value());
	qmc2MainWindow->textBrowserEmulatorLog->setMaximumBlockCount(spinBoxEmulatorLogSize->value());
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/NativeFileDialogs", checkBoxNativeFileDialogs->isChecked());

	// Files and directories
	config->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", lineEditTemporaryFile->text());
	config->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/LogFile", lineEditFrontendLogFile->text());
	config->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/DataDirectory", lineEditDataDirectory->text());
	config->setValue(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/DatInfoDatabase", lineEditDatInfoDatabase->text());
	needReopenPreviewFile = (qmc2UsePreviewFile != (stackedWidgetPreview->currentIndex() == 1)) || (initialCall && qmc2UsePreviewFile);
	needReopenPreviewFile |= (QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/PreviewFile").toString() != lineEditPreviewFile->text());
	qmc2UsePreviewFile = (stackedWidgetPreview->currentIndex() == 1);
	config->setValue("MAME/FilesAndDirectories/UsePreviewFile", qmc2UsePreviewFile);
	config->setValue("MAME/FilesAndDirectories/PreviewDirectory", lineEditPreviewDirectory->text());
	config->setValue("MAME/FilesAndDirectories/PreviewFile", lineEditPreviewFile->text());
	config->setValue("MAME/FilesAndDirectories/PreviewFileType", comboBoxPreviewFileType->currentIndex());
	needReopenFlyerFile = (qmc2UseFlyerFile != (stackedWidgetFlyer->currentIndex() == 1)) || (initialCall && qmc2UseFlyerFile);
	needReopenFlyerFile |= (QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/FlyerFile").toString() != lineEditFlyerFile->text());
	qmc2UseFlyerFile = (stackedWidgetFlyer->currentIndex() == 1);
	config->setValue("MAME/FilesAndDirectories/UseFlyerFile", qmc2UseFlyerFile);
	config->setValue("MAME/FilesAndDirectories/FlyerDirectory", lineEditFlyerDirectory->text());
	config->setValue("MAME/FilesAndDirectories/FlyerFile", lineEditFlyerFile->text());
	config->setValue("MAME/FilesAndDirectories/FlyerFileType", comboBoxFlyerFileType->currentIndex());
	needReopenIconFile = (qmc2UseIconFile != (stackedWidgetIcon->currentIndex() == 1)) || (initialCall && qmc2UseIconFile);
	needReopenIconFile |= (QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/IconFile").toString() != lineEditIconFile->text());
	qmc2UseIconFile = (stackedWidgetIcon->currentIndex() == 1);
	config->setValue("MAME/FilesAndDirectories/UseIconFile", qmc2UseIconFile);
	config->setValue("MAME/FilesAndDirectories/IconDirectory", lineEditIconDirectory->text());
	config->setValue("MAME/FilesAndDirectories/IconFile", lineEditIconFile->text());
	config->setValue("MAME/FilesAndDirectories/IconFileType", comboBoxIconFileType->currentIndex());
	needReopenCabinetFile = (qmc2UseCabinetFile != (stackedWidgetCabinet->currentIndex() == 1)) || (initialCall && qmc2UseCabinetFile);
	needReopenCabinetFile |= (QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/CabinetFile").toString() != lineEditCabinetFile->text());
	qmc2UseCabinetFile = (stackedWidgetCabinet->currentIndex() == 1);
	config->setValue("MAME/FilesAndDirectories/UseCabinetFile", qmc2UseCabinetFile);
	config->setValue("MAME/FilesAndDirectories/CabinetDirectory", lineEditCabinetDirectory->text());
	config->setValue("MAME/FilesAndDirectories/CabinetFile", lineEditCabinetFile->text());
	config->setValue("MAME/FilesAndDirectories/CabinetFileType", comboBoxCabinetFileType->currentIndex());
	needReopenControllerFile = (qmc2UseControllerFile != (stackedWidgetController->currentIndex() == 1)) || (initialCall && qmc2UseControllerFile);
	needReopenControllerFile |= (QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/ControllerFile").toString() != lineEditControllerFile->text());
	qmc2UseControllerFile = (stackedWidgetController->currentIndex() == 1);
	config->setValue("MAME/FilesAndDirectories/UseControllerFile", qmc2UseControllerFile);
	config->setValue("MAME/FilesAndDirectories/ControllerDirectory", lineEditControllerDirectory->text());
	config->setValue("MAME/FilesAndDirectories/ControllerFile", lineEditControllerFile->text());
	config->setValue("MAME/FilesAndDirectories/ControllerFileType", comboBoxControllerFileType->currentIndex());
	needReopenMarqueeFile = (qmc2UseMarqueeFile != (stackedWidgetMarquee->currentIndex() == 1)) || (initialCall && qmc2UseMarqueeFile);
	needReopenMarqueeFile |= (QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/MarqueeFile").toString() != lineEditMarqueeFile->text());
	qmc2UseMarqueeFile = (stackedWidgetMarquee->currentIndex() == 1);
	config->setValue("MAME/FilesAndDirectories/UseMarqueeFile", qmc2UseMarqueeFile);
	config->setValue("MAME/FilesAndDirectories/MarqueeDirectory", lineEditMarqueeDirectory->text());
	config->setValue("MAME/FilesAndDirectories/MarqueeFile", lineEditMarqueeFile->text());
	config->setValue("MAME/FilesAndDirectories/MarqueeFileType", comboBoxMarqueeFileType->currentIndex());
	needReopenTitleFile = (qmc2UseTitleFile != (stackedWidgetTitle->currentIndex() == 1)) || (initialCall && qmc2UseTitleFile);
	needReopenTitleFile |= (QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/TitleFile").toString() != lineEditTitleFile->text());
	qmc2UseTitleFile = (stackedWidgetTitle->currentIndex() == 1);
	config->setValue("MAME/FilesAndDirectories/UseTitleFile", qmc2UseTitleFile);
	config->setValue("MAME/FilesAndDirectories/TitleDirectory", lineEditTitleDirectory->text());
	config->setValue("MAME/FilesAndDirectories/TitleFile", lineEditTitleFile->text());
	config->setValue("MAME/FilesAndDirectories/TitleFileType", comboBoxTitleFileType->currentIndex());
	needReopenPCBFile = (qmc2UsePCBFile != (stackedWidgetPCB->currentIndex() == 1)) || (initialCall && qmc2UsePCBFile);
	needReopenPCBFile |= (QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/PCBFile").toString() != lineEditPCBFile->text());
	qmc2UsePCBFile = (stackedWidgetPCB->currentIndex() == 1);
	config->setValue("MAME/FilesAndDirectories/UsePCBFile", qmc2UsePCBFile);
	config->setValue("MAME/FilesAndDirectories/PCBDirectory", lineEditPCBDirectory->text());
	config->setValue("MAME/FilesAndDirectories/PCBFile", lineEditPCBFile->text());
	config->setValue("MAME/FilesAndDirectories/PCBFileType", comboBoxPCBFileType->currentIndex());
	needReopenSoftwareSnapFile = (qmc2UseSoftwareSnapFile != (stackedWidgetSWSnap->currentIndex() == 1)) || (initialCall && qmc2UseSoftwareSnapFile);
	needReopenSoftwareSnapFile |= (QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/SoftwareSnapFile").toString() != lineEditSoftwareSnapFile->text());
	qmc2UseSoftwareSnapFile = (stackedWidgetSWSnap->currentIndex() == 1);
	config->setValue("MAME/FilesAndDirectories/UseSoftwareSnapFile", qmc2UseSoftwareSnapFile);
	config->setValue("MAME/FilesAndDirectories/SoftwareSnapDirectory", lineEditSoftwareSnapDirectory->text());
	config->setValue("MAME/FilesAndDirectories/SoftwareSnapFile", lineEditSoftwareSnapFile->text());
	config->setValue("MAME/FilesAndDirectories/SoftwareSnapFileType", comboBoxSoftwareSnapFileType->currentIndex());
	config->setValue("MAME/FilesAndDirectories/SoftwareNotesFolder", lineEditSoftwareNotesFolder->text());
	config->setValue("MAME/FilesAndDirectories/UseSoftwareNotesTemplate", checkBoxUseSoftwareNotesTemplate->isChecked());
	config->setValue("MAME/FilesAndDirectories/SoftwareNotesTemplate", lineEditSoftwareNotesTemplate->text());
	config->setValue("MAME/FilesAndDirectories/SystemNotesFolder", lineEditSystemNotesFolder->text());
	config->setValue("MAME/FilesAndDirectories/UseSystemNotesTemplate", checkBoxUseSystemNotesTemplate->isChecked());
	config->setValue("MAME/FilesAndDirectories/SystemNotesTemplate", lineEditSystemNotesTemplate->text());
	config->setValue("MAME/FilesAndDirectories/VideoSnapFolder", lineEditVideoSnapFolder->text());
	s = lineEditMameHistoryDat->text();
	needManualReload |= (QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MameHistoryDat").toString() != s);
	invalidateGameInfoDB |= (QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MameHistoryDat").toString() != s);
	config->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MameHistoryDat", lineEditMameHistoryDat->text());
	s = lineEditMessSysinfoDat->text();
	needManualReload |= (QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MessSysinfoDat").toString() != s);
	invalidateGameInfoDB |= (QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MessSysinfoDat").toString() != s);
	config->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MessSysinfoDat", lineEditMessSysinfoDat->text());
	s = lineEditMameInfoDat->text();
	needManualReload |= (QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MameInfoDat").toString() != s);
	invalidateEmuInfoDB |= (QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MameInfoDat").toString() != s);
	config->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MameInfoDat", lineEditMameInfoDat->text());
	s = lineEditMessInfoDat->text();
	needManualReload |= (QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MessInfoDat").toString() != s);
	invalidateEmuInfoDB |= (QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MessInfoDat").toString() != s);
	config->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MessInfoDat", lineEditMessInfoDat->text());
	config->setValue("MAME/FilesAndDirectories/CatverIni", lineEditCatverIniFile->text());
	config->setValue("MAME/FilesAndDirectories/CategoryIni", lineEditCategoryIniFile->text());
	s = lineEditSoftwareInfoDB->text();
	needManualReload |= (config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareInfoDB").toString() != s);
	invalidateSoftwareInfoDB |= (config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareInfoDB").toString() != s);
	config->setValue(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareInfoDB", lineEditSoftwareInfoDB->text());
	if ( qmc2SystemNotesEditor ) {
		qmc2SystemNotesEditor->enableFileNewFromTemplateAction(checkBoxUseSystemNotesTemplate->isChecked());
		qmc2SystemNotesEditor->setCurrentTemplateName(lineEditSystemNotesTemplate->text());
	}
	if ( qmc2SoftwareNotesEditor ) {
		qmc2SoftwareNotesEditor->enableFileNewFromTemplateAction(checkBoxUseSoftwareNotesTemplate->isChecked());
		qmc2SoftwareNotesEditor->setCurrentTemplateName(lineEditSoftwareNotesTemplate->text());
	}
	bool catverUsed = checkBoxUseCatverIni->isChecked();
	needReload |= (config->value(QMC2_FRONTEND_PREFIX + "MachineList/UseCatverIni", false).toBool() != catverUsed );
	config->setValue(QMC2_FRONTEND_PREFIX + "MachineList/UseCatverIni", catverUsed);
	bool categoryUsed = checkBoxUseCategoryIni->isChecked();
	needReload |= (config->value(QMC2_FRONTEND_PREFIX + "MachineList/UseCategoryIni", false).toBool() != categoryUsed );
	config->setValue(QMC2_FRONTEND_PREFIX + "MachineList/UseCategoryIni", categoryUsed);

	qmc2CategoryInfoUsed = catverUsed | categoryUsed;
	qmc2VersionInfoUsed = catverUsed;

	if ( qmc2CategoryInfoUsed ) {
		if ( !qmc2MainWindow->treeWidgetMachineList->isColumnHidden(QMC2_MACHINELIST_COLUMN_CATEGORY) )
			qmc2MainWindow->treeWidgetMachineList->showColumn(QMC2_MACHINELIST_COLUMN_CATEGORY);
		if ( !qmc2MainWindow->treeWidgetHierarchy->isColumnHidden(QMC2_MACHINELIST_COLUMN_CATEGORY) )
			qmc2MainWindow->treeWidgetHierarchy->showColumn(QMC2_MACHINELIST_COLUMN_CATEGORY);
		qmc2MainWindow->actionViewByCategory->setVisible(true);
		qmc2MainWindow->actionViewByCategory->setEnabled(true);
		qmc2MainWindow->actionMenuMachineListHeaderCategory->setVisible(true);
		qmc2MainWindow->actionMenuMachineListHeaderCategory->setEnabled(true);
		qmc2MainWindow->actionMenuHierarchyHeaderCategory->setVisible(true);
		qmc2MainWindow->actionMenuHierarchyHeaderCategory->setEnabled(true);
		qmc2MainWindow->actionMenuVersionHeaderCategory->setVisible(true);
		qmc2MainWindow->actionMenuVersionHeaderCategory->setEnabled(true);
		int index = comboBoxSortCriteria->findText(tr("Category"));
		if ( index < 0 )
			comboBoxSortCriteria->insertItem(comboBoxSortCriteria->count(), tr("Category"));
		index = qmc2MainWindow->comboBoxViewSelect->findText(tr("Category view"));
		if ( index < 0 ) {
			index = qmc2MainWindow->comboBoxViewSelect->count();
			qmc2MainWindow->comboBoxViewSelect->insertItem(index, tr("Category view"));
			qmc2MainWindow->comboBoxViewSelect->setItemIcon(index, QIcon(QString::fromUtf8(":/data/img/category.png")));
		}
	} else {
		qmc2MainWindow->treeWidgetMachineList->hideColumn(QMC2_MACHINELIST_COLUMN_CATEGORY);
		qmc2MainWindow->treeWidgetHierarchy->hideColumn(QMC2_MACHINELIST_COLUMN_CATEGORY);
		qmc2MainWindow->actionViewByCategory->setVisible(false);
		qmc2MainWindow->actionViewByCategory->setEnabled(false);
		qmc2MainWindow->actionMenuMachineListHeaderCategory->setVisible(false);
		qmc2MainWindow->actionMenuMachineListHeaderCategory->setEnabled(false);
		qmc2MainWindow->actionMenuHierarchyHeaderCategory->setVisible(false);
		qmc2MainWindow->actionMenuHierarchyHeaderCategory->setEnabled(false);
		qmc2MainWindow->actionMenuVersionHeaderCategory->setVisible(false);
		qmc2MainWindow->actionMenuVersionHeaderCategory->setEnabled(false);
		int index = comboBoxSortCriteria->findText(tr("Category"));
		if ( index >= 0 )
			comboBoxSortCriteria->removeItem(index);
		index = qmc2MainWindow->comboBoxViewSelect->findText(tr("Category view"));
		if ( index >= 0 )
			qmc2MainWindow->comboBoxViewSelect->removeItem(index);
	}

	if ( qmc2VersionInfoUsed ) {
		if ( !qmc2MainWindow->treeWidgetMachineList->isColumnHidden(QMC2_MACHINELIST_COLUMN_VERSION) )
			qmc2MainWindow->treeWidgetMachineList->showColumn(QMC2_MACHINELIST_COLUMN_VERSION);
		if ( !qmc2MainWindow->treeWidgetHierarchy->isColumnHidden(QMC2_MACHINELIST_COLUMN_VERSION) )
			qmc2MainWindow->treeWidgetHierarchy->showColumn(QMC2_MACHINELIST_COLUMN_VERSION);
		qmc2MainWindow->actionViewByVersion->setVisible(true);
		qmc2MainWindow->actionViewByVersion->setEnabled(true);
		qmc2MainWindow->actionMenuMachineListHeaderVersion->setVisible(true);
		qmc2MainWindow->actionMenuMachineListHeaderVersion->setEnabled(true);
		qmc2MainWindow->actionMenuHierarchyHeaderVersion->setVisible(true);
		qmc2MainWindow->actionMenuHierarchyHeaderVersion->setEnabled(true);
		qmc2MainWindow->actionMenuCategoryHeaderVersion->setVisible(true);
		qmc2MainWindow->actionMenuCategoryHeaderVersion->setEnabled(true);
		int index = comboBoxSortCriteria->findText(tr("Version"));
		if ( index < 0 )
			comboBoxSortCriteria->insertItem(comboBoxSortCriteria->count(), tr("Version"));
		index = qmc2MainWindow->comboBoxViewSelect->findText(tr("Version view"));
		if ( index < 0 ) {
			index = qmc2MainWindow->comboBoxViewSelect->count();
			qmc2MainWindow->comboBoxViewSelect->insertItem(index, tr("Version view"));
			qmc2MainWindow->comboBoxViewSelect->setItemIcon(index, QIcon(QString::fromUtf8(":/data/img/version.png")));
		}
	} else {
		qmc2MainWindow->treeWidgetMachineList->hideColumn(QMC2_MACHINELIST_COLUMN_VERSION);
		qmc2MainWindow->treeWidgetHierarchy->hideColumn(QMC2_MACHINELIST_COLUMN_VERSION);
		qmc2MainWindow->actionViewByVersion->setVisible(false);
		qmc2MainWindow->actionViewByVersion->setEnabled(false);
		qmc2MainWindow->actionMenuMachineListHeaderVersion->setVisible(false);
		qmc2MainWindow->actionMenuMachineListHeaderVersion->setEnabled(false);
		qmc2MainWindow->actionMenuHierarchyHeaderVersion->setVisible(false);
		qmc2MainWindow->actionMenuHierarchyHeaderVersion->setEnabled(false);
		qmc2MainWindow->actionMenuCategoryHeaderVersion->setVisible(false);
		qmc2MainWindow->actionMenuCategoryHeaderVersion->setEnabled(false);
		int index = comboBoxSortCriteria->findText(tr("Version"));
		if ( index >= 0 )
			comboBoxSortCriteria->removeItem(index);
		index = qmc2MainWindow->comboBoxViewSelect->findText(tr("Version view"));
		if ( index >= 0 )
			qmc2MainWindow->comboBoxViewSelect->removeItem(index);
	}

	if ( qmc2ToolBarCustomizer )
		QTimer::singleShot(0, qmc2ToolBarCustomizer, SLOT(refreshAvailableActions()));

	// MachineList
	bool showROMStatusIcons = checkBoxShowROMStatusIcons->isChecked();
	needReload |= (config->value(QMC2_FRONTEND_PREFIX + "MachineList/ShowROMStatusIcons", true).toBool() != showROMStatusIcons );
	config->setValue(QMC2_FRONTEND_PREFIX + "MachineList/ShowROMStatusIcons", showROMStatusIcons);
	bool showDeviceSets = checkBoxShowDeviceSets->isChecked();
	needReload |= (config->value(QMC2_FRONTEND_PREFIX + "MachineList/ShowDeviceSets", true).toBool() != showDeviceSets );
	config->setValue(QMC2_FRONTEND_PREFIX + "MachineList/ShowDeviceSets", showDeviceSets);
	bool showBiosSets = checkBoxShowBiosSets->isChecked();
	needReload |= (config->value(QMC2_FRONTEND_PREFIX + "MachineList/ShowBiosSets", true).toBool() != showBiosSets );
	config->setValue(QMC2_FRONTEND_PREFIX + "MachineList/ShowBiosSets", showBiosSets);
	config->setValue(QMC2_FRONTEND_PREFIX + "MachineList/AutoTriggerROMCheck", checkBoxAutoTriggerROMCheck->isChecked());
	config->setValue(QMC2_FRONTEND_PREFIX + "MachineList/DoubleClickActivation", checkBoxDoubleClickActivation->isChecked());
	config->setValue(QMC2_FRONTEND_PREFIX + "MachineList/PlayOnSublistActivation", checkBoxPlayOnSublistActivation->isChecked());
	qmc2CursorPositioningMode = (QAbstractItemView::ScrollHint)comboBoxCursorPosition->currentIndex();
	config->setValue(QMC2_FRONTEND_PREFIX + "MachineList/CursorPosition", qmc2CursorPositioningMode);
	qmc2DefaultLaunchMode = comboBoxDefaultLaunchMode->currentIndex();
	config->setValue(QMC2_FRONTEND_PREFIX + "MachineList/DefaultLaunchMode", qmc2DefaultLaunchMode);
	qmc2SoftwareSnapPosition = comboBoxSoftwareSnapPosition->currentIndex();
	config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SoftwareSnapPosition", qmc2SoftwareSnapPosition);
	config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SoftwareSnapOnMouseHover", checkBoxSoftwareSnapOnMouseHover->isChecked());
	config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/AutoDisableSoftwareSnap", checkBoxAutoDisableSoftwareSnap->isChecked());
	qmc2MachineListResponsiveness = spinBoxResponsiveness->value();
	config->setValue(QMC2_FRONTEND_PREFIX + "MachineList/Responsiveness", qmc2MachineListResponsiveness);
	qmc2UpdateDelay = spinBoxUpdateDelay->value();
	config->setValue(QMC2_FRONTEND_PREFIX + "MachineList/UpdateDelay", qmc2UpdateDelay);
	i = comboBoxSortCriteria->currentIndex();
	needResort = (i != qmc2SortCriteria);
	qmc2SortCriteria = i;
	config->setValue(QMC2_FRONTEND_PREFIX + "MachineList/SortCriteria", qmc2SortCriteria);
	i = comboBoxSortOrder->currentIndex();
	needResort = needResort || (i == 0 ? qmc2SortOrder != Qt::AscendingOrder : qmc2SortOrder != Qt::DescendingOrder);
	qmc2SortOrder = (i == 0 ? Qt::AscendingOrder : Qt::DescendingOrder);
	config->setValue(QMC2_FRONTEND_PREFIX + "MachineList/SortOrder", qmc2SortOrder);
	QBitArray newFilter(QMC2_ROMSTATE_COUNT);
	config->setValue(QMC2_FRONTEND_PREFIX + "RomStateFilter/ShowCorrect", toolButtonShowC->isChecked());
	newFilter.setBit(QMC2_ROMSTATE_INT_C, toolButtonShowC->isChecked());
	config->setValue(QMC2_FRONTEND_PREFIX + "RomStateFilter/ShowMostlyCorrect", toolButtonShowM->isChecked());
	newFilter.setBit(QMC2_ROMSTATE_INT_M, toolButtonShowM->isChecked());
	config->setValue(QMC2_FRONTEND_PREFIX + "RomStateFilter/ShowIncorrect", toolButtonShowI->isChecked());
	newFilter.setBit(QMC2_ROMSTATE_INT_I, toolButtonShowI->isChecked());
	config->setValue(QMC2_FRONTEND_PREFIX + "RomStateFilter/ShowNotFound", toolButtonShowN->isChecked());
	newFilter.setBit(QMC2_ROMSTATE_INT_N, toolButtonShowN->isChecked());
	config->setValue(QMC2_FRONTEND_PREFIX + "RomStateFilter/ShowUnknown", toolButtonShowU->isChecked());
	newFilter.setBit(QMC2_ROMSTATE_INT_U, toolButtonShowU->isChecked());
	needFilter = (qmc2Filter != newFilter);
	qmc2Filter = newFilter;

	bool oldRSF = config->value(QMC2_FRONTEND_PREFIX + "RomStateFilter/Enabled", true).toBool();
	if ( checkBoxRomStateFilter->isChecked() ) {
		if ( qmc2MainWindow->comboBoxViewSelect->currentIndex() == QMC2_VIEWMACHINELIST_INDEX ) {
			qmc2MainWindow->toolButtonSelectRomFilter->setVisible(true);
			qmc2MainWindow->actionTagVisible->setVisible(true);
			qmc2MainWindow->actionUntagVisible->setVisible(true);
			qmc2MainWindow->actionInvertVisibleTags->setVisible(true);
		}
		qmc2MainWindow->romStateFilter->setEnabled(true);
		config->setValue(QMC2_FRONTEND_PREFIX + "RomStateFilter/Enabled", true);
		if ( !oldRSF ) {
			needReload = true;
			needFilter = true;
		}
	} else {
		qmc2MainWindow->toolButtonSelectRomFilter->setVisible(false);
		qmc2MainWindow->actionTagVisible->setVisible(false);
		qmc2MainWindow->actionUntagVisible->setVisible(false);
		qmc2MainWindow->actionInvertVisibleTags->setVisible(false);
		qmc2MainWindow->romStateFilter->setEnabled(false);
		config->setValue(QMC2_FRONTEND_PREFIX + "RomStateFilter/Enabled", false);
		if ( oldRSF ) {
			needReload = true;
			needFilter = false;
		}
	}

	// Shortcuts / Keys
	QHashIterator<QString, QPair<QString, QAction *> > it(qmc2ShortcutHash);
	while ( it.hasNext() ) {
		it.next();
		QString itShortcut = it.key();
		config->setValue(QString(QMC2_FRONTEND_PREFIX + "Shortcuts/%1").arg(itShortcut), qmc2CustomShortcutHash[itShortcut]);
	}
	setupShortcutActions();

	// Joystick
#if QMC2_JOYSTICK == 1
	config->setValue(QMC2_FRONTEND_PREFIX + "Joystick/EnableJoystickControl", checkBoxEnableJoystickControl->isChecked());
	config->setValue(QMC2_FRONTEND_PREFIX + "Joystick/Index", comboBoxSelectJoysticks->currentIndex());
	config->setValue(QMC2_FRONTEND_PREFIX + "Joystick/AutoRepeat", checkBoxJoystickAutoRepeat->isChecked());
	config->setValue(QMC2_FRONTEND_PREFIX + "Joystick/AutoRepeatTimeout", spinBoxJoystickAutoRepeatTimeout->value());
	config->setValue(QMC2_FRONTEND_PREFIX + "Joystick/EventTimeout", spinBoxJoystickEventTimeout->value());

	// Joystick function map
	it.toFront();
	while ( it.hasNext() ) {
		it.next();
		QString itShortcut = it.key();
		QString myKey = qmc2JoystickFunctionHash.key(itShortcut);
		if ( myKey.isEmpty() )
			config->remove(QString(QMC2_FRONTEND_PREFIX + "Joystick/Map/%1").arg(itShortcut));
		else
			config->setValue(QString(QMC2_FRONTEND_PREFIX + "Joystick/Map/%1").arg(itShortcut), myKey);
	}
	if ( joystick ) {
		if ( joystick->isOpen() )
			joystick->close();
		// (re)connect joystick callbacks to main widget
		joystick->disconnect(qmc2MainWindow);
		if ( config->value(QMC2_FRONTEND_PREFIX + "Joystick/EnableJoystickControl").toBool() ) {
			if ( joystick->open(config->value(QMC2_FRONTEND_PREFIX + "Joystick/Index").toInt()) ) {
				connect(joystick, SIGNAL(axisValueChanged(int, int)), qmc2MainWindow, SLOT(joystickAxisValueChanged(int, int)));
				connect(joystick, SIGNAL(buttonValueChanged(int, bool)), qmc2MainWindow, SLOT(joystickButtonValueChanged(int, bool)));
				connect(joystick, SIGNAL(hatValueChanged(int, int)), qmc2MainWindow, SLOT(joystickHatValueChanged(int, int)));
				connect(joystick, SIGNAL(trackballValueChanged(int, int, int)), qmc2MainWindow, SLOT(joystickTrackballValueChanged(int, int, int)));
				qmc2MainWindow->joyIndex = config->value(QMC2_FRONTEND_PREFIX + "Joystick/Index").toInt();
			}
		}
	}
	toolButtonMapJoystick->setChecked(true);
	on_toolButtonMapJoystick_clicked();
#endif

	// Network / Tools
	CookieJar *cj = 0;
	bool restoreCookies = config->value(QMC2_FRONTEND_PREFIX + "WebBrowser/RestoreCookies", true).toBool();
	config->setValue(QMC2_FRONTEND_PREFIX + "WebBrowser/RestoreCookies", checkBoxRestoreCookies->isChecked());
	needChangeCookieJar = restoreCookies != checkBoxRestoreCookies->isChecked();
	if ( restoreCookies )
		cj = (CookieJar *)qmc2NetworkAccessManager->cookieJar();
	QString cookieDatabase = config->value(QMC2_FRONTEND_PREFIX + "WebBrowser/CookieDatabase", QString()).toString();
	config->setValue(QMC2_FRONTEND_PREFIX + "WebBrowser/CookieDatabase", lineEditCookieDatabase->text());
	needChangeCookieJar |= cookieDatabase != lineEditCookieDatabase->text();
	config->setValue(QMC2_FRONTEND_PREFIX + "Tools/ZipTool", lineEditZipTool->text());
	config->setValue(QMC2_FRONTEND_PREFIX + "Tools/ZipToolRemovalArguments", lineEditZipToolRemovalArguments->text());
	config->setValue(QMC2_FRONTEND_PREFIX + "Tools/SevenZipTool", lineEditSevenZipTool->text());
	config->setValue(QMC2_FRONTEND_PREFIX + "Tools/SevenZipToolRemovalArguments", lineEditSevenZipToolRemovalArguments->text());
	config->setValue(QMC2_FRONTEND_PREFIX + "Tools/RomTool", lineEditRomTool->text());
	config->setValue(QMC2_FRONTEND_PREFIX + "Tools/RomToolArguments", lineEditRomToolArguments->text());
	config->setValue(QMC2_FRONTEND_PREFIX + "Tools/RomToolWorkingDirectory", lineEditRomToolWorkingDirectory->text());
	config->setValue(QMC2_FRONTEND_PREFIX + "Tools/CopyToolOutput", checkBoxCopyToolOutput->isChecked());
	config->setValue(QMC2_FRONTEND_PREFIX + "Tools/CloseToolDialog", checkBoxCloseToolDialog->isChecked());
	config->setValue("Network/HTTPProxy/Enable", groupBoxHTTPProxy->isChecked());
	config->setValue("Network/HTTPProxy/Host", lineEditHTTPProxyHost->text());
	config->setValue("Network/HTTPProxy/Port", spinBoxHTTPProxyPort->value());
	config->setValue("Network/HTTPProxy/UserID", lineEditHTTPProxyUserID->text());
	CryptedByteArray cpw(lineEditHTTPProxyPassword->text().toLatin1());
	config->setValue("Network/HTTPProxy/Password", cpw.encryptedData());
	if ( groupBoxHTTPProxy->isChecked() ) {
		QNetworkProxy::setApplicationProxy(QNetworkProxy(QNetworkProxy::HttpProxy, 
					lineEditHTTPProxyHost->text(),
					spinBoxHTTPProxyPort->value(),
					lineEditHTTPProxyUserID->text().isEmpty() ? QString() : lineEditHTTPProxyUserID->text(),
					lineEditHTTPProxyPassword->text().isEmpty() ? QString() : lineEditHTTPProxyPassword->text()));
	} else
		QNetworkProxy::setApplicationProxy(QNetworkProxy(QNetworkProxy::NoProxy));

	if ( needChangeCookieJar ) {
		if ( cj )
			cj->db.close();
		if ( checkBoxRestoreCookies->isChecked() )
			qmc2NetworkAccessManager->setCookieJar(new CookieJar(qmc2NetworkAccessManager));
		else
			qmc2NetworkAccessManager->setCookieJar(new QNetworkCookieJar(qmc2NetworkAccessManager));
	}

	if ( config->value(QMC2_FRONTEND_PREFIX + "GUI/MachineListView").toInt() >= qmc2MainWindow->comboBoxViewSelect->count() )
		config->setValue(QMC2_FRONTEND_PREFIX + "GUI/MachineListView", QMC2_VIEWMACHINELIST_INDEX);

	qmc2MainWindow->comboBoxViewSelect->setCurrentIndex(config->value(QMC2_FRONTEND_PREFIX + "GUI/MachineListView", QMC2_VIEWMACHINELIST_INDEX).toInt());
	switch ( qmc2MainWindow->comboBoxViewSelect->currentIndex() ) {
		case QMC2_VIEWMACHINELIST_INDEX:
			qmc2MainWindow->tabWidgetMachineList->setTabIcon(QMC2_MACHINELIST_INDEX, QIcon(QString::fromUtf8(":/data/img/flat.png")));
			break;
		case QMC2_VIEWHIERARCHY_INDEX:
			qmc2MainWindow->tabWidgetMachineList->setTabIcon(QMC2_MACHINELIST_INDEX, QIcon(QString::fromUtf8(":/data/img/clone.png")));
			break;
		case QMC2_VIEWCATEGORY_INDEX:
			qmc2MainWindow->tabWidgetMachineList->setTabIcon(QMC2_MACHINELIST_INDEX, QIcon(QString::fromUtf8(":/data/img/category.png")));
			break;
		case QMC2_VIEWVERSION_INDEX:
			qmc2MainWindow->tabWidgetMachineList->setTabIcon(QMC2_MACHINELIST_INDEX, QIcon(QString::fromUtf8(":/data/img/version.png")));
			break;
	}

	// Emulator

	// Configuration
	if ( qmc2GuiReady ) {
		if ( qmc2GlobalEmulatorOptions->changed ) {
			if ( qmc2EmulatorOptions ) {
				switch ( QMessageBox::question(this, tr("Confirm"), 
							tr("An open machine-specific emulator configuration has been detected.\nUse local machine-settings, overwrite with global settings or don't apply?"),
							tr("&Local"), tr("&Overwrite"), tr("Do&n't apply"), 0, 2) ) {
					case 0:
						qmc2EmulatorOptions->save();
						qmc2GlobalEmulatorOptions->save();
						qmc2GlobalEmulatorOptions->load();
						qmc2EmulatorOptions->load();
						break;

					case 1:
						qmc2GlobalEmulatorOptions->save();
						qmc2GlobalEmulatorOptions->load();
						qmc2EmulatorOptions->load(true);
						qmc2EmulatorOptions->save();
						break;

					case 2: 
					default:
						break;
				}
			} else {
				qmc2GlobalEmulatorOptions->save();
				qmc2GlobalEmulatorOptions->load();
			}
		}
	}

	QTimer::singleShot(0, qmc2GlobalEmulatorOptions, SLOT(updateAllEmuOptActions()));

	if ( qmc2EmulatorOptions )
		QTimer::singleShot(0, qmc2EmulatorOptions, SLOT(updateAllEmuOptActions()));

	// Files and directories
	needReload |= config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ExecutableFile").toString() != lineEditExecutableFile->text();
	config->setValue(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ExecutableFile", lineEditExecutableFile->text());
	config->setValue(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory", lineEditWorkingDirectory->text());
	config->setValue(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/LogFile", lineEditEmulatorLogFile->text());
	config->setValue(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/XmlCacheDatabase", lineEditXmlCacheDatabase->text());
	config->setValue(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/UserDataDatabase", lineEditUserDataDatabase->text());
	config->setValue(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/MachineListCacheFile", lineEditMachineListCacheFile->text());
	config->setValue(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ROMStateCacheFile", lineEditROMStateCacheFile->text());
	config->setValue(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SlotInfoCacheFile", lineEditSlotInfoCacheFile->text());
	config->setValue(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareListCacheDatabase", lineEditSoftwareListCacheDb->text());
	config->setValue(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareStateCache", lineEditSoftwareStateCache->text());
	config->setValue(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/GeneralSoftwareFolder", lineEditGeneralSoftwareFolder->text());
	s = lineEditOptionsTemplateFile->text();
	needRecreateTemplateMap = needRecreateTemplateMap || (config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/OptionsTemplateFile").toString() != s );
	config->setValue(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/OptionsTemplateFile", s);
	config->setValue(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/FavoritesFile", lineEditFavoritesFile->text());
	config->setValue(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/HistoryFile", lineEditHistoryFile->text());
	config->setValue(QMC2_EMULATOR_PREFIX + "AutoClearEmuCaches", checkBoxAutoClearEmuCaches->isChecked());
	config->setValue(QMC2_EMULATOR_PREFIX + "SkipEmuIdent", checkBoxSkipEmuIdent->isChecked());

	// Additional emulators
	tableWidgetRegisteredEmulators->setSortingEnabled(false);
	config->remove(QMC2_EMULATOR_PREFIX + "RegisteredEmulators");
	for (i = 0; i < tableWidgetRegisteredEmulators->rowCount(); i++) {
		if ( tableWidgetRegisteredEmulators->item(i, QMC2_ADDTLEMUS_COLUMN_NAME) ) {
			QString emuIcon, emuName, emuCommand, emuWorkDir, emuArgs;
			emuName = tableWidgetRegisteredEmulators->item(i, QMC2_ADDTLEMUS_COLUMN_NAME)->text();
			if ( tableWidgetRegisteredEmulators->item(i, QMC2_ADDTLEMUS_COLUMN_EXEC) )
				emuCommand = tableWidgetRegisteredEmulators->item(i, QMC2_ADDTLEMUS_COLUMN_EXEC)->text();
			if ( tableWidgetRegisteredEmulators->item(i, QMC2_ADDTLEMUS_COLUMN_WDIR) )
				emuWorkDir = tableWidgetRegisteredEmulators->item(i, QMC2_ADDTLEMUS_COLUMN_WDIR)->text();
			if ( tableWidgetRegisteredEmulators->item(i, QMC2_ADDTLEMUS_COLUMN_ARGS) )
				emuArgs = tableWidgetRegisteredEmulators->item(i, QMC2_ADDTLEMUS_COLUMN_ARGS)->text();
			config->setValue(QString(QMC2_EMULATOR_PREFIX + "RegisteredEmulators/%1/Executable").arg(emuName), emuCommand);
			config->setValue(QString(QMC2_EMULATOR_PREFIX + "RegisteredEmulators/%1/WorkingDirectory").arg(emuName), emuWorkDir);
			config->setValue(QString(QMC2_EMULATOR_PREFIX + "RegisteredEmulators/%1/Arguments").arg(emuName), emuArgs);
			QToolButton *tb = (QToolButton *)tableWidgetRegisteredEmulators->cellWidget(i, QMC2_ADDTLEMUS_COLUMN_ICON);
			if ( tb )
				emuIcon = tb->whatsThis();
			if ( !emuIcon.startsWith(":") && !emuIcon.isEmpty() )
				config->setValue(QString(QMC2_EMULATOR_PREFIX + "RegisteredEmulators/%1/Icon").arg(emuName), emuIcon);
			else
				config->remove(QString(QMC2_EMULATOR_PREFIX + "RegisteredEmulators/%1/Icon").arg(emuName));
		}
	}
	tableWidgetRegisteredEmulators->setSortingEnabled(true);

	// remove custom ID lists, if applicable
	foreach (QString emuName, registeredEmulatorsToBeRemoved)
		config->remove(QString(QMC2_EMULATOR_PREFIX + "CustomIDs/%1").arg(emuName));
	registeredEmulatorsToBeRemoved.clear();

	// sync settings (write settings to disk) and apply
	apply();

	if ( invalidateGameInfoDB )
		qmc2MachineList->datInfoDb()->recreateMachineInfoTable();

	if ( invalidateEmuInfoDB )
		qmc2MachineList->datInfoDb()->recreateEmuInfoTable();

	if ( invalidateSoftwareInfoDB )
		qmc2MachineList->datInfoDb()->recreateSoftwareInfoTable();

	if ( needManualReload )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("please reload machine list for some changes to take effect"));

	if ( needRestart )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("please restart QMC2 for some changes to take effect"));

	if ( needRecreateTemplateMap )
		qmc2MainWindow->on_actionRecreateTemplateMap_triggered();

	if ( needResort && !needReload ) {
		qmc2SortingActive = true;
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("sorting machine list by %1 in %2 order").arg(qmc2MachineList->sortCriteriaName(qmc2SortCriteria)).arg(qmc2SortOrder == Qt::AscendingOrder ? tr("ascending") : tr("descending")));
		foreach (QTreeWidgetItem *ti, qmc2ExpandedMachineListItems) {
			qmc2MainWindow->treeWidgetMachineList->collapseItem(ti);
			QList<QTreeWidgetItem *> childrenList = ti->takeChildren();
			foreach (QTreeWidgetItem *ci, ti->takeChildren())
				delete ci;
			QTreeWidgetItem *nameItem = new QTreeWidgetItem(ti);
			nameItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, MachineList::trWaitingForData);
			nameItem->setText(QMC2_MACHINELIST_COLUMN_ICON, ti->text(QMC2_MACHINELIST_COLUMN_NAME));
		}
		qmc2ExpandedMachineListItems.clear();
		qmc2MainWindow->treeWidgetMachineList->setUpdatesEnabled(false);
		qmc2MainWindow->treeWidgetHierarchy->setUpdatesEnabled(false);
		qmc2MainWindow->treeWidgetCategoryView->setUpdatesEnabled(false);
		qmc2MainWindow->treeWidgetVersionView->setUpdatesEnabled(false);
		if ( qmc2SortCriteria == QMC2_SORT_BY_RANK && !qmc2MachineList->userDataDb()->rankCacheComplete() )
			qmc2MachineList->userDataDb()->fillUpRankCache();
		qmc2MainWindow->treeWidgetMachineList->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
		qmc2MainWindow->treeWidgetHierarchy->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
		if ( qmc2MainWindow->treeWidgetCategoryView->topLevelItemCount() > 0 )
			qmc2MainWindow->treeWidgetCategoryView->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
		if ( qmc2MainWindow->treeWidgetVersionView->topLevelItemCount() > 0 )
			qmc2MainWindow->treeWidgetVersionView->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
		qmc2MainWindow->treeWidgetMachineList->setUpdatesEnabled(true);
		qmc2MainWindow->treeWidgetHierarchy->setUpdatesEnabled(true);
		qmc2MainWindow->treeWidgetCategoryView->setUpdatesEnabled(true);
		qmc2MainWindow->treeWidgetVersionView->setUpdatesEnabled(true);
		qmc2SortingActive = false;
		QTimer::singleShot(0, qmc2MainWindow, SLOT(scrollToCurrentItem()));
	}

	switch ( qmc2SortCriteria ) {
		case QMC2_SORT_BY_DESCRIPTION:
			qmc2MainWindow->treeWidgetMachineList->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2SortOrder);
			qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2SortOrder);
			qmc2MainWindow->treeWidgetMachineList->header()->setSortIndicatorShown(true);
			qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(true);
			qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2SortOrder);
			qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicatorShown(true);
			qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2SortOrder);
			qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicatorShown(true);
			break;

		case QMC2_SORT_BY_TAG:
			qmc2MainWindow->treeWidgetMachineList->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_TAG, qmc2SortOrder);
			qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_TAG, qmc2SortOrder);
			qmc2MainWindow->treeWidgetMachineList->header()->setSortIndicatorShown(true);
			qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(true);
			qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_TAG, qmc2SortOrder);
			qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicatorShown(true);
			qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_TAG, qmc2SortOrder);
			qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicatorShown(true);
			break;

		case QMC2_SORT_BY_YEAR:
			qmc2MainWindow->treeWidgetMachineList->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_YEAR, qmc2SortOrder);
			qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_YEAR, qmc2SortOrder);
			qmc2MainWindow->treeWidgetMachineList->header()->setSortIndicatorShown(true);
			qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(true);
			qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_YEAR, qmc2SortOrder);
			qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicatorShown(true);
			qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_YEAR, qmc2SortOrder);
			qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicatorShown(true);
			break;

		case QMC2_SORT_BY_MANUFACTURER:
			qmc2MainWindow->treeWidgetMachineList->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_MANU, qmc2SortOrder);
			qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_MANU, qmc2SortOrder);
			qmc2MainWindow->treeWidgetMachineList->header()->setSortIndicatorShown(true);
			qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(true);
			qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_MANU, qmc2SortOrder);
			qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicatorShown(true);
			qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_MANU, qmc2SortOrder);
			qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicatorShown(true);
			break;

		case QMC2_SORT_BY_NAME:
			qmc2MainWindow->treeWidgetMachineList->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_NAME, qmc2SortOrder);
			qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_NAME, qmc2SortOrder);
			qmc2MainWindow->treeWidgetMachineList->header()->setSortIndicatorShown(true);
			qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(true);
			qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_NAME, qmc2SortOrder);
			qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicatorShown(true);
			qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_NAME, qmc2SortOrder);
			qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicatorShown(true);
			break;

		case QMC2_SORT_BY_ROMTYPES:
			qmc2MainWindow->treeWidgetMachineList->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_RTYPES, qmc2SortOrder);
			qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_RTYPES, qmc2SortOrder);
			qmc2MainWindow->treeWidgetMachineList->header()->setSortIndicatorShown(true);
			qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(true);
			qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_RTYPES, qmc2SortOrder);
			qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicatorShown(true);
			qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_RTYPES, qmc2SortOrder);
			qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicatorShown(true);
			break;

		case QMC2_SORT_BY_PLAYERS:
			qmc2MainWindow->treeWidgetMachineList->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_PLAYERS, qmc2SortOrder);
			qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_PLAYERS, qmc2SortOrder);
			qmc2MainWindow->treeWidgetMachineList->header()->setSortIndicatorShown(true);
			qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(true);
			qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_PLAYERS, qmc2SortOrder);
			qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicatorShown(true);
			qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_PLAYERS, qmc2SortOrder);
			qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicatorShown(true);
			break;

		case QMC2_SORT_BY_DRVSTAT:
			qmc2MainWindow->treeWidgetMachineList->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_DRVSTAT, qmc2SortOrder);
			qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_DRVSTAT, qmc2SortOrder);
			qmc2MainWindow->treeWidgetMachineList->header()->setSortIndicatorShown(true);
			qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(true);
			qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_DRVSTAT, qmc2SortOrder);
			qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicatorShown(true);
			qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_DRVSTAT, qmc2SortOrder);
			qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicatorShown(true);
			break;

		case QMC2_SORT_BY_SRCFILE:
			qmc2MainWindow->treeWidgetMachineList->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_SRCFILE, qmc2SortOrder);
			qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_SRCFILE, qmc2SortOrder);
			qmc2MainWindow->treeWidgetMachineList->header()->setSortIndicatorShown(true);
			qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(true);
			qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_SRCFILE, qmc2SortOrder);
			qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicatorShown(true);
			qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_SRCFILE, qmc2SortOrder);
			qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicatorShown(true);
			break;

		case QMC2_SORT_BY_RANK:
			qmc2MainWindow->treeWidgetMachineList->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_RANK, qmc2SortOrder);
			qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_RANK, qmc2SortOrder);
			qmc2MainWindow->treeWidgetMachineList->header()->setSortIndicatorShown(true);
			qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(true);
			qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_RANK, qmc2SortOrder);
			qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicatorShown(true);
			qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_RANK, qmc2SortOrder);
			qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicatorShown(true);
			break;

		case QMC2_SORT_BY_CATEGORY:
			qmc2MainWindow->treeWidgetMachineList->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_CATEGORY, qmc2SortOrder);
			qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_CATEGORY, qmc2SortOrder);
			qmc2MainWindow->treeWidgetMachineList->header()->setSortIndicatorShown(true);
			qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(true);
			qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_CATEGORY, qmc2SortOrder);
			qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_CATEGORY, qmc2SortOrder);
			qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicatorShown(true);
			qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicatorShown(true);
			break;

		case QMC2_SORT_BY_VERSION:
			qmc2MainWindow->treeWidgetMachineList->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_VERSION, qmc2SortOrder);
			qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_VERSION, qmc2SortOrder);
			qmc2MainWindow->treeWidgetMachineList->header()->setSortIndicatorShown(true);
			qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(true);
			qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_VERSION, qmc2SortOrder);
			qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicator(QMC2_MACHINELIST_COLUMN_VERSION, qmc2SortOrder);
			qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicatorShown(true);
			qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicatorShown(true);
			break;

		default:
			qmc2MainWindow->treeWidgetMachineList->header()->setSortIndicatorShown(false);
			qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(false);
			qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicatorShown(false);
			qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicatorShown(false);
			break;
	}

	if ( needFilter && !needReload ) {
		qmc2StatesTogglesEnabled = false;
		qmc2MainWindow->romStateFilter->toolButtonCorrect->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_C]);
		qmc2MainWindow->romStateFilter->toolButtonMostlyCorrect->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_M]);
		qmc2MainWindow->romStateFilter->toolButtonIncorrect->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_I]);
		qmc2MainWindow->romStateFilter->toolButtonNotFound->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_N]);
		qmc2MainWindow->romStateFilter->toolButtonUnknown->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_U]);
		qmc2MachineList->filter();
	}

	QList<ImageWidget *> iwl;
	iwl << qmc2Preview << qmc2Flyer << qmc2Cabinet << qmc2Controller << qmc2Marquee << qmc2Title << qmc2PCB;
	foreach (ImageWidget *iw, iwl) {
		if ( iw ) {
			bool needReopenFile = false;
			switch ( iw->imageTypeNumeric() ) {
				case QMC2_IMGTYPE_PREVIEW:
					needReopenFile |= needReopenPreviewFile;
					break;
				case QMC2_IMGTYPE_FLYER:
					needReopenFile |= needReopenFlyerFile;
					break;
				case QMC2_IMGTYPE_CABINET:
					needReopenFile |= needReopenCabinetFile;
					break;
				case QMC2_IMGTYPE_CONTROLLER:
					needReopenFile |= needReopenControllerFile;
					break;
				case QMC2_IMGTYPE_MARQUEE:
					needReopenFile |= needReopenMarqueeFile;
					break;
				case QMC2_IMGTYPE_TITLE:
					needReopenFile |= needReopenTitleFile;
					break;
				case QMC2_IMGTYPE_PCB:
					needReopenFile |= needReopenPCBFile;
					break;
			}
			if ( needReopenFile )
				iw->reopenSource();
			iw->update();
		}
	}

	if ( qmc2SoftwareSnap ) {
		if ( needReopenSoftwareSnapFile )
			qmc2SoftwareSnap->reopenSource();
		qmc2SoftwareSnap->update();
	}

	if ( needReopenIconFile ) {
		foreach (unzFile iconFile, qmc2IconFileMap)
			unzClose(iconFile);
		qmc2IconFileMap.clear();
		foreach (SevenZipFile *iconFile, qmc2IconFileMap7z) {
			iconFile->close();
			delete iconFile;
		}
		qmc2IconFileMap7z.clear();
#if defined(QMC2_LIBARCHIVE_ENABLED)
		foreach (ArchiveFile *iconFile, qmc2IconArchiveMap) {
			iconFile->close();
			delete iconFile;
		}
		qmc2IconArchiveMap.clear();
#endif
		switch ( iconFileType() ) {
			case QMC2_ICON_FILETYPE_ZIP:
				foreach (QString filePath, config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/IconFile").toString().split(";", QString::SkipEmptyParts)) {
					unzFile iconFile = unzOpen(filePath.toUtf8().constData());
					if ( iconFile == 0 )
						qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open icon file, please check access permissions for %1").arg(filePath));
					else
						qmc2IconFileMap.insert(filePath, iconFile);
				}
				break;
			case QMC2_ICON_FILETYPE_7Z:
				foreach (QString filePath, config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/IconFile").toString().split(";", QString::SkipEmptyParts)) {
					SevenZipFile *iconFile = new SevenZipFile(filePath);
					if ( !iconFile->open() ) {
						qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open icon file %1").arg(filePath) + " - " + tr("7z error") + ": " + iconFile->lastError());
						delete iconFile;
					} else
						qmc2IconFileMap7z.insert(filePath, iconFile);
				}
				break;
#if defined(QMC2_LIBARCHIVE_ENABLED)
			case QMC2_ICON_FILETYPE_ARCHIVE:
				foreach (QString filePath, qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/IconFile").toString().split(";", QString::SkipEmptyParts)) {
					ArchiveFile *archiveFile = new ArchiveFile(filePath, true);
					if ( !archiveFile->open() ) {
						qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open icon file %1").arg(filePath) + " - " + tr("libarchive error") + ": " + archiveFile->errorString());
						delete archiveFile;
					} else
						qmc2IconArchiveMap.insert(filePath, archiveFile);
				}
			break;
#endif
		}
	}
	if ( needReload ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("triggering automatic reload of machine list"));
		qmc2AutomaticReload = true;
		QTimer::singleShot(0, qmc2MainWindow->actionReload, SLOT(trigger()));
	}
	pushButtonApply->setEnabled(true);
	pushButtonRestore->setEnabled(true);
	pushButtonDefault->setEnabled(true);
	pushButtonOk->setEnabled(true);
	pushButtonCancel->setEnabled(true);
	QTimer::singleShot(0, this, SLOT(applyDelayed()));
	initialCall = false;
}

void Options::on_pushButtonDefault_clicked()
{
	restoreCurrentConfig(true);
}

void Options::restoreCurrentConfig(bool useDefaultSettings)
{
	treeWidgetShortcuts->clear();

	if ( useDefaultSettings ) {
		QString fn = config->fileName();
		delete config;
		QFile f(fn);
		f.copy(fn + ".bak");
		f.remove();
		config = new Settings(QSettings::IniFormat, QSettings::UserScope, "qmc2");
		qmc2Config = config;
	}

	QString userScopePath = configPath();

	// Frontend

	// GUI
	checkBoxToolbar->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/Toolbar", true).toBool());
#if defined(QMC2_OS_MAC)
	checkBoxUnifiedTitleAndToolBarOnMac->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/UnifiedTitleAndToolBarOnMac", false).toBool());
#endif
	checkBoxSaveMachineSelection->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveMachineSelection", true).toBool());
	checkBoxRestoreMachineSelection->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/RestoreMachineSelection", true).toBool());
	checkBoxStatusbar->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/Statusbar", true).toBool());
	checkBoxStandardColorPalette->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/StandardColorPalette", true).toBool());
	checkBoxProgressTexts->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts", false).toBool());
	checkBoxProcessMameHistoryDat->setChecked(config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMameHistoryDat", false).toBool());
	checkBoxProcessMessSysinfoDat->setChecked(config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMessSysinfoDat", false).toBool());
	checkBoxProcessMameInfoDat->setChecked(config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMameInfoDat", false).toBool());
	checkBoxProcessMessInfoDat->setChecked(config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMessInfoDat", false).toBool());
	checkBoxProcessSoftwareInfoDB->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/ProcessSoftwareInfoDB", false).toBool());
	qmc2ScaledPreview = config->value(QMC2_FRONTEND_PREFIX + "GUI/ScaledPreview", true).toBool();
	checkBoxScaledPreview->setChecked(qmc2ScaledPreview);
	qmc2ScaledFlyer = config->value(QMC2_FRONTEND_PREFIX + "GUI/ScaledFlyer", true).toBool();
	checkBoxScaledFlyer->setChecked(qmc2ScaledFlyer);
	qmc2ScaledCabinet = config->value(QMC2_FRONTEND_PREFIX + "GUI/ScaledCabinet", true).toBool();
	checkBoxScaledCabinet->setChecked(qmc2ScaledCabinet);
	qmc2ScaledController = config->value(QMC2_FRONTEND_PREFIX + "GUI/ScaledController", true).toBool();
	checkBoxScaledController->setChecked(qmc2ScaledController);
	qmc2ScaledMarquee = config->value(QMC2_FRONTEND_PREFIX + "GUI/ScaledMarquee", true).toBool();
	checkBoxScaledMarquee->setChecked(qmc2ScaledMarquee);
	qmc2ScaledTitle = config->value(QMC2_FRONTEND_PREFIX + "GUI/ScaledTitle", true).toBool();
	checkBoxScaledTitle->setChecked(qmc2ScaledTitle);
	qmc2ScaledPCB = config->value(QMC2_FRONTEND_PREFIX + "GUI/ScaledPCB", true).toBool();
	checkBoxScaledPCB->setChecked(qmc2ScaledPCB);
	qmc2ScaledSoftwareSnapshot = config->value(QMC2_FRONTEND_PREFIX + "GUI/ScaledSoftwareSnapshot", true).toBool();
	checkBoxScaledSoftwareSnapshot->setChecked(qmc2ScaledSoftwareSnapshot);
	qmc2SmoothScaling = config->value(QMC2_FRONTEND_PREFIX + "GUI/SmoothScaling", true).toBool();
	checkBoxSmoothScaling->setChecked(qmc2SmoothScaling);
	qmc2RetryLoadingImages = config->value(QMC2_FRONTEND_PREFIX + "GUI/RetryLoadingImages", true).toBool();
	checkBoxRetryLoadingImages->setChecked(qmc2RetryLoadingImages);
	qmc2ParentImageFallback = config->value(QMC2_FRONTEND_PREFIX + "GUI/ParentImageFallback", true).toBool();
	checkBoxParentImageFallback->setChecked(qmc2ParentImageFallback);
	comboBoxLanguage->setCurrentIndex(comboBoxLanguage->findText(config->value(QMC2_FRONTEND_PREFIX + "GUI/Language", "us").toString().toUpper(), Qt::MatchContains | Qt::MatchCaseSensitive));
	comboBoxStyle->clear();
	comboBoxStyle->addItem(QObject::tr("Default"));
	comboBoxStyle->addItems(QStyleFactory::keys());
	QString myStyle = QObject::tr((const char *)config->value(QMC2_FRONTEND_PREFIX + "GUI/Style", "Default").toString().toUtf8());
	int styleIndex = comboBoxStyle->findText(myStyle, Qt::MatchFixedString);
	if ( styleIndex < 0 )
		styleIndex = 0;
	comboBoxStyle->setCurrentIndex(styleIndex);
	lineEditStyleSheet->setText(config->value(QMC2_FRONTEND_PREFIX + "GUI/StyleSheet", QString()).toString());
	lineEditFont->setText(config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFont f;
	f.fromString(lineEditFont->text());
	lineEditFont->setFont(f);
	lineEditLogFont->setText(config->value(QMC2_FRONTEND_PREFIX + "GUI/LogFont").toString());
	f.fromString(lineEditLogFont->text());
	lineEditLogFont->setFont(f);
	int pixmapCacheSize = config->value(QMC2_FRONTEND_PREFIX + "GUI/PixmapCacheSize", 64).toInt();
	spinBoxPixmapCacheSize->setValue(pixmapCacheSize);
	checkBoxMinimizeOnEmuLaunch->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/MinimizeOnEmuLaunch", false).toBool());
	checkBoxKillEmulatorsOnExit->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/KillEmulatorsOnExit", true).toBool());
	checkBoxOneEmulatorOnly->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/OneEmulatorOnly", false).toBool());
	checkBoxShowMenuBar->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/ShowMenuBar", true).toBool());
	checkBoxCheckSingleInstance->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/CheckSingleInstance", true).toBool());
	qmc2SuppressQtMessages = config->value(QMC2_FRONTEND_PREFIX + "GUI/SuppressQtMessages", false).toBool();
	checkBoxSuppressQtMessages->setChecked(qmc2SuppressQtMessages);
	checkBoxShowSplashScreen->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/ShowSplashScreen", true).toBool());
	checkBoxShowLoadingAnimation->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/ShowLoadingAnimation", true).toBool());
	checkBoxSetWorkDirFromExec->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/SetWorkDirFromExec", false).toBool());
	if ( checkBoxSetWorkDirFromExec->isChecked() )
		QDir::setCurrent(QCoreApplication::applicationDirPath());
	else
		QDir::setCurrent(standardWorkDir());
	checkBoxMachineStatusIndicator->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/MachineStatusIndicator", true).toBool());
	checkBoxMachineStatusIndicatorOnlyWhenRequired->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/MachineStatusIndicatorOnlyWhenRequired", false).toBool());
	checkBoxShowMachineName->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/ShowMachineName", true).toBool());
	qmc2ShowMachineName = checkBoxShowMachineName->isChecked();
	checkBoxShowMachineNameOnlyWhenRequired->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/ShowMachineNameOnlyWhenRequired", false).toBool());
	qmc2ShowMachineNameOnlyWhenRequired = checkBoxShowMachineNameOnlyWhenRequired->isChecked();
	spinBoxFrontendLogSize->setValue(config->value(QMC2_FRONTEND_PREFIX + "GUI/FrontendLogSize", 0).toInt());
	spinBoxEmulatorLogSize->setValue(config->value(QMC2_FRONTEND_PREFIX + "GUI/EmulatorLogSize", 0).toInt());
#if defined(QMC2_OS_MAC)
	checkBoxNativeFileDialogs->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/NativeFileDialogs", true).toBool());
#else
	checkBoxNativeFileDialogs->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/NativeFileDialogs", false).toBool());
#endif

	// Files / Directories
	int curIdx;
#if defined(QMC2_YOUTUBE_ENABLED)
	QDir youTubeCacheDir(config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/CacheDirectory", userScopePath + "/youtube/").toString());
	if ( !youTubeCacheDir.exists() )
		youTubeCacheDir.mkdir(youTubeCacheDir.absolutePath());
	config->setValue(QMC2_FRONTEND_PREFIX + "YouTubeWidget/CacheDirectory", QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/CacheDirectory", userScopePath + "/youtube/").toString());
#endif
	lineEditDataDirectory->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/DataDirectory", QMC2_DEFAULT_DATA_PATH + "/").toString());
	lineEditDatInfoDatabase->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/DatInfoDatabase", QString(userScopePath + "/%1-dat-info.db").arg(QMC2_EMU_NAME.toLower())).toString());
#if defined(QMC2_SDLMAME)
	lineEditTemporaryFile->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmame.tmp").toString());
	lineEditFrontendLogFile->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/LogFile", userScopePath + "/qmc2-sdlmame.log").toString());
#elif defined(QMC2_MAME)
	lineEditTemporaryFile->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mame.tmp").toString());
	lineEditFrontendLogFile->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/LogFile", userScopePath + "/qmc2-mame.log").toString());
#endif
	lineEditPreviewDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/PreviewDirectory", QMC2_DEFAULT_DATA_PATH + "/prv/").toString());
	lineEditPreviewFile->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/PreviewFile", QMC2_DEFAULT_DATA_PATH + "/prv/previews.zip").toString());
	curIdx = QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/PreviewFileType", QMC2_IMG_FILETYPE_ZIP).toInt();
#if defined(QMC2_LIBARCHIVE_ENABLED)
	if ( curIdx < 0 || curIdx > 2 ) curIdx = 0;
#else
	if ( curIdx < 0 || curIdx > 1 ) curIdx = 0;
#endif
	comboBoxPreviewFileType->setCurrentIndex(curIdx);
	qmc2UsePreviewFile = config->value("MAME/FilesAndDirectories/UsePreviewFile", false).toBool();
	stackedWidgetPreview->setCurrentIndex(qmc2UsePreviewFile ? 1 : 0);
	radioButtonPreviewSelect->setText(qmc2UsePreviewFile ? tr("Preview file") : tr("Preview directory"));
	lineEditFlyerDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/FlyerDirectory", QMC2_DEFAULT_DATA_PATH + "/fly/").toString());
	lineEditFlyerFile->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/FlyerFile", QMC2_DEFAULT_DATA_PATH + "/fly/flyers.zip").toString());
	curIdx = QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/FlyerFileType", QMC2_IMG_FILETYPE_ZIP).toInt();
#if defined(QMC2_LIBARCHIVE_ENABLED)
	if ( curIdx < 0 || curIdx > 2 ) curIdx = 0;
#else
	if ( curIdx < 0 || curIdx > 1 ) curIdx = 0;
#endif
	comboBoxFlyerFileType->setCurrentIndex(curIdx);
	qmc2UseFlyerFile = config->value("MAME/FilesAndDirectories/UseFlyerFile", false).toBool();
	stackedWidgetFlyer->setCurrentIndex(qmc2UseFlyerFile ? 1 : 0);
	radioButtonFlyerSelect->setText(qmc2UseFlyerFile ? tr("Flyer file") : tr("Flyer directory"));
	lineEditIconDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/IconDirectory", QMC2_DEFAULT_DATA_PATH + "/ico/").toString());
	lineEditIconFile->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/IconFile", QMC2_DEFAULT_DATA_PATH + "/ico/icons.zip").toString());
	curIdx = QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/IconFileType", QMC2_IMG_FILETYPE_ZIP).toInt();
#if defined(QMC2_LIBARCHIVE_ENABLED)
	if ( curIdx < 0 || curIdx > 2 ) curIdx = 0;
#else
	if ( curIdx < 0 || curIdx > 1 ) curIdx = 0;
#endif
	comboBoxIconFileType->setCurrentIndex(curIdx);
	qmc2UseIconFile = config->value("MAME/FilesAndDirectories/UseIconFile", false).toBool();
	stackedWidgetIcon->setCurrentIndex(qmc2UseIconFile ? 1 : 0);
	radioButtonIconSelect->setText(qmc2UseIconFile ? tr("Icon file") : tr("Icon directory"));
	lineEditCabinetDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/CabinetDirectory", QMC2_DEFAULT_DATA_PATH + "/cab/").toString());
	lineEditCabinetFile->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/CabinetFile", QMC2_DEFAULT_DATA_PATH + "/cab/cabinets.zip").toString());
	curIdx = QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/CabinetFileType", QMC2_IMG_FILETYPE_ZIP).toInt();
#if defined(QMC2_LIBARCHIVE_ENABLED)
	if ( curIdx < 0 || curIdx > 2 ) curIdx = 0;
#else
	if ( curIdx < 0 || curIdx > 1 ) curIdx = 0;
#endif
	comboBoxCabinetFileType->setCurrentIndex(curIdx);
	qmc2UseCabinetFile = config->value("MAME/FilesAndDirectories/UseCabinetFile", false).toBool();
	stackedWidgetCabinet->setCurrentIndex(qmc2UseCabinetFile ? 1 : 0);
	radioButtonCabinetSelect->setText(qmc2UseCabinetFile ? tr("Cabinet file") : tr("Cabinet directory"));
	lineEditControllerDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/ControllerDirectory", QMC2_DEFAULT_DATA_PATH + "/ctl/").toString());
	lineEditControllerFile->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/ControllerFile", QMC2_DEFAULT_DATA_PATH + "/ctl/controllers.zip").toString());
	curIdx = QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/ControllerFileType", QMC2_IMG_FILETYPE_ZIP).toInt();
#if defined(QMC2_LIBARCHIVE_ENABLED)
	if ( curIdx < 0 || curIdx > 2 ) curIdx = 0;
#else
	if ( curIdx < 0 || curIdx > 1 ) curIdx = 0;
#endif
	comboBoxControllerFileType->setCurrentIndex(curIdx);
	qmc2UseControllerFile = config->value("MAME/FilesAndDirectories/UseControllerFile", false).toBool();
	stackedWidgetController->setCurrentIndex(qmc2UseControllerFile ? 1 : 0);
	radioButtonControllerSelect->setText(qmc2UseControllerFile ? tr("Controller file") : tr("Controller directory"));
	lineEditMarqueeDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/MarqueeDirectory", QMC2_DEFAULT_DATA_PATH + "/mrq/").toString());
	lineEditMarqueeFile->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/MarqueeFile", QMC2_DEFAULT_DATA_PATH + "/mrq/marquees.zip").toString());
	curIdx = QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/MarqueeFileType", QMC2_IMG_FILETYPE_ZIP).toInt();
#if defined(QMC2_LIBARCHIVE_ENABLED)
	if ( curIdx < 0 || curIdx > 2 ) curIdx = 0;
#else
	if ( curIdx < 0 || curIdx > 1 ) curIdx = 0;
#endif
	comboBoxMarqueeFileType->setCurrentIndex(curIdx);
	qmc2UseMarqueeFile = config->value("MAME/FilesAndDirectories/UseMarqueeFile", false).toBool();
	stackedWidgetMarquee->setCurrentIndex(qmc2UseMarqueeFile ? 1 : 0);
	radioButtonMarqueeSelect->setText(qmc2UseMarqueeFile ? tr("Marquee file") : tr("Marquee directory"));
	lineEditTitleDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/TitleDirectory", QMC2_DEFAULT_DATA_PATH + "/ttl/").toString());
	lineEditTitleFile->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/TitleFile", QMC2_DEFAULT_DATA_PATH + "/ttl/titles.zip").toString());
	curIdx = QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/TitleFileType", QMC2_IMG_FILETYPE_ZIP).toInt();
#if defined(QMC2_LIBARCHIVE_ENABLED)
	if ( curIdx < 0 || curIdx > 2 ) curIdx = 0;
#else
	if ( curIdx < 0 || curIdx > 1 ) curIdx = 0;
#endif
	comboBoxTitleFileType->setCurrentIndex(curIdx);
	qmc2UseTitleFile = config->value("MAME/FilesAndDirectories/UseTitleFile", false).toBool();
	stackedWidgetTitle->setCurrentIndex(qmc2UseTitleFile ? 1 : 0);
	radioButtonTitleSelect->setText(qmc2UseTitleFile ? tr("Title file") : tr("Title directory"));
	lineEditPCBDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/PCBDirectory", QMC2_DEFAULT_DATA_PATH + "/pcb/").toString());
	lineEditPCBFile->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/PCBFile", QMC2_DEFAULT_DATA_PATH + "/pcb/pcbs.zip").toString());
	curIdx = QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/PCBFileType", QMC2_IMG_FILETYPE_ZIP).toInt();
#if defined(QMC2_LIBARCHIVE_ENABLED)
	if ( curIdx < 0 || curIdx > 2 ) curIdx = 0;
#else
	if ( curIdx < 0 || curIdx > 1 ) curIdx = 0;
#endif
	comboBoxPCBFileType->setCurrentIndex(curIdx);
	qmc2UsePCBFile = config->value("MAME/FilesAndDirectories/UsePCBFile", false).toBool();
	stackedWidgetPCB->setCurrentIndex(qmc2UsePCBFile ? 1 : 0);
	radioButtonPCBSelect->setText(qmc2UsePCBFile ? tr("PCB file") : tr("PCB directory"));
	lineEditSoftwareSnapDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/SoftwareSnapDirectory", QMC2_DEFAULT_DATA_PATH + "/sws/").toString());
	lineEditSoftwareSnapFile->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/SoftwareSnapFile", QMC2_DEFAULT_DATA_PATH + "/sws/swsnaps.zip").toString());
	curIdx = QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/SoftwareSnapFileType", QMC2_IMG_FILETYPE_ZIP).toInt();
#if defined(QMC2_LIBARCHIVE_ENABLED)
	if ( curIdx < 0 || curIdx > 2 ) curIdx = 0;
#else
	if ( curIdx < 0 || curIdx > 1 ) curIdx = 0;
#endif
	comboBoxSoftwareSnapFileType->setCurrentIndex(curIdx);
	qmc2UseSoftwareSnapFile = config->value("MAME/FilesAndDirectories/UseSoftwareSnapFile", false).toBool();
	stackedWidgetSWSnap->setCurrentIndex(qmc2UseSoftwareSnapFile ? 1 : 0);
	radioButtonSoftwareSnapSelect->setText(qmc2UseSoftwareSnapFile ? tr("SW snap file") : tr("SW snap folder"));
	lineEditSoftwareNotesFolder->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/SoftwareNotesFolder", QMC2_DEFAULT_DATA_PATH + "/swn/").toString());
	lineEditSoftwareNotesTemplate->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/SoftwareNotesTemplate", QMC2_DEFAULT_DATA_PATH + "/swn/template.html").toString());
	checkBoxUseSoftwareNotesTemplate->setChecked(config->value("MAME/FilesAndDirectories/UseSoftwareNotesTemplate", false).toBool());
	lineEditSystemNotesFolder->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/SystemNotesFolder", QMC2_DEFAULT_DATA_PATH + "/gmn/").toString());
	lineEditSystemNotesTemplate->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/SystemNotesTemplate", QMC2_DEFAULT_DATA_PATH + "/gmn/template.html").toString());
	lineEditVideoSnapFolder->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/VideoSnapFolder", QMC2_DEFAULT_DATA_PATH + "/vdo/").toString());
	checkBoxUseSystemNotesTemplate->setChecked(config->value("MAME/FilesAndDirectories/UseSystemNotesTemplate", false).toBool());
	lineEditMameHistoryDat->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MameHistoryDat", QMC2_DEFAULT_DATA_PATH + "/cat/history.dat").toString());
	lineEditMessSysinfoDat->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MessSysinfoDat", QMC2_DEFAULT_DATA_PATH + "/cat/sysinfo.dat").toString());
	lineEditMameInfoDat->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MameInfoDat", QMC2_DEFAULT_DATA_PATH + "/cat/mameinfo.dat").toString());
	lineEditMessInfoDat->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MessInfoDat", QMC2_DEFAULT_DATA_PATH + "/cat/messinfo.dat").toString());
	lineEditCatverIniFile->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/CatverIni", QMC2_DEFAULT_DATA_PATH + "/cat/catver.ini").toString());
	checkBoxUseCatverIni->setChecked(config->value(QMC2_FRONTEND_PREFIX + "MachineList/UseCatverIni", false).toBool());
	lineEditCategoryIniFile->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/CategoryIni", QMC2_DEFAULT_DATA_PATH + "/cat/category.ini").toString());
	checkBoxUseCategoryIni->setChecked(config->value(QMC2_FRONTEND_PREFIX + "MachineList/UseCategoryIni", false).toBool());
	lineEditSoftwareInfoDB->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareInfoDB", QMC2_DEFAULT_DATA_PATH + "/cat/history.dat").toString());

	// MachineList
	checkBoxShowROMStatusIcons->setChecked(config->value(QMC2_FRONTEND_PREFIX + "MachineList/ShowROMStatusIcons", true).toBool());
	checkBoxShowDeviceSets->setChecked(config->value(QMC2_FRONTEND_PREFIX + "MachineList/ShowDeviceSets", true).toBool());
	checkBoxShowBiosSets->setChecked(config->value(QMC2_FRONTEND_PREFIX + "MachineList/ShowBiosSets", true).toBool());
	checkBoxAutoTriggerROMCheck->setChecked(config->value(QMC2_FRONTEND_PREFIX + "MachineList/AutoTriggerROMCheck", false).toBool());
	checkBoxDoubleClickActivation->setChecked(config->value(QMC2_FRONTEND_PREFIX + "MachineList/DoubleClickActivation", true).toBool());
	checkBoxPlayOnSublistActivation->setChecked(config->value(QMC2_FRONTEND_PREFIX + "MachineList/PlayOnSublistActivation", false).toBool());
	qmc2CursorPositioningMode = (QAbstractItemView::ScrollHint)config->value(QMC2_FRONTEND_PREFIX + "MachineList/CursorPosition", QMC2_CURSOR_POS_TOP).toInt();
	comboBoxCursorPosition->setCurrentIndex((int)qmc2CursorPositioningMode);
	qmc2DefaultLaunchMode = config->value(QMC2_FRONTEND_PREFIX + "MachineList/DefaultLaunchMode", QMC2_LAUNCH_MODE_INDEPENDENT).toInt();
	comboBoxDefaultLaunchMode->setCurrentIndex(qmc2DefaultLaunchMode);
	qmc2SoftwareSnapPosition = config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SoftwareSnapPosition", QMC2_SWSNAP_POS_BELOW_LEFT).toInt();
	comboBoxSoftwareSnapPosition->setCurrentIndex(qmc2SoftwareSnapPosition);
	checkBoxSoftwareSnapOnMouseHover->setChecked(config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SoftwareSnapOnMouseHover", false).toBool());
	checkBoxAutoDisableSoftwareSnap->setChecked(config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/AutoDisableSoftwareSnap", true).toBool());
	spinBoxResponsiveness->setValue(config->value(QMC2_FRONTEND_PREFIX + "MachineList/Responsiveness", QMC2_DEFAULT_MACHINELIST_RESPONSE).toInt());
	qmc2MachineListResponsiveness = spinBoxResponsiveness->value();
	spinBoxUpdateDelay->setValue(config->value(QMC2_FRONTEND_PREFIX + "MachineList/UpdateDelay", 10).toInt());
	qmc2UpdateDelay = spinBoxUpdateDelay->value();
	comboBoxSortCriteria->setCurrentIndex(config->value(QMC2_FRONTEND_PREFIX + "MachineList/SortCriteria", 0).toInt());
	qmc2SortCriteria = comboBoxSortCriteria->currentIndex();
	comboBoxSortOrder->setCurrentIndex(config->value(QMC2_FRONTEND_PREFIX + "MachineList/SortOrder", 0).toInt());
	qmc2SortOrder = comboBoxSortOrder->currentIndex() == 0 ? Qt::AscendingOrder : Qt::DescendingOrder;
	qmc2Filter.setBit(QMC2_ROMSTATE_INT_C, config->value(QMC2_FRONTEND_PREFIX + "RomStateFilter/ShowCorrect", true).toBool());
	toolButtonShowC->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_C]);
	qmc2Filter.setBit(QMC2_ROMSTATE_INT_M, config->value(QMC2_FRONTEND_PREFIX + "RomStateFilter/ShowMostlyCorrect", true).toBool());
	toolButtonShowM->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_M]);
	qmc2Filter.setBit(QMC2_ROMSTATE_INT_I, config->value(QMC2_FRONTEND_PREFIX + "RomStateFilter/ShowIncorrect", true).toBool());
	toolButtonShowI->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_I]);
	qmc2Filter.setBit(QMC2_ROMSTATE_INT_N, config->value(QMC2_FRONTEND_PREFIX + "RomStateFilter/ShowNotFound", true).toBool());
	toolButtonShowN->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_N]);
	qmc2Filter.setBit(QMC2_ROMSTATE_INT_U, config->value(QMC2_FRONTEND_PREFIX + "RomStateFilter/ShowUnknown", true).toBool());
	toolButtonShowU->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_U]);
	bool rsf = config->value(QMC2_FRONTEND_PREFIX + "RomStateFilter/Enabled", true).toBool();
	checkBoxRomStateFilter->setChecked(rsf);
	if ( !qmc2EarlyStartup ) {
		qmc2MainWindow->toolButtonSelectRomFilter->setVisible(rsf);
		qmc2MainWindow->romStateFilter->setEnabled(rsf);
	}
	if ( qmc2MainWindow ) {
		qmc2StatesTogglesEnabled = false;
		qmc2MainWindow->romStateFilter->toolButtonCorrect->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_C]);
		qmc2MainWindow->romStateFilter->toolButtonMostlyCorrect->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_M]);
		qmc2MainWindow->romStateFilter->toolButtonIncorrect->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_I]);
		qmc2MainWindow->romStateFilter->toolButtonNotFound->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_N]);
		qmc2MainWindow->romStateFilter->toolButtonUnknown->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_U]);
		if ( !qmc2EarlyStartup )
			qmc2StatesTogglesEnabled = true;
	}

	// Shortcuts / Keys
	QHashIterator<QString, QPair<QString, QAction *> > it(qmc2ShortcutHash);
	while ( it.hasNext() ) {
		it.next();
		QString itShortcut = it.key();
		QString itFunction = it.value().first;
		QTreeWidgetItem *item = new QTreeWidgetItem(treeWidgetShortcuts);
		item->setText(0, itFunction);
		QStringList words = itShortcut.split("+");
		QString itemText;
		for (int i = 0; i < words.count(); i++) {
			if ( i > 0 )
				itemText += "+";
			itemText += QObject::tr(words[i].toUtf8().constData());
		}
		item->setText(1, itemText);
		QString customSC = config->value(QString(QMC2_FRONTEND_PREFIX + "Shortcuts/%1").arg(itShortcut), itShortcut).toString();
		qmc2CustomShortcutHash[itShortcut] = customSC;
		if ( customSC != itShortcut ) {
			words = customSC.split("+");
			customSC = "";
			for (int i = 0; i < words.count(); i++) {
				if ( i > 0 )
					customSC += "+";
				customSC += QObject::tr(words[i].toUtf8().constData());
			}
			item->setText(2, customSC);
		}
	}
	if ( !qmc2EarlyStartup )
		QTimer::singleShot(0, this, SLOT(checkShortcuts()));

	// Joystick
#if QMC2_JOYSTICK == 1
	checkBoxEnableJoystickControl->setChecked(config->value(QMC2_FRONTEND_PREFIX + "Joystick/EnableJoystickControl", false).toBool());
	checkBoxJoystickAutoRepeat->setChecked(config->value(QMC2_FRONTEND_PREFIX + "Joystick/AutoRepeat", true).toBool());
	spinBoxJoystickAutoRepeatTimeout->setValue(config->value(QMC2_FRONTEND_PREFIX + "Joystick/AutoRepeatTimeout", 250).toInt());
	spinBoxJoystickEventTimeout->setValue(config->value(QMC2_FRONTEND_PREFIX + "Joystick/EventTimeout", 25).toInt());
	on_pushButtonRescanJoysticks_clicked();

	// Recreate joystick function map
	treeWidgetJoystickMappings->clear();
	it.toFront();
	while ( it.hasNext() ) {
		it.next();
		QString itShortcut = it.key();
		QString itFunction = it.value().first;
		QTreeWidgetItem *item = new QTreeWidgetItem(treeWidgetJoystickMappings);
		item->setText(0, itFunction);
		item->setWhatsThis(0, itShortcut);
		QString joyMapFunction = config->value(QString(QMC2_FRONTEND_PREFIX + "Joystick/Map/%1").arg(itShortcut), "").toString();
		if ( !joyMapFunction.isEmpty() ) {
			qmc2JoystickFunctionHash.insertMulti(joyMapFunction, itShortcut);
			item->setText(1, joyMapFunction);
		}
	}
	if ( !qmc2EarlyStartup )
		QTimer::singleShot(0, this, SLOT(checkJoystickMappings()));
#endif

	// Network / Tools
	checkBoxRestoreCookies->setChecked(config->value(QMC2_FRONTEND_PREFIX + "WebBrowser/RestoreCookies", true).toBool());
	lineEditCookieDatabase->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "WebBrowser/CookieDatabase", userScopePath + "/qmc2-" + QMC2_EMU_NAME_VARIANT.toLower() + "-cookies.db").toString());
	lineEditZipTool->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "Tools/ZipTool", "zip").toString());
	lineEditZipToolRemovalArguments->setText(config->value(QMC2_FRONTEND_PREFIX + "Tools/ZipToolRemovalArguments", "$ARCHIVE$ -d $FILELIST$").toString());
	lineEditSevenZipTool->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "Tools/SevenZipTool", "7za").toString());
	lineEditSevenZipToolRemovalArguments->setText(config->value(QMC2_FRONTEND_PREFIX + "Tools/SevenZipToolRemovalArguments", "-mhc=off -ms=off d $ARCHIVE$ $FILELIST$").toString());
	lineEditRomTool->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "Tools/RomTool", "").toString());
	lineEditRomToolArguments->setText(config->value(QMC2_FRONTEND_PREFIX + "Tools/RomToolArguments", "$ID$ \"$DESCRIPTION$\"").toString());
	lineEditRomToolWorkingDirectory->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "Tools/RomToolWorkingDirectory", "").toString());
	checkBoxCopyToolOutput->setChecked(config->value(QMC2_FRONTEND_PREFIX + "Tools/CopyToolOutput", true).toBool());
	checkBoxCloseToolDialog->setChecked(config->value(QMC2_FRONTEND_PREFIX + "Tools/CloseToolDialog", false).toBool());

	groupBoxHTTPProxy->setChecked(config->value("Network/HTTPProxy/Enable", false).toBool());
	lineEditHTTPProxyHost->setText(config->value("Network/HTTPProxy/Host", QString()).toString());
	spinBoxHTTPProxyPort->setValue(config->value("Network/HTTPProxy/Port", 80).toInt());
	lineEditHTTPProxyUserID->setText(config->value("Network/HTTPProxy/UserID", QString()).toString());
	CryptedByteArray cpw(config->value("Network/HTTPProxy/Password", QString()).toByteArray());
	lineEditHTTPProxyPassword->setText(QString(cpw.decryptedData()));

	// Emulator

	// Configuration
	if ( qmc2GuiReady )
		qmc2GlobalEmulatorOptions->load();

	// Files and directories
	lineEditExecutableFile->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ExecutableFile", "").toString());
	lineEditWorkingDirectory->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory", "").toString());
	lineEditEmulatorLogFile->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/LogFile", userScopePath + "/mame.log").toString());
	lineEditXmlCacheDatabase->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/XmlCacheDatabase", userScopePath + "/mame-xml-cache.db").toString());
	lineEditUserDataDatabase->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/UserDataDatabase", userScopePath + "/mame-user-data.db").toString());
	lineEditMachineListCacheFile->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/MachineListCacheFile", userScopePath + "/mame.mlc").toString());
	lineEditMachineListDatabase->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/MachineListDatabase", userScopePath + "/mame-machine-list.db").toString());
	lineEditROMStateCacheFile->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ROMStateCacheFile", userScopePath + "/mame.rsc").toString());
	lineEditSlotInfoCacheFile->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SlotInfoCacheFile", userScopePath + "/mame.sic").toString());
	lineEditSoftwareListCacheDb->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareListCacheDatabase", userScopePath + "/mame-swl-cache.db").toString());
#if defined(QMC2_SDLMAME)
	lineEditOptionsTemplateFile->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/OptionsTemplateFile", QMC2_DEFAULT_DATA_PATH + "/opt/SDLMAME/template-SDL2.xml").toString());
#elif defined(QMC2_MAME)
	lineEditOptionsTemplateFile->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/OptionsTemplateFile", QMC2_DEFAULT_DATA_PATH + "/opt/MAME/template.xml").toString());
#endif
	lineEditFavoritesFile->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/FavoritesFile", userScopePath + "/mame.fav").toString());
	lineEditHistoryFile->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/HistoryFile", userScopePath + "/mame.hst").toString());
	QDir swStateCacheDir(config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareStateCache", userScopePath + "/sw-state-cache/").toString());
	if ( !swStateCacheDir.exists() )
		swStateCacheDir.mkdir(swStateCacheDir.absolutePath());
	lineEditSoftwareStateCache->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareStateCache", userScopePath + "/sw-state-cache/").toString());
	lineEditGeneralSoftwareFolder->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/GeneralSoftwareFolder", QString()).toString());
	checkBoxAutoClearEmuCaches->setChecked(QMC2_QSETTINGS_CAST(config)->value(QMC2_EMULATOR_PREFIX + "AutoClearEmuCaches", true).toBool());
	checkBoxSkipEmuIdent->setChecked(QMC2_QSETTINGS_CAST(config)->value(QMC2_EMULATOR_PREFIX + "SkipEmuIdent", true).toBool());

	// Additional emulators
	tableWidgetRegisteredEmulators->clearContents();
	tableWidgetRegisteredEmulators->setRowCount(0);

	config->beginGroup(QMC2_EMULATOR_PREFIX + "RegisteredEmulators");
	QStringList additionalEmulators = config->childGroups();
	tableWidgetRegisteredEmulators->setSortingEnabled(false);
	QFontMetrics fm(qApp->font());
	QSize iconSize(fm.height(), fm.height());
	foreach (QString emuName, additionalEmulators) {
		QString emuCommand = config->value(QString("%1/Executable").arg(emuName), QString()).toString();
		QString emuWorkDir = config->value(QString("%1/WorkingDirectory").arg(emuName), QString()).toString();
		QString emuArgs = config->value(QString("%1/Arguments").arg(emuName), QString()).toString();
		QString emuIcon = config->value(QString("%1/Icon").arg(emuName), QString()).toString();
		int row = tableWidgetRegisteredEmulators->rowCount();
		tableWidgetRegisteredEmulators->insertRow(row);
		tableWidgetRegisteredEmulators->setItem(row, QMC2_ADDTLEMUS_COLUMN_NAME, new QTableWidgetItem(emuName));
		tableWidgetRegisteredEmulators->setItem(row, QMC2_ADDTLEMUS_COLUMN_EXEC, new QTableWidgetItem(emuCommand));
		tableWidgetRegisteredEmulators->setItem(row, QMC2_ADDTLEMUS_COLUMN_WDIR, new QTableWidgetItem(emuWorkDir));
		tableWidgetRegisteredEmulators->setItem(row, QMC2_ADDTLEMUS_COLUMN_ARGS, new QTableWidgetItem(emuArgs));
		QToolButton *tb = new QToolButton(0);
		tb->setObjectName(emuName);
		tb->setAutoFillBackground(true);
		tb->setText(tr("Custom IDs..."));
		tb->setToolTip(tr("Specify pre-defined foreign IDs for this emulator, launchable from the 'foreign emulators' view"));
		tableWidgetRegisteredEmulators->setCellWidget(row, QMC2_ADDTLEMUS_COLUMN_CUID, tb);
		connect(tb, SIGNAL(clicked()), this, SLOT(setupCustomIDsClicked()));
		tb = new QToolButton(0);
		tb->setAutoFillBackground(true);
		tb->setIconSize(iconSize);
		if ( emuIcon.isEmpty() ) {
			tb->setIcon(QIcon(QString::fromUtf8(":/data/img/alien.png")));
			tb->setWhatsThis(":/data/img/alien.png");
		} else {
			if ( emuIcon == "[none]" ) {
				tb->setIcon(QIcon());
				tb->setWhatsThis("[none]");
			} else {
				tb->setIcon(QIcon(emuIcon));
				tb->setWhatsThis(emuIcon);
			}
		}
		tb->setToolTip(tr("Choose icon for this foreign emulator (hold down for menu)"));
		QMenu *menu = new QMenu(tb);
		QAction *action = menu->addAction(QIcon(QString::fromUtf8(":/data/img/alien.png")), tr("Default icon"));
		connect(action, SIGNAL(triggered(bool)), this, SLOT(actionDefaultEmuIconTriggered()));
		menu->addSeparator();
		action = menu->addAction(QIcon(QString::fromUtf8(":/data/img/no.png")), tr("No icon"));
		connect(action, SIGNAL(triggered(bool)), this, SLOT(actionNoEmuIconTriggered()));
		tb->setMenu(menu);
		tableWidgetRegisteredEmulators->setCellWidget(row, QMC2_ADDTLEMUS_COLUMN_ICON, tb);
		connect(tb, SIGNAL(clicked()), this, SLOT(chooseEmuIconClicked()));
	}
	config->endGroup();
	tableWidgetRegisteredEmulators->setSortingEnabled(true);

	// remove custom ID lists, if applicable
	foreach (QString emuName, registeredEmulatorsToBeRemoved)
		config->remove(QString(QMC2_EMULATOR_PREFIX + "CustomIDs/%1").arg(emuName));
	registeredEmulatorsToBeRemoved.clear();

	if ( useDefaultSettings ) {
		QString fn = config->fileName();
		delete config;
		QFile f0(fn);
		f0.remove();
		QFile f(fn + ".bak");
		f.copy(fn);
		f.remove();
		config = new Settings(QSettings::IniFormat, QSettings::UserScope, "qmc2");
		qmc2Config = config;
	}

	QTimer::singleShot(0, this, SLOT(applyDelayed()));
}

QString Options::configPath()
{
	QDir cd(QMC2_DYNAMIC_DOT_PATH);
	cd.makeAbsolute();
	return cd.absolutePath();
}

int Options::iconFileType()
{
	if ( qmc2UseIconFile )
		return config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/IconFileType", QMC2_IMG_FILETYPE_ZIP).toInt();
	else
		return QMC2_ICON_FILETYPE_NONE;
}

void Options::applyDelayed()
{
	// paranoia :)
	if ( !qmc2MainWindow ) {
		QTimer::singleShot(0, this, SLOT(applyDelayed()));
		return;
	}

	static bool firstTime = true;

	if ( firstTime ) {
#if defined(QMC2_OS_WIN)
		setParent(qmc2MainWindow, Qt::Dialog);
#else
		setParent(qmc2MainWindow, Qt::Dialog | Qt::SubWindow);
#endif
		// restore layout
		tabWidgetFrontendSettings->setCurrentIndex(config->value(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/FrontendTab", 0).toInt());
		tabWidgetGlobalMAMESetup->setCurrentIndex(config->value(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/MAMETab", 0).toInt());
		tabWidgetOptions->setCurrentIndex(config->value(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/OptionsTab", 0).toInt());
		QStringList cl(config->allKeys());
		if ( cl.contains(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/Size") )
			resize(config->value(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/Size").toSize());
		if ( cl.contains(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/Position") )
			move(config->value(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/Position").toPoint());
		treeWidgetShortcuts->header()->restoreState(config->value(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/ShortcutsHeaderState").toByteArray());
#if QT_VERSION < 0x050000
		treeWidgetShortcuts->header()->setClickable(true);
#else
		treeWidgetShortcuts->header()->setSectionsClickable(true);
#endif
		treeWidgetShortcuts->header()->setSortIndicatorShown(true);
#if QMC2_JOYSTICK == 1
		treeWidgetJoystickMappings->header()->restoreState(config->value(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/JoyMapHeaderState").toByteArray());
#if QT_VERSION < 0x050000
		treeWidgetJoystickMappings->header()->setClickable(true);
#else
		treeWidgetJoystickMappings->header()->setSectionsClickable(true);
#endif
		treeWidgetJoystickMappings->header()->setSortIndicatorShown(true);
#endif
		tableWidgetRegisteredEmulators->horizontalHeader()->restoreState(config->value(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/RegisteredEmulatorsHeaderState").toByteArray());
		firstTime = false;
	}

	// adjust row-sizes of foreign emulator table-widget items
	tableWidgetRegisteredEmulators->resizeRowsToContents();

	// redraw detail if setup changed
	qmc2MainWindow->on_tabWidgetMachineDetail_currentChanged(qmc2MainWindow->tabWidgetMachineDetail->currentIndex());

	if ( !cancelClicked ) {
		if ( qmc2GuiReady ) {
			// style
			if ( qmc2StandardPalettes.contains(currentStyleName()) )
				qApp->setPalette(qmc2StandardPalettes[currentStyleName()]);
			QString styleName(comboBoxStyle->currentText());
			if ( styleName == tr("Default") )
				styleName = "Default";
			config->setValue(QMC2_FRONTEND_PREFIX + "GUI/Style", styleName);
			qmc2MainWindow->signalStyleSetupRequested(styleName);
			// style sheet
			QString styleSheetName(lineEditStyleSheet->text());
			config->setValue(QMC2_FRONTEND_PREFIX + "GUI/StyleSheet", styleSheetName);
			qmc2MainWindow->signalStyleSheetSetupRequested(styleSheetName);
			// palette
			qmc2MainWindow->signalPaletteSetupRequested(styleName);
		}
		qmc2MainWindow->treeWidgetForeignIDs->setUpdatesEnabled(false);
		if ( !qmc2EarlyStartup ) {
			// save foreign ID selection
			QTreeWidgetItem *foreignItem = qmc2MainWindow->treeWidgetForeignIDs->currentItem();
			if ( foreignItem && foreignItem->isSelected() ) {
				QTreeWidgetItem *parentItem = foreignItem;
				if ( foreignItem->parent() )
					parentItem = foreignItem->parent();
				QStringList foreignIdState;
				if ( parentItem == foreignItem )
					foreignIdState << QString::number(qmc2MainWindow->treeWidgetForeignIDs->indexOfTopLevelItem(parentItem));
				else
					foreignIdState << QString::number(qmc2MainWindow->treeWidgetForeignIDs->indexOfTopLevelItem(parentItem)) << QString::number(parentItem->indexOfChild(foreignItem));
				qmc2Config->setValue(QMC2_EMULATOR_PREFIX + "SelectedForeignID", foreignIdState);
			} else
				qmc2Config->remove(QMC2_EMULATOR_PREFIX + "SelectedForeignID");
		}
		// (re)create foreign IDs tree-widget, if applicable
		qmc2MainWindow->treeWidgetForeignIDs->clear();
		QString displayFormat(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/CustomIDSetup/DisplayFormat", "$ID$ - $DESCRIPTION$").toString());
		config->beginGroup(QMC2_EMULATOR_PREFIX + "RegisteredEmulators");
		QStringList registeredEmus(config->childGroups());
		config->endGroup();
		if ( !registeredEmus.isEmpty() ) {
			QList<QTreeWidgetItem *> itemList;
			foreach (QString emuName, registeredEmus) {
				QTreeWidgetItem *emuItem = new QTreeWidgetItem();
				emuItem->setText(0, emuName);
				QString emuIcon = config->value(QString(QMC2_EMULATOR_PREFIX + "RegisteredEmulators/%1/Icon").arg(emuName), QString()).toString();
				if ( emuIcon.isEmpty() )
					emuItem->setIcon(0, QIcon(QString::fromUtf8(":/data/img/alien.png")));
				else {
					QIcon icon = QIcon(emuIcon);
					if ( !icon.isNull() )
						emuItem->setIcon(0, icon);
					else
						emuItem->setIcon(0, QIcon(QString::fromUtf8(":/data/img/alien.png")));
				}
				emuItem->setWhatsThis(0, emuName + "\t" + tr("N/A") + "\t" + tr("N/A"));
				itemList << emuItem;
				QStringList idList(config->value(QMC2_EMULATOR_PREFIX + QString("CustomIDs/%1/IDs").arg(emuName), QStringList()).toStringList());
				if ( !idList.isEmpty() ) {
					QStringList descriptionList(config->value(QMC2_EMULATOR_PREFIX + QString("CustomIDs/%1/Descriptions").arg(emuName), QStringList()).toStringList());
					while ( descriptionList.count() < idList.count() )
						descriptionList << QString();
					QStringList iconList(config->value(QMC2_EMULATOR_PREFIX + QString("CustomIDs/%1/Icons").arg(emuName), QStringList()).toStringList());
					while ( iconList.count() < idList.count() )
						iconList << QString();
					for (int i = 0; i < idList.count(); i++) {
						QString id(idList.at(i));
						if ( !id.isEmpty() ) {
							QString description(descriptionList.at(i));
							QString idIcon(iconList.at(i));
							QString itemText(displayFormat);
							itemText.replace("$ID$", id).replace("$DESCRIPTION$", description);
							QTreeWidgetItem *idItem = new QTreeWidgetItem(emuItem);
							idItem->setText(0, itemText);
							idItem->setWhatsThis(0, emuName + "\t" + id + "\t" + description);
							if ( idIcon.isEmpty() )
								idItem->setIcon(0, QIcon(QString::fromUtf8(":/data/img/pacman.png")));
							else {
								QIcon icon(idIcon);
								if ( !icon.isNull() )
									idItem->setIcon(0, icon);
								else
									idItem->setIcon(0, QIcon(QString::fromUtf8(":/data/img/pacman.png")));
							}
						}
					}
				}
			}
			qmc2MainWindow->treeWidgetForeignIDs->insertTopLevelItems(0, itemList);
			ComponentInfo *componentInfo = qmc2ComponentSetup->componentInfoHash().value("Component1");
			if ( componentInfo->appliedFeatureList().contains(QMC2_FOREIGN_INDEX) ) {
				int index = qmc2MainWindow->tabWidgetMachineList->indexOf(qmc2MainWindow->tabForeignEmulators);
				int foreignIndex = componentInfo->appliedFeatureList().indexOf(QMC2_FOREIGN_INDEX);
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
				int embedIndex = componentInfo->appliedFeatureList().indexOf(QMC2_EMBED_INDEX);
				if ( embedIndex >= 0 && embedIndex < foreignIndex )
					if ( qmc2MainWindow->tabWidgetMachineList->indexOf(qmc2MainWindow->tabEmbeddedEmus) < 0 )
						foreignIndex--;
#endif
				if ( index == -1 ) {
					qmc2MainWindow->tabWidgetMachineList->insertTab(foreignIndex, qmc2MainWindow->tabForeignEmulators, tr("&Foreign emulators"));
					qmc2MainWindow->tabWidgetMachineList->setTabIcon(foreignIndex, QIcon(QString::fromUtf8(":/data/img/alien.png")));
				}
			}
		} else {
			int index = qmc2MainWindow->tabWidgetMachineList->indexOf(qmc2MainWindow->tabForeignEmulators);
			if ( index >= 0 )
				qmc2MainWindow->tabWidgetMachineList->removeTab(index);
		}
		// restore foreign ID selection
		QStringList foreignIdState(qmc2Config->value(QMC2_EMULATOR_PREFIX + "SelectedForeignID", QStringList()).toStringList());
		if ( !foreignIdState.isEmpty() ) {
			int parentIndex = foreignIdState[0].toInt();
			int childIndex = -1;
			if ( foreignIdState.count() > 1 )
				childIndex = foreignIdState[1].toInt();
			if ( parentIndex >= 0 && parentIndex < qmc2MainWindow->treeWidgetForeignIDs->topLevelItemCount() ) {
				QTreeWidgetItem *parentItem = qmc2MainWindow->treeWidgetForeignIDs->topLevelItem(parentIndex);
				if ( childIndex >= 0 && childIndex < parentItem->childCount() ) {
					parentItem->setExpanded(true);
					QTreeWidgetItem *childItem = parentItem->child(childIndex);
					childItem->setSelected(true);
					qmc2MainWindow->treeWidgetForeignIDs->setCurrentItem(childItem);
					qmc2MainWindow->treeWidgetForeignIDs->scrollToItem(childItem, qmc2CursorPositioningMode);
				} else {
					parentItem->setSelected(true);
					qmc2MainWindow->treeWidgetForeignIDs->setCurrentItem(parentItem);
					qmc2MainWindow->treeWidgetForeignIDs->scrollToItem(parentItem, qmc2CursorPositioningMode);
				}
			}
		}
		qmc2MainWindow->treeWidgetForeignIDs->setUpdatesEnabled(true);
		tableWidgetRegisteredEmulators->resizeRowsToContents();
	}
	checkPlaceholderStatus();
	// hide / show the menu bar
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
	if ( qmc2MainWindow->tabWidgetMachineList->currentIndex() != QMC2_EMBED_INDEX || !qmc2MainWindow->toolButtonEmbedderMaximizeToggle->isChecked() )
		qmc2MainWindow->menuBar()->setVisible(checkBoxShowMenuBar->isChecked());
#else
	qmc2MainWindow->menuBar()->setVisible(checkBoxShowMenuBar->isChecked());
#endif
	if ( checkBoxShowLoadingAnimation->isChecked() ) {
		qmc2MainWindow->labelLoadingMachineList->setMovie(qmc2MainWindow->loadAnimMovie);
		qmc2MainWindow->labelLoadingHierarchy->setMovie(qmc2MainWindow->loadAnimMovie);
		qmc2MainWindow->labelCreatingCategoryView->setMovie(qmc2MainWindow->loadAnimMovie);
		qmc2MainWindow->labelCreatingVersionView->setMovie(qmc2MainWindow->loadAnimMovie);
		if ( qmc2SoftwareList )
			qmc2SoftwareList->labelLoadingSoftwareLists->setMovie(qmc2SoftwareList->loadAnimMovie);
	} else {
		qmc2MainWindow->labelLoadingMachineList->setMovie(qmc2MainWindow->nullMovie);
		qmc2MainWindow->labelLoadingHierarchy->setMovie(qmc2MainWindow->nullMovie);
		qmc2MainWindow->labelCreatingCategoryView->setMovie(qmc2MainWindow->nullMovie);
		qmc2MainWindow->labelCreatingVersionView->setMovie(qmc2MainWindow->nullMovie);
		if ( qmc2SoftwareList )
			qmc2SoftwareList->labelLoadingSoftwareLists->setMovie(qmc2MainWindow->nullMovie);
	}
	cancelClicked = false;
}

void Options::on_pushButtonClearCookieDatabase_clicked()
{
	if ( qmc2NetworkAccessManager ) {
		CookieJar *cj = (CookieJar *)qmc2NetworkAccessManager->cookieJar();
		cj->recreateDatabase();
	}
}

void Options::on_pushButtonManageCookies_clicked()
{
	CookieManager cm(this);
	cm.exec();
}

void Options::on_pushButtonAdditionalArtworkSetup_clicked()
{
	AdditionalArtworkSetup as(this);
	as.exec();
}

void Options::on_pushButtonImageFormats_clicked()
{
	ImageFormatSetup ifs(this);
	ifs.exec();
}

void Options::on_toolButtonImportGameInfo_clicked()
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

	qmc2LoadingMachineInfoDB = true;
	qmc2Options->toolButtonImportGameInfo->setEnabled(false);
	qmc2Options->toolButtonImportMachineInfo->setEnabled(false);
	qApp->processEvents();
	qmc2MachineList->datInfoDb()->importMachineInfo(pathList, emulatorList);
	qmc2Options->toolButtonImportGameInfo->setEnabled(true);
	qmc2Options->toolButtonImportMachineInfo->setEnabled(true);
	qmc2LoadingMachineInfoDB = false;
}

void Options::on_toolButtonImportMachineInfo_clicked()
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

	qmc2LoadingMachineInfoDB = true;
	qmc2Options->toolButtonImportGameInfo->setEnabled(false);
	qmc2Options->toolButtonImportMachineInfo->setEnabled(false);
	qApp->processEvents();
	qmc2MachineList->datInfoDb()->importMachineInfo(pathList, emulatorList);
	qmc2Options->toolButtonImportGameInfo->setEnabled(true);
	qmc2Options->toolButtonImportMachineInfo->setEnabled(true);
	qmc2LoadingMachineInfoDB = false;
}

void Options::on_toolButtonImportMameInfo_clicked()
{
	QStringList pathList;
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMameInfoDat").toBool() )
		pathList << qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MameInfoDat").toString();
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMessInfoDat").toBool() )
		pathList << qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MessInfoDat").toString();

	qmc2LoadingEmuInfoDB = true;
	toolButtonImportMameInfo->setEnabled(false);
	toolButtonImportMessInfo->setEnabled(false);
	qApp->processEvents();
	qmc2MachineList->datInfoDb()->importEmuInfo(pathList);
	toolButtonImportMameInfo->setEnabled(true);
	toolButtonImportMessInfo->setEnabled(true);
	qmc2LoadingEmuInfoDB = false;
}

void Options::on_toolButtonImportMessInfo_clicked()
{
	QStringList pathList;
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMameInfoDat").toBool() )
		pathList << qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MameInfoDat").toString();
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMessInfoDat").toBool() )
		pathList << qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MessInfoDat").toString();

	qmc2LoadingEmuInfoDB = true;
	toolButtonImportMameInfo->setEnabled(false);
	toolButtonImportMessInfo->setEnabled(false);
	qApp->processEvents();
	qmc2MachineList->datInfoDb()->importEmuInfo(pathList);
	toolButtonImportMameInfo->setEnabled(true);
	toolButtonImportMessInfo->setEnabled(true);
	qmc2LoadingEmuInfoDB = false;
}

void Options::on_toolButtonImportSoftwareInfo_clicked()
{
	qmc2LoadingSoftwareInfoDB = true;
	toolButtonImportSoftwareInfo->setEnabled(false);
	qApp->processEvents();
	QStringList pathList = QStringList() << QMC2_QSETTINGS_CAST(config)->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareInfoDB").toString();
	qmc2MachineList->datInfoDb()->importSoftwareInfo(pathList);
	toolButtonImportSoftwareInfo->setEnabled(true);
	qmc2LoadingSoftwareInfoDB = false;
}

void Options::on_toolButtonOptimizeCatverIni_clicked()
{
	CatverIniOptimizer optimizer(lineEditCatverIniFile->text(), this);
	optimizer.exec();
}

void Options::on_toolButtonBrowseStyleSheet_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose Qt style sheet file"), lineEditStyleSheet->text(), tr("Qt Style Sheets (*.qss)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditStyleSheet->setText(s);
	raise();
}

void Options::on_toolButtonBrowseTemporaryFile_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose temporary work file"), lineEditTemporaryFile->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditTemporaryFile->setText(s);
	raise();
}

void Options::on_toolButtonBrowsePreviewDirectory_clicked()
{
	QString s = QFileDialog::getExistingDirectory(this, tr("Choose preview directory"), lineEditPreviewDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog));
	if ( !s.isNull() ) {
		if ( !s.endsWith("/") )
			s += "/";
		lineEditPreviewDirectory->setText(s);
	}
	raise();
}

void Options::on_toolButtonBrowseFlyerDirectory_clicked()
{
	QString s = QFileDialog::getExistingDirectory(this, tr("Choose flyer directory"), lineEditFlyerDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog));
	if ( !s.isNull() ) {
		if ( !s.endsWith("/") )
			s += "/";
		lineEditFlyerDirectory->setText(s);
	}
	raise();
}

void Options::on_toolButtonBrowseIconDirectory_clicked()
{
	QString s = QFileDialog::getExistingDirectory(this, tr("Choose icon directory"), lineEditIconDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog));
	if ( !s.isNull() ) {
		if ( !s.endsWith("/") )
			s += "/";
		lineEditIconDirectory->setText(s);
	}
	raise();
}

void Options::on_toolButtonBrowseCabinetDirectory_clicked()
{
	QString s = QFileDialog::getExistingDirectory(this, tr("Choose cabinet directory"), lineEditCabinetDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog));
	if ( !s.isNull() ) {
		if ( !s.endsWith("/") )
			s += "/";
		lineEditCabinetDirectory->setText(s);
	}
	raise();
}

void Options::on_toolButtonBrowseControllerDirectory_clicked()
{
	QString s = QFileDialog::getExistingDirectory(this, tr("Choose controller directory"), lineEditControllerDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog));
	if ( !s.isNull() ) {
		if ( !s.endsWith("/") )
			s += "/";
		lineEditControllerDirectory->setText(s);
	}
	raise();
}

void Options::on_toolButtonBrowseMarqueeDirectory_clicked()
{
	QString s = QFileDialog::getExistingDirectory(this, tr("Choose marquee directory"), lineEditMarqueeDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog));
	if ( !s.isNull() ) {
		if ( !s.endsWith("/") )
			s += "/";
		lineEditMarqueeDirectory->setText(s);
	}
	raise();
}

void Options::on_toolButtonBrowseTitleDirectory_clicked()
{
	QString s = QFileDialog::getExistingDirectory(this, tr("Choose title directory"), lineEditTitleDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog));
	if ( !s.isNull() ) {
		if ( !s.endsWith("/") )
			s += "/";
		lineEditTitleDirectory->setText(s);
	}
	raise();
}

void Options::on_toolButtonBrowsePCBDirectory_clicked()
{
	QString s = QFileDialog::getExistingDirectory(this, tr("Choose PCB directory"), lineEditPCBDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog));
	if ( !s.isNull() ) {
		if ( !s.endsWith("/") )
			s += "/";
		lineEditPCBDirectory->setText(s);
	}
	raise();
}

void Options::on_toolButtonBrowseOptionsTemplateFile_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose options template file"), lineEditOptionsTemplateFile->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditOptionsTemplateFile->setText(s);
	raise();
}

void Options::on_toolButtonBrowseExecutableFile_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose emulator executable file"), lineEditExecutableFile->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditExecutableFile->setText(s);
	raise();
}

void Options::on_toolButtonBrowseEmulatorLogFile_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose emulator log file"), lineEditEmulatorLogFile->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditEmulatorLogFile->setText(s);
	raise();
}

void Options::on_toolButtonBrowseXmlCacheDatabase_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose XML cache database file"), lineEditXmlCacheDatabase->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditXmlCacheDatabase->setText(s);
	raise();
}

void Options::on_toolButtonClearUserDataDatabase_clicked()
{
	if ( qmc2MachineList->userDataDb()->userDataRowCount() > 0 ) {
		switch ( QMessageBox::question(this, tr("Confirm"), tr("This will remove <b>all</b> existing user data and recreate the database.\nAre you sure you want to do this?"), tr("&Yes"), tr("&No"), QString(), 0, 1) ) {
			case 0:
				break;

			default:
			case 1:
				return;
		}
	}

	qmc2MachineList->userDataDb()->clearRankCache();
	qmc2MachineList->userDataDb()->clearCommentCache();
	qmc2MachineList->userDataDb()->recreateDatabase();
	qmc2MachineList->userDataDb()->setEmulatorVersion(qmc2MachineList->emulatorVersion);
	qmc2MachineList->userDataDb()->setQmc2Version(XSTR(QMC2_VERSION));
	qmc2MachineList->userDataDb()->setUserDataVersion(QMC2_USERDATA_VERSION);
	QTimer::singleShot(0, qmc2MainWindow, SLOT(updateUserData()));
}

void Options::on_toolButtonCleanupUserDataDatabase_clicked()
{
	qmc2MachineList->userDataDb()->cleanUp();
}

void Options::on_toolButtonBrowseUserDataDatabase_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose user data database file"), lineEditUserDataDatabase->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditUserDataDatabase->setText(s);
	raise();
}

void Options::on_toolButtonBrowseCookieDatabase_clicked()
{
	QString s = QFileDialog::getSaveFileName(this, tr("Choose cookie database file"), lineEditCookieDatabase->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditCookieDatabase->setText(s);
	raise();
}

void Options::on_toolButtonBrowseZipTool_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose zip tool"), lineEditZipTool->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditZipTool->setText(s);
	raise();
}

void Options::on_toolButtonBrowseSevenZipTool_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose 7-zip tool"), lineEditSevenZipTool->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditSevenZipTool->setText(s);
	raise();
}

void Options::on_toolButtonBrowseRomTool_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose ROM tool"), lineEditRomTool->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditRomTool->setText(s);
	raise();
}

void Options::on_toolButtonBrowseRomToolWorkingDirectory_clicked()
{
	QString s = QFileDialog::getExistingDirectory(this, tr("Choose working directory"), lineEditRomToolWorkingDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog));
	if ( !s.isNull() )
		lineEditRomToolWorkingDirectory->setText(s);
	raise();
}

void Options::on_toolButtonBrowseFavoritesFile_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose machine favorites file"), lineEditFavoritesFile->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditFavoritesFile->setText(s);
	raise();
}

void Options::on_toolButtonBrowseHistoryFile_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose play history file"), lineEditHistoryFile->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditHistoryFile->setText(s);
	raise();
}

void Options::on_toolButtonBrowseMachineListCacheFile_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose machine list cache file"), lineEditMachineListCacheFile->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditMachineListCacheFile->setText(s);
	raise();
}

void Options::on_toolButtonBrowseMachineListDatabase_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose machine list database file"), lineEditMachineListDatabase->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditMachineListDatabase->setText(s);
	raise();
}

void Options::on_toolButtonBrowseROMStateCacheFile_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose ROM state cache file"), lineEditROMStateCacheFile->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditROMStateCacheFile->setText(s);
	raise();
}

void Options::on_toolButtonBrowseSlotInfoCacheFile_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose slot info cache file"), lineEditSlotInfoCacheFile->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditSlotInfoCacheFile->setText(s);
	raise();
}

void Options::on_toolButtonBrowseWorkingDirectory_clicked()
{
	QString s = QFileDialog::getExistingDirectory(this, tr("Choose working directory"), lineEditWorkingDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog));
	if ( !s.isNull() ) {
		if ( !s.endsWith("/") )
			s += "/";
		lineEditWorkingDirectory->setText(s);
	}
	raise();
}

void Options::on_toolButtonBrowseSoftwareListCacheDb_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose software list cache database file"), lineEditSoftwareListCacheDb->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditSoftwareListCacheDb->setText(s);
	raise();
}

void Options::on_toolButtonBrowseSoftwareStateCache_clicked()
{
	QString s = QFileDialog::getExistingDirectory(this, tr("Choose software state cache directory"), lineEditSoftwareStateCache->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog));
	if ( !s.isEmpty() ) {
		if ( !s.endsWith("/") )
			s += "/";
		lineEditSoftwareStateCache->setText(s);
	}
	raise();
}

void Options::on_toolButtonBrowseGeneralSoftwareFolder_clicked()
{
	QString s = QFileDialog::getExistingDirectory(this, tr("Choose general software folder"), lineEditGeneralSoftwareFolder->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog));
	if ( !s.isNull() ) {
		if ( !s.endsWith("/") )
			s += "/";
		lineEditGeneralSoftwareFolder->setText(s);
	}
	raise();
}

void Options::on_toolButtonBrowseFrontendLogFile_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose front end log file"), lineEditFrontendLogFile->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditFrontendLogFile->setText(s);
	raise();
}

void Options::on_toolButtonBrowseDataDirectory_clicked()
{
	QString s = QFileDialog::getExistingDirectory(this, tr("Choose data directory"), lineEditDataDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog));
	if ( !s.isNull() ) {
		if ( !s.endsWith("/") )
			s += "/";
		lineEditDataDirectory->setText(s);
	}
	raise();
}

void Options::on_toolButtonBrowseDatInfoDatabase_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose dat-info database file"), lineEditDatInfoDatabase->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditDatInfoDatabase->setText(s);
	raise();
}

void Options::on_toolButtonBrowseMameHistoryDat_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose MAME machine info DB"), lineEditMameHistoryDat->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditMameHistoryDat->setText(s);
	raise();
}

void Options::on_toolButtonBrowseMessSysinfoDat_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose MESS machine info DB"), lineEditMessSysinfoDat->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditMessSysinfoDat->setText(s);
	raise();
}

void Options::on_toolButtonBrowseMameInfoDat_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose MAME emulator info DB"), lineEditMameInfoDat->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditMameInfoDat->setText(s);
	raise();
}

void Options::on_toolButtonBrowseMessInfoDat_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose MESS emulator info DB"), lineEditMessInfoDat->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditMessInfoDat->setText(s);
	raise();
}

void Options::on_toolButtonBrowseSoftwareInfoDB_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose software info DB"), lineEditSoftwareInfoDB->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditSoftwareInfoDB->setText(s);
	raise();
}

void Options::on_toolButtonBrowseCatverIniFile_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose catver.ini file"), lineEditCatverIniFile->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditCatverIniFile->setText(s);
	raise();
}

void Options::on_toolButtonBrowseCategoryIniFile_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose category.ini file"), lineEditCategoryIniFile->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditCategoryIniFile->setText(s);
	raise();
}

void Options::on_toolButtonBrowseFont_clicked()
{
	bool ok;
	QFont currentFont;
	if ( lineEditFont->text().isEmpty() )
#if defined(QMC2_OS_MAC)
		currentFont.fromString("Arial,10,-1,5,50,0,0,0,0,0");
#else
		currentFont = QApplication::font();
#endif
	else
		currentFont.fromString(lineEditFont->text());
	QFont f = QFontDialog::getFont(&ok, currentFont, 0);
	if ( ok ) {
		lineEditFont->setFont(f);
		lineEditFont->setText(f.toString());
	}
	raise();
}

void Options::on_toolButtonBrowseLogFont_clicked()
{
	bool ok;
	QFont currentFont;
	if ( lineEditLogFont->text().isEmpty() )
#if defined(QMC2_OS_MAC)
		currentFont.fromString("Courier New,10,-1,5,50,0,0,0,0,0");
#else
		currentFont = QApplication::font();
#endif
	else
		currentFont.fromString(lineEditLogFont->text());
	QFont f = QFontDialog::getFont(&ok, currentFont, 0);
	if ( ok ) {
		lineEditLogFont->setFont(f);
		lineEditLogFont->setText(f.toString());
	}
	raise();
}

void Options::moveEvent(QMoveEvent *e)
{
	if ( !qmc2CleaningUp && !qmc2EarlyStartup )
		config->setValue(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/Position", pos());

	e->accept();
}

void Options::resizeEvent(QResizeEvent *e)
{
	if ( !qmc2CleaningUp && !qmc2EarlyStartup )
		config->setValue(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/Size", size());

	e->accept();
}

void Options::on_radioButtonPreviewSelect_clicked()
{
	bool currentUsePreviewFile = (stackedWidgetPreview->currentIndex() == 1);
	stackedWidgetPreview->setCurrentIndex(!currentUsePreviewFile);
	radioButtonPreviewSelect->setText(!currentUsePreviewFile ? tr("Preview file") : tr("Preview directory"));
}

void Options::on_radioButtonFlyerSelect_clicked()
{
	bool currentUseFlyerFile = (stackedWidgetFlyer->currentIndex() == 1);
	stackedWidgetFlyer->setCurrentIndex(!currentUseFlyerFile);
	radioButtonFlyerSelect->setText(!currentUseFlyerFile ? tr("Flyer file") : tr("Flyer directory"));
}

void Options::on_radioButtonIconSelect_clicked()
{
	bool currentUseIconFile = (stackedWidgetIcon->currentIndex() == 1);
	stackedWidgetIcon->setCurrentIndex(!currentUseIconFile);
	radioButtonIconSelect->setText(!currentUseIconFile ? tr("Icon file") : tr("Icon directory"));
}

void Options::on_radioButtonCabinetSelect_clicked()
{
	bool currentUseCabinetFile = (stackedWidgetCabinet->currentIndex() == 1);
	stackedWidgetCabinet->setCurrentIndex(!currentUseCabinetFile);
	radioButtonCabinetSelect->setText(!currentUseCabinetFile ? tr("Cabinet file") : tr("Cabinet directory"));
}

void Options::on_radioButtonControllerSelect_clicked()
{
	bool currentUseControllerFile = (stackedWidgetController->currentIndex() == 1);
	stackedWidgetController->setCurrentIndex(!currentUseControllerFile);
	radioButtonControllerSelect->setText(!currentUseControllerFile ? tr("Controller file") : tr("Controller directory"));
}

void Options::on_radioButtonMarqueeSelect_clicked()
{
	bool currentUseMarqueeFile = (stackedWidgetMarquee->currentIndex() == 1);
	stackedWidgetMarquee->setCurrentIndex(!currentUseMarqueeFile);
	radioButtonMarqueeSelect->setText(!currentUseMarqueeFile ? tr("Marquee file") : tr("Marquee directory"));
}

void Options::on_radioButtonTitleSelect_clicked()
{
	bool currentUseTitleFile = (stackedWidgetTitle->currentIndex() == 1);
	stackedWidgetTitle->setCurrentIndex(!currentUseTitleFile);
	radioButtonTitleSelect->setText(!currentUseTitleFile ? tr("Title file") : tr("Title directory"));
}

void Options::on_radioButtonPCBSelect_clicked()
{
	bool currentUsePCBFile = (stackedWidgetPCB->currentIndex() == 1);
	stackedWidgetPCB->setCurrentIndex(!currentUsePCBFile);
	radioButtonPCBSelect->setText(!currentUsePCBFile ? tr("PCB file") : tr("PCB directory"));
}

void Options::on_radioButtonSoftwareSnapSelect_clicked()
{
	bool currentUseSoftwareSnapFile = (stackedWidgetSWSnap->currentIndex() == 1);
	stackedWidgetSWSnap->setCurrentIndex(!currentUseSoftwareSnapFile);
	radioButtonSoftwareSnapSelect->setText(!currentUseSoftwareSnapFile ? tr("SW snap file") : tr("SW snap folder"));
}

void Options::on_toolButtonBrowsePreviewFile_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose compressed preview file"), lineEditPreviewFile->text(), tr("Supported archives") + " (*.[zZ][iI][pP] *.7[zZ]);;" + tr("ZIP archives") + " (*.[zZ][iI][pP]);;" + tr("7z archives") + " (*.7[zZ]);;" + tr("All files") + " (*)", 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() ) {
		lineEditPreviewFile->setText(s);
		if ( s.toLower().endsWith(".zip") )
			comboBoxPreviewFileType->setCurrentIndex(QMC2_IMG_FILETYPE_ZIP);
		else if ( s.toLower().endsWith(".7z") )
			comboBoxPreviewFileType->setCurrentIndex(QMC2_IMG_FILETYPE_7Z);
#if defined(QMC2_LIBARCHIVE_ENABLED)
		else
			comboBoxPreviewFileType->setCurrentIndex(QMC2_IMG_FILETYPE_ARCHIVE);
#endif
	}
	raise();
}

void Options::on_toolButtonBrowseFlyerFile_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose compressed flyer file"), lineEditFlyerFile->text(), tr("Supported archives") + " (*.[zZ][iI][pP] *.7[zZ]);;" + tr("ZIP archives") + " (*.[zZ][iI][pP]);;" + tr("7z archives") + " (*.7[zZ]);;" + tr("All files") + " (*)", 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() ) {
		lineEditFlyerFile->setText(s);
		if ( s.toLower().endsWith(".zip") )
			comboBoxFlyerFileType->setCurrentIndex(QMC2_IMG_FILETYPE_ZIP);
		else if ( s.toLower().endsWith(".7z") )
			comboBoxFlyerFileType->setCurrentIndex(QMC2_IMG_FILETYPE_7Z);
#if defined(QMC2_LIBARCHIVE_ENABLED)
		else
			comboBoxFlyerFileType->setCurrentIndex(QMC2_IMG_FILETYPE_ARCHIVE);
#endif
	}
	raise();
}

void Options::on_toolButtonBrowseIconFile_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose compressed icon file"), lineEditIconFile->text(), tr("Supported archives") + " (*.[zZ][iI][pP] *.7[zZ]);;" + tr("ZIP archives") + " (*.[zZ][iI][pP]);;" + tr("7z archives") + " (*.7[zZ]);;" + tr("All files") + " (*)", 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() ) {
		lineEditIconFile->setText(s);
		if ( s.toLower().endsWith(".zip") )
			comboBoxIconFileType->setCurrentIndex(QMC2_IMG_FILETYPE_ZIP);
		else if ( s.toLower().endsWith(".7z") )
			comboBoxIconFileType->setCurrentIndex(QMC2_IMG_FILETYPE_7Z);
#if defined(QMC2_LIBARCHIVE_ENABLED)
		else
			comboBoxIconFileType->setCurrentIndex(QMC2_IMG_FILETYPE_ARCHIVE);
#endif
	}
	raise();
}

void Options::on_toolButtonBrowseCabinetFile_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose compressed cabinet file"), lineEditCabinetFile->text(), tr("Supported archives") + " (*.[zZ][iI][pP] *.7[zZ]);;" + tr("ZIP archives") + " (*.[zZ][iI][pP]);;" + tr("7z archives") + " (*.7[zZ]);;" + tr("All files") + " (*)", 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() ) {
		lineEditCabinetFile->setText(s);
		if ( s.toLower().endsWith(".zip") )
			comboBoxCabinetFileType->setCurrentIndex(QMC2_IMG_FILETYPE_ZIP);
		else if ( s.toLower().endsWith(".7z") )
			comboBoxCabinetFileType->setCurrentIndex(QMC2_IMG_FILETYPE_7Z);
#if defined(QMC2_LIBARCHIVE_ENABLED)
		else
			comboBoxCabinetFileType->setCurrentIndex(QMC2_IMG_FILETYPE_ARCHIVE);
#endif
	}
	raise();
}

void Options::on_toolButtonBrowseControllerFile_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose compressed controller file"), lineEditControllerFile->text(), tr("Supported archives") + " (*.[zZ][iI][pP] *.7[zZ]);;" + tr("ZIP archives") + " (*.[zZ][iI][pP]);;" + tr("7z archives") + " (*.7[zZ]);;" + tr("All files") + " (*)", 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() ) {
		lineEditControllerFile->setText(s);
		if ( s.toLower().endsWith(".zip") )
			comboBoxControllerFileType->setCurrentIndex(QMC2_IMG_FILETYPE_ZIP);
		else if ( s.toLower().endsWith(".7z") )
			comboBoxControllerFileType->setCurrentIndex(QMC2_IMG_FILETYPE_7Z);
#if defined(QMC2_LIBARCHIVE_ENABLED)
		else
			comboBoxControllerFileType->setCurrentIndex(QMC2_IMG_FILETYPE_ARCHIVE);
#endif
	}
	raise();
}

void Options::on_toolButtonBrowseMarqueeFile_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose compressed marquee file"), lineEditMarqueeFile->text(), tr("Supported archives") + " (*.[zZ][iI][pP] *.7[zZ]);;" + tr("ZIP archives") + " (*.[zZ][iI][pP]);;" + tr("7z archives") + " (*.7[zZ]);;" + tr("All files") + " (*)", 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() ) {
		lineEditMarqueeFile->setText(s);
		if ( s.toLower().endsWith(".zip") )
			comboBoxMarqueeFileType->setCurrentIndex(QMC2_IMG_FILETYPE_ZIP);
		else if ( s.toLower().endsWith(".7z") )
			comboBoxMarqueeFileType->setCurrentIndex(QMC2_IMG_FILETYPE_7Z);
#if defined(QMC2_LIBARCHIVE_ENABLED)
		else
			comboBoxMarqueeFileType->setCurrentIndex(QMC2_IMG_FILETYPE_ARCHIVE);
#endif
	}
	raise();
}

void Options::on_toolButtonBrowseTitleFile_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose compressed title file"), lineEditTitleFile->text(), tr("Supported archives") + " (*.[zZ][iI][pP] *.7[zZ]);;" + tr("ZIP archives") + " (*.[zZ][iI][pP]);;" + tr("7z archives") + " (*.7[zZ]);;" + tr("All files") + " (*)", 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() ) {
		lineEditTitleFile->setText(s);
		if ( s.toLower().endsWith(".zip") )
			comboBoxTitleFileType->setCurrentIndex(QMC2_IMG_FILETYPE_ZIP);
		else if ( s.toLower().endsWith(".7z") )
			comboBoxTitleFileType->setCurrentIndex(QMC2_IMG_FILETYPE_7Z);
#if defined(QMC2_LIBARCHIVE_ENABLED)
		else
			comboBoxTitleFileType->setCurrentIndex(QMC2_IMG_FILETYPE_ARCHIVE);
#endif
	}
	raise();
}

void Options::on_toolButtonBrowsePCBFile_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose compressed PCB file"), lineEditPCBFile->text(), tr("Supported archives") + " (*.[zZ][iI][pP] *.7[zZ]);;" + tr("ZIP archives") + " (*.[zZ][iI][pP]);;" + tr("7z archives") + " (*.7[zZ]);;" + tr("All files") + " (*)", 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() ) {
		lineEditPCBFile->setText(s);
		if ( s.toLower().endsWith(".zip") )
			comboBoxPCBFileType->setCurrentIndex(QMC2_IMG_FILETYPE_ZIP);
		else if ( s.toLower().endsWith(".7z") )
			comboBoxPCBFileType->setCurrentIndex(QMC2_IMG_FILETYPE_7Z);
#if defined(QMC2_LIBARCHIVE_ENABLED)
		else
			comboBoxPCBFileType->setCurrentIndex(QMC2_IMG_FILETYPE_ARCHIVE);
#endif
	}
	raise();
}

void Options::on_toolButtonBrowseSoftwareSnapDirectory_clicked()
{
	QString s = QFileDialog::getExistingDirectory(this, tr("Choose software snap directory"), lineEditSoftwareSnapDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog));
	if ( !s.isNull() ) {
		if ( !s.endsWith("/") )
			s += "/";
		lineEditSoftwareSnapDirectory->setText(s);
	}
	raise();
}

void Options::on_toolButtonBrowseSoftwareSnapFile_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose compressed software snap file"), lineEditSoftwareSnapFile->text(), tr("Supported archives") + " (*.[zZ][iI][pP] *.7[zZ]);;" + tr("ZIP archives") + " (*.[zZ][iI][pP]);;" + tr("7z archives") + " (*.7[zZ]);;" + tr("All files") + " (*)", 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() ) {
		lineEditSoftwareSnapFile->setText(s);
		if ( s.toLower().endsWith(".zip") )
			comboBoxSoftwareSnapFileType->setCurrentIndex(QMC2_IMG_FILETYPE_ZIP);
		else if ( s.toLower().endsWith(".7z") )
			comboBoxSoftwareSnapFileType->setCurrentIndex(QMC2_IMG_FILETYPE_7Z);
#if defined(QMC2_LIBARCHIVE_ENABLED)
		else
			comboBoxSoftwareSnapFileType->setCurrentIndex(QMC2_IMG_FILETYPE_ARCHIVE);
#endif
	}
	raise();
}

void Options::on_toolButtonBrowseSoftwareNotesFolder_clicked()
{
	QString s = QFileDialog::getExistingDirectory(this, tr("Choose software notes folder"), lineEditSoftwareNotesFolder->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog));
	if ( !s.isNull() ) {
		if ( !s.endsWith("/") )
			s += "/";
		lineEditSoftwareNotesFolder->setText(s);
	}
	raise();
}

void Options::on_toolButtonBrowseSoftwareNotesTemplate_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose software notes template"), lineEditSoftwareNotesTemplate->text(), tr("HTML files (*.html *.htm)") + ";;" + tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditSoftwareNotesTemplate->setText(s);
	raise();
}

void Options::on_toolButtonBrowseSystemNotesFolder_clicked()
{
	QString s = QFileDialog::getExistingDirectory(this, tr("Choose system notes folder"), lineEditSystemNotesFolder->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog));
	if ( !s.isNull() ) {
		if ( !s.endsWith("/") )
			s += "/";
		lineEditSystemNotesFolder->setText(s);
	}
	raise();
}

void Options::on_toolButtonBrowseSystemNotesTemplate_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose system notes template"), lineEditSystemNotesTemplate->text(), tr("HTML files (*.html *.htm)") + ";;" + tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditSystemNotesTemplate->setText(s);
	raise();
}

void Options::on_toolButtonBrowseVideoSnapFolder_clicked()
{
	QString s = QFileDialog::getExistingDirectory(this, tr("Choose video snap folder"), lineEditVideoSnapFolder->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog));
	if ( !s.isNull() ) {
		if ( !s.endsWith("/") )
			s += "/";
		lineEditVideoSnapFolder->setText(s);
	}
	raise();
}

void Options::on_treeWidgetShortcuts_itemActivated(QTreeWidgetItem *item)
{
	if ( !item )
		return;

	qApp->removeEventFilter(qmc2MainEventFilter);

	KeySequenceScanner keySeqScanner(this, qmc2QtKeyHash.contains(item->text(1)));
	if ( keySeqScanner.exec() == QDialog::Accepted ) {
		QStringList words = item->text(1).split("+");
		QString nativeShortcut = "";
		for (int i = 0; i < words.count(); i++) {
			if ( i > 0 )
				nativeShortcut += "+";
			nativeShortcut += QObject::tr(words[i].toUtf8().constData());
		}

		bool found = false;
		QHashIterator<QString, QPair<QString, QAction *> > it(qmc2ShortcutHash);
		while ( it.hasNext() && !found ) {
			it.next();
			words = it.key().split("+");
			QString itShortcut;
			for (int i = 0; i < words.count(); i++) {
				if ( i > 0 )
					itShortcut += "+";
				itShortcut += QObject::tr(words[i].toUtf8().constData());
			}

			if ( itShortcut == nativeShortcut ) {
				found = true;
				nativeShortcut = it.key();
			}
		}

		if ( found ) {
			qmc2CustomShortcutHash[nativeShortcut] = keySeqScanner.currentKeySequence;
			item->setText(2, keySeqScanner.labelKeySequence->text());
			QTimer::singleShot(0, this, SLOT(checkShortcuts()));
		}

		pushButtonResetShortcut->setEnabled(true);
	}

	qApp->installEventFilter(qmc2MainEventFilter);
}

void Options::on_treeWidgetShortcuts_itemSelectionChanged()
{
	QList<QTreeWidgetItem *> selItems = treeWidgetShortcuts->selectedItems();
	if ( selItems.count() > 0 ) {
		pushButtonRedefineKeySequence->setEnabled(true);
		pushButtonResetShortcut->setEnabled(selItems[0]->text(2).length() > 0);
	} else {
		pushButtonRedefineKeySequence->setEnabled(false);
		pushButtonResetShortcut->setEnabled(false);
	}
}

void Options::on_pushButtonRedefineKeySequence_clicked()
{
	QList<QTreeWidgetItem *> selItems = treeWidgetShortcuts->selectedItems();
	if ( selItems.count() > 0 )
		on_treeWidgetShortcuts_itemActivated(selItems[0]);
}

void Options::on_pushButtonResetShortcut_clicked()
{
	QList<QTreeWidgetItem *> selItems = treeWidgetShortcuts->selectedItems();
	if ( selItems.count() > 0 ) {
		QStringList words = selItems[0]->text(1).split("+");
		QString nativeShortcut = "";
		for (int i = 0; i < words.count(); i++) {
			if ( i > 0 )
				nativeShortcut += "+";
			nativeShortcut += QObject::tr(words[i].toUtf8().constData());
		}

		bool found = false;
		QHashIterator<QString, QPair<QString, QAction *> > it(qmc2ShortcutHash);
		while ( it.hasNext() && !found ) {
			it.next();
			words = it.key().split("+");
			QString itShortcut;
			for (int i = 0; i < words.count(); i++) {
				if ( i > 0 )
					itShortcut += "+";
				itShortcut += QObject::tr(words[i].toUtf8().constData());
			}

			if ( itShortcut == nativeShortcut ) {
				found = true;
				nativeShortcut = it.key();
			}
		}

		if ( found ) {
			qmc2CustomShortcutHash[nativeShortcut] = nativeShortcut;
			selItems[0]->setText(2, "");
			QTimer::singleShot(0, this, SLOT(checkShortcuts()));
		}

		pushButtonResetShortcut->setEnabled(false);
	}
}

void Options::on_pushButtonComponentSetup_clicked()
{
	if ( !qmc2ComponentSetup )
		return;
	qmc2ComponentSetup->adjustIconSizes();
	qmc2ComponentSetup->setParent(this);
	qmc2ComponentSetup->setWindowFlags(Qt::Dialog);
	if ( qmc2ComponentSetup->isHidden() )
		qmc2ComponentSetup->show();
	else if ( qmc2ComponentSetup->isMinimized() )
		qmc2ComponentSetup->showNormal();
	QTimer::singleShot(0, qmc2ComponentSetup, SLOT(raise()));
}

void Options::on_pushButtonCustomizeToolBar_clicked()
{
	if ( !qmc2ToolBarCustomizer )
		qmc2ToolBarCustomizer = new ToolBarCustomizer(this);

	qmc2ToolBarCustomizer->exec();
}

void Options::on_pushButtonIndividualFallbackSettings_clicked()
{
	IndividualFallbackSettings ifs(this);
	ifs.exec();
}

void Options::on_pushButtonEditPalette_clicked()
{
	QPalette currentPalette = qApp->palette();
	if ( !qmc2PaletteEditor )
		qmc2PaletteEditor = new PaletteEditor(this);
	loadCustomPalette(currentStyleName());
	qmc2PaletteEditor->activePalette = qmc2CustomPalette;
	bool wasChecked = qmc2PaletteEditor->toolButtonPreview->isChecked();
	qmc2PaletteEditor->toolButtonPreview->blockSignals(true);
	qmc2PaletteEditor->toolButtonPreview->setChecked(false);
	qmc2PaletteEditor->on_pushButtonRestore_clicked();
	qmc2PaletteEditor->toolButtonPreview->setChecked(wasChecked);
	qmc2PaletteEditor->toolButtonPreview->blockSignals(false);
	qmc2PaletteEditor->activePalette = currentPalette;
	qmc2PaletteEditor->show();
	if ( qmc2PaletteEditor->toolButtonPreview->isChecked() ) {
		qApp->setPalette(qmc2PaletteEditor->customPalette);
		QTimer::singleShot(0, qmc2MainWindow, SLOT(updateUserData()));
	} else
		qApp->setPalette(currentPalette);
	if ( qmc2PaletteEditor->exec() == QDialog::Accepted ) {
		config->setValue(QMC2_FRONTEND_PREFIX + "GUI/StandardColorPalette", false);
		qmc2CustomPalette = qmc2PaletteEditor->customPalette;
		saveCustomPalette();
	} else {
		checkBoxStandardColorPalette->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/StandardColorPalette", true).toBool());
		if ( checkBoxStandardColorPalette->isChecked() )
			qApp->setPalette(qmc2StandardPalettes[currentStyleName()]);
		else
			qApp->setPalette(qmc2CustomPalette);
	}
	QTimer::singleShot(0, qmc2MainWindow, SLOT(updateUserData()));
}

void Options::checkShortcuts()
{
	static char lastShortcutsState = -1;
	char shortcutsState = 1;
	int itemCount = treeWidgetShortcuts->topLevelItemCount();
	for (int i = 0; i < itemCount; i++) {
		QTreeWidgetItem *iItem = treeWidgetShortcuts->topLevelItem(i);
		if ( iItem->text(2).isEmpty() )
			iItem->setForeground(1, greenBrush);
		else
			iItem->setForeground(1, greyBrush);
		iItem->setForeground(2, greenBrush);
	}
	for (int i = 0; i < itemCount; i++) {
		QTreeWidgetItem *iItem = treeWidgetShortcuts->topLevelItem(i);
		int iColumn = 1;
		if ( !iItem->text(2).isEmpty() )
			iColumn = 2;
		QString iShortcut(iItem->text(iColumn));
		for (int j = i + 1; j < itemCount; j++) {
			QTreeWidgetItem *jItem = treeWidgetShortcuts->topLevelItem(j);
			int jColumn = 1;
			if ( !jItem->text(2).isEmpty() )
				jColumn = 2;
			QString jShortcut(jItem->text(jColumn));
			if ( iShortcut.compare(jShortcut) == 0 ) {
				iItem->setForeground(iColumn, redBrush);
				jItem->setForeground(jColumn, redBrush);
				shortcutsState = 0;
			}
		}
	}
	if ( shortcutsState != lastShortcutsState || lastShortcutsState == -1 ) {
		if ( shortcutsState == 1 )
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("shortcut map is clean"));
		else
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: shortcut map contains duplicates"));
	}
	lastShortcutsState = shortcutsState;
}

void Options::setupShortcutActions()
{
	QHashIterator<QString, QPair<QString, QAction *> > it(qmc2ShortcutHash);
	while ( it.hasNext() ) {
		it.next();
		QAction *action = it.value().second;
		if ( action ) {
			action->setShortcut(QKeySequence(qmc2CustomShortcutHash[it.key()]));
			action->setShortcutContext(Qt::ApplicationShortcut);
		}
	}
}

void Options::on_toolButtonBrowseAdditionalEmulatorExecutable_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose emulator executable file"), lineEditAdditionalEmulatorExecutableFile->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditAdditionalEmulatorExecutableFile->setText(s);
	raise();
}

void Options::on_toolButtonBrowseAdditionalEmulatorWorkingDirectory_clicked()
{
	QString startPath = lineEditAdditionalEmulatorWorkingDirectory->text();
	if ( startPath.isEmpty() ) {
		QString exePath = lineEditAdditionalEmulatorExecutableFile->text();
		if ( !exePath.isEmpty() ) {
			QFileInfo fi(exePath);
			startPath = fi.absolutePath();
		}
	}
	QString s = QFileDialog::getExistingDirectory(this, tr("Choose working directory"), startPath, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog));
	if ( !s.isNull() ) {
		if ( !s.endsWith("/") )
			s += "/";
		lineEditAdditionalEmulatorWorkingDirectory->setText(s);
	}
	raise();
}

void Options::on_toolButtonAddEmulator_clicked()
{
	tableWidgetRegisteredEmulators->setSortingEnabled(false);
	int row = tableWidgetRegisteredEmulators->rowCount();
	tableWidgetRegisteredEmulators->insertRow(row);
	tableWidgetRegisteredEmulators->setItem(row, QMC2_ADDTLEMUS_COLUMN_NAME, new QTableWidgetItem(lineEditAdditionalEmulatorName->text()));
	tableWidgetRegisteredEmulators->setItem(row, QMC2_ADDTLEMUS_COLUMN_EXEC, new QTableWidgetItem(lineEditAdditionalEmulatorExecutableFile->text()));
	tableWidgetRegisteredEmulators->setItem(row, QMC2_ADDTLEMUS_COLUMN_WDIR, new QTableWidgetItem(lineEditAdditionalEmulatorWorkingDirectory->text()));
	tableWidgetRegisteredEmulators->setItem(row, QMC2_ADDTLEMUS_COLUMN_ARGS, new QTableWidgetItem(lineEditAdditionalEmulatorArguments->text()));
	QToolButton *tb = new QToolButton(0);
	tb->setObjectName(lineEditAdditionalEmulatorName->text());
	tb->setAutoFillBackground(true);
	tb->setText(tr("Custom IDs..."));
	tb->setToolTip(tr("Specify pre-defined foreign IDs for this emulator, launchable from the 'foreign emulators' view"));
	tableWidgetRegisteredEmulators->setCellWidget(row, QMC2_ADDTLEMUS_COLUMN_CUID, tb);
	connect(tb, SIGNAL(clicked()), this, SLOT(setupCustomIDsClicked()));
	tb = new QToolButton(0);
	QFontMetrics fm(qApp->font());
	tb->setIconSize(QSize(fm.height(), fm.height()));
	tb->setIcon(QIcon(QString::fromUtf8(":/data/img/alien.png")));
	tb->setWhatsThis(":/data/img/alien.png");
	tb->setToolTip(tr("Choose icon for this foreign emulator (hold down for menu)"));
	QMenu *menu = new QMenu(tb);
	QAction *action = menu->addAction(QIcon(QString::fromUtf8(":/data/img/alien.png")), tr("Default icon"));
	connect(action, SIGNAL(triggered(bool)), this, SLOT(actionDefaultEmuIconTriggered()));
	menu->addSeparator();
	action = menu->addAction(QIcon(QString::fromUtf8(":/data/img/no.png")), tr("No icon"));
	connect(action, SIGNAL(triggered(bool)), this, SLOT(actionNoEmuIconTriggered()));
	tb->setMenu(menu);
	tableWidgetRegisteredEmulators->setCellWidget(row, QMC2_ADDTLEMUS_COLUMN_ICON, tb);
	connect(tb, SIGNAL(clicked()), this, SLOT(chooseEmuIconClicked()));
	tableWidgetRegisteredEmulators->resizeRowsToContents();
	on_lineEditAdditionalEmulatorName_textChanged(lineEditAdditionalEmulatorName->text());
	tableWidgetRegisteredEmulators->setSortingEnabled(true);
}

void Options::on_toolButtonSaveEmulator_clicked()
{
	tableWidgetRegisteredEmulators->setSortingEnabled(false);
	QString name = lineEditAdditionalEmulatorName->text();
	if ( !name.isEmpty() ) {
		QList<QTableWidgetItem *> il = tableWidgetRegisteredEmulators->findItems(name, Qt::MatchExactly);
		if ( !il.isEmpty() ) {
			int row = il[0]->row();
			if ( tableWidgetRegisteredEmulators->item(row, QMC2_ADDTLEMUS_COLUMN_NAME) )
				tableWidgetRegisteredEmulators->item(row, QMC2_ADDTLEMUS_COLUMN_NAME)->setText(name);
			else
				tableWidgetRegisteredEmulators->setItem(row, QMC2_ADDTLEMUS_COLUMN_NAME, new QTableWidgetItem(lineEditAdditionalEmulatorName->text()));
			if ( tableWidgetRegisteredEmulators->item(row, QMC2_ADDTLEMUS_COLUMN_EXEC) )
				tableWidgetRegisteredEmulators->item(row, QMC2_ADDTLEMUS_COLUMN_EXEC)->setText(lineEditAdditionalEmulatorExecutableFile->text());
			else
				tableWidgetRegisteredEmulators->setItem(row, QMC2_ADDTLEMUS_COLUMN_EXEC, new QTableWidgetItem(lineEditAdditionalEmulatorExecutableFile->text()));
			if ( tableWidgetRegisteredEmulators->item(row, QMC2_ADDTLEMUS_COLUMN_WDIR) )
				tableWidgetRegisteredEmulators->item(row, QMC2_ADDTLEMUS_COLUMN_WDIR)->setText(lineEditAdditionalEmulatorWorkingDirectory->text());
			else
				tableWidgetRegisteredEmulators->setItem(row, QMC2_ADDTLEMUS_COLUMN_WDIR, new QTableWidgetItem(lineEditAdditionalEmulatorWorkingDirectory->text()));
			if ( tableWidgetRegisteredEmulators->item(row, QMC2_ADDTLEMUS_COLUMN_ARGS) )
				tableWidgetRegisteredEmulators->item(row, QMC2_ADDTLEMUS_COLUMN_ARGS)->setText(lineEditAdditionalEmulatorArguments->text());
			else
				tableWidgetRegisteredEmulators->setItem(row, QMC2_ADDTLEMUS_COLUMN_ARGS, new QTableWidgetItem(lineEditAdditionalEmulatorArguments->text()));
		}
	}
	tableWidgetRegisteredEmulators->setSortingEnabled(true);
}

void Options::on_toolButtonRemoveEmulator_clicked()
{
	QList<QTableWidgetItem *> sl = tableWidgetRegisteredEmulators->selectedItems();
	if ( !sl.isEmpty() ) {
		int row = sl[0]->row();
		registeredEmulatorsToBeRemoved << tableWidgetRegisteredEmulators->item(row, QMC2_ADDTLEMUS_COLUMN_NAME)->text();
		tableWidgetRegisteredEmulators->removeRow(row);
	}
}

void Options::on_toolButtonCustomIDs_clicked()
{
	QList<QTableWidgetItem *> sl = tableWidgetRegisteredEmulators->selectedItems();
	if ( !sl.isEmpty() ) {
		int row = sl[0]->row();
		if ( tableWidgetRegisteredEmulators->item(row, QMC2_ADDTLEMUS_COLUMN_NAME) ) {
			CustomIDSetup cidSetup(tableWidgetRegisteredEmulators->item(row, QMC2_ADDTLEMUS_COLUMN_NAME)->text(), this);
			if ( cidSetup.exec() == QDialog::Accepted )
				cidSetup.save();
		}
	}
}

void Options::checkPlaceholderStatus()
{
	QPalette pal(labelIDStatus->palette());
	QString text(lineEditAdditionalEmulatorArguments->text());
	if ( lineEditAdditionalEmulatorName->text().isEmpty() )
		pal.setBrush(QPalette::Window, Options::lightgreyBrush);
	else if ( text.isEmpty() )
		pal.setBrush(QPalette::Window, Options::yellowBrush);
	else if ( text.contains("$ID$") )
		pal.setBrush(QPalette::Window, Options::greenBrush);
	else
		pal.setBrush(QPalette::Window, Options::yellowBrush);
	labelIDStatus->setPalette(pal);
}

void Options::on_tableWidgetRegisteredEmulators_itemSelectionChanged()
{
	QList<QTableWidgetItem *> sl = tableWidgetRegisteredEmulators->selectedItems();
	if ( !sl.isEmpty() ) {
		int row = sl[0]->row();
		if ( tableWidgetRegisteredEmulators->item(row, QMC2_ADDTLEMUS_COLUMN_NAME) ) {
			lineEditAdditionalEmulatorName->setText(tableWidgetRegisteredEmulators->item(row, QMC2_ADDTLEMUS_COLUMN_NAME)->text());
			toolButtonRemoveEmulator->setEnabled(true);
			toolButtonCustomIDs->setEnabled(true);
		} else {
			lineEditAdditionalEmulatorName->clear();
			toolButtonRemoveEmulator->setEnabled(false);
			toolButtonCustomIDs->setEnabled(false);
		}
		if ( tableWidgetRegisteredEmulators->item(row, QMC2_ADDTLEMUS_COLUMN_EXEC) )
			lineEditAdditionalEmulatorExecutableFile->setText(tableWidgetRegisteredEmulators->item(row, QMC2_ADDTLEMUS_COLUMN_EXEC)->text());
		else
			lineEditAdditionalEmulatorExecutableFile->clear();
		if ( tableWidgetRegisteredEmulators->item(row, QMC2_ADDTLEMUS_COLUMN_WDIR) )
			lineEditAdditionalEmulatorWorkingDirectory->setText(tableWidgetRegisteredEmulators->item(row, QMC2_ADDTLEMUS_COLUMN_WDIR)->text());
		else
			lineEditAdditionalEmulatorWorkingDirectory->clear();
		if ( tableWidgetRegisteredEmulators->item(row, QMC2_ADDTLEMUS_COLUMN_ARGS) )
			lineEditAdditionalEmulatorArguments->setText(tableWidgetRegisteredEmulators->item(row, QMC2_ADDTLEMUS_COLUMN_ARGS)->text());
		else
			lineEditAdditionalEmulatorArguments->clear();
	} else {
		lineEditAdditionalEmulatorName->clear();
		lineEditAdditionalEmulatorExecutableFile->clear();
		lineEditAdditionalEmulatorWorkingDirectory->clear();
		lineEditAdditionalEmulatorArguments->clear();
		toolButtonRemoveEmulator->setEnabled(false);
		toolButtonCustomIDs->setEnabled(false);
	}
	checkPlaceholderStatus();
}

void Options::on_lineEditAdditionalEmulatorName_textChanged(const QString &text)
{
	if ( !text.isEmpty() ) {
		if ( text == tr("Default") ) {
			// this name isn't allowed!
			toolButtonAddEmulator->setEnabled(false);
			toolButtonSaveEmulator->setEnabled(false);
			toolButtonRemoveEmulator->setEnabled(false);
			toolButtonCustomIDs->setEnabled(false);
		} else {
			QList<QTableWidgetItem *> il = tableWidgetRegisteredEmulators->findItems(text, Qt::MatchExactly);
			toolButtonAddEmulator->setEnabled(il.isEmpty());
			toolButtonSaveEmulator->setEnabled(!il.isEmpty());
			toolButtonRemoveEmulator->setEnabled(!il.isEmpty());
			toolButtonCustomIDs->setEnabled(!il.isEmpty());
		}
	} else {
		toolButtonAddEmulator->setEnabled(false);
		toolButtonSaveEmulator->setEnabled(false);
		toolButtonRemoveEmulator->setEnabled(false);
		toolButtonCustomIDs->setEnabled(false);
	}
	checkPlaceholderStatus();
}

void Options::on_lineEditAdditionalEmulatorArguments_textChanged(const QString &)
{
	checkPlaceholderStatus();
}

void Options::setupCustomIDsClicked()
{
	QToolButton *tb = (QToolButton *)sender();
	if ( tb ) {
		if ( !tb->objectName().isEmpty() ) {
			CustomIDSetup cidSetup(tb->objectName(), this);
			if ( cidSetup.exec() == QDialog::Accepted )
				cidSetup.save();
		}
	}
}

void Options::chooseEmuIconClicked()
{
	QToolButton *tb = (QToolButton *)sender();
	if ( tb ) {
		QString emuIcon = tb->whatsThis();
		if ( emuIcon.startsWith(":") )
			emuIcon.clear();
		QStringList imageFileTypes;
		foreach (QByteArray imageFormat, QImageReader::supportedImageFormats())
			imageFileTypes << "*." + QString(imageFormat).toLower();
		QString fileName = QFileDialog::getOpenFileName(this, tr("Choose image file"), emuIcon, tr("Supported image files (%1)").arg(imageFileTypes.join(" ")) + ";;" + tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
		if ( !fileName.isEmpty() ) {
			QIcon icon = QIcon(fileName);
			if ( !icon.isNull() ) {
				tb->setIcon(icon);
				tb->setWhatsThis(fileName);
			}
		}
	}
}

void Options::actionDefaultEmuIconTriggered()
{
	QAction *action = (QAction *)sender();
	if ( action ) {
		QToolButton *tb = (QToolButton *)action->parentWidget()->parentWidget();
		if ( tb ) {
			tb->setIcon(QIcon(QString::fromUtf8(":/data/img/alien.png")));
			tb->setWhatsThis(":/data/img/alien.png");
		}
	}
}

void Options::actionNoEmuIconTriggered()
{
	QAction *action = (QAction *)sender();
	if ( action ) {
		QToolButton *tb = (QToolButton *)action->parentWidget()->parentWidget();
		if ( tb ) {
			tb->setIcon(QIcon());
			tb->setWhatsThis("[none]");
		}
	}
}

void Options::loadCustomPalette(QString styleName)
{
	qmc2CustomPalette = qmc2StandardPalettes[styleName];
	QStringList activeColors, inactiveColors, disabledColors;
	if ( config->contains(QMC2_FRONTEND_PREFIX + "Layout/CustomPalette/ActiveColors") )
		activeColors = config->value(QMC2_FRONTEND_PREFIX + "Layout/CustomPalette/ActiveColors").toStringList();
	for (int i = 0; i < activeColors.count(); i++) {
		QPalette::ColorRole colorRole = PaletteEditor::colorNameToRole(PaletteEditor::colorNames[i]);
		qmc2CustomPalette.setColor(QPalette::Active, colorRole, QColor(activeColors[i]));
	}
	if ( config->contains(QMC2_FRONTEND_PREFIX + "Layout/CustomPalette/InactiveColors") )
		inactiveColors = config->value(QMC2_FRONTEND_PREFIX + "Layout/CustomPalette/InactiveColors").toStringList();
	for (int i = 0; i < inactiveColors.count(); i++) {
		QPalette::ColorRole colorRole = PaletteEditor::colorNameToRole(PaletteEditor::colorNames[i]);
		qmc2CustomPalette.setColor(QPalette::Inactive, colorRole, QColor(inactiveColors[i]));
	}
	if ( config->contains(QMC2_FRONTEND_PREFIX + "Layout/CustomPalette/DisabledColors") )
		disabledColors = config->value(QMC2_FRONTEND_PREFIX + "Layout/CustomPalette/DisabledColors").toStringList();
	for (int i = 0; i < disabledColors.count(); i++) {
		QPalette::ColorRole colorRole = PaletteEditor::colorNameToRole(PaletteEditor::colorNames[i]);
		qmc2CustomPalette.setColor(QPalette::Disabled, colorRole, QColor(disabledColors[i]));
	}
}

void Options::saveCustomPalette()
{
	QStringList activeColors, inactiveColors, disabledColors;
	for (int i = 0; i < PaletteEditor::colorNames.count(); i++) {
		QPalette::ColorRole colorRole = PaletteEditor::colorNameToRole(PaletteEditor::colorNames[i]);
		activeColors << qmc2CustomPalette.color(QPalette::Active, colorRole).name();
		inactiveColors << qmc2CustomPalette.color(QPalette::Inactive, colorRole).name();
		disabledColors << qmc2CustomPalette.color(QPalette::Disabled, colorRole).name();
	}
	config->setValue(QMC2_FRONTEND_PREFIX + "Layout/CustomPalette/ActiveColors", activeColors);
	config->setValue(QMC2_FRONTEND_PREFIX + "Layout/CustomPalette/InactiveColors", inactiveColors);
	config->setValue(QMC2_FRONTEND_PREFIX + "Layout/CustomPalette/DisabledColors", disabledColors);
}

void Options::enableWidgets(bool enable)
{
	toolButtonBrowseStyleSheet->setEnabled(enable);
	toolButtonBrowseFont->setEnabled(enable);
	toolButtonBrowseLogFont->setEnabled(enable);
	toolButtonBrowseTemporaryFile->setEnabled(enable);
	toolButtonBrowseFrontendLogFile->setEnabled(enable);
	toolButtonBrowsePreviewDirectory->setEnabled(enable);
	toolButtonBrowsePreviewFile->setEnabled(enable);
	toolButtonBrowseDataDirectory->setEnabled(enable);
	toolButtonBrowseDatInfoDatabase->setEnabled(enable);
	toolButtonBrowseMameHistoryDat->setEnabled(enable);
	toolButtonBrowseMessSysinfoDat->setEnabled(enable);
	checkBoxProcessMameHistoryDat->setEnabled(enable);
	checkBoxProcessMessSysinfoDat->setEnabled(enable);
	toolButtonBrowseMameInfoDat->setEnabled(enable);
	toolButtonBrowseMessInfoDat->setEnabled(enable);
	checkBoxProcessMameInfoDat->setEnabled(enable);
	checkBoxProcessMessInfoDat->setEnabled(enable);
	toolButtonBrowseSoftwareInfoDB->setEnabled(enable);
	checkBoxProcessSoftwareInfoDB->setEnabled(enable);
	toolButtonImportGameInfo->setEnabled(enable);
	toolButtonImportMachineInfo->setEnabled(enable);
	toolButtonImportMameInfo->setEnabled(enable);
	toolButtonImportMessInfo->setEnabled(enable);
	toolButtonImportSoftwareInfo->setEnabled(enable);
	toolButtonOptimizeCatverIni->setEnabled(enable);
	toolButtonBrowseCatverIniFile->setEnabled(enable);
	checkBoxUseCatverIni->setEnabled(enable);
	toolButtonBrowseCategoryIniFile->setEnabled(enable);
	checkBoxUseCategoryIni->setEnabled(enable);
	checkBoxShowROMStatusIcons->setEnabled(enable);
	checkBoxRomStateFilter->setEnabled(enable);
	checkBoxShowBiosSets->setEnabled(enable);
	checkBoxShowDeviceSets->setEnabled(enable);
	toolButtonBrowseSoftwareListCacheDb->setEnabled(enable);
	toolButtonBrowseSoftwareStateCache->setEnabled(enable);
	toolButtonBrowseGeneralSoftwareFolder->setEnabled(enable);
	toolButtonBrowseExecutableFile->setEnabled(enable);
	lineEditExecutableFile->setEnabled(enable);
	toolButtonBrowseWorkingDirectory->setEnabled(enable);
	toolButtonBrowseEmulatorLogFile->setEnabled(enable);
	toolButtonBrowseOptionsTemplateFile->setEnabled(enable);
	toolButtonBrowseXmlCacheDatabase->setEnabled(enable);
	toolButtonBrowseUserDataDatabase->setEnabled(enable);
	toolButtonCleanupUserDataDatabase->setEnabled(enable);
	toolButtonClearUserDataDatabase->setEnabled(enable);
	toolButtonBrowseFavoritesFile->setEnabled(enable);
	toolButtonBrowseHistoryFile->setEnabled(enable);
	toolButtonBrowseMachineListCacheFile->setEnabled(enable);
	toolButtonBrowseMachineListDatabase->setEnabled(enable);
	toolButtonBrowseROMStateCacheFile->setEnabled(enable);
	toolButtonBrowseSlotInfoCacheFile->setEnabled(enable);
	toolButtonBrowseFlyerDirectory->setEnabled(enable);
	toolButtonBrowseFlyerFile->setEnabled(enable);
	toolButtonBrowseIconDirectory->setEnabled(enable);
	toolButtonBrowseIconFile->setEnabled(enable);
	toolButtonBrowseCabinetDirectory->setEnabled(enable);
	toolButtonBrowseCabinetFile->setEnabled(enable);
	toolButtonBrowseControllerDirectory->setEnabled(enable);
	toolButtonBrowseControllerFile->setEnabled(enable);
	toolButtonBrowseMarqueeDirectory->setEnabled(enable);
	toolButtonBrowseMarqueeFile->setEnabled(enable);
	toolButtonBrowseTitleDirectory->setEnabled(enable);
	toolButtonBrowseTitleFile->setEnabled(enable);
	toolButtonBrowsePCBDirectory->setEnabled(enable);
	toolButtonBrowsePCBFile->setEnabled(enable);
	comboBoxIconFileType->setEnabled(enable);
	comboBoxPreviewFileType->setEnabled(enable);
	comboBoxFlyerFileType->setEnabled(enable);
	comboBoxCabinetFileType->setEnabled(enable);
	comboBoxControllerFileType->setEnabled(enable);
	comboBoxMarqueeFileType->setEnabled(enable);
	comboBoxTitleFileType->setEnabled(enable);
	comboBoxPCBFileType->setEnabled(enable);
	comboBoxSoftwareSnapFileType->setEnabled(enable);
	toolButtonBrowseSoftwareSnapDirectory->setEnabled(enable);
	toolButtonBrowseSoftwareSnapFile->setEnabled(enable);
	toolButtonBrowseSoftwareNotesFolder->setEnabled(enable);
	toolButtonBrowseSoftwareNotesTemplate->setEnabled(enable);
	toolButtonBrowseSystemNotesFolder->setEnabled(enable);
	toolButtonBrowseSystemNotesTemplate->setEnabled(enable);
	toolButtonBrowseVideoSnapFolder->setEnabled(enable);
	toolButtonShowC->setEnabled(enable);
	toolButtonShowM->setEnabled(enable);
	toolButtonShowI->setEnabled(enable);
	toolButtonShowN->setEnabled(enable);
	toolButtonShowU->setEnabled(enable);
	comboBoxSortCriteria->setEnabled(enable);
	comboBoxSortOrder->setEnabled(enable);
	treeWidgetShortcuts->clearSelection();
	treeWidgetShortcuts->setEnabled(enable);
	treeWidgetJoystickMappings->clearSelection();
	treeWidgetJoystickMappings->setEnabled(enable);
	toolButtonBrowseCookieDatabase->setEnabled(enable);
	pushButtonManageCookies->setEnabled(enable ? checkBoxRestoreCookies->isChecked() : false);
	toolButtonBrowseZipTool->setEnabled(enable);
	toolButtonBrowseSevenZipTool->setEnabled(enable);
	toolButtonBrowseRomTool->setEnabled(enable);
	toolButtonBrowseRomToolWorkingDirectory->setEnabled(enable);
	toolButtonBrowseAdditionalEmulatorExecutable->setEnabled(enable);
	toolButtonBrowseAdditionalEmulatorWorkingDirectory->setEnabled(enable);
	pushButtonCustomizeToolBar->setEnabled(enable);
	checkBoxParentImageFallback->setEnabled(enable);
	pushButtonIndividualFallbackSettings->setEnabled(enable && checkBoxParentImageFallback->isChecked());
	checkBoxStandardColorPalette->setEnabled(enable);
	pushButtonEditPalette->setEnabled(enable && !checkBoxStandardColorPalette->isChecked());
	pushButtonAdditionalArtworkSetup->setEnabled(enable);
	pushButtonImageFormats->setEnabled(enable);
	for (int row = 0; row < tableWidgetRegisteredEmulators->rowCount(); row++) {
		QWidget *w = tableWidgetRegisteredEmulators->cellWidget(row, QMC2_ADDTLEMUS_COLUMN_ICON);
		if ( w )
			w->setEnabled(enable);
		w = tableWidgetRegisteredEmulators->cellWidget(row, QMC2_ADDTLEMUS_COLUMN_CUID);
		if ( w )
			w->setEnabled(enable);
	}
}

#if QMC2_JOYSTICK == 1
void Options::on_pushButtonRescanJoysticks_clicked()
{
	toolButtonMapJoystick->setChecked(true);
	on_toolButtonMapJoystick_clicked();

	if ( !cancelClicked ) {
		QStringList joystickNames;
		joystickNames << tr("No joysticks found");

		if ( joystick )
			delete joystick;

		joystick = new Joystick(0, spinBoxJoystickEventTimeout->value(), checkBoxJoystickAutoRepeat->isChecked(), spinBoxJoystickAutoRepeatTimeout->value());

		if ( joystick ) {
			if ( joystick->joystickNames.count() > 0 )
				joystickNames = joystick->joystickNames;
		} else
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: couldn't initialize SDL joystick support"));

		comboBoxSelectJoysticks->clear();
		comboBoxSelectJoysticks->insertItems(0, joystickNames);
		comboBoxSelectJoysticks->setCurrentIndex(config->value(QMC2_FRONTEND_PREFIX + "Joystick/Index", 0).toInt());
	}
}

void Options::on_toolButtonCalibrateAxes_clicked()
{
	qmc2JoystickIsCalibrating = false;

	if ( comboBoxSelectJoysticks->currentText() == tr("No joysticks found") || comboBoxSelectJoysticks->currentIndex() < 0 ) {
		toolButtonMapJoystick->setChecked(true);
		on_toolButtonMapJoystick_clicked();
		return;
	}

	if ( joystick ) {
		if ( joystick->isOpen() )
			joystick->close();
		if ( joystick->open(comboBoxSelectJoysticks->currentIndex()) ) {
			// create joystick calibration widget
			QGridLayout *myLayout = (QGridLayout *)qmc2Options->frameCalibrationAndTest->layout();
			if ( joystickTestWidget ) {
				myLayout->removeWidget(scrollArea);
				scrollArea->takeWidget();
				scrollArea->hide();
				delete joystickTestWidget;
				joystickTestWidget = 0;
			} else if ( joystickCalibrationWidget ) {
				myLayout->removeWidget(scrollArea);
				scrollArea->takeWidget();
				scrollArea->hide();
				delete joystickCalibrationWidget;
				joystickCalibrationWidget = 0;
			} else {
				myLayout->removeWidget(pushButtonRemapJoystickFunction);
				myLayout->removeWidget(pushButtonRemoveJoystickMapping);
				myLayout->removeWidget(treeWidgetJoystickMappings);
				pushButtonRemapJoystickFunction->hide();
				pushButtonRemoveJoystickMapping->hide();
				treeWidgetJoystickMappings->hide();
			}
			joystickCalibrationWidget = new JoystickCalibrationWidget(joystick, frameCalibrationAndTest);
			myLayout->addWidget(scrollArea);
			scrollArea->setWidget(joystickCalibrationWidget);
			scrollArea->show();
			joystickCalibrationWidget->show();
			qmc2JoystickIsCalibrating = true;
		} else {
			toolButtonMapJoystick->setChecked(true);
			on_toolButtonMapJoystick_clicked();
		}
	} else {
		toolButtonMapJoystick->setChecked(true);
		on_toolButtonMapJoystick_clicked();
	}
}

void Options::on_toolButtonTestJoystick_clicked()
{
	qmc2JoystickIsCalibrating = false;

	if ( comboBoxSelectJoysticks->currentText() == tr("No joysticks found") || comboBoxSelectJoysticks->currentIndex() < 0 ) {
		toolButtonMapJoystick->setChecked(true);
		on_toolButtonMapJoystick_clicked();
		return;
	}

	if ( joystick ) {
		if ( joystick->isOpen() )
			joystick->close();
		if ( joystick->open(comboBoxSelectJoysticks->currentIndex()) ) {
			// create joystick test widget
			QGridLayout *myLayout = (QGridLayout *)qmc2Options->frameCalibrationAndTest->layout();
			if ( joystickTestWidget ) {
				myLayout->removeWidget(scrollArea);
				scrollArea->takeWidget();
				scrollArea->hide();
				delete joystickTestWidget;
				joystickTestWidget = 0;
			} else if ( joystickCalibrationWidget ) {
				myLayout->removeWidget(scrollArea);
				scrollArea->takeWidget();
				scrollArea->hide();
				delete joystickCalibrationWidget;
				joystickCalibrationWidget = 0;
			} else {
				myLayout->removeWidget(pushButtonRemapJoystickFunction);
				myLayout->removeWidget(pushButtonRemoveJoystickMapping);
				myLayout->removeWidget(treeWidgetJoystickMappings);
				pushButtonRemapJoystickFunction->hide();
				pushButtonRemoveJoystickMapping->hide();
				treeWidgetJoystickMappings->hide();
			}
			joystickTestWidget = new JoystickTestWidget(joystick, frameCalibrationAndTest);
			myLayout->addWidget(scrollArea);
			scrollArea->setWidget(joystickTestWidget);
			scrollArea->show();
			joystickTestWidget->show();
		} else {
			toolButtonMapJoystick->setChecked(true);
			on_toolButtonMapJoystick_clicked();
		}
	} else {
		toolButtonMapJoystick->setChecked(true);
		on_toolButtonMapJoystick_clicked();
	}
}

void Options::on_toolButtonMapJoystick_clicked()
{
	qmc2JoystickIsCalibrating = false;

	bool relayout = ( joystickCalibrationWidget || joystickTestWidget );
  
	if ( joystickCalibrationWidget ) {
		frameCalibrationAndTest->layout()->removeWidget(scrollArea);
		scrollArea->takeWidget();
		scrollArea->hide();
		delete joystickCalibrationWidget;
		joystickCalibrationWidget = 0;
	}
	if ( joystickTestWidget ) {
		frameCalibrationAndTest->layout()->removeWidget(scrollArea);
		scrollArea->takeWidget();
		scrollArea->hide();
		delete joystickTestWidget;
		joystickTestWidget = 0;
	}

	if ( relayout ) {
		QGridLayout *myLayout = (QGridLayout *)qmc2Options->frameCalibrationAndTest->layout();
		myLayout->addWidget(pushButtonRemapJoystickFunction, 0, 0);
		myLayout->addWidget(pushButtonRemoveJoystickMapping, 0, 1);
		myLayout->addWidget(treeWidgetJoystickMappings, 1, 0, 1, 2);
		pushButtonRemapJoystickFunction->show();
		pushButtonRemoveJoystickMapping->show();
		treeWidgetJoystickMappings->show();
	}
}

void Options::on_comboBoxSelectJoysticks_currentIndexChanged(int index)
{
	if ( comboBoxSelectJoysticks->currentText() == tr("No joysticks found") || index < 0 ) {
		labelJoystickAxesNum->setText("0");
		labelJoystickButtonsNum->setText("0");
		labelJoystickHatsNum->setText("0");
		labelJoystickTrackballsNum->setText("0");
		return;
	}

	if ( joystick )
		if ( joystick->open(index) ) {
			labelJoystickAxesNum->setText(QString::number(joystick->numAxes));
			labelJoystickButtonsNum->setText(QString::number(joystick->numButtons));
			labelJoystickHatsNum->setText(QString::number(joystick->numHats));
			labelJoystickTrackballsNum->setText(QString::number(joystick->numTrackballs));
			if ( qmc2MainWindow ) {
				// (re)connect joystick callbacks to main widget
				joystick->disconnect(qmc2MainWindow);
				connect(joystick, SIGNAL(axisValueChanged(int, int)), qmc2MainWindow, SLOT(joystickAxisValueChanged(int, int)));
				connect(joystick, SIGNAL(buttonValueChanged(int, bool)), qmc2MainWindow, SLOT(joystickButtonValueChanged(int, bool)));
				connect(joystick, SIGNAL(hatValueChanged(int, int)), qmc2MainWindow, SLOT(joystickHatValueChanged(int, int)));
				connect(joystick, SIGNAL(trackballValueChanged(int, int, int)), qmc2MainWindow, SLOT(joystickTrackballValueChanged(int, int, int)));
			}
		}
}

void Options::on_checkBoxEnableJoystickControl_toggled(bool enable)
{
	toolButtonMapJoystick->setChecked(true);
	on_toolButtonMapJoystick_clicked();
}

void Options::on_checkBoxJoystickAutoRepeat_toggled(bool repeat)
{
	if ( joystick )
		joystick->autoRepeat = repeat;
}

void Options::on_spinBoxJoystickAutoRepeatTimeout_valueChanged(int value)
{
	if ( joystick )
		joystick->autoRepeatDelay = value;
}

void Options::on_spinBoxJoystickEventTimeout_valueChanged(int value)
{
	if ( joystick ) {
		joystick->eventTimeout = value;
		if ( joystick->isOpen() )
			joystick->joystickTimer.start(joystick->eventTimeout);
	}
}

void Options::on_treeWidgetJoystickMappings_itemActivated(QTreeWidgetItem *item)
{
	if ( !item )
		return;

	if ( joystick ) {
		if ( joystick->isOpen() )
			joystick->close();
		if ( joystick->open(comboBoxSelectJoysticks->currentIndex()) ) {
			// suppress strange Qt warning messages (this works - basta! :)
			bool saveSQM = qmc2SuppressQtMessages;
			qmc2SuppressQtMessages = true;
			JoystickFunctionScanner joyFuncScanner(joystick, false, this);
			if ( joyFuncScanner.exec() == QDialog::Accepted ) {
				item->setText(1, joyFuncScanner.labelJoystickFunction->text());
				qmc2JoystickFunctionHash.insertMulti(joyFuncScanner.labelJoystickFunction->text(), item->whatsThis(0));
				pushButtonRemoveJoystickMapping->setEnabled(item->text(1).length() > 0);
				QTimer::singleShot(0, this, SLOT(checkJoystickMappings()));
			}
			qmc2SuppressQtMessages = saveSQM;
		}
	}
}

void Options::on_treeWidgetJoystickMappings_itemSelectionChanged()
{
	QList<QTreeWidgetItem *> selItems = treeWidgetJoystickMappings->selectedItems();
	if ( selItems.count() > 0 ) {
		pushButtonRemapJoystickFunction->setEnabled(true);
		pushButtonRemoveJoystickMapping->setEnabled(selItems[0]->text(1).length() > 0);
	} else {
		pushButtonRemapJoystickFunction->setEnabled(false);
		pushButtonRemoveJoystickMapping->setEnabled(false);
	}
}

void Options::on_pushButtonRemapJoystickFunction_clicked()
{
	QList<QTreeWidgetItem *> selItems = treeWidgetJoystickMappings->selectedItems();
	if ( selItems.count() > 0 )
		on_treeWidgetJoystickMappings_itemActivated(selItems[0]);
}

void Options::on_pushButtonRemoveJoystickMapping_clicked()
{
	QList<QTreeWidgetItem *> selItems = treeWidgetJoystickMappings->selectedItems();
	if ( selItems.count() > 0 ) {
		QList<QString> valueList = qmc2JoystickFunctionHash.values(selItems[0]->text(1));
		qmc2JoystickFunctionHash.remove(selItems[0]->text(1));
		if ( valueList.count() > 1 ) {
			int i;
			QString valueToRemove = selItems[0]->whatsThis(0);
			for (i = 0; i < valueList.count(); i++)
				if ( valueList[i] != valueToRemove )
					qmc2JoystickFunctionHash.insertMulti(selItems[0]->text(1), valueList[i]);
		}
		selItems[0]->setText(1, "");
		pushButtonRemoveJoystickMapping->setEnabled(false);
		QTimer::singleShot(0, this, SLOT(checkJoystickMappings()));
	}
}

void Options::checkJoystickMappings()
{
	static char lastJoystickMappingsState = -1;
	char joystickMappingsState = 1;
	int itemCount = treeWidgetJoystickMappings->topLevelItemCount();
	for (int i = 0; i < itemCount; i++) {
		QTreeWidgetItem *iItem = treeWidgetJoystickMappings->topLevelItem(i);
		if ( !iItem->text(1).isEmpty() )
			iItem->setForeground(1, greenBrush);
	}
	for (int i = 0; i < itemCount; i++) {
		QTreeWidgetItem *iItem = treeWidgetJoystickMappings->topLevelItem(i);
		QString iMapping(iItem->text(1));
		for (int j = i + 1; j < itemCount; j++) {
			QTreeWidgetItem *jItem = treeWidgetJoystickMappings->topLevelItem(j);
			QString jMapping(jItem->text(1));
			if ( iMapping.compare(jMapping) == 0 && !jMapping.isEmpty() ) {
				iItem->setForeground(1, redBrush);
				jItem->setForeground(1, redBrush);
				joystickMappingsState = 0;
			}
		}
	}
	if ( joystickMappingsState != lastJoystickMappingsState || lastJoystickMappingsState == -1 ) {
		if ( joystickMappingsState == 1 )
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("joystick map is clean"));
		else
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: joystick map contains duplicates"));
	}
	lastJoystickMappingsState = joystickMappingsState;
}

JoystickCalibrationWidget::JoystickCalibrationWidget(Joystick *joystick, QWidget *parent)
	: QWidget(parent)
{
	myJoystick = joystick;

	int i;
	int joyIndex = qmc2Options->comboBoxSelectJoysticks->currentIndex();

	myLayout = new QGridLayout(this);

	int minButtonWidth = 0;
	for (i = 0; i < myJoystick->numAxes; i++) {
		QHBoxLayout *hLayout = new QHBoxLayout();

		bool enabled = qmc2Config->value(QString(QMC2_FRONTEND_PREFIX + "Joystick/%1/Axis%2Enabled").arg(joyIndex).arg(i), true).toBool();
		int minValue = qmc2Config->value(QString(QMC2_FRONTEND_PREFIX + "Joystick/%1/Axis%2Minimum").arg(joyIndex).arg(i), 0).toInt();
		axesMinimums[i] = minValue;
		int maxValue = qmc2Config->value(QString(QMC2_FRONTEND_PREFIX + "Joystick/%1/Axis%2Maximum").arg(joyIndex).arg(i), 0).toInt();
		axesMaximums[i] = maxValue;
		int dzValue = qmc2Config->value(QString(QMC2_FRONTEND_PREFIX + "Joystick/%1/Axis%2Deadzone").arg(joyIndex).arg(i), 0).toInt();
		int sValue = qmc2Config->value(QString(QMC2_FRONTEND_PREFIX + "Joystick/%1/Axis%2Sensitivity").arg(joyIndex).arg(i), 0).toInt();

		axesEnablers[i] = new QCheckBox(this);
		axesEnablers[i]->setChecked(enabled);
		axesEnablers[i]->setToolTip(tr("Enable/disable axis %1").arg(i));
		axesEnablers[i]->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
		connect(axesEnablers[i], SIGNAL(stateChanged(int)), this, SLOT(on_axisEnablerStateChanged(int)));
		axesButtons[i] = new QToolButton(this);
		axesButtons[i]->setText(tr("Axis %1:").arg(i));
		axesButtons[i]->setToolTip(tr("Reset calibration of axis %1").arg(i));
		axesButtons[i]->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
		axesButtons[i]->setEnabled(enabled);
		connect(axesButtons[i], SIGNAL(clicked()), this, SLOT(on_resetAxisCalibration()));
		if ( minButtonWidth < axesButtons[i]->sizeHint().width() )
			minButtonWidth = axesButtons[i]->sizeHint().width();
		axesRanges[i] = new QProgressBar(this);
		axesRanges[i]->setFormat("%v");
		axesRanges[i]->setToolTip(tr("Current value of axis %1").arg(i));
		axesRanges[i]->setRange(minValue, maxValue);
		axesRanges[i]->setValue(myJoystick->getAxisValue(i));
		axesRanges[i]->setEnabled(enabled);
		deadzoneLabels[i] = new QLabel(tr("DZ:"), this);
		axesDeadzones[i] = new QSpinBox(this);
		axesDeadzones[i]->setToolTip(tr("Deadzone of axis %1").arg(i));
		axesDeadzones[i]->setRange(0, 32767);
		axesDeadzones[i]->setSingleStep(1000);
		axesDeadzones[i]->setValue(dzValue);
		axesDeadzones[i]->setEnabled(enabled);
		connect(axesDeadzones[i], SIGNAL(valueChanged(int)), this, SLOT(on_deadzoneValueChanged(int)));
		sensitivityLabels[i] = new QLabel(tr("S:"), this);
		axesSensitivities[i] = new QSpinBox(this);
		axesSensitivities[i]->setToolTip(tr("Sensitivity of axis %1").arg(i));
		axesSensitivities[i]->setRange(0, 32767);
		axesSensitivities[i]->setSingleStep(100);
		axesSensitivities[i]->setValue(sValue);
		axesSensitivities[i]->setEnabled(enabled);
		connect(axesSensitivities[i], SIGNAL(valueChanged(int)), this, SLOT(on_sensitivityValueChanged(int)));
    
		hLayout->addWidget(axesEnablers[i]);
		hLayout->addWidget(axesButtons[i]);
		hLayout->addWidget(axesRanges[i]);
		hLayout->addWidget(deadzoneLabels[i]);
		hLayout->addWidget(axesDeadzones[i]);
		hLayout->addWidget(sensitivityLabels[i]);
		hLayout->addWidget(axesSensitivities[i]);

		myLayout->addLayout(hLayout, i, 0);
	}
	for (i = 0; i < myJoystick->numAxes; i++)
		axesButtons[i]->setMinimumWidth(minButtonWidth);
	// add spacer
	myLayout->addItem(new QSpacerItem(10, 10, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding), i, 0);

	// connect joystick callbacks
	connect(myJoystick, SIGNAL(axisValueChanged(int, int)), this, SLOT(on_joystickAxisValueChanged(int, int)));
	connect(myJoystick, SIGNAL(buttonValueChanged(int, bool)), this, SLOT(on_joystickButtonValueChanged(int, bool)));
	connect(myJoystick, SIGNAL(hatValueChanged(int, int)), this, SLOT(on_joystickHatValueChanged(int, int)));
	connect(myJoystick, SIGNAL(trackballValueChanged(int, int, int)), this, SLOT(on_joystickTrackballValueChanged(int, int, int)));
}

JoystickCalibrationWidget::~JoystickCalibrationWidget()
{
	// ignore destruction when we are already cleaning up the application...
	if ( !qmc2Options->frameCalibrationAndTest->layout() )
		return;

	// remove spacer item first
	QLayoutItem *layoutItem = (QLayoutItem *)myLayout->takeAt(myJoystick->numAxes);
	delete layoutItem;
	for (int i = myJoystick->numAxes - 1; i >= 0; i--) {
		QLayout *hLayout = (QLayout *)myLayout->takeAt(i);
		hLayout->removeWidget(axesSensitivities[i]);
		delete axesSensitivities[i];
		hLayout->removeWidget(sensitivityLabels[i]);
		delete sensitivityLabels[i];
		hLayout->removeWidget(axesDeadzones[i]);
		delete axesDeadzones[i];
		hLayout->removeWidget(deadzoneLabels[i]);
		delete deadzoneLabels[i];
		hLayout->removeWidget(axesRanges[i]);
		delete axesRanges[i];
		hLayout->removeWidget(axesButtons[i]);
		delete axesButtons[i];
		hLayout->removeWidget(axesEnablers[i]);
		delete axesEnablers[i];
		delete hLayout;
	}
}

void JoystickCalibrationWidget::on_joystickAxisValueChanged(int axis, int value)
{
	int joyIndex = qmc2Options->comboBoxSelectJoysticks->currentIndex();

	if ( value > axesMaximums[axis] ) {
		axesMaximums[axis] = value;
		qmc2Config->setValue(QString(QMC2_FRONTEND_PREFIX + "Joystick/%1/Axis%2Maximum").arg(joyIndex).arg(axis), value);
	}

	if ( value < axesMinimums[axis] ) {
		axesMinimums[axis] = value;
		qmc2Config->setValue(QString(QMC2_FRONTEND_PREFIX + "Joystick/%1/Axis%2Minimum").arg(joyIndex).arg(axis), value);
	}

	axesRanges[axis]->setRange(axesMinimums[axis], axesMaximums[axis]);
	axesRanges[axis]->setValue(value);
}

void JoystickCalibrationWidget::on_joystickButtonValueChanged(int button, bool value)
{
}

void JoystickCalibrationWidget::on_joystickHatValueChanged(int hat, int value)
{
}

void JoystickCalibrationWidget::on_joystickTrackballValueChanged(int trackball, int deltaX, int deltaY)
{
}

void JoystickCalibrationWidget::on_resetAxisCalibration()
{
	QToolButton *pressedButton = (QToolButton *)sender();
	QList<QToolButton *> buttonList = axesButtons.values();
	int i;
	for (i = 0; i < buttonList.count() && buttonList[i] != pressedButton; i++) ;
	if ( pressedButton == buttonList[i] ) {
		int joyIndex = qmc2Options->comboBoxSelectJoysticks->currentIndex();
		axesMinimums[i] = 0;
		qmc2Config->setValue(QString(QMC2_FRONTEND_PREFIX + "Joystick/%1/Axis%2Minimum").arg(joyIndex).arg(i), 0);
		axesMaximums[i] = 0;
		qmc2Config->setValue(QString(QMC2_FRONTEND_PREFIX + "Joystick/%1/Axis%2Maximum").arg(joyIndex).arg(i), 0);
		axesRanges[i]->setRange(0, 0);
		axesRanges[i]->setValue(0);
		axesDeadzones[i]->setValue(0);
		qmc2Config->setValue(QString(QMC2_FRONTEND_PREFIX + "Joystick/%1/Axis%2Deadzone").arg(joyIndex).arg(i), 0);
		axesSensitivities[i]->setValue(0);
		qmc2Config->setValue(QString(QMC2_FRONTEND_PREFIX + "Joystick/%1/Axis%2Sensitivity").arg(joyIndex).arg(i), 0);
	}
}

void JoystickCalibrationWidget::on_deadzoneValueChanged(int value)
{
	QSpinBox *spinBox = (QSpinBox *)sender();
	QList<QSpinBox *> spinBoxList = axesDeadzones.values();
	int i;
	for (i = 0; i < spinBoxList.count() && spinBoxList[i] != spinBox; i++) ;
	if ( spinBox == spinBoxList[i] ) {
		int joyIndex = qmc2Options->comboBoxSelectJoysticks->currentIndex();
		qmc2Config->setValue(QString(QMC2_FRONTEND_PREFIX + "Joystick/%1/Axis%2Deadzone").arg(joyIndex).arg(i), value);
		myJoystick->deadzones[i] = value;
	}
}

void JoystickCalibrationWidget::on_sensitivityValueChanged(int value)
{
	QSpinBox *spinBox = (QSpinBox *)sender();
	QList<QSpinBox *> spinBoxList = axesSensitivities.values();
	int i;
	for (i = 0; i < spinBoxList.count() && spinBoxList[i] != spinBox; i++) ;
	if ( spinBox == spinBoxList[i] ) {
		int joyIndex = qmc2Options->comboBoxSelectJoysticks->currentIndex();
		qmc2Config->setValue(QString(QMC2_FRONTEND_PREFIX + "Joystick/%1/Axis%2Sensitivity").arg(joyIndex).arg(i), value);
		myJoystick->sensitivities[i] = value;
	}
}

void JoystickCalibrationWidget::on_axisEnablerStateChanged(int state)
{
	QCheckBox *checkBox = (QCheckBox *)sender();
	QList<QCheckBox *> checkBoxList = axesEnablers.values();
	int i;
	for (i = 0; i < checkBoxList.count() && checkBoxList[i] != checkBox; i++) ;
	if ( checkBox == checkBoxList[i] ) {
		bool enabled = (state == 0 ? false : true);
		int joyIndex = qmc2Options->comboBoxSelectJoysticks->currentIndex();
		qmc2Config->setValue(QString(QMC2_FRONTEND_PREFIX + "Joystick/%1/Axis%2Enabled").arg(joyIndex).arg(i), enabled);
    
		axesButtons[i]->setEnabled(enabled);
		axesRanges[i]->setEnabled(enabled);
		axesDeadzones[i]->setEnabled(enabled);
		axesSensitivities[i]->setEnabled(enabled);
	}
}

JoystickTestWidget::JoystickTestWidget(Joystick *joystick, QWidget *parent)
	: QWidget(parent)
{
	myJoystick = joystick;

	int joyIndex = qmc2Options->comboBoxSelectJoysticks->currentIndex();
	int maxRows = QMC2_MAX(QMC2_MAX(QMC2_MAX(myJoystick->numAxes, myJoystick->numButtons), myJoystick->numHats), myJoystick->numTrackballs);

	myLayout = new QGridLayout(this);

	for (int i = 0; i < myJoystick->numAxes; i++) {
		int minValue = qmc2Config->value(QString(QMC2_FRONTEND_PREFIX + "Joystick/%1/Axis%2Minimum").arg(joyIndex).arg(i), 0).toInt();
		int maxValue = qmc2Config->value(QString(QMC2_FRONTEND_PREFIX + "Joystick/%1/Axis%2Maximum").arg(joyIndex).arg(i), 0).toInt();
		int dzValue = qmc2Config->value(QString(QMC2_FRONTEND_PREFIX + "Joystick/%1/Axis%2Deadzone").arg(joyIndex).arg(i), 0).toInt();
		myJoystick->deadzones[i] = dzValue;
		int sValue = qmc2Config->value(QString(QMC2_FRONTEND_PREFIX + "Joystick/%1/Axis%2Sensitivity").arg(joyIndex).arg(i), 0).toInt();
		myJoystick->sensitivities[i] = sValue;
		bool enabled = qmc2Config->value(QString(QMC2_FRONTEND_PREFIX + "Joystick/%1/Axis%2Enabled").arg(joyIndex).arg(i), true).toBool();

		axesRanges[i] = new QProgressBar(this);
		axesRanges[i]->setFormat(tr("A%1: %v").arg(i));
		axesRanges[i]->setToolTip(tr("Current value of axis %1").arg(i));
		axesRanges[i]->setRange(minValue, maxValue);
		axesRanges[i]->setValue(myJoystick->getAxisValue(i));
		axesRanges[i]->setSizePolicy(QSizePolicy::Expanding, myJoystick->numAxes < maxRows ? QSizePolicy::Ignored : QSizePolicy::Minimum);
		axesRanges[i]->setEnabled(enabled);
    
		myLayout->addWidget(axesRanges[i], i, 0);
	}

	for (int i = 0; i < myJoystick->numButtons; i++) {
		buttonLabels[i] = new QLabel(tr("B%1").arg(i), this);
		buttonLabels[i]->setToolTip(tr("Current state of button %1").arg(i));
		buttonLabels[i]->setAlignment(Qt::AlignCenter);
		buttonLabels[i]->setAutoFillBackground(true);
		buttonLabels[i]->setFrameShape(QFrame::Box);
		buttonLabels[i]->setSizePolicy(QSizePolicy::Expanding, myJoystick->numButtons < maxRows ? QSizePolicy::Ignored : QSizePolicy::Minimum);

		myLayout->addWidget(buttonLabels[i], i, 1);
	}

	for (int i = 0; i < myJoystick->numHats; i++) {
		hatValueLabels[i] = new QLabel(tr("H%1: 0").arg(i), this);
		hatValueLabels[i]->setToolTip(tr("Current value of hat %1").arg(i));
		hatValueLabels[i]->setAlignment(Qt::AlignCenter);
		hatValueLabels[i]->setAutoFillBackground(true);
		hatValueLabels[i]->setFrameShape(QFrame::Box);
		hatValueLabels[i]->setSizePolicy(QSizePolicy::Expanding, myJoystick->numHats < maxRows ? QSizePolicy::Ignored : QSizePolicy::Minimum);

		myLayout->addWidget(hatValueLabels[i], i, 2);
	}

	for (int i = 0; i < myJoystick->numTrackballs; i++) {
		trackballDeltaXLabels[i] = new QLabel(tr("T%1 DX: 0").arg(i), this);
		trackballDeltaXLabels[i]->setToolTip(tr("Current X-delta of trackball %1").arg(i));
		trackballDeltaXLabels[i]->setAlignment(Qt::AlignCenter);
		trackballDeltaXLabels[i]->setAutoFillBackground(true);
		trackballDeltaXLabels[i]->setFrameShape(QFrame::Box);
		trackballDeltaXLabels[i]->setSizePolicy(QSizePolicy::Expanding, myJoystick->numTrackballs < maxRows ? QSizePolicy::Ignored : QSizePolicy::Minimum);

		myLayout->addWidget(trackballDeltaXLabels[i], i, 3);
	}

	for (int i = 0; i < myJoystick->numTrackballs; i++) {
		trackballDeltaYLabels[i] = new QLabel(tr("T%1 DY: 0").arg(i), this);
		trackballDeltaYLabels[i]->setToolTip(tr("Current Y-delta of trackball %1").arg(i));
		trackballDeltaYLabels[i]->setAlignment(Qt::AlignCenter);
		trackballDeltaYLabels[i]->setAutoFillBackground(true);
		trackballDeltaYLabels[i]->setFrameShape(QFrame::Box);
		trackballDeltaYLabels[i]->setSizePolicy(QSizePolicy::Expanding, myJoystick->numTrackballs < maxRows ? QSizePolicy::Ignored : QSizePolicy::Minimum);
    
		myLayout->addWidget(trackballDeltaYLabels[i], i, 4);
	}

	// add spacer
	myLayout->addItem(new QSpacerItem(10, 10, QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding), maxRows, 1);

	// connect joystick callbacks
	connect(myJoystick, SIGNAL(axisValueChanged(int, int)), this, SLOT(on_joystickAxisValueChanged(int, int)));
	connect(myJoystick, SIGNAL(buttonValueChanged(int, bool)), this, SLOT(on_joystickButtonValueChanged(int, bool)));
	connect(myJoystick, SIGNAL(hatValueChanged(int, int)), this, SLOT(on_joystickHatValueChanged(int, int)));
	connect(myJoystick, SIGNAL(trackballValueChanged(int, int, int)), this, SLOT(on_joystickTrackballValueChanged(int, int, int)));
}

JoystickTestWidget::~JoystickTestWidget()
{
	// ignore destruction when we are already cleaning up the application...
	if ( !qmc2Options->frameCalibrationAndTest->layout() )
		return;

	QLayoutItem *childItem;
	while ((childItem = myLayout->takeAt(0)) != 0) {
		delete childItem->widget();
		delete childItem;
	}
}

void JoystickTestWidget::on_joystickAxisValueChanged(int axis, int value)
{
	axesRanges[axis]->setValue(value);
}

void JoystickTestWidget::on_joystickButtonValueChanged(int button, bool value)
{
	if ( qApp->styleSheet().isEmpty() ) {
		QPalette greenPalette(QApplication::palette());
		greenPalette.setBrush(QPalette::Window, QColor(0, 255, 0));

		if ( value )
			buttonLabels[button]->setPalette(greenPalette);
		else
			buttonLabels[button]->setPalette(QApplication::palette());
	} else {
		if ( value )
			buttonLabels[button]->setStyleSheet("background: #00ff00; color: black");
		else
			buttonLabels[button]->setStyleSheet("");
	}
}

void JoystickTestWidget::on_joystickHatValueChanged(int hat, int value)
{
	if ( qApp->styleSheet().isEmpty() ) {
		QPalette greenPalette(QApplication::palette());
		greenPalette.setBrush(QPalette::Window, QColor(0, 255, 0));

		if ( value != 0 )
			hatValueLabels[hat]->setPalette(greenPalette);
		else
			hatValueLabels[hat]->setPalette(QApplication::palette());
	} else {
		if ( value != 0 )
			hatValueLabels[hat]->setStyleSheet("background: #00ff00; color: black");
		else
			hatValueLabels[hat]->setStyleSheet("");
	}

	hatValueLabels[hat]->setText(tr("H%1: %2").arg(hat).arg(value));
}

void JoystickTestWidget::on_joystickTrackballValueChanged(int trackball, int deltaX, int deltaY)
{
	if ( qApp->styleSheet().isEmpty() ) {
		QPalette greenPalette(QApplication::palette());
		greenPalette.setBrush(QPalette::Window, QColor(0, 255, 0));

		if ( deltaX != 0 )
			trackballDeltaXLabels[trackball]->setPalette(greenPalette);
		else
			trackballDeltaXLabels[trackball]->setPalette(QApplication::palette());

		if ( deltaY != 0 )
			trackballDeltaYLabels[trackball]->setPalette(greenPalette);
		else
			trackballDeltaYLabels[trackball]->setPalette(QApplication::palette());
	} else {
		if ( deltaX != 0 )
			trackballDeltaXLabels[trackball]->setStyleSheet("background: #00ff00; color: black");
		else
			trackballDeltaXLabels[trackball]->setStyleSheet("");

		if ( deltaY != 0 )
			trackballDeltaYLabels[trackball]->setStyleSheet("background: #00ff00; color: black");
		else
			trackballDeltaYLabels[trackball]->setStyleSheet("");
	}

	trackballDeltaXLabels[trackball]->setText(tr("T%1 DX: %2").arg(trackball).arg(deltaX));
	trackballDeltaYLabels[trackball]->setText(tr("T%1 DY: %2").arg(trackball).arg(deltaY));
}

void JoystickTestWidget::cleanupPalette()
{
	if ( qApp->styleSheet().isEmpty() ) {
		for (int i = 0; i < buttonLabels.count(); i++)
			buttonLabels[i]->setPalette(QApplication::palette());
		for (int i = 0; i < hatValueLabels.count(); i++)
			hatValueLabels[i]->setPalette(QApplication::palette());
		for (int i = 0; i < trackballDeltaXLabels.count(); i++)
			trackballDeltaXLabels[i]->setPalette(QApplication::palette());
		for (int i = 0; i < trackballDeltaYLabels.count(); i++)
			trackballDeltaYLabels[i]->setPalette(QApplication::palette());
	} else {
		for (int i = 0; i < buttonLabels.count(); i++)
			buttonLabels[i]->setStyleSheet("");
		for (int i = 0; i < hatValueLabels.count(); i++)
			hatValueLabels[i]->setStyleSheet("");
		for (int i = 0; i < trackballDeltaXLabels.count(); i++)
			trackballDeltaXLabels[i]->setStyleSheet("");
		for (int i = 0; i < trackballDeltaYLabels.count(); i++)
			trackballDeltaYLabels[i]->setStyleSheet("");
	}
}
#endif
