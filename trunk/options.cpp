#include <QPixmapCache>
#include <QTranslator>
#include <QFileInfo>
#include <QFileDialog>
#include <QFontDialog>
#include <QMessageBox>
#include <QTimer>
#include <QMap>
#include <QStyleFactory>
#include <QHeaderView>
#include <QBitArray>
#include <QAction>
#include <QPair>
#include <QLocale>
#include <QNetworkProxy>
#include <QScrollBar>

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
#include "imgcheck.h"
#include "macros.h"
#include "unzip.h"
#include "keyseqscan.h"
#include "romalyzer.h"
#include "romstatusexport.h"
#include "docbrowser.h"
#include "detailsetup.h"
#include "mawsqdlsetup.h"
#if QMC2_JOYSTICK == 1
#include "joystick.h"
#include "joyfuncscan.h"
#endif
#if defined(QMC2_EMUTYPE_MESS)
#include "messdevcfg.h"
#include "messswlist.h"
#endif
#if defined(Q_WS_X11)
#include "embedder.h"
#include "embedderopt.h"
#endif
#if QMC2_USE_PHONON_API
#include "audioeffects.h"
#if defined(QMC2_YOUTUBE_ENABLED)
#include "youtubevideoplayer.h"
#endif
#endif

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
extern Title *qmc2Title;
extern PCB *qmc2PCB;
extern Gamelist *qmc2Gamelist;
extern ImageChecker *qmc2ImageChecker;
extern ROMAlyzer *qmc2ROMAlyzer;
extern ROMStatusExporter *qmc2ROMStatusExporter;
extern DocBrowser *qmc2DocBrowser;
extern int qmc2SortCriteria;
extern Qt::SortOrder qmc2SortOrder;
extern QMap<QString, QString> qmc2GamelistNameMap;
extern QSettings *qmc2Config;
extern QBitArray qmc2Filter;
extern unzFile qmc2IconFile;
extern QMap<QString, QPair<QString, QAction *> > qmc2ShortcutMap;
extern QMap<QString, QString> qmc2CustomShortcutMap;
extern QMap<QString, QString> qmc2JoystickFunctionMap;
extern KeyPressFilter *qmc2KeyPressFilter;
extern QMap<QString, int> qmc2QtKeyMap;
extern QMap<QString, QByteArray *> qmc2GameInfoDB;
extern MiniWebBrowser *qmc2MAWSLookup;
extern MawsQuickDownloadSetup *qmc2MawsQuickDownloadSetup;
extern DetailSetup *qmc2DetailSetup;
extern QWidget *qmc2DetailSetupParent;
#if defined(QMC2_EMUTYPE_MAME)
extern QMap<QString, QByteArray *> qmc2EmuInfoDB;
#endif
#if QMC2_JOYSTICK == 1
extern Joystick *qmc2Joystick;
#endif
#if defined(QMC2_EMUTYPE_MESS)
extern MESSDeviceConfigurator *qmc2MESSDeviceConfigurator;
extern MESSSoftwareList *qmc2MESSSoftwareList;
#endif
#if QMC2_USE_PHONON_API
extern AudioEffectDialog *qmc2AudioEffectDialog;
#if defined(QMC2_YOUTUBE_ENABLED)
extern YouTubeVideoPlayer *qmc2YouTubeWidget;
#endif
#endif
extern QAbstractItemView::ScrollHint qmc2CursorPositioningMode;

Options::Options(QWidget *parent)
#if defined(Q_WS_WIN)
  : QDialog(parent, Qt::Dialog)
#else
  : QDialog(parent, Qt::Dialog | Qt::SubWindow)
#endif
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::Options(QWidget *parent = 0x" + QString::number((ulong)parent, 16) + ")");
#endif

  qmc2Filter.resize(QMC2_ROMSTATE_COUNT);
#if !defined(Q_WS_WIN)
  QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, QMC2_SYSCONF_PATH);
#endif
  QString userScopePath = QMC2_DOT_PATH;
  QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, userScopePath);
  QDir userScopeDir(userScopePath);
  if ( !userScopeDir.exists() )
    userScopeDir.mkdir(userScopePath);
  qApp->setOrganizationName("qmc2");
  config = new QSettings(QSettings::IniFormat, QSettings::UserScope, "qmc2");

  setupUi(this);

#if !defined(QMC2_SHOWMEMINFO)
  checkBoxMemoryIndicator->setVisible(FALSE);
#endif

#if !defined(QMC2_VARIANT_LAUNCHER)
  checkBoxMinimizeOnVariantLaunch->setVisible(FALSE);
  checkBoxExitOnVariantLaunch->setVisible(FALSE);
#endif

#if defined(QMC2_EMUTYPE_MESS)
  toolButtonBrowseEmuInfoDB->setVisible(FALSE);
  checkBoxProcessEmuInfoDB->setVisible(FALSE);
  checkBoxCompressEmuInfoDB->setVisible(FALSE);
  lineEditEmuInfoDB->setVisible(FALSE);
  labelEmuInfoDB->setVisible(FALSE);
  labelEmuInfoDBPic->setVisible(FALSE);
  labelGameInfoDB->setText(tr("Machine info DB"));
  checkBoxProcessGameInfoDB->setText(tr("Load machine info DB"));
  checkBoxProcessGameInfoDB->setToolTip(tr("Load machine information database (MESS sysinfo.dat)"));
  checkBoxCompressGameInfoDB->setToolTip(tr("Use in-memory compression for machine info DB (a bit slower, but consumes distinctly less memory; compression rate is usually about 1:16)"));
  lineEditGameInfoDB->setToolTip(tr("Machine information database - MESS sysinfo.dat (read)"));
  toolButtonBrowseGameInfoDB->setToolTip(tr("Browse machine information database (MESS sysinfo.dat)"));
  tabWidgetFrontendSettings->setTabText(QMC2_OPTIONS_FE_MACHINELIST_INDEX, tr("Machine &list"));
  comboBoxSortCriteria->setItemText(QMC2_SORTCRITERIA_DESCRIPTION, tr("Machine description"));
  comboBoxSortCriteria->setItemText(QMC2_SORTCRITERIA_MACHINENAME, tr("Machine name"));
  spinBoxResponsiveness->setToolTip(tr("Number of item insertions between machine list updates during reload (higher means faster, but makes the GUI less responsive)"));
  spinBoxUpdateDelay->setToolTip(tr("Delay update of any machine details (preview, flyer, info, configuration, ...) by how many milliseconds?"));
  checkBoxSortOnline->setToolTip(tr("Sort machine list while reloading (slower)"));
  checkBoxScaledCabinet->setVisible(FALSE);
  checkBoxScaledController->setVisible(FALSE);
  checkBoxScaledMarquee->setVisible(FALSE);
  checkBoxScaledTitle->setVisible(FALSE);
  radioButtonIconSelect->setVisible(FALSE);
  radioButtonCabinetSelect->setVisible(FALSE);
  radioButtonControllerSelect->setVisible(FALSE);
  radioButtonMarqueeSelect->setVisible(FALSE);
  radioButtonTitleSelect->setVisible(FALSE);
  stackedWidgetIcon->setVisible(FALSE);
  stackedWidgetCabinet->setVisible(FALSE);
  stackedWidgetController->setVisible(FALSE);
  stackedWidgetMarquee->setVisible(FALSE);
  stackedWidgetTitle->setVisible(FALSE);
  labelMAWSCacheDirectory->setVisible(FALSE);
  lineEditMAWSCacheDirectory->setVisible(FALSE);
  toolButtonBrowseMAWSCacheDirectory->setVisible(FALSE);
  checkBoxUseCatverIni->setVisible(FALSE);
  lineEditCatverIniFile->setVisible(FALSE);
  toolButtonBrowseCatverIniFile->setVisible(FALSE);
  labelLegendFrontendGUI->setToolTip(tr("Option requires a reload of the entire machine list to take effect"));
  labelLegendFrontendFilesAndDirectories->setToolTip(tr("Option requires a reload of the entire machine list to take effect"));
  labelLegendEmulatorFilesAndDirectories->setToolTip(tr("Option requires a reload of the entire machine list to take effect"));
  checkBoxHideWhileLoading->setToolTip(tr("Hide primary machine list while loading (recommended, much faster)"));
  tableWidgetRegisteredEmulators->setToolTip(tr("Registered emulators -- you may select one of these in the machine-specific emulator configuration"));
#elif defined(QMC2_EMUTYPE_MAME)
  comboBoxSortCriteria->insertItem(QMC2_SORTCRITERIA_CATEGORY, tr("Category"));
  comboBoxSortCriteria->insertItem(QMC2_SORTCRITERIA_VERSION, tr("Version"));
  labelSoftwareListCache->setVisible(FALSE);
  lineEditSoftwareListCache->setVisible(FALSE);
  toolButtonBrowseSoftwareListCache->setVisible(FALSE);
#endif

  // shortcuts
  qmc2ShortcutMap["Ctrl+1"] = QPair<QString, QAction *>(tr("Check all ROM states"), NULL);
  qmc2ShortcutMap["Ctrl+2"] = QPair<QString, QAction *>(tr("Check all sample sets"), NULL);
  qmc2ShortcutMap["Ctrl+3"] = QPair<QString, QAction *>(tr("Check preview images"), NULL);
  qmc2ShortcutMap["Ctrl+4"] = QPair<QString, QAction *>(tr("Check flyer images"), NULL);
  qmc2ShortcutMap["Ctrl+5"] = QPair<QString, QAction *>(tr("Check icon images"), NULL);
  qmc2ShortcutMap["Ctrl+A"] = QPair<QString, QAction *>(tr("About QMC2"), NULL);
  qmc2ShortcutMap["Ctrl+D"] = QPair<QString, QAction *>(tr("Analyze current game"), NULL);
  qmc2ShortcutMap["Ctrl+E"] = QPair<QString, QAction *>(tr("Export ROM Status"), NULL);
  qmc2ShortcutMap["Ctrl+F"] = QPair<QString, QAction *>(tr("Copy game to favorites"), NULL);
  qmc2ShortcutMap["Ctrl+H"] = QPair<QString, QAction *>(tr("Online documentation"), NULL);
  qmc2ShortcutMap["Ctrl+I"] = QPair<QString, QAction *>(tr("Clear image cache"), NULL);
  qmc2ShortcutMap["Ctrl+Shift+A"] = QPair<QString, QAction *>(tr("Setup arcade mode"), NULL);
#if defined(QMC2_EMUTYPE_MAME)
  qmc2ShortcutMap["Ctrl+Shift+D"] = QPair<QString, QAction *>(tr("Demo mode"), NULL);
#endif
  qmc2ShortcutMap["Ctrl+M"] = QPair<QString, QAction *>(tr("Clear MAWS cache"), NULL);
  qmc2ShortcutMap["Ctrl+N"] = QPair<QString, QAction *>(tr("Clear icon cache"), NULL);
  qmc2ShortcutMap["Ctrl+O"] = QPair<QString, QAction *>(tr("Open options dialog"), NULL);
  qmc2ShortcutMap["Ctrl+P"] = QPair<QString, QAction *>(tr("Play (independent)"), NULL);
#if defined(Q_WS_X11)
  qmc2ShortcutMap["Ctrl+Shift+P"] = QPair<QString, QAction *>(tr("Play (embedded)"), NULL);
#endif
  qmc2ShortcutMap["Ctrl+Q"] = QPair<QString, QAction *>(tr("About Qt"), NULL);
  qmc2ShortcutMap["Ctrl+R"] = QPair<QString, QAction *>(tr("Reload gamelist"), NULL);
  qmc2ShortcutMap["Ctrl+S"] = QPair<QString, QAction *>(tr("Check game's ROM state"), NULL);
  qmc2ShortcutMap["Ctrl+T"] = QPair<QString, QAction *>(tr("Recreate template map"), NULL);
  qmc2ShortcutMap["Ctrl+C"] = QPair<QString, QAction *>(tr("Check template map"), NULL);
  qmc2ShortcutMap["Ctrl+X"] = QPair<QString, QAction *>(tr("Stop processing / exit QMC2"), NULL);
  qmc2ShortcutMap["Ctrl+Z"] = QPair<QString, QAction *>(tr("Open ROMAlyzer dialog"), NULL);
  qmc2ShortcutMap["Ctrl+Alt+C"] = QPair<QString, QAction *>(tr("Toggle ROM state C"), NULL);
  qmc2ShortcutMap["Ctrl+Alt+M"] = QPair<QString, QAction *>(tr("Toggle ROM state M"), NULL);
  qmc2ShortcutMap["Ctrl+Alt+I"] = QPair<QString, QAction *>(tr("Toggle ROM state I"), NULL);
  qmc2ShortcutMap["Ctrl+Alt+N"] = QPair<QString, QAction *>(tr("Toggle ROM state N"), NULL);
  qmc2ShortcutMap["Ctrl+Alt+U"] = QPair<QString, QAction *>(tr("Toggle ROM state U"), NULL);
#if defined(QMC2_VARIANT_LAUNCHER)
#if defined(Q_WS_WIN)
  qmc2ShortcutMap["Ctrl+Alt+1"] = QPair<QString, QAction *>(tr("Launch QMC2 for MAME"), NULL);
  qmc2ShortcutMap["Ctrl+Alt+2"] = QPair<QString, QAction *>(tr("Launch QMC2 for MESS"), NULL);
#else
  qmc2ShortcutMap["Ctrl+Alt+1"] = QPair<QString, QAction *>(tr("Launch QMC2 for SDLMAME"), NULL);
  qmc2ShortcutMap["Ctrl+Alt+2"] = QPair<QString, QAction *>(tr("Launch QMC2 for SDLMESS"), NULL);
#endif
#endif
  qmc2ShortcutMap["F5"] = QPair<QString, QAction *>(tr("Gamelist with full detail"), NULL);
  qmc2ShortcutMap["F6"] = QPair<QString, QAction *>(tr("Parent / clone hierarchy"), NULL);
#if defined(QMC2_EMUTYPE_MAME)
  qmc2ShortcutMap["F7"] = QPair<QString, QAction *>(tr("View games by category"), NULL);
  qmc2ShortcutMap["F8"] = QPair<QString, QAction *>(tr("View games by version"), NULL);
#endif
  qmc2ShortcutMap["F9"] = QPair<QString, QAction *>(tr("Run external ROM tool"), NULL);
  qmc2ShortcutMap["F11"] = QPair<QString, QAction *>(tr("Toggle full screen"), NULL);
  qmc2ShortcutMap["F12"] = QPair<QString, QAction *>(tr("Toggle arcade mode"), NULL);
  qmc2ShortcutMap["Meta+F"] = QPair<QString, QAction *>(tr("Show FPS (arcade mode)"), NULL);
  qmc2ShortcutMap["Meta+F12"] = QPair<QString, QAction *>(tr("Take snapshot (arcade mode)"), NULL);
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

  // special keys
  qmc2ShortcutMap["+"] = QPair<QString, QAction *>(tr("Plus (+)"), NULL);
  qmc2ShortcutMap["-"] = QPair<QString, QAction *>(tr("Minus (-)"), NULL);
  qmc2ShortcutMap["Down"] = QPair<QString, QAction *>(tr("Cursor down"), NULL);
  qmc2ShortcutMap["End"] = QPair<QString, QAction *>(tr("End"), NULL);
  qmc2ShortcutMap["Esc"] = QPair<QString, QAction *>(tr("Escape"), NULL);
  qmc2ShortcutMap["Left"] = QPair<QString, QAction *>(tr("Cursor left"), NULL);
  qmc2ShortcutMap["Home"] = QPair<QString, QAction *>(tr("Home"), NULL);
  qmc2ShortcutMap["PgDown"] = QPair<QString, QAction *>(tr("Page down"), NULL);
  qmc2ShortcutMap["PgUp"] = QPair<QString, QAction *>(tr("Page up"), NULL);
  qmc2ShortcutMap["Return"] = QPair<QString, QAction *>(tr("Enter key"), NULL);
  qmc2ShortcutMap["Right"] = QPair<QString, QAction *>(tr("Cursor right"), NULL);
  qmc2ShortcutMap["Tab"] = QPair<QString, QAction *>(tr("Tabulator"), NULL);
  qmc2ShortcutMap["Up"] = QPair<QString, QAction *>(tr("Cursor up"), NULL);

  if ( !config->isWritable() ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: configuration is not writeable, please check access permissions for ") + config->fileName());
  }

#if QMC2_JOYSTICK != 1
  tabWidgetFrontendSettings->removeTab(tabWidgetFrontendSettings->indexOf(tabFrontendJoystick));
#else
  joystick = NULL;
  joystickCalibrationWidget = NULL;
  joystickTestWidget = NULL;
  scrollArea = new QScrollArea(groupBoxCalibrationAndTest);
  scrollArea->hide();
  scrollArea->setWidgetResizable(TRUE);
#endif

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

#if defined(Q_WS_X11)
  if ( qmc2MainWindow->tabWidgetGamelist->currentIndex() != QMC2_EMBED_INDEX || !qmc2MainWindow->toolButtonEmbedderMaximizeToggle->isChecked() ) {
    qmc2MainWindow->statusBar()->setVisible(config->value(QMC2_FRONTEND_PREFIX + "GUI/Statusbar", TRUE).toBool());
    qmc2MainWindow->toolbar->setVisible(config->value(QMC2_FRONTEND_PREFIX + "GUI/Toolbar", TRUE).toBool());
  }
#else
  qmc2MainWindow->statusBar()->setVisible(config->value(QMC2_FRONTEND_PREFIX + "GUI/Statusbar", TRUE).toBool());
  qmc2MainWindow->toolbar->setVisible(config->value(QMC2_FRONTEND_PREFIX + "GUI/Toolbar", TRUE).toBool());
