#include <QTranslator>
#include <QFileInfo>
#include <QFileDialog>
#include <QFontDialog>
#include <QMessageBox>
#include <QTimer>
#include <QMap>
#include <QMultiMap>
#include <QHash>
#include <QStyleFactory>
#include <QHeaderView>
#include <QBitArray>
#include <QAction>
#include <QPair>
#include <QLocale>
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
#include "gamelist.h"
#include "imagechecker.h"
#include "macros.h"
#include "unzip.h"
#include "keyseqscan.h"
#include "romalyzer.h"
#include "romstatusexport.h"
#include "docbrowser.h"
#include "detailsetup.h"
#include "toolbarcustomizer.h"
#include "paletteeditor.h"
#include "iconlineedit.h"
#include "mawsqdlsetup.h"
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
#if defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
#include "messdevcfg.h"
#endif
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
extern bool qmc2ShowGameName;
extern bool qmc2ShowGameNameOnlyWhenRequired;
extern bool qmc2StatesTogglesEnabled;
extern bool qmc2VariantSwitchReady;
extern int qmc2GamelistResponsiveness;
extern int qmc2UpdateDelay;
extern QTranslator *qmc2Translator;
extern QTranslator *qmc2QtTranslator;
extern EmulatorOptions *qmc2GlobalEmulatorOptions;
extern EmulatorOptions *qmc2EmulatorOptions;
extern Preview *qmc2Preview;
extern Flyer *qmc2Flyer;
extern Cabinet *qmc2Cabinet;
extern Controller *qmc2Controller;
extern Marquee *qmc2Marquee;
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
extern Title *qmc2Title;
#endif
extern PCB *qmc2PCB;
extern SoftwareSnap *qmc2SoftwareSnap;
extern Gamelist *qmc2Gamelist;
extern ImageChecker *qmc2ImageChecker;
extern ROMAlyzer *qmc2ROMAlyzer;
extern ROMStatusExporter *qmc2ROMStatusExporter;
extern DocBrowser *qmc2DocBrowser;
extern int qmc2SortCriteria;
extern Qt::SortOrder qmc2SortOrder;
extern Settings *qmc2Config;
extern QBitArray qmc2Filter;
extern QMap<QString, unzFile> qmc2IconFileMap;
extern QMap<QString, SevenZipFile *> qmc2IconFileMap7z;
extern QMap<QString, QPair<QString, QAction *> > qmc2ShortcutMap;
extern QMap<QString, QString> qmc2CustomShortcutMap;
extern KeyPressFilter *qmc2KeyPressFilter;
extern QMap<QString, QKeySequence> qmc2QtKeyMap;
extern QHash<QString, QByteArray *> qmc2GameInfoDB;
extern QHash<QString, QByteArray *> qmc2EmuInfoDB;
extern QHash<QString, QByteArray *> qmc2SoftwareInfoDB;
extern MiniWebBrowser *qmc2MAWSLookup;
extern MawsQuickDownloadSetup *qmc2MawsQuickDownloadSetup;
extern DetailSetup *qmc2DetailSetup;
extern ToolBarCustomizer *qmc2ToolBarCustomizer;
extern PaletteEditor *qmc2PaletteEditor;
extern QWidget *qmc2DetailSetupParent;
#if QMC2_JOYSTICK == 1
extern QMap<QString, QString> qmc2JoystickFunctionMap;
extern bool qmc2JoystickIsCalibrating;
#endif
#if defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
extern MESSDeviceConfigurator *qmc2MESSDeviceConfigurator;
extern MiniWebBrowser *qmc2ProjectMESS;
#endif
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
extern QList<QTreeWidgetItem *> qmc2ExpandedGamelistItems;
extern bool qmc2SortingActive;
extern SampleChecker *qmc2SampleChecker;
extern NetworkAccessManager *qmc2NetworkAccessManager;
extern QPalette qmc2CustomPalette;
extern QMap<QString, QPalette> qmc2StandardPalettes;
extern bool qmc2CategoryInfoUsed;
extern bool qmc2VersionInfoUsed;
#if defined(QMC2_EMUTYPE_UME)
extern QMultiMap<QString, QString> qmc2GameInfoSourceMap;
#endif

QBrush Options::greenBrush(QColor(0, 255, 0));
QBrush Options::yellowBrush(QColor(255, 255, 0));
QBrush Options::blueBrush(QColor(0, 0, 255));
QBrush Options::redBrush(QColor(255, 0, 0));
QBrush Options::greyBrush(QColor(128, 128, 128));
QBrush Options::lightgreyBrush(QColor(200, 200, 200));

QString qmc2StandardWorkDir;
QString qmc2CurrentStyleName;

Options::Options(QWidget *parent)
#if defined(QMC2_OS_WIN)
  : QDialog(parent, Qt::Dialog)
#else
  : QDialog(parent, Qt::Dialog | Qt::SubWindow)
#endif
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Options::Options(QWidget *parent = %1)").arg((qulonglong)parent));
#endif

  qmc2Filter.resize(QMC2_ROMSTATE_COUNT);

  QCoreApplication::setOrganizationName(QMC2_ORGANIZATION_NAME);
  QCoreApplication::setOrganizationDomain(QMC2_ORGANIZATION_DOMAIN);
  QCoreApplication::setApplicationName(QMC2_VARIANT_NAME);

#if !defined(QMC2_OS_WIN)
  QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, QMC2_SYSCONF_PATH);
#endif
  QString userScopePath = QMC2_DYNAMIC_DOT_PATH;
  QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, userScopePath);
  QDir userScopeDir(userScopePath);
  if ( !userScopeDir.exists() )
    userScopeDir.mkdir(userScopePath);

  config = new Settings(QSettings::IniFormat, QSettings::UserScope, "qmc2");

  QWebSettings::enablePersistentStorage(userScopePath);

  setupUi(this);

  cancelClicked = false;

#if !defined(QMC2_WIP_ENABLED)
  // FIXME: remove WIP clause when "additional artwork support" is working
  pushButtonAdditionalArtworkSetup->setVisible(false);
#endif

  qmc2StandardWorkDir = QDir::currentPath();

#if !defined(QMC2_OS_MAC)
  checkBoxUnifiedTitleAndToolBarOnMac->setVisible(false);
#endif

#if !defined(QMC2_OS_UNIX) && !defined(QMC2_OS_WIN)
  labelDefaultLaunchMode->setVisible(false);
  comboBoxDefaultLaunchMode->setVisible(false);
#endif

#if !defined(QMC2_MEMORY_INFO_ENABLED)
  checkBoxMemoryIndicator->setVisible(false);
#endif

#if !defined(QMC2_VARIANT_LAUNCHER)
  checkBoxMinimizeOnVariantLaunch->setVisible(false);
  checkBoxExitOnVariantLaunch->setVisible(false);
#endif

#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
  checkBoxMinimizeOnEmuLaunch->setToolTip(tr("Minimize when launching (non-embedded) emulators?"));
#endif

#if !defined(QMC2_VARIANT_LAUNCHER) || !defined(QMC2_OS_WIN)
  labelMAMEVariantExe->setVisible(false);
  lineEditMAMEVariantExe->setVisible(false);
  toolButtonBrowseMAMEVariantExe->setVisible(false);
  labelMESSVariantExe->setVisible(false);
  lineEditMESSVariantExe->setVisible(false);
  toolButtonBrowseMESSVariantExe->setVisible(false);
  labelUMEVariantExe->setVisible(false);
  lineEditUMEVariantExe->setVisible(false);
  toolButtonBrowseUMEVariantExe->setVisible(false);
#else
#if defined(QMC2_EMUTYPE_MESS)
  labelMESSVariantExe->setVisible(false);
  lineEditMESSVariantExe->setVisible(false);
  toolButtonBrowseMESSVariantExe->setVisible(false);
  QMenu *variantMAMEMenu = new QMenu(0);
  QAction *variantMAMEAction = variantMAMEMenu->addAction(tr("Specify arguments..."));
  connect(variantMAMEAction, SIGNAL(triggered()), this, SLOT(mameVariantSpecifyArguments()));
  variantMAMEMenu->addSeparator();
  connect(variantMAMEMenu->addAction(tr("Reset to default (same path assumed)")), SIGNAL(triggered()), lineEditMAMEVariantExe, SLOT(clear()));
  toolButtonBrowseMAMEVariantExe->setMenu(variantMAMEMenu);
  QMenu *variantUMEMenu = new QMenu(0);
  QAction *variantUMEAction = variantUMEMenu->addAction(tr("Specify arguments..."));
  connect(variantUMEAction, SIGNAL(triggered()), this, SLOT(umeVariantSpecifyArguments()));
  variantUMEMenu->addSeparator();
  connect(variantUMEMenu->addAction(tr("Reset to default (same path assumed)")), SIGNAL(triggered()), lineEditUMEVariantExe, SLOT(clear()));
  toolButtonBrowseUMEVariantExe->setMenu(variantUMEMenu);
#elif defined(QMC2_EMUTYPE_MAME)
  labelMAMEVariantExe->setVisible(false);
  lineEditMAMEVariantExe->setVisible(false);
  toolButtonBrowseMAMEVariantExe->setVisible(false);
  QMenu *variantMESSMenu = new QMenu(0);
  QAction *variantMESSAction = variantMESSMenu->addAction(tr("Specify arguments..."));
  connect(variantMESSAction, SIGNAL(triggered()), this, SLOT(messVariantSpecifyArguments()));
  variantMESSMenu->addSeparator();
  connect(variantMESSMenu->addAction(tr("Reset to default (same path assumed)")), SIGNAL(triggered()), lineEditMESSVariantExe, SLOT(clear()));
  toolButtonBrowseMESSVariantExe->setMenu(variantMESSMenu);
  QMenu *variantUMEMenu = new QMenu(0);
  QAction *variantUMEAction = variantUMEMenu->addAction(tr("Specify arguments..."));
  connect(variantUMEAction, SIGNAL(triggered()), this, SLOT(umeVariantSpecifyArguments()));
  variantUMEMenu->addSeparator();
  connect(variantUMEMenu->addAction(tr("Reset to default (same path assumed)")), SIGNAL(triggered()), lineEditUMEVariantExe, SLOT(clear()));
  toolButtonBrowseUMEVariantExe->setMenu(variantUMEMenu);
#elif defined(QMC2_EMUTYPE_UME)
  labelUMEVariantExe->setVisible(false);
  lineEditUMEVariantExe->setVisible(false);
  toolButtonBrowseUMEVariantExe->setVisible(false);
  QMenu *variantMAMEMenu = new QMenu(0);
  QAction *variantMAMEAction = variantMAMEMenu->addAction(tr("Specify arguments..."));
  connect(variantMAMEAction, SIGNAL(triggered()), this, SLOT(mameVariantSpecifyArguments()));
  variantMAMEMenu->addSeparator();
  connect(variantMAMEMenu->addAction(tr("Reset to default (same path assumed)")), SIGNAL(triggered()), lineEditMAMEVariantExe, SLOT(clear()));
  toolButtonBrowseMAMEVariantExe->setMenu(variantMAMEMenu);
  QMenu *variantMESSMenu = new QMenu(0);
  QAction *variantMESSAction = variantMESSMenu->addAction(tr("Specify arguments..."));
  connect(variantMESSAction, SIGNAL(triggered()), this, SLOT(messVariantSpecifyArguments()));
  variantMESSMenu->addSeparator();
  connect(variantMESSMenu->addAction(tr("Reset to default (same path assumed)")), SIGNAL(triggered()), lineEditMESSVariantExe, SLOT(clear()));
  toolButtonBrowseMESSVariantExe->setMenu(variantMESSMenu);
#endif
#endif

#if defined(QMC2_EMUTYPE_MESS)
  tabWidgetFrontendSettings->setTabText(QMC2_OPTIONS_FE_MACHINELIST_INDEX, tr("Machine &list"));
  comboBoxSortCriteria->setItemText(QMC2_SORTCRITERIA_DESCRIPTION, tr("Machine description"));
  comboBoxSortCriteria->setItemText(QMC2_SORTCRITERIA_MACHINENAME, tr("Machine name"));
  spinBoxResponsiveness->setToolTip(tr("Number of item insertions between machine list updates during reload (higher means faster, but makes the GUI less responsive)"));
  spinBoxUpdateDelay->setToolTip(tr("Delay update of any machine details (preview, flyer, info, configuration, ...) by how many milliseconds?"));
  checkBoxScaledTitle->setVisible(false);
  checkBoxScaledMarquee->setText(tr("Scaled logo"));
  radioButtonMarqueeSelect->setText(tr("Logo directory"));
  radioButtonMarqueeSelect->setToolTip(tr("Switch between specifying a logo directory or a compressed logo file"));
  lineEditMarqueeDirectory->setToolTip(tr("Logo directory (read)"));
  lineEditMarqueeFile->setToolTip(tr("Compressed logo file (read)"));
  toolButtonBrowseMarqueeDirectory->setToolTip(tr("Browse logo directory"));
  toolButtonBrowseMarqueeFile->setToolTip(tr("Browse compressed logo file"));
  radioButtonTitleSelect->setVisible(false);
  stackedWidgetTitle->setVisible(false);
  labelMAWSCacheDirectory->setVisible(false);
  lineEditMAWSCacheDirectory->setVisible(false);
  toolButtonBrowseMAWSCacheDirectory->setVisible(false);
  checkBoxUseCatverIni->setVisible(false);
  lineEditCatverIniFile->setVisible(false);
  toolButtonBrowseCatverIniFile->setVisible(false);
  labelLegendFrontendFilesAndDirectories->setToolTip(tr("Option requires a reload of the entire machine list to take effect"));
  labelLegendEmulatorFilesAndDirectories->setToolTip(tr("Option requires a reload of the entire machine list to take effect"));
  tableWidgetRegisteredEmulators->setToolTip(tr("Registered emulators -- you may select one of these in the machine-specific emulator configuration"));
  checkBoxSaveGameSelection->setText(tr("Save machine selection"));
  checkBoxSaveGameSelection->setToolTip(tr("Save machine selection on exit and before reloading the machine list"));
  checkBoxRestoreGameSelection->setText(tr("Restore machine selection"));
  checkBoxRestoreGameSelection->setToolTip(tr("Restore saved machine selection at start and after reloading the machine list"));
  labelGamelistCacheFile->setText(tr("Machine list cache"));
  lineEditGamelistCacheFile->setToolTip(tr("Machine list cache file (write)"));
  toolButtonBrowseGamelistCacheFile->setToolTip(tr("Browse machine list cache file"));
  checkBoxProcessMessSysinfoDat->setText(tr("Machine info DB"));
  checkBoxProcessMameHistoryDat->setVisible(false);
  toolButtonCompressMameHistoryDat->setVisible(false);
  toolButtonBrowseMameHistoryDat->setVisible(false);
  lineEditMameHistoryDat->setVisible(false);
  checkBoxProcessMessInfoDat->setText(tr("Emu info DB"));
  checkBoxProcessMameInfoDat->setVisible(false);
  toolButtonCompressMameInfoDat->setVisible(false);
  lineEditMameInfoDat->setVisible(false);
  toolButtonBrowseMameInfoDat->setVisible(false);
#endif
#if defined(QMC2_EMUTYPE_MAME)
  labelGeneralSoftwareFolder->setVisible(false);
  lineEditGeneralSoftwareFolder->setVisible(false);
  toolButtonBrowseGeneralSoftwareFolder->setVisible(false);
  checkBoxUseCategoryIni->setVisible(false);
  lineEditCategoryIniFile->setVisible(false);
  toolButtonBrowseCategoryIniFile->setVisible(false);
  labelSlotInfoCacheFile->setVisible(false);
  lineEditSlotInfoCacheFile->setVisible(false);
  toolButtonBrowseSlotInfoCacheFile->setVisible(false);
  checkBoxProcessMameHistoryDat->setText(tr("Game info DB"));
  checkBoxProcessMessSysinfoDat->setVisible(false);
  toolButtonCompressMessSysinfoDat->setVisible(false);
  lineEditMessSysinfoDat->setVisible(false);
  toolButtonBrowseMessSysinfoDat->setVisible(false);
  checkBoxProcessMameInfoDat->setText(tr("Emu info DB"));
  checkBoxProcessMessInfoDat->setVisible(false);
  toolButtonCompressMessInfoDat->setVisible(false);
  lineEditMessInfoDat->setVisible(false);
  toolButtonBrowseMessInfoDat->setVisible(false);
#endif
#if defined(QMC2_EMUTYPE_UME)
  labelMAWSCacheDirectory->setVisible(false);
  lineEditMAWSCacheDirectory->setVisible(false);
  toolButtonBrowseMAWSCacheDirectory->setVisible(false);
#endif
  comboBoxSortCriteria->insertItem(QMC2_SORTCRITERIA_CATEGORY, tr("Category"));
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
  comboBoxSortCriteria->insertItem(QMC2_SORTCRITERIA_VERSION, tr("Version"));
#endif

  // shortcuts
  qmc2ShortcutMap["Ctrl+1"] = QPair<QString, QAction *>(tr("Check all ROM states"), NULL);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
  qmc2ShortcutMap["Ctrl+2"] = QPair<QString, QAction *>(tr("Check all sample sets"), NULL);
#endif
  qmc2ShortcutMap["Ctrl+3"] = QPair<QString, QAction *>(tr("Check images and icons"), NULL);
  qmc2ShortcutMap["Ctrl+B"] = QPair<QString, QAction *>(tr("About QMC2"), NULL);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
  qmc2ShortcutMap["Ctrl+D"] = QPair<QString, QAction *>(tr("Analyze current game"), NULL);
#elif defined(QMC2_EMUTYPE_MESS)
  qmc2ShortcutMap["Ctrl+D"] = QPair<QString, QAction *>(tr("Analyze current machine"), NULL);
#endif
  qmc2ShortcutMap["Ctrl+Shift+D"] = QPair<QString, QAction *>(tr("Analyze tagged sets"), NULL);
  qmc2ShortcutMap["Ctrl+E"] = QPair<QString, QAction *>(tr("Export ROM Status"), NULL);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
  qmc2ShortcutMap["Ctrl+J"] = QPair<QString, QAction *>(tr("Copy game to favorites"), NULL);
#elif defined(QMC2_EMUTYPE_MESS)
  qmc2ShortcutMap["Ctrl+J"] = QPair<QString, QAction *>(tr("Copy machine to favorites"), NULL);
#endif
  qmc2ShortcutMap["Ctrl+Shift+J"] = QPair<QString, QAction *>(tr("Copy tagged sets to favorites"), NULL);
  qmc2ShortcutMap["Ctrl+H"] = QPair<QString, QAction *>(tr("Online documentation"), NULL);
  qmc2ShortcutMap["Ctrl+I"] = QPair<QString, QAction *>(tr("Clear image cache"), NULL);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME) // FIXME: we have no MESS arcade theme yet!
  qmc2ShortcutMap["Ctrl+Shift+A"] = QPair<QString, QAction *>(tr("Setup arcade mode"), NULL);
#endif
#if defined(QMC2_EMUTYPE_MAME)
  qmc2ShortcutMap["Ctrl+M"] = QPair<QString, QAction *>(tr("Clear MAWS cache"), NULL);
#elif defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
  qmc2ShortcutMap["Ctrl+M"] = QPair<QString, QAction *>(tr("Clear ProjectMESS cache"), NULL);
#endif
  qmc2ShortcutMap["Ctrl+N"] = QPair<QString, QAction *>(tr("Clear icon cache"), NULL);
#if defined(QMC2_OS_MAC)
  qmc2ShortcutMap["Ctrl+,"] = QPair<QString, QAction *>(tr("Open options dialog"), NULL);
#else
  qmc2ShortcutMap["Ctrl+O"] = QPair<QString, QAction *>(tr("Open options dialog"), NULL);
#endif
  qmc2ShortcutMap["Ctrl+P"] = QPair<QString, QAction *>(tr("Play (independent)"), NULL);
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
  qmc2ShortcutMap["Ctrl+Shift+P"] = QPair<QString, QAction *>(tr("Play (embedded)"), NULL);
#endif
  qmc2ShortcutMap["Ctrl+Q"] = QPair<QString, QAction *>(tr("About Qt"), NULL);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
  qmc2ShortcutMap["Ctrl+R"] = QPair<QString, QAction *>(tr("Reload game list"), NULL);
  qmc2ShortcutMap["Ctrl+S"] = QPair<QString, QAction *>(tr("Check game's ROM state"), NULL);
#elif defined(QMC2_EMUTYPE_MESS)
  qmc2ShortcutMap["Ctrl+R"] = QPair<QString, QAction *>(tr("Reload machine list"), NULL);
  qmc2ShortcutMap["Ctrl+S"] = QPair<QString, QAction *>(tr("Check machine's ROM state"), NULL);
#endif
  qmc2ShortcutMap["Ctrl+Shift+S"] = QPair<QString, QAction *>(tr("Check states of tagged ROMs"), NULL);
  qmc2ShortcutMap["Ctrl+T"] = QPair<QString, QAction *>(tr("Recreate template map"), NULL);
  qmc2ShortcutMap["Ctrl+Shift+C"] = QPair<QString, QAction *>(tr("Check template map"), NULL);
  qmc2ShortcutMap["Ctrl+X"] = QPair<QString, QAction *>(tr("Stop processing / exit QMC2"), NULL);
#if defined(QMC2_YOUTUBE_ENABLED)
  qmc2ShortcutMap["Ctrl+Y"] = QPair<QString, QAction *>(tr("Clear YouTube cache"), NULL);
#endif
  qmc2ShortcutMap["Ctrl+Z"] = QPair<QString, QAction *>(tr("Open ROMAlyzer dialog"), NULL);
  qmc2ShortcutMap["Ctrl+Alt+C"] = QPair<QString, QAction *>(tr("Toggle ROM state C"), NULL);
  qmc2ShortcutMap["Ctrl+Alt+M"] = QPair<QString, QAction *>(tr("Toggle ROM state M"), NULL);
  qmc2ShortcutMap["Ctrl+Alt+I"] = QPair<QString, QAction *>(tr("Toggle ROM state I"), NULL);
  qmc2ShortcutMap["Ctrl+Alt+N"] = QPair<QString, QAction *>(tr("Toggle ROM state N"), NULL);
  qmc2ShortcutMap["Ctrl+Alt+U"] = QPair<QString, QAction *>(tr("Toggle ROM state U"), NULL);
#if defined(QMC2_VARIANT_LAUNCHER)
#if defined(QMC2_OS_WIN)
  qmc2ShortcutMap["Ctrl+Alt+1"] = QPair<QString, QAction *>(tr("Launch QMC2 for MAME"), NULL);
  qmc2ShortcutMap["Ctrl+Alt+2"] = QPair<QString, QAction *>(tr("Launch QMC2 for MESS"), NULL);
  qmc2ShortcutMap["Ctrl+Alt+3"] = QPair<QString, QAction *>(tr("Launch QMC2 for UME"), NULL);
#else
  qmc2ShortcutMap["Ctrl+Alt+1"] = QPair<QString, QAction *>(tr("Launch QMC2 for SDLMAME"), NULL);
  qmc2ShortcutMap["Ctrl+Alt+2"] = QPair<QString, QAction *>(tr("Launch QMC2 for SDLMESS"), NULL);
  qmc2ShortcutMap["Ctrl+Alt+3"] = QPair<QString, QAction *>(tr("Launch QMC2 for SDLUME"), NULL);
#endif
#endif
  qmc2ShortcutMap["Ctrl+Shift+T"] = QPair<QString, QAction *>(tr("Tag current set"), NULL);
  qmc2ShortcutMap["Ctrl+Shift+U"] = QPair<QString, QAction *>(tr("Untag current set"), NULL);
  qmc2ShortcutMap["Ctrl+Shift+G"] = QPair<QString, QAction *>(tr("Toggle tag mark"), NULL);
  qmc2ShortcutMap["Shift+Down"] = QPair<QString, QAction *>(tr("Toggle tag / cursor down"), NULL);
  qmc2ShortcutMap["Shift+Up"] = QPair<QString, QAction *>(tr("Toggle tag / cursor up"), NULL);
  qmc2ShortcutMap["Ctrl+Shift+L"] = QPair<QString, QAction *>(tr("Tag all sets"), NULL);
  qmc2ShortcutMap["Ctrl+Shift+N"] = QPair<QString, QAction *>(tr("Untag all sets"), NULL);
  qmc2ShortcutMap["Ctrl+Shift+I"] = QPair<QString, QAction *>(tr("Invert all tags"), NULL);
  qmc2ShortcutMap["Ctrl+Shift+X"] = QPair<QString, QAction *>(tr("Tag visible sets"), NULL);
  qmc2ShortcutMap["Ctrl+Shift+Y"] = QPair<QString, QAction *>(tr("Untag visible sets"), NULL);
  qmc2ShortcutMap["Ctrl+Shift+Z"] = QPair<QString, QAction *>(tr("Invert visible tags"), NULL);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
  qmc2ShortcutMap["F5"] = QPair<QString, QAction *>(tr("Game list with full detail"), NULL);
#elif defined(QMC2_EMUTYPE_MESS)
  qmc2ShortcutMap["F5"] = QPair<QString, QAction *>(tr("Machine list with full detail"), NULL);
#endif
  qmc2ShortcutMap["F6"] = QPair<QString, QAction *>(tr("Parent / clone hierarchy"), NULL);
#if defined(QMC2_EMUTYPE_MESS)
  qmc2ShortcutMap["F7"] = QPair<QString, QAction *>(tr("View machines by category"), NULL);
#elif defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
  qmc2ShortcutMap["F7"] = QPair<QString, QAction *>(tr("View games by category"), NULL);
  qmc2ShortcutMap["F8"] = QPair<QString, QAction *>(tr("View games by version"), NULL);
#endif
  qmc2ShortcutMap["F9"] = QPair<QString, QAction *>(tr("Run external ROM tool"), NULL);
  qmc2ShortcutMap["Ctrl+Shift+F9"] = QPair<QString, QAction *>(tr("Run ROM tool for tagged sets"), NULL);
  qmc2ShortcutMap["F10"] = QPair<QString, QAction *>(tr("Check software-states"), NULL);
  qmc2ShortcutMap["F11"] = QPair<QString, QAction *>(tr("Toggle full screen"), NULL);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME) // FIXME: we have no MESS arcade theme yet!
  qmc2ShortcutMap["F12"] = QPair<QString, QAction *>(tr("Launch arcade mode"), NULL);
#endif
#if QMC2_USE_PHONON_API
  qmc2ShortcutMap["Ctrl+Alt+Left"] = QPair<QString, QAction *>(tr("Previous track (audio player)"), NULL);
  qmc2ShortcutMap["Ctrl+Alt+Right"] = QPair<QString, QAction *>(tr("Next track (audio player)"), NULL);
  qmc2ShortcutMap["Ctrl+Alt+B"] = QPair<QString, QAction *>(tr("Fast backward (audio player)"), NULL);
  qmc2ShortcutMap["Ctrl+Alt+F"] = QPair<QString, QAction *>(tr("Fast forward (audio player)"), NULL);
  qmc2ShortcutMap["Ctrl+Alt+S"] = QPair<QString, QAction *>(tr("Stop track (audio player)"), NULL);
  qmc2ShortcutMap["Ctrl+Alt+#"] = QPair<QString, QAction *>(tr("Pause track (audio player)"), NULL);
  qmc2ShortcutMap["Ctrl+Alt+P"] = QPair<QString, QAction *>(tr("Play track (audio player)"), NULL);
  qmc2ShortcutMap["Ctrl+Alt+PgUp"] = QPair<QString, QAction *>(tr("Raise volume (audio player)"), NULL);
  qmc2ShortcutMap["Ctrl+Alt+PgDown"] = QPair<QString, QAction *>(tr("Lower volume (audio player)"), NULL);
#endif
  qmc2ShortcutMap["Alt+PgUp"] = QPair<QString, QAction *>(tr("Increase rank"), NULL);
  qmc2ShortcutMap["Alt+PgDown"] = QPair<QString, QAction *>(tr("Decrease rank"), NULL);

  // special keys
  qmc2ShortcutMap["+"] = QPair<QString, QAction *>(tr("Plus (+)"), NULL);
  qmc2ShortcutMap["-"] = QPair<QString, QAction *>(tr("Minus (-)"), NULL);
  qmc2ShortcutMap["Down"] = QPair<QString, QAction *>(tr("Cursor down"), NULL);
  qmc2ShortcutMap["End"] = QPair<QString, QAction *>(tr("End"), NULL);
  qmc2ShortcutMap["Enter"] = QPair<QString, QAction *>(tr("Enter key"), NULL);
  qmc2ShortcutMap["Esc"] = QPair<QString, QAction *>(tr("Escape"), NULL);
  qmc2ShortcutMap["Left"] = QPair<QString, QAction *>(tr("Cursor left"), NULL);
  qmc2ShortcutMap["Home"] = QPair<QString, QAction *>(tr("Home"), NULL);
  qmc2ShortcutMap["PgDown"] = QPair<QString, QAction *>(tr("Page down"), NULL);
  qmc2ShortcutMap["PgUp"] = QPair<QString, QAction *>(tr("Page up"), NULL);
  qmc2ShortcutMap["Return"] = QPair<QString, QAction *>(tr("Return key"), NULL);
  qmc2ShortcutMap["Right"] = QPair<QString, QAction *>(tr("Cursor right"), NULL);
  qmc2ShortcutMap["Tab"] = QPair<QString, QAction *>(tr("Tabulator"), NULL);
  qmc2ShortcutMap["Up"] = QPair<QString, QAction *>(tr("Cursor up"), NULL);
#if defined(QMC2_OS_MAC)
  qmc2ShortcutMap["Ctrl+O"] = QPair<QString, QAction *>(tr("Activate item"), NULL);
#endif

  if ( !config->isWritable() ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: configuration is not writeable, please check access permissions for ") + config->fileName());
  }

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
  lineEditMAMEVariantExe->setPlaceholderText(tr("Search in the folder we were called from"));
  lineEditMESSVariantExe->setPlaceholderText(tr("Search in the folder we were called from"));
  lineEditUMEVariantExe->setPlaceholderText(tr("Search in the folder we were called from"));

#if QMC2_JOYSTICK != 1
  tabWidgetFrontendSettings->removeTab(tabWidgetFrontendSettings->indexOf(tabFrontendJoystick));
#else
  joystick = NULL;
  joystickCalibrationWidget = NULL;
  joystickTestWidget = NULL;
  scrollArea = new QScrollArea(frameCalibrationAndTest);
  scrollArea->hide();
  scrollArea->setWidgetResizable(true);
#endif

  checkPlaceholderStatus();

  restoreCurrentConfig();
}