#endif

  QFont f;
  f.fromString(config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
  qApp->setFont(f);
  QFontMetrics fm(f);
  foreach (QWidget *widget, QApplication::allWidgets())
    widget->setFont(f);
  QFont logFont = f;
  if ( !config->value(QMC2_FRONTEND_PREFIX + "GUI/LogFont").toString().isEmpty() )
    logFont.fromString(config->value(QMC2_FRONTEND_PREFIX + "GUI/LogFont").toString());
  qmc2MainWindow->textBrowserFrontendLog->setFont(logFont);
  qmc2MainWindow->textBrowserEmulatorLog->setFont(logFont);
  lineEditLogFont->setFont(logFont);

  QSize iconSize(fm.height() - 2, fm.height() - 2);
  QSize iconSizeLarge = iconSize + QSize(4, 4);
  qmc2MainWindow->treeWidgetGamelist->setIconSize(iconSize);
  qmc2MainWindow->treeWidgetHierarchy->setIconSize(iconSize);
  qmc2MainWindow->treeWidgetEmulators->setIconSize(iconSize);
  pushButtonApply->setIconSize(iconSize);
  pushButtonRestore->setIconSize(iconSize);
  pushButtonDefault->setIconSize(iconSize);
  toolButtonBrowseStyleSheet->setIconSize(iconSize);
  toolButtonBrowseFont->setIconSize(iconSize);
  toolButtonBrowseLogFont->setIconSize(iconSize);
  toolButtonBrowseTemporaryFile->setIconSize(iconSize);
  toolButtonBrowseFrontendLogFile->setIconSize(iconSize);
  toolButtonBrowseFavoritesFile->setIconSize(iconSize);
  toolButtonBrowseHistoryFile->setIconSize(iconSize);
  toolButtonBrowseGamelistCacheFile->setIconSize(iconSize);
  toolButtonBrowseROMStateCacheFile->setIconSize(iconSize);
  toolButtonBrowseDataDirectory->setIconSize(iconSize);
  toolButtonBrowseGameInfoDB->setIconSize(iconSize);
#if defined(QMC2_EMUTYPE_MAME)
  qmc2MainWindow->treeWidgetCategoryView->setIconSize(iconSize);
  qmc2MainWindow->treeWidgetVersionView->setIconSize(iconSize);
  toolButtonBrowseEmuInfoDB->setIconSize(iconSize);
  toolButtonBrowseMAWSCacheDirectory->setIconSize(iconSize);
  checkBoxUseCatverIni->setIconSize(iconSize);
  toolButtonBrowseCatverIniFile->setIconSize(iconSize);
#endif
  toolButtonBrowsePreviewDirectory->setIconSize(iconSize);
  toolButtonBrowsePreviewFile->setIconSize(iconSize);
  toolButtonBrowseFlyerDirectory->setIconSize(iconSize);
  toolButtonBrowseFlyerFile->setIconSize(iconSize);
  toolButtonBrowseIconDirectory->setIconSize(iconSize);
  toolButtonBrowseIconFile->setIconSize(iconSize);
  toolButtonBrowseCabinetDirectory->setIconSize(iconSize);
  toolButtonBrowseCabinetFile->setIconSize(iconSize);
  toolButtonBrowseControllerDirectory->setIconSize(iconSize);
  toolButtonBrowseControllerFile->setIconSize(iconSize);
  toolButtonBrowseMarqueeDirectory->setIconSize(iconSize);
  toolButtonBrowseMarqueeFile->setIconSize(iconSize);
  toolButtonBrowseTitleDirectory->setIconSize(iconSize);
  toolButtonBrowseTitleFile->setIconSize(iconSize);
  toolButtonBrowsePCBDirectory->setIconSize(iconSize);
  toolButtonBrowsePCBFile->setIconSize(iconSize);
  toolButtonShowC->setIconSize(iconSize);
  toolButtonShowM->setIconSize(iconSize);
  toolButtonShowI->setIconSize(iconSize);
  toolButtonShowN->setIconSize(iconSize);
  toolButtonShowU->setIconSize(iconSize);
  comboBoxSortOrder->setIconSize(iconSize);
  toolButtonBrowseExecutableFile->setIconSize(iconSize);
  toolButtonBrowseWorkingDirectory->setIconSize(iconSize);
  toolButtonBrowseEmulatorLogFile->setIconSize(iconSize);
  toolButtonBrowseOptionsTemplateFile->setIconSize(iconSize);
  toolButtonBrowseListXMLCache->setIconSize(iconSize);
  toolButtonBrowseZipTool->setIconSize(iconSize);
  toolButtonBrowseFileRemovalTool->setIconSize(iconSize);
  toolButtonBrowseRomTool->setIconSize(iconSize);
  pushButtonRedefineKeySequence->setIconSize(iconSize);
  pushButtonResetShortcut->setIconSize(iconSize);
  toolButtonAddEmulator->setIconSize(iconSize);
  toolButtonSaveEmulator->setIconSize(iconSize);
  toolButtonRemoveEmulator->setIconSize(iconSize);
  toolButtonBrowseAdditionalEmulatorExecutable->setIconSize(iconSize);
  toolButtonBrowseAdditionalEmulatorWorkingDirectory->setIconSize(iconSize);
  checkBoxProcessEmuInfoDB->setIconSize(iconSize);
  checkBoxCompressEmuInfoDB->setIconSize(iconSize);
  checkBoxProcessGameInfoDB->setIconSize(iconSize);
  checkBoxCompressGameInfoDB->setIconSize(iconSize);
  QPixmap exitPixmap = QPixmap(QString::fromUtf8(":/data/img/exit.png")).scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  QPixmap reloadPixmap = QPixmap(QString::fromUtf8(":/data/img/reload.png")).scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  labelLanguagePic->setPixmap(exitPixmap);
  labelLegend1Pic->setPixmap(exitPixmap);
  labelLegend2Pic->setPixmap(reloadPixmap);
  labelGameInfoDBPic->setPixmap(reloadPixmap);
  labelEmuInfoDBPic->setPixmap(reloadPixmap);
  labelLegend3Pic->setPixmap(reloadPixmap);
  labelExecutableFilePic->setPixmap(reloadPixmap);
  labelLegend4Pic->setPixmap(reloadPixmap);
  
  tableWidgetRegisteredEmulators->resizeRowsToContents();

#if QMC2_JOYSTICK == 1
  pushButtonRescanJoysticks->setIconSize(iconSize);
  pushButtonRemapJoystickFunction->setIconSize(iconSize);
  pushButtonRemoveJoystickMapping->setIconSize(iconSize);
#endif
  if ( qmc2ROMAlyzer ) {
    qmc2ROMAlyzer->textBrowserLog->setFont(logFont);
    QTimer::singleShot(0, qmc2ROMAlyzer, SLOT(adjustIconSizes()));
  }
  if ( qmc2ImageChecker ) {
    QTabBar *tabBar = qmc2ImageChecker->tabWidgetImageChecker->findChild<QTabBar *>();
    if ( tabBar ) tabBar->setIconSize(iconSize);
  }
  checkBoxProcessGameInfoDB->setIconSize(iconSize);
  checkBoxCompressGameInfoDB->setIconSize(iconSize);
#if defined(QMC2_EMUTYPE_MAME)
  checkBoxProcessEmuInfoDB->setIconSize(iconSize);
  checkBoxCompressEmuInfoDB->setIconSize(iconSize);
  if ( qmc2MAWSLookup ) {
    qmc2MAWSLookup->toolButtonBack->setIconSize(iconSize);
    qmc2MAWSLookup->toolButtonForward->setIconSize(iconSize);
    qmc2MAWSLookup->toolButtonReload->setIconSize(iconSize);
    qmc2MAWSLookup->toolButtonStop->setIconSize(iconSize);
    qmc2MAWSLookup->toolButtonHome->setIconSize(iconSize);
    qmc2MAWSLookup->toolButtonLoad->setIconSize(iconSize);
    if ( qmc2MainWindow->toolButtonMAWSQuickLinks )
      qmc2MainWindow->toolButtonMAWSQuickLinks->setIconSize(iconSize);
  }
  if ( qmc2MawsQuickDownloadSetup )
    QTimer::singleShot(0, qmc2MawsQuickDownloadSetup, SLOT(adjustIconSizes()));
#endif
  if ( qmc2DocBrowser ) {
    qmc2DocBrowser->browser->toolButtonBack->setIconSize(iconSize);
    qmc2DocBrowser->browser->toolButtonForward->setIconSize(iconSize);
    qmc2DocBrowser->browser->toolButtonReload->setIconSize(iconSize);
    qmc2DocBrowser->browser->toolButtonStop->setIconSize(iconSize);
    qmc2DocBrowser->browser->toolButtonHome->setIconSize(iconSize);
    qmc2DocBrowser->browser->toolButtonLoad->setIconSize(iconSize);
  }
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
#if defined(QMC2_YOUTUBE_ENABLED)
  if ( qmc2YouTubeWidget )
    QTimer::singleShot(0, qmc2YouTubeWidget, SLOT(adjustIconSizes()));
#endif
#endif
  if ( qmc2ROMStatusExporter )
    QTimer::singleShot(0, qmc2ROMStatusExporter, SLOT(adjustIconSizes()));
#if defined(QMC2_EMUTYPE_MESS)
  toolButtonBrowseSoftwareListCache->setIconSize(iconSize);
  if ( qmc2MESSDeviceConfigurator ) {
    qmc2MESSDeviceConfigurator->pushButtonConfiguration->setIconSize(iconSize);
    qmc2MESSDeviceConfigurator->pushButtonNewConfiguration->setIconSize(iconSize);
    qmc2MESSDeviceConfigurator->pushButtonCloneConfiguration->setIconSize(iconSize);
    qmc2MESSDeviceConfigurator->pushButtonSaveConfiguration->setIconSize(iconSize);
    qmc2MESSDeviceConfigurator->pushButtonRemoveConfiguration->setIconSize(iconSize);
  }
  if ( qmc2MESSSoftwareList ) {
    qmc2MESSSoftwareList->toolButtonAddToFavorites->setIconSize(iconSize);
    qmc2MESSSoftwareList->toolButtonRemoveFromFavorites->setIconSize(iconSize);
    qmc2MESSSoftwareList->toolButtonPlay->setIconSize(iconSize);
#if defined(Q_WS_X11)
    qmc2MESSSoftwareList->toolButtonPlayEmbedded->setIconSize(iconSize);
#endif
    qmc2MESSSoftwareList->toolButtonReload->setIconSize(iconSize);
    qmc2MESSSoftwareList->toolBoxSoftwareList->setItemIcon(QMC2_SWLIST_KNOWN_SW_PAGE, QIcon(QPixmap(QString::fromUtf8(":/data/img/flat.png")).scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    qmc2MESSSoftwareList->toolBoxSoftwareList->setItemIcon(QMC2_SWLIST_FAVORITES_PAGE, QIcon(QPixmap(QString::fromUtf8(":/data/img/favorites.png")).scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
    qmc2MESSSoftwareList->toolBoxSoftwareList->setItemIcon(QMC2_SWLIST_SEARCH_PAGE, QIcon(QPixmap(QString::fromUtf8(":/data/img/hint.png")).scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
  }
#endif
  qmc2MainWindow->pushButtonClearFinishedDownloads->setIconSize(iconSize);
  qmc2MainWindow->pushButtonReloadSelectedDownloads->setIconSize(iconSize);
  qmc2MainWindow->pushButtonStopSelectedDownloads->setIconSize(iconSize);
  qmc2MainWindow->treeWidgetDownloads->setIconSize(iconSize);
  qmc2MainWindow->pushButtonSelectRomFilter->setIconSize(iconSize);
  qmc2MainWindow->comboBoxViewSelect->setIconSize(iconSize);

  QTabBar *tabBar = qmc2MainWindow->tabWidgetGamelist->findChild<QTabBar *>();
  if ( tabBar ) tabBar->setIconSize(iconSize);
  tabBar = qmc2MainWindow->tabWidgetGameDetail->findChild<QTabBar *>();
  if ( tabBar ) tabBar->setIconSize(iconSize);
  tabBar = qmc2MainWindow->tabWidgetLogsAndEmulators->findChild<QTabBar *>();
  if ( tabBar ) tabBar->setIconSize(iconSize);

  qmc2MainWindow->toolbar->setIconSize(iconSizeLarge);

#if defined(Q_WS_X11)
  int i;
  for (i = 0; i < qmc2MainWindow->tabWidgetEmbeddedEmulators->count(); i++) {
    Embedder *embedder = (Embedder *)qmc2MainWindow->tabWidgetEmbeddedEmulators->widget(i);
    embedder->adjustIconSizes();
    if ( embedder->embedderOptions )
      embedder->embedderOptions->adjustIconSizes();
  }
  qmc2MainWindow->toolButtonEmbedderMaximizeToggle->setIconSize(iconSizeLarge);
  qmc2MainWindow->toolButtonEmbedderAutoPause->setIconSize(iconSizeLarge);
#endif

  if ( qmc2DetailSetup )
    if ( qmc2DetailSetup->isVisible() )
      QTimer::singleShot(0, qmc2DetailSetup, SLOT(adjustIconSizes()));
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
  static QString oldStyleName = "";
  static QString oldStyleSheet = "";
  QString s;
  int i;
  bool needRestart = FALSE,
       needResort = FALSE,
       needRecreateTemplateMap = FALSE,
       needFilter = FALSE,
       needReopenPreviewFile = FALSE,
       needReopenFlyerFile = FALSE,
       needReopenIconFile = FALSE,
       needReopenCabinetFile = FALSE,
       needReopenControllerFile = FALSE,
       needReopenMarqueeFile = FALSE,
       needReopenTitleFile = FALSE,
       needReopenPCBFile = FALSE,
       needReload = FALSE,
       needManualReload = FALSE;

  // General
#ifdef BETA
  config->setValue("Version", QString::number(MAJOR) + "." + QString::number(MINOR) + ".b" + QString::number(BETA));
#else 
  config->setValue("Version", QString::number(MAJOR) + "." + QString::number(MINOR));
#endif
#if QMC2_SVN_REV > 0
  config->setValue("SVN_Revision", QMC2_SVN_REV);
#else
  config->remove("SVN_Revision");
#endif

  // Frontend

  // GUI
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/Toolbar", checkBoxToolbar->isChecked());
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout", checkBoxSaveLayout->isChecked());
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/RestoreLayout", checkBoxRestoreLayout->isChecked());
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/SaveGameSelection", checkBoxSaveGameSelection->isChecked());
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/RestoreGameSelection", checkBoxRestoreGameSelection->isChecked());
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/Statusbar", checkBoxStatusbar->isChecked());
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/StandardColorPalette", checkBoxStandardColorPalette->isChecked());
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts", checkBoxProgressTexts->isChecked());
  bool b = checkBoxProcessGameInfoDB->isChecked();
  needManualReload |= (config->value(QMC2_FRONTEND_PREFIX + "GUI/ProcessGameInfoDB").toBool() != b);
  bool invalidateGameInfoDB = (config->value(QMC2_FRONTEND_PREFIX + "GUI/ProcessGameInfoDB").toBool() != b);
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/ProcessGameInfoDB", checkBoxProcessGameInfoDB->isChecked());
  b = checkBoxCompressGameInfoDB->isChecked();
  needManualReload |= (config->value(QMC2_FRONTEND_PREFIX + "GUI/CompressGameInfoDB").toBool() != b);
  invalidateGameInfoDB |= (config->value(QMC2_FRONTEND_PREFIX + "GUI/CompressGameInfoDB").toBool() != b);
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/CompressGameInfoDB", checkBoxCompressGameInfoDB->isChecked());
#if defined(QMC2_EMUTYPE_MAME)
  b = checkBoxProcessEmuInfoDB->isChecked();
  needManualReload |= (config->value(QMC2_FRONTEND_PREFIX + "GUI/ProcessEmuInfoDB").toBool() != b);
  bool invalidateEmuInfoDB = (config->value(QMC2_FRONTEND_PREFIX + "GUI/ProcessEmuInfoDB").toBool() != b);
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/ProcessEmuInfoDB", checkBoxProcessEmuInfoDB->isChecked());
  b = checkBoxCompressEmuInfoDB->isChecked();
  needManualReload |= (config->value(QMC2_FRONTEND_PREFIX + "GUI/CompressEmuInfoDB").toBool() != b);
  invalidateEmuInfoDB |= (config->value(QMC2_FRONTEND_PREFIX + "GUI/CompressEmuInfoDB").toBool() != b);
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/CompressEmuInfoDB", checkBoxCompressEmuInfoDB->isChecked());
#endif
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

  s = lineEditStyleSheet->text();
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/StyleSheet", s);
  bool newStyleSheet = FALSE;
  if ( s != oldStyleSheet ) {
    oldStyleSheet = s;
    newStyleSheet = TRUE;
  }
  if ( !qmc2EarlyStartup ) {
    // style sheet
    if ( newStyleSheet )
      qmc2MainWindow->setupStyleSheet(s);
    // style
    s = comboBoxStyle->currentText();
    if ( s == QObject::tr("Default") ) s = "Default";
    config->setValue(QMC2_FRONTEND_PREFIX + "GUI/Style", s);
    if ( s != oldStyleName || newStyleSheet ) {
      qmc2MainWindow->setupStyle(s);
      oldStyleName = s;
      qApp->processEvents();
    }
  }

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
  QPixmapCache::setCacheLimit(oldCacheSize * 1024);
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/KillEmulatorsOnExit", checkBoxKillEmulatorsOnExit->isChecked());
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/ShowMenuBar", checkBoxShowMenuBar->isChecked());
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/CheckSingleInstance", checkBoxCheckSingleInstance->isChecked());
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
        qmc2MainWindow->labelGameStatus->setVisible(TRUE);
      } else {
        qmc2MainWindow->labelGameStatus->setVisible(FALSE);
      }
    } else {
      qmc2MainWindow->labelGameStatus->setVisible(TRUE);
    }
  } else
    qmc2MainWindow->labelGameStatus->setVisible(FALSE);
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/FrontendLogSize", spinBoxFrontendLogSize->value());
  qmc2MainWindow->textBrowserFrontendLog->setMaximumBlockCount(spinBoxFrontendLogSize->value());
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/EmulatorLogSize", spinBoxEmulatorLogSize->value());
  qmc2MainWindow->textBrowserEmulatorLog->setMaximumBlockCount(spinBoxEmulatorLogSize->value());
#if defined(QMC2_VARIANT_LAUNCHER)
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/MinimizeOnVariantLaunch", checkBoxMinimizeOnVariantLaunch->isChecked());
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/ExitOnVariantLaunch", checkBoxExitOnVariantLaunch->isChecked());
#endif
#if defined(QMC2_SHOWMEMINFO)
  config->setValue(QMC2_FRONTEND_PREFIX + "GUI/MemoryIndicator", checkBoxMemoryIndicator->isChecked());
  if ( checkBoxMemoryIndicator->isChecked() ) {
    qmc2MainWindow->progressBarMemory->setRange(0, 100);
    qmc2MainWindow->on_memoryUpdateTimer_timeout();
    qmc2MainWindow->progressBarMemory->setVisible(TRUE);
    connect(&qmc2MainWindow->memoryUpdateTimer, SIGNAL(timeout()), qmc2MainWindow, SLOT(on_memoryUpdateTimer_timeout()));
    qmc2MainWindow->memoryUpdateTimer.start(QMC2_MEMORY_UPDATE_TIME);
  } else {
    qmc2MainWindow->memoryUpdateTimer.stop();
    disconnect(&qmc2MainWindow->memoryUpdateTimer);
    qmc2MainWindow->progressBarMemory->setVisible(FALSE);
  }
#endif

  // Files and directories
  config->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", lineEditTemporaryFile->text());
  config->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/LogFile", lineEditFrontendLogFile->text());
  config->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/DataDirectory", lineEditDataDirectory->text());

#if defined(QMC2_EMUTYPE_MAME)
  needReopenPreviewFile = (qmc2UsePreviewFile != (stackedWidgetPreview->currentIndex() == 1));
  needReopenPreviewFile |= (config->value("MAME/FilesAndDirectories/PreviewFile").toString() != lineEditPreviewFile->text());
  qmc2UsePreviewFile = (stackedWidgetPreview->currentIndex() == 1);
  config->setValue("MAME/FilesAndDirectories/UsePreviewFile", qmc2UsePreviewFile);
  config->setValue("MAME/FilesAndDirectories/PreviewDirectory", lineEditPreviewDirectory->text());
  config->setValue("MAME/FilesAndDirectories/PreviewFile", lineEditPreviewFile->text());
  needReopenFlyerFile = (qmc2UseFlyerFile != (stackedWidgetFlyer->currentIndex() == 1));
  needReopenFlyerFile |= (config->value("MAME/FilesAndDirectories/FlyerFile").toString() != lineEditFlyerFile->text());
  qmc2UseFlyerFile = (stackedWidgetFlyer->currentIndex() == 1);
  config->setValue("MAME/FilesAndDirectories/UseFlyerFile", qmc2UseFlyerFile);
  config->setValue("MAME/FilesAndDirectories/FlyerDirectory", lineEditFlyerDirectory->text());
  config->setValue("MAME/FilesAndDirectories/FlyerFile", lineEditFlyerFile->text());
  needReopenIconFile = (qmc2UseIconFile != (stackedWidgetIcon->currentIndex() == 1));
  needReopenIconFile |= (config->value("MAME/FilesAndDirectories/IconFile").toString() != lineEditIconFile->text());
  qmc2UseIconFile = (stackedWidgetIcon->currentIndex() == 1);
  config->setValue("MAME/FilesAndDirectories/UseIconFile", qmc2UseIconFile);
  config->setValue("MAME/FilesAndDirectories/IconDirectory", lineEditIconDirectory->text());
  config->setValue("MAME/FilesAndDirectories/IconFile", lineEditIconFile->text());
  needReopenCabinetFile = (qmc2UseCabinetFile != (stackedWidgetCabinet->currentIndex() == 1));
  needReopenCabinetFile |= (config->value("MAME/FilesAndDirectories/CabinetFile").toString() != lineEditCabinetFile->text());
  qmc2UseCabinetFile = (stackedWidgetCabinet->currentIndex() == 1);
  config->setValue("MAME/FilesAndDirectories/UseCabinetFile", qmc2UseCabinetFile);
  config->setValue("MAME/FilesAndDirectories/CabinetDirectory", lineEditCabinetDirectory->text());
  config->setValue("MAME/FilesAndDirectories/CabinetFile", lineEditCabinetFile->text());
  needReopenControllerFile = (qmc2UseControllerFile != (stackedWidgetController->currentIndex() == 1));
  needReopenControllerFile |= (config->value("MAME/FilesAndDirectories/ControllerFile").toString() != lineEditControllerFile->text());
  qmc2UseControllerFile = (stackedWidgetController->currentIndex() == 1);
  config->setValue("MAME/FilesAndDirectories/UseControllerFile", qmc2UseControllerFile);
  config->setValue("MAME/FilesAndDirectories/ControllerDirectory", lineEditControllerDirectory->text());
  config->setValue("MAME/FilesAndDirectories/ControllerFile", lineEditControllerFile->text());
  needReopenMarqueeFile = (qmc2UseMarqueeFile != (stackedWidgetMarquee->currentIndex() == 1));
  needReopenMarqueeFile |= (config->value("MAME/FilesAndDirectories/MarqueeFile").toString() != lineEditMarqueeFile->text());
  qmc2UseMarqueeFile = (stackedWidgetMarquee->currentIndex() == 1);
  config->setValue("MAME/FilesAndDirectories/UseMarqueeFile", qmc2UseMarqueeFile);
  config->setValue("MAME/FilesAndDirectories/MarqueeDirectory", lineEditMarqueeDirectory->text());
  config->setValue("MAME/FilesAndDirectories/MarqueeFile", lineEditMarqueeFile->text());
  needReopenTitleFile = (qmc2UseTitleFile != (stackedWidgetTitle->currentIndex() == 1));
  needReopenTitleFile |= (config->value("MAME/FilesAndDirectories/TitleFile").toString() != lineEditTitleFile->text());
  qmc2UseTitleFile = (stackedWidgetTitle->currentIndex() == 1);
  config->setValue("MAME/FilesAndDirectories/UseTitleFile", qmc2UseTitleFile);
  config->setValue("MAME/FilesAndDirectories/TitleDirectory", lineEditTitleDirectory->text());
  config->setValue("MAME/FilesAndDirectories/TitleFile", lineEditTitleFile->text());
  needReopenPCBFile = (qmc2UsePCBFile != (stackedWidgetPCB->currentIndex() == 1));
  needReopenPCBFile |= (config->value("MAME/FilesAndDirectories/PCBFile").toString() != lineEditPCBFile->text());
  qmc2UsePCBFile = (stackedWidgetTitle->currentIndex() == 1);
  config->setValue("MAME/FilesAndDirectories/UsePCBFile", qmc2UsePCBFile);
  config->setValue("MAME/FilesAndDirectories/PCBDirectory", lineEditPCBDirectory->text());
  config->setValue("MAME/FilesAndDirectories/PCBFile", lineEditPCBFile->text());
  s = lineEditGameInfoDB->text();
  needManualReload |= (config->value("MAME/FilesAndDirectories/GameInfoDB").toString() != s);
  invalidateGameInfoDB |= (config->value("MAME/FilesAndDirectories/GameInfoDB").toString() != s);
  config->setValue("MAME/FilesAndDirectories/GameInfoDB", lineEditGameInfoDB->text());
  s = lineEditEmuInfoDB->text();
  needManualReload |= (config->value("MAME/FilesAndDirectories/EmuInfoDB").toString() != s);
  invalidateEmuInfoDB |= (config->value("MAME/FilesAndDirectories/EmuInfoDB").toString() != s);
  config->setValue("MAME/FilesAndDirectories/EmuInfoDB", lineEditEmuInfoDB->text());
  config->setValue("MAME/FilesAndDirectories/CatverIni", lineEditCatverIniFile->text());
  bool catverUsed = checkBoxUseCatverIni->isChecked();
  needReload |= (config->value(QMC2_FRONTEND_PREFIX + "Gamelist/UseCatverIni", FALSE).toBool() != catverUsed );
  config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/UseCatverIni", catverUsed);
  if ( catverUsed ) {
    qmc2MainWindow->treeWidgetGamelist->showColumn(QMC2_GAMELIST_COLUMN_CATEGORY);
    qmc2MainWindow->treeWidgetGamelist->showColumn(QMC2_GAMELIST_COLUMN_VERSION);
    qmc2MainWindow->treeWidgetHierarchy->showColumn(QMC2_GAMELIST_COLUMN_CATEGORY);
    qmc2MainWindow->treeWidgetHierarchy->showColumn(QMC2_GAMELIST_COLUMN_VERSION);
    qmc2MainWindow->actionViewByCategory->setVisible(TRUE);
    qmc2MainWindow->actionViewByCategory->setEnabled(TRUE);
    qmc2MainWindow->actionViewByVersion->setVisible(TRUE);
    qmc2MainWindow->actionViewByVersion->setEnabled(TRUE);
    if ( comboBoxSortCriteria->count() - 1 < QMC2_SORTCRITERIA_CATEGORY ) {
      comboBoxSortCriteria->insertItem(QMC2_SORTCRITERIA_CATEGORY, tr("Category"));
      comboBoxSortCriteria->insertItem(QMC2_SORTCRITERIA_VERSION, tr("Version"));
      qmc2MainWindow->treeWidgetGamelist->resizeColumnToContents(qmc2MainWindow->treeWidgetGamelist->header()->logicalIndex(QMC2_GAMELIST_COLUMN_VERSION));
      qmc2MainWindow->treeWidgetHierarchy->resizeColumnToContents(qmc2MainWindow->treeWidgetHierarchy->header()->logicalIndex(QMC2_GAMELIST_COLUMN_VERSION));
    }
    if ( qmc2MainWindow->comboBoxViewSelect->count() - 1 < QMC2_VIEWCATEGORY_INDEX ) {
      qmc2MainWindow->comboBoxViewSelect->insertItem(QMC2_VIEWCATEGORY_INDEX, tr("View games by category (not filtered)"));
      qmc2MainWindow->comboBoxViewSelect->setItemIcon(QMC2_VIEWCATEGORY_INDEX, QIcon(QString::fromUtf8(":/data/img/category.png")));
      qmc2MainWindow->comboBoxViewSelect->insertItem(QMC2_VIEWVERSION_INDEX, tr("View games by emulator version (not filtered)"));
      qmc2MainWindow->comboBoxViewSelect->setItemIcon(QMC2_VIEWVERSION_INDEX, QIcon(QString::fromUtf8(":/data/img/version.png")));
    }
  } else {
    qmc2MainWindow->treeWidgetGamelist->hideColumn(QMC2_GAMELIST_COLUMN_VERSION);
    qmc2MainWindow->treeWidgetGamelist->hideColumn(QMC2_GAMELIST_COLUMN_CATEGORY);
    qmc2MainWindow->treeWidgetHierarchy->hideColumn(QMC2_GAMELIST_COLUMN_VERSION);
    qmc2MainWindow->treeWidgetHierarchy->hideColumn(QMC2_GAMELIST_COLUMN_CATEGORY);
    qmc2MainWindow->actionViewByCategory->setVisible(FALSE);
    qmc2MainWindow->actionViewByCategory->setEnabled(FALSE);
    qmc2MainWindow->actionViewByVersion->setVisible(FALSE);
    qmc2MainWindow->actionViewByVersion->setEnabled(FALSE);
    if ( comboBoxSortCriteria->count() > QMC2_SORTCRITERIA_VERSION ) {
      comboBoxSortCriteria->removeItem(QMC2_SORTCRITERIA_VERSION);
      comboBoxSortCriteria->removeItem(QMC2_SORTCRITERIA_CATEGORY);
      qmc2MainWindow->treeWidgetGamelist->resizeColumnToContents(qmc2MainWindow->treeWidgetGamelist->header()->logicalIndex(QMC2_GAMELIST_COLUMN_PLAYERS));
      qmc2MainWindow->treeWidgetHierarchy->resizeColumnToContents(qmc2MainWindow->treeWidgetHierarchy->header()->logicalIndex(QMC2_GAMELIST_COLUMN_PLAYERS));
    }
    if ( qmc2MainWindow->comboBoxViewSelect->count() > QMC2_VIEWVERSION_INDEX ) {
      qmc2MainWindow->comboBoxViewSelect->removeItem(QMC2_VIEWVERSION_INDEX);
      qmc2MainWindow->comboBoxViewSelect->removeItem(QMC2_VIEWCATEGORY_INDEX);
    }
  }
#elif defined(QMC2_EMUTYPE_MESS)
  needReopenPreviewFile = (qmc2UsePreviewFile != (stackedWidgetPreview->currentIndex() == 1));
  needReopenPreviewFile |= (config->value("MESS/FilesAndDirectories/PreviewFile").toString() != lineEditPreviewFile->text());
  qmc2UsePreviewFile = (stackedWidgetPreview->currentIndex() == 1);
  config->setValue("MESS/FilesAndDirectories/UsePreviewFile", qmc2UsePreviewFile);
  config->setValue("MESS/FilesAndDirectories/PreviewDirectory", lineEditPreviewDirectory->text());
  config->setValue("MESS/FilesAndDirectories/PreviewFile", lineEditPreviewFile->text());
  needReopenFlyerFile = (qmc2UseFlyerFile != (stackedWidgetFlyer->currentIndex() == 1));
  needReopenFlyerFile |= (config->value("MESS/FilesAndDirectories/FlyerFile").toString() != lineEditFlyerFile->text());
  qmc2UseFlyerFile = (stackedWidgetFlyer->currentIndex() == 1);
  config->setValue("MESS/FilesAndDirectories/UseFlyerFile", qmc2UseFlyerFile);
  config->setValue("MESS/FilesAndDirectories/FlyerDirectory", lineEditFlyerDirectory->text());
  config->setValue("MESS/FilesAndDirectories/FlyerFile", lineEditFlyerFile->text());
  needReopenIconFile = (qmc2UseIconFile != (stackedWidgetIcon->currentIndex() == 1));
  needReopenIconFile |= (config->value("MESS/FilesAndDirectories/IconFile").toString() != lineEditIconFile->text());
  qmc2UseIconFile = (stackedWidgetIcon->currentIndex() == 1);
  config->setValue("MESS/FilesAndDirectories/UseIconFile", qmc2UseIconFile);
  config->setValue("MESS/FilesAndDirectories/IconDirectory", lineEditIconDirectory->text());
  config->setValue("MESS/FilesAndDirectories/IconFile", lineEditIconFile->text());
  needReopenCabinetFile = (qmc2UseCabinetFile != (stackedWidgetCabinet->currentIndex() == 1));
  needReopenCabinetFile |= (config->value("MESS/FilesAndDirectories/CabinetFile").toString() != lineEditCabinetFile->text());
  qmc2UseCabinetFile = (stackedWidgetCabinet->currentIndex() == 1);
  config->setValue("MESS/FilesAndDirectories/UseCabinetFile", qmc2UseCabinetFile);
  config->setValue("MESS/FilesAndDirectories/CabinetDirectory", lineEditCabinetDirectory->text());
  config->setValue("MESS/FilesAndDirectories/CabinetFile", lineEditCabinetFile->text());
  needReopenControllerFile = (qmc2UseControllerFile != (stackedWidgetController->currentIndex() == 1));
  needReopenControllerFile |= (config->value("MESS/FilesAndDirectories/ControllerFile").toString() != lineEditControllerFile->text());
  qmc2UseControllerFile = (stackedWidgetController->currentIndex() == 1);
  config->setValue("MESS/FilesAndDirectories/UseControllerFile", qmc2UseControllerFile);
  config->setValue("MESS/FilesAndDirectories/ControllerDirectory", lineEditControllerDirectory->text());
  config->setValue("MESS/FilesAndDirectories/ControllerFile", lineEditControllerFile->text());
  needReopenMarqueeFile = (qmc2UseMarqueeFile != (stackedWidgetMarquee->currentIndex() == 1));
  needReopenMarqueeFile |= (config->value("MESS/FilesAndDirectories/MarqueeFile").toString() != lineEditMarqueeFile->text());
  qmc2UseMarqueeFile = (stackedWidgetMarquee->currentIndex() == 1);
  config->setValue("MESS/FilesAndDirectories/UseMarqueeFile", qmc2UseMarqueeFile);
  config->setValue("MESS/FilesAndDirectories/MarqueeDirectory", lineEditMarqueeDirectory->text());
  config->setValue("MESS/FilesAndDirectories/MarqueeFile", lineEditMarqueeFile->text());
  needReopenTitleFile = (qmc2UseTitleFile != (stackedWidgetTitle->currentIndex() == 1));
  needReopenTitleFile |= (config->value("MESS/FilesAndDirectories/TitleFile").toString() != lineEditTitleFile->text());
  qmc2UseTitleFile = (stackedWidgetTitle->currentIndex() == 1);
  config->setValue("MESS/FilesAndDirectories/UseTitleFile", qmc2UseTitleFile);
  config->setValue("MESS/FilesAndDirectories/TitleDirectory", lineEditTitleDirectory->text());
  config->setValue("MESS/FilesAndDirectories/TitleFile", lineEditTitleFile->text());
  needReopenPCBFile = (qmc2UsePCBFile != (stackedWidgetPCB->currentIndex() == 1));
  needReopenPCBFile |= (config->value("MESS/FilesAndDirectories/PCBFile").toString() != lineEditPCBFile->text());
  qmc2UsePCBFile = (stackedWidgetTitle->currentIndex() == 1);
  config->setValue("MESS/FilesAndDirectories/UsePCBFile", qmc2UsePCBFile);
  config->setValue("MESS/FilesAndDirectories/PCBDirectory", lineEditPCBDirectory->text());
  config->setValue("MESS/FilesAndDirectories/PCBFile", lineEditPCBFile->text());
  s = lineEditGameInfoDB->text();
  needManualReload |= (config->value("MESS/FilesAndDirectories/GameInfoDB").toString() != s);
  invalidateGameInfoDB |= (config->value("MESS/FilesAndDirectories/GameInfoDB").toString() != s);
  config->setValue("MESS/FilesAndDirectories/GameInfoDB", lineEditGameInfoDB->text());
#endif

  // Gamelist
  config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/SortOnline", checkBoxSortOnline->isChecked());
  config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/AutoTriggerROMCheck", checkBoxAutoTriggerROMCheck->isChecked());
  config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/DoubleClickActivation", checkBoxDoubleClickActivation->isChecked());
  config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/HideWhileLoading", checkBoxHideWhileLoading->isChecked());
  config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/PlayOnSublistActivation", checkBoxPlayOnSublistActivation->isChecked());
  qmc2CursorPositioningMode = (QAbstractItemView::ScrollHint)comboBoxCursorPosition->currentIndex();
  config->setValue(QMC2_FRONTEND_PREFIX + "Gamelist/CursorPosition", qmc2CursorPositioningMode);
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
    // reconnect joystick callbacks to main widget if applicable
    joystick->disconnect(qmc2MainWindow);
    if ( config->value(QMC2_FRONTEND_PREFIX + "Joystick/EnableJoystickControl").toBool() ) {
      if ( !joystick->open(config->value(QMC2_FRONTEND_PREFIX + "Joystick/Index").toInt()) ) {
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: can't initialize joystick"));
      } else {
        // (re)connect joystick callbacks to main widget
        connect(joystick, SIGNAL(axisValueChanged(int, int)), qmc2MainWindow, SLOT(on_joystickAxisValueChanged(int, int)));
        connect(joystick, SIGNAL(buttonValueChanged(int, bool)), qmc2MainWindow, SLOT(on_joystickButtonValueChanged(int, bool)));
        connect(joystick, SIGNAL(hatValueChanged(int, int)), qmc2MainWindow, SLOT(on_joystickHatValueChanged(int, int)));
        connect(joystick, SIGNAL(trackballValueChanged(int, int, int)), qmc2MainWindow, SLOT(on_joystickTrackballValueChanged(int, int, int)));
        qmc2MainWindow->joyIndex = config->value(QMC2_FRONTEND_PREFIX + "Joystick/Index").toInt();
      }
    }
  }
#endif

  // Tools / Proxy
  config->setValue(QMC2_FRONTEND_PREFIX + "Tools/ZipTool", lineEditZipTool->text());
  config->setValue(QMC2_FRONTEND_PREFIX + "Tools/ZipToolRemovalArguments", lineEditZipToolRemovalArguments->text());
  config->setValue(QMC2_FRONTEND_PREFIX + "Tools/FileRemovalTool", lineEditFileRemovalTool->text());
  config->setValue(QMC2_FRONTEND_PREFIX + "Tools/FileRemovalToolArguments", lineEditFileRemovalToolArguments->text());
  config->setValue(QMC2_FRONTEND_PREFIX + "Tools/RomTool", lineEditRomTool->text());
  config->setValue(QMC2_FRONTEND_PREFIX + "Tools/RomToolArguments", lineEditRomToolArguments->text());
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
  } else {
      QNetworkProxy::setApplicationProxy(QNetworkProxy(QNetworkProxy::NoProxy));
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
#if defined(QMC2_EMUTYPE_MAME)
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
#if defined(QMC2_EMUTYPE_MAME)
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
             qmc2EmulatorOptions->load(TRUE);
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

  // Files and directories
#if defined(QMC2_EMUTYPE_MAME)
  needReload |= config->value("MAME/FilesAndDirectories/ExecutableFile").toString() != lineEditExecutableFile->text();
  config->setValue("MAME/FilesAndDirectories/ExecutableFile", lineEditExecutableFile->text());
  config->setValue("MAME/FilesAndDirectories/WorkingDirectory", lineEditWorkingDirectory->text());
  config->setValue("MAME/FilesAndDirectories/LogFile", lineEditEmulatorLogFile->text());
  config->setValue("MAME/FilesAndDirectories/ListXMLCache", lineEditListXMLCache->text());
  config->setValue("MAME/FilesAndDirectories/GamelistCacheFile", lineEditGamelistCacheFile->text());
  config->setValue("MAME/FilesAndDirectories/ROMStateCacheFile", lineEditROMStateCacheFile->text());
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
  config->setValue("MESS/FilesAndDirectories/LogFile", lineEditEmulatorLogFile->text());
  config->setValue("MESS/FilesAndDirectories/ListXMLCache", lineEditListXMLCache->text());
  config->setValue("MESS/FilesAndDirectories/GamelistCacheFile", lineEditGamelistCacheFile->text());
  config->setValue("MESS/FilesAndDirectories/ROMStateCacheFile", lineEditROMStateCacheFile->text());
  config->setValue("MESS/FilesAndDirectories/SoftwareListCache", lineEditSoftwareListCache->text());
  s = lineEditOptionsTemplateFile->text();
  needRecreateTemplateMap = needRecreateTemplateMap || (config->value("MESS/FilesAndDirectories/OptionsTemplateFile").toString() != s );
  config->setValue("MESS/FilesAndDirectories/OptionsTemplateFile", s);
  config->setValue("MESS/FilesAndDirectories/FavoritesFile", lineEditFavoritesFile->text());
  config->setValue("MESS/FilesAndDirectories/HistoryFile", lineEditHistoryFile->text());
#endif

  // Additional emulators
  tableWidgetRegisteredEmulators->setSortingEnabled(FALSE);
#if defined(QMC2_EMUTYPE_MAME)
  config->remove("MAME/RegisteredEmulators");
#elif defined(QMC2_EMUTYPE_MESS)
  config->remove("MESS/RegisteredEmulators");
#endif
  for (i = 0; i < tableWidgetRegisteredEmulators->rowCount(); i++) {
    if ( tableWidgetRegisteredEmulators->item(i, QMC2_ADDTLEMUS_COLUMN_NAME) ) {
      QString emuName, emuCommand, emuWorkDir, emuArgs;
      emuName = tableWidgetRegisteredEmulators->item(i, QMC2_ADDTLEMUS_COLUMN_NAME)->text();
      if ( tableWidgetRegisteredEmulators->item(i, QMC2_ADDTLEMUS_COLUMN_EXEC) )
        emuCommand = tableWidgetRegisteredEmulators->item(i, QMC2_ADDTLEMUS_COLUMN_EXEC)->text();
      if ( tableWidgetRegisteredEmulators->item(i, QMC2_ADDTLEMUS_COLUMN_WDIR) )
        emuWorkDir = tableWidgetRegisteredEmulators->item(i, QMC2_ADDTLEMUS_COLUMN_WDIR)->text();
      if ( tableWidgetRegisteredEmulators->item(i, QMC2_ADDTLEMUS_COLUMN_ARGS) )
        emuArgs = tableWidgetRegisteredEmulators->item(i, QMC2_ADDTLEMUS_COLUMN_ARGS)->text();
#if defined(QMC2_EMUTYPE_MAME)
      config->setValue(QString("MAME/RegisteredEmulators/%1/Executable").arg(emuName), emuCommand);
      config->setValue(QString("MAME/RegisteredEmulators/%1/WorkingDirectory").arg(emuName), emuWorkDir);
      config->setValue(QString("MAME/RegisteredEmulators/%1/Arguments").arg(emuName), emuArgs);
#elif defined(QMC2_EMUTYPE_MESS)
      config->setValue(QString("MESS/RegisteredEmulators/%1/Executable").arg(emuName), emuCommand);
      config->setValue(QString("MESS/RegisteredEmulators/%1/WorkingDirectory").arg(emuName), emuWorkDir);
      config->setValue(QString("MESS/RegisteredEmulators/%1/Arguments").arg(emuName), emuArgs);
#endif
    }
  }
  tableWidgetRegisteredEmulators->setSortingEnabled(TRUE);

  // sync settings (write settings to disk) and apply
  config->sync();
  applied = TRUE;
  if ( qmc2GuiReady )
    apply();

  if ( invalidateGameInfoDB ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("invalidating game info DB"));
    QMapIterator<QString, QByteArray *> it(qmc2GameInfoDB);
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
  }

#if defined(QMC2_EMUTYPE_MAME)
  if ( invalidateEmuInfoDB ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("invalidating emulator info DB"));
    QMapIterator<QString, QByteArray *> it(qmc2EmuInfoDB);
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
#endif

  if ( needManualReload )
#if defined(QMC2_EMUTYPE_MAME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("please reload game list for some changes to take effect"));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("please reload machine list for some changes to take effect"));
#endif

  if ( needRestart )
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("please restart QMC2 for some changes to take effect"));

  if ( needRecreateTemplateMap )
    qmc2MainWindow->on_actionRecreateTemplateMap_activated();

  if ( needResort && !needReload ) {
    bool doResort = TRUE;

    if ( qmc2VerifyActive ) {
#if defined(QMC2_EMUTYPE_MAME)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("re-sort of game list impossible at this time, please wait for ROM verification to finish and try again"));
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("re-sort of machine list impossible at this time, please wait for ROM verification to finish and try again"));
#endif
      qmc2SortCriteria = oldSortCriteria;
      qmc2SortOrder = oldSortOrder;
      doResort = FALSE;
    }

    if ( doResort ) {
      QString sortCriteria = "?";
      switch ( qmc2SortCriteria ) {
        case QMC2_SORT_BY_DESCRIPTION:
#if defined(QMC2_EMUTYPE_MAME)
          sortCriteria = QObject::tr("game description");
#elif defined(QMC2_EMUTYPE_MESS)
          sortCriteria = QObject::tr("machine description");
#endif
          break;
        case QMC2_SORT_BY_ROM_STATE:
          sortCriteria = QObject::tr("ROM state");
          break;
        case QMC2_SORT_BY_YEAR:
          sortCriteria = QObject::tr("year");
          break;
        case QMC2_SORT_BY_MANUFACTURER:
          sortCriteria = QObject::tr("manufacturer");
          break;
        case QMC2_SORT_BY_NAME:
#if defined(QMC2_EMUTYPE_MAME)
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
#if defined(QMC2_EMUTYPE_MAME)
        case QMC2_SORT_BY_CATEGORY:
          sortCriteria = QObject::tr("category");
          break;
        case QMC2_SORT_BY_VERSION:
          sortCriteria = QObject::tr("version");
          break;
#endif
      }
#if defined(QMC2_EMUTYPE_MAME)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("sorting game list by %1 in %2 order").arg(sortCriteria).arg(qmc2SortOrder == Qt::AscendingOrder ? tr("ascending") : tr("descending")));
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("sorting machine list by %1 in %2 order").arg(sortCriteria).arg(qmc2SortOrder == Qt::AscendingOrder ? tr("ascending") : tr("descending")));
#endif
      qApp->processEvents();
      QList<QTreeWidgetItem *> itemList = qmc2MainWindow->treeWidgetGamelist->findItems("*", Qt::MatchContains | Qt::MatchWildcard);
      for (i = 0; i < itemList.count(); i++) {
        if ( itemList[i]->childCount() > 1 ) {
          qmc2MainWindow->treeWidgetGamelist->collapseItem(itemList[i]);
          QList<QTreeWidgetItem *> childrenList = itemList[i]->takeChildren();
          int j;
          for (j = 0; j < childrenList.count(); j++)
            delete childrenList[j];
          QTreeWidgetItem *nameItem = new QTreeWidgetItem(itemList[i]);
          nameItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Waiting for data..."));
          nameItem->setText(QMC2_GAMELIST_COLUMN_ICON, qmc2GamelistNameMap[itemList[i]->text(QMC2_GAMELIST_COLUMN_GAME)]);
          qApp->processEvents();
        }
      }
      qApp->processEvents();
      qmc2MainWindow->treeWidgetGamelist->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
      qmc2MainWindow->treeWidgetHierarchy->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
#if defined(QMC2_EMUTYPE_MAME)
      qmc2MainWindow->treeWidgetCategoryView->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
      qmc2MainWindow->treeWidgetVersionView->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
#endif
      QTimer::singleShot(0, qmc2MainWindow, SLOT(scrollToCurrentItem()));
    }
  }

  switch ( qmc2SortCriteria ) {
    case QMC2_SORT_BY_DESCRIPTION:
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_GAME, qmc2SortOrder);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_GAME, qmc2SortOrder);
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicatorShown(TRUE);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(TRUE);
#if defined(QMC2_EMUTYPE_MAME)
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_GAME, qmc2SortOrder);
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_GAME, qmc2SortOrder);
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicatorShown(TRUE);
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicatorShown(TRUE);
#endif
      break;

    case QMC2_SORT_BY_YEAR:
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_YEAR, qmc2SortOrder);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_YEAR, qmc2SortOrder);
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicatorShown(TRUE);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(TRUE);
#if defined(QMC2_EMUTYPE_MAME)
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_YEAR, qmc2SortOrder);
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_YEAR, qmc2SortOrder);
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicatorShown(TRUE);
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicatorShown(TRUE);
#endif
      break;

    case QMC2_SORT_BY_MANUFACTURER:
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_MANU, qmc2SortOrder);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_MANU, qmc2SortOrder);
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicatorShown(TRUE);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(TRUE);
#if defined(QMC2_EMUTYPE_MAME)
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_MANU, qmc2SortOrder);
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_MANU, qmc2SortOrder);
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicatorShown(TRUE);
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicatorShown(TRUE);
#endif
      break;

    case QMC2_SORT_BY_NAME:
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_NAME, qmc2SortOrder);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_NAME, qmc2SortOrder);
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicatorShown(TRUE);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(TRUE);
#if defined(QMC2_EMUTYPE_MAME)
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_NAME, qmc2SortOrder);
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_NAME, qmc2SortOrder);
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicatorShown(TRUE);
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicatorShown(TRUE);
#endif
      break;

    case QMC2_SORT_BY_ROMTYPES:
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_RTYPES, qmc2SortOrder);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_RTYPES, qmc2SortOrder);
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicatorShown(TRUE);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(TRUE);
#if defined(QMC2_EMUTYPE_MAME)
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_RTYPES, qmc2SortOrder);
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_RTYPES, qmc2SortOrder);
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicatorShown(TRUE);
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicatorShown(TRUE);
#endif
      break;

    case QMC2_SORT_BY_PLAYERS:
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_PLAYERS, qmc2SortOrder);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_PLAYERS, qmc2SortOrder);
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicatorShown(TRUE);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(TRUE);
#if defined(QMC2_EMUTYPE_MAME)
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_PLAYERS, qmc2SortOrder);
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_PLAYERS, qmc2SortOrder);
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicatorShown(TRUE);
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicatorShown(TRUE);
#endif
      break;

#if defined(QMC2_EMUTYPE_MAME)
    case QMC2_SORT_BY_CATEGORY:
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_CATEGORY, qmc2SortOrder);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_CATEGORY, qmc2SortOrder);
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicatorShown(TRUE);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(TRUE);
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_CATEGORY, qmc2SortOrder);
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_CATEGORY, qmc2SortOrder);
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicatorShown(TRUE);
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicatorShown(TRUE);
      break;

    case QMC2_SORT_BY_VERSION:
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_VERSION, qmc2SortOrder);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_VERSION, qmc2SortOrder);
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicatorShown(TRUE);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(TRUE);
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_VERSION, qmc2SortOrder);
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicator(QMC2_GAMELIST_COLUMN_VERSION, qmc2SortOrder);
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicatorShown(TRUE);
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicatorShown(TRUE);
      break;
#endif

    default:
      qmc2MainWindow->treeWidgetGamelist->header()->setSortIndicatorShown(FALSE);
      qmc2MainWindow->treeWidgetHierarchy->header()->setSortIndicatorShown(FALSE);
#if defined(QMC2_EMUTYPE_MAME)
      qmc2MainWindow->treeWidgetCategoryView->header()->setSortIndicatorShown(FALSE);
      qmc2MainWindow->treeWidgetVersionView->header()->setSortIndicatorShown(FALSE);