Options::~Options()
{
#ifdef QMC2_DEBUG
   qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::~Options()");
#endif

  if ( config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() ) {
    config->setValue(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/FrontendTab", tabWidgetFrontendSettings->currentIndex());
    config->setValue(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/MAMETab", tabWidgetGlobalMAMESetup->currentIndex());
    config->setValue(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/OptionsTab", tabWidgetOptions->currentIndex());
    config->setValue(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/ShortcutsHeaderState", treeWidgetShortcuts->header()->saveState());
#if QMC2_JOYSTICK == 1
    config->setValue(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/JoyMapHeaderState", treeWidgetJoystickMappings->header()->saveState());
#endif
    config->setValue(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/RegisteredEmulatorsHeaderState", tableWidgetRegisteredEmulators->horizontalHeader()->saveState());
  }

#if QMC2_JOYSTICK == 1
  if ( joystick )
    delete joystick;
#endif

  delete config;
  close();
}

void Options::apply()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::apply()");
#endif

#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
  if ( qmc2MainWindow->tabWidgetGamelist->currentIndex() != QMC2_EMBED_INDEX || !qmc2MainWindow->toolButtonEmbedderMaximizeToggle->isChecked() ) {
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
  qmc2MainWindow->treeWidgetGamelist->setIconSize(iconSizeMiddle);
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
  toolButtonBrowseGamelistCacheFile->setIconSize(iconSize);
  toolButtonBrowseROMStateCacheFile->setIconSize(iconSize);
  toolButtonBrowseSlotInfoCacheFile->setIconSize(iconSize);
  toolButtonBrowseDataDirectory->setIconSize(iconSize);
  toolButtonBrowseMameHistoryDat->setIconSize(iconSize);
  toolButtonBrowseMessSysinfoDat->setIconSize(iconSize);
  toolButtonBrowseMameInfoDat->setIconSize(iconSize);
  toolButtonBrowseMessInfoDat->setIconSize(iconSize);
  toolButtonBrowseSoftwareInfoDB->setIconSize(iconSize);
#if defined(QMC2_EMUTYPE_MAME)
  toolButtonBrowseMAWSCacheDirectory->setIconSize(iconSize);
#endif
  qmc2MainWindow->treeWidgetCategoryView->setIconSize(iconSizeMiddle);
#if defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
  checkBoxUseCategoryIni->setIconSize(iconSize);
  toolButtonBrowseCategoryIniFile->setIconSize(iconSize);
#endif
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
  qmc2MainWindow->treeWidgetVersionView->setIconSize(iconSizeMiddle);
  checkBoxUseCatverIni->setIconSize(iconSize);
  toolButtonBrowseCatverIniFile->setIconSize(iconSize);
#endif
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
#if defined(QMC2_VARIANT_LAUNCHER) && defined(QMC2_OS_WIN)
  toolButtonBrowseMAMEVariantExe->setIconSize(iconSize);
  toolButtonBrowseMESSVariantExe->setIconSize(iconSize);
  toolButtonBrowseUMEVariantExe->setIconSize(iconSize);
#endif
  toolButtonBrowseWorkingDirectory->setIconSize(iconSize);
  toolButtonBrowseEmulatorLogFile->setIconSize(iconSize);
  toolButtonBrowseOptionsTemplateFile->setIconSize(iconSize);
  toolButtonBrowseXmlCacheDatabase->setIconSize(iconSize);
  toolButtonBrowseUserDataDatabase->setIconSize(iconSize);
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
  checkBoxProcessMameInfoDat->setIconSize(iconSize);
  toolButtonCompressMameInfoDat->setIconSize(iconSize);
  checkBoxProcessMessInfoDat->setIconSize(iconSize);
  toolButtonCompressMessInfoDat->setIconSize(iconSize);
  checkBoxProcessMameHistoryDat->setIconSize(iconSize);
  checkBoxProcessMessSysinfoDat->setIconSize(iconSize);
  toolButtonCompressMameHistoryDat->setIconSize(iconSize);
  toolButtonCompressMessSysinfoDat->setIconSize(iconSize);
  checkBoxProcessSoftwareInfoDB->setIconSize(iconSize);
  toolButtonCompressSoftwareInfoDB->setIconSize(iconSize);
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
  if ( qmc2ROMAlyzer ) {
    qmc2ROMAlyzer->textBrowserLog->setFont(logFont);
    QTimer::singleShot(0, qmc2ROMAlyzer, SLOT(adjustIconSizes()));
  }
  if ( qmc2ImageChecker )
	  qmc2ImageChecker->adjustIconSizes();
#if defined(QMC2_EMUTYPE_MAME)
  if ( qmc2MAWSLookup ) {
    if ( qmc2MainWindow->toolButtonMAWSQuickLinks )
	    qmc2MainWindow->toolButtonMAWSQuickLinks->setIconSize(iconSize);
    if ( qmc2MawsQuickDownloadSetup )
	    QTimer::singleShot(0, qmc2MawsQuickDownloadSetup, SLOT(adjustIconSizes()));
  }
#endif
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
  if ( qmc2SampleChecker )
	  QTimer::singleShot(0, qmc2SampleChecker, SLOT(adjustIconSizes()));
#endif
#if QMC2_USE_PHONON_API
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
  if ( qmc2AudioEffectDialog )
    QTimer::singleShot(0, qmc2AudioEffectDialog, SLOT(adjustIconSizes()));
#endif
#if defined(QMC2_YOUTUBE_ENABLED)
  if ( qmc2YouTubeWidget )
    QTimer::singleShot(0, qmc2YouTubeWidget, SLOT(adjustIconSizes()));
#endif
  if ( qmc2ROMStatusExporter )
    QTimer::singleShot(0, qmc2ROMStatusExporter, SLOT(adjustIconSizes()));
  toolButtonBrowseSoftwareListCache->setIconSize(iconSize);
  toolButtonBrowseSoftwareStateCache->setIconSize(iconSize);
#if defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
  toolButtonBrowseGeneralSoftwareFolder->setIconSize(iconSize);
  if ( qmc2MESSDeviceConfigurator ) {
    qmc2MESSDeviceConfigurator->toolButtonConfiguration->setIconSize(iconSize);
    qmc2MESSDeviceConfigurator->toolButtonNewConfiguration->setIconSize(iconSize);
    qmc2MESSDeviceConfigurator->toolButtonCloneConfiguration->setIconSize(iconSize);
    qmc2MESSDeviceConfigurator->toolButtonSaveConfiguration->setIconSize(iconSize);
    qmc2MESSDeviceConfigurator->toolButtonRemoveConfiguration->setIconSize(iconSize);
    qmc2MESSDeviceConfigurator->toolButtonChooserPlay->setIconSize(iconSize);
    qmc2MESSDeviceConfigurator->toolButtonChooserPlayEmbedded->setIconSize(iconSize);
    qmc2MESSDeviceConfigurator->toolButtonChooserReload->setIconSize(iconSize);
    qmc2MESSDeviceConfigurator->toolButtonChooserClearFilterPattern->setIconSize(iconSize);
    qmc2MESSDeviceConfigurator->toolButtonChooserAutoSelect->setIconSize(iconSize);
    qmc2MESSDeviceConfigurator->toolButtonChooserFilter->setIconSize(iconSize);
    qmc2MESSDeviceConfigurator->toolButtonChooserProcessZIPs->setIconSize(iconSize);
    qmc2MESSDeviceConfigurator->toolButtonChooserMergeMaps->setIconSize(iconSize);
    qmc2MESSDeviceConfigurator->toolButtonChooserSaveConfiguration->setIconSize(iconSize);
    qmc2MESSDeviceConfigurator->comboBoxDeviceInstanceChooser->setIconSize(iconSize);
    qmc2MESSDeviceConfigurator->treeWidgetDeviceSetup->setIconSize(iconSize);
    qmc2MESSDeviceConfigurator->treeWidgetSlotOptions->setIconSize(iconSize);
    ((IconLineEdit *)qmc2MESSDeviceConfigurator->comboBoxChooserFilterPattern->lineEdit())->setIconSize(iconSizeMiddle);
  }
#endif
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
    qmc2SoftwareList->toolBoxSoftwareList->setItemIcon(QMC2_SWLIST_KNOWN_SW_PAGE, QIcon(QPixmap(QString::fromUtf8(":/data/img/flat.png")).scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    qmc2SoftwareList->toolBoxSoftwareList->setItemIcon(QMC2_SWLIST_FAVORITES_PAGE, QIcon(QPixmap(QString::fromUtf8(":/data/img/favorites.png")).scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    qmc2SoftwareList->toolBoxSoftwareList->setItemIcon(QMC2_SWLIST_SEARCH_PAGE, QIcon(QPixmap(QString::fromUtf8(":/data/img/hint.png")).scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    ((IconLineEdit *)qmc2SoftwareList->comboBoxSearch->lineEdit())->setIconSize(iconSizeMiddle);
    if ( qmc2SoftwareList->exporter ) QTimer::singleShot(0, qmc2SoftwareList->exporter, SLOT(adjustIconSizes()));
    if ( qmc2SoftwareNotesEditor ) qmc2SoftwareNotesEditor->adjustIconSizes();
    if ( qmc2SystemNotesEditor ) qmc2SystemNotesEditor->adjustIconSizes();
  }
  qmc2MainWindow->pushButtonClearFinishedDownloads->setIconSize(iconSize);
  qmc2MainWindow->pushButtonReloadSelectedDownloads->setIconSize(iconSize);
  qmc2MainWindow->pushButtonStopSelectedDownloads->setIconSize(iconSize);
  qmc2MainWindow->treeWidgetDownloads->setIconSize(iconSize);
  qmc2MainWindow->pushButtonSelectRomFilter->setIconSize(iconSize);
  qmc2MainWindow->comboBoxViewSelect->setIconSize(iconSize);

  QTabBar *tabBar = qmc2MainWindow->tabWidgetGamelist->findChild<QTabBar *>();
  if ( tabBar ) tabBar->setIconSize(iconSizeMiddle);
  tabBar = qmc2MainWindow->tabWidgetGameDetail->findChild<QTabBar *>();
  if ( tabBar ) tabBar->setIconSize(iconSizeMiddle);
  tabBar = qmc2MainWindow->tabWidgetLogsAndEmulators->findChild<QTabBar *>();
  if ( tabBar ) tabBar->setIconSize(iconSizeMiddle);
  tabBar = qmc2MainWindow->tabWidgetSoftwareDetail->findChild<QTabBar *>();
  if ( tabBar ) tabBar->setIconSize(iconSizeMiddle);

  qmc2MainWindow->toolbar->setIconSize(iconSizeLarge);

#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
  int i;
  for (i = 0; i < qmc2MainWindow->tabWidgetEmbeddedEmulators->count(); i++) {
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

  if ( qmc2DetailSetup )
    if ( qmc2DetailSetup->isVisible() )
      QTimer::singleShot(0, qmc2DetailSetup, SLOT(adjustIconSizes()));

  if ( qmc2ToolBarCustomizer )
    if ( qmc2ToolBarCustomizer->isVisible() )
      QTimer::singleShot(0, qmc2ToolBarCustomizer, SLOT(adjustIconSizes()));

  if ( qmc2GlobalEmulatorOptions )
    QTimer::singleShot(0, qmc2GlobalEmulatorOptions, SLOT(adjustIconSizes()));

  if ( qmc2EmulatorOptions )
    QTimer::singleShot(0, qmc2EmulatorOptions, SLOT(adjustIconSizes()));

  QTimer::singleShot(0, qmc2MainWindow, SLOT(updateUserData()));
}

void Options::on_pushButtonOk_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_pushButtonOk_clicked()");
#endif

  on_pushButtonApply_clicked();
}

void Options::on_pushButtonCancel_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_pushButtonCancel_clicked()");
#endif

  cancelClicked = true;
  restoreCurrentConfig();
}

void Options::on_pushButtonRestore_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_pushButtonRestore_clicked()");
#endif

  restoreCurrentConfig();
}

void Options::on_pushButtonApply_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_pushButtonApply_clicked()");
#endif

  static int oldCacheSize = 0;
  static QString oldStyleName;
  static QString oldStyleSheet;
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
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout", checkBoxSaveLayout->isChecked());
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/RestoreLayout", checkBoxRestoreLayout->isChecked());
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/SaveGameSelection", checkBoxSaveGameSelection->isChecked());
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/RestoreGameSelection", checkBoxRestoreGameSelection->isChecked());
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/Statusbar", checkBoxStatusbar->isChecked());
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/StandardColorPalette", checkBoxStandardColorPalette->isChecked());
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts", checkBoxProgressTexts->isChecked());

  bool b;

  bool invalidateGameInfoDB = false;
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
  b = checkBoxProcessMameHistoryDat->isChecked();
  needManualReload |= (config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMameHistoryDat").toBool() != b);
  invalidateGameInfoDB |= (config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMameHistoryDat").toBool() != b);
  config->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMameHistoryDat", checkBoxProcessMameHistoryDat->isChecked());
  b = toolButtonCompressMameHistoryDat->isChecked();
  needManualReload |= (config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/CompressMameHistoryDat").toBool() != b);
  invalidateGameInfoDB |= (config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/CompressMameHistoryDat").toBool() != b);
  config->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/CompressMameHistoryDat", toolButtonCompressMameHistoryDat->isChecked());
#endif
#if defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
  b = checkBoxProcessMessSysinfoDat->isChecked();
  needManualReload |= (config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMessSysinfoDat").toBool() != b);
  invalidateGameInfoDB |= (config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMessSysinfoDat").toBool() != b);
  config->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMessSysinfoDat", checkBoxProcessMessSysinfoDat->isChecked());
  b = toolButtonCompressMessSysinfoDat->isChecked();
  needManualReload |= (config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/CompressMessSysinfoDat").toBool() != b);
  invalidateGameInfoDB |= (config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/CompressMessSysinfoDat").toBool() != b);
  config->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/CompressMessSysinfoDat", toolButtonCompressMessSysinfoDat->isChecked());
#endif

  bool invalidateEmuInfoDB = false;
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
  b = checkBoxProcessMameInfoDat->isChecked();
  needManualReload |= (config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMameInfoDat").toBool() != b);
  invalidateEmuInfoDB |= (config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMameInfoDat").toBool() != b);
  config->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMameInfoDat", checkBoxProcessMameInfoDat->isChecked());
  b = toolButtonCompressMameInfoDat->isChecked();
  needManualReload |= (config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/CompressMameInfoDat").toBool() != b);
  invalidateEmuInfoDB |= (config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/CompressMameInfoDat").toBool() != b);
  config->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/CompressMameInfoDat", toolButtonCompressMameInfoDat->isChecked());
#endif
#if defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
  b = checkBoxProcessMessInfoDat->isChecked();
  needManualReload |= (config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMessInfoDat").toBool() != b);
  invalidateEmuInfoDB |= (config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMessInfoDat").toBool() != b);
  config->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMessInfoDat", checkBoxProcessMessInfoDat->isChecked());
  b = toolButtonCompressMessInfoDat->isChecked();
  needManualReload |= (config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/CompressMessInfoDat").toBool() != b);
  invalidateEmuInfoDB |= (config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/CompressMessInfoDat").toBool() != b);
  config->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/CompressMessInfoDat", toolButtonCompressMameInfoDat->isChecked());
#endif
  b = checkBoxProcessSoftwareInfoDB->isChecked();
  needManualReload |= (config->value(QMC2_FRONTEND_PREFIX + "GUI/ProcessSoftwareInfoDB").toBool() != b);
  bool invalidateSoftwareInfoDB = (config->value(QMC2_FRONTEND_PREFIX + "GUI/ProcessSoftwareInfoDB").toBool() != b);
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/ProcessSoftwareInfoDB", checkBoxProcessSoftwareInfoDB->isChecked());
  b = toolButtonCompressSoftwareInfoDB->isChecked();
  needManualReload |= (config->value(QMC2_FRONTEND_PREFIX + "GUI/CompressSoftwareInfoDB").toBool() != b);
  invalidateSoftwareInfoDB |= (config->value(QMC2_FRONTEND_PREFIX + "GUI/CompressSoftwareInfoDB").toBool() != b);
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/CompressSoftwareInfoDB", toolButtonCompressSoftwareInfoDB->isChecked());
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
  qmc2SmoothScaling = checkBoxSmoothScaling->isChecked();
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/SmoothScaling", qmc2SmoothScaling);
  qmc2RetryLoadingImages = checkBoxRetryLoadingImages->isChecked();
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/RetryLoadingImages", qmc2RetryLoadingImages);
  qmc2ParentImageFallback = checkBoxParentImageFallback->isChecked();
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/ParentImageFallback", qmc2ParentImageFallback);
  s = comboBoxLanguage->currentText().left(2).toLower();
  needRestart |= (config->value(QMC2_FRONTEND_PREFIX + "GUI/Language").toString() != s);
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
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/SetWorkDirFromExec", checkBoxSetWorkDirFromExec->isChecked());
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/GameStatusIndicator", checkBoxGameStatusIndicator->isChecked());
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/GameStatusIndicatorOnlyWhenRequired", checkBoxGameStatusIndicatorOnlyWhenRequired->isChecked());
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/ShowGameName", checkBoxShowGameName->isChecked());
  qmc2ShowGameName = checkBoxShowGameName->isChecked();
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/ShowGameNameOnlyWhenRequired", checkBoxShowGameNameOnlyWhenRequired->isChecked());
  qmc2ShowGameNameOnlyWhenRequired = checkBoxShowGameNameOnlyWhenRequired->isChecked();
  // show / hide game status indicator
  if ( config->value(QMC2_FRONTEND_PREFIX + "GUI/GameStatusIndicator").toBool() ) {
    if ( config->value(QMC2_FRONTEND_PREFIX + "GUI/GameStatusIndicatorOnlyWhenRequired").toBool() ) {
      if ( qmc2MainWindow->hSplitter->sizes()[0] == 0 || qmc2MainWindow->tabWidgetGamelist->currentIndex() != QMC2_GAMELIST_INDEX ) {
        qmc2MainWindow->labelGameStatus->setVisible(true);
      } else {
        qmc2MainWindow->labelGameStatus->setVisible(false);
      }
    } else {
      qmc2MainWindow->labelGameStatus->setVisible(true);
    }
  } else
    qmc2MainWindow->labelGameStatus->setVisible(false);
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/FrontendLogSize", spinBoxFrontendLogSize->value());
  qmc2MainWindow->textBrowserFrontendLog->setMaximumBlockCount(spinBoxFrontendLogSize->value());
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/EmulatorLogSize", spinBoxEmulatorLogSize->value());
  qmc2MainWindow->textBrowserEmulatorLog->setMaximumBlockCount(spinBoxEmulatorLogSize->value());
#if defined(QMC2_VARIANT_LAUNCHER)
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/MinimizeOnVariantLaunch", checkBoxMinimizeOnVariantLaunch->isChecked());
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/ExitOnVariantLaunch", checkBoxExitOnVariantLaunch->isChecked());
#endif

#if defined(QMC2_MEMORY_INFO_ENABLED)
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/MemoryIndicator", checkBoxMemoryIndicator->isChecked());
  if ( checkBoxMemoryIndicator->isChecked() ) {
    qmc2MainWindow->memoryUpdateTimer.start(QMC2_MEMORY_UPDATE_TIME);
    qmc2MainWindow->progressBarMemory->setVisible(true);
    qmc2MainWindow->memoryUpdateTimer_timeout();
  } else {
    qmc2MainWindow->memoryUpdateTimer.stop();
    qmc2MainWindow->progressBarMemory->setVisible(false);
  }
#endif

  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/NativeFileDialogs", checkBoxNativeFileDialogs->isChecked());

  // Files and directories
  config->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", lineEditTemporaryFile->text());
  config->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/LogFile", lineEditFrontendLogFile->text());
  config->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/DataDirectory", lineEditDataDirectory->text());

#if defined(QMC2_EMUTYPE_MAME)
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
  s = lineEditMameHistoryDat->text();
  needManualReload |= (QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MameHistoryDat").toString() != s);
  invalidateGameInfoDB |= (QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MameHistoryDat").toString() != s);
  config->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MameHistoryDat", lineEditMameHistoryDat->text());
  s = lineEditMameInfoDat->text();
  needManualReload |= (QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MameInfoDat").toString() != s);
  invalidateEmuInfoDB |= (QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MameInfoDat").toString() != s);
  config->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MameInfoDat", lineEditMameInfoDat->text());
  config->setValue("MAME/FilesAndDirectories/CatverIni", lineEditCatverIniFile->text());
#elif defined(QMC2_EMUTYPE_MESS)
  needReopenPreviewFile = (qmc2UsePreviewFile != (stackedWidgetPreview->currentIndex() == 1)) || (initialCall && qmc2UsePreviewFile);
  needReopenPreviewFile |= (QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/PreviewFile").toString() != lineEditPreviewFile->text());
  qmc2UsePreviewFile = (stackedWidgetPreview->currentIndex() == 1);
  config->setValue("MESS/FilesAndDirectories/UsePreviewFile", qmc2UsePreviewFile);
  config->setValue("MESS/FilesAndDirectories/PreviewDirectory", lineEditPreviewDirectory->text());
  config->setValue("MESS/FilesAndDirectories/PreviewFile", lineEditPreviewFile->text());
  config->setValue("MESS/FilesAndDirectories/PreviewFileType", comboBoxPreviewFileType->currentIndex());
  needReopenFlyerFile = (qmc2UseFlyerFile != (stackedWidgetFlyer->currentIndex() == 1)) || (initialCall && qmc2UseFlyerFile);
  needReopenFlyerFile |= (QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/FlyerFile").toString() != lineEditFlyerFile->text());
  qmc2UseFlyerFile = (stackedWidgetFlyer->currentIndex() == 1);
  config->setValue("MESS/FilesAndDirectories/UseFlyerFile", qmc2UseFlyerFile);
  config->setValue("MESS/FilesAndDirectories/FlyerDirectory", lineEditFlyerDirectory->text());
  config->setValue("MESS/FilesAndDirectories/FlyerFile", lineEditFlyerFile->text());
  config->setValue("MESS/FilesAndDirectories/FlyerFileType", comboBoxFlyerFileType->currentIndex());
  needReopenIconFile = (qmc2UseIconFile != (stackedWidgetIcon->currentIndex() == 1)) || (initialCall && qmc2UseIconFile);
  needReopenIconFile |= (QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/IconFile").toString() != lineEditIconFile->text());
  qmc2UseIconFile = (stackedWidgetIcon->currentIndex() == 1);
  config->setValue("MESS/FilesAndDirectories/UseIconFile", qmc2UseIconFile);
  config->setValue("MESS/FilesAndDirectories/IconDirectory", lineEditIconDirectory->text());
  config->setValue("MESS/FilesAndDirectories/IconFile", lineEditIconFile->text());
  config->setValue("MESS/FilesAndDirectories/IconFileType", comboBoxIconFileType->currentIndex());
  needReopenCabinetFile = (qmc2UseCabinetFile != (stackedWidgetCabinet->currentIndex() == 1)) || (initialCall && qmc2UseCabinetFile);
  needReopenCabinetFile |= (QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/CabinetFile").toString() != lineEditCabinetFile->text());
  qmc2UseCabinetFile = (stackedWidgetCabinet->currentIndex() == 1);
  config->setValue("MESS/FilesAndDirectories/UseCabinetFile", qmc2UseCabinetFile);
  config->setValue("MESS/FilesAndDirectories/CabinetDirectory", lineEditCabinetDirectory->text());
  config->setValue("MESS/FilesAndDirectories/CabinetFile", lineEditCabinetFile->text());
  config->setValue("MESS/FilesAndDirectories/CabinetFileType", comboBoxCabinetFileType->currentIndex());
  needReopenControllerFile = (qmc2UseControllerFile != (stackedWidgetController->currentIndex() == 1)) || (initialCall && qmc2UseControllerFile);
  needReopenControllerFile |= (QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/ControllerFile").toString() != lineEditControllerFile->text());
  qmc2UseControllerFile = (stackedWidgetController->currentIndex() == 1);
  config->setValue("MESS/FilesAndDirectories/UseControllerFile", qmc2UseControllerFile);
  config->setValue("MESS/FilesAndDirectories/ControllerDirectory", lineEditControllerDirectory->text());
  config->setValue("MESS/FilesAndDirectories/ControllerFile", lineEditControllerFile->text());
  config->setValue("MESS/FilesAndDirectories/ControllerFileType", comboBoxControllerFileType->currentIndex());
  needReopenMarqueeFile = (qmc2UseMarqueeFile != (stackedWidgetMarquee->currentIndex() == 1)) || (initialCall && qmc2UseMarqueeFile);
  needReopenMarqueeFile |= (QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/MarqueeFile").toString() != lineEditMarqueeFile->text());
  qmc2UseMarqueeFile = (stackedWidgetMarquee->currentIndex() == 1);
  config->setValue("MESS/FilesAndDirectories/UseMarqueeFile", qmc2UseMarqueeFile);
  config->setValue("MESS/FilesAndDirectories/MarqueeDirectory", lineEditMarqueeDirectory->text());
  config->setValue("MESS/FilesAndDirectories/MarqueeFile", lineEditMarqueeFile->text());
  config->setValue("MESS/FilesAndDirectories/MarqueeFileType", comboBoxMarqueeFileType->currentIndex());
  needReopenTitleFile = (qmc2UseTitleFile != (stackedWidgetTitle->currentIndex() == 1)) || (initialCall && qmc2UseTitleFile);
  needReopenTitleFile |= (QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/TitleFile").toString() != lineEditTitleFile->text());
  qmc2UseTitleFile = (stackedWidgetTitle->currentIndex() == 1);
  config->setValue("MESS/FilesAndDirectories/UseTitleFile", qmc2UseTitleFile);
  config->setValue("MESS/FilesAndDirectories/TitleDirectory", lineEditTitleDirectory->text());
  config->setValue("MESS/FilesAndDirectories/TitleFile", lineEditTitleFile->text());
  config->setValue("MESS/FilesAndDirectories/TitleFileType", comboBoxTitleFileType->currentIndex());
  needReopenPCBFile = (qmc2UsePCBFile != (stackedWidgetPCB->currentIndex() == 1)) || (initialCall && qmc2UsePCBFile);
  needReopenPCBFile |= (QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/PCBFile").toString() != lineEditPCBFile->text());
  qmc2UsePCBFile = (stackedWidgetPCB->currentIndex() == 1);
  config->setValue("MESS/FilesAndDirectories/UsePCBFile", qmc2UsePCBFile);
  config->setValue("MESS/FilesAndDirectories/PCBDirectory", lineEditPCBDirectory->text());
  config->setValue("MESS/FilesAndDirectories/PCBFile", lineEditPCBFile->text());
  config->setValue("MESS/FilesAndDirectories/PCBFileType", comboBoxPCBFileType->currentIndex());
  needReopenSoftwareSnapFile = (qmc2UseSoftwareSnapFile != (stackedWidgetSWSnap->currentIndex() == 1)) || (initialCall && qmc2UseSoftwareSnapFile);
  needReopenSoftwareSnapFile |= (QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/SoftwareSnapFile").toString() != lineEditSoftwareSnapFile->text());
  qmc2UseSoftwareSnapFile = (stackedWidgetSWSnap->currentIndex() == 1);
  config->setValue("MESS/FilesAndDirectories/UseSoftwareSnapFile", qmc2UseSoftwareSnapFile);
  config->setValue("MESS/FilesAndDirectories/SoftwareSnapDirectory", lineEditSoftwareSnapDirectory->text());
  config->setValue("MESS/FilesAndDirectories/SoftwareSnapFile", lineEditSoftwareSnapFile->text());
  config->setValue("MESS/FilesAndDirectories/SoftwareSnapFileType", comboBoxSoftwareSnapFileType->currentIndex());
  config->setValue("MESS/FilesAndDirectories/SoftwareNotesFolder", lineEditSoftwareNotesFolder->text());
  config->setValue("MESS/FilesAndDirectories/UseSoftwareNotesTemplate", checkBoxUseSoftwareNotesTemplate->isChecked());
  config->setValue("MESS/FilesAndDirectories/SoftwareNotesTemplate", lineEditSoftwareNotesTemplate->text());
  config->setValue("MESS/FilesAndDirectories/SystemNotesFolder", lineEditSystemNotesFolder->text());
  config->setValue("MESS/FilesAndDirectories/UseSystemNotesTemplate", checkBoxUseSystemNotesTemplate->isChecked());
  config->setValue("MESS/FilesAndDirectories/SystemNotesTemplate", lineEditSystemNotesTemplate->text());
  s = lineEditMessSysinfoDat->text();
  needManualReload |= (QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MessSysinfoDat").toString() != s);
  invalidateGameInfoDB |= (QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MessSysinfoDat").toString() != s);
  config->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MessSysinfoDat", lineEditMessSysinfoDat->text());
  s = lineEditMessInfoDat->text();
  needManualReload |= (QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MessInfoDat").toString() != s);
  invalidateEmuInfoDB |= (QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MessInfoDat").toString() != s);
  config->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MessInfoDat", lineEditMessInfoDat->text());
  config->setValue("MESS/FilesAndDirectories/CategoryIni", lineEditCategoryIniFile->text());
#elif defined(QMC2_EMUTYPE_UME)
  needReopenPreviewFile = (qmc2UsePreviewFile != (stackedWidgetPreview->currentIndex() == 1)) || (initialCall && qmc2UsePreviewFile);
  needReopenPreviewFile |= (QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/PreviewFile").toString() != lineEditPreviewFile->text());
  qmc2UsePreviewFile = (stackedWidgetPreview->currentIndex() == 1);
  config->setValue("UME/FilesAndDirectories/UsePreviewFile", qmc2UsePreviewFile);
  config->setValue("UME/FilesAndDirectories/PreviewDirectory", lineEditPreviewDirectory->text());
  config->setValue("UME/FilesAndDirectories/PreviewFile", lineEditPreviewFile->text());
  config->setValue("UME/FilesAndDirectories/PreviewFileType", comboBoxPreviewFileType->currentIndex());
  needReopenFlyerFile = (qmc2UseFlyerFile != (stackedWidgetFlyer->currentIndex() == 1)) || (initialCall && qmc2UseFlyerFile);
  needReopenFlyerFile |= (QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/FlyerFile").toString() != lineEditFlyerFile->text());
  qmc2UseFlyerFile = (stackedWidgetFlyer->currentIndex() == 1);
  config->setValue("UME/FilesAndDirectories/UseFlyerFile", qmc2UseFlyerFile);
  config->setValue("UME/FilesAndDirectories/FlyerDirectory", lineEditFlyerDirectory->text());
  config->setValue("UME/FilesAndDirectories/FlyerFile", lineEditFlyerFile->text());
  config->setValue("UME/FilesAndDirectories/FlyerFileType", comboBoxFlyerFileType->currentIndex());
  needReopenIconFile = (qmc2UseIconFile != (stackedWidgetIcon->currentIndex() == 1)) || (initialCall && qmc2UseIconFile);
  needReopenIconFile |= (QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/IconFile").toString() != lineEditIconFile->text());
  qmc2UseIconFile = (stackedWidgetIcon->currentIndex() == 1);
  config->setValue("UME/FilesAndDirectories/UseIconFile", qmc2UseIconFile);
  config->setValue("UME/FilesAndDirectories/IconDirectory", lineEditIconDirectory->text());
  config->setValue("UME/FilesAndDirectories/IconFile", lineEditIconFile->text());
  config->setValue("UME/FilesAndDirectories/IconFileType", comboBoxIconFileType->currentIndex());
  needReopenCabinetFile = (qmc2UseCabinetFile != (stackedWidgetCabinet->currentIndex() == 1)) || (initialCall && qmc2UseCabinetFile);
  needReopenCabinetFile |= (QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/CabinetFile").toString() != lineEditCabinetFile->text());
  qmc2UseCabinetFile = (stackedWidgetCabinet->currentIndex() == 1);
  config->setValue("UME/FilesAndDirectories/UseCabinetFile", qmc2UseCabinetFile);
  config->setValue("UME/FilesAndDirectories/CabinetDirectory", lineEditCabinetDirectory->text());
  config->setValue("UME/FilesAndDirectories/CabinetFile", lineEditCabinetFile->text());
  config->setValue("UME/FilesAndDirectories/CabinetFileType", comboBoxCabinetFileType->currentIndex());
  needReopenControllerFile = (qmc2UseControllerFile != (stackedWidgetController->currentIndex() == 1)) || (initialCall && qmc2UseControllerFile);
  needReopenControllerFile |= (QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/ControllerFile").toString() != lineEditControllerFile->text());
  qmc2UseControllerFile = (stackedWidgetController->currentIndex() == 1);
  config->setValue("UME/FilesAndDirectories/UseControllerFile", qmc2UseControllerFile);
  config->setValue("UME/FilesAndDirectories/ControllerDirectory", lineEditControllerDirectory->text());
  config->setValue("UME/FilesAndDirectories/ControllerFile", lineEditControllerFile->text());
  config->setValue("UME/FilesAndDirectories/ControllerFileType", comboBoxControllerFileType->currentIndex());
  needReopenMarqueeFile = (qmc2UseMarqueeFile != (stackedWidgetMarquee->currentIndex() == 1)) || (initialCall && qmc2UseMarqueeFile);
  needReopenMarqueeFile |= (QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/MarqueeFile").toString() != lineEditMarqueeFile->text());
  qmc2UseMarqueeFile = (stackedWidgetMarquee->currentIndex() == 1);
  config->setValue("UME/FilesAndDirectories/UseMarqueeFile", qmc2UseMarqueeFile);
  config->setValue("UME/FilesAndDirectories/MarqueeDirectory", lineEditMarqueeDirectory->text());
  config->setValue("UME/FilesAndDirectories/MarqueeFile", lineEditMarqueeFile->text());
  config->setValue("UME/FilesAndDirectories/MarqueeFileType", comboBoxMarqueeFileType->currentIndex());
  needReopenTitleFile = (qmc2UseTitleFile != (stackedWidgetTitle->currentIndex() == 1)) || (initialCall && qmc2UseTitleFile);
  needReopenTitleFile |= (QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/TitleFile").toString() != lineEditTitleFile->text());
  qmc2UseTitleFile = (stackedWidgetTitle->currentIndex() == 1);
  config->setValue("UME/FilesAndDirectories/UseTitleFile", qmc2UseTitleFile);
  config->setValue("UME/FilesAndDirectories/TitleDirectory", lineEditTitleDirectory->text());
  config->setValue("UME/FilesAndDirectories/TitleFile", lineEditTitleFile->text());
  config->setValue("UME/FilesAndDirectories/TitleFileType", comboBoxTitleFileType->currentIndex());
  needReopenPCBFile = (qmc2UsePCBFile != (stackedWidgetPCB->currentIndex() == 1)) || (initialCall && qmc2UsePCBFile);
  needReopenPCBFile |= (QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/PCBFile").toString() != lineEditPCBFile->text());
  qmc2UsePCBFile = (stackedWidgetPCB->currentIndex() == 1);
  config->setValue("UME/FilesAndDirectories/UsePCBFile", qmc2UsePCBFile);
  config->setValue("UME/FilesAndDirectories/PCBDirectory", lineEditPCBDirectory->text());
  config->setValue("UME/FilesAndDirectories/PCBFile", lineEditPCBFile->text());
  config->setValue("UME/FilesAndDirectories/PCBFileType", comboBoxPCBFileType->currentIndex());
  needReopenSoftwareSnapFile = (qmc2UseSoftwareSnapFile != (stackedWidgetSWSnap->currentIndex() == 1)) || (initialCall && qmc2UseSoftwareSnapFile);
  needReopenSoftwareSnapFile |= (QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/SoftwareSnapFile").toString() != lineEditSoftwareSnapFile->text());
  qmc2UseSoftwareSnapFile = (stackedWidgetSWSnap->currentIndex() == 1);
  config->setValue("UME/FilesAndDirectories/UseSoftwareSnapFile", qmc2UseSoftwareSnapFile);
  config->setValue("UME/FilesAndDirectories/SoftwareSnapDirectory", lineEditSoftwareSnapDirectory->text());
  config->setValue("UME/FilesAndDirectories/SoftwareSnapFile", lineEditSoftwareSnapFile->text());
  config->setValue("UME/FilesAndDirectories/SoftwareSnapFileType", comboBoxSoftwareSnapFileType->currentIndex());
  config->setValue("UME/FilesAndDirectories/SoftwareNotesFolder", lineEditSoftwareNotesFolder->text());
  config->setValue("UME/FilesAndDirectories/UseSoftwareNotesTemplate", checkBoxUseSoftwareNotesTemplate->isChecked());
  config->setValue("UME/FilesAndDirectories/SoftwareNotesTemplate", lineEditSoftwareNotesTemplate->text());
  config->setValue("UME/FilesAndDirectories/SystemNotesFolder", lineEditSystemNotesFolder->text());
  config->setValue("UME/FilesAndDirectories/UseSystemNotesTemplate", checkBoxUseSystemNotesTemplate->isChecked());
  config->setValue("UME/FilesAndDirectories/SystemNotesTemplate", lineEditSystemNotesTemplate->text());
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
  config->setValue("UME/FilesAndDirectories/CatverIni", lineEditCatverIniFile->text());
  config->setValue("UME/FilesAndDirectories/CategoryIni", lineEditCategoryIniFile->text());
#endif
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

#if defined(QMC2_EMUTYPE_MAME)
  bool catverUsed = checkBoxUseCatverIni->isChecked();
  bool categoryUsed = false;
  needReload |= (config->value(QMC2_FRONTEND_PREFIX + "Gamelist/UseCatverIni", false).toBool() != catverUsed );
  config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/UseCatverIni", catverUsed);
#elif defined(QMC2_EMUTYPE_MESS)
  bool catverUsed = false;
  bool categoryUsed = checkBoxUseCategoryIni->isChecked();
  needReload |= (config->value(QMC2_FRONTEND_PREFIX + "Gamelist/UseCategoryIni", false).toBool() != categoryUsed );
  config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/UseCategoryIni", categoryUsed);
#elif defined(QMC2_EMUTYPE_UME)
  bool catverUsed = checkBoxUseCatverIni->isChecked();
  needReload |= (config->value(QMC2_FRONTEND_PREFIX + "Gamelist/UseCatverIni", false).toBool() != catverUsed );
  config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/UseCatverIni", catverUsed);
  bool categoryUsed = checkBoxUseCategoryIni->isChecked();
  needReload |= (config->value(QMC2_FRONTEND_PREFIX + "Gamelist/UseCategoryIni", false).toBool() != categoryUsed );
  config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/UseCategoryIni", categoryUsed);
#endif
  qmc2CategoryInfoUsed = catverUsed | categoryUsed;
  qmc2VersionInfoUsed = catverUsed;

  if ( catverUsed || categoryUsed ) {
    qmc2MainWindow->treeWidgetGamelist->showColumn(QMC2_GAMELIST_COLUMN_CATEGORY);
    qmc2MainWindow->treeWidgetHierarchy->showColumn(QMC2_GAMELIST_COLUMN_CATEGORY);
    qmc2MainWindow->actionViewByCategory->setVisible(true);
    qmc2MainWindow->actionViewByCategory->setEnabled(true);
    qmc2MainWindow->actionMenuGamelistHeaderCategory->setVisible(true);
    qmc2MainWindow->actionMenuGamelistHeaderCategory->setEnabled(true);
    qmc2MainWindow->actionMenuHierarchyHeaderCategory->setVisible(true);
    qmc2MainWindow->actionMenuHierarchyHeaderCategory->setEnabled(true);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
    qmc2MainWindow->treeWidgetGamelist->showColumn(QMC2_GAMELIST_COLUMN_VERSION);
    qmc2MainWindow->treeWidgetHierarchy->showColumn(QMC2_GAMELIST_COLUMN_VERSION);
    qmc2MainWindow->actionViewByVersion->setVisible(true);
    qmc2MainWindow->actionViewByVersion->setEnabled(true);
    qmc2MainWindow->actionMenuGamelistHeaderVersion->setVisible(true);
    qmc2MainWindow->actionMenuGamelistHeaderVersion->setEnabled(true);
    qmc2MainWindow->actionMenuHierarchyHeaderVersion->setVisible(true);
    qmc2MainWindow->actionMenuHierarchyHeaderVersion->setEnabled(true);
#endif
    if ( comboBoxSortCriteria->count() - 1 < QMC2_SORTCRITERIA_CATEGORY ) {
      comboBoxSortCriteria->insertItem(QMC2_SORTCRITERIA_CATEGORY, tr("Category"));
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
      comboBoxSortCriteria->insertItem(QMC2_SORTCRITERIA_VERSION, tr("Version"));
#endif
    }
    if ( qmc2MainWindow->comboBoxViewSelect->count() - 1 < QMC2_VIEWCATEGORY_INDEX ) {
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
      qmc2MainWindow->comboBoxViewSelect->insertItem(QMC2_VIEWCATEGORY_INDEX, tr("View games by category (not filtered)"));
      qmc2MainWindow->comboBoxViewSelect->setItemIcon(QMC2_VIEWCATEGORY_INDEX, QIcon(QString::fromUtf8(":/data/img/category.png")));
      qmc2MainWindow->comboBoxViewSelect->insertItem(QMC2_VIEWVERSION_INDEX, tr("View games by emulator version (not filtered)"));
      qmc2MainWindow->comboBoxViewSelect->setItemIcon(QMC2_VIEWVERSION_INDEX, QIcon(QString::fromUtf8(":/data/img/version.png")));
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2MainWindow->comboBoxViewSelect->insertItem(QMC2_VIEWCATEGORY_INDEX, tr("View machines by category (not filtered)"));
      qmc2MainWindow->comboBoxViewSelect->setItemIcon(QMC2_VIEWCATEGORY_INDEX, QIcon(QString::fromUtf8(":/data/img/category.png")));
#endif
    }
  } else {
    qmc2MainWindow->treeWidgetGamelist->hideColumn(QMC2_GAMELIST_COLUMN_CATEGORY);
    qmc2MainWindow->treeWidgetHierarchy->hideColumn(QMC2_GAMELIST_COLUMN_CATEGORY);
    qmc2MainWindow->actionViewByCategory->setVisible(false);
    qmc2MainWindow->actionViewByCategory->setEnabled(false);
    qmc2MainWindow->actionMenuGamelistHeaderCategory->setVisible(false);
    qmc2MainWindow->actionMenuGamelistHeaderCategory->setEnabled(false);
    qmc2MainWindow->actionMenuHierarchyHeaderCategory->setVisible(false);
    qmc2MainWindow->actionMenuHierarchyHeaderCategory->setEnabled(false);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
    qmc2MainWindow->treeWidgetGamelist->hideColumn(QMC2_GAMELIST_COLUMN_VERSION);
    qmc2MainWindow->treeWidgetHierarchy->hideColumn(QMC2_GAMELIST_COLUMN_VERSION);
    qmc2MainWindow->actionViewByVersion->setVisible(false);
    qmc2MainWindow->actionViewByVersion->setEnabled(false);
    qmc2MainWindow->actionMenuGamelistHeaderVersion->setVisible(false);
    qmc2MainWindow->actionMenuGamelistHeaderVersion->setEnabled(false);
    qmc2MainWindow->actionMenuHierarchyHeaderVersion->setVisible(false);
    qmc2MainWindow->actionMenuHierarchyHeaderVersion->setEnabled(false);
#endif
    if ( comboBoxSortCriteria->count() > QMC2_SORTCRITERIA_CATEGORY ) {
      comboBoxSortCriteria->removeItem(QMC2_SORTCRITERIA_CATEGORY);
      comboBoxSortCriteria->removeItem(QMC2_SORTCRITERIA_VERSION);
    }
    if ( qmc2MainWindow->comboBoxViewSelect->count() > QMC2_VIEWCATEGORY_INDEX ) {
      qmc2MainWindow->comboBoxViewSelect->removeItem(QMC2_VIEWCATEGORY_INDEX);
      qmc2MainWindow->comboBoxViewSelect->removeItem(QMC2_VIEWVERSION_INDEX);
    }
  }

  if ( qmc2ToolBarCustomizer )
	  QTimer::singleShot(0, qmc2ToolBarCustomizer, SLOT(refreshAvailableActions()));

  // Gamelist
  bool showROMStatusIcons = checkBoxShowROMStatusIcons->isChecked();
  needReload |= (config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowROMStatusIcons", true).toBool() != showROMStatusIcons );
  config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/ShowROMStatusIcons", showROMStatusIcons);
  bool showDeviceSets = checkBoxShowDeviceSets->isChecked();
  needReload |= (config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowDeviceSets", true).toBool() != showDeviceSets );
  config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/ShowDeviceSets", showDeviceSets);
  bool showBiosSets = checkBoxShowBiosSets->isChecked();
  needReload |= (config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowBiosSets", true).toBool() != showBiosSets );
  config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/ShowBiosSets", showBiosSets);
  config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/AutoTriggerROMCheck", checkBoxAutoTriggerROMCheck->isChecked());
  config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/DoubleClickActivation", checkBoxDoubleClickActivation->isChecked());
  config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/PlayOnSublistActivation", checkBoxPlayOnSublistActivation->isChecked());
  qmc2CursorPositioningMode = (QAbstractItemView::ScrollHint)comboBoxCursorPosition->currentIndex();
  config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/CursorPosition", qmc2CursorPositioningMode);
  qmc2DefaultLaunchMode = comboBoxDefaultLaunchMode->currentIndex();
  config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/DefaultLaunchMode", qmc2DefaultLaunchMode);
  qmc2SoftwareSnapPosition = comboBoxSoftwareSnapPosition->currentIndex();
  config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SoftwareSnapPosition", qmc2SoftwareSnapPosition);
  config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SoftwareSnapOnMouseHover", checkBoxSoftwareSnapOnMouseHover->isChecked());
  config->setValue(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/AutoDisableSoftwareSnap", checkBoxAutoDisableSoftwareSnap->isChecked());
  qmc2GamelistResponsiveness = spinBoxResponsiveness->value();
  config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/Responsiveness", qmc2GamelistResponsiveness);
  qmc2UpdateDelay = spinBoxUpdateDelay->value();
  config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/UpdateDelay", qmc2UpdateDelay);
  i = comboBoxSortCriteria->currentIndex();
  needResort = (i != qmc2SortCriteria);
  int oldSortCriteria = qmc2SortCriteria;
  qmc2SortCriteria = i;
  config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/SortCriteria", qmc2SortCriteria);
  i = comboBoxSortOrder->currentIndex();
  needResort = needResort || (i == 0 ? qmc2SortOrder != Qt::AscendingOrder : qmc2SortOrder != Qt::DescendingOrder);
  Qt::SortOrder oldSortOrder = qmc2SortOrder; 
  qmc2SortOrder = (i == 0 ? Qt::AscendingOrder : Qt::DescendingOrder);
  config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/SortOrder", qmc2SortOrder);
  QBitArray newFilter(QMC2_ROMSTATE_COUNT);
  config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/ShowC", toolButtonShowC->isChecked());
  newFilter.setBit(QMC2_ROMSTATE_INT_C, toolButtonShowC->isChecked());
  config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/ShowM", toolButtonShowM->isChecked());
  newFilter.setBit(QMC2_ROMSTATE_INT_M, toolButtonShowM->isChecked());
  config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/ShowI", toolButtonShowI->isChecked());
  newFilter.setBit(QMC2_ROMSTATE_INT_I, toolButtonShowI->isChecked());
  config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/ShowN", toolButtonShowN->isChecked());
  newFilter.setBit(QMC2_ROMSTATE_INT_N, toolButtonShowN->isChecked());
  config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/ShowU", toolButtonShowU->isChecked());
  newFilter.setBit(QMC2_ROMSTATE_INT_U, toolButtonShowU->isChecked());
  needFilter = (qmc2Filter != newFilter);
  qmc2Filter = newFilter;

  bool oldRSF = config->value(QMC2_FRONTEND_PREFIX + "Gamelist/EnableRomStateFilter", true).toBool();
  if ( checkBoxRomStateFilter->isChecked() ) {
	  if ( qmc2MainWindow->comboBoxViewSelect->currentIndex() == QMC2_VIEWGAMELIST_INDEX ) {
	  	qmc2MainWindow->pushButtonSelectRomFilter->setVisible(true);
		qmc2MainWindow->actionTagVisible->setVisible(true);
		qmc2MainWindow->actionUntagVisible->setVisible(true);
		qmc2MainWindow->actionInvertVisibleTags->setVisible(true);
	  }
	  qmc2MainWindow->actionRomStatusFilterC->setEnabled(true);
	  qmc2MainWindow->actionRomStatusFilterM->setEnabled(true);
	  qmc2MainWindow->actionRomStatusFilterI->setEnabled(true);
	  qmc2MainWindow->actionRomStatusFilterN->setEnabled(true);
	  qmc2MainWindow->actionRomStatusFilterU->setEnabled(true);
	  config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/EnableRomStateFilter", true);
	  if ( !oldRSF ) {
		  needReload = true;
		  needFilter = true;
	  }
  } else {
	  qmc2MainWindow->pushButtonSelectRomFilter->setVisible(false);
	  qmc2MainWindow->actionTagVisible->setVisible(false);
	  qmc2MainWindow->actionUntagVisible->setVisible(false);
	  qmc2MainWindow->actionInvertVisibleTags->setVisible(false);
	  qmc2MainWindow->actionRomStatusFilterC->setEnabled(false);
	  qmc2MainWindow->actionRomStatusFilterM->setEnabled(false);
	  qmc2MainWindow->actionRomStatusFilterI->setEnabled(false);
	  qmc2MainWindow->actionRomStatusFilterN->setEnabled(false);
	  qmc2MainWindow->actionRomStatusFilterU->setEnabled(false);
	  config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/EnableRomStateFilter", false);
	  if ( oldRSF ) {
		  needReload = true;
		  needFilter = false;
	  }
  }

  // Shortcuts / Keys
  QMapIterator<QString, QPair<QString, QAction *> > it(qmc2ShortcutMap);
  while ( it.hasNext() ) {
    it.next();
    QString itShortcut = it.key();
    config->setValue(QString(QMC2_FRONTEND_PREFIX + "Shortcuts/%1").arg(itShortcut), qmc2CustomShortcutMap[itShortcut]);
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
    QString myKey = qmc2JoystickFunctionMap.key(itShortcut);
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
  CookieJar *cj = NULL;
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
  config->setValue("Network/HTTPProxy/Password", QMC2_COMPRESS(lineEditHTTPProxyPassword->text().toLatin1()));
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

  if ( config->value(QMC2_FRONTEND_PREFIX + "GUI/GamelistView").toInt() >= qmc2MainWindow->comboBoxViewSelect->count() )
    config->setValue(QMC2_FRONTEND_PREFIX + "GUI/GamelistView", QMC2_VIEW_DETAIL_INDEX);

  qmc2MainWindow->comboBoxViewSelect->setCurrentIndex(config->value(QMC2_FRONTEND_PREFIX + "GUI/GamelistView", QMC2_VIEW_DETAIL_INDEX).toInt());
  switch ( qmc2MainWindow->comboBoxViewSelect->currentIndex() ) {
	  case QMC2_VIEW_DETAIL_INDEX:
		  qmc2MainWindow->tabWidgetGamelist->setTabIcon(QMC2_GAMELIST_INDEX, QIcon(QString::fromUtf8(":/data/img/flat.png")));
		  break;
	  case QMC2_VIEW_TREE_INDEX:
		  qmc2MainWindow->tabWidgetGamelist->setTabIcon(QMC2_GAMELIST_INDEX, QIcon(QString::fromUtf8(":/data/img/clone.png")));
		  break;
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
	  case QMC2_VIEW_CATEGORY_INDEX:
		  qmc2MainWindow->tabWidgetGamelist->setTabIcon(QMC2_GAMELIST_INDEX, QIcon(QString::fromUtf8(":/data/img/category.png")));
		  break;
	  case QMC2_VIEW_VERSION_INDEX:
		  qmc2MainWindow->tabWidgetGamelist->setTabIcon(QMC2_GAMELIST_INDEX, QIcon(QString::fromUtf8(":/data/img/version.png")));
		  break;
#endif
  }

  // Emulator

  // Configuration
  if ( qmc2GuiReady ) {
    if ( qmc2GlobalEmulatorOptions->changed ) {
      if ( qmc2EmulatorOptions ) {
        switch ( QMessageBox::question(this, tr("Confirm"), 
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
                 tr("An open game-specific emulator configuration has been detected.\nUse local game-settings, overwrite with global settings or don't apply?"),
#elif defined(QMC2_EMUTYPE_MESS)
                 tr("An open machine-specific emulator configuration has been detected.\nUse local machine-settings, overwrite with global settings or don't apply?"),
#endif
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
#if defined(QMC2_EMUTYPE_MAME)
  needReload |= config->value("MAME/FilesAndDirectories/ExecutableFile").toString() != lineEditExecutableFile->text();
  config->setValue("MAME/FilesAndDirectories/ExecutableFile", lineEditExecutableFile->text());
  config->setValue("MAME/FilesAndDirectories/WorkingDirectory", lineEditWorkingDirectory->text());
#if defined(QMC2_VARIANT_LAUNCHER) && defined(QMC2_OS_WIN)
  config->setValue("MAME/FilesAndDirectories/MESSVariantExe", lineEditMESSVariantExe->text());
  config->setValue("MAME/FilesAndDirectories/UMEVariantExe", lineEditUMEVariantExe->text());
#endif
  config->setValue("MAME/FilesAndDirectories/LogFile", lineEditEmulatorLogFile->text());
  config->setValue("MAME/FilesAndDirectories/XmlCacheDatabase", lineEditXmlCacheDatabase->text());
  config->setValue("MAME/FilesAndDirectories/UserDataDatabase", lineEditUserDataDatabase->text());
  config->setValue("MAME/FilesAndDirectories/GamelistCacheFile", lineEditGamelistCacheFile->text());
  config->setValue("MAME/FilesAndDirectories/ROMStateCacheFile", lineEditROMStateCacheFile->text());
  //config->setValue("MAME/FilesAndDirectories/SlotInfoCacheFile", lineEditSlotInfoCacheFile->text());
  config->setValue("MAME/FilesAndDirectories/SoftwareListCache", lineEditSoftwareListCache->text());
  config->setValue("MAME/FilesAndDirectories/SoftwareStateCache", lineEditSoftwareStateCache->text());
  config->setValue("MAME/FilesAndDirectories/MAWSCacheDirectory", lineEditMAWSCacheDirectory->text());
  s = lineEditOptionsTemplateFile->text();
  needRecreateTemplateMap = needRecreateTemplateMap || (config->value("MAME/FilesAndDirectories/OptionsTemplateFile").toString() != s );
  config->setValue("MAME/FilesAndDirectories/OptionsTemplateFile", s);
  config->setValue("MAME/FilesAndDirectories/FavoritesFile", lineEditFavoritesFile->text());
  config->setValue("MAME/FilesAndDirectories/HistoryFile", lineEditHistoryFile->text());
#elif defined(QMC2_EMUTYPE_MESS)
  needReload |= config->value("MESS/FilesAndDirectories/ExecutableFile").toString() != lineEditExecutableFile->text();
  config->setValue("MESS/FilesAndDirectories/ExecutableFile", lineEditExecutableFile->text());
  config->setValue("MESS/FilesAndDirectories/WorkingDirectory", lineEditWorkingDirectory->text());
#if defined(QMC2_VARIANT_LAUNCHER) && defined(QMC2_OS_WIN)
  config->setValue("MESS/FilesAndDirectories/MAMEVariantExe", lineEditMAMEVariantExe->text());
  config->setValue("MESS/FilesAndDirectories/UMEVariantExe", lineEditUMEVariantExe->text());
#endif
  config->setValue("MESS/FilesAndDirectories/LogFile", lineEditEmulatorLogFile->text());
  config->setValue("MESS/FilesAndDirectories/XmlCacheDatabase", lineEditXmlCacheDatabase->text());
  config->setValue("MESS/FilesAndDirectories/UserDataDatabase", lineEditUserDataDatabase->text());
  config->setValue("MESS/FilesAndDirectories/GamelistCacheFile", lineEditGamelistCacheFile->text());
  config->setValue("MESS/FilesAndDirectories/ROMStateCacheFile", lineEditROMStateCacheFile->text());
  config->setValue("MESS/FilesAndDirectories/SlotInfoCacheFile", lineEditSlotInfoCacheFile->text());
  config->setValue("MESS/FilesAndDirectories/SoftwareListCache", lineEditSoftwareListCache->text());
  config->setValue("MESS/FilesAndDirectories/SoftwareStateCache", lineEditSoftwareStateCache->text());
  config->setValue("MESS/FilesAndDirectories/GeneralSoftwareFolder", lineEditGeneralSoftwareFolder->text());
  s = lineEditOptionsTemplateFile->text();
  needRecreateTemplateMap = needRecreateTemplateMap || (config->value("MESS/FilesAndDirectories/OptionsTemplateFile").toString() != s );
  config->setValue("MESS/FilesAndDirectories/OptionsTemplateFile", s);
  config->setValue("MESS/FilesAndDirectories/FavoritesFile", lineEditFavoritesFile->text());
  config->setValue("MESS/FilesAndDirectories/HistoryFile", lineEditHistoryFile->text());
#elif defined(QMC2_EMUTYPE_UME)
  needReload |= config->value("UME/FilesAndDirectories/ExecutableFile").toString() != lineEditExecutableFile->text();
  config->setValue("UME/FilesAndDirectories/ExecutableFile", lineEditExecutableFile->text());
  config->setValue("UME/FilesAndDirectories/WorkingDirectory", lineEditWorkingDirectory->text());
#if defined(QMC2_VARIANT_LAUNCHER) && defined(QMC2_OS_WIN)
  config->setValue("UME/FilesAndDirectories/MESSVariantExe", lineEditMESSVariantExe->text());
  config->setValue("UME/FilesAndDirectories/MAMEVariantExe", lineEditMAMEVariantExe->text());
#endif
  config->setValue("UME/FilesAndDirectories/LogFile", lineEditEmulatorLogFile->text());
  config->setValue("UME/FilesAndDirectories/XmlCacheDatabase", lineEditXmlCacheDatabase->text());
  config->setValue("UME/FilesAndDirectories/UserDataDatabase", lineEditUserDataDatabase->text());
  config->setValue("UME/FilesAndDirectories/GamelistCacheFile", lineEditGamelistCacheFile->text());
  config->setValue("UME/FilesAndDirectories/ROMStateCacheFile", lineEditROMStateCacheFile->text());
  config->setValue("UME/FilesAndDirectories/SlotInfoCacheFile", lineEditSlotInfoCacheFile->text());
  config->setValue("UME/FilesAndDirectories/SoftwareListCache", lineEditSoftwareListCache->text());
  config->setValue("UME/FilesAndDirectories/SoftwareStateCache", lineEditSoftwareStateCache->text());
  config->setValue("UME/FilesAndDirectories/GeneralSoftwareFolder", lineEditGeneralSoftwareFolder->text());
  s = lineEditOptionsTemplateFile->text();
  needRecreateTemplateMap = needRecreateTemplateMap || (config->value("UME/FilesAndDirectories/OptionsTemplateFile").toString() != s );
  config->setValue("UME/FilesAndDirectories/OptionsTemplateFile", s);
  config->setValue("UME/FilesAndDirectories/FavoritesFile", lineEditFavoritesFile->text());
  config->setValue("UME/FilesAndDirectories/HistoryFile", lineEditHistoryFile->text());
#endif
  config->setValue(QMC2_EMULATOR_PREFIX + "AutoClearEmuCaches", checkBoxAutoClearEmuCaches->isChecked());

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
  config->sync();
  applied = true;
  if ( qmc2GuiReady )
    apply();

  if ( invalidateGameInfoDB ) {
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("invalidating game info DB"));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("invalidating machine info DB"));
#endif
    QHashIterator<QString, QByteArray *> it(qmc2GameInfoDB);
    QList<QByteArray *> deletedRecords;
    while ( it.hasNext() ) {
      it.next();
      if ( !deletedRecords.contains(it.value()) ) {
        if ( it.value() )
          delete it.value();
        deletedRecords.append(it.value());
      }
    }
    deletedRecords.clear();
    qmc2GameInfoDB.clear();
#if defined(QMC2_EMUTYPE_UME)
    foreach (QString key, qmc2GameInfoSourceMap)
	    qmc2GameInfoSourceMap.remove(key);
    qmc2GameInfoSourceMap.clear();
#endif
  }

  if ( invalidateEmuInfoDB ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("invalidating emulator info DB"));
    QHashIterator<QString, QByteArray *> it(qmc2EmuInfoDB);
    QList<QByteArray *> deletedRecords;
    while ( it.hasNext() ) {
      it.next();
      if ( !deletedRecords.contains(it.value()) ) {
        if ( it.value() )
          delete it.value();
        deletedRecords.append(it.value());
      }
    }
    deletedRecords.clear();
    qmc2EmuInfoDB.clear();
  }

  if ( invalidateSoftwareInfoDB ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("invalidating software info DB"));
    QHashIterator<QString, QByteArray *> it(qmc2SoftwareInfoDB);
    QList<QByteArray *> deletedRecords;
    while ( it.hasNext() ) {
      it.next();
      if ( !deletedRecords.contains(it.value()) ) {
        if ( it.value() )
          delete it.value();
        deletedRecords.append(it.value());
      }
    }
    deletedRecords.clear();
    qmc2SoftwareInfoDB.clear();
  }

  if ( needManualReload )
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("please reload game list for some changes to take effect"));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("please reload machine list for some changes to take effect"));
#endif

  if ( needRestart )
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("please restart QMC2 for some changes to take effect"));

  if ( needRecreateTemplateMap )
    qmc2MainWindow->on_actionRecreateTemplateMap_triggered();

  if ( needResort && !needReload ) {
    bool doResort = true;

    if ( qmc2VerifyActive ) {
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("re-sort of game list impossible at this time, please wait for ROM verification to finish and try again"));
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("re-sort of machine list impossible at this time, please wait for ROM verification to finish and try again"));
#endif
      qmc2SortCriteria = oldSortCriteria;
      qmc2SortOrder = oldSortOrder;
      doResort = false;
    }

    if ( doResort ) {
      qmc2SortingActive = true;
      QString sortCriteria = tr("?");
      switch ( qmc2SortCriteria ) {
        case QMC2_SORT_BY_DESCRIPTION:
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
          sortCriteria = QObject::tr("game description");
#elif defined(QMC2_EMUTYPE_MESS)
          sortCriteria = QObject::tr("machine description");
#endif
          break;
        case QMC2_SORT_BY_ROM_STATE:
          sortCriteria = QObject::tr("ROM state");
          break;
        case QMC2_SORT_BY_TAG:
          sortCriteria = QObject::tr("tag");
          break;
        case QMC2_SORT_BY_YEAR:
          sortCriteria = QObject::tr("year");
          break;
        case QMC2_SORT_BY_MANUFACTURER:
          sortCriteria = QObject::tr("manufacturer");
          break;
        case QMC2_SORT_BY_NAME:
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
          sortCriteria = QObject::tr("game name");
#elif defined(QMC2_EMUTYPE_MESS)
          sortCriteria = QObject::tr("machine name");
#endif
          break;
        case QMC2_SORT_BY_ROMTYPES:
          sortCriteria = QObject::tr("ROM types");
          break;
        case QMC2_SORT_BY_PLAYERS:
          sortCriteria = QObject::tr("players");
          break;
        case QMC2_SORT_BY_DRVSTAT:
          sortCriteria = QObject::tr("driver status");
          break;
        case QMC2_SORT_BY_SRCFILE:
          sortCriteria = QObject::tr("source file");
          break;
        case QMC2_SORT_BY_RANK:
          sortCriteria = QObject::tr("rank");
          break;
        case QMC2_SORT_BY_CATEGORY:
          sortCriteria = QObject::tr("category");
          break;
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
        case QMC2_SORT_BY_VERSION:
          sortCriteria = QObject::tr("version");
          break;
#endif
      }
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("sorting game list by %1 in %2 order").arg(sortCriteria).arg(qmc2SortOrder == Qt::AscendingOrder ? tr("ascending") : tr("descending")));
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("sorting machine list by %1 in %2 order").arg(sortCriteria).arg(qmc2SortOrder == Qt::AscendingOrder ? tr("ascending") : tr("descending")));
#endif
      qApp->processEvents();
      foreach (QTreeWidgetItem *ti, qmc2ExpandedGamelistItems) {
	      qmc2MainWindow->treeWidgetGamelist->collapseItem(ti);
	      QList<QTreeWidgetItem *> childrenList = ti->takeChildren();
	      foreach (QTreeWidgetItem *ci, ti->takeChildren())
		      delete ci;
	      QTreeWidgetItem *nameItem = new QTreeWidgetItem(ti);
	      nameItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Waiting for data..."));
	      nameItem->setText(QMC2_GAMELIST_COLUMN_ICON, ti->text(QMC2_GAMELIST_COLUMN_NAME));
      }
      qmc2ExpandedGamelistItems.clear();
      qApp->processEvents();
      qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(false);
      qmc2MainWindow->treeWidgetHierarchy->setUpdatesEnabled(false);
      qmc2MainWindow->treeWidgetCategoryView->setUpdatesEnabled(false);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
      qmc2MainWindow->treeWidgetVersionView->setUpdatesEnabled(false);
#endif
      if ( qmc2SortCriteria == QMC2_SORT_BY_RANK && !qmc2Gamelist->userDataDb()->rankCacheComplete() )
        qmc2Gamelist->userDataDb()->fillUpRankCache();
      qmc2MainWindow->treeWidgetGamelist->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
      qApp->processEvents();
      qmc2MainWindow->treeWidgetHierarchy->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
      qApp->processEvents();
      if ( qmc2MainWindow->treeWidgetCategoryView->topLevelItemCount() > 0 ) {
	      qmc2MainWindow->treeWidgetCategoryView->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
	      qApp->processEvents();
      }
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
      if ( qmc2MainWindow->treeWidgetVersionView->topLevelItemCount() > 0 ) {
	      qmc2MainWindow->treeWidgetVersionView->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
	      qApp->processEvents();
      }
#endif
      qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(true);
      qmc2MainWindow->treeWidgetHierarchy->setUpdatesEnabled(true);
      qmc2MainWindow->treeWidgetCategoryView->setUpdatesEnabled(true);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
      qmc2MainWindow->treeWidgetVersionView->setUpdatesEnabled(true);
#endif
      qmc2SortingActive = false;
      QTimer::singleShot(0, qmc2MainWindow, SLOT(scrollToCurrentItem()));
    }
  }

  switch ( qmc2SortCriteria ) {
    case QMC2_SORT_BY_DESCRIPTION:
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_GAME, qmc2SortOrder);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_GAME, qmc2SortOrder);
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicatorShown(true);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(true);
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_GAME, qmc2SortOrder);
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicatorShown(true);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_GAME, qmc2SortOrder);
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicatorShown(true);
#endif
      break;

    case QMC2_SORT_BY_TAG:
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_TAG, qmc2SortOrder);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_TAG, qmc2SortOrder);
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicatorShown(true);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(true);
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_TAG, qmc2SortOrder);
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicatorShown(true);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_TAG, qmc2SortOrder);
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicatorShown(true);
#endif
      break;

    case QMC2_SORT_BY_YEAR:
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_YEAR, qmc2SortOrder);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_YEAR, qmc2SortOrder);
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicatorShown(true);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(true);
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_YEAR, qmc2SortOrder);
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicatorShown(true);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_YEAR, qmc2SortOrder);
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicatorShown(true);
#endif
      break;

    case QMC2_SORT_BY_MANUFACTURER:
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_MANU, qmc2SortOrder);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_MANU, qmc2SortOrder);
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicatorShown(true);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(true);
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_MANU, qmc2SortOrder);
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicatorShown(true);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_MANU, qmc2SortOrder);
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicatorShown(true);
#endif
      break;

    case QMC2_SORT_BY_NAME:
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_NAME, qmc2SortOrder);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_NAME, qmc2SortOrder);
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicatorShown(true);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(true);
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_NAME, qmc2SortOrder);
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicatorShown(true);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_NAME, qmc2SortOrder);
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicatorShown(true);
#endif
      break;

    case QMC2_SORT_BY_ROMTYPES:
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_RTYPES, qmc2SortOrder);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_RTYPES, qmc2SortOrder);
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicatorShown(true);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(true);
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_RTYPES, qmc2SortOrder);
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicatorShown(true);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_RTYPES, qmc2SortOrder);
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicatorShown(true);
#endif
      break;

    case QMC2_SORT_BY_PLAYERS:
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_PLAYERS, qmc2SortOrder);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_PLAYERS, qmc2SortOrder);
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicatorShown(true);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(true);
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_PLAYERS, qmc2SortOrder);
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicatorShown(true);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_PLAYERS, qmc2SortOrder);
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicatorShown(true);
#endif
      break;

    case QMC2_SORT_BY_DRVSTAT:
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_DRVSTAT, qmc2SortOrder);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_DRVSTAT, qmc2SortOrder);
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicatorShown(true);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(true);
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_DRVSTAT, qmc2SortOrder);
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicatorShown(true);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_DRVSTAT, qmc2SortOrder);
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicatorShown(true);
#endif
      break;

    case QMC2_SORT_BY_SRCFILE:
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_SRCFILE, qmc2SortOrder);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_SRCFILE, qmc2SortOrder);
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicatorShown(true);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(true);
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_SRCFILE, qmc2SortOrder);
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicatorShown(true);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_SRCFILE, qmc2SortOrder);
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicatorShown(true);
#endif
      break;

    case QMC2_SORT_BY_RANK:
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_RANK, qmc2SortOrder);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_RANK, qmc2SortOrder);
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicatorShown(true);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(true);
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_RANK, qmc2SortOrder);
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicatorShown(true);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_RANK, qmc2SortOrder);
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicatorShown(true);
#endif
      break;

    case QMC2_SORT_BY_CATEGORY:
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_CATEGORY, qmc2SortOrder);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_CATEGORY, qmc2SortOrder);
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicatorShown(true);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(true);
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_CATEGORY, qmc2SortOrder);
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_CATEGORY, qmc2SortOrder);
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicatorShown(true);
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicatorShown(true);
      break;

#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
    case QMC2_SORT_BY_VERSION:
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_VERSION, qmc2SortOrder);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_VERSION, qmc2SortOrder);
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicatorShown(true);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(true);
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_VERSION, qmc2SortOrder);
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_VERSION, qmc2SortOrder);
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicatorShown(true);
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicatorShown(true);
      break;
#endif

    default:
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicatorShown(false);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(false);
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicatorShown(false);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicatorShown(false);
#endif
      break;
  }

  if ( needFilter && !needReload ) {
    qmc2StatesTogglesEnabled = false;
    qmc2MainWindow->actionRomStatusFilterC->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_C]);
    qmc2MainWindow->actionRomStatusFilterM->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_M]);
    qmc2MainWindow->actionRomStatusFilterI->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_I]);
    qmc2MainWindow->actionRomStatusFilterN->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_N]);
    qmc2MainWindow->actionRomStatusFilterU->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_U]);
    qmc2Gamelist->filter();
  }

  QList<ImageWidget *> iwl;
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
  iwl << qmc2Preview << qmc2Flyer << qmc2Cabinet << qmc2Controller << qmc2Marquee << qmc2Title << qmc2PCB;