#endif
      break;
  }

  if ( needFilter && !needReload ) {
    qmc2StatesTogglesEnabled = FALSE;
    qmc2MainWindow->actionRomStatusFilterC->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_C]);
    qmc2MainWindow->actionRomStatusFilterM->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_M]);
    qmc2MainWindow->actionRomStatusFilterI->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_I]);
    qmc2MainWindow->actionRomStatusFilterN->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_N]);
    qmc2MainWindow->actionRomStatusFilterU->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_U]);
    qmc2Gamelist->filter();
  }

  if ( qmc2Preview ) {
    if ( needReopenPreviewFile ) {
      if ( qmc2UsePreviewFile ) {
#if defined(QMC2_EMUTYPE_MAME)
        qmc2Preview->previewFile = unzOpen((const char *)config->value("MAME/FilesAndDirectories/PreviewFile").toString().toAscii());
        if ( qmc2Preview->previewFile == NULL )
          qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open preview file, please check access permissions for %1").arg(config->value("MAME/FilesAndDirectories/PreviewFile").toString()));
#elif defined(QMC2_EMUTYPE_MESS)
        qmc2Preview->previewFile = unzOpen((const char *)config->value("MESS/FilesAndDirectories/PreviewFile").toString().toAscii());
        if ( qmc2Preview->previewFile == NULL )
          qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open preview file, please check access permissions for %1").arg(config->value("MESS/FilesAndDirectories/PreviewFile").toString()));
#endif
      } else
        unzClose(qmc2Preview->previewFile);
    }
    qmc2Preview->update();
  }

  if ( qmc2Flyer ) {
    if ( needReopenFlyerFile ) {
      if ( qmc2UseFlyerFile ) {
#if defined(QMC2_EMUTYPE_MAME)
        qmc2Flyer->flyerFile = unzOpen((const char *)config->value("MAME/FilesAndDirectories/FlyerFile").toString().toAscii());
        if ( qmc2Flyer->flyerFile == NULL )
          qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open flyer file, please check access permissions for %1").arg(config->value("MAME/FilesAndDirectories/FlyerFile").toString()));
#elif defined(QMC2_EMUTYPE_MESS)
        qmc2Flyer->flyerFile = unzOpen((const char *)config->value("MESS/FilesAndDirectories/FlyerFile").toString().toAscii());
        if ( qmc2Flyer->flyerFile == NULL )
          qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open flyer file, please check access permissions for %1").arg(config->value("MESS/FilesAndDirectories/FlyerFile").toString()));
#endif
      } else
        unzClose(qmc2Flyer->flyerFile);
    }
    qmc2Flyer->update();
  }

  if ( qmc2Cabinet ) {
    if ( needReopenCabinetFile ) {
      if ( qmc2UseCabinetFile ) {
#if defined(QMC2_EMUTYPE_MAME)
        qmc2Cabinet->cabinetFile = unzOpen((const char *)config->value("MAME/FilesAndDirectories/CabinetFile").toString().toAscii());
        if ( qmc2Cabinet->cabinetFile == NULL )
          qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open cabinet file, please check access permissions for %1").arg(config->value("MAME/FilesAndDirectories/CabinetFile").toString()));
#elif defined(QMC2_EMUTYPE_MESS)
        qmc2Cabinet->cabinetFile = unzOpen((const char *)config->value("MESS/FilesAndDirectories/CabinetFile").toString().toAscii());
        if ( qmc2Cabinet->cabinetFile == NULL )
          qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open cabinet file, please check access permissions for %1").arg(config->value("MESS/FilesAndDirectories/CabinetFile").toString()));
#endif
      } else
        unzClose(qmc2Cabinet->cabinetFile);
    }
    qmc2Cabinet->update();
  }

  if ( qmc2Controller ) {
    if ( needReopenControllerFile ) {
      if ( qmc2UseControllerFile ) {
#if defined(QMC2_EMUTYPE_MAME)
        qmc2Controller->controllerFile = unzOpen((const char *)config->value("MAME/FilesAndDirectories/ControllerFile").toString().toAscii());
        if ( qmc2Controller->controllerFile == NULL )
          qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open controller file, please check access permissions for %1").arg(config->value("MAME/FilesAndDirectories/ControllerFile").toString()));
#elif defined(QMC2_EMUTYPE_MESS)
        qmc2Controller->controllerFile = unzOpen((const char *)config->value("MESS/FilesAndDirectories/ControllerFile").toString().toAscii());
        if ( qmc2Controller->controllerFile == NULL )
          qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open controller file, please check access permissions for %1").arg(config->value("MESS/FilesAndDirectories/ControllerFile").toString()));
#endif
      } else
        unzClose(qmc2Controller->controllerFile);
    }
    qmc2Controller->update();
  }

  if ( qmc2Marquee ) {
    if ( needReopenMarqueeFile ) {
      if ( qmc2UseMarqueeFile ) {
#if defined(QMC2_EMUTYPE_MAME)
        qmc2Marquee->marqueeFile = unzOpen((const char *)config->value("MAME/FilesAndDirectories/MarqueeFile").toString().toAscii());
        if ( qmc2Marquee->marqueeFile == NULL )
          qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open marquee file, please check access permissions for %1").arg(config->value("MAME/FilesAndDirectories/MarqueeFile").toString()));
#elif defined(QMC2_EMUTYPE_MESS)
        qmc2Marquee->marqueeFile = unzOpen((const char *)config->value("MESS/FilesAndDirectories/MarqueeFile").toString().toAscii());
        if ( qmc2Marquee->marqueeFile == NULL )
          qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open marquee file, please check access permissions for %1").arg(config->value("MESS/FilesAndDirectories/MarqueeFile").toString()));
#endif
      } else
        unzClose(qmc2Marquee->marqueeFile);
    }
    qmc2Marquee->update();
  }

  if ( qmc2Title ) {
    if ( needReopenTitleFile ) {
      if ( qmc2UseTitleFile ) {
#if defined(QMC2_EMUTYPE_MAME)
        qmc2Title->titleFile = unzOpen((const char *)config->value("MAME/FilesAndDirectories/TitleFile").toString().toAscii());
        if ( qmc2Title->titleFile == NULL )
          qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open title file, please check access permissions for %1").arg(config->value("MAME/FilesAndDirectories/TitleFile").toString()));
#elif defined(QMC2_EMUTYPE_MESS)
        qmc2Title->titleFile = unzOpen((const char *)config->value("MESS/FilesAndDirectories/TitleFile").toString().toAscii());
        if ( qmc2Title->titleFile == NULL )
          qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open title file, please check access permissions for %1").arg(config->value("MESS/FilesAndDirectories/TitleFile").toString()));
#endif
      } else
        unzClose(qmc2Title->titleFile);
    }
    qmc2Title->update();
  }

  if ( qmc2PCB ) {
    if ( needReopenPCBFile ) {
      if ( qmc2UsePCBFile ) {
#if defined(QMC2_EMUTYPE_MAME)
        qmc2PCB->pcbFile = unzOpen((const char *)config->value("MAME/FilesAndDirectories/PCBFile").toString().toAscii());
        if ( qmc2PCB->pcbFile == NULL )
          qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open PCB file, please check access permissions for %1").arg(config->value("MAME/FilesAndDirectories/PCBFile").toString()));
#elif defined(QMC2_EMUTYPE_MESS)
        qmc2PCB->pcbFile = unzOpen((const char *)config->value("MESS/FilesAndDirectories/PCBFile").toString().toAscii());
        if ( qmc2PCB->pcbFile == NULL )
          qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open PCB file, please check access permissions for %1").arg(config->value("MESS/FilesAndDirectories/TitleFile").toString()));
#endif
      } else
        unzClose(qmc2PCB->pcbFile);
    }
    qmc2PCB->update();
  }

  if ( needReopenIconFile ) {
    if ( qmc2UseIconFile ) {
#if defined(QMC2_EMUTYPE_MAME)
      qmc2IconFile = unzOpen((const char *)config->value("MAME/FilesAndDirectories/IconFile").toString().toAscii());
      if ( qmc2IconFile == NULL )
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open icon file, please check access permissions for %1").arg(config->value("MAME/FilesAndDirectories/IconFile").toString()));
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2IconFile = unzOpen((const char *)config->value("MESS/FilesAndDirectories/IconFile").toString().toAscii());
      if ( qmc2IconFile == NULL )
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open icon file, please check access permissions for %1").arg(config->value("MESS/FilesAndDirectories/IconFile").toString()));
#endif
    } else
      unzClose(qmc2IconFile);
  }

  if ( needReload ) {
#if defined(QMC2_EMUTYPE_MAME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("triggering automatic reload of game list"));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("triggering automatic reload of machine list"));
#endif
    qmc2AutomaticReload = TRUE;
    QTimer::singleShot(0, qmc2MainWindow->actionReload, SLOT(trigger()));
  }

  QTimer::singleShot(0, this, SLOT(applyDelayed()));
}

void Options::on_pushButtonDefault_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_pushButtonDefault_clicked()");
#endif

  restoreCurrentConfig(TRUE);
}

void Options::restoreCurrentConfig(bool useDefaultSettings)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::restoreCurrentConfig()");
#endif

  treeWidgetShortcuts->clear();
  treeWidgetJoystickMappings->clear();

  if ( useDefaultSettings ) {
    QString fn = config->fileName();
    delete config;
    QFile f(fn);
    f.copy(fn + ".bak");
    f.remove();
    config = new QSettings(QSettings::IniFormat, QSettings::UserScope, "qmc2");
    qmc2Config = config;
  }

  QString userScopePath = QMC2_DOT_PATH;

  // Frontend

  // GUI
  checkBoxToolbar->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/Toolbar", TRUE).toBool());
  checkBoxSaveLayout->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout", TRUE).toBool());
  checkBoxRestoreLayout->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/RestoreLayout", TRUE).toBool());
  checkBoxSaveGameSelection->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveGameSelection", TRUE).toBool());
  checkBoxRestoreGameSelection->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/RestoreGameSelection", TRUE).toBool());
  checkBoxStatusbar->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/Statusbar", TRUE).toBool());
  checkBoxStandardColorPalette->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/StandardColorPalette", TRUE).toBool());
  checkBoxProgressTexts->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts", FALSE).toBool());
  checkBoxProcessGameInfoDB->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/ProcessGameInfoDB", TRUE).toBool());
  checkBoxCompressGameInfoDB->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/CompressGameInfoDB", FALSE).toBool());
#if defined(QMC2_EMUTYPE_MAME)
  checkBoxProcessEmuInfoDB->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/ProcessEmuInfoDB", TRUE).toBool());
  checkBoxCompressEmuInfoDB->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/CompressEmuInfoDB", FALSE).toBool());
#endif
  qmc2ScaledPreview = config->value(QMC2_FRONTEND_PREFIX + "GUI/ScaledPreview", TRUE).toBool();
  checkBoxScaledPreview->setChecked(qmc2ScaledPreview);
  qmc2ScaledFlyer = config->value(QMC2_FRONTEND_PREFIX + "GUI/ScaledFlyer", TRUE).toBool();
  checkBoxScaledFlyer->setChecked(qmc2ScaledFlyer);
  qmc2ScaledCabinet = config->value(QMC2_FRONTEND_PREFIX + "GUI/ScaledCabinet", TRUE).toBool();
  checkBoxScaledCabinet->setChecked(qmc2ScaledCabinet);
  qmc2ScaledController = config->value(QMC2_FRONTEND_PREFIX + "GUI/ScaledController", TRUE).toBool();
  checkBoxScaledController->setChecked(qmc2ScaledController);
  qmc2ScaledMarquee = config->value(QMC2_FRONTEND_PREFIX + "GUI/ScaledMarquee", TRUE).toBool();
  checkBoxScaledMarquee->setChecked(qmc2ScaledMarquee);
  qmc2ScaledTitle = config->value(QMC2_FRONTEND_PREFIX + "GUI/ScaledTitle", TRUE).toBool();
  checkBoxScaledTitle->setChecked(qmc2ScaledTitle);
  qmc2ScaledPCB = config->value(QMC2_FRONTEND_PREFIX + "GUI/ScaledPCB", TRUE).toBool();
  checkBoxScaledPCB->setChecked(qmc2ScaledPCB);
  qmc2SmoothScaling = config->value(QMC2_FRONTEND_PREFIX + "GUI/SmoothScaling", FALSE).toBool();
  checkBoxSmoothScaling->setChecked(qmc2SmoothScaling);
  qmc2RetryLoadingImages = config->value(QMC2_FRONTEND_PREFIX + "GUI/RetryLoadingImages", TRUE).toBool();
  checkBoxRetryLoadingImages->setChecked(qmc2RetryLoadingImages);
  qmc2ParentImageFallback = config->value(QMC2_FRONTEND_PREFIX + "GUI/ParentImageFallback", FALSE).toBool();
  checkBoxParentImageFallback->setChecked(qmc2ParentImageFallback);
  comboBoxLanguage->setCurrentIndex(comboBoxLanguage->findText(config->value(QMC2_FRONTEND_PREFIX + "GUI/Language", "us").toString().toUpper(), Qt::MatchContains | Qt::MatchCaseSensitive));
  comboBoxStyle->clear();
  comboBoxStyle->addItem(QObject::tr("Default"));
  comboBoxStyle->addItems(QStyleFactory::keys());
  QString myStyle = QObject::tr((const char *)config->value(QMC2_FRONTEND_PREFIX + "GUI/Style", "Default").toString().toUtf8());
  comboBoxStyle->setCurrentIndex(comboBoxStyle->findText(myStyle, Qt::MatchFixedString));
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
  checkBoxKillEmulatorsOnExit->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/KillEmulatorsOnExit", TRUE).toBool());
  checkBoxShowMenuBar->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/ShowMenuBar", TRUE).toBool());
  checkBoxCheckSingleInstance->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/CheckSingleInstance", TRUE).toBool());
  checkBoxGameStatusIndicator->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/GameStatusIndicator", FALSE).toBool());
  checkBoxGameStatusIndicatorOnlyWhenRequired->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/GameStatusIndicatorOnlyWhenRequired", TRUE).toBool());
  checkBoxShowGameName->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/ShowGameName", FALSE).toBool());
  qmc2ShowGameName = checkBoxShowGameName->isChecked();
  checkBoxShowGameNameOnlyWhenRequired->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/ShowGameNameOnlyWhenRequired", TRUE).toBool());
  qmc2ShowGameNameOnlyWhenRequired = checkBoxShowGameNameOnlyWhenRequired->isChecked();
  spinBoxFrontendLogSize->setValue(config->value(QMC2_FRONTEND_PREFIX + "GUI/FrontendLogSize", 0).toInt());
  spinBoxEmulatorLogSize->setValue(config->value(QMC2_FRONTEND_PREFIX + "GUI/EmulatorLogSize", 0).toInt());
#if defined(QMC2_VARIANT_LAUNCHER)
  checkBoxMinimizeOnVariantLaunch->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/MinimizeOnVariantLaunch", FALSE).toBool());
  checkBoxExitOnVariantLaunch->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/ExitOnVariantLaunch", FALSE).toBool());
#endif
#if defined(QMC2_SHOWMEMINFO)
  checkBoxMemoryIndicator->setChecked(config->value(QMC2_FRONTEND_PREFIX + "GUI/MemoryIndicator", FALSE).toBool());