#else
  iwl << qmc2Preview << qmc2Flyer << qmc2Cabinet << qmc2Controller << qmc2Marquee << qmc2PCB;
#endif
  foreach (ImageWidget *iw, iwl) {
	  if ( iw ) {
		  bool needReopenFile = false;
		  switch ( iw->imageTypeNumeric() ) {
			  case QMC2_IMGTYPE_PREVIEW: needReopenFile |= needReopenPreviewFile; break;
			  case QMC2_IMGTYPE_FLYER: needReopenFile |= needReopenFlyerFile; break;
			  case QMC2_IMGTYPE_CABINET: needReopenFile |= needReopenCabinetFile; break;
			  case QMC2_IMGTYPE_CONTROLLER: needReopenFile |= needReopenControllerFile; break;
			  case QMC2_IMGTYPE_MARQUEE: needReopenFile |= needReopenMarqueeFile; break;
			  case QMC2_IMGTYPE_TITLE: needReopenFile |= needReopenTitleFile; break;
			  case QMC2_IMGTYPE_PCB: needReopenFile |= needReopenPCBFile; break;
		  }
		  if ( needReopenFile ) {
			  foreach (unzFile imageFile, iw->imageFileMap)
				  unzClose(imageFile);
			  foreach (SevenZipFile *imageFile, iw->imageFileMap7z) {
				  imageFile->close();
				  delete imageFile;
			  }
			  iw->imageFileMap.clear();
			  iw->imageFileMap7z.clear();
			  if ( iw->useZip() ) {
				  foreach (QString filePath, Settings::stResolve(iw->imageZip()).split(";", QString::SkipEmptyParts)) {
					  unzFile imageFile = unzOpen(filePath.toLocal8Bit());
					  if ( imageFile == NULL )
						  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open %1 file, please check access permissions for %2").arg(iw->imageType()).arg(filePath));
					  else
						  iw->imageFileMap[filePath] = imageFile;
				  }
			  } else if ( iw->useSevenZip() ) {
				  foreach (QString filePath, Settings::stResolve(iw->imageZip()).split(";", QString::SkipEmptyParts)) {
					  SevenZipFile *imageFile = new SevenZipFile(filePath);
					  if ( !imageFile->open() ) {
						  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open %1 file %2").arg(iw->imageType()).arg(filePath) + " - " + tr("7z error") + ": " + imageFile->lastError());
						  delete imageFile;
					  } else {
						  iw->imageFileMap7z[filePath] = imageFile;
						  connect(imageFile, SIGNAL(dataReady()), iw, SLOT(sevenZipDataReady()));
					  }
				  }
			  }
		  }
		  iw->update();
	  }
  }

  if ( qmc2SoftwareSnap ) {
	  if ( needReopenSoftwareSnapFile ) {
		  foreach (unzFile imageFile, qmc2SoftwareSnap->snapFileMap)
			  unzClose(imageFile);
		  foreach (SevenZipFile *imageFile, qmc2SoftwareSnap->snapFileMap7z) {
			  imageFile->close();
			  delete imageFile;
		  }
		  qmc2SoftwareSnap->snapFileMap.clear();
		  qmc2SoftwareSnap->snapFileMap7z.clear();
		  if ( qmc2SoftwareSnap->useZip() ) {
			  foreach (QString filePath, config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareSnapFile").toString().split(";", QString::SkipEmptyParts)) {
				  unzFile imageFile = unzOpen(filePath.toLocal8Bit());
				  if ( imageFile == NULL )
					  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open software snap-shot file, please check access permissions for %1").arg(filePath));
				  else
					  qmc2SoftwareSnap->snapFileMap[filePath] = imageFile;
			  }
		  } else if ( qmc2SoftwareSnap->useSevenZip() ) {
			  foreach (QString filePath, config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareSnapFile").toString().split(";", QString::SkipEmptyParts)) {
				  SevenZipFile *imageFile = new SevenZipFile(filePath);
				  if ( !imageFile->open() ) {
					  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open software snap-shot file %1").arg(filePath) + " - " + tr("7z error") + ": " + imageFile->lastError());
					  delete imageFile;
				  } else {
					  qmc2SoftwareSnap->snapFileMap7z[filePath] = imageFile;
					  connect(imageFile, SIGNAL(dataReady()), qmc2SoftwareSnap, SLOT(sevenZipDataReady()));
				  }
			  }
		  }
	  }
	  qmc2SoftwareSnap->update();
  }

  if ( needReopenIconFile ) {
	  foreach (unzFile iconFile, qmc2IconFileMap)
		  unzClose(iconFile);
	  foreach (SevenZipFile *iconFile, qmc2IconFileMap7z) {
		  iconFile->close();
		  delete iconFile;
	  }
	  qmc2IconFileMap.clear();
	  qmc2IconFileMap7z.clear();
	  if ( QMC2_ICON_FILETYPE_ZIP ) {
		  foreach (QString filePath, config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/IconFile").toString().split(";", QString::SkipEmptyParts)) {
			  unzFile iconFile = unzOpen(filePath.toLocal8Bit());
			  if ( iconFile == NULL )
				  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open icon file, please check access permissions for %1").arg(filePath));
			  else
				  qmc2IconFileMap[filePath] = iconFile;
		  }
	  } else if ( QMC2_ICON_FILETYPE_7Z ) {
		  foreach (QString filePath, config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/IconFile").toString().split(";", QString::SkipEmptyParts)) {
			  SevenZipFile *iconFile = new SevenZipFile(filePath);
			  if ( !iconFile->open() ) {
				  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open icon file %1").arg(filePath) + " - " + tr("7z error") + ": " + iconFile->lastError());
				  delete iconFile;
			  } else
				  qmc2IconFileMap7z[filePath] = iconFile;
		  }
	  }
  }

  if ( needReload ) {
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("triggering automatic reload of game list"));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("triggering automatic reload of machine list"));
#endif
    qmc2AutomaticReload = true;
    QTimer::singleShot(0, qmc2MainWindow->actionReload, SLOT(trigger()));
  }

  if ( !qmc2EarlyStartup ) {
	// style
	if ( qmc2StandardPalettes.contains(qmc2CurrentStyleName) )
		qApp->setPalette(qmc2StandardPalettes[qmc2CurrentStyleName]);
	if ( oldStyleName.isEmpty() )
		oldStyleName = qmc2CurrentStyleName;
	QString styleName = comboBoxStyle->currentText();
	if ( styleName == QObject::tr("Default") )
		styleName = "Default";
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/Style", styleName);
	if ( styleName != oldStyleName )
		qmc2MainWindow->signalStyleSetupRequested(styleName);
	qmc2CurrentStyleName = oldStyleName = styleName;

	// style sheet
	QString styleSheetName = lineEditStyleSheet->text();
	config->setValue(QMC2_FRONTEND_PREFIX + "GUI/StyleSheet", styleSheetName);
	if ( styleSheetName != oldStyleSheet || styleSheetName.isEmpty() )
		qmc2MainWindow->signalStyleSheetSetupRequested(styleSheetName);
	oldStyleSheet = styleSheetName;

	// palette
	qmc2MainWindow->signalPaletteSetupRequested(styleName);
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
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_pushButtonDefault_clicked()");
#endif

  restoreCurrentConfig(true);
}