#endif
  
  // Files / Directories
  lineEditDataDirectory->setText(config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/DataDirectory", QMC2_DEFAULT_DATA_PATH + "/").toString());
#if defined(QMC2_EMUTYPE_MAME)
#if defined(QMC2_SDLMAME)
  lineEditTemporaryFile->setText(config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmame.tmp").toString());
  lineEditFrontendLogFile->setText(config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/LogFile", userScopePath + "/qmc2-sdlmame.log").toString());
#elif defined(QMC2_MAME)
  lineEditTemporaryFile->setText(config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mame.tmp").toString());
  lineEditFrontendLogFile->setText(config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/LogFile", userScopePath + "/qmc2-mame.log").toString());
#endif
  lineEditPreviewDirectory->setText(config->value("MAME/FilesAndDirectories/PreviewDirectory", QMC2_DEFAULT_DATA_PATH + "/prv/").toString());
  lineEditPreviewFile->setText(config->value("MAME/FilesAndDirectories/PreviewFile", QMC2_DEFAULT_DATA_PATH + "/prv/previews.zip").toString());
  qmc2UsePreviewFile = config->value("MAME/FilesAndDirectories/UsePreviewFile", FALSE).toBool();
  stackedWidgetPreview->setCurrentIndex(qmc2UsePreviewFile ? 1 : 0);
  radioButtonPreviewSelect->setText(qmc2UsePreviewFile ? tr("Preview file") : tr("Preview directory"));
  lineEditFlyerDirectory->setText(config->value("MAME/FilesAndDirectories/FlyerDirectory", QMC2_DEFAULT_DATA_PATH + "/fly/").toString());
  lineEditFlyerFile->setText(config->value("MAME/FilesAndDirectories/FlyerFile", QMC2_DEFAULT_DATA_PATH + "/fly/flyers.zip").toString());
  qmc2UseFlyerFile = config->value("MAME/FilesAndDirectories/UseFlyerFile", FALSE).toBool();
  stackedWidgetFlyer->setCurrentIndex(qmc2UseFlyerFile ? 1 : 0);
  radioButtonFlyerSelect->setText(qmc2UseFlyerFile ? tr("Flyer file") : tr("Flyer directory"));
  lineEditIconDirectory->setText(config->value("MAME/FilesAndDirectories/IconDirectory", QMC2_DEFAULT_DATA_PATH + "/ico/").toString());
  lineEditIconFile->setText(config->value("MAME/FilesAndDirectories/IconFile", QMC2_DEFAULT_DATA_PATH + "/ico/icons.zip").toString());
  qmc2UseIconFile = config->value("MAME/FilesAndDirectories/UseIconFile", FALSE).toBool();
  stackedWidgetIcon->setCurrentIndex(qmc2UseIconFile ? 1 : 0);
  radioButtonIconSelect->setText(qmc2UseIconFile ? tr("Icon file") : tr("Icon directory"));
  lineEditCabinetDirectory->setText(config->value("MAME/FilesAndDirectories/CabinetDirectory", QMC2_DEFAULT_DATA_PATH + "/cab/").toString());
  lineEditCabinetFile->setText(config->value("MAME/FilesAndDirectories/CabinetFile", QMC2_DEFAULT_DATA_PATH + "/cab/cabinets.zip").toString());
  qmc2UseCabinetFile = config->value("MAME/FilesAndDirectories/UseCabinetFile", FALSE).toBool();
  stackedWidgetCabinet->setCurrentIndex(qmc2UseCabinetFile ? 1 : 0);
  radioButtonCabinetSelect->setText(qmc2UseCabinetFile ? tr("Cabinet file") : tr("Cabinet directory"));
  lineEditControllerDirectory->setText(config->value("MAME/FilesAndDirectories/ControllerDirectory", QMC2_DEFAULT_DATA_PATH + "/ctl/").toString());
  lineEditControllerFile->setText(config->value("MAME/FilesAndDirectories/ControllerFile", QMC2_DEFAULT_DATA_PATH + "/ctl/controllers.zip").toString());
  qmc2UseControllerFile = config->value("MAME/FilesAndDirectories/UseControllerFile", FALSE).toBool();
  stackedWidgetController->setCurrentIndex(qmc2UseControllerFile ? 1 : 0);
  radioButtonControllerSelect->setText(qmc2UseControllerFile ? tr("Controller file") : tr("Controller directory"));
  lineEditMarqueeDirectory->setText(config->value("MAME/FilesAndDirectories/MarqueeDirectory", QMC2_DEFAULT_DATA_PATH + "/mrq/").toString());
  lineEditMarqueeFile->setText(config->value("MAME/FilesAndDirectories/MarqueeFile", QMC2_DEFAULT_DATA_PATH + "/mrq/marquees.zip").toString());
  qmc2UseMarqueeFile = config->value("MAME/FilesAndDirectories/UseMarqueeFile", FALSE).toBool();
  stackedWidgetMarquee->setCurrentIndex(qmc2UseMarqueeFile ? 1 : 0);
  radioButtonMarqueeSelect->setText(qmc2UseMarqueeFile ? tr("Marquee file") : tr("Marquee directory"));
  lineEditTitleDirectory->setText(config->value("MAME/FilesAndDirectories/TitleDirectory", QMC2_DEFAULT_DATA_PATH + "/ttl/").toString());
  lineEditTitleFile->setText(config->value("MAME/FilesAndDirectories/TitleFile", QMC2_DEFAULT_DATA_PATH + "/ttl/titles.zip").toString());
  qmc2UseTitleFile = config->value("MAME/FilesAndDirectories/UseTitleFile", FALSE).toBool();
  stackedWidgetTitle->setCurrentIndex(qmc2UseTitleFile ? 1 : 0);
  radioButtonTitleSelect->setText(qmc2UseTitleFile ? tr("Title file") : tr("Title directory"));
  lineEditPCBDirectory->setText(config->value("MAME/FilesAndDirectories/PCBDirectory", QMC2_DEFAULT_DATA_PATH + "/pcb/").toString());
  lineEditPCBFile->setText(config->value("MAME/FilesAndDirectories/PCBFile", QMC2_DEFAULT_DATA_PATH + "/pcb/pcbs.zip").toString());
  qmc2UsePCBFile = config->value("MAME/FilesAndDirectories/UsePCBFile", FALSE).toBool();
  stackedWidgetPCB->setCurrentIndex(qmc2UsePCBFile ? 1 : 0);
  radioButtonPCBSelect->setText(qmc2UsePCBFile ? tr("PCB file") : tr("PCB directory"));
  lineEditGameInfoDB->setText(config->value("MAME/FilesAndDirectories/GameInfoDB", QMC2_DEFAULT_DATA_PATH + "/cat/history.dat").toString());
  lineEditEmuInfoDB->setText(config->value("MAME/FilesAndDirectories/EmuInfoDB", QMC2_DEFAULT_DATA_PATH + "/cat/mameinfo.dat").toString());
  lineEditCatverIniFile->setText(config->value("MAME/FilesAndDirectories/CatverIni", userScopePath + "/catver.ini").toString());
  checkBoxUseCatverIni->setChecked(config->value(QMC2_FRONTEND_PREFIX + "Gamelist/UseCatverIni", FALSE).toBool());
#elif defined(QMC2_EMUTYPE_MESS)
#if defined(QMC2_SDLMESS)
  lineEditTemporaryFile->setText(config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmess.tmp").toString());
  lineEditFrontendLogFile->setText(config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/LogFile", userScopePath + "/qmc2-sdlmess.log").toString());
#elif defined(QMC2_MESS)
  lineEditTemporaryFile->setText(config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mess.tmp").toString());
  lineEditFrontendLogFile->setText(config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/LogFile", userScopePath + "/qmc2-mess.log").toString());
#endif
  lineEditPreviewDirectory->setText(config->value("MESS/FilesAndDirectories/PreviewDirectory", QMC2_DEFAULT_DATA_PATH + "/prv/").toString());
  lineEditPreviewFile->setText(config->value("MESS/FilesAndDirectories/PreviewFile", QMC2_DEFAULT_DATA_PATH + "/prv/previews.zip").toString());
  qmc2UsePreviewFile = config->value("MESS/FilesAndDirectories/UsePreviewFile", FALSE).toBool();
  stackedWidgetPreview->setCurrentIndex(qmc2UsePreviewFile ? 1 : 0);
  radioButtonPreviewSelect->setText(qmc2UsePreviewFile ? tr("Preview file") : tr("Preview directory"));
  lineEditFlyerDirectory->setText(config->value("MESS/FilesAndDirectories/FlyerDirectory", QMC2_DEFAULT_DATA_PATH + "/fly/").toString());
  lineEditFlyerFile->setText(config->value("MESS/FilesAndDirectories/FlyerFile", QMC2_DEFAULT_DATA_PATH + "/fly/flyers.zip").toString());
  qmc2UseFlyerFile = config->value("MESS/FilesAndDirectories/UseFlyerFile", FALSE).toBool();
  stackedWidgetFlyer->setCurrentIndex(qmc2UseFlyerFile ? 1 : 0);
  radioButtonFlyerSelect->setText(qmc2UseFlyerFile ? tr("Flyer file") : tr("Flyer directory"));
  lineEditIconDirectory->setText(config->value("MESS/FilesAndDirectories/IconDirectory", QMC2_DEFAULT_DATA_PATH + "/ico/").toString());
  lineEditIconFile->setText(config->value("MESS/FilesAndDirectories/IconFile", QMC2_DEFAULT_DATA_PATH + "/ico/icons.zip").toString());
  qmc2UseIconFile = config->value("MESS/FilesAndDirectories/UseIconFile", FALSE).toBool();
  stackedWidgetIcon->setCurrentIndex(qmc2UseIconFile ? 1 : 0);
  radioButtonIconSelect->setText(qmc2UseIconFile ? tr("Icon file") : tr("Icon directory"));
  lineEditCabinetDirectory->setText(config->value("MESS/FilesAndDirectories/CabinetDirectory", QMC2_DEFAULT_DATA_PATH + "/cab/").toString());
  lineEditCabinetFile->setText(config->value("MESS/FilesAndDirectories/CabinetFile", QMC2_DEFAULT_DATA_PATH + "/cab/cabinets.zip").toString());
  qmc2UseCabinetFile = config->value("MESS/FilesAndDirectories/UseCabinetFile", FALSE).toBool();
  stackedWidgetCabinet->setCurrentIndex(qmc2UseCabinetFile ? 1 : 0);
  radioButtonCabinetSelect->setText(qmc2UseCabinetFile ? tr("Cabinet file") : tr("Cabinet directory"));
  lineEditControllerDirectory->setText(config->value("MESS/FilesAndDirectories/ControllerDirectory", QMC2_DEFAULT_DATA_PATH + "/ctl/").toString());
  lineEditControllerFile->setText(config->value("MESS/FilesAndDirectories/ControllerFile", QMC2_DEFAULT_DATA_PATH + "/ctl/controllers.zip").toString());
  qmc2UseControllerFile = config->value("MESS/FilesAndDirectories/UseControllerFile", FALSE).toBool();
  stackedWidgetController->setCurrentIndex(qmc2UseControllerFile ? 1 : 0);
  radioButtonControllerSelect->setText(qmc2UseControllerFile ? tr("Controller file") : tr("Controller directory"));
  lineEditMarqueeDirectory->setText(config->value("MESS/FilesAndDirectories/MarqueeDirectory", QMC2_DEFAULT_DATA_PATH + "/mrq/").toString());
  lineEditMarqueeFile->setText(config->value("MESS/FilesAndDirectories/MarqueeFile", QMC2_DEFAULT_DATA_PATH + "/mrq/marquees.zip").toString());
  qmc2UseMarqueeFile = config->value("MESS/FilesAndDirectories/UseMarqueeFile", FALSE).toBool();
  stackedWidgetMarquee->setCurrentIndex(qmc2UseMarqueeFile ? 1 : 0);
  radioButtonMarqueeSelect->setText(qmc2UseMarqueeFile ? tr("Marquee file") : tr("Marquee directory"));
  lineEditTitleDirectory->setText(config->value("MESS/FilesAndDirectories/TitleDirectory", QMC2_DEFAULT_DATA_PATH + "/ttl/").toString());
  lineEditTitleFile->setText(config->value("MESS/FilesAndDirectories/TitleFile", QMC2_DEFAULT_DATA_PATH + "/ttl/titles.zip").toString());
  qmc2UseTitleFile = config->value("MESS/FilesAndDirectories/UseTitleFile", FALSE).toBool();
  stackedWidgetTitle->setCurrentIndex(qmc2UseTitleFile ? 1 : 0);
  radioButtonTitleSelect->setText(qmc2UseTitleFile ? tr("Title file") : tr("Title directory"));
  lineEditPCBDirectory->setText(config->value("MESS/FilesAndDirectories/PCBDirectory", QMC2_DEFAULT_DATA_PATH + "/pcb/").toString());
  lineEditPCBFile->setText(config->value("MESS/FilesAndDirectories/PCBFile", QMC2_DEFAULT_DATA_PATH + "/pcb/pcbs.zip").toString());
  qmc2UsePCBFile = config->value("MESS/FilesAndDirectories/UsePCBFile", FALSE).toBool();
  stackedWidgetPCB->setCurrentIndex(qmc2UsePCBFile ? 1 : 0);
  radioButtonPCBSelect->setText(qmc2UsePCBFile ? tr("PCB file") : tr("PCB directory"));
  lineEditGameInfoDB->setText(config->value("MESS/FilesAndDirectories/GameInfoDB", QMC2_DEFAULT_DATA_PATH + "/cat/sysinfo.dat").toString());
#endif

  // Gamelist
  checkBoxSortOnline->setChecked(config->value(QMC2_FRONTEND_PREFIX + "Gamelist/SortOnline", FALSE).toBool());
  checkBoxAutoTriggerROMCheck->setChecked(config->value(QMC2_FRONTEND_PREFIX + "Gamelist/AutoTriggerROMCheck", FALSE).toBool());
  checkBoxDoubleClickActivation->setChecked(config->value(QMC2_FRONTEND_PREFIX + "Gamelist/DoubleClickActivation", TRUE).toBool());
  checkBoxHideWhileLoading->setChecked(config->value(QMC2_FRONTEND_PREFIX + "Gamelist/HideWhileLoading", TRUE).toBool());
  checkBoxPlayOnSublistActivation->setChecked(config->value(QMC2_FRONTEND_PREFIX + "Gamelist/PlayOnSublistActivation", FALSE).toBool());
  qmc2CursorPositioningMode = (QAbstractItemView::ScrollHint)config->value(QMC2_FRONTEND_PREFIX + "Gamelist/CursorPosition", QMC2_CURSOR_POS_TOP).toInt();
  comboBoxCursorPosition->setCurrentIndex((int)qmc2CursorPositioningMode);
  spinBoxResponsiveness->setValue(config->value(QMC2_FRONTEND_PREFIX + "Gamelist/Responsiveness", 100).toInt());
  qmc2GamelistResponsiveness = spinBoxResponsiveness->value();
  spinBoxUpdateDelay->setValue(config->value(QMC2_FRONTEND_PREFIX + "Gamelist/UpdateDelay", 10).toInt());
  qmc2UpdateDelay = spinBoxUpdateDelay->value();
  comboBoxSortCriteria->setCurrentIndex(config->value(QMC2_FRONTEND_PREFIX + "Gamelist/SortCriteria", 0).toInt());
  qmc2SortCriteria = comboBoxSortCriteria->currentIndex();
  comboBoxSortOrder->setCurrentIndex(config->value(QMC2_FRONTEND_PREFIX + "Gamelist/SortOrder", 0).toInt());
  qmc2SortOrder = comboBoxSortOrder->currentIndex() == 0 ? Qt::AscendingOrder : Qt::DescendingOrder;
  qmc2Filter.setBit(QMC2_ROMSTATE_INT_C, config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowC", TRUE).toBool());
  toolButtonShowC->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_C]);
  qmc2Filter.setBit(QMC2_ROMSTATE_INT_M, config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowM", TRUE).toBool());
  toolButtonShowM->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_M]);
  qmc2Filter.setBit(QMC2_ROMSTATE_INT_I, config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowI", TRUE).toBool());
  toolButtonShowI->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_I]);
  qmc2Filter.setBit(QMC2_ROMSTATE_INT_N, config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowN", TRUE).toBool());
  toolButtonShowN->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_N]);
  qmc2Filter.setBit(QMC2_ROMSTATE_INT_U, config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowU", TRUE).toBool());
  toolButtonShowU->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_U]);

  if ( qmc2MainWindow ) {
    qmc2StatesTogglesEnabled = FALSE;
    qmc2MainWindow->actionRomStatusFilterC->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_C]);
    qmc2MainWindow->actionRomStatusFilterM->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_M]);
    qmc2MainWindow->actionRomStatusFilterI->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_I]);
    qmc2MainWindow->actionRomStatusFilterN->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_N]);
    qmc2MainWindow->actionRomStatusFilterU->setChecked(qmc2Filter[QMC2_ROMSTATE_INT_U]);
    if ( !qmc2EarlyStartup )
      qmc2StatesTogglesEnabled = TRUE;
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
      itemText += QObject::tr(words[i].toAscii());
    }
    item->setText(1, itemText);
    QString customSC = config->value(QString(QMC2_FRONTEND_PREFIX + "Shortcuts/%1").arg(itShortcut), itShortcut).toString();
    qmc2CustomShortcutMap[itShortcut] = customSC;
    if ( customSC != itShortcut ) {
      words = customSC.split("+");
      customSC = "";
      for (i = 0; i < words.count(); i++) {
        if ( i > 0 ) customSC += "+";
        customSC += QObject::tr(words[i].toAscii());
      }
      item->setText(2, customSC);
    }
  }

  // Joystick
#if QMC2_JOYSTICK == 1
  checkBoxEnableJoystickControl->setChecked(config->value(QMC2_FRONTEND_PREFIX + "Joystick/EnableJoystickControl", FALSE).toBool());
  checkBoxJoystickAutoRepeat->setChecked(config->value(QMC2_FRONTEND_PREFIX + "Joystick/AutoRepeat", TRUE).toBool());
  spinBoxJoystickAutoRepeatTimeout->setValue(config->value(QMC2_FRONTEND_PREFIX + "Joystick/AutoRepeatTimeout", 250).toInt());
  spinBoxJoystickEventTimeout->setValue(config->value(QMC2_FRONTEND_PREFIX + "Joystick/EventTimeout", 25).toInt());
  on_pushButtonRescanJoysticks_clicked();

  // Joystick function map
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

  // Tools / Proxy
#if defined (Q_WS_WIN)
  lineEditZipTool->setText(config->value(QMC2_FRONTEND_PREFIX + "Tools/ZipTool", "zip").toString());
  lineEditZipToolRemovalArguments->setText(config->value(QMC2_FRONTEND_PREFIX + "Tools/ZipToolRemovalArguments", "$ARCHIVE$ -d $FILELIST$").toString());
  lineEditFileRemovalTool->setText(config->value(QMC2_FRONTEND_PREFIX + "Tools/FileRemovalTool", "del").toString());
  lineEditFileRemovalToolArguments->setText(config->value(QMC2_FRONTEND_PREFIX + "Tools/FileRemovalToolArguments", "$FILELIST$").toString());
#else
  lineEditZipTool->setText(config->value(QMC2_FRONTEND_PREFIX + "Tools/ZipTool", "zip").toString());
  lineEditZipToolRemovalArguments->setText(config->value(QMC2_FRONTEND_PREFIX + "Tools/ZipToolRemovalArguments", "$ARCHIVE$ -d $FILELIST$").toString());
  lineEditFileRemovalTool->setText(config->value(QMC2_FRONTEND_PREFIX + "Tools/FileRemovalTool", "rm").toString());
  lineEditFileRemovalToolArguments->setText(config->value(QMC2_FRONTEND_PREFIX + "Tools/FileRemovalToolArguments", "-f -v $FILELIST$").toString());
#endif
  lineEditRomTool->setText(config->value(QMC2_FRONTEND_PREFIX + "Tools/RomTool", "").toString());
  lineEditRomToolArguments->setText(config->value(QMC2_FRONTEND_PREFIX + "Tools/RomToolArguments", "$ID$ $DESCRIPTION$").toString());
  checkBoxCopyToolOutput->setChecked(config->value(QMC2_FRONTEND_PREFIX + "Tools/CopyToolOutput", true).toBool());
  checkBoxCloseToolDialog->setChecked(config->value(QMC2_FRONTEND_PREFIX + "Tools/CloseToolDialog", false).toBool());

  groupBoxHTTPProxy->setChecked(config->value("Network/HTTPProxy/Enable", FALSE).toBool());
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
  lineEditExecutableFile->setText(config->value("MAME/FilesAndDirectories/ExecutableFile", "").toString());
  lineEditWorkingDirectory->setText(config->value("MAME/FilesAndDirectories/WorkingDirectory", "").toString());
  lineEditEmulatorLogFile->setText(config->value("MAME/FilesAndDirectories/LogFile", userScopePath + "/mame.log").toString());
  lineEditListXMLCache->setText(config->value("MAME/FilesAndDirectories/ListXMLCache", userScopePath + "/mame.lxc").toString());
  lineEditGamelistCacheFile->setText(config->value("MAME/FilesAndDirectories/GamelistCacheFile", userScopePath + "/mame.glc").toString());
  lineEditROMStateCacheFile->setText(config->value("MAME/FilesAndDirectories/ROMStateCacheFile", userScopePath + "/mame.rsc").toString());
  QString mawsCachePath = config->value("MAME/FilesAndDirectories/MAWSCacheDirectory", userScopePath + "/maws/").toString();
  lineEditMAWSCacheDirectory->setText(mawsCachePath);
  QDir mawsCacheDir(mawsCachePath);
  if ( !mawsCacheDir.exists() )
    mawsCacheDir.mkdir(mawsCachePath);
#if defined(QMC2_SDLMAME)
  lineEditOptionsTemplateFile->setText(config->value("MAME/FilesAndDirectories/OptionsTemplateFile", QMC2_DEFAULT_DATA_PATH + "/opt/SDLMAME/template.xml").toString());
#elif defined(QMC2_MAME)
  lineEditOptionsTemplateFile->setText(config->value("MAME/FilesAndDirectories/OptionsTemplateFile", QMC2_DEFAULT_DATA_PATH + "/opt/MAME/template.xml").toString());
#endif
  lineEditFavoritesFile->setText(config->value("MAME/FilesAndDirectories/FavoritesFile", userScopePath + "/mame.fav").toString());
  lineEditHistoryFile->setText(config->value("MAME/FilesAndDirectories/HistoryFile", userScopePath + "/mame.hst").toString());