void Options::restoreCurrentConfig(bool useDefaultSettings)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::restoreCurrentConfig()");
#endif

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

  QString userScopePath = QMC2_DYNAMIC_DOT_PATH;

  // Frontend

  // GUI
  checkBoxToolbar->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/Toolbar", true).toBool());
#if defined(QMC2_OS_MAC)
  checkBoxUnifiedTitleAndToolBarOnMac->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/UnifiedTitleAndToolBarOnMac", false).toBool());
#endif
  checkBoxSaveLayout->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout", true).toBool());
  checkBoxRestoreLayout->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/RestoreLayout", true).toBool());
  checkBoxSaveGameSelection->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveGameSelection", true).toBool());
  checkBoxRestoreGameSelection->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/RestoreGameSelection", true).toBool());
  checkBoxStatusbar->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/Statusbar", true).toBool());
  checkBoxStandardColorPalette->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/StandardColorPalette", true).toBool());
  checkBoxProgressTexts->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts", false).toBool());
  checkBoxProcessMameHistoryDat->setChecked(config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMameHistoryDat", false).toBool());
  checkBoxProcessMessSysinfoDat->setChecked(config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMessSysinfoDat", false).toBool());
  toolButtonCompressMameHistoryDat->setChecked(config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/CompressMameHistoryDat", false).toBool());
  toolButtonCompressMessSysinfoDat->setChecked(config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/CompressMessSysinfoDat", false).toBool());
  checkBoxProcessMameInfoDat->setChecked(config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMameInfoDat", false).toBool());
  checkBoxProcessMessInfoDat->setChecked(config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/ProcessMessInfoDat", false).toBool());
  toolButtonCompressMameInfoDat->setChecked(config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/CompressMameInfoDat", false).toBool());
  toolButtonCompressMessInfoDat->setChecked(config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/CompressMessInfoDat", false).toBool());
  checkBoxProcessSoftwareInfoDB->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/ProcessSoftwareInfoDB", false).toBool());
  toolButtonCompressSoftwareInfoDB->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/CompressSoftwareInfoDB", false).toBool());
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
  qmc2SmoothScaling = config->value(QMC2_FRONTEND_PREFIX + "GUI/SmoothScaling", false).toBool();
  checkBoxSmoothScaling->setChecked(qmc2SmoothScaling);
  qmc2RetryLoadingImages = config->value(QMC2_FRONTEND_PREFIX + "GUI/RetryLoadingImages", true).toBool();
  checkBoxRetryLoadingImages->setChecked(qmc2RetryLoadingImages);
  qmc2ParentImageFallback = config->value(QMC2_FRONTEND_PREFIX + "GUI/ParentImageFallback", false).toBool();
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
  checkBoxSetWorkDirFromExec->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/SetWorkDirFromExec", false).toBool());
  if ( checkBoxSetWorkDirFromExec->isChecked() )
	  QDir::setCurrent(QCoreApplication::applicationDirPath());
  else
	  QDir::setCurrent(qmc2StandardWorkDir);
  checkBoxGameStatusIndicator->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/GameStatusIndicator", false).toBool());
  checkBoxGameStatusIndicatorOnlyWhenRequired->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/GameStatusIndicatorOnlyWhenRequired", true).toBool());
  checkBoxShowGameName->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/ShowGameName", false).toBool());
  qmc2ShowGameName = checkBoxShowGameName->isChecked();
  checkBoxShowGameNameOnlyWhenRequired->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/ShowGameNameOnlyWhenRequired", false).toBool());
  qmc2ShowGameNameOnlyWhenRequired = checkBoxShowGameNameOnlyWhenRequired->isChecked();
  spinBoxFrontendLogSize->setValue(config->value(QMC2_FRONTEND_PREFIX + "GUI/FrontendLogSize", 0).toInt());
  spinBoxEmulatorLogSize->setValue(config->value(QMC2_FRONTEND_PREFIX + "GUI/EmulatorLogSize", 0).toInt());
#if defined(QMC2_VARIANT_LAUNCHER)
  checkBoxMinimizeOnVariantLaunch->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/MinimizeOnVariantLaunch", false).toBool());
  checkBoxExitOnVariantLaunch->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/ExitOnVariantLaunch", false).toBool());
#endif
#if defined(QMC2_MEMORY_INFO_ENABLED)
  checkBoxMemoryIndicator->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/MemoryIndicator", false).toBool());
#endif
  checkBoxNativeFileDialogs->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/NativeFileDialogs", false).toBool());

  // Files / Directories
#if defined(QMC2_YOUTUBE_ENABLED)
  QDir youTubeCacheDir(config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/CacheDirectory", userScopePath + "/youtube/").toString());
  if ( !youTubeCacheDir.exists() )
    youTubeCacheDir.mkdir(youTubeCacheDir.absolutePath());
  config->setValue(QMC2_FRONTEND_PREFIX + "YouTubeWidget/CacheDirectory", QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/CacheDirectory", userScopePath + "/youtube/").toString());
#endif
  lineEditDataDirectory->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/DataDirectory", QMC2_DEFAULT_DATA_PATH + "/").toString());
#if defined(QMC2_EMUTYPE_MAME)
#if defined(QMC2_SDLMAME)
  lineEditTemporaryFile->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmame.tmp").toString());
  lineEditFrontendLogFile->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/LogFile", userScopePath + "/qmc2-sdlmame.log").toString());
#elif defined(QMC2_MAME)
  lineEditTemporaryFile->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mame.tmp").toString());
  lineEditFrontendLogFile->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/LogFile", userScopePath + "/qmc2-mame.log").toString());
#endif
  lineEditPreviewDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/PreviewDirectory", QMC2_DEFAULT_DATA_PATH + "/prv/").toString());
  lineEditPreviewFile->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/PreviewFile", QMC2_DEFAULT_DATA_PATH + "/prv/previews.zip").toString());
  comboBoxPreviewFileType->setCurrentIndex(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/PreviewFileType", QMC2_IMG_FILETYPE_ZIP).toInt());
  qmc2UsePreviewFile = config->value("MAME/FilesAndDirectories/UsePreviewFile", false).toBool();
  stackedWidgetPreview->setCurrentIndex(qmc2UsePreviewFile ? 1 : 0);
  radioButtonPreviewSelect->setText(qmc2UsePreviewFile ? tr("Preview file") : tr("Preview directory"));
  lineEditFlyerDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/FlyerDirectory", QMC2_DEFAULT_DATA_PATH + "/fly/").toString());
  lineEditFlyerFile->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/FlyerFile", QMC2_DEFAULT_DATA_PATH + "/fly/flyers.zip").toString());
  comboBoxFlyerFileType->setCurrentIndex(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/FlyerFileType", QMC2_IMG_FILETYPE_ZIP).toInt());
  qmc2UseFlyerFile = config->value("MAME/FilesAndDirectories/UseFlyerFile", false).toBool();
  stackedWidgetFlyer->setCurrentIndex(qmc2UseFlyerFile ? 1 : 0);
  radioButtonFlyerSelect->setText(qmc2UseFlyerFile ? tr("Flyer file") : tr("Flyer directory"));
  lineEditIconDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/IconDirectory", QMC2_DEFAULT_DATA_PATH + "/ico/").toString());
  lineEditIconFile->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/IconFile", QMC2_DEFAULT_DATA_PATH + "/ico/icons.zip").toString());
  comboBoxIconFileType->setCurrentIndex(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/IconFileType", QMC2_IMG_FILETYPE_ZIP).toInt());
  qmc2UseIconFile = config->value("MAME/FilesAndDirectories/UseIconFile", false).toBool();
  stackedWidgetIcon->setCurrentIndex(qmc2UseIconFile ? 1 : 0);
  radioButtonIconSelect->setText(qmc2UseIconFile ? tr("Icon file") : tr("Icon directory"));
  lineEditCabinetDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/CabinetDirectory", QMC2_DEFAULT_DATA_PATH + "/cab/").toString());
  lineEditCabinetFile->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/CabinetFile", QMC2_DEFAULT_DATA_PATH + "/cab/cabinets.zip").toString());
  comboBoxCabinetFileType->setCurrentIndex(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/CabinetFileType", QMC2_IMG_FILETYPE_ZIP).toInt());
  qmc2UseCabinetFile = config->value("MAME/FilesAndDirectories/UseCabinetFile", false).toBool();
  stackedWidgetCabinet->setCurrentIndex(qmc2UseCabinetFile ? 1 : 0);
  radioButtonCabinetSelect->setText(qmc2UseCabinetFile ? tr("Cabinet file") : tr("Cabinet directory"));
  lineEditControllerDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/ControllerDirectory", QMC2_DEFAULT_DATA_PATH + "/ctl/").toString());
  lineEditControllerFile->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/ControllerFile", QMC2_DEFAULT_DATA_PATH + "/ctl/controllers.zip").toString());
  comboBoxControllerFileType->setCurrentIndex(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/ControllerFileType", QMC2_IMG_FILETYPE_ZIP).toInt());
  qmc2UseControllerFile = config->value("MAME/FilesAndDirectories/UseControllerFile", false).toBool();
  stackedWidgetController->setCurrentIndex(qmc2UseControllerFile ? 1 : 0);
  radioButtonControllerSelect->setText(qmc2UseControllerFile ? tr("Controller file") : tr("Controller directory"));
  lineEditMarqueeDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/MarqueeDirectory", QMC2_DEFAULT_DATA_PATH + "/mrq/").toString());
  lineEditMarqueeFile->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/MarqueeFile", QMC2_DEFAULT_DATA_PATH + "/mrq/marquees.zip").toString());
  comboBoxMarqueeFileType->setCurrentIndex(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/MarqueeFileType", QMC2_IMG_FILETYPE_ZIP).toInt());
  qmc2UseMarqueeFile = config->value("MAME/FilesAndDirectories/UseMarqueeFile", false).toBool();
  stackedWidgetMarquee->setCurrentIndex(qmc2UseMarqueeFile ? 1 : 0);
  radioButtonMarqueeSelect->setText(qmc2UseMarqueeFile ? tr("Marquee file") : tr("Marquee directory"));
  lineEditTitleDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/TitleDirectory", QMC2_DEFAULT_DATA_PATH + "/ttl/").toString());
  lineEditTitleFile->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/TitleFile", QMC2_DEFAULT_DATA_PATH + "/ttl/titles.zip").toString());
  comboBoxTitleFileType->setCurrentIndex(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/TitleFileType", QMC2_IMG_FILETYPE_ZIP).toInt());
  qmc2UseTitleFile = config->value("MAME/FilesAndDirectories/UseTitleFile", false).toBool();
  stackedWidgetTitle->setCurrentIndex(qmc2UseTitleFile ? 1 : 0);
  radioButtonTitleSelect->setText(qmc2UseTitleFile ? tr("Title file") : tr("Title directory"));
  lineEditPCBDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/PCBDirectory", QMC2_DEFAULT_DATA_PATH + "/pcb/").toString());
  lineEditPCBFile->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/PCBFile", QMC2_DEFAULT_DATA_PATH + "/pcb/pcbs.zip").toString());
  comboBoxPCBFileType->setCurrentIndex(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/PCBFileType", QMC2_IMG_FILETYPE_ZIP).toInt());
  qmc2UsePCBFile = config->value("MAME/FilesAndDirectories/UsePCBFile", false).toBool();
  stackedWidgetPCB->setCurrentIndex(qmc2UsePCBFile ? 1 : 0);
  radioButtonPCBSelect->setText(qmc2UsePCBFile ? tr("PCB file") : tr("PCB directory"));
  lineEditSoftwareSnapDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/SoftwareSnapDirectory", QMC2_DEFAULT_DATA_PATH + "/sws/").toString());
  lineEditSoftwareSnapFile->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/SoftwareSnapFile", QMC2_DEFAULT_DATA_PATH + "/sws/swsnaps.zip").toString());
  comboBoxSoftwareSnapFileType->setCurrentIndex(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/SoftwareSnapFileType", QMC2_IMG_FILETYPE_ZIP).toInt());
  qmc2UseSoftwareSnapFile = config->value("MAME/FilesAndDirectories/UseSoftwareSnapFile", false).toBool();
  stackedWidgetSWSnap->setCurrentIndex(qmc2UseSoftwareSnapFile ? 1 : 0);
  radioButtonSoftwareSnapSelect->setText(qmc2UseSoftwareSnapFile ? tr("SW snap file") : tr("SW snap folder"));
  lineEditSoftwareNotesFolder->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/SoftwareNotesFolder", QMC2_DEFAULT_DATA_PATH + "/swn/").toString());
  lineEditSoftwareNotesTemplate->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/SoftwareNotesTemplate", QMC2_DEFAULT_DATA_PATH + "/swn/template.html").toString());
  checkBoxUseSoftwareNotesTemplate->setChecked(config->value("MAME/FilesAndDirectories/UseSoftwareNotesTemplate", false).toBool());
  lineEditSystemNotesFolder->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/SystemNotesFolder", QMC2_DEFAULT_DATA_PATH + "/gmn/").toString());
  lineEditSystemNotesTemplate->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/SystemNotesTemplate", QMC2_DEFAULT_DATA_PATH + "/gmn/template.html").toString());
  checkBoxUseSystemNotesTemplate->setChecked(config->value("MAME/FilesAndDirectories/UseSystemNotesTemplate", false).toBool());
  lineEditMameHistoryDat->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MameHistoryDat", QMC2_DEFAULT_DATA_PATH + "/cat/history.dat").toString());
  lineEditMameInfoDat->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MameInfoDat", QMC2_DEFAULT_DATA_PATH + "/cat/mameinfo.dat").toString());
  lineEditCatverIniFile->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/CatverIni", QMC2_DEFAULT_DATA_PATH + "/cat/catver.ini").toString());
  checkBoxUseCatverIni->setChecked(config->value(QMC2_FRONTEND_PREFIX + "Gamelist/UseCatverIni", false).toBool());
#elif defined(QMC2_EMUTYPE_MESS)
#if defined(QMC2_SDLMESS)
  lineEditTemporaryFile->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmess.tmp").toString());
  lineEditFrontendLogFile->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/LogFile", userScopePath + "/qmc2-sdlmess.log").toString());
#elif defined(QMC2_MESS)
  lineEditTemporaryFile->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mess.tmp").toString());
  lineEditFrontendLogFile->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/LogFile", userScopePath + "/qmc2-mess.log").toString());
#endif
  lineEditPreviewDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/PreviewDirectory", QMC2_DEFAULT_DATA_PATH + "/prv/").toString());
  lineEditPreviewFile->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/PreviewFile", QMC2_DEFAULT_DATA_PATH + "/prv/previews.zip").toString());
  comboBoxPreviewFileType->setCurrentIndex(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/PreviewFileType", QMC2_IMG_FILETYPE_ZIP).toInt());
  qmc2UsePreviewFile = config->value("MESS/FilesAndDirectories/UsePreviewFile", false).toBool();
  stackedWidgetPreview->setCurrentIndex(qmc2UsePreviewFile ? 1 : 0);
  radioButtonPreviewSelect->setText(qmc2UsePreviewFile ? tr("Preview file") : tr("Preview directory"));
  lineEditFlyerDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/FlyerDirectory", QMC2_DEFAULT_DATA_PATH + "/fly/").toString());
  lineEditFlyerFile->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/FlyerFile", QMC2_DEFAULT_DATA_PATH + "/fly/flyers.zip").toString());
  comboBoxFlyerFileType->setCurrentIndex(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/FlyerFileType", QMC2_IMG_FILETYPE_ZIP).toInt());
  qmc2UseFlyerFile = config->value("MESS/FilesAndDirectories/UseFlyerFile", false).toBool();
  stackedWidgetFlyer->setCurrentIndex(qmc2UseFlyerFile ? 1 : 0);
  radioButtonFlyerSelect->setText(qmc2UseFlyerFile ? tr("Flyer file") : tr("Flyer directory"));
  lineEditIconDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/IconDirectory", QMC2_DEFAULT_DATA_PATH + "/ico/").toString());
  lineEditIconFile->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/IconFile", QMC2_DEFAULT_DATA_PATH + "/ico/icons.zip").toString());
  comboBoxIconFileType->setCurrentIndex(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/IconFileType", QMC2_IMG_FILETYPE_ZIP).toInt());
  qmc2UseIconFile = config->value("MESS/FilesAndDirectories/UseIconFile", false).toBool();
  stackedWidgetIcon->setCurrentIndex(qmc2UseIconFile ? 1 : 0);
  radioButtonIconSelect->setText(qmc2UseIconFile ? tr("Icon file") : tr("Icon directory"));
  lineEditCabinetDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/CabinetDirectory", QMC2_DEFAULT_DATA_PATH + "/cab/").toString());
  lineEditCabinetFile->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/CabinetFile", QMC2_DEFAULT_DATA_PATH + "/cab/cabinets.zip").toString());
  comboBoxCabinetFileType->setCurrentIndex(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/CabinetFileType", QMC2_IMG_FILETYPE_ZIP).toInt());
  qmc2UseCabinetFile = config->value("MESS/FilesAndDirectories/UseCabinetFile", false).toBool();
  stackedWidgetCabinet->setCurrentIndex(qmc2UseCabinetFile ? 1 : 0);
  radioButtonCabinetSelect->setText(qmc2UseCabinetFile ? tr("Cabinet file") : tr("Cabinet directory"));
  lineEditControllerDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/ControllerDirectory", QMC2_DEFAULT_DATA_PATH + "/ctl/").toString());
  lineEditControllerFile->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/ControllerFile", QMC2_DEFAULT_DATA_PATH + "/ctl/controllers.zip").toString());
  comboBoxControllerFileType->setCurrentIndex(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/ControllerFileType", QMC2_IMG_FILETYPE_ZIP).toInt());
  qmc2UseControllerFile = config->value("MESS/FilesAndDirectories/UseControllerFile", false).toBool();
  stackedWidgetController->setCurrentIndex(qmc2UseControllerFile ? 1 : 0);
  radioButtonControllerSelect->setText(qmc2UseControllerFile ? tr("Controller file") : tr("Controller directory"));
  lineEditMarqueeDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/MarqueeDirectory", QMC2_DEFAULT_DATA_PATH + "/mrq/").toString());
  lineEditMarqueeFile->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/MarqueeFile", QMC2_DEFAULT_DATA_PATH + "/mrq/logos.zip").toString());
  comboBoxMarqueeFileType->setCurrentIndex(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/MarqueeFileType", QMC2_IMG_FILETYPE_ZIP).toInt());
  qmc2UseMarqueeFile = config->value("MESS/FilesAndDirectories/UseMarqueeFile", false).toBool();
  stackedWidgetMarquee->setCurrentIndex(qmc2UseMarqueeFile ? 1 : 0);
  radioButtonMarqueeSelect->setText(qmc2UseMarqueeFile ? tr("Logo file") : tr("Logo directory"));
  lineEditTitleDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/TitleDirectory", QMC2_DEFAULT_DATA_PATH + "/ttl/").toString());
  lineEditTitleFile->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/TitleFile", QMC2_DEFAULT_DATA_PATH + "/ttl/titles.zip").toString());
  comboBoxTitleFileType->setCurrentIndex(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/TitleFileType", QMC2_IMG_FILETYPE_ZIP).toInt());
  qmc2UseTitleFile = config->value("MESS/FilesAndDirectories/UseTitleFile", false).toBool();
  stackedWidgetTitle->setCurrentIndex(qmc2UseTitleFile ? 1 : 0);
  radioButtonTitleSelect->setText(qmc2UseTitleFile ? tr("Title file") : tr("Title directory"));
  lineEditPCBDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/PCBDirectory", QMC2_DEFAULT_DATA_PATH + "/pcb/").toString());
  lineEditPCBFile->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/PCBFile", QMC2_DEFAULT_DATA_PATH + "/pcb/pcbs.zip").toString());
  comboBoxPCBFileType->setCurrentIndex(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/PCBFileType", QMC2_IMG_FILETYPE_ZIP).toInt());
  qmc2UsePCBFile = config->value("MESS/FilesAndDirectories/UsePCBFile", false).toBool();
  stackedWidgetPCB->setCurrentIndex(qmc2UsePCBFile ? 1 : 0);
  radioButtonPCBSelect->setText(qmc2UsePCBFile ? tr("PCB file") : tr("PCB directory"));
  lineEditSoftwareSnapDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/SoftwareSnapDirectory", QMC2_DEFAULT_DATA_PATH + "/sws/").toString());
  lineEditSoftwareSnapFile->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/SoftwareSnapFile", QMC2_DEFAULT_DATA_PATH + "/sws/swsnaps.zip").toString());
  comboBoxSoftwareSnapFileType->setCurrentIndex(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/SoftwareSnapFileType", QMC2_IMG_FILETYPE_ZIP).toInt());
  qmc2UseSoftwareSnapFile = config->value("MESS/FilesAndDirectories/UseSoftwareSnapFile", false).toBool();
  stackedWidgetSWSnap->setCurrentIndex(qmc2UseSoftwareSnapFile ? 1 : 0);
  radioButtonSoftwareSnapSelect->setText(qmc2UseSoftwareSnapFile ? tr("SW snap file") : tr("SW snap folder"));
  lineEditSoftwareNotesFolder->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/SoftwareNotesFolder", QMC2_DEFAULT_DATA_PATH + "/swn/").toString());
  lineEditSoftwareNotesTemplate->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/SoftwareNotesTemplate", QMC2_DEFAULT_DATA_PATH + "/swn/template.html").toString());
  checkBoxUseSoftwareNotesTemplate->setChecked(config->value("MESS/FilesAndDirectories/UseSoftwareNotesTemplate", false).toBool());
  lineEditSystemNotesFolder->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/SystemNotesFolder", QMC2_DEFAULT_DATA_PATH + "/gmn/").toString());
  lineEditSystemNotesTemplate->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/SystemNotesTemplate", QMC2_DEFAULT_DATA_PATH + "/gmn/template.html").toString());
  checkBoxUseSystemNotesTemplate->setChecked(config->value("MESS/FilesAndDirectories/UseSystemNotesTemplate", false).toBool());
  lineEditMessSysinfoDat->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MessSysinfoDat", QMC2_DEFAULT_DATA_PATH + "/cat/sysinfo.dat").toString());
  lineEditMessInfoDat->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MessInfoDat", QMC2_DEFAULT_DATA_PATH + "/cat/messinfo.dat").toString());
  lineEditCategoryIniFile->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/CategoryIni", QMC2_DEFAULT_DATA_PATH + "/cat/category.ini").toString());
  checkBoxUseCategoryIni->setChecked(config->value(QMC2_FRONTEND_PREFIX + "Gamelist/UseCategoryIni", false).toBool());
#elif defined(QMC2_EMUTYPE_UME)
#if defined(QMC2_SDLUME)
  lineEditTemporaryFile->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlume.tmp").toString());
  lineEditFrontendLogFile->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/LogFile", userScopePath + "/qmc2-sdlume.log").toString());
#elif defined(QMC2_UME)
  lineEditTemporaryFile->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-ume.tmp").toString());
  lineEditFrontendLogFile->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/LogFile", userScopePath + "/qmc2-ume.log").toString());
#endif
  lineEditPreviewDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/PreviewDirectory", QMC2_DEFAULT_DATA_PATH + "/prv/").toString());
  lineEditPreviewFile->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/PreviewFile", QMC2_DEFAULT_DATA_PATH + "/prv/previews.zip").toString());
  comboBoxPreviewFileType->setCurrentIndex(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/PreviewFileType", QMC2_IMG_FILETYPE_ZIP).toInt());
  qmc2UsePreviewFile = config->value("UME/FilesAndDirectories/UsePreviewFile", false).toBool();
  stackedWidgetPreview->setCurrentIndex(qmc2UsePreviewFile ? 1 : 0);
  radioButtonPreviewSelect->setText(qmc2UsePreviewFile ? tr("Preview file") : tr("Preview directory"));
  lineEditFlyerDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/FlyerDirectory", QMC2_DEFAULT_DATA_PATH + "/fly/").toString());
  lineEditFlyerFile->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/FlyerFile", QMC2_DEFAULT_DATA_PATH + "/fly/flyers.zip").toString());
  comboBoxFlyerFileType->setCurrentIndex(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/FlyerFileType", QMC2_IMG_FILETYPE_ZIP).toInt());
  qmc2UseFlyerFile = config->value("UME/FilesAndDirectories/UseFlyerFile", false).toBool();
  stackedWidgetFlyer->setCurrentIndex(qmc2UseFlyerFile ? 1 : 0);
  radioButtonFlyerSelect->setText(qmc2UseFlyerFile ? tr("Flyer file") : tr("Flyer directory"));
  lineEditIconDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/IconDirectory", QMC2_DEFAULT_DATA_PATH + "/ico/").toString());
  lineEditIconFile->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/IconFile", QMC2_DEFAULT_DATA_PATH + "/ico/icons.zip").toString());
  comboBoxIconFileType->setCurrentIndex(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/IconFileType", QMC2_IMG_FILETYPE_ZIP).toInt());
  qmc2UseIconFile = config->value("UME/FilesAndDirectories/UseIconFile", false).toBool();
  stackedWidgetIcon->setCurrentIndex(qmc2UseIconFile ? 1 : 0);
  radioButtonIconSelect->setText(qmc2UseIconFile ? tr("Icon file") : tr("Icon directory"));
  lineEditCabinetDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/CabinetDirectory", QMC2_DEFAULT_DATA_PATH + "/cab/").toString());
  lineEditCabinetFile->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/CabinetFile", QMC2_DEFAULT_DATA_PATH + "/cab/cabinets.zip").toString());
  comboBoxCabinetFileType->setCurrentIndex(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/CabinetFileType", QMC2_IMG_FILETYPE_ZIP).toInt());
  qmc2UseCabinetFile = config->value("UME/FilesAndDirectories/UseCabinetFile", false).toBool();
  stackedWidgetCabinet->setCurrentIndex(qmc2UseCabinetFile ? 1 : 0);
  radioButtonCabinetSelect->setText(qmc2UseCabinetFile ? tr("Cabinet file") : tr("Cabinet directory"));
  lineEditControllerDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/ControllerDirectory", QMC2_DEFAULT_DATA_PATH + "/ctl/").toString());
  lineEditControllerFile->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/ControllerFile", QMC2_DEFAULT_DATA_PATH + "/ctl/controllers.zip").toString());
  comboBoxControllerFileType->setCurrentIndex(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/ControllerFileType", QMC2_IMG_FILETYPE_ZIP).toInt());
  qmc2UseControllerFile = config->value("UME/FilesAndDirectories/UseControllerFile", false).toBool();
  stackedWidgetController->setCurrentIndex(qmc2UseControllerFile ? 1 : 0);
  radioButtonControllerSelect->setText(qmc2UseControllerFile ? tr("Controller file") : tr("Controller directory"));
  lineEditMarqueeDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/MarqueeDirectory", QMC2_DEFAULT_DATA_PATH + "/mrq/").toString());
  lineEditMarqueeFile->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/MarqueeFile", QMC2_DEFAULT_DATA_PATH + "/mrq/marquees.zip").toString());
  comboBoxMarqueeFileType->setCurrentIndex(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/MarqueeFileType", QMC2_IMG_FILETYPE_ZIP).toInt());
  qmc2UseMarqueeFile = config->value("UME/FilesAndDirectories/UseMarqueeFile", false).toBool();
  stackedWidgetMarquee->setCurrentIndex(qmc2UseMarqueeFile ? 1 : 0);
  radioButtonMarqueeSelect->setText(qmc2UseMarqueeFile ? tr("Marquee file") : tr("Marquee directory"));
  lineEditTitleDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/TitleDirectory", QMC2_DEFAULT_DATA_PATH + "/ttl/").toString());
  lineEditTitleFile->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/TitleFile", QMC2_DEFAULT_DATA_PATH + "/ttl/titles.zip").toString());
  comboBoxTitleFileType->setCurrentIndex(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/TitleFileType", QMC2_IMG_FILETYPE_ZIP).toInt());
  qmc2UseTitleFile = config->value("UME/FilesAndDirectories/UseTitleFile", false).toBool();
  stackedWidgetTitle->setCurrentIndex(qmc2UseTitleFile ? 1 : 0);
  radioButtonTitleSelect->setText(qmc2UseTitleFile ? tr("Title file") : tr("Title directory"));
  lineEditPCBDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/PCBDirectory", QMC2_DEFAULT_DATA_PATH + "/pcb/").toString());
  lineEditPCBFile->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/PCBFile", QMC2_DEFAULT_DATA_PATH + "/pcb/pcbs.zip").toString());
  comboBoxPCBFileType->setCurrentIndex(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/PCBFileType", QMC2_IMG_FILETYPE_ZIP).toInt());
  qmc2UsePCBFile = config->value("UME/FilesAndDirectories/UsePCBFile", false).toBool();
  stackedWidgetPCB->setCurrentIndex(qmc2UsePCBFile ? 1 : 0);
  radioButtonPCBSelect->setText(qmc2UsePCBFile ? tr("PCB file") : tr("PCB directory"));
  lineEditSoftwareSnapDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/SoftwareSnapDirectory", QMC2_DEFAULT_DATA_PATH + "/sws/").toString());
  lineEditSoftwareSnapFile->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/SoftwareSnapFile", QMC2_DEFAULT_DATA_PATH + "/sws/swsnaps.zip").toString());
  comboBoxSoftwareSnapFileType->setCurrentIndex(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/SoftwareSnapFileType", QMC2_IMG_FILETYPE_ZIP).toInt());
  qmc2UseSoftwareSnapFile = config->value("UME/FilesAndDirectories/UseSoftwareSnapFile", false).toBool();
  stackedWidgetSWSnap->setCurrentIndex(qmc2UseSoftwareSnapFile ? 1 : 0);
  radioButtonSoftwareSnapSelect->setText(qmc2UseSoftwareSnapFile ? tr("SW snap file") : tr("SW snap folder"));
  lineEditSoftwareNotesFolder->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/SoftwareNotesFolder", QMC2_DEFAULT_DATA_PATH + "/swn/").toString());
  lineEditSoftwareNotesTemplate->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/SoftwareNotesTemplate", QMC2_DEFAULT_DATA_PATH + "/swn/template.html").toString());
  checkBoxUseSoftwareNotesTemplate->setChecked(config->value("UME/FilesAndDirectories/UseSoftwareNotesTemplate", false).toBool());
  lineEditSystemNotesFolder->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/SystemNotesFolder", QMC2_DEFAULT_DATA_PATH + "/gmn/").toString());
  lineEditSystemNotesTemplate->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/SystemNotesTemplate", QMC2_DEFAULT_DATA_PATH + "/gmn/template.html").toString());
  checkBoxUseSystemNotesTemplate->setChecked(config->value("UME/FilesAndDirectories/UseSystemNotesTemplate", false).toBool());
  lineEditMameHistoryDat->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MameHistoryDat", QMC2_DEFAULT_DATA_PATH + "/cat/history.dat").toString());
  lineEditMessSysinfoDat->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MessSysinfoDat", QMC2_DEFAULT_DATA_PATH + "/cat/sysinfo.dat").toString());
  lineEditMameInfoDat->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MameInfoDat", QMC2_DEFAULT_DATA_PATH + "/cat/mameinfo.dat").toString());
  lineEditMessInfoDat->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/MessInfoDat", QMC2_DEFAULT_DATA_PATH + "/cat/messinfo.dat").toString());
  lineEditCatverIniFile->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/CatverIni", QMC2_DEFAULT_DATA_PATH + "/cat/catver.ini").toString());
  checkBoxUseCatverIni->setChecked(config->value(QMC2_FRONTEND_PREFIX + "Gamelist/UseCatverIni", false).toBool());
  lineEditCategoryIniFile->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/CategoryIni", QMC2_DEFAULT_DATA_PATH + "/cat/category.ini").toString());
  checkBoxUseCategoryIni->setChecked(config->value(QMC2_FRONTEND_PREFIX + "Gamelist/UseCategoryIni", false).toBool());
#endif
  lineEditSoftwareInfoDB->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareInfoDB", QMC2_DEFAULT_DATA_PATH + "/cat/history.dat").toString());

  // Gamelist
  checkBoxShowROMStatusIcons->setChecked(config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowROMStatusIcons", true).toBool());
  checkBoxShowDeviceSets->setChecked(config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowDeviceSets", true).toBool());
  checkBoxShowBiosSets->setChecked(config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowBiosSets", true).toBool());
  checkBoxAutoTriggerROMCheck->setChecked(config->value(QMC2_FRONTEND_PREFIX + "Gamelist/AutoTriggerROMCheck", false).toBool());
  checkBoxDoubleClickActivation->setChecked(config->value(QMC2_FRONTEND_PREFIX + "Gamelist/DoubleClickActivation", true).toBool());
  checkBoxPlayOnSublistActivation->setChecked(config->value(QMC2_FRONTEND_PREFIX + "Gamelist/PlayOnSublistActivation", false).toBool());
  qmc2CursorPositioningMode = (QAbstractItemView::ScrollHint)config->value(QMC2_FRONTEND_PREFIX + "Gamelist/CursorPosition", QMC2_CURSOR_POS_TOP).toInt();
  comboBoxCursorPosition->setCurrentIndex((int)qmc2CursorPositioningMode);
  qmc2DefaultLaunchMode = config->value(QMC2_FRONTEND_PREFIX + "Gamelist/DefaultLaunchMode", QMC2_LAUNCH_MODE_INDEPENDENT).toInt();
  comboBoxDefaultLaunchMode->setCurrentIndex(qmc2DefaultLaunchMode);
  qmc2SoftwareSnapPosition = config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SoftwareSnapPosition", QMC2_SWSNAP_POS_BELOW_LEFT).toInt();
  comboBoxSoftwareSnapPosition->setCurrentIndex(qmc2SoftwareSnapPosition);
  checkBoxSoftwareSnapOnMouseHover->setChecked(config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/SoftwareSnapOnMouseHover", false).toBool());
  checkBoxAutoDisableSoftwareSnap->setChecked(config->value(QMC2_FRONTEND_PREFIX + "Layout/SoftwareList/AutoDisableSoftwareSnap", true).toBool());
  spinBoxResponsiveness->setValue(config->value(QMC2_FRONTEND_PREFIX + "Gamelist/Responsiveness", 100).toInt());
  qmc2GamelistResponsiveness = spinBoxResponsiveness->value();
  spinBoxUpdateDelay->setValue(config->value(QMC2_FRONTEND_PREFIX + "Gamelist/UpdateDelay", 10).toInt());
  qmc2UpdateDelay = spinBoxUpdateDelay->value();
  comboBoxSortCriteria->setCurrentIndex(config->value(QMC2_FRONTEND_PREFIX + "Gamelist/SortCriteria", 0).toInt());
  qmc2SortCriteria = comboBoxSortCriteria->currentIndex();
  comboBoxSortOrder->setCurrentIndex(config->value(QMC2_FRONTEND_PREFIX + "Gamelist/SortOrder", 0).toInt());
  qmc2SortOrder = comboBoxSortOrder->currentIndex() == 0 ? Qt::AscendingOrder : Qt::DescendingOrder;
  qmc2Filter.setBit(QMC2_ROMSTATE_INT_C, config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowC", true).toBool());
  toolButtonShowC->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_C]);
  qmc2Filter.setBit(QMC2_ROMSTATE_INT_M, config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowM", true).toBool());
  toolButtonShowM->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_M]);
  qmc2Filter.setBit(QMC2_ROMSTATE_INT_I, config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowI", true).toBool());
  toolButtonShowI->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_I]);
  qmc2Filter.setBit(QMC2_ROMSTATE_INT_N, config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowN", true).toBool());
  toolButtonShowN->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_N]);
  qmc2Filter.setBit(QMC2_ROMSTATE_INT_U, config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowU", true).toBool());
  toolButtonShowU->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_U]);
  bool rsf = config->value(QMC2_FRONTEND_PREFIX + "Gamelist/EnableRomStateFilter", true).toBool();
  checkBoxRomStateFilter->setChecked(rsf);
  if ( !qmc2EarlyStartup ) {
	  qmc2MainWindow->pushButtonSelectRomFilter->setVisible(rsf);
	  qmc2MainWindow->actionRomStatusFilterC->setEnabled(rsf);
	  qmc2MainWindow->actionRomStatusFilterM->setEnabled(rsf);
	  qmc2MainWindow->actionRomStatusFilterI->setEnabled(rsf);
	  qmc2MainWindow->actionRomStatusFilterN->setEnabled(rsf);
	  qmc2MainWindow->actionRomStatusFilterU->setEnabled(rsf);
  }
  if ( qmc2MainWindow ) {
    qmc2StatesTogglesEnabled = false;
    qmc2MainWindow->actionRomStatusFilterC->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_C]);
    qmc2MainWindow->actionRomStatusFilterM->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_M]);
    qmc2MainWindow->actionRomStatusFilterI->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_I]);
    qmc2MainWindow->actionRomStatusFilterN->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_N]);
    qmc2MainWindow->actionRomStatusFilterU->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_U]);
    if ( !qmc2EarlyStartup )
      qmc2StatesTogglesEnabled = true;
  }

  // Shortcuts / Keys
  QMapIterator<QString, QPair<QString, QAction *> > it(qmc2ShortcutMap);
  while ( it.hasNext() ) {
    it.next();
    QString itShortcut = it.key();
    QString itFunction = it.value().first;
    QTreeWidgetItem *item = new QTreeWidgetItem(treeWidgetShortcuts);
    item->setText(0, itFunction);
    QStringList words = itShortcut.split("+");
    QString itemText;
    int i;
    for (i = 0; i < words.count(); i++) {
      if ( i > 0 ) itemText += "+";
      itemText += QObject::tr(words[i].toLocal8Bit());
    }
    item->setText(1, itemText);
    QString customSC = config->value(QString(QMC2_FRONTEND_PREFIX + "Shortcuts/%1").arg(itShortcut), itShortcut).toString();
    qmc2CustomShortcutMap[itShortcut] = customSC;
    if ( customSC != itShortcut ) {
      words = customSC.split("+");
      customSC = "";
      for (i = 0; i < words.count(); i++) {
        if ( i > 0 ) customSC += "+";
        customSC += QObject::tr(words[i].toLocal8Bit());
      }
      item->setText(2, customSC);
    }
  }

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
      qmc2JoystickFunctionMap.insertMulti(joyMapFunction, itShortcut);
      item->setText(1, joyMapFunction);
    }
  }
#endif

  // Network / Tools
  checkBoxRestoreCookies->setChecked(config->value(QMC2_FRONTEND_PREFIX + "WebBrowser/RestoreCookies", true).toBool());
  lineEditCookieDatabase->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "WebBrowser/CookieDatabase", userScopePath + "/qmc2-" + QMC2_EMU_NAME_VARIANT.toLower() + "-cookies.db").toString());
  lineEditZipTool->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "Tools/ZipTool", "zip").toString());
  lineEditZipToolRemovalArguments->setText(config->value(QMC2_FRONTEND_PREFIX + "Tools/ZipToolRemovalArguments", "$ARCHIVE$ -d $FILELIST$").toString());
  lineEditSevenZipTool->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "Tools/SevenZipTool", "7za").toString());
  lineEditSevenZipToolRemovalArguments->setText(config->value(QMC2_FRONTEND_PREFIX + "Tools/SevenZipToolRemovalArguments", "d $ARCHIVE$ $FILELIST$").toString());
  lineEditRomTool->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "Tools/RomTool", "").toString());
  lineEditRomToolArguments->setText(config->value(QMC2_FRONTEND_PREFIX + "Tools/RomToolArguments", "$ID$ \"$DESCRIPTION$\"").toString());
  lineEditRomToolWorkingDirectory->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_FRONTEND_PREFIX + "Tools/RomToolWorkingDirectory", "").toString());
  checkBoxCopyToolOutput->setChecked(config->value(QMC2_FRONTEND_PREFIX + "Tools/CopyToolOutput", true).toBool());
  checkBoxCloseToolDialog->setChecked(config->value(QMC2_FRONTEND_PREFIX + "Tools/CloseToolDialog", false).toBool());

  groupBoxHTTPProxy->setChecked(config->value("Network/HTTPProxy/Enable", false).toBool());
  lineEditHTTPProxyHost->setText(config->value("Network/HTTPProxy/Host", "").toString());
  spinBoxHTTPProxyPort->setValue(config->value("Network/HTTPProxy/Port", 80).toInt());
  lineEditHTTPProxyUserID->setText(config->value("Network/HTTPProxy/UserID", "").toString());
  lineEditHTTPProxyPassword->setText(QString(QMC2_UNCOMPRESS(config->value("Network/HTTPProxy/Password", "").toByteArray())));

  // Emulator

  // Configuration
  if ( qmc2GuiReady )
    qmc2GlobalEmulatorOptions->load();

  // Files and directories
#if defined(QMC2_EMUTYPE_MAME)
  lineEditExecutableFile->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/ExecutableFile", "").toString());
  lineEditWorkingDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/WorkingDirectory", "").toString());
#if defined(QMC2_VARIANT_LAUNCHER) && defined(QMC2_OS_WIN)
  lineEditMESSVariantExe->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/MESSVariantExe", "").toString());
  lineEditUMEVariantExe->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/UMEVariantExe", "").toString());
#endif
  lineEditEmulatorLogFile->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/LogFile", userScopePath + "/mame.log").toString());
  lineEditXmlCacheDatabase->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/XmlCacheDatabase", userScopePath + "/mame-xml-cache.db").toString());
  lineEditUserDataDatabase->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/UserDataDatabase", userScopePath + "/mame-user-data.db").toString());
  lineEditGamelistCacheFile->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/GamelistCacheFile", userScopePath + "/mame.glc").toString());
  lineEditROMStateCacheFile->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/ROMStateCacheFile", userScopePath + "/mame.rsc").toString());
  //lineEditSlotInfoCacheFile->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/SlotInfoCacheFile", userScopePath + "/mame.sic").toString());
  lineEditSoftwareListCache->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/SoftwareListCache", userScopePath + "/mame.swl").toString());
  QString mawsCachePath = config->value("MAME/FilesAndDirectories/MAWSCacheDirectory", userScopePath + "/maws/").toString();
  QDir mawsCacheDir(mawsCachePath);
  mawsCachePath = mawsCacheDir.absolutePath();
  if ( !mawsCacheDir.exists() )
    mawsCacheDir.mkdir(mawsCachePath);
  lineEditMAWSCacheDirectory->setText(mawsCachePath);
#if defined(QMC2_SDLMAME)
  lineEditOptionsTemplateFile->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/OptionsTemplateFile", QMC2_DEFAULT_DATA_PATH + "/opt/SDLMAME/template.xml").toString());
#elif defined(QMC2_MAME)
  lineEditOptionsTemplateFile->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/OptionsTemplateFile", QMC2_DEFAULT_DATA_PATH + "/opt/MAME/template.xml").toString());
#endif
  lineEditFavoritesFile->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/FavoritesFile", userScopePath + "/mame.fav").toString());
  lineEditHistoryFile->setText(QMC2_QSETTINGS_CAST(config)->value("MAME/FilesAndDirectories/HistoryFile", userScopePath + "/mame.hst").toString());
#elif defined(QMC2_EMUTYPE_MESS)
  lineEditExecutableFile->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/ExecutableFile", "").toString());
  lineEditWorkingDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/WorkingDirectory", "").toString());
#if defined(QMC2_VARIANT_LAUNCHER) && defined(QMC2_OS_WIN)
  lineEditMAMEVariantExe->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/MAMEVariantExe", "").toString());
  lineEditUMEVariantExe->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/UMEVariantExe", "").toString());
#endif
  lineEditEmulatorLogFile->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/LogFile", userScopePath + "/mess.log").toString());
  lineEditXmlCacheDatabase->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/XmlCacheDatabase", userScopePath + "/mess-xml-cache.db").toString());
  lineEditUserDataDatabase->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/UserDataDatabase", userScopePath + "/mess-user-data.db").toString());
  lineEditGamelistCacheFile->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/GamelistCacheFile", userScopePath + "/mess.glc").toString());
  lineEditROMStateCacheFile->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/ROMStateCacheFile", userScopePath + "/mess.rsc").toString());
  lineEditSlotInfoCacheFile->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/SlotInfoCacheFile", userScopePath + "/mess.sic").toString());
  lineEditSoftwareListCache->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/SoftwareListCache", userScopePath + "/mess.swl").toString());
  lineEditGeneralSoftwareFolder->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/GeneralSoftwareFolder", QString()).toString());
#if defined(QMC2_SDLMESS)
  lineEditOptionsTemplateFile->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/OptionsTemplateFile", QMC2_DEFAULT_DATA_PATH + "/opt/SDLMESS/template.xml").toString());
#elif defined(QMC2_MESS)
  lineEditOptionsTemplateFile->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/OptionsTemplateFile", QMC2_DEFAULT_DATA_PATH + "/opt/MESS/template.xml").toString());
#endif
  lineEditFavoritesFile->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/FavoritesFile", userScopePath + "/mess.fav").toString());
  lineEditHistoryFile->setText(QMC2_QSETTINGS_CAST(config)->value("MESS/FilesAndDirectories/HistoryFile", userScopePath + "/mess.hst").toString());
#elif defined(QMC2_EMUTYPE_UME)
  lineEditExecutableFile->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/ExecutableFile", "").toString());
  lineEditWorkingDirectory->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/WorkingDirectory", "").toString());
#if defined(QMC2_VARIANT_LAUNCHER) && defined(QMC2_OS_WIN)
  lineEditMAMEVariantExe->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/MAMEVariantExe", "").toString());
  lineEditMESSVariantExe->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/MESSVariantExe", "").toString());
#endif
  lineEditEmulatorLogFile->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/LogFile", userScopePath + "/ume.log").toString());
  lineEditXmlCacheDatabase->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/XmlCacheDatabase", userScopePath + "/ume-xml-cache.db").toString());
  lineEditUserDataDatabase->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/UserDataDatabase", userScopePath + "/ume-user-data.db").toString());
  lineEditGamelistCacheFile->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/GamelistCacheFile", userScopePath + "/ume.glc").toString());
  lineEditROMStateCacheFile->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/ROMStateCacheFile", userScopePath + "/ume.rsc").toString());
  lineEditSlotInfoCacheFile->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/SlotInfoCacheFile", userScopePath + "/ume.sic").toString());
  lineEditSoftwareListCache->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/SoftwareListCache", userScopePath + "/ume.swl").toString());
  lineEditGeneralSoftwareFolder->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/GeneralSoftwareFolder", QString()).toString());
#if defined(QMC2_SDLUME)
  lineEditOptionsTemplateFile->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/OptionsTemplateFile", QMC2_DEFAULT_DATA_PATH + "/opt/SDLUME/template.xml").toString());
#elif defined(QMC2_UME)
  lineEditOptionsTemplateFile->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/OptionsTemplateFile", QMC2_DEFAULT_DATA_PATH + "/opt/UME/template.xml").toString());
#endif
  lineEditFavoritesFile->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/FavoritesFile", userScopePath + "/ume.fav").toString());
  lineEditHistoryFile->setText(QMC2_QSETTINGS_CAST(config)->value("UME/FilesAndDirectories/HistoryFile", userScopePath + "/ume.hst").toString());
#endif
  QDir swStateCacheDir(config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareStateCache", userScopePath + "/sw-state-cache/").toString());
  if ( !swStateCacheDir.exists() )
	  swStateCacheDir.mkdir(swStateCacheDir.absolutePath());
  lineEditSoftwareStateCache->setText(QMC2_QSETTINGS_CAST(config)->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareStateCache", userScopePath + "/sw-state-cache/").toString());
  checkBoxAutoClearEmuCaches->setChecked(QMC2_QSETTINGS_CAST(config)->value(QMC2_EMULATOR_PREFIX + "AutoClearEmuCaches", true).toBool());

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

void Options::applyDelayed()
{
	// paranoia :)
	if ( qmc2MainWindow == NULL ) {
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
		if ( config->value(QMC2_FRONTEND_PREFIX + "GUI/RestoreLayout").toBool() ) {
			tabWidgetFrontendSettings->setCurrentIndex(config->value(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/FrontendTab", 0).toInt());
			tabWidgetGlobalMAMESetup->setCurrentIndex(config->value(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/MAMETab", 0).toInt());
			tabWidgetOptions->setCurrentIndex(config->value(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/OptionsTab", 0).toInt());
			QStringList cl = config->allKeys();
			if ( cl.contains(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/Size") )
				resize(config->value(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/Size").toSize());
			if ( cl.contains(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/Position") )
				move(config->value(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/Position").toPoint());
			if ( cl.contains(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/Visible") )
				if ( config->value(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/Visible").toBool() )
					show();
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
		}
		firstTime = false;
	}

	// adjust row-sizes of foreign emulator table-widget items
	tableWidgetRegisteredEmulators->resizeRowsToContents();

	// redraw detail if setup changed
	qmc2MainWindow->on_tabWidgetGameDetail_currentChanged(qmc2MainWindow->tabWidgetGameDetail->currentIndex());

	if ( !cancelClicked ) {
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
		QString displayFormat = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/CustomIDSetup/DisplayFormat", "$ID$ - $DESCRIPTION$").toString();
		config->beginGroup(QMC2_EMULATOR_PREFIX + "RegisteredEmulators");
		QStringList registeredEmus = config->childGroups();
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
				QStringList idList = config->value(QMC2_EMULATOR_PREFIX + QString("CustomIDs/%1/IDs").arg(emuName), QStringList()).toStringList();
				if ( !idList.isEmpty() ) {
					QStringList descriptionList = config->value(QMC2_EMULATOR_PREFIX + QString("CustomIDs/%1/Descriptions").arg(emuName), QStringList()).toStringList();
					while ( descriptionList.count() < idList.count() )
						descriptionList << QString();
					QStringList iconList = config->value(QMC2_EMULATOR_PREFIX + QString("CustomIDs/%1/Icons").arg(emuName), QStringList()).toStringList();
					while ( iconList.count() < idList.count() )
						iconList << QString();
					for (int i = 0; i < idList.count(); i++) {
						QString id = idList[i];
						if ( !id.isEmpty() ) {
							QString description = descriptionList[i];
							QString idIcon = iconList[i];
							QString itemText = displayFormat;
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
			int index = qmc2MainWindow->tabWidgetGamelist->indexOf(qmc2MainWindow->tabForeignEmulators);
			if ( index == -1 ) {
				qmc2MainWindow->tabWidgetGamelist->insertTab(QMC2_FOREIGN_INDEX, qmc2MainWindow->tabForeignEmulators, tr("Foreign IDs"));
				qmc2MainWindow->tabWidgetGamelist->setTabIcon(QMC2_FOREIGN_INDEX, QIcon(QString::fromUtf8(":/data/img/alien.png")));
			}
		} else {
			int index = qmc2MainWindow->tabWidgetGamelist->indexOf(qmc2MainWindow->tabForeignEmulators);
			if ( index >= 0 )
				qmc2MainWindow->tabWidgetGamelist->removeTab(index);
		}

		// restore foreign ID selection
		QStringList foreignIdState = qmc2Config->value(QMC2_EMULATOR_PREFIX + "SelectedForeignID", QStringList()).toStringList();
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
	if ( qmc2MainWindow->tabWidgetGamelist->currentIndex() != QMC2_EMBED_INDEX || !qmc2MainWindow->toolButtonEmbedderMaximizeToggle->isChecked() )
		qmc2MainWindow->menuBar()->setVisible(checkBoxShowMenuBar->isChecked());
#else
	qmc2MainWindow->menuBar()->setVisible(checkBoxShowMenuBar->isChecked());
#endif
	qApp->processEvents();
	qmc2VariantSwitchReady = true;
	cancelClicked = false;
}

void Options::on_pushButtonClearCookieDatabase_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_pushButtonClearCookieDatabase_clicked()");
#endif

	if ( qmc2NetworkAccessManager ) {
		CookieJar *cj = (CookieJar *)qmc2NetworkAccessManager->cookieJar();
		cj->recreateDatabase();
	}
}

void Options::on_pushButtonManageCookies_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_pushButtonManageCookies_clicked()");
#endif

	CookieManager cm(this);
	cm.exec();
}

void Options::on_pushButtonAdditionalArtworkSetup_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_pushButtonAdditionalArtworkSetup_clicked()");
#endif

	AdditionalArtworkSetup as(this);
	as.exec();
}

void Options::on_pushButtonImageFormats_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_pushButtonImageFormats_clicked()");
#endif

	ImageFormatSetup ifs(this);
	ifs.exec();
}

#if defined(QMC2_EMUTYPE_UME)
void Options::on_toolButtonCompressMameHistoryDat_clicked()
{
	toolButtonCompressMessSysinfoDat->setChecked(toolButtonCompressMameHistoryDat->isChecked());
}

void Options::on_toolButtonCompressMessSysinfoDat_clicked()
{
	toolButtonCompressMameHistoryDat->setChecked(toolButtonCompressMessSysinfoDat->isChecked());
}

void Options::on_toolButtonCompressMameInfoDat_clicked()
{
	toolButtonCompressMessInfoDat->setChecked(toolButtonCompressMameInfoDat->isChecked());
}

void Options::on_toolButtonCompressMessInfoDat_clicked()
{
	toolButtonCompressMameInfoDat->setChecked(toolButtonCompressMessInfoDat->isChecked());
}
#endif

void Options::on_toolButtonBrowseStyleSheet_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseStyleSheet_clicked()");
#endif

  QString s = QFileDialog::getOpenFileName(this, tr("Choose Qt style sheet file"), lineEditStyleSheet->text(), tr("Qt Style Sheets (*.qss)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
  if ( !s.isNull() )
    lineEditStyleSheet->setText(s);
  raise();
}

void Options::on_toolButtonBrowseTemporaryFile_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseTemporaryFile_clicked()");
#endif

  QString s = QFileDialog::getOpenFileName(this, tr("Choose temporary work file"), lineEditTemporaryFile->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
  if ( !s.isNull() )
    lineEditTemporaryFile->setText(s);
  raise();
}

void Options::on_toolButtonBrowsePreviewDirectory_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowsePreviewDirectory_clicked()");
#endif

  QString s = QFileDialog::getExistingDirectory(this, tr("Choose preview directory"), lineEditPreviewDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
  if ( !s.isNull() ) {
    if ( !s.endsWith("/") ) s += "/";
    lineEditPreviewDirectory->setText(s);
  }
  raise();
}

void Options::on_toolButtonBrowseFlyerDirectory_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseFlyerDirectory_clicked()");
#endif

  QString s = QFileDialog::getExistingDirectory(this, tr("Choose flyer directory"), lineEditFlyerDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
  if ( !s.isNull() ) {
    if ( !s.endsWith("/") ) s += "/";
    lineEditFlyerDirectory->setText(s);
  }
  raise();
}

void Options::on_toolButtonBrowseIconDirectory_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseIconDirectory_clicked()");
#endif

  QString s = QFileDialog::getExistingDirectory(this, tr("Choose icon directory"), lineEditIconDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
  if ( !s.isNull() ) {
    if ( !s.endsWith("/") ) s += "/";
    lineEditIconDirectory->setText(s);
  }
  raise();
}

void Options::on_toolButtonBrowseCabinetDirectory_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseCabinetDirectory_clicked()");
#endif

  QString s = QFileDialog::getExistingDirectory(this, tr("Choose cabinet directory"), lineEditCabinetDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
  if ( !s.isNull() ) {
    if ( !s.endsWith("/") ) s += "/";
    lineEditCabinetDirectory->setText(s);
  }
  raise();
}

void Options::on_toolButtonBrowseControllerDirectory_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseControllerDirectory_clicked()");
#endif

  QString s = QFileDialog::getExistingDirectory(this, tr("Choose controller directory"), lineEditControllerDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
  if ( !s.isNull() ) {
    if ( !s.endsWith("/") ) s += "/";
    lineEditControllerDirectory->setText(s);
  }
  raise();
}

void Options::on_toolButtonBrowseMarqueeDirectory_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseMarqueeDirectory_clicked()");
#endif

#if defined(QMC2_EMUTYPE_MESS)
  QString s = QFileDialog::getExistingDirectory(this, tr("Choose logo directory"), lineEditMarqueeDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
#else
  QString s = QFileDialog::getExistingDirectory(this, tr("Choose marquee directory"), lineEditMarqueeDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
#endif
  if ( !s.isNull() ) {
    if ( !s.endsWith("/") ) s += "/";
    lineEditMarqueeDirectory->setText(s);
  }
  raise();
}

void Options::on_toolButtonBrowseTitleDirectory_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseTitleDirectory_clicked()");
#endif

  QString s = QFileDialog::getExistingDirectory(this, tr("Choose title directory"), lineEditTitleDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
  if ( !s.isNull() ) {
    if ( !s.endsWith("/") ) s += "/";
    lineEditTitleDirectory->setText(s);
  }
  raise();
}

void Options::on_toolButtonBrowsePCBDirectory_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowsePCBDirectory_clicked()");
#endif

  QString s = QFileDialog::getExistingDirectory(this, tr("Choose PCB directory"), lineEditPCBDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
  if ( !s.isNull() ) {
    if ( !s.endsWith("/") ) s += "/";
    lineEditPCBDirectory->setText(s);
  }
  raise();
}

void Options::on_toolButtonBrowseOptionsTemplateFile_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseOptionsTemplateFile_clicked()");
#endif

  QString s = QFileDialog::getOpenFileName(this, tr("Choose options template file"), lineEditOptionsTemplateFile->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
  if ( !s.isNull() )
    lineEditOptionsTemplateFile->setText(s);
  raise();
}

void Options::on_toolButtonBrowseExecutableFile_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseExecutableFile_clicked()");
#endif

  QString s = QFileDialog::getOpenFileName(this, tr("Choose emulator executable file"), lineEditExecutableFile->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
  if ( !s.isNull() )
    lineEditExecutableFile->setText(s);
  raise();
}

#if defined(QMC2_VARIANT_LAUNCHER) && defined(QMC2_OS_WIN)
void Options::on_toolButtonBrowseMAMEVariantExe_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseMAMEVariantExe_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose MAME variant's exe file"), lineEditMAMEVariantExe->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditMAMEVariantExe->setText(s);
	raise();
}

void Options::on_toolButtonBrowseMESSVariantExe_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseMESSVariantExe_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose MESS variant's exe file"), lineEditMESSVariantExe->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditMESSVariantExe->setText(s);
	raise();
}

void Options::on_toolButtonBrowseUMEVariantExe_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseUMEVariantExe_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose UME variant's exe file"), lineEditUMEVariantExe->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditUMEVariantExe->setText(s);
	raise();
}

void Options::mameVariantSpecifyArguments()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::mameVariantSpecifyArguments()");
#endif

#if !defined(QMC2_EMUTYPE_MAME)
	bool ok;
	QString mameVariantExeArgs = QInputDialog::getText(this,
							tr("MAME variant arguments"),
							tr("Specify command line arguments passed to the MAME variant\n(empty means: 'pass the arguments we were called with'):"),
							QLineEdit::Normal,
							config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/MAMEVariantExeArguments", "").toString(),
							&ok);
	if ( ok )
		config->setValue(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/MAMEVariantExeArguments", mameVariantExeArgs);
#endif
}

void Options::messVariantSpecifyArguments()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::messVariantSpecifyArguments()");
#endif

#if !defined(QMC2_EMUTYPE_MESS)
	bool ok;
	QString messVariantExeArgs = QInputDialog::getText(this,
							tr("MESS variant arguments"),
							tr("Specify command line arguments passed to the MESS variant\n(empty means: 'pass the arguments we were called with'):"),
							QLineEdit::Normal,
							config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/MESSVariantExeArguments", "").toString(),
							&ok);
	if ( ok )
		config->setValue(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/MESSVariantExeArguments", messVariantExeArgs);
#endif
}

void Options::umeVariantSpecifyArguments()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::umeVariantSpecifyArguments()");
#endif

#if !defined(QMC2_EMUTYPE_UME)
	bool ok;
	QString umeVariantExeArgs = QInputDialog::getText(this,
							tr("UME variant arguments"),
							tr("Specify command line arguments passed to the UME variant\n(empty means: 'pass the arguments we were called with'):"),
							QLineEdit::Normal,
							config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/UMEVariantExeArguments", "").toString(),
							&ok);
	if ( ok )
		config->setValue(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/UMEVariantExeArguments", umeVariantExeArgs);
#endif
}
#endif

void Options::on_toolButtonBrowseEmulatorLogFile_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseEmulatorLogFile_clicked()");
#endif

  QString s = QFileDialog::getOpenFileName(this, tr("Choose emulator log file"), lineEditEmulatorLogFile->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
  if ( !s.isNull() )
    lineEditEmulatorLogFile->setText(s);
  raise();
}

void Options::on_toolButtonBrowseXmlCacheDatabase_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseXmlCacheDatabase_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose XML cache database file"), lineEditXmlCacheDatabase->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditXmlCacheDatabase->setText(s);
	raise();
}

void Options::on_toolButtonClearUserDataDatabase_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonClearUserDataDatabase_clicked()");
#endif

	if ( qmc2Gamelist->userDataDb()->userDataRowCount() > 0 ) {
		switch ( QMessageBox::question(this, tr("Confirm"), tr("This will remove <b>all</b> existing user data and recreate the database.\nAre you sure you want to do this?"), tr("&Yes"), tr("&No"), QString(), 0, 1) ) {
			case 0:
				break;

			default:
			case 1:
				return;
		}
	}

	qmc2Gamelist->userDataDb()->clearRankCache();
	qmc2Gamelist->userDataDb()->clearCommentCache();
	qmc2Gamelist->userDataDb()->recreateDatabase();
	qmc2Gamelist->userDataDb()->setEmulatorVersion(qmc2Gamelist->emulatorVersion);
	qmc2Gamelist->userDataDb()->setQmc2Version(XSTR(QMC2_VERSION));
	qmc2Gamelist->userDataDb()->setUserDataVersion(QMC2_USERDATA_VERSION);
	QTimer::singleShot(0, qmc2MainWindow, SLOT(updateUserData()));
}

void Options::on_toolButtonCleanupUserDataDatabase_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonCleanupUserDataDatabase_clicked()");
#endif

	qmc2Gamelist->userDataDb()->cleanUp();
}

void Options::on_toolButtonBrowseUserDataDatabase_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseUserDataDatabase_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose user data database file"), lineEditUserDataDatabase->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditUserDataDatabase->setText(s);
	raise();
}

void Options::on_toolButtonBrowseCookieDatabase_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseCookieDatabase_clicked()");
#endif

	QString s = QFileDialog::getSaveFileName(this, tr("Choose cookie database file"), lineEditCookieDatabase->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditCookieDatabase->setText(s);
	raise();
}

void Options::on_toolButtonBrowseZipTool_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseZipTool_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose zip tool"), lineEditZipTool->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditZipTool->setText(s);
	raise();
}

void Options::on_toolButtonBrowseSevenZipTool_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseSevenZipTool_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose 7-zip tool"), lineEditSevenZipTool->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditSevenZipTool->setText(s);
	raise();
}

void Options::on_toolButtonBrowseRomTool_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseRomTool_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose ROM tool"), lineEditRomTool->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditRomTool->setText(s);
	raise();
}

void Options::on_toolButtonBrowseRomToolWorkingDirectory_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseRomToolWorkingDirectory_clicked()");
#endif

	QString s = QFileDialog::getExistingDirectory(this, tr("Choose working directory"), lineEditRomToolWorkingDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditRomToolWorkingDirectory->setText(s);
	raise();
}

void Options::on_toolButtonBrowseFavoritesFile_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseFavoritesFile_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose game favorites file"), lineEditFavoritesFile->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditFavoritesFile->setText(s);
	raise();
}

void Options::on_toolButtonBrowseHistoryFile_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseHistoryFile_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose play history file"), lineEditHistoryFile->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditHistoryFile->setText(s);
	raise();
}

void Options::on_toolButtonBrowseGamelistCacheFile_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseGamelistCacheFile_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose gamelist cache file"), lineEditGamelistCacheFile->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() ) {
		if ( !s.endsWith("/") ) s += "/";
		lineEditGamelistCacheFile->setText(s);
	}
	raise();
}

void Options::on_toolButtonBrowseROMStateCacheFile_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseROMStateCacheFile_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose ROM state cache file"), lineEditROMStateCacheFile->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditROMStateCacheFile->setText(s);
	raise();
}

void Options::on_toolButtonBrowseSlotInfoCacheFile_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseSlotInfoCacheFile_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose slot info cache file"), lineEditSlotInfoCacheFile->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditSlotInfoCacheFile->setText(s);
	raise();
}

void Options::on_toolButtonBrowseWorkingDirectory_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseWorkingDirectory_clicked()");
#endif

	QString s = QFileDialog::getExistingDirectory(this, tr("Choose working directory"), lineEditWorkingDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() ) {
		if ( !s.endsWith("/") ) s += "/";
		lineEditWorkingDirectory->setText(s);
	}
	raise();
}

void Options::on_toolButtonBrowseMAWSCacheDirectory_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseMAWSCacheDirectory_clicked()");
#endif

	QString s = QFileDialog::getExistingDirectory(this, tr("Choose MAWS cache directory"), lineEditMAWSCacheDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() ) {
		if ( !s.endsWith("/") ) s += "/";
		lineEditMAWSCacheDirectory->setText(s);
	}
	raise();
}

void Options::on_toolButtonBrowseSoftwareListCache_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseSoftwareListCache_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose software list cache file"), lineEditSoftwareListCache->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditSoftwareListCache->setText(s);
	raise();
}

void Options::on_toolButtonBrowseSoftwareStateCache_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseSoftwareStateCache_clicked()");
#endif

	QString s = QFileDialog::getExistingDirectory(this, tr("Choose software state cache directory"), lineEditSoftwareStateCache->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isEmpty() ) {
		if ( !s.endsWith("/") )
			s += "/";
		lineEditSoftwareStateCache->setText(s);
	}
	raise();
}

void Options::on_toolButtonBrowseGeneralSoftwareFolder_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseGeneralSoftwareFolder_clicked()");
#endif

	QString s = QFileDialog::getExistingDirectory(this, tr("Choose general software folder"), lineEditGeneralSoftwareFolder->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() ) {
		if ( !s.endsWith("/") ) s += "/";
		lineEditGeneralSoftwareFolder->setText(s);
	}
	raise();
}

void Options::on_toolButtonBrowseFrontendLogFile_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseFrontendLogFile_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose front end log file"), lineEditFrontendLogFile->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditFrontendLogFile->setText(s);
	raise();
}

void Options::on_toolButtonBrowseDataDirectory_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseDataDirectory_clicked()");
#endif

	QString s = QFileDialog::getExistingDirectory(this, tr("Choose data directory"), lineEditDataDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() ) {
		if ( !s.endsWith("/") ) s += "/";
		lineEditDataDirectory->setText(s);
	}
	raise();
}

void Options::on_toolButtonBrowseMameHistoryDat_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseMameHistoryDat_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose MAME game info DB"), lineEditMameHistoryDat->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditMameHistoryDat->setText(s);
	raise();
}

void Options::on_toolButtonBrowseMessSysinfoDat_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseMessSysinfoDat_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose MESS machine info DB"), lineEditMessSysinfoDat->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditMessSysinfoDat->setText(s);
	raise();
}

void Options::on_toolButtonBrowseMameInfoDat_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseMameInfoDat_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose MAME emulator info DB"), lineEditMameInfoDat->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditMameInfoDat->setText(s);
	raise();
}

void Options::on_toolButtonBrowseMessInfoDat_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseMessInfoDat_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose MESS emulator info DB"), lineEditMessInfoDat->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditMessInfoDat->setText(s);
	raise();
}

void Options::on_toolButtonBrowseSoftwareInfoDB_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseSoftwareInfoDB_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose software info DB"), lineEditSoftwareInfoDB->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditSoftwareInfoDB->setText(s);
	raise();
}

#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
void Options::on_toolButtonBrowseCatverIniFile_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseCatverIniFile_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose catver.ini file"), lineEditCatverIniFile->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditCatverIniFile->setText(s);
	raise();
}
#endif

#if defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
void Options::on_toolButtonBrowseCategoryIniFile_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseCategoryIniFile_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose category.ini file"), lineEditCategoryIniFile->text(), tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditCategoryIniFile->setText(s);
	raise();
}
#endif

void Options::on_toolButtonBrowseFont_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseFont_clicked()");
#endif

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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseLogFont_clicked()");
#endif

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

void Options::showEvent(QShowEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Options::showEvent(QShowEvent *e = %1)").arg((qulonglong)e));
#endif

	if ( !qmc2CleaningUp && !qmc2EarlyStartup )
		if ( config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
			config->setValue(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/Visible", true);

	e->accept();
}

void Options::hideEvent(QHideEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Options::hideEvent(QHideEvent *e = %1)").arg((qulonglong)e));
#endif

	if ( !qmc2CleaningUp && !qmc2EarlyStartup )
		if ( config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
			config->setValue(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/Visible", false);

	e->accept();
}

void Options::moveEvent(QMoveEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Options::moveEvent(QMoveEvent *e = %1)").arg((qulonglong)e));
#endif

	if ( !qmc2CleaningUp && !qmc2EarlyStartup )
		if ( config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
			config->setValue(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/Position", pos());

	e->accept();
}

void Options::resizeEvent(QResizeEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Options::resizeEvent(QResizeEvent *e = %1)").arg((qulonglong)e));
#endif

	if ( !qmc2CleaningUp && !qmc2EarlyStartup )
		if ( config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
			config->setValue(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/Size", size());

	e->accept();
}

void Options::on_radioButtonPreviewSelect_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_radioButtonPreviewSelect_clicked()");
#endif

	bool currentUsePreviewFile = (stackedWidgetPreview->currentIndex() == 1);
	stackedWidgetPreview->setCurrentIndex(!currentUsePreviewFile);
	radioButtonPreviewSelect->setText(!currentUsePreviewFile ? tr("Preview file") : tr("Preview directory"));
}

void Options::on_radioButtonFlyerSelect_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_radioButtonFlyerSelect_clicked()");
#endif

	bool currentUseFlyerFile = (stackedWidgetFlyer->currentIndex() == 1);
	stackedWidgetFlyer->setCurrentIndex(!currentUseFlyerFile);
	radioButtonFlyerSelect->setText(!currentUseFlyerFile ? tr("Flyer file") : tr("Flyer directory"));
}

void Options::on_radioButtonIconSelect_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_radioButtonIconSelect_clicked()");
#endif

	bool currentUseIconFile = (stackedWidgetIcon->currentIndex() == 1);
	stackedWidgetIcon->setCurrentIndex(!currentUseIconFile);
	radioButtonIconSelect->setText(!currentUseIconFile ? tr("Icon file") : tr("Icon directory"));
}

void Options::on_radioButtonCabinetSelect_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_radioButtonCabinetSelect_clicked()");
#endif

	bool currentUseCabinetFile = (stackedWidgetCabinet->currentIndex() == 1);
	stackedWidgetCabinet->setCurrentIndex(!currentUseCabinetFile);
	radioButtonCabinetSelect->setText(!currentUseCabinetFile ? tr("Cabinet file") : tr("Cabinet directory"));
}

void Options::on_radioButtonControllerSelect_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_radioButtonControllerSelect_clicked()");
#endif

	bool currentUseControllerFile = (stackedWidgetController->currentIndex() == 1);
	stackedWidgetController->setCurrentIndex(!currentUseControllerFile);
	radioButtonControllerSelect->setText(!currentUseControllerFile ? tr("Controller file") : tr("Controller directory"));
}

void Options::on_radioButtonMarqueeSelect_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_radioButtonMarqueeSelect_clicked()");
#endif

	bool currentUseMarqueeFile = (stackedWidgetMarquee->currentIndex() == 1);
	stackedWidgetMarquee->setCurrentIndex(!currentUseMarqueeFile);
#if defined(QMC2_EMUTYPE_MESS)
	radioButtonMarqueeSelect->setText(!currentUseMarqueeFile ? tr("Logo file") : tr("Logo directory"));
#else
	radioButtonMarqueeSelect->setText(!currentUseMarqueeFile ? tr("Marquee file") : tr("Marquee directory"));
#endif
}

void Options::on_radioButtonTitleSelect_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_radioButtonTitleSelect_clicked()");
#endif

	bool currentUseTitleFile = (stackedWidgetTitle->currentIndex() == 1);
	stackedWidgetTitle->setCurrentIndex(!currentUseTitleFile);
	radioButtonTitleSelect->setText(!currentUseTitleFile ? tr("Title file") : tr("Title directory"));
}

void Options::on_radioButtonPCBSelect_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_radioButtonPCBSelect_clicked()");
#endif

	bool currentUsePCBFile = (stackedWidgetPCB->currentIndex() == 1);
	stackedWidgetPCB->setCurrentIndex(!currentUsePCBFile);
	radioButtonPCBSelect->setText(!currentUsePCBFile ? tr("PCB file") : tr("PCB directory"));
}

void Options::on_radioButtonSoftwareSnapSelect_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_radioButtonSoftwareSnapSelect_clicked()");
#endif

	bool currentUseSoftwareSnapFile = (stackedWidgetSWSnap->currentIndex() == 1);
	stackedWidgetSWSnap->setCurrentIndex(!currentUseSoftwareSnapFile);
	radioButtonSoftwareSnapSelect->setText(!currentUseSoftwareSnapFile ? tr("SW snap file") : tr("SW snap folder"));
}

void Options::on_toolButtonBrowsePreviewFile_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowsePreviewFile_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose compressed preview file"), lineEditPreviewFile->text(), tr("Supported archives") + " (*.[zZ][iI][pP] *.7[zZ]);;" + tr("ZIP archives") + " (*.[zZ][iI][pP]);;" + tr("7z archives") + " (*.7[zZ]);;" + tr("All files") + " (*)", 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() ) {
		lineEditPreviewFile->setText(s);
		if ( s.toLower().endsWith(".zip") )
			comboBoxPreviewFileType->setCurrentIndex(QMC2_IMG_FILETYPE_ZIP);
		else if ( s.toLower().endsWith(".7z") )
			comboBoxPreviewFileType->setCurrentIndex(QMC2_IMG_FILETYPE_7Z);
	}
	raise();
}

void Options::on_toolButtonBrowseFlyerFile_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseFlyerFile_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose compressed flyer file"), lineEditFlyerFile->text(), tr("Supported archives") + " (*.[zZ][iI][pP] *.7[zZ]);;" + tr("ZIP archives") + " (*.[zZ][iI][pP]);;" + tr("7z archives") + " (*.7[zZ]);;" + tr("All files") + " (*)", 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() ) {
		lineEditFlyerFile->setText(s);
		if ( s.toLower().endsWith(".zip") )
			comboBoxFlyerFileType->setCurrentIndex(QMC2_IMG_FILETYPE_ZIP);
		else if ( s.toLower().endsWith(".7z") )
			comboBoxFlyerFileType->setCurrentIndex(QMC2_IMG_FILETYPE_7Z);
	}
	raise();
}