#elif defined(QMC2_EMUTYPE_MESS)
  lineEditExecutableFile->setText(config->value("MESS/FilesAndDirectories/ExecutableFile", "").toString());
  lineEditWorkingDirectory->setText(config->value("MESS/FilesAndDirectories/WorkingDirectory", "").toString());
  lineEditEmulatorLogFile->setText(config->value("MESS/FilesAndDirectories/LogFile", userScopePath + "/mess.log").toString());
  lineEditListXMLCache->setText(config->value("MESS/FilesAndDirectories/ListXMLCache", userScopePath + "/mess.lxc").toString());
  lineEditGamelistCacheFile->setText(config->value("MESS/FilesAndDirectories/GamelistCacheFile", userScopePath + "/mess.glc").toString());
  lineEditROMStateCacheFile->setText(config->value("MESS/FilesAndDirectories/ROMStateCacheFile", userScopePath + "/mess.rsc").toString());
  lineEditSoftwareListCache->setText(config->value("MESS/FilesAndDirectories/SoftwareListCache", userScopePath + "/mess.swl").toString());
#if defined(QMC2_SDLMESS)
  lineEditOptionsTemplateFile->setText(config->value("MESS/FilesAndDirectories/OptionsTemplateFile", QMC2_DEFAULT_DATA_PATH + "/opt/SDLMESS/template.xml").toString());
#elif defined(QMC2_MESS)
  lineEditOptionsTemplateFile->setText(config->value("MESS/FilesAndDirectories/OptionsTemplateFile", QMC2_DEFAULT_DATA_PATH + "/opt/MESS/template.xml").toString());
#endif
  lineEditFavoritesFile->setText(config->value("MESS/FilesAndDirectories/FavoritesFile", userScopePath + "/mess.fav").toString());
  lineEditHistoryFile->setText(config->value("MESS/FilesAndDirectories/HistoryFile", userScopePath + "/mess.hst").toString());
#endif

  // Additional emulators
  tableWidgetRegisteredEmulators->clearContents();
#if defined(QMC2_EMUTYPE_MAME)
  config->beginGroup("MAME/RegisteredEmulators");
#elif defined(QMC2_EMUTYPE_MESS)
  config->beginGroup("MESS/RegisteredEmulators");
#endif
  QStringList additionalEmulators = config->childGroups();
  tableWidgetRegisteredEmulators->setSortingEnabled(FALSE);
  foreach (QString emuName, additionalEmulators) {
    QString emuCommand = config->value(QString("%1/Executable").arg(emuName)).toString();
    QString emuWorkDir = config->value(QString("%1/WorkingDirectory").arg(emuName)).toString();
    QString emuArgs = config->value(QString("%1/Arguments").arg(emuName)).toString();
    int row = tableWidgetRegisteredEmulators->rowCount();
    tableWidgetRegisteredEmulators->insertRow(row);
    tableWidgetRegisteredEmulators->setItem(row, QMC2_ADDTLEMUS_COLUMN_NAME, new QTableWidgetItem(emuName));
    tableWidgetRegisteredEmulators->setItem(row, QMC2_ADDTLEMUS_COLUMN_EXEC, new QTableWidgetItem(emuCommand));
    tableWidgetRegisteredEmulators->setItem(row, QMC2_ADDTLEMUS_COLUMN_WDIR, new QTableWidgetItem(emuWorkDir));
    tableWidgetRegisteredEmulators->setItem(row, QMC2_ADDTLEMUS_COLUMN_ARGS, new QTableWidgetItem(emuArgs));
  }
  config->endGroup();
  tableWidgetRegisteredEmulators->setSortingEnabled(TRUE);

  if ( useDefaultSettings ) {
    QString fn = config->fileName();
    delete config;
    QFile f0(fn);
    f0.remove();
    QFile f(fn + ".bak");
    f.copy(fn);
    f.remove();
    config = new QSettings(QSettings::IniFormat, QSettings::UserScope, "qmc2");
    qmc2Config = config;
  }

  QTimer::singleShot(0, this, SLOT(applyDelayed()));
}

void Options::applyDelayed()
{
  // just for safety :)...
  if ( qmc2MainWindow == NULL ) {
    QTimer::singleShot(0, this, SLOT(applyDelayed()));
    return;
  }

  static bool firstTime = TRUE;

  if ( firstTime ) {
#if defined(Q_WS_WIN)
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
#if QMC2_JOYSTICK == 1
      treeWidgetJoystickMappings->header()->restoreState(config->value(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/JoyMapHeaderState").toByteArray());
#endif
      tableWidgetRegisteredEmulators->horizontalHeader()->restoreState(config->value(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/RegisteredEmulatorsHeaderState").toByteArray());
    }
    tableWidgetRegisteredEmulators->resizeRowsToContents();
    firstTime = FALSE;
  }

  // redraw detail if setup changed
  qmc2MainWindow->on_tabWidgetGameDetail_currentChanged(qmc2MainWindow->tabWidgetGameDetail->currentIndex());

  // hide / show the menu bar
#if defined(Q_WS_X11)
  if ( qmc2MainWindow->tabWidgetGamelist->currentIndex() != QMC2_EMBED_INDEX || !qmc2MainWindow->toolButtonEmbedderMaximizeToggle->isChecked() )
    qmc2MainWindow->menuBar()->setVisible(checkBoxShowMenuBar->isChecked());
#else
  qmc2MainWindow->menuBar()->setVisible(checkBoxShowMenuBar->isChecked());
#endif
  qApp->processEvents();
  qmc2VariantSwitchReady = TRUE;
}

void Options::on_toolButtonBrowseStyleSheet_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseStyleSheet_clicked()");
#endif

  QString s = QFileDialog::getOpenFileName(this, tr("Choose Qt style sheet file"), lineEditStyleSheet->text(), tr("Qt Style Sheets (*.qss)"));
  if ( !s.isNull() )
    lineEditStyleSheet->setText(s);
  raise();
}

void Options::on_toolButtonBrowseTemporaryFile_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseTemporaryFile_clicked()");
#endif

  QString s = QFileDialog::getOpenFileName(this, tr("Choose temporary work file"), lineEditTemporaryFile->text(), tr("All files (*)"));
  if ( !s.isNull() )
    lineEditTemporaryFile->setText(s);
  raise();
}

void Options::on_toolButtonBrowsePreviewDirectory_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowsePreviewDirectory_clicked()");
#endif

  QString s = QFileDialog::getExistingDirectory(this, tr("Choose preview directory"), lineEditPreviewDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
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

  QString s = QFileDialog::getExistingDirectory(this, tr("Choose flyer directory"), lineEditFlyerDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
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

  QString s = QFileDialog::getExistingDirectory(this, tr("Choose icon directory"), lineEditIconDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
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

  QString s = QFileDialog::getExistingDirectory(this, tr("Choose cabinet directory"), lineEditCabinetDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
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

  QString s = QFileDialog::getExistingDirectory(this, tr("Choose controller directory"), lineEditControllerDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
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

  QString s = QFileDialog::getExistingDirectory(this, tr("Choose marquee directory"), lineEditMarqueeDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
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

  QString s = QFileDialog::getExistingDirectory(this, tr("Choose title directory"), lineEditTitleDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
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

  QString s = QFileDialog::getExistingDirectory(this, tr("Choose PCB directory"), lineEditPCBDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
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

  QString s = QFileDialog::getOpenFileName(this, tr("Choose options template file"), lineEditOptionsTemplateFile->text(), tr("All files (*)"));
  if ( !s.isNull() )
    lineEditOptionsTemplateFile->setText(s);
  raise();
}

void Options::on_toolButtonBrowseExecutableFile_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseExecutableFile_clicked()");
#endif

  QString s = QFileDialog::getOpenFileName(this, tr("Choose emulator executable file"), lineEditExecutableFile->text(), tr("All files (*)"));
  if ( !s.isNull() )
    lineEditExecutableFile->setText(s);
  raise();
}

void Options::on_toolButtonBrowseEmulatorLogFile_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseEmulatorLogFile_clicked()");
#endif

  QString s = QFileDialog::getOpenFileName(this, tr("Choose emulator log file"), lineEditEmulatorLogFile->text(), tr("All files (*)"));
  if ( !s.isNull() )
    lineEditEmulatorLogFile->setText(s);
  raise();
}

void Options::on_toolButtonBrowseListXMLCache_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseListXMLCache_clicked()");
#endif

  QString s = QFileDialog::getOpenFileName(this, tr("Choose XML gamelist cache file"), lineEditListXMLCache->text(), tr("All files (*)"));
  if ( !s.isNull() )
    lineEditListXMLCache->setText(s);
  raise();
}

void Options::on_toolButtonBrowseZipTool_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseZipTool_clicked()");
#endif

  QString s = QFileDialog::getOpenFileName(this, tr("Choose zip tool"), lineEditZipTool->text(), tr("All files (*)"));
  if ( !s.isNull() )
    lineEditZipTool->setText(s);
  raise();
}

void Options::on_toolButtonBrowseFileRemovalTool_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseFileRemovalTool_clicked()");
#endif

  QString s = QFileDialog::getOpenFileName(this, tr("Choose file removal tool"), lineEditFileRemovalTool->text(), tr("All files (*)"));
  if ( !s.isNull() )
    lineEditFileRemovalTool->setText(s);
  raise();
}

void Options::on_toolButtonBrowseRomTool_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseRomTool_clicked()");
#endif

  QString s = QFileDialog::getOpenFileName(this, tr("Choose ROM tool"), lineEditRomTool->text(), tr("All files (*)"));
  if ( !s.isNull() )
    lineEditRomTool->setText(s);
  raise();
}

void Options::on_toolButtonBrowseFavoritesFile_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseFavoritesFile_clicked()");
#endif

  QString s = QFileDialog::getOpenFileName(this, tr("Choose game favorites file"), lineEditFavoritesFile->text(), tr("All files (*)"));
  if ( !s.isNull() )
    lineEditFavoritesFile->setText(s);
  raise();
}

void Options::on_toolButtonBrowseHistoryFile_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseHistoryFile_clicked()");
#endif

  QString s = QFileDialog::getOpenFileName(this, tr("Choose play history file"), lineEditHistoryFile->text(), tr("All files (*)"));
  if ( !s.isNull() )
    lineEditHistoryFile->setText(s);
  raise();
}

void Options::on_toolButtonBrowseGamelistCacheFile_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseGamelistCacheFile_clicked()");
#endif

  QString s = QFileDialog::getOpenFileName(this, tr("Choose gamelist cache file"), lineEditGamelistCacheFile->text(), tr("All files (*)"));
  if ( !s.isNull() )
    lineEditGamelistCacheFile->setText(s);
  raise();
}

void Options::on_toolButtonBrowseROMStateCacheFile_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseROMStateCacheFile_clicked()");
#endif

  QString s = QFileDialog::getOpenFileName(this, tr("Choose ROM state cache file"), lineEditROMStateCacheFile->text(), tr("All files (*)"));
  if ( !s.isNull() )
    lineEditROMStateCacheFile->setText(s);
  raise();
}

void Options::on_toolButtonBrowseWorkingDirectory_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseWorkingDirectory_clicked()");
#endif

  QString s = QFileDialog::getExistingDirectory(this, tr("Choose working directory"), lineEditWorkingDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
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

  QString s = QFileDialog::getExistingDirectory(this, tr("Choose MAWS cache directory"), lineEditMAWSCacheDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
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

  QString s = QFileDialog::getOpenFileName(this, tr("Choose software list cache file"), lineEditSoftwareListCache->text(), tr("All files (*)"));
  if ( !s.isNull() )
    lineEditSoftwareListCache->setText(s);
  raise();
}


void Options::on_toolButtonBrowseFrontendLogFile_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseFrontendLogFile_clicked()");
#endif

  QString s = QFileDialog::getOpenFileName(this, tr("Choose front end log file"), lineEditFrontendLogFile->text(), tr("All files (*)"));
  if ( !s.isNull() )
    lineEditFrontendLogFile->setText(s);
  raise();
}

void Options::on_toolButtonBrowseDataDirectory_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseDataDirectory_clicked()");
#endif

  QString s = QFileDialog::getExistingDirectory(this, tr("Choose data directory"), lineEditDataDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if ( !s.isNull() ) {
    if ( !s.endsWith("/") ) s += "/";
    lineEditDataDirectory->setText(s);
  }
  raise();
}

void Options::on_toolButtonBrowseGameInfoDB_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseGameInfoDB_clicked()");
#endif

  QString s = QFileDialog::getOpenFileName(this, tr("Choose game info DB"), lineEditGameInfoDB->text(), tr("All files (*)"));
  if ( !s.isNull() )
    lineEditGameInfoDB->setText(s);
  raise();
}

#if defined(QMC2_EMUTYPE_MAME)
void Options::on_toolButtonBrowseEmuInfoDB_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseEmuInfoDB_clicked()");
#endif

  QString s = QFileDialog::getOpenFileName(this, tr("Choose emulator info DB"), lineEditEmuInfoDB->text(), tr("All files (*)"));
  if ( !s.isNull() )
    lineEditEmuInfoDB->setText(s);
  raise();
}

void Options::on_toolButtonBrowseCatverIniFile_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseCatverIniFile_clicked()");
#endif

  QString s = QFileDialog::getOpenFileName(this, tr("Choose catver.ini file"), lineEditCatverIniFile->text(), tr("All files (*)"));
  if ( !s.isNull() )
    lineEditCatverIniFile->setText(s);
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
#if defined(Q_WS_MAC)
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
#if defined(Q_WS_MAC)
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

void Options::closeEvent(QCloseEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::closeEvent(QCloseEvent *e = 0x" + QString::number((ulong)e, 16) + ")");
#endif

  e->accept();
}

void Options::showEvent(QShowEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::showEvent(QShowEvent *e = 0x" + QString::number((ulong)e, 16) + ")");
#endif

  if ( !qmc2CleaningUp && !qmc2EarlyStartup )
    if ( config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
      config->setValue(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/Visible", TRUE);

  e->accept();
}

void Options::hideEvent(QHideEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::hideEvent(QHideEvent *e = 0x" + QString::number((ulong)e, 16) + ")");
#endif

  if ( !qmc2CleaningUp && !qmc2EarlyStartup )
    if ( config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
      config->setValue(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/Visible", FALSE);

  e->accept();
}

void Options::moveEvent(QMoveEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::moveEvent(QMoveEvent *e = 0x" + QString::number((ulong)e, 16) + ")");
#endif

  if ( !qmc2CleaningUp && !qmc2EarlyStartup )
    if ( config->value(QMC2_FRONTEND_PREFIX + "GUI/SaveLayout").toBool() )
      config->setValue(QMC2_FRONTEND_PREFIX + "Layout/OptionsWidget/Position", pos());

  e->accept();
}

void Options::resizeEvent(QResizeEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::resizeEvent(QResizeEvent *e = 0x" + QString::number((ulong)e, 16) + ")");
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
  radioButtonMarqueeSelect->setText(!currentUseMarqueeFile ? tr("Marquee file") : tr("Marquee directory"));
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

void Options::on_toolButtonBrowsePreviewFile_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowsePreviewFile_clicked()");
#endif

  QString s = QFileDialog::getOpenFileName(this, tr("Choose ZIP-compressed preview file"), lineEditPreviewFile->text(), tr("All files (*)"));
  if ( !s.isNull() )
    lineEditPreviewFile->setText(s);
  raise();
}

void Options::on_toolButtonBrowseFlyerFile_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseFlyerFile_clicked()");
#endif

  QString s = QFileDialog::getOpenFileName(this, tr("Choose ZIP-compressed flyer file"), lineEditFlyerFile->text(), tr("All files (*)"));
  if ( !s.isNull() )
    lineEditFlyerFile->setText(s);
  raise();
}

void Options::on_toolButtonBrowseIconFile_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseIconFile_clicked()");
#endif

  QString s = QFileDialog::getOpenFileName(this, tr("Choose ZIP-compressed icon file"), lineEditIconFile->text(), tr("All files (*)"));
  if ( !s.isNull() )
    lineEditIconFile->setText(s);
  raise();
}

void Options::on_toolButtonBrowseCabinetFile_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseCabinetFile_clicked()");
#endif

  QString s = QFileDialog::getOpenFileName(this, tr("Choose ZIP-compressed cabinet file"), lineEditCabinetFile->text(), tr("All files (*)"));
  if ( !s.isNull() )
    lineEditCabinetFile->setText(s);
  raise();
}

void Options::on_toolButtonBrowseControllerFile_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseControllerFile_clicked()");
#endif

  QString s = QFileDialog::getOpenFileName(this, tr("Choose ZIP-compressed controller file"), lineEditControllerFile->text(), tr("All files (*)"));
  if ( !s.isNull() )
    lineEditControllerFile->setText(s);
  raise();
}

void Options::on_toolButtonBrowseMarqueeFile_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseMarqueeFile_clicked()");
#endif

  QString s = QFileDialog::getOpenFileName(this, tr("Choose ZIP-compressed marquee file"), lineEditMarqueeFile->text(), tr("All files (*)"));
  if ( !s.isNull() )
    lineEditMarqueeFile->setText(s);
  raise();
}

void Options::on_toolButtonBrowseTitleFile_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseTitleFile_clicked()");
#endif

  QString s = QFileDialog::getOpenFileName(this, tr("Choose ZIP-compressed title file"), lineEditTitleFile->text(), tr("All files (*)"));
  if ( !s.isNull() )
    lineEditTitleFile->setText(s);
  raise();
}

void Options::on_toolButtonBrowsePCBFile_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowsePCBFile_clicked()");
#endif

  QString s = QFileDialog::getOpenFileName(this, tr("Choose ZIP-compressed PCB file"), lineEditPCBFile->text(), tr("All files (*)"));
  if ( !s.isNull() )
    lineEditPCBFile->setText(s);
  raise();
}