void Options::on_toolButtonBrowseIconFile_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseIconFile_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose compressed icon file"), lineEditIconFile->text(), tr("Supported archives") + " (*.[zZ][iI][pP] *.7[zZ]);;" + tr("ZIP archives") + " (*.[zZ][iI][pP]);;" + tr("7z archives") + " (*.7[zZ]);;" + tr("All files") + " (*)", 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() ) {
		lineEditIconFile->setText(s);
		if ( s.toLower().endsWith(".zip") )
			comboBoxIconFileType->setCurrentIndex(QMC2_IMG_FILETYPE_ZIP);
		else if ( s.toLower().endsWith(".7z") )
			comboBoxIconFileType->setCurrentIndex(QMC2_IMG_FILETYPE_7Z);
	}
	raise();
}

void Options::on_toolButtonBrowseCabinetFile_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseCabinetFile_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose compressed cabinet file"), lineEditCabinetFile->text(), tr("Supported archives") + " (*.[zZ][iI][pP] *.7[zZ]);;" + tr("ZIP archives") + " (*.[zZ][iI][pP]);;" + tr("7z archives") + " (*.7[zZ]);;" + tr("All files") + " (*)", 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() ) {
		lineEditCabinetFile->setText(s);
		if ( s.toLower().endsWith(".zip") )
			comboBoxCabinetFileType->setCurrentIndex(QMC2_IMG_FILETYPE_ZIP);
		else if ( s.toLower().endsWith(".7z") )
			comboBoxCabinetFileType->setCurrentIndex(QMC2_IMG_FILETYPE_7Z);
	}
	raise();
}