void Options::on_treeWidgetShortcuts_itemActivated(QTreeWidgetItem *item)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_treeWidgetShortcuts_itemActivated(QTreeWidgetItem *item = 0x"+ QString::number((ulong)item, 16) + ")");
#endif

  if ( !item )
    return;

  qApp->removeEventFilter(qmc2KeyPressFilter);

  KeySequenceScanner keySeqScanner(this, qmc2QtKeyMap.contains(item->text(1)));
  if ( keySeqScanner.exec() == QDialog::Accepted ) {
    QStringList words = item->text(1).split("+");
    QString nativeShortcut = "";
    int i;
    for (i = 0; i < words.count(); i++) {
      if ( i > 0 ) nativeShortcut += "+";
      nativeShortcut += QObject::tr(words[i].toAscii());
    }

    bool found = FALSE;
    QMapIterator<QString, QPair<QString, QAction *> > it(qmc2ShortcutMap);
    while ( it.hasNext() && !found ) {
      it.next();
      words = it.key().split("+");
      QString itShortcut = "";
      for (i = 0; i < words.count(); i++) {
        if ( i > 0 ) itShortcut += "+";
        itShortcut += QObject::tr(words[i].toAscii());
      }

      if ( itShortcut == nativeShortcut ) {
        found = TRUE;
        nativeShortcut = it.key();
      }
    }

    if ( found ) {
      qmc2CustomShortcutMap[nativeShortcut] = keySeqScanner.currentKeySequence;
      item->setText(2, keySeqScanner.labelKeySequence->text());
      QTimer::singleShot(0, this, SLOT(checkShortcuts()));
    }

    pushButtonResetShortcut->setEnabled(TRUE);
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
    pushButtonRedefineKeySequence->setEnabled(TRUE);
    pushButtonResetShortcut->setEnabled(selItems[0]->text(2).length() > 0);
  } else {
    pushButtonRedefineKeySequence->setEnabled(FALSE);
    pushButtonResetShortcut->setEnabled(FALSE);
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
    int i;
    for (i = 0; i < words.count(); i++) {
      if ( i > 0 ) nativeShortcut += "+";
      nativeShortcut += QObject::tr(words[i].toAscii());
    }

    bool found = FALSE;
    QMapIterator<QString, QPair<QString, QAction *> > it(qmc2ShortcutMap);
    while ( it.hasNext() && !found ) {
      it.next();
      words = it.key().split("+");
      QString itShortcut = "";
      for (i = 0; i < words.count(); i++) {
        if ( i > 0 ) itShortcut += "+";
        itShortcut += QObject::tr(words[i].toAscii());
      }

      if ( itShortcut == nativeShortcut ) {
        found = TRUE;
        nativeShortcut = it.key();
      }
    }

    if ( found ) {
      qmc2CustomShortcutMap[nativeShortcut] = nativeShortcut;
      selItems[0]->setText(2, "");
      QTimer::singleShot(0, this, SLOT(checkShortcuts()));
    }

    pushButtonResetShortcut->setEnabled(FALSE);
  }
}

void Options::on_pushButtonDetailSetup_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_pushButtonDetailSetup_clicked()");
#endif

  qmc2DetailSetupParent = this;
  qmc2MainWindow->on_menuTabWidgetGameDetail_Setup_activated();
}

void Options::checkShortcuts()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::checkShortcuts()");
#endif

  static QBrush redBrush(QColor(255, 0, 0));
  static QBrush greenBrush(QColor(0, 255, 0));
  static QBrush greyBrush(QColor(128, 128, 128));
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
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseAdditionalEmulatorExecutable_clicked()");
#endif

  QString s = QFileDialog::getOpenFileName(this, tr("Choose emulator executable file"), lineEditAdditionalEmulatorExecutableFile->text(), tr("All files (*)"));
  if ( !s.isNull() )
    lineEditAdditionalEmulatorExecutableFile->setText(s);
  raise();
}

void Options::on_toolButtonBrowseAdditionalEmulatorWorkingDirectory_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonBrowseAdditionalEmulatorWorkingDirectory_clicked()");
#endif

  QString s = QFileDialog::getExistingDirectory(this, tr("Choose working directory"), lineEditAdditionalEmulatorWorkingDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if ( !s.isNull() ) {
    if ( !s.endsWith("/") ) s += "/";
    lineEditAdditionalEmulatorWorkingDirectory->setText(s);
  }
  raise();
}


void Options::on_toolButtonAddEmulator_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonAddEmulator_clicked()");
#endif

  tableWidgetRegisteredEmulators->setSortingEnabled(FALSE);
  int row = tableWidgetRegisteredEmulators->rowCount();
  tableWidgetRegisteredEmulators->insertRow(row);
  tableWidgetRegisteredEmulators->setItem(row, QMC2_ADDTLEMUS_COLUMN_NAME, new QTableWidgetItem(lineEditAdditionalEmulatorName->text()));
  tableWidgetRegisteredEmulators->setItem(row, QMC2_ADDTLEMUS_COLUMN_EXEC, new QTableWidgetItem(lineEditAdditionalEmulatorExecutableFile->text()));
  tableWidgetRegisteredEmulators->setItem(row, QMC2_ADDTLEMUS_COLUMN_WDIR, new QTableWidgetItem(lineEditAdditionalEmulatorWorkingDirectory->text()));
  tableWidgetRegisteredEmulators->setItem(row, QMC2_ADDTLEMUS_COLUMN_ARGS, new QTableWidgetItem(lineEditAdditionalEmulatorArguments->text()));
  on_lineEditAdditionalEmulatorName_textChanged(lineEditAdditionalEmulatorName->text());
  tableWidgetRegisteredEmulators->setSortingEnabled(TRUE);
}

void Options::on_toolButtonSaveEmulator_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonSaveEmulator_clicked()");
#endif

  tableWidgetRegisteredEmulators->setSortingEnabled(FALSE);
  QString name = lineEditAdditionalEmulatorName->text();
  if ( !name.isEmpty() ) {
    QList<QTableWidgetItem *> il = tableWidgetRegisteredEmulators->findItems(name, Qt::MatchExactly);
    int row = il[QMC2_ADDTLEMUS_COLUMN_NAME]->row();
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
  tableWidgetRegisteredEmulators->setSortingEnabled(TRUE);
}

void Options::on_toolButtonRemoveEmulator_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonRemoveEmulator_clicked()");
#endif

  QList<QTableWidgetItem *> sl = tableWidgetRegisteredEmulators->selectedItems();
  if ( !sl.isEmpty() ) {
    tableWidgetRegisteredEmulators->removeRow(sl[QMC2_ADDTLEMUS_COLUMN_NAME]->row());
  }
}

void Options::on_tableWidgetRegisteredEmulators_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_tableWidgetRegisteredEmulators_itemSelectionChanged()");
#endif

  QList<QTableWidgetItem *> sl = tableWidgetRegisteredEmulators->selectedItems();
  if ( !sl.isEmpty() ) {
    int row = sl[QMC2_ADDTLEMUS_COLUMN_NAME]->row();
    if ( tableWidgetRegisteredEmulators->item(row, QMC2_ADDTLEMUS_COLUMN_NAME) ) {
      lineEditAdditionalEmulatorName->setText(tableWidgetRegisteredEmulators->item(row, QMC2_ADDTLEMUS_COLUMN_NAME)->text());
      toolButtonRemoveEmulator->setEnabled(TRUE);
    } else {
      lineEditAdditionalEmulatorName->clear();
      toolButtonRemoveEmulator->setEnabled(FALSE);
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
    toolButtonRemoveEmulator->setEnabled(FALSE);
  }
}

void Options::on_lineEditAdditionalEmulatorName_textChanged(const QString &s)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_lineEditAdditionalEmulatorName_textChanged(const QString &s = ...)");
#endif

  QString text = lineEditAdditionalEmulatorName->text();
  if ( !text.isEmpty() ) {
    if ( text == tr("Default") ) {
      // this name isn't allowed!
      toolButtonAddEmulator->setEnabled(FALSE);
      toolButtonSaveEmulator->setEnabled(FALSE);
      toolButtonRemoveEmulator->setEnabled(FALSE);
    } else {
      QList<QTableWidgetItem *> il = tableWidgetRegisteredEmulators->findItems(text, Qt::MatchExactly);
      toolButtonAddEmulator->setEnabled(il.isEmpty());
      toolButtonSaveEmulator->setEnabled(!il.isEmpty());
    }
  } else {
    toolButtonAddEmulator->setEnabled(FALSE);
    toolButtonSaveEmulator->setEnabled(FALSE);
  }
}

#if QMC2_JOYSTICK == 1
void Options::on_pushButtonRescanJoysticks_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_pushButtonRescanJoysticks_clicked()");
#endif

  toolButtonMapJoystick->setChecked(TRUE);
  on_toolButtonMapJoystick_clicked();

  QStringList joystickNames;
  joystickNames << tr("No joysticks found");

  if ( joystick )
    delete joystick;
  joystick = new Joystick(0, 
                          spinBoxJoystickEventTimeout->value(),
                          checkBoxJoystickAutoRepeat->isChecked(),
                          spinBoxJoystickAutoRepeatTimeout->value());
  if ( joystick )
    if ( joystick->joystickNames.count() > 0 )
      joystickNames = joystick->joystickNames;

  comboBoxSelectJoysticks->clear();
  comboBoxSelectJoysticks->insertItems(0, joystickNames);
  comboBoxSelectJoysticks->setCurrentIndex(config->value(QMC2_FRONTEND_PREFIX + "Joystick/Index", 0).toInt());
}

void Options::on_toolButtonCalibrateAxes_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonCalibrateAxes_clicked()");
#endif

  if ( comboBoxSelectJoysticks->currentText() == tr("No joysticks found") || comboBoxSelectJoysticks->currentIndex() < 0 ) {
    toolButtonMapJoystick->setChecked(TRUE);
    on_toolButtonMapJoystick_clicked();
    return;
  }

  if ( joystick ) {
    if ( joystick->isOpen() )
      joystick->close();
    if ( joystick->open(comboBoxSelectJoysticks->currentIndex()) ) {
      // create joystick calibration widget
      QGridLayout *myLayout = (QGridLayout *)qmc2Options->groupBoxCalibrationAndTest->layout();
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
      joystickCalibrationWidget = new JoystickCalibrationWidget(joystick, groupBoxCalibrationAndTest);
      myLayout->addWidget(scrollArea);
      scrollArea->setWidget(joystickCalibrationWidget);
      scrollArea->show();
      joystickCalibrationWidget->show();
    } else {
      toolButtonMapJoystick->setChecked(TRUE);
      on_toolButtonMapJoystick_clicked();
    }
  } else {
    toolButtonMapJoystick->setChecked(TRUE);
    on_toolButtonMapJoystick_clicked();
  }
}

void Options::on_toolButtonTestJoystick_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonTestJoystick_clicked()");
#endif

  if ( comboBoxSelectJoysticks->currentText() == tr("No joysticks found") || comboBoxSelectJoysticks->currentIndex() < 0 ) {
    toolButtonMapJoystick->setChecked(TRUE);
    on_toolButtonMapJoystick_clicked();
    return;
  }

  if ( joystick ) {
    if ( joystick->isOpen() )
      joystick->close();
    if ( joystick->open(comboBoxSelectJoysticks->currentIndex()) ) {
      // create joystick test widget
      QGridLayout *myLayout = (QGridLayout *)qmc2Options->groupBoxCalibrationAndTest->layout();
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
      joystickTestWidget = new JoystickTestWidget(joystick, groupBoxCalibrationAndTest);
      myLayout->addWidget(scrollArea);
      scrollArea->setWidget(joystickTestWidget);
      scrollArea->show();
      joystickTestWidget->show();
    } else {
      toolButtonMapJoystick->setChecked(TRUE);
      on_toolButtonMapJoystick_clicked();
    }
  } else {
    toolButtonMapJoystick->setChecked(TRUE);
    on_toolButtonMapJoystick_clicked();
  }
}

void Options::on_toolButtonMapJoystick_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::on_toolButtonMapJoystick_clicked()");
#endif

  bool relayout = ( joystickCalibrationWidget || joystickTestWidget );
  
  if ( joystickCalibrationWidget ) {
    groupBoxCalibrationAndTest->layout()->removeWidget(scrollArea);
    scrollArea->takeWidget();
    scrollArea->hide();
    delete joystickCalibrationWidget;
    joystickCalibrationWidget = NULL;
  }
  if ( joystickTestWidget ) {
    groupBoxCalibrationAndTest->layout()->removeWidget(scrollArea);
    scrollArea->takeWidget();
    scrollArea->hide();
    delete joystickTestWidget;
    joystickTestWidget = NULL;
  }

  if ( relayout ) {
    QGridLayout *myLayout = (QGridLayout *)qmc2Options->groupBoxCalibrationAndTest->layout();
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
      joystick->close();
    }
}

void Options::on_checkBoxEnableJoystickControl_toggled(bool enable)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Options::on_checkBoxEnableJoystickControl_toggled(bool enable = %1)").arg(enable));
#endif

  toolButtonMapJoystick->setChecked(TRUE);
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
      qmc2SuppressQtMessages = TRUE;
      JoystickFunctionScanner joyFuncScanner(joystick, this);
      if ( joyFuncScanner.exec() == QDialog::Accepted ) {
        item->setText(1, joyFuncScanner.labelJoystickFunction->text());
        qmc2JoystickFunctionMap.insertMulti(joyFuncScanner.labelJoystickFunction->text(), item->whatsThis(0));
        pushButtonRemoveJoystickMapping->setEnabled(item->text(1).length() > 0);
        QTimer::singleShot(0, this, SLOT(checkJoystickMappings()));
      }
      qmc2SuppressQtMessages = FALSE;
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
    pushButtonRemapJoystickFunction->setEnabled(TRUE);
    pushButtonRemoveJoystickMapping->setEnabled(selItems[0]->text(1).length() > 0);
  } else {
    pushButtonRemapJoystickFunction->setEnabled(FALSE);
    pushButtonRemoveJoystickMapping->setEnabled(FALSE);
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
    pushButtonRemoveJoystickMapping->setEnabled(FALSE);
    QTimer::singleShot(0, this, SLOT(checkJoystickMappings()));
  }
}

void Options::checkJoystickMappings()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Options::checkJoystickMappings()");
#endif

  static QBrush redBrush(QColor(255, 0, 0));
  static QBrush greenBrush(QColor(0, 255, 0));
  static QBrush greyBrush(QColor(128, 128, 128));
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

    bool enabled = qmc2Config->value(QString(QMC2_FRONTEND_PREFIX + "Joystick/%1/Axis%2Enabled").arg(joyIndex).arg(i), TRUE).toBool();
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
  if ( !qmc2Options->groupBoxCalibrationAndTest->layout() )
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
    bool enabled = (state == 0 ? FALSE : TRUE);
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
  int maxRows = MAX(MAX(MAX(myJoystick->numAxes, myJoystick->numButtons), myJoystick->numHats), myJoystick->numTrackballs);

  myLayout = new QGridLayout(this);

  for (i = 0; i < myJoystick->numAxes; i++) {
    int minValue = qmc2Config->value(QString(QMC2_FRONTEND_PREFIX + "Joystick/%1/Axis%2Minimum").arg(joyIndex).arg(i), 0).toInt();
    int maxValue = qmc2Config->value(QString(QMC2_FRONTEND_PREFIX + "Joystick/%1/Axis%2Maximum").arg(joyIndex).arg(i), 0).toInt();
    int dzValue = qmc2Config->value(QString(QMC2_FRONTEND_PREFIX + "Joystick/%1/Axis%2Deadzone").arg(joyIndex).arg(i), 0).toInt();
    myJoystick->deadzones[i] = dzValue;
    int sValue = qmc2Config->value(QString(QMC2_FRONTEND_PREFIX + "Joystick/%1/Axis%2Sensitivity").arg(joyIndex).arg(i), 0).toInt();
    myJoystick->sensitivities[i] = sValue;
    bool enabled = qmc2Config->value(QString(QMC2_FRONTEND_PREFIX + "Joystick/%1/Axis%2Enabled").arg(joyIndex).arg(i), TRUE).toBool();

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
    buttonLabels[i]->setAutoFillBackground(TRUE);
    buttonLabels[i]->setFrameShape(QFrame::Box);
    buttonLabels[i]->setSizePolicy(QSizePolicy::Expanding, myJoystick->numButtons < maxRows ? QSizePolicy::Ignored : QSizePolicy::Minimum);

    myLayout->addWidget(buttonLabels[i], i, 1);
  }

  for (i = 0; i < myJoystick->numHats; i++) {
    hatValueLabels[i] = new QLabel(tr("H%1: 0").arg(i), this);
    hatValueLabels[i]->setToolTip(tr("Current value of hat %1").arg(i));
    hatValueLabels[i]->setAlignment(Qt::AlignCenter);
    hatValueLabels[i]->setAutoFillBackground(TRUE);
    hatValueLabels[i]->setFrameShape(QFrame::Box);
    hatValueLabels[i]->setSizePolicy(QSizePolicy::Expanding, myJoystick->numHats < maxRows ? QSizePolicy::Ignored : QSizePolicy::Minimum);

    myLayout->addWidget(hatValueLabels[i], i, 2);
  }

  for (i = 0; i < myJoystick->numTrackballs; i++) {
    trackballDeltaXLabels[i] = new QLabel(tr("T%1 DX: 0").arg(i), this);
    trackballDeltaXLabels[i]->setToolTip(tr("Current X-delta of trackball %1").arg(i));
    trackballDeltaXLabels[i]->setAlignment(Qt::AlignCenter);
    trackballDeltaXLabels[i]->setAutoFillBackground(TRUE);
    trackballDeltaXLabels[i]->setFrameShape(QFrame::Box);
    trackballDeltaXLabels[i]->setSizePolicy(QSizePolicy::Expanding, myJoystick->numTrackballs < maxRows ? QSizePolicy::Ignored : QSizePolicy::Minimum);

    myLayout->addWidget(trackballDeltaXLabels[i], i, 3);
  }

  for (i = 0; i < myJoystick->numTrackballs; i++) {
    trackballDeltaYLabels[i] = new QLabel(tr("T%1 DY: 0").arg(i), this);
    trackballDeltaYLabels[i]->setToolTip(tr("Current Y-delta of trackball %1").arg(i));
    trackballDeltaYLabels[i]->setAlignment(Qt::AlignCenter);
    trackballDeltaYLabels[i]->setAutoFillBackground(TRUE);
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
  if ( !qmc2Options->groupBoxCalibrationAndTest->layout() )
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