void Options::on_toolButtonBrowseControllerFile_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseControllerFile_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose compressed controller file"), lineEditControllerFile->text(), tr("Supported archives") + " (*.[zZ][iI][pP] *.7[zZ]);;" + tr("ZIP archives") + " (*.[zZ][iI][pP]);;" + tr("7z archives") + " (*.7[zZ]);;" + tr("All files") + " (*)", 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() ) {
		lineEditControllerFile->setText(s);
		if ( s.toLower().endsWith(".zip") )
			comboBoxControllerFileType->setCurrentIndex(QMC2_IMG_FILETYPE_ZIP);
		else if ( s.toLower().endsWith(".7z") )
			comboBoxControllerFileType->setCurrentIndex(QMC2_IMG_FILETYPE_7Z);
	}
	raise();
}

void Options::on_toolButtonBrowseMarqueeFile_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseMarqueeFile_clicked()");
#endif

#if defined(QMC2_EMUTYPE_MESS)
	QString s = QFileDialog::getOpenFileName(this, tr("Choose compressed logo file"), lineEditMarqueeFile->text(), tr("Supported archives") + " (*.[zZ][iI][pP] *.7[zZ]);;" + tr("ZIP archives") + " (*.[zZ][iI][pP]);;" + tr("7z archives") + " (*.7[zZ]);;" + tr("All files") + " (*)", 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
#else
	QString s = QFileDialog::getOpenFileName(this, tr("Choose compressed marquee file"), lineEditMarqueeFile->text(), tr("Supported archives") + " (*.[zZ][iI][pP] *.7[zZ]);;" + tr("ZIP archives") + " (*.[zZ][iI][pP]);;" + tr("7z archives") + " (*.7[zZ]);;" + tr("All files") + " (*)", 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
#endif
	if ( !s.isNull() ) {
		lineEditMarqueeFile->setText(s);
		if ( s.toLower().endsWith(".zip") )
			comboBoxMarqueeFileType->setCurrentIndex(QMC2_IMG_FILETYPE_ZIP);
		else if ( s.toLower().endsWith(".7z") )
			comboBoxMarqueeFileType->setCurrentIndex(QMC2_IMG_FILETYPE_7Z);
	}
	raise();
}

void Options::on_toolButtonBrowseTitleFile_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseTitleFile_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose compressed title file"), lineEditTitleFile->text(), tr("Supported archives") + " (*.[zZ][iI][pP] *.7[zZ]);;" + tr("ZIP archives") + " (*.[zZ][iI][pP]);;" + tr("7z archives") + " (*.7[zZ]);;" + tr("All files") + " (*)", 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() ) {
		lineEditTitleFile->setText(s);
		if ( s.toLower().endsWith(".zip") )
			comboBoxTitleFileType->setCurrentIndex(QMC2_IMG_FILETYPE_ZIP);
		else if ( s.toLower().endsWith(".7z") )
			comboBoxTitleFileType->setCurrentIndex(QMC2_IMG_FILETYPE_7Z);
	}
	raise();
}

void Options::on_toolButtonBrowsePCBFile_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowsePCBFile_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose compressed PCB file"), lineEditPCBFile->text(), tr("Supported archives") + " (*.[zZ][iI][pP] *.7[zZ]);;" + tr("ZIP archives") + " (*.[zZ][iI][pP]);;" + tr("7z archives") + " (*.7[zZ]);;" + tr("All files") + " (*)", 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() ) {
		lineEditPCBFile->setText(s);
		if ( s.toLower().endsWith(".zip") )
			comboBoxPCBFileType->setCurrentIndex(QMC2_IMG_FILETYPE_ZIP);
		else if ( s.toLower().endsWith(".7z") )
			comboBoxPCBFileType->setCurrentIndex(QMC2_IMG_FILETYPE_7Z);
	}
	raise();
}

void Options::on_toolButtonBrowseSoftwareSnapDirectory_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseSoftwareSnapDirectory_clicked()");
#endif

	QString s = QFileDialog::getExistingDirectory(this, tr("Choose software snap directory"), lineEditSoftwareSnapDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() ) {
		if ( !s.endsWith("/") ) s += "/";
		lineEditSoftwareSnapDirectory->setText(s);
	}
	raise();
}

void Options::on_toolButtonBrowseSoftwareSnapFile_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseSoftwareSnapFile_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose compressed software snap file"), lineEditSoftwareSnapFile->text(), tr("Supported archives") + " (*.[zZ][iI][pP] *.7[zZ]);;" + tr("ZIP archives") + " (*.[zZ][iI][pP]);;" + tr("7z archives") + " (*.7[zZ]);;" + tr("All files") + " (*)", 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() ) {
		lineEditSoftwareSnapFile->setText(s);
		if ( s.toLower().endsWith(".zip") )
			comboBoxSoftwareSnapFileType->setCurrentIndex(QMC2_IMG_FILETYPE_ZIP);
		else if ( s.toLower().endsWith(".7z") )
			comboBoxSoftwareSnapFileType->setCurrentIndex(QMC2_IMG_FILETYPE_7Z);
	}
	raise();
}

void Options::on_toolButtonBrowseSoftwareNotesFolder_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseSoftwareNotesFolder_clicked()");
#endif

	QString s = QFileDialog::getExistingDirectory(this, tr("Choose software notes folder"), lineEditSoftwareNotesFolder->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() ) {
		if ( !s.endsWith("/") ) s += "/";
		lineEditSoftwareNotesFolder->setText(s);
	}
	raise();
}

void Options::on_toolButtonBrowseSoftwareNotesTemplate_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseSoftwareNotesTemplate_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose software notes template"), lineEditSoftwareNotesTemplate->text(), tr("HTML files (*.html *.htm)") + ";;" + tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditSoftwareNotesTemplate->setText(s);
	raise();
}

void Options::on_toolButtonBrowseSystemNotesFolder_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseSystemNotesFolder_clicked()");
#endif

	QString s = QFileDialog::getExistingDirectory(this, tr("Choose system notes folder"), lineEditSystemNotesFolder->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() ) {
		if ( !s.endsWith("/") ) s += "/";
		lineEditSystemNotesFolder->setText(s);
	}
	raise();
}

void Options::on_toolButtonBrowseSystemNotesTemplate_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseSystemNotesTemplate_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose system notes template"), lineEditSystemNotesTemplate->text(), tr("HTML files (*.html *.htm)") + ";;" + tr("All files (*)"), 0, useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditSystemNotesTemplate->setText(s);
	raise();
}

void Options::on_treeWidgetShortcuts_itemActivated(QTreeWidgetItem *item)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Options::on_treeWidgetShortcuts_itemActivated(QTreeWidgetItem *item = %1)").arg((qulonglong)item));
#endif

	if ( !item )
		return;

	qApp->removeEventFilter(qmc2KeyPressFilter);

	KeySequenceScanner keySeqScanner(this, qmc2QtKeyMap.contains(item->text(1)));
	if ( keySeqScanner.exec() == QDialog::Accepted ) {
		QStringList words = item->text(1).split("+");
		QString nativeShortcut = "";
		for (int i = 0; i < words.count(); i++) {
			if ( i > 0 ) nativeShortcut += "+";
			nativeShortcut += QObject::tr(words[i].toLocal8Bit());
		}

		bool found = false;
		QMapIterator<QString, QPair<QString, QAction *> > it(qmc2ShortcutMap);
		while ( it.hasNext() && !found ) {
			it.next();
			words = it.key().split("+");
			QString itShortcut;
			for (int i = 0; i < words.count(); i++) {
				if ( i > 0 ) itShortcut += "+";
				itShortcut += QObject::tr(words[i].toLocal8Bit());
			}

			if ( itShortcut == nativeShortcut ) {
				found = true;
				nativeShortcut = it.key();
			}
		}

		if ( found ) {
			qmc2CustomShortcutMap[nativeShortcut] = keySeqScanner.currentKeySequence;
			item->setText(2, keySeqScanner.labelKeySequence->text());
			QTimer::singleShot(0, this, SLOT(checkShortcuts()));
		}

		pushButtonResetShortcut->setEnabled(true);
	}

	qApp->installEventFilter(qmc2KeyPressFilter);
}

void Options::on_treeWidgetShortcuts_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_treeWidgetShortcuts_itemSelectionChanged()");
#endif

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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_pushButtonRedefineKeySequence_clicked()");
#endif

	QList<QTreeWidgetItem *> selItems = treeWidgetShortcuts->selectedItems();
	if ( selItems.count() > 0 )
		on_treeWidgetShortcuts_itemActivated(selItems[0]);
}

void Options::on_pushButtonResetShortcut_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_pushButtonResetShortcut_clicked()");
#endif

	QList<QTreeWidgetItem *> selItems = treeWidgetShortcuts->selectedItems();
	if ( selItems.count() > 0 ) {
		QStringList words = selItems[0]->text(1).split("+");
		QString nativeShortcut = "";
		for (int i = 0; i < words.count(); i++) {
			if ( i > 0 ) nativeShortcut += "+";
			nativeShortcut += QObject::tr(words[i].toLocal8Bit());
		}

		bool found = false;
		QMapIterator<QString, QPair<QString, QAction *> > it(qmc2ShortcutMap);
		while ( it.hasNext() && !found ) {
			it.next();
			words = it.key().split("+");
			QString itShortcut;
			for (int i = 0; i < words.count(); i++) {
				if ( i > 0 ) itShortcut += "+";
				itShortcut += QObject::tr(words[i].toLocal8Bit());
			}

			if ( itShortcut == nativeShortcut ) {
				found = true;
				nativeShortcut = it.key();
			}
		}

		if ( found ) {
			qmc2CustomShortcutMap[nativeShortcut] = nativeShortcut;
			selItems[0]->setText(2, "");
			QTimer::singleShot(0, this, SLOT(checkShortcuts()));
		}

		pushButtonResetShortcut->setEnabled(false);
	}
}

void Options::on_pushButtonDetailSetup_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_pushButtonDetailSetup_clicked()");
#endif

	qmc2DetailSetupParent = this;
	qmc2MainWindow->menuTabWidgetGameDetail_Setup_activated();
}

void Options::on_pushButtonCustomizeToolBar_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_pushButtonCustomizeToolBar_clicked()");
#endif

	if ( !qmc2ToolBarCustomizer )
		qmc2ToolBarCustomizer = new ToolBarCustomizer(this);

	qmc2ToolBarCustomizer->exec();
}

void Options::on_pushButtonEditPalette_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_pushButtonEditPalette_clicked()");
#endif

	QPalette currentPalette = qApp->palette();
	if ( !qmc2PaletteEditor )
		qmc2PaletteEditor = new PaletteEditor(this);
	loadCustomPalette(qmc2CurrentStyleName);
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
			qApp->setPalette(qmc2StandardPalettes[qmc2CurrentStyleName]);
		else
			qApp->setPalette(qmc2CustomPalette);
	}
	QTimer::singleShot(0, qmc2MainWindow, SLOT(updateUserData()));
}

void Options::checkShortcuts()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::checkShortcuts()");
#endif

	static char lastShortcutsState = -1;

	char shortcutsState = 1;
	int itemCount = treeWidgetShortcuts->topLevelItemCount();
	int i, j;
	for (i = 0; i < itemCount; i++) {
		QTreeWidgetItem *iItem = treeWidgetShortcuts->topLevelItem(i);
		if ( iItem->text(2).isEmpty() )
			iItem->setForeground(1, greenBrush);
		else
			iItem->setForeground(1, greyBrush);
		iItem->setForeground(2, greenBrush);
	}
	for (i = 0; i < itemCount; i++) {
		QTreeWidgetItem *iItem = treeWidgetShortcuts->topLevelItem(i);
		QString iShortcut;
		int iColumn = 1;
		if ( !iItem->text(2).isEmpty() )
			iColumn = 2;
		iShortcut = iItem->text(iColumn);
		for (j = i + 1; j < itemCount; j++) {
			QTreeWidgetItem *jItem = treeWidgetShortcuts->topLevelItem(j);
			QString jShortcut;
			int jColumn = 1;
			if ( !jItem->text(2).isEmpty() )
				jColumn = 2;
			jShortcut = jItem->text(jColumn);
			if ( iShortcut == jShortcut ) {
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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::setupShortcutActions()");
#endif

	QMapIterator<QString, QPair<QString, QAction *> > it(qmc2ShortcutMap);
	while ( it.hasNext() ) {
		it.next();
		QAction *action = it.value().second;
		if ( action ) {
			action->setShortcut(QKeySequence(qmc2CustomShortcutMap[it.key()]));
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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonAddEmulator_clicked()");
#endif

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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonSaveEmulator_clicked()");
#endif

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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonRemoveEmulator_clicked()");
#endif

	QList<QTableWidgetItem *> sl = tableWidgetRegisteredEmulators->selectedItems();
	if ( !sl.isEmpty() ) {
		int row = sl[0]->row();
		registeredEmulatorsToBeRemoved << tableWidgetRegisteredEmulators->item(row, QMC2_ADDTLEMUS_COLUMN_NAME)->text();
		tableWidgetRegisteredEmulators->removeRow(row);
	}
}

void Options::on_toolButtonCustomIDs_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonCustomIDs_clicked()");
#endif

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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::checkPlaceholderStatus()");
#endif

	QPalette pal = labelIDStatus->palette();
	QString text = lineEditAdditionalEmulatorArguments->text();
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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_tableWidgetRegisteredEmulators_itemSelectionChanged()");
#endif

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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Options::on_lineEditAdditionalEmulatorName_textChanged(const QString &text = %1)").arg(text));
#endif

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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_lineEditAdditionalEmulatorArguments_textChanged(const QString &)");
#endif

	checkPlaceholderStatus();
}

void Options::setupCustomIDsClicked()
{
	QToolButton *tb = (QToolButton *)sender();
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Options::setupCustomIDsClicked() : tb = %1)").arg((qulonglong)tb));
#endif
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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Options::chooseEmuIconClicked() : tb = %1)").arg((qulonglong)tb));
#endif
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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Options::actionDefaultEmuIconTriggered() : action = %1)").arg((qulonglong)action));
#endif
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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Options::actionNoEmuIconTriggered() : action = %1)").arg((qulonglong)action));
#endif
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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Options::loadCustomPalette(QString styleName = %1)").arg(styleName));
#endif

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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::saveCustomPalette()");
#endif

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

#if QMC2_JOYSTICK == 1
void Options::on_pushButtonRescanJoysticks_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_pushButtonRescanJoysticks_clicked()");
#endif

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
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonCalibrateAxes_clicked()");
#endif

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
        joystickTestWidget = NULL;
      } else if ( joystickCalibrationWidget ) {
        myLayout->removeWidget(scrollArea);
        scrollArea->takeWidget();
        scrollArea->hide();
        delete joystickCalibrationWidget;
        joystickCalibrationWidget = NULL;
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
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonTestJoystick_clicked()");
#endif

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
        joystickTestWidget = NULL;
      } else if ( joystickCalibrationWidget ) {
        myLayout->removeWidget(scrollArea);
        scrollArea->takeWidget();
        scrollArea->hide();
        delete joystickCalibrationWidget;
        joystickCalibrationWidget = NULL;
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
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonMapJoystick_clicked()");
#endif

  qmc2JoystickIsCalibrating = false;

  bool relayout = ( joystickCalibrationWidget || joystickTestWidget );
  
  if ( joystickCalibrationWidget ) {
    frameCalibrationAndTest->layout()->removeWidget(scrollArea);
    scrollArea->takeWidget();
    scrollArea->hide();
    delete joystickCalibrationWidget;
    joystickCalibrationWidget = NULL;
  }
  if ( joystickTestWidget ) {
    frameCalibrationAndTest->layout()->removeWidget(scrollArea);
    scrollArea->takeWidget();
    scrollArea->hide();
    delete joystickTestWidget;
    joystickTestWidget = NULL;
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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Options::on_comboBoxSelectJoysticks_currentIndexChanged(int index = %1)").arg(index));
#endif

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
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Options::on_checkBoxEnableJoystickControl_toggled(bool enable = %1)").arg(enable));
#endif

  toolButtonMapJoystick->setChecked(true);
  on_toolButtonMapJoystick_clicked();
}

void Options::on_checkBoxJoystickAutoRepeat_toggled(bool repeat)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Options::on_checkBoxJoystickAutoRepeat_toggled(bool repeat = %1)").arg(repeat));
#endif

  if ( joystick )
    joystick->autoRepeat = repeat;
}

void Options::on_spinBoxJoystickAutoRepeatTimeout_valueChanged(int value)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Options::on_spinBoxJoystickAutoRepeatTimeout_valueChanged(int value = %1)").arg(value));
#endif

  if ( joystick )
    joystick->autoRepeatDelay = value;
}

void Options::on_spinBoxJoystickEventTimeout_valueChanged(int value)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Options::on_spinBoxJoystickEventTimeout_valueChanged(int value = %1)").arg(value));
#endif

  if ( joystick ) {
    joystick->eventTimeout = value;
    if ( joystick->isOpen() )
      joystick->joystickTimer.start(joystick->eventTimeout);
  }
}

void Options::on_treeWidgetJoystickMappings_itemActivated(QTreeWidgetItem *item)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Options::on_treeWidgetJoystickMappings_itemActivated(QTreeWidgetItem *item = %1)").arg((qulonglong)item));
#endif

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
        qmc2JoystickFunctionMap.insertMulti(joyFuncScanner.labelJoystickFunction->text(), item->whatsThis(0));
        pushButtonRemoveJoystickMapping->setEnabled(item->text(1).length() > 0);
        QTimer::singleShot(0, this, SLOT(checkJoystickMappings()));
      }
      qmc2SuppressQtMessages = saveSQM;
    }
  }
}

void Options::on_treeWidgetJoystickMappings_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_treeWidgetJoystickMappings_itemSelectionChanged()");
#endif

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
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_pushButtonRemapJoystickFunction_clicked()");
#endif

  QList<QTreeWidgetItem *> selItems = treeWidgetJoystickMappings->selectedItems();
  if ( selItems.count() > 0 )
    on_treeWidgetJoystickMappings_itemActivated(selItems[0]);
}

void Options::on_pushButtonRemoveJoystickMapping_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_pushButtonRemapJoystickFunction_clicked()");
#endif

  QList<QTreeWidgetItem *> selItems = treeWidgetJoystickMappings->selectedItems();
  if ( selItems.count() > 0 ) {
    QList<QString> valueList = qmc2JoystickFunctionMap.values(selItems[0]->text(1));
    qmc2JoystickFunctionMap.remove(selItems[0]->text(1));
    if ( valueList.count() > 1 ) {
      int i;
      QString valueToRemove = selItems[0]->whatsThis(0);
      for (i = 0; i < valueList.count(); i++)
        if ( valueList[i] != valueToRemove )
          qmc2JoystickFunctionMap.insertMulti(selItems[0]->text(1), valueList[i]);
    }
    selItems[0]->setText(1, "");
    pushButtonRemoveJoystickMapping->setEnabled(false);
    QTimer::singleShot(0, this, SLOT(checkJoystickMappings()));
  }
}

void Options::checkJoystickMappings()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::checkJoystickMappings()");
#endif

  static char lastJoystickMappingsState = -1;

  char joystickMappingsState = 1;
  int itemCount = treeWidgetJoystickMappings->topLevelItemCount();
  int i, j;
  for (i = 0; i < itemCount; i++) {
    QTreeWidgetItem *iItem = treeWidgetJoystickMappings->topLevelItem(i);
    if ( !iItem->text(1).isEmpty() )
      iItem->setForeground(1, greenBrush);
  }
  for (i = 0; i < itemCount; i++) {
    QTreeWidgetItem *iItem = treeWidgetJoystickMappings->topLevelItem(i);
    QString iMapping = iItem->text(1);
    for (j = i + 1; j < itemCount; j++) {
      QTreeWidgetItem *jItem = treeWidgetJoystickMappings->topLevelItem(j);
      QString jMapping = jItem->text(1);
      if ( iMapping == jMapping && !jMapping.isEmpty() ) {
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
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: JoystickCalibrationWidget::JoystickCalibrationWidget(Joystick *joystick = %1, QWidget *parent = %2)").arg((qulonglong)joystick).arg((qulonglong)parent));
#endif

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
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: JoystickCalibrationWidget::~JoystickCalibrationWidget()");
#endif

  // ignore destruction when we are already cleaning up the application...
  if ( !qmc2Options->frameCalibrationAndTest->layout() )
    return;

  int i;
  // remove spacer item first
  QLayoutItem *layoutItem = (QLayoutItem *)myLayout->takeAt(myJoystick->numAxes);
  delete layoutItem;
  for (i = myJoystick->numAxes - 1; i >= 0; i--) {
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
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: JoystickCalibrationWidget::on_joystickAxisValueChanged(int axis = %1, int value = %2)").arg(axis).arg(value));
#endif

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
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: JoystickCalibrationWidget::on_joystickButtonValueChanged(int button = %1, bool value = %2)").arg(button).arg(value));
#endif

}

void JoystickCalibrationWidget::on_joystickHatValueChanged(int hat, int value)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: JoystickCalibrationWidget::on_joystickHatValueChanged(int hat = %1, int value = %2)").arg(hat).arg(value));
#endif

}

void JoystickCalibrationWidget::on_joystickTrackballValueChanged(int trackball, int deltaX, int deltaY)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: JoystickCalibrationWidget::on_joystickTrackballValueChanged(int trackball = %1, int deltaX = %2, int deltaY = %3)").arg(trackball).arg(deltaX).arg(deltaY));
#endif

}

void JoystickCalibrationWidget::on_resetAxisCalibration()
{
  QToolButton *pressedButton = (QToolButton *)sender();

#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: JoystickCalibrationWidget::on_resetAxisCalibration()");
#endif

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

#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: JoystickCalibrationWidget::on_deadzoneValueChanged(int value = %1)").arg(value));
#endif

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

#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: JoystickCalibrationWidget::on_sensitivityValueChanged(int value = %1)").arg(value));
#endif

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

#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: JoystickCalibrationWidget::on_axisEnablerStateChanged(int state = %1)").arg(state));
#endif

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
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: JoystickTestWidget::JoystickTestWidget(Joystick *joystick = %1, QWidget *parent = %2)").arg((qulonglong)joystick).arg((qulonglong)parent));
#endif

  myJoystick = joystick;

  int i;
  int joyIndex = qmc2Options->comboBoxSelectJoysticks->currentIndex();
  int maxRows = QMC2_MAX(QMC2_MAX(QMC2_MAX(myJoystick->numAxes, myJoystick->numButtons), myJoystick->numHats), myJoystick->numTrackballs);

  myLayout = new QGridLayout(this);

  for (i = 0; i < myJoystick->numAxes; i++) {
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

  for (i = 0; i < myJoystick->numButtons; i++) {
    buttonLabels[i] = new QLabel(tr("B%1").arg(i), this);
    buttonLabels[i]->setToolTip(tr("Current state of button %1").arg(i));
    buttonLabels[i]->setAlignment(Qt::AlignCenter);
    buttonLabels[i]->setAutoFillBackground(true);
    buttonLabels[i]->setFrameShape(QFrame::Box);
    buttonLabels[i]->setSizePolicy(QSizePolicy::Expanding, myJoystick->numButtons < maxRows ? QSizePolicy::Ignored : QSizePolicy::Minimum);

    myLayout->addWidget(buttonLabels[i], i, 1);
  }

  for (i = 0; i < myJoystick->numHats; i++) {
    hatValueLabels[i] = new QLabel(tr("H%1: 0").arg(i), this);
    hatValueLabels[i]->setToolTip(tr("Current value of hat %1").arg(i));
    hatValueLabels[i]->setAlignment(Qt::AlignCenter);
    hatValueLabels[i]->setAutoFillBackground(true);
    hatValueLabels[i]->setFrameShape(QFrame::Box);
    hatValueLabels[i]->setSizePolicy(QSizePolicy::Expanding, myJoystick->numHats < maxRows ? QSizePolicy::Ignored : QSizePolicy::Minimum);

    myLayout->addWidget(hatValueLabels[i], i, 2);
  }

  for (i = 0; i < myJoystick->numTrackballs; i++) {
    trackballDeltaXLabels[i] = new QLabel(tr("T%1 DX: 0").arg(i), this);
    trackballDeltaXLabels[i]->setToolTip(tr("Current X-delta of trackball %1").arg(i));
    trackballDeltaXLabels[i]->setAlignment(Qt::AlignCenter);
    trackballDeltaXLabels[i]->setAutoFillBackground(true);
    trackballDeltaXLabels[i]->setFrameShape(QFrame::Box);
    trackballDeltaXLabels[i]->setSizePolicy(QSizePolicy::Expanding, myJoystick->numTrackballs < maxRows ? QSizePolicy::Ignored : QSizePolicy::Minimum);

    myLayout->addWidget(trackballDeltaXLabels[i], i, 3);
  }

  for (i = 0; i < myJoystick->numTrackballs; i++) {
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
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: JoystickTestWidget::~JoystickTestWidget()");
#endif

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
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: JoystickTestWidget::on_joystickAxisValueChanged(int axis = %1, int value = %2)").arg(axis).arg(value));
#endif

  axesRanges[axis]->setValue(value);
}

void JoystickTestWidget::on_joystickButtonValueChanged(int button, bool value)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: JoystickTestWidget::on_joystickButtonValueChanged(int button = %1, bool value = %2)").arg(button).arg(value));
#endif

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
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: JoystickTestWidget::on_joystickHatValueChanged(int hat = %1, int value = %2)").arg(hat).arg(value));
#endif

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
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: JoystickTestWidget::on_joystickTrackballValueChanged(int trackball = %1, int deltaX = %2, int deltaY = %3)").arg(trackball).arg(deltaX).arg(deltaY));
#endif

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
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: JoystickTestWidget::cleanupPalette()");
#endif

  int i;
  if ( qApp->styleSheet().isEmpty() ) {
    for (i = 0; i < buttonLabels.count(); i++)
      buttonLabels[i]->setPalette(QApplication::palette());
    for (i = 0; i < hatValueLabels.count(); i++)
      hatValueLabels[i]->setPalette(QApplication::palette());
    for (i = 0; i < trackballDeltaXLabels.count(); i++)
      trackballDeltaXLabels[i]->setPalette(QApplication::palette());
    for (i = 0; i < trackballDeltaYLabels.count(); i++)
      trackballDeltaYLabels[i]->setPalette(QApplication::palette());
  } else {
    for (i = 0; i < buttonLabels.count(); i++)
      buttonLabels[i]->setStyleSheet("");
    for (i = 0; i < hatValueLabels.count(); i++)
      hatValueLabels[i]->setStyleSheet("");
    for (i = 0; i < trackballDeltaXLabels.count(); i++)
      trackballDeltaXLabels[i]->setStyleSheet("");
    for (i = 0; i < trackballDeltaYLabels.count(); i++)
      trackballDeltaYLabels[i]->setStyleSheet("");
  }
}
#endif
