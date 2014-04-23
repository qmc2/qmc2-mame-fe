#include <Qt>
#if QT_VERSION >= 0x050000
#include <QtWebKitWidgets/QWebView>
#else
#include <QWebView>
#endif
#include <QTextStream>
#include <QHeaderView>
#include <QTreeWidgetItem>
#include <QStringList>
#include <QFile>
#include <QFontMetrics>
#include <QFont>
#include <QTimer>
#include <QMap>
#include <QHash>
#include <QSet>
#include <QDir>
#include <QBitArray>
#include <QByteArray>

#include "gamelist.h"
#include "emuopt.h"
#include "qmc2main.h"
#include "options.h"
#include "preview.h"
#include "flyer.h"
#include "cabinet.h"
#include "controller.h"
#include "marquee.h"
#include "title.h"
#include "pcb.h"
#include "romstatusexport.h"
#include "miniwebbrowser.h"
#include "romalyzer.h"
#include "macros.h"
#include "unzip.h"
#include "sevenzipfile.h"
#if defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
#include "messdevcfg.h"
#endif
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
#include "demomode.h"
#endif
#include "softwarelist.h"
#if defined(QMC2_YOUTUBE_ENABLED)
#include "youtubevideoplayer.h"
#endif
#include "htmleditor/htmleditor.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern Options *qmc2Options;
extern Settings *qmc2Config;
extern EmulatorOptions *qmc2EmulatorOptions;
extern ROMStatusExporter *qmc2ROMStatusExporter;
extern ROMAlyzer *qmc2ROMAlyzer;
extern bool qmc2ReloadActive;
extern bool qmc2EarlyReloadActive;
extern bool qmc2StopParser;
extern bool qmc2StartingUp;
extern bool qmc2VerifyActive;
extern bool qmc2FilterActive;
extern bool qmc2UseIconFile;
extern bool qmc2IconsPreloaded;
extern bool qmc2WidgetsEnabled;
extern bool qmc2StatesTogglesEnabled;
extern int qmc2GamelistResponsiveness;
extern Preview *qmc2Preview;
extern Flyer *qmc2Flyer;
extern Cabinet *qmc2Cabinet;
extern Controller *qmc2Controller;
extern Marquee *qmc2Marquee;
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
extern Title *qmc2Title;
#endif
extern PCB *qmc2PCB;
extern QTreeWidgetItem *qmc2CurrentItem;
extern QTreeWidgetItem *qmc2LastGameInfoItem;
extern QTreeWidgetItem *qmc2LastEmuInfoItem;
extern QTreeWidgetItem *qmc2LastSoftwareListItem;
#if defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
extern QTreeWidgetItem *qmc2LastDeviceConfigItem;
extern MESSDeviceConfigurator *qmc2MESSDeviceConfigurator;
extern QMap<QString, QMap<QString, QStringList> > messSystemSlotMap;
extern QMap<QString, QString> messSlotNameMap;
extern bool messSystemSlotsSupported;
#endif
extern SoftwareList *qmc2SoftwareList;
extern QMap<QString, QStringList> systemSoftwareListMap;
extern QMap<QString, QStringList> systemSoftwareFilterMap;
extern QMap<QString, QString> softwareListXmlDataCache;
extern QString swlBuffer;
extern bool swlSupported;
extern QMap<QString, QTreeWidgetItem *> qmc2GamelistItemMap;
extern QMap<QString, QTreeWidgetItem *> qmc2HierarchyItemMap;
extern QMap<QString, QString> qmc2GamelistNameMap;
extern QMap<QString, QStringList> qmc2HierarchyMap;
extern QMap<QString, QString> qmc2ParentMap;
extern int qmc2SortCriteria;
extern Qt::SortOrder qmc2SortOrder;
extern QBitArray qmc2Filter;
extern QMap<QString, unzFile> qmc2IconFileMap;
extern QMap<QString, SevenZipFile *> qmc2IconFileMap7z;
extern QMap<QString, QIcon> qmc2IconMap;
extern QHash<QString, QByteArray *> qmc2EmuInfoDB;
extern QTreeWidgetItem *qmc2LastMAWSItem;
extern MiniWebBrowser *qmc2MAWSLookup;
extern QMap<QString, QTreeWidgetItem *> qmc2CategoryItemMap;
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
extern QMap<QString, QTreeWidgetItem *> qmc2VersionItemMap;
extern DemoModeDialog *qmc2DemoModeDialog;
#endif
#if defined(QMC2_YOUTUBE_ENABLED)
extern YouTubeVideoPlayer *qmc2YouTubeWidget;
extern QTreeWidgetItem *qmc2LastYouTubeItem;
#endif
extern HtmlEditor *qmc2SystemNotesEditor;
extern HtmlEditor *qmc2SoftwareNotesEditor;
extern QList<QTreeWidgetItem *> qmc2ExpandedGamelistItems;
extern Gamelist *qmc2Gamelist;

// local global variables
QStringList Gamelist::phraseTranslatorList;
QStringList Gamelist::romTypeNames;
QMap<QString, QString> Gamelist::reverseTranslation;

Gamelist::Gamelist(QObject *parent)
  : QObject(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::Gamelist()");
#endif

  numGames = numTotalGames = numCorrectGames = numMostlyCorrectGames = numIncorrectGames = numUnknownGames = numNotFoundGames = numDevices = -1;
  uncommittedXmlDbRows = cachedGamesCounter = numTaggedSets = numSearchGames = numVerifyRoms = 0;
  loadProc = verifyProc = NULL;
  checkedItem = NULL;
  emulatorVersion = tr("unknown");
  mergeCategories = autoRomCheck = verifyCurrentOnly = dtdBufferReady = false;

  QString imgDir = qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/DataDirectory", "data/").toString() + "img/";
  qmc2UnknownImageIcon.addFile(imgDir + "sphere_blue.png");
  qmc2UnknownBIOSImageIcon.addFile(imgDir + "sphere_blue_bios.png");
  qmc2UnknownDeviceImageIcon.addFile(imgDir + "sphere_blue_device.png");
  qmc2CorrectImageIcon.addFile(imgDir + "sphere_green.png");
  qmc2CorrectBIOSImageIcon.addFile(imgDir + "sphere_green_bios.png");
  qmc2CorrectDeviceImageIcon.addFile(imgDir + "sphere_green_device.png");
  qmc2MostlyCorrectImageIcon.addFile(imgDir + "sphere_yellowgreen.png");
  qmc2MostlyCorrectBIOSImageIcon.addFile(imgDir + "sphere_yellowgreen_bios.png");
  qmc2MostlyCorrectDeviceImageIcon.addFile(imgDir + "sphere_yellowgreen_device.png");
  qmc2IncorrectImageIcon.addFile(imgDir + "sphere_red.png");
  qmc2IncorrectBIOSImageIcon.addFile(imgDir + "sphere_red_bios.png");
  qmc2IncorrectDeviceImageIcon.addFile(imgDir + "sphere_red_device.png");
  qmc2NotFoundImageIcon.addFile(imgDir + "sphere_grey.png");
  qmc2NotFoundBIOSImageIcon.addFile(imgDir + "sphere_grey_bios.png");
  qmc2NotFoundDeviceImageIcon.addFile(imgDir + "sphere_grey_device.png");

  if ( phraseTranslatorList.isEmpty() ) {
    phraseTranslatorList << tr("good") << tr("bad") << tr("preliminary") << tr("supported") << tr("unsupported")
                         << tr("imperfect") << tr("yes") << tr("no") << tr("baddump") << tr("nodump")
                         << tr("vertical") << tr("horizontal") << tr("raster") << tr("unknown") << tr("Unknown") 
                         << tr("On") << tr("Off") << tr("audio") << tr("unused") << tr("Unused") << tr("cpu")
                         << tr("vector") << tr("lcd") << tr("joy4way") << tr("joy8way") << tr("trackball")
                         << tr("joy2way") << tr("doublejoy8way") << tr("dial") << tr("paddle") << tr("pedal")
                         << tr("stick") << tr("vjoy2way") << tr("lightgun") << tr("doublejoy4way") << tr("vdoublejoy2way")
                         << tr("doublejoy2way") << tr("printer") << tr("cdrom") << tr("cartridge") << tr("cassette")
                         << tr("quickload") << tr("floppydisk") << tr("serial") << tr("snapshot") << tr("original")
			 << tr("compatible");
    reverseTranslation[tr("good")] = "good";
    reverseTranslation[tr("bad")] = "bad";
    reverseTranslation[tr("preliminary")] = "preliminary";
    reverseTranslation[tr("supported")] = "supported";
    reverseTranslation[tr("unsupported")] = "unsupported";
    reverseTranslation[tr("imperfect")] = "imperfect";
    reverseTranslation[QObject::tr("yes")] = "yes";
    reverseTranslation[QObject::tr("no")] = "no";
    reverseTranslation[QObject::tr("partially")] = "partially";
  }

  if ( romTypeNames.isEmpty() )
	  romTypeNames << "--" << tr("ROM") << tr("CHD") << tr("ROM, CHD");

#if defined(QMC2_EMUTYPE_MAME)
  emulatorIdentifiers << "MAME" << "M.A.M.E." << "HB.M.A.M.E.";
#elif defined(QMC2_EMUTYPE_MESS)
  emulatorIdentifiers << "MESS" << "M.E.S.S.";
#elif defined(QMC2_EMUTYPE_UME)
  emulatorIdentifiers << "UME" << "U.M.E.";
#endif

  if ( QMC2_ICON_FILETYPE_ZIP ) {
	  foreach (QString filePath, qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/IconFile").toString().split(";", QString::SkipEmptyParts)) {
		  unzFile iconFile = unzOpen(filePath.toLocal8Bit());
		  if ( iconFile == NULL )
			  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open icon file, please check access permissions for %1").arg(filePath));
		  else
			  qmc2IconFileMap[filePath] = iconFile;
	  }
  } else if ( QMC2_ICON_FILETYPE_7Z ) {
	  foreach (QString filePath, qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/IconFile").toString().split(";", QString::SkipEmptyParts)) {
		  SevenZipFile *iconFile = new SevenZipFile(filePath);
		  if ( !iconFile->open() ) {
			  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open icon file %1").arg(filePath) + " - " + tr("7z error") + ": " + iconFile->lastError());
			  delete iconFile;
		  } else
			  qmc2IconFileMap7z[filePath] = iconFile;
	  }
  }

  m_xmlDb = new XmlDatabaseManager(this);
  m_userDataDb = new UserDataDatabaseManager(this);
}

Gamelist::~Gamelist()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::~Gamelist()");
#endif

	if ( loadProc )
		loadProc->kill();

	if ( verifyProc )
		verifyProc->kill();

	clearCategoryNames();
	categoryMap.clear();
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
	clearVersionNames();
	versionMap.clear();
#endif

	foreach (unzFile iconFile, qmc2IconFileMap)
		unzClose(iconFile);

	foreach (SevenZipFile *iconFile, qmc2IconFileMap7z) {
		iconFile->close();
		delete iconFile;
	}

	delete m_xmlDb;
	delete m_userDataDb;
}

void Gamelist::enableWidgets(bool enable)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::enableWidgets(bool enable = " + QString(enable ? "true" : "false") + ")");
#endif

  // store widget enablement flag for later dialog setups
  qmc2WidgetsEnabled = enable;

  qmc2Options->toolButtonBrowseStyleSheet->setEnabled(enable);
  qmc2Options->toolButtonBrowseFont->setEnabled(enable);
  qmc2Options->toolButtonBrowseLogFont->setEnabled(enable);
  qmc2Options->toolButtonBrowseTemporaryFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseFrontendLogFile->setEnabled(enable);
  qmc2Options->toolButtonBrowsePreviewDirectory->setEnabled(enable);
  qmc2Options->toolButtonBrowsePreviewFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseDataDirectory->setEnabled(enable);
  qmc2Options->toolButtonBrowseMameHistoryDat->setEnabled(enable);
  qmc2Options->toolButtonBrowseMessSysinfoDat->setEnabled(enable);
  qmc2Options->toolButtonCompressMameHistoryDat->setEnabled(enable);
  qmc2Options->toolButtonCompressMessSysinfoDat->setEnabled(enable);
  qmc2Options->checkBoxProcessMameHistoryDat->setEnabled(enable);
  qmc2Options->checkBoxProcessMessSysinfoDat->setEnabled(enable);
  qmc2Options->toolButtonBrowseMameInfoDat->setEnabled(enable);
  qmc2Options->toolButtonBrowseMessInfoDat->setEnabled(enable);
  qmc2Options->toolButtonCompressMameInfoDat->setEnabled(enable);
  qmc2Options->toolButtonCompressMessInfoDat->setEnabled(enable);
  qmc2Options->checkBoxProcessMameInfoDat->setEnabled(enable);
  qmc2Options->checkBoxProcessMessInfoDat->setEnabled(enable);
  qmc2Options->toolButtonBrowseSoftwareInfoDB->setEnabled(enable);
  qmc2Options->toolButtonCompressSoftwareInfoDB->setEnabled(enable);
  qmc2Options->checkBoxProcessSoftwareInfoDB->setEnabled(enable);
#if defined(QMC2_EMUTYPE_MAME)
  qmc2Options->toolButtonBrowseMAWSCacheDirectory->setEnabled(enable);
#endif
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
  qmc2Options->toolButtonBrowseCatverIniFile->setEnabled(enable);
  qmc2Options->checkBoxUseCatverIni->setEnabled(enable);
#endif
#if defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
  qmc2Options->toolButtonBrowseCategoryIniFile->setEnabled(enable);
  qmc2Options->checkBoxUseCategoryIni->setEnabled(enable);
#endif
  qmc2Options->checkBoxShowROMStatusIcons->setEnabled(enable);
  qmc2Options->checkBoxRomStateFilter->setEnabled(enable);
  qmc2Options->checkBoxShowBiosSets->setEnabled(enable);
  qmc2Options->checkBoxShowDeviceSets->setEnabled(enable);
  qmc2Options->toolButtonBrowseSoftwareListCache->setEnabled(enable);
  qmc2Options->toolButtonBrowseSoftwareStateCache->setEnabled(enable);
#if defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
  qmc2Options->toolButtonBrowseGeneralSoftwareFolder->setEnabled(enable);
#endif
  qmc2Options->toolButtonBrowseExecutableFile->setEnabled(enable);
#if defined(QMC2_VARIANT_LAUNCHER) && defined(QMC2_OS_WIN)
  qmc2Options->toolButtonBrowseMAMEVariantExe->setEnabled(enable);
  qmc2Options->toolButtonBrowseMESSVariantExe->setEnabled(enable);
  qmc2Options->toolButtonBrowseUMEVariantExe->setEnabled(enable);
#endif
  qmc2Options->lineEditExecutableFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseWorkingDirectory->setEnabled(enable);
  qmc2Options->toolButtonBrowseEmulatorLogFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseOptionsTemplateFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseXmlCacheDatabase->setEnabled(enable);
  qmc2Options->toolButtonBrowseUserDataDatabase->setEnabled(enable);
  qmc2Options->toolButtonCleanupUserDataDatabase->setEnabled(enable);
  qmc2Options->toolButtonClearUserDataDatabase->setEnabled(enable);
  qmc2Options->toolButtonBrowseFavoritesFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseHistoryFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseGamelistCacheFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseROMStateCacheFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseSlotInfoCacheFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseFlyerDirectory->setEnabled(enable);
  qmc2Options->toolButtonBrowseFlyerFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseIconDirectory->setEnabled(enable);
  qmc2Options->toolButtonBrowseIconFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseCabinetDirectory->setEnabled(enable);
  qmc2Options->toolButtonBrowseCabinetFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseControllerDirectory->setEnabled(enable);
  qmc2Options->toolButtonBrowseControllerFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseMarqueeDirectory->setEnabled(enable);
  qmc2Options->toolButtonBrowseMarqueeFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseTitleDirectory->setEnabled(enable);
  qmc2Options->toolButtonBrowseTitleFile->setEnabled(enable);
  qmc2Options->toolButtonBrowsePCBDirectory->setEnabled(enable);
  qmc2Options->toolButtonBrowsePCBFile->setEnabled(enable);
  qmc2Options->comboBoxIconFileType->setEnabled(enable);
  qmc2Options->comboBoxPreviewFileType->setEnabled(enable);
  qmc2Options->comboBoxFlyerFileType->setEnabled(enable);
  qmc2Options->comboBoxCabinetFileType->setEnabled(enable);
  qmc2Options->comboBoxControllerFileType->setEnabled(enable);
  qmc2Options->comboBoxMarqueeFileType->setEnabled(enable);
  qmc2Options->comboBoxTitleFileType->setEnabled(enable);
  qmc2Options->comboBoxPCBFileType->setEnabled(enable);
  qmc2Options->comboBoxSoftwareSnapFileType->setEnabled(enable);
  qmc2Options->toolButtonBrowseSoftwareSnapDirectory->setEnabled(enable);
  qmc2Options->toolButtonBrowseSoftwareSnapFile->setEnabled(enable);
  qmc2Options->toolButtonBrowseSoftwareNotesFolder->setEnabled(enable);
  qmc2Options->toolButtonBrowseSoftwareNotesTemplate->setEnabled(enable);
  qmc2Options->toolButtonBrowseSystemNotesFolder->setEnabled(enable);
  qmc2Options->toolButtonBrowseSystemNotesTemplate->setEnabled(enable);
  qmc2Options->toolButtonShowC->setEnabled(enable);
  qmc2Options->toolButtonShowM->setEnabled(enable);
  qmc2Options->toolButtonShowI->setEnabled(enable);
  qmc2Options->toolButtonShowN->setEnabled(enable);
  qmc2Options->toolButtonShowU->setEnabled(enable);
  qmc2Options->comboBoxSortCriteria->setEnabled(enable);
  qmc2Options->comboBoxSortOrder->setEnabled(enable);
  qmc2Options->treeWidgetShortcuts->clearSelection();
  qmc2Options->treeWidgetShortcuts->setEnabled(enable);
  qmc2Options->treeWidgetJoystickMappings->clearSelection();
  qmc2Options->treeWidgetJoystickMappings->setEnabled(enable);
  qmc2Options->toolButtonBrowseCookieDatabase->setEnabled(enable);
  qmc2Options->pushButtonManageCookies->setEnabled(enable ? qmc2Options->checkBoxRestoreCookies->isChecked() : false);
  qmc2Options->toolButtonBrowseZipTool->setEnabled(enable);
  qmc2Options->toolButtonBrowseSevenZipTool->setEnabled(enable);
  qmc2Options->toolButtonBrowseRomTool->setEnabled(enable);
  qmc2Options->toolButtonBrowseRomToolWorkingDirectory->setEnabled(enable);
  qmc2Options->toolButtonBrowseAdditionalEmulatorExecutable->setEnabled(enable);
  qmc2Options->toolButtonBrowseAdditionalEmulatorWorkingDirectory->setEnabled(enable);
  qmc2Options->pushButtonCustomizeToolBar->setEnabled(enable);
  qmc2Options->checkBoxStandardColorPalette->setEnabled(enable);
  qmc2Options->pushButtonEditPalette->setEnabled(enable && !qmc2Options->checkBoxStandardColorPalette->isChecked());
  qmc2Options->pushButtonAdditionalArtworkSetup->setEnabled(enable);
  qmc2Options->pushButtonImageFormats->setEnabled(enable);
  for (int row = 0; row < qmc2Options->tableWidgetRegisteredEmulators->rowCount(); row++) {
	  QWidget *w = qmc2Options->tableWidgetRegisteredEmulators->cellWidget(row, QMC2_ADDTLEMUS_COLUMN_CUID);
	  if ( w ) w->setEnabled(enable);
  }
#if QMC2_USE_PHONON_API
  qmc2MainWindow->toolButtonAudioAddTracks->setEnabled(enable);
  qmc2MainWindow->toolButtonAudioAddURL->setEnabled(enable);
#endif
  if ( qmc2ROMStatusExporter )
    qmc2ROMStatusExporter->pushButtonExport->setEnabled(enable);
  if ( qmc2ROMAlyzer ) {
    qmc2ROMAlyzer->pushButtonAnalyze->setEnabled(enable);
    qmc2ROMAlyzer->toolButtonToolsMenu->setEnabled(enable);
    if ( qmc2ROMAlyzer->groupBoxCHDManager->isChecked() ) {
      qmc2ROMAlyzer->toolButtonBrowseCHDManagerExecutableFile->setEnabled(enable);
      qmc2ROMAlyzer->toolButtonBrowseTemporaryWorkingDirectory->setEnabled(enable);
    }
    if ( qmc2ROMAlyzer->groupBoxSetRewriter->isChecked() )
      qmc2ROMAlyzer->toolButtonBrowseSetRewriterOutputPath->setEnabled(enable);
      qmc2ROMAlyzer->toolButtonBrowseSetRewriterAdditionalRomPath->setEnabled(enable);
#if defined(QMC2_DATABASE_ENABLED)
    if ( qmc2ROMAlyzer->groupBoxDatabase->isChecked() )
      qmc2ROMAlyzer->toolButtonBrowseDatabaseOutputPath->setEnabled(enable);
#endif
  } else {
    qmc2MainWindow->actionROMAlyzer->setEnabled(enable);
    qmc2MainWindow->actionAnalyseCurrentROM->setEnabled(enable);
    qmc2MainWindow->actionAnalyseROMTagged->setEnabled(enable);
    foreach (QAction *action, qmc2MainWindow->criticalActions)
	    action->setEnabled(enable);
  }
  qmc2MainWindow->pushButtonSelectRomFilter->setEnabled(enable);
  qmc2MainWindow->actionLaunchArcade->setEnabled(enable);
  qmc2MainWindow->actionArcadeSetup->setEnabled(enable);
}

void Gamelist::load()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::load()");
#endif

  QString userScopePath = QMC2_DYNAMIC_DOT_PATH;

  QString gameName;
  if ( qmc2CurrentItem && qmc2CurrentItem->child(0) )
	  gameName = qmc2CurrentItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON);

#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
  if ( qmc2DemoModeDialog )
	  qmc2DemoModeDialog->saveCategoryFilter();
#endif

  qmc2ReloadActive = qmc2EarlyReloadActive = true;
  qmc2StopParser = false;
  gameStatusHash.clear();
  qmc2GamelistItemMap.clear();
  qmc2GamelistNameMap.clear();
  biosSets.clear();
  deviceSets.clear();
  qmc2HierarchyItemMap.clear();
  qmc2ExpandedGamelistItems.clear();

  enableWidgets(false);

  qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);

  numGames = numTotalGames = numCorrectGames = numMostlyCorrectGames = numIncorrectGames = numUnknownGames = numNotFoundGames = numDevices = -1;
  numTaggedSets = numSearchGames = 0;
  qmc2MainWindow->treeWidgetGamelist->clear();
  qmc2MainWindow->treeWidgetHierarchy->clear();
  qmc2CategoryItemMap.clear();
  qmc2MainWindow->treeWidgetCategoryView->clear();
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
  qmc2VersionItemMap.clear();
  qmc2MainWindow->treeWidgetVersionView->clear();
#endif
  qmc2MainWindow->listWidgetSearch->clear();
  qmc2MainWindow->listWidgetFavorites->clear();
  qmc2MainWindow->listWidgetPlayed->clear();
  qmc2MainWindow->textBrowserGameInfo->clear();
  qmc2MainWindow->textBrowserEmuInfo->clear();
  qmc2MainWindow->labelGameStatus->setPalette(MainWindow::qmc2StatusColorBlue);
  qmc2CurrentItem = NULL;
#if defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
  if ( qmc2MESSDeviceConfigurator ) {
    qmc2MESSDeviceConfigurator->save();
    qmc2MESSDeviceConfigurator->setVisible(false);
    QLayout *vbl = qmc2MainWindow->tabDevices->layout();
    if ( vbl ) delete vbl;
    delete qmc2MESSDeviceConfigurator;
    qmc2MESSDeviceConfigurator = NULL;
  }
  qmc2LastDeviceConfigItem = NULL;
  messSystemSlotsSupported = true;
  messSystemSlotMap.clear();
  messSlotNameMap.clear();
#endif
  if ( qmc2SystemNotesEditor ) {
	  qmc2SystemNotesEditor->save();
	  qmc2SystemNotesEditor->closeXmlBuffer();
	  qmc2SystemNotesEditor->clearContent();
  }

  if ( qmc2SoftwareNotesEditor ) {
	  qmc2SoftwareNotesEditor->save();
	  qmc2SoftwareNotesEditor->closeXmlBuffer();
	  qmc2SoftwareNotesEditor->clearContent();
  }

  if ( qmc2SoftwareList ) {
    if ( qmc2SoftwareList->isLoading ) {
      qmc2SoftwareList->interruptLoad = true;
      qmc2LastSoftwareListItem = NULL;
      QTimer::singleShot(0, this, SLOT(load()));
      return;
    }
    qmc2SoftwareList->save();
    qmc2SoftwareList->setVisible(false);
    QLayout *vbl = qmc2MainWindow->tabSoftwareList->layout();
    if ( vbl ) delete vbl;
    delete qmc2SoftwareList;
    qmc2SoftwareList = NULL;
  }
  qmc2LastSoftwareListItem = NULL;
  swlSupported = true;
  systemSoftwareListMap.clear();
  systemSoftwareFilterMap.clear();
  softwareListXmlDataCache.clear();
  swlBuffer.clear();
  qmc2LastGameInfoItem = NULL;
  qmc2LastEmuInfoItem = NULL;
  if ( qmc2MAWSLookup ) {
    qmc2MAWSLookup->setVisible(false);
    QLayout *vbl = qmc2MainWindow->tabMAWS->layout();
    if ( vbl ) delete vbl;
    delete qmc2MAWSLookup;
    qmc2MAWSLookup = NULL;
  }
  qmc2LastMAWSItem = NULL;

#if defined(QMC2_YOUTUBE_ENABLED)
  qmc2LastYouTubeItem = NULL;
  if ( qmc2YouTubeWidget ) {
      qmc2YouTubeWidget->setVisible(false);
      QLayout *vbl = qmc2MainWindow->tabYouTube->layout();
      if ( vbl ) delete vbl;
      delete qmc2YouTubeWidget;
      qmc2YouTubeWidget = NULL;
  }
#endif

  qmc2Preview->update();
  qmc2Flyer->update();
  qmc2Cabinet->update();
  qmc2Controller->update();
  qmc2Marquee->update();
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
  qmc2Title->update();
#endif
  qmc2PCB->update();

  qApp->processEvents();

  QTreeWidgetItem *dummyItem = new QTreeWidgetItem(qmc2MainWindow->treeWidgetGamelist);
  dummyItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Waiting for data..."));
  dummyItem = new QTreeWidgetItem(qmc2MainWindow->treeWidgetHierarchy);
  dummyItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Waiting for data..."));
  dummyItem = new QTreeWidgetItem(qmc2MainWindow->treeWidgetCategoryView);
  dummyItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Waiting for data..."));
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
  dummyItem = new QTreeWidgetItem(qmc2MainWindow->treeWidgetVersionView);
  dummyItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Waiting for data..."));
#endif
  if ( qmc2EmulatorOptions ) {
    qmc2EmulatorOptions->save();
    QLayout *vbl = qmc2MainWindow->tabConfiguration->layout();
    if ( vbl ) delete vbl;
    delete qmc2MainWindow->labelEmuSelector;
    if ( !gameName.isEmpty() ) {
	    QString selectedEmulator = qmc2MainWindow->comboBoxEmuSelector->currentText();
	    if ( selectedEmulator == tr("Default") || selectedEmulator.isEmpty() )
		    qmc2Config->remove(QString(QMC2_EMULATOR_PREFIX + "Configuration/%1/SelectedEmulator").arg(gameName));
	    else
		    qmc2Config->setValue(QString(QMC2_EMULATOR_PREFIX + "Configuration/%1/SelectedEmulator").arg(gameName), selectedEmulator);
    }
    delete qmc2MainWindow->comboBoxEmuSelector;
    qmc2MainWindow->comboBoxEmuSelector = NULL;
    delete qmc2EmulatorOptions;
    delete qmc2MainWindow->pushButtonCurrentEmulatorOptionsExportToFile;
    delete qmc2MainWindow->pushButtonCurrentEmulatorOptionsImportFromFile;
    qmc2EmulatorOptions = NULL;
  }
  qmc2MainWindow->labelGamelistStatus->setText(status());

  if ( qmc2MainWindow->tabWidgetGamelist->currentIndex() == QMC2_GAMELIST_INDEX ) {
	switch ( qmc2MainWindow->stackedWidgetView->currentIndex() ) {
		case QMC2_VIEW_CATEGORY_INDEX:
			QTimer::singleShot(0, qmc2MainWindow, SLOT(viewByCategory()));
			break;
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
		case QMC2_VIEW_VERSION_INDEX:
			QTimer::singleShot(0, qmc2MainWindow, SLOT(viewByVersion()));
			break;
#endif
		default:
			break;
	}
  }

  // determine emulator version and supported sets
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("determining emulator version and supported sets"));

  QStringList args;
  QTime elapsedTime(0, 0, 0, 0);
  parseTimer.start();
  QString command;

  // emulator version
  QProcess commandProc;
#if defined(QMC2_SDLMAME)
  commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmame.tmp").toString());
#elif defined(QMC2_MAME)
  commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mame.tmp").toString());
#elif defined(QMC2_SDLMESS)
  commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmess.tmp").toString());
#elif defined(QMC2_MESS)
  commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mess.tmp").toString());
#elif defined(QMC2_SDLUME)
  commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlume.tmp").toString());
#elif defined(QMC2_UME)
  commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-ume.tmp").toString());
#else
  commandProc.setStandardOutputFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-unknown.tmp").toString());
#endif

#if !defined(QMC2_OS_WIN)
  commandProc.setStandardErrorFile("/dev/null");
#endif

  args << "-help";
  qApp->processEvents();
  bool commandProcStarted = false;
  int retries = 0;
  QString execFile = qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ExecutableFile").toString();
  QFileInfo fi(execFile);
  bool started = false;
  if ( fi.exists() && fi.isReadable() ) {
	  commandProc.start(execFile, args);
	  started = commandProc.waitForStarted(QMC2_PROCESS_POLL_TIME);
	  while ( !started && retries++ < QMC2_PROCESS_POLL_RETRIES ) {
		  qApp->processEvents();
		  started = commandProc.waitForStarted(QMC2_PROCESS_POLL_TIME_LONG);
	  }
	  if ( started ) {
		  commandProcStarted = true;
		  bool commandProcRunning = (commandProc.state() == QProcess::Running);
		  while ( !commandProc.waitForFinished(QMC2_PROCESS_POLL_TIME) && commandProcRunning ) {
			  qApp->processEvents();
			  commandProcRunning = (commandProc.state() == QProcess::Running);
		  }
	  } else {
#if defined(QMC2_EMUTYPE_MAME)
		  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't start MAME executable within a reasonable time frame, giving up"));
#elif defined(QMC2_EMUTYPE_MESS)
		  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't start MESS executable within a reasonable time frame, giving up"));
#elif defined(QMC2_EMUTYPE_UME)
		  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't start UME executable within a reasonable time frame, giving up"));
#endif
		  qmc2ReloadActive = qmc2EarlyReloadActive = false;
		  qmc2StopParser = true;
		  enableWidgets(true);
		  return;
	  }
  } else {
	  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't start %1 executable, file '%2' does not exist").arg(QMC2_EMU_NAME).arg(execFile));
	  qmc2ReloadActive = qmc2EarlyReloadActive = false;
	  qmc2StopParser = true;
	  enableWidgets(true);
	  return;
  }

#if defined(QMC2_SDLMAME)
  QFile qmc2TempVersion(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmame.tmp").toString());
#elif defined(QMC2_MAME)
  QFile qmc2TempVersion(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mame.tmp").toString());
#elif defined(QMC2_SDLMESS)
  QFile qmc2TempVersion(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmess.tmp").toString());
#elif defined(QMC2_MESS)
  QFile qmc2TempVersion(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mess.tmp").toString());
#elif defined(QMC2_SDLUME)
  QFile qmc2TempVersion(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlume.tmp").toString());
#elif defined(QMC2_UME)
  QFile qmc2TempVersion(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-ume.tmp").toString());
#else
  QFile qmc2TempVersion(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-unknown.tmp").toString());
#endif
  if ( commandProcStarted && qmc2TempVersion.open(QFile::ReadOnly) ) {
    QTextStream ts(&qmc2TempVersion);
    qApp->processEvents();
    QString s = ts.readAll();
    qApp->processEvents();
    qmc2TempVersion.close();
    qmc2TempVersion.remove();
#if defined(QMC2_OS_WIN)
    s.replace("\r\n", "\n"); // convert WinDOS's "0x0D 0x0A" to just "0x0A" 
#endif
    QStringList versionLines = s.split("\n");
#if defined(QMC2_EMUTYPE_MAME)
    QStringList versionWords = versionLines.first().split(" ");
    if ( versionWords.count() > 1 ) {
      if ( emulatorIdentifiers.contains(versionWords[0]) ) {
        emulatorVersion = versionWords[1].remove("v");
        emulatorType = "MAME";
      } else {
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: selected executable file is not MAME"));
        emulatorVersion = tr("unknown");
        emulatorType = versionWords[0];
      }
    } else {
      emulatorVersion = tr("unknown");
      emulatorType = tr("unknown");
    }
#elif defined(QMC2_EMUTYPE_MESS)
    QStringList versionWords = versionLines.first().split(" ");
    if ( versionWords.count() > 1 ) {
      if ( emulatorIdentifiers.contains(versionWords[0]) ) {
        emulatorVersion = versionWords[1].remove("v");
        emulatorType = "MESS";
      } else {
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: selected executable file is not MESS"));
        emulatorVersion = tr("unknown");
        emulatorType = versionWords[0];
      }
    } else {
      emulatorVersion = tr("unknown");
      emulatorType = tr("unknown");
    }
#elif defined(QMC2_EMUTYPE_UME)
    QStringList versionWords = versionLines.first().split(" ");
    if ( versionWords.count() > 1 ) {
      if ( emulatorIdentifiers.contains(versionWords[0]) ) {
        emulatorVersion = versionWords[1].remove("v");
        emulatorType = "UME";
      } else {
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: selected executable file is not UME"));
        emulatorVersion = tr("unknown");
        emulatorType = versionWords[0];
      }
    } else {
      emulatorVersion = tr("unknown");
      emulatorType = tr("unknown");
    }
#endif
  } else {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't create temporary file, please check emulator executable and permissions"));
    emulatorVersion = tr("unknown");
    emulatorType = tr("unknown");
  }

  // supported games/machines
  args.clear();
  args << "-listfull";
  qApp->processEvents();
  commandProcStarted = false;
  retries = 0;
  commandProc.start(execFile, args);
  started = commandProc.waitForStarted(QMC2_PROCESS_POLL_TIME);
  while ( !started && retries++ < QMC2_PROCESS_POLL_RETRIES ) {
    qApp->processEvents();
    started = commandProc.waitForStarted(QMC2_PROCESS_POLL_TIME_LONG);
  }
  if ( started ) {
    commandProcStarted = true;
    bool commandProcRunning = (commandProc.state() == QProcess::Running);
    while ( !commandProc.waitForFinished(QMC2_PROCESS_POLL_TIME) && commandProcRunning ) {
      qApp->processEvents();
      commandProcRunning = (commandProc.state() == QProcess::Running);
    }
  } else {
#if defined(QMC2_EMUTYPE_MAME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't start MAME executable within a reasonable time frame, giving up"));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't start MESS executable within a reasonable time frame, giving up"));
#elif defined(QMC2_EMUTYPE_UME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't start UME executable within a reasonable time frame, giving up"));
#endif
    qmc2ReloadActive = qmc2EarlyReloadActive = false;
    qmc2StopParser = true;
    return;
  }

#if defined(QMC2_SDLMAME)
  QFile qmc2Temp(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmame.tmp").toString());
#elif defined(QMC2_MAME)
  QFile qmc2Temp(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mame.tmp").toString());
#elif defined(QMC2_SDLMESS)
  QFile qmc2Temp(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlmess.tmp").toString());
#elif defined(QMC2_MESS)
  QFile qmc2Temp(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-mess.tmp").toString());
#elif defined(QMC2_SDLUME)
  QFile qmc2Temp(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-sdlume.tmp").toString());
#elif defined(QMC2_UME)
  QFile qmc2Temp(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-ume.tmp").toString());
#else
  QFile qmc2Temp(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/TemporaryFile", userScopePath + "/qmc2-unknown.tmp").toString());
#endif
  if ( commandProcStarted && qmc2Temp.open(QFile::ReadOnly) ) {
    QTextStream ts(&qmc2Temp);
    qApp->processEvents();
    numTotalGames = ts.readAll().count("\n") - 1;
    qmc2Temp.close();
    qmc2Temp.remove();
    qApp->processEvents();
    elapsedTime = elapsedTime.addMSecs(parseTimer.elapsed());
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (determining emulator version and supported sets, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
  } else {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't create temporary file, please check emulator executable and permissions"));
  }
  qmc2MainWindow->labelGamelistStatus->setText(status());

  if ( emulatorVersion != tr("unknown") )
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("emulator info: type = %1, version = %2").arg(emulatorType).arg(emulatorVersion));
  else {
    if ( emulatorType == tr("unknown") )
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: couldn't determine emulator type and version"));
    else
#if defined(QMC2_EMUTYPE_MAME)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: couldn't determine emulator version, type identification string is '%1' -- please inform developers if you're sure that this is a valid MAME binary"));
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: couldn't determine emulator version, type identification string is '%1' -- please inform developers if you're sure that this is a valid MESS binary"));
#elif defined(QMC2_EMUTYPE_UME)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: couldn't determine emulator version, type identification string is '%1' -- please inform developers if you're sure that this is a valid UME binary"));
#else
      ;
#endif
    qmc2ReloadActive = false;
    enableWidgets(true);
    return;
  }

  if ( numTotalGames > 0 )
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("%n supported (non-device) set(s)", "", numTotalGames));
  else {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: couldn't determine the number of supported sets"));
    qmc2ReloadActive = false;
    enableWidgets(true);
    return;
  }

  categoryMap.clear();
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
  versionMap.clear();
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/UseCatverIni").toBool() ) {
    loadCatverIni();
    mergeCategories = true;
  }
#endif
#if defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/UseCategoryIni").toBool() )
    loadCategoryIni();
#endif
  mergeCategories = false;

#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
  if ( qmc2DemoModeDialog )
	  QTimer::singleShot(0, qmc2DemoModeDialog, SLOT(updateCategoryFilter()));
#endif

  bool xmlCacheOkay = (emulatorVersion == xmlDb()->emulatorVersion() && xmlDb()->xmlRowCount() > 0);
  qmc2EarlyReloadActive = false;

  if ( qmc2StopParser ) {
    qmc2MainWindow->progressBarGamelist->reset();
    qmc2ReloadActive = false;
    enableWidgets(true);
    return;
  }

  // hide game list
  qmc2MainWindow->treeWidgetGamelist->setVisible(false);
  qmc2MainWindow->labelLoadingGamelist->setVisible(true);
  qmc2MainWindow->treeWidgetHierarchy->setVisible(false);
  qmc2MainWindow->labelLoadingHierarchy->setVisible(true);
  qApp->processEvents();

  if ( xmlCacheOkay ) {
    parse();
    loadFavorites();
    loadPlayHistory();

    // show game list
    qmc2MainWindow->labelLoadingGamelist->setVisible(false);
    qmc2MainWindow->treeWidgetGamelist->setVisible(true);
    qmc2MainWindow->labelLoadingHierarchy->setVisible(false);
    qmc2MainWindow->treeWidgetHierarchy->setVisible(true);

    if ( qmc2MainWindow->tabWidgetGamelist->currentIndex() == QMC2_GAMELIST_INDEX ) {
	    if ( qApp->focusWidget() != qmc2MainWindow->comboBoxToolbarSearch ) {
		    switch ( qmc2MainWindow->stackedWidgetView->currentIndex() ) {
			    case QMC2_VIEW_DETAIL_INDEX:
				    qmc2MainWindow->treeWidgetGamelist->setFocus();
				    break;
			    case QMC2_VIEW_TREE_INDEX:
				    qmc2MainWindow->treeWidgetHierarchy->setFocus();
				    break;
			    case QMC2_VIEW_CATEGORY_INDEX:
				    qmc2MainWindow->treeWidgetCategoryView->setFocus();
				    break;
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
			    case QMC2_VIEW_VERSION_INDEX:
				    qmc2MainWindow->treeWidgetVersionView->setFocus();
				    break;
#endif
			    default:
				    qmc2MainWindow->treeWidgetGamelist->setFocus();
				    break;
		    }
	    }
	    switch ( qmc2MainWindow->stackedWidgetView->currentIndex() ) {
		    case QMC2_VIEW_DETAIL_INDEX:
			    qmc2MainWindow->treeWidgetGamelist_verticalScrollChanged();
			    break;
		    case QMC2_VIEW_TREE_INDEX:
			    qmc2MainWindow->treeWidgetHierarchy_verticalScrollChanged();
			    break;
		    case QMC2_VIEW_CATEGORY_INDEX:
			    qmc2MainWindow->treeWidgetCategoryView_verticalScrollChanged();
			    break;
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
		    case QMC2_VIEW_VERSION_INDEX:
			    qmc2MainWindow->treeWidgetVersionView_verticalScrollChanged();
			    break;
#endif
	    }
    }

    qApp->processEvents();
  } else {
    loadTimer.start();
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading XML data and recreating cache"));
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
      qmc2MainWindow->progressBarGamelist->setFormat(tr("XML data - %p%"));
    else
      qmc2MainWindow->progressBarGamelist->setFormat("%p%");
    command = qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ExecutableFile").toString();
    args.clear();
    args << "-listxml";

    uncommittedXmlDbRows = 0;
    dtdBufferReady = false;
    xmlLineBuffer.clear();
    xmlDb()->setLogActive(false);
    xmlDb()->recreateDatabase();
    xmlDb()->setLogActive(true);
    xmlDb()->setEmulatorVersion(emulatorVersion);
    xmlDb()->setQmc2Version(XSTR(QMC2_VERSION));
    xmlDb()->setXmlCacheVersion(QMC2_XMLCACHE_VERSION);

    loadProc = new QProcess(this);
    connect(loadProc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(loadError(QProcess::ProcessError)));
    connect(loadProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(loadFinished(int, QProcess::ExitStatus)));
    connect(loadProc, SIGNAL(readyReadStandardOutput()), this, SLOT(loadReadyReadStandardOutput()));
    connect(loadProc, SIGNAL(started()), this, SLOT(loadStarted()));
    connect(loadProc, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(loadStateChanged(QProcess::ProcessState)));
    loadProc->setProcessChannelMode(QProcess::MergedChannels);
    loadProc->start(command, args);
  }

  userDataDb()->setEmulatorVersion(emulatorVersion);
  userDataDb()->setQmc2Version(XSTR(QMC2_VERSION));
  userDataDb()->setUserDataVersion(QMC2_USERDATA_VERSION);
}

void Gamelist::verify(bool currentOnly)
{
  if ( currentOnly )
    if ( !qmc2CurrentItem )
      return;

#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Gamelist::verify(bool currentOnly = %1)").arg(currentOnly));
#endif

  verifyCurrentOnly = currentOnly;
  qmc2VerifyActive = true;
  qmc2StopParser = false;

  enableWidgets(false);

  verifiedList.clear();
  verifyLastLine.clear();
  verifyTimer.start();
  numVerifyRoms = 0;
  if ( verifyCurrentOnly ) {
    checkedItem = qmc2CurrentItem;
    romCache.setFileName(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ROMStateCacheFile").toString());
    romCache.open(QIODevice::WriteOnly | QIODevice::Text);
    if ( !romCache.isOpen() ) {
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: can't open ROM state cache for writing, path = %1").arg(romCache.fileName()));
      qmc2VerifyActive = false;
      enableWidgets(true);
      return;
    } else {
      tsRomCache.setDevice(&romCache);
      tsRomCache.reset();
      tsRomCache << "# THIS FILE IS AUTO-GENERATED - PLEASE DO NOT EDIT!\n";
    }
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("verifying ROM status for '%1'").arg(checkedItem->text(QMC2_GAMELIST_COLUMN_GAME)));
    oldRomState = gameStatusHash[checkedItem->text(QMC2_GAMELIST_COLUMN_NAME)];
    // decrease counter for current game's/machine's state
    switch ( oldRomState ) {
      case 'C':
        numCorrectGames--;
        numUnknownGames++;
        break;

      case 'M':
        numMostlyCorrectGames--;
        numUnknownGames++;
        break;

      case 'I':
        numIncorrectGames--;
        numUnknownGames++;
        break;

      case 'N':
        numNotFoundGames--;
        numUnknownGames++;
        break;

      case 'U':
      default:
        break;
    }
  } else {
    checkedItem = NULL;
    romCache.setFileName(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ROMStateCacheFile").toString());
    romCache.open(QIODevice::WriteOnly | QIODevice::Text);
    if ( !romCache.isOpen() ) {
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: can't open ROM state cache for writing, path = %1").arg(romCache.fileName()));
      qmc2VerifyActive = false;
      enableWidgets(true);
      return;
    } else {
      tsRomCache.setDevice(&romCache);
      tsRomCache.reset();
      tsRomCache << "# THIS FILE IS AUTO-GENERATED - PLEASE DO NOT EDIT!\n";
    }
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("verifying ROM status for all sets"));
    numCorrectGames = numMostlyCorrectGames = numIncorrectGames = numNotFoundGames = numUnknownGames = 0;
    qmc2MainWindow->labelGamelistStatus->setText(status());
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
      qmc2MainWindow->progressBarGamelist->setFormat(tr("ROM check - %p%"));
    else
      qmc2MainWindow->progressBarGamelist->setFormat("%p%");
    qmc2MainWindow->progressBarGamelist->setRange(0, numTotalGames + numDevices);
    qmc2MainWindow->progressBarGamelist->reset();
  }
  
  QStringList args;
  QString command = qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ExecutableFile").toString();
  if ( qmc2Config->contains(QMC2_EMULATOR_PREFIX + "Configuration/Global/rompath") )
    args << "-rompath" << qmc2Config->value(QMC2_EMULATOR_PREFIX + "Configuration/Global/rompath").toString().replace("~", "$HOME");
  args << "-verifyroms";
  if ( verifyCurrentOnly )
    args << checkedItem->text(QMC2_GAMELIST_COLUMN_NAME);

  verifyProc = new QProcess(this);
  connect(verifyProc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(verifyError(QProcess::ProcessError)));
  connect(verifyProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(verifyFinished(int, QProcess::ExitStatus)));
  connect(verifyProc, SIGNAL(readyReadStandardOutput()), this, SLOT(verifyReadyReadStandardOutput()));
  connect(verifyProc, SIGNAL(started()), this, SLOT(verifyStarted()));
  connect(verifyProc, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(verifyStateChanged(QProcess::ProcessState)));
  if ( !qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory", QString()).toString().isEmpty() )
	  verifyProc->setWorkingDirectory(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory").toString());
  verifyProc->setProcessChannelMode(QProcess::MergedChannels);
  verifyProc->start(command, args);
}

QString Gamelist::value(QString element, QString attribute, bool translate)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::value(QString element = " + element + ", QString attribute = \"" + attribute + "\", translate = " + QString(translate ? "true" : "false") + ")");
#endif

  QString attributePattern = " " + attribute + "=\"";
  if ( element.contains(attributePattern) ) {
    QString valueString = element.remove(0, element.indexOf(attributePattern) + attributePattern.length());
    valueString = valueString.remove(valueString.indexOf("\""), valueString.lastIndexOf(">")).replace("&amp;", "&").replace("&lt;", "<").replace("&gt;", ">").replace("&quot;", "\"");
    if ( valueString == ">" )
	    return QString::null;
    if ( translate )
      return tr(valueString.toLocal8Bit());
    else
      return valueString;
  } else
    return QString::null;
}

void Gamelist::insertAttributeItems(QTreeWidgetItem *parent, QString element, QStringList attributes, QStringList descriptions, bool translate)
{
	QList<QTreeWidgetItem *> itemList;
	for (int i = 0; i < attributes.count(); i++) {
		QString valueString = value(element, attributes.at(i), translate);
		if ( !valueString.isEmpty() ) {
			QTreeWidgetItem *attributeItem = new QTreeWidgetItem();
			attributeItem->setText(QMC2_GAMELIST_COLUMN_GAME, descriptions.at(i));
			attributeItem->setText(QMC2_GAMELIST_COLUMN_ICON, tr(valueString.toLocal8Bit()));
			itemList << attributeItem;
		}
	}
	parent->addChildren(itemList);
}

void Gamelist::insertAttributeItems(QList<QTreeWidgetItem *> *itemList, QString element, QStringList attributes, QStringList descriptions, bool translate)
{
	for (int i = 0; i < attributes.count(); i++) {
		QString valueString = value(element, attributes.at(i), translate);
		if ( !valueString.isEmpty() ) {
			QTreeWidgetItem *attributeItem = new QTreeWidgetItem();
			attributeItem->setText(QMC2_GAMELIST_COLUMN_GAME, descriptions.at(i));
			attributeItem->setText(QMC2_GAMELIST_COLUMN_ICON, tr(valueString.toLocal8Bit()));
			itemList->append(attributeItem);
		}
	}
}

void Gamelist::parseGameDetail(QTreeWidgetItem *item)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Gamelist::parseGameDetail(QTreeWidgetItem *item = %1)").arg((qulonglong)item));
#endif

  QString gameName = item->text(QMC2_GAMELIST_COLUMN_NAME);
  QStringList xmlLines = xmlDb()->xml(gameName).split("\n", QString::SkipEmptyParts);
  if ( xmlLines.count() < 2 ) {
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: couldn't find game information for '%1'").arg(gameName));
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: couldn't find machine information for '%1'").arg(gameName));
#endif
      return;
  }

  int gamePos = 1;
  item->child(0)->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Updating"));
  qmc2MainWindow->treeWidgetGamelist->viewport()->repaint();
  qApp->processEvents();

  QString element, content;
  QStringList attributes, descriptions;
  QTreeWidgetItem *childItem = NULL;

  QList<QTreeWidgetItem *> itemList;

  // game/machine element
  attributes << "name" << "sourcefile" << "isbios" << "isdevice" << "runnable" << "cloneof" << "romof" << "sampleof";
  descriptions << tr("Name") << tr("Source file") << tr("Is BIOS?") << tr("Is device?") << tr("Runnable") << tr("Clone of") << tr("ROM of") << tr("Sample of");
  element = xmlLines.at(gamePos - 1).simplified();
  insertAttributeItems(&itemList, element, attributes, descriptions, true);

#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
  QString endMark = "</game>";
#elif defined(QMC2_EMUTYPE_MESS)
  QString endMark = "</machine>";
#else
  QString endMark = "</undefined>";
#endif

  while ( !xmlLines[gamePos].contains(endMark) ) {
    childItem = NULL;
    element = xmlLines[gamePos].simplified();
    if ( element.contains("<year>") ) {
      content = element.remove("<year>").remove("</year>");
      childItem = new QTreeWidgetItem();
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Year"));
      childItem->setText(QMC2_GAMELIST_COLUMN_ICON, content);
    }
    if ( element.contains("<manufacturer>") ) {
      content = element.remove("<manufacturer>").remove("</manufacturer>");
      content.replace("&amp;", "&").replace("&lt;", "<").replace("&gt;", ">").replace("&quot;", "\"");
      childItem = new QTreeWidgetItem();
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Manufacturer"));
      childItem->setText(QMC2_GAMELIST_COLUMN_ICON, content);
    }
    if ( element.contains("<rom ") ) {
      childItem = new QTreeWidgetItem();
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("ROM"));
      childItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(element, "name"));
      attributes.clear();
      attributes << "bios" << "size" << "crc" << "sha1" << "merge" << "region" << "offset" << "status" << "optional";
      descriptions.clear();
      descriptions << tr("BIOS") << tr("Size") << tr("CRC") << tr("SHA1") << tr("Merge") << tr("Region") << tr("Offset") << tr("Status") << tr("Optional");
      insertAttributeItems(childItem, element, attributes, descriptions, true);
    }
    if ( element.contains("<device_ref ") ) {
      childItem = new QTreeWidgetItem();
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Device reference"));
      childItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(element, "name"));
    }
    if ( element.contains("<chip ") ) {
      childItem = new QTreeWidgetItem();
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Chip"));
      childItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(element, "name"));
      attributes.clear();
      attributes << "tag" << "type" << "clock";
      descriptions.clear();
      descriptions << tr("Tag") << tr("Type") << tr("Clock");
      insertAttributeItems(childItem, element, attributes, descriptions, true);
    }
    if ( element.contains("<display ") ) {
      childItem = new QTreeWidgetItem();
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Display"));
      attributes.clear();
      attributes << "type" << "rotate" << "flipx" << "width" << "height" << "refresh" << "pixclock" << "htotal" << "hbend" << "hbstart" << "vtotal" << "vbend" << "vbstart";
      descriptions.clear();
      descriptions << tr("Type") << tr("Rotate") << tr("Flip-X") << tr("Width") << tr("Height") << tr("Refresh") << tr("Pixel clock") << tr("H-Total") << tr("H-Bend") << tr("HB-Start") << tr("V-Total") << tr("V-Bend") << tr("VB-Start");
      insertAttributeItems(childItem, element, attributes, descriptions, true);
    }
    if ( element.contains("<sound ") ) {
      childItem = new QTreeWidgetItem();
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Sound"));
      attributes.clear();
      attributes << "channels";
      descriptions.clear();
      descriptions << tr("Channels");
      insertAttributeItems(childItem, element, attributes, descriptions, true);
    }
    if ( element.contains("<input ") ) {
      childItem = new QTreeWidgetItem();
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Input"));
      attributes.clear();
      attributes << "service" << "tilt" << "players" << "buttons" << "coins";
      descriptions.clear();
      descriptions << tr("Service") << tr("Tilt") << tr("Players") << tr("Buttons") << tr("Coins");
      insertAttributeItems(childItem, element, attributes, descriptions, true);

      gamePos++;
      while ( xmlLines[gamePos].contains("<control ") ) {
        QString subElement = xmlLines[gamePos].simplified();
        QTreeWidgetItem *nextChildItem = new QTreeWidgetItem(childItem);
        nextChildItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Control"));
        nextChildItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(subElement, "type", true));
        attributes.clear();
        attributes << "minimum" << "maximum" << "sensitivity" << "keydelta" << "reverse";
        descriptions.clear();
        descriptions << tr("Minimum") << tr("Maximum") << tr("Sensitivity") << tr("Key Delta") << tr("Reverse");
        insertAttributeItems(nextChildItem, subElement, attributes, descriptions, true);
        gamePos++;
      }
    }
    if ( element.contains("<dipswitch ") ) {
      childItem = new QTreeWidgetItem();
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("DIP switch"));
      childItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(element, "name", true));

      gamePos++;
      while ( xmlLines[gamePos].contains("<dipvalue ") ) {
        QString subElement = xmlLines[gamePos].simplified();
        QTreeWidgetItem *secondChildItem = new QTreeWidgetItem(childItem);
        secondChildItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("DIP value"));
        secondChildItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(subElement, "name", true));
        attributes.clear();
        attributes << "default";
        descriptions.clear();
        descriptions << tr("Default");
        insertAttributeItems(secondChildItem, subElement, attributes, descriptions, true);
        gamePos++;
      }
    }
    if ( element.contains("<configuration ") ) {
      childItem = new QTreeWidgetItem();
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Configuration"));
      childItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(element, "name", true));
      attributes.clear();
      attributes << "tag" << "mask";
      descriptions.clear();
      descriptions << tr("Tag") << tr("Mask");
      insertAttributeItems(childItem, element, attributes, descriptions, true);

      gamePos++;
      while ( xmlLines[gamePos].contains("<confsetting ") ) {
        QString subElement = xmlLines[gamePos].simplified();
        QTreeWidgetItem *secondChildItem = new QTreeWidgetItem(childItem);
        secondChildItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Setting"));
        secondChildItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(subElement, "name", true));
        attributes.clear();
        attributes << "value" << "default";
        descriptions.clear();
        descriptions << tr("Value") << tr("Default");
        insertAttributeItems(secondChildItem, subElement, attributes, descriptions, true);
        gamePos++;
      }
    }
    if ( element.contains("<driver ") ) {
      childItem = new QTreeWidgetItem();
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Driver"));
      attributes.clear();
      attributes << "status" << "emulation" << "color" << "sound" << "graphic" << "cocktail" << "protection" << "savestate" << "palettesize";
      descriptions.clear();
      descriptions << tr("Status") << tr("Emulation") << tr("Color") << tr("Sound") << tr("Graphic") << tr("Cocktail") << tr("Protection") << tr("Save state") << tr("Palette size");
      insertAttributeItems(childItem, element, attributes, descriptions, true);
    }
    if ( element.contains("<biosset ") ) {
      childItem = new QTreeWidgetItem();
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("BIOS set"));
      childItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(element, "name"));
      attributes.clear();
      attributes << "description" << "default";
      descriptions.clear();
      descriptions << tr("Description") << tr("Default");
      insertAttributeItems(childItem, element, attributes, descriptions, true);
    }
    if ( element.contains("<sample ") ) {
      childItem = new QTreeWidgetItem();
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Sample"));
      childItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(element, "name"));
    }
    if ( element.contains("<disk ") ) {
      childItem = new QTreeWidgetItem();
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Disk"));
      childItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(element, "name"));
      attributes.clear();
      attributes << "md5" << "sha1" << "merge" << "region" << "index" << "status" << "optional";
      descriptions.clear();
      descriptions << tr("MD5") << tr("SHA1") << tr("Merge") << tr("Region") << tr("Index") << tr("Status") << tr("Optional");
      insertAttributeItems(childItem, element, attributes, descriptions, true);
    }
    if ( element.contains("<adjuster ") ) {
      childItem = new QTreeWidgetItem();
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Adjuster"));
      childItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(element, "name"));
      attributes.clear();
      attributes << "default";
      descriptions.clear();
      descriptions << tr("Default");
      insertAttributeItems(childItem, element, attributes, descriptions, true);
    }
    if ( element.contains("<softwarelist ") ) {
      childItem = new QTreeWidgetItem();
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Software list"));
      childItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(element, "name"));
      attributes.clear();
      attributes << "status";
      descriptions.clear();
      descriptions << tr("Status");
      insertAttributeItems(childItem, element, attributes, descriptions, true);
    }
    if ( element.contains("<category ") ) {
      childItem = new QTreeWidgetItem();
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Category"));
      childItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(element, "name", true));

      gamePos++;
      while ( xmlLines[gamePos].contains("<item ") ) {
        QString subElement = xmlLines[gamePos].simplified();
        QTreeWidgetItem *secondChildItem = new QTreeWidgetItem(childItem);
        secondChildItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Item"));
        secondChildItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(subElement, "name", true));
        attributes.clear();
        attributes << "default";
        descriptions.clear();
        descriptions << tr("Default");
        insertAttributeItems(secondChildItem, subElement, attributes, descriptions, true);
        gamePos++;
      }
    }
    if ( element.contains("<device ") ) {
      childItem = new QTreeWidgetItem();
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Device"));
      childItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(element, "type", true));
      attributes.clear();
      attributes << "tag" << "mandatory" << "interface";
      descriptions.clear();
      descriptions << tr("Tag") << tr("Mandatory") << tr("Interface");
      insertAttributeItems(childItem, element, attributes, descriptions, false);

      gamePos++;
      while ( xmlLines[gamePos].contains("<instance ") ) {
        QString subElement = xmlLines[gamePos].simplified();
        QTreeWidgetItem *secondChildItem = new QTreeWidgetItem(childItem);
        secondChildItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Instance"));
        secondChildItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(subElement, "name", false));
        attributes.clear();
        attributes << "briefname";
        descriptions.clear();
        descriptions << tr("Brief name");
        insertAttributeItems(secondChildItem, element, attributes, descriptions, false);
        gamePos++;
      }
      while ( xmlLines[gamePos].contains("<extension ") ) {
        QString subElement = xmlLines[gamePos].simplified();
        QTreeWidgetItem *secondChildItem = new QTreeWidgetItem(childItem);
        secondChildItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Extension"));
        secondChildItem->setText(QMC2_GAMELIST_COLUMN_ICON, value(subElement, "name", false));
        gamePos++;
      }
    }
    if ( element.contains("<ramoption") ) {
      childItem = new QTreeWidgetItem();
      childItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("RAM options"));
      while ( xmlLines[gamePos].contains("<ramoption") ) {
        QString subElement = xmlLines[gamePos].simplified();
        QTreeWidgetItem *secondChildItem = new QTreeWidgetItem(childItem);
        secondChildItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Option"));
        int fromIndex = subElement.indexOf('>') + 1;
        int toIndex = subElement.indexOf('<', fromIndex);
        QString ramOptionValue = subElement.mid(fromIndex, toIndex - fromIndex);
        secondChildItem->setText(QMC2_GAMELIST_COLUMN_ICON, ramOptionValue);
        attributes.clear();
        attributes << "default";
        descriptions.clear();
        descriptions << tr("Default");
        insertAttributeItems(secondChildItem, subElement, attributes, descriptions, false);
        gamePos++;
      }
      if ( xmlLines[gamePos].contains(endMark) )
	      gamePos--;
    }
    gamePos++;
    if ( childItem )
	    itemList << childItem;
  }

  qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(false);
  delete item->takeChild(0);
  item->addChildren(itemList);
  qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(true);
}

void Gamelist::parse()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::parse()");
#endif

  if ( qmc2StopParser ) {
    qmc2ReloadActive = false;
    enableWidgets(true);
    return;
  }

  bool showROMStatusIcons = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowROMStatusIcons", true).toBool();
  bool showDeviceSets = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowDeviceSets", true).toBool();
  bool showBiosSets = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowBiosSets", true).toBool();

  QTime elapsedTime(0, 0, 0, 0);
  qmc2MainWindow->progressBarGamelist->setRange(0, numTotalGames);
  romCache.setFileName(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ROMStateCacheFile").toString());
  romCache.open(QIODevice::ReadOnly | QIODevice::Text);
  if ( !romCache.isOpen() ) {
    if ( !autoRomCheck )
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: can't open ROM state cache, please check ROMs"));
  } else {
    parseTimer.start();
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading ROM state from cache"));
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
      qmc2MainWindow->progressBarGamelist->setFormat(tr("ROM states - %p%"));
    else
      qmc2MainWindow->progressBarGamelist->setFormat("%p%");
    qmc2MainWindow->progressBarGamelist->reset();
    qApp->processEvents();
    tsRomCache.setDevice(&romCache);
    tsRomCache.reset();
    cachedGamesCounter = 0;
    while ( !tsRomCache.atEnd() ) {
      QString line = tsRomCache.readLine();
      if ( !line.isNull() && !line.startsWith("#") ) {
        QStringList words = line.split(" ");
        gameStatusHash[words[0]] = words[1].at(0).toLatin1();
        cachedGamesCounter++;
      }
      if ( cachedGamesCounter % QMC2_ROMCACHE_RESPONSIVENESS == 0 ) {
        qmc2MainWindow->progressBarGamelist->setValue(cachedGamesCounter);
        qApp->processEvents();
      }
    }
    numCorrectGames = numMostlyCorrectGames = numIncorrectGames = numNotFoundGames = 0;
    elapsedTime = elapsedTime.addMSecs(parseTimer.elapsed());
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading ROM state from cache, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("%n cached ROM state(s) loaded", "", cachedGamesCounter));

    romCache.close();
    qApp->processEvents();
  }

  QTime processGamelistElapsedTimer(0, 0, 0, 0);
  parseTimer.start();
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("processing game list"));
#elif defined(QMC2_EMUTYPE_MESS)
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("processing machine list"));
#endif
  qmc2MainWindow->treeWidgetGamelist->clear();
  qmc2HierarchyMap.clear();
  qmc2ParentMap.clear();
  qmc2MainWindow->progressBarGamelist->reset();

  gamelistCache.setFileName(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/GamelistCacheFile").toString());
  gamelistCache.open(QIODevice::ReadOnly | QIODevice::Text);
  bool reparseGamelist = true;
  bool romStateCacheUpdate = false;

  if ( gamelistCache.isOpen() ) {
    QString line;
    tsGamelistCache.setDevice(&gamelistCache);
    tsGamelistCache.setCodec(QTextCodec::codecForName("UTF-8"));
    tsGamelistCache.seek(0);
    
    if ( !tsGamelistCache.atEnd() ) {
      line = tsGamelistCache.readLine();
      while ( line.startsWith("#") && !tsGamelistCache.atEnd() )
        line = tsGamelistCache.readLine();
      QStringList words = line.split("\t");
      if ( words.count() >= 2 ) {
#if defined(QMC2_EMUTYPE_MAME)
        if ( words[0] == "MAME_VERSION" ) {
#elif defined(QMC2_EMUTYPE_MESS)
        if ( words[0] == "MESS_VERSION" ) {
#elif defined(QMC2_EMUTYPE_UME)
        if ( words[0] == "UME_VERSION" ) {
#endif
	  romStateCacheUpdate = reparseGamelist = (words[1] != emulatorVersion);
        }
      } else {
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: couldn't determine emulator version of game list cache"));
#elif defined(QMC2_EMUTYPE_MESS)
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: couldn't determine emulator version of machine list cache"));
#endif
      }
      if ( words.count() < 4 ) {
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("INFORMATION: the game list cache will now be updated due to a new format"));
#elif defined(QMC2_EMUTYPE_MESS)
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("INFORMATION: the machine list cache will now be updated due to a new format"));
#endif
	reparseGamelist = true;
      } else {
	      int cacheGlcVersion = words[3].toInt();
	      if ( cacheGlcVersion < QMC2_GLC_VERSION ) {
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
		      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("INFORMATION: the game list cache will now be updated due to a new format"));
#elif defined(QMC2_EMUTYPE_MESS)
		      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("INFORMATION: the machine list cache will now be updated due to a new format"));
#endif
		      reparseGamelist = true;
	      }
      }
    }

#if defined(QMC2_EMUTYPE_MAME)
    bool useCatverIni = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/UseCatverIni").toBool();
    bool useCategoryIni = false;
#elif defined(QMC2_EMUTYPE_UME)
    bool useCatverIni = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/UseCatverIni").toBool();
    bool useCategoryIni = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/UseCategoryIni").toBool();
#elif defined(QMC2_EMUTYPE_MESS)
    bool useCatverIni = false;
    bool useCategoryIni = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/UseCategoryIni").toBool();
#endif

    if ( !reparseGamelist && !qmc2StopParser ) {
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading game data from game list cache"));
      if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
        qmc2MainWindow->progressBarGamelist->setFormat(tr("Game data - %p%"));
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading machine data from machine list cache"));
      if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
        qmc2MainWindow->progressBarGamelist->setFormat(tr("Machine data - %p%"));
#endif
      else
        qmc2MainWindow->progressBarGamelist->setFormat("%p%");
      QTime gameDataCacheElapsedTime(0, 0, 0, 0);
      miscTimer.start();
      numGames = numUnknownGames = numDevices = 0;
      qmc2MainWindow->progressBarGamelist->setValue(0);
      QString readBuffer;
      QList<QTreeWidgetItem *> itemList;
      QList <QTreeWidgetItem *> hideList;
      while ( (!tsGamelistCache.atEnd() || !readBuffer.isEmpty() ) && !qmc2StopParser ) {
        readBuffer += tsGamelistCache.read(QMC2_FILE_BUFFER_SIZE);
        bool endsWithNewLine = readBuffer.endsWith("\n");
        QStringList lines = readBuffer.split("\n");
        int lc = lines.count();
        if ( !endsWithNewLine )
          lc -= 1;
        for (int l = 0; l < lc; l++) {
          line = lines[l];
          if ( !line.isEmpty() && !line.startsWith("#") ) {
            QStringList words = line.split("\t");
            QString gameName = words[0];
            QString gameDescription = words[1];
            QString gameManufacturer = words[2];
            QString gameYear = words[3];
            QString gameCloneOf = words[4];
            bool isBIOS = (words[5] == "1");
            bool hasROMs = (words[6] == "1");
            bool hasCHDs = (words[7] == "1");
            QString gamePlayers = words[8];
            QString gameStatus = words[9];
            bool isDevice = (words[10] == "1");
	    QString gameSource = words[11];

#ifdef QMC2_DEBUG
            qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Gamelist::parse(): gameName = %1, gameDescription = %2, gameManufacturer = %3, gameYear = %4, gameCloneOf = %5, isBIOS = %6, hasROMs = %7, hasCHDs = %8, gamePlayers = %9, gameStatus = %10, isDevice = %11, gameSource = %12").
                            arg(gameName).arg(gameDescription).arg(gameManufacturer).arg(gameYear).arg(gameCloneOf).arg(isBIOS).arg(hasROMs).arg(hasCHDs).arg(gamePlayers).arg(gameStatus).arg(isDevice).arg(gameSource));
#endif

            GamelistItem *gameDescriptionItem = new GamelistItem();
            gameDescriptionItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
            gameDescriptionItem->setCheckState(QMC2_GAMELIST_COLUMN_TAG, Qt::Unchecked);

	    if ( (isBIOS && !showBiosSets) || (isDevice && !showDeviceSets) )
		    hideList << gameDescriptionItem;

            if ( !gameCloneOf.isEmpty() )
              qmc2HierarchyMap[gameCloneOf].append(gameName);
            else if ( !qmc2HierarchyMap.contains(gameName) )
              qmc2HierarchyMap.insert(gameName, QStringList());

            gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_GAME, gameDescription);
            gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_YEAR, gameYear);
            gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_MANU, gameManufacturer);
            gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_NAME, gameName);
            gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_SRCFILE, gameSource);
	    gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_RTYPES, romTypeNames[int(hasROMs) + int(hasCHDs) * 2]);
            if ( isDevice ) {
              if ( gamePlayers != tr("?") )
	        gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_PLAYERS, gamePlayers);
              else
	        gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_PLAYERS, tr("N/A"));
	      gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_DRVSTAT, tr("N/A"));
            } else {
	      gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_PLAYERS, gamePlayers);
	      gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_DRVSTAT, tr(gameStatus.toLocal8Bit()));
            }
            if ( useCatverIni || useCategoryIni ) {
              QString *categoryString = categoryMap[gameName];
              gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_CATEGORY, categoryString ? *categoryString : tr("Unknown"));
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
              QString *versionString = versionMap[gameName];
              gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_VERSION, versionString ? *versionString : tr("?"));
#endif
            }
            switch ( gameStatusHash[gameName] ) {
              case 'C': 
                numCorrectGames++;
                if ( isBIOS ) {
			if ( showROMStatusIcons )
				gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
			biosSets << gameName;
		} else if ( isDevice ) {
			if ( showROMStatusIcons )
				gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectDeviceImageIcon);
			deviceSets << gameName;
		} else if ( showROMStatusIcons )
			gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
                break;

              case 'M': 
                numMostlyCorrectGames++;
                if ( isBIOS ) {
			if ( showROMStatusIcons )
				gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectBIOSImageIcon);
			biosSets << gameName;
		} else if ( isDevice ) {
			if ( showROMStatusIcons )
				gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectDeviceImageIcon);
			deviceSets << gameName;
		} else if ( showROMStatusIcons )
			gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectImageIcon);
                break;

              case 'I':
                numIncorrectGames++;
                if ( isBIOS ) {
			if ( showROMStatusIcons )
				gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectBIOSImageIcon);
			biosSets << gameName;
		} else if ( isDevice ) {
			if ( showROMStatusIcons )
				gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectDeviceImageIcon);
			deviceSets << gameName;
		} else if ( showROMStatusIcons )
			gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectImageIcon);
                break;

              case 'N':
                numNotFoundGames++;
                if ( isBIOS ) {
			if ( showROMStatusIcons )
				gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundBIOSImageIcon);
			biosSets << gameName;
		} else if ( isDevice ) {
			if ( showROMStatusIcons )
				gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundDeviceImageIcon);
			deviceSets << gameName;
		} else if ( showROMStatusIcons )
			gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundImageIcon);
                break;

              default:
                numUnknownGames++;
                gameStatusHash[gameName] = 'U';
                if ( isBIOS ) {
			if ( showROMStatusIcons )
				gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownBIOSImageIcon);
			biosSets << gameName;
		} else if ( isDevice ) {
			if ( showROMStatusIcons )
				gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownDeviceImageIcon);
			deviceSets << gameName;
		} else if ( showROMStatusIcons )
			gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownImageIcon);
                break;
            }

            QTreeWidgetItem *nameItem = new QTreeWidgetItem(gameDescriptionItem);
            nameItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Waiting for data..."));
            nameItem->setText(QMC2_GAMELIST_COLUMN_ICON, gameName);
            qmc2GamelistItemMap[gameName] = gameDescriptionItem;
            qmc2GamelistNameMap[gameDescription] = gameName;

            loadIcon(gameName, gameDescriptionItem);

            numGames++;
            if ( isDevice ) numDevices++;

            itemList << gameDescriptionItem;
          }

          if ( numGames % qmc2GamelistResponsiveness == 0 ) {
            qmc2MainWindow->progressBarGamelist->setValue(numGames);
            qmc2MainWindow->labelGamelistStatus->setText(status());
            qApp->processEvents();
          }
        }

        if ( endsWithNewLine )
          readBuffer.clear();
        else
          readBuffer = lines.last();
      }
      qmc2MainWindow->treeWidgetGamelist->addTopLevelItems(itemList);
      foreach (QTreeWidgetItem *hiddenItem, hideList)
	      hiddenItem->setHidden(true);
      qmc2MainWindow->progressBarGamelist->setValue(numGames);
      qApp->processEvents();

      gameDataCacheElapsedTime = gameDataCacheElapsedTime.addMSecs(miscTimer.elapsed());
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading game data from game list cache, elapsed time = %1)").arg(gameDataCacheElapsedTime.toString("mm:ss.zzz")));
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading machine data from machine list cache, elapsed time = %1)").arg(gameDataCacheElapsedTime.toString("mm:ss.zzz")));
#endif
    }
  } 

  if ( gamelistCache.isOpen() )
    gamelistCache.close();

  if ( reparseGamelist && !qmc2StopParser ) {
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("parsing game data and (re)creating game list cache"));
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
      qmc2MainWindow->progressBarGamelist->setFormat(tr("Game data - %p%"));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("parsing machine data and (re)creating machine list cache"));
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
      qmc2MainWindow->progressBarGamelist->setFormat(tr("Machine data - %p%"));
#endif
    else
      qmc2MainWindow->progressBarGamelist->setFormat("%p%");
    gamelistCache.open(QIODevice::WriteOnly | QIODevice::Text);
    if ( !gamelistCache.isOpen() ) {
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: can't open game list cache for writing, path = %1").arg(gamelistCache.fileName()));
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: can't open machine list cache for writing, path = %1").arg(gamelistCache.fileName()));
#endif
    } else {
      tsGamelistCache.setDevice(&gamelistCache);
      tsGamelistCache.setCodec(QTextCodec::codecForName("UTF-8"));
      tsGamelistCache.reset();
      tsGamelistCache << "# THIS FILE IS AUTO-GENERATED - PLEASE DO NOT EDIT!\n";
#if defined(QMC2_EMUTYPE_MAME)
      tsGamelistCache << "MAME_VERSION\t" + emulatorVersion + "\tGLC_VERSION\t" + QString::number(QMC2_GLC_VERSION) + "\n";
#elif defined(QMC2_EMUTYPE_MESS)
      tsGamelistCache << "MESS_VERSION\t" + emulatorVersion + "\tGLC_VERSION\t" + QString::number(QMC2_GLC_VERSION) + "\n";
#elif defined(QMC2_EMUTYPE_UME)
      tsGamelistCache << "UME_VERSION\t" + emulatorVersion + "\tGLC_VERSION\t" + QString::number(QMC2_GLC_VERSION) + "\n";
#endif
    }

#if defined(QMC2_EMUTYPE_MAME)
    bool useCatverIni = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/UseCatverIni").toBool();
    bool useCategoryIni = false;
#elif defined(QMC2_EMUTYPE_UME)
    bool useCatverIni = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/UseCatverIni").toBool();
    bool useCategoryIni = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/UseCategoryIni").toBool();
#elif defined(QMC2_EMUTYPE_MESS)
    bool useCatverIni = false;
    bool useCategoryIni = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/UseCategoryIni").toBool();
#endif

    // parse XML data
    numGames = numUnknownGames = numDevices = 0;
    bool endParser = qmc2StopParser;
    qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(false);

    QList <QTreeWidgetItem *> itemList;
    QList <QTreeWidgetItem *> hideList;
    int xmlRowCount = xmlDb()->xmlRowCount();
    for (int rowCounter = 1; rowCounter <= xmlRowCount && !qmc2StopParser; rowCounter++) {
      QStringList xmlLines = xmlDb()->xml(rowCounter).split("\n", QString::SkipEmptyParts);
      endParser = qmc2StopParser;
      for (int lineCounter = 0; lineCounter < xmlLines.count() && !endParser; lineCounter++) {
	      while ( lineCounter < xmlLines.count() && !xmlLines[lineCounter].contains("<description>") )
		      lineCounter++;
	      if ( !endParser && lineCounter < xmlLines.count() ) {
		QString descriptionElement = xmlLines[lineCounter].simplified();
		QString gameElement = xmlLines[lineCounter - 1].simplified();
		if ( !gameElement.contains(" name=\"") )
			continue;
		bool isBIOS = ( value(gameElement, "isbios") == "yes" );
		bool isDevice = ( value(gameElement, "isdevice") == "yes" );
		QString gameName = value(gameElement, "name");
		QString gameSource = value(gameElement, "sourcefile");
		if ( gameName.isEmpty() ) {
#if defined(QMC2_EMUTYPE_MAME)
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: name attribute empty on XML line %1 (set will be ignored!) -- please inform MAME developers and include the offending output from -listxml").arg(lineCounter + 2));
#else
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: name attribute empty on XML line %1 (set will be ignored!) -- please inform MESS developers and include the offending output from -listxml").arg(lineCounter + 2));
#endif
			qApp->processEvents();
			continue;
		}
		QString gameCloneOf = value(gameElement, "cloneof");
		QString gameDescription = descriptionElement.remove("<description>").remove("</description>").replace("&amp;", "&").replace("&lt;", "<").replace("&gt;", ">").replace("&quot;", "\"");
		GamelistItem *gameDescriptionItem = new GamelistItem();
		gameDescriptionItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
		gameDescriptionItem->setCheckState(QMC2_GAMELIST_COLUMN_TAG, Qt::Unchecked);

		if ( (isBIOS && !showBiosSets) || (isDevice && !showDeviceSets) )
			hideList << gameDescriptionItem;

		// find year & manufacturer and determine ROM/CHD requirements
		bool endGame = false;
		int i = lineCounter;
		QString gameYear = tr("?"), gameManufacturer = tr("?"), gamePlayers = tr("?"), gameStatus = tr("?");
		bool yearFound = false, manufacturerFound = false, hasROMs = false, hasCHDs = false, playersFound = false, statusFound = false;
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
		QString endMark = "</game>";
#elif defined(QMC2_EMUTYPE_MESS)
		QString endMark = "</machine>";
#else
		QString endMark = "</undefined>";
#endif
		while ( !endGame ) {
		  QString xmlLine = xmlLines[i];
		  if ( xmlLine.contains("<year>") ) {
		    gameYear = xmlLine.simplified().remove("<year>").remove("</year>");
		    yearFound = true;
		  } else if ( xmlLine.contains("<manufacturer>") ) {
		    gameManufacturer = xmlLine.simplified().remove("<manufacturer>").remove("</manufacturer>").replace("&amp;", "&").replace("&lt;", "<").replace("&gt;", ">").replace("&quot;", "\"");
		    manufacturerFound = true;
		  } else if ( xmlLine.contains("<rom name") ) {
		    hasROMs = true;
		  } else if ( xmlLine.contains("<disk name") ) {
		    hasCHDs = true;
		  } else if ( xmlLine.contains("<input players") ) {
		    int playersPos = xmlLine.indexOf("input players=\"") + 15;
		    if ( playersPos >= 0 ) {
		      gamePlayers = xmlLine.mid(playersPos, xmlLine.indexOf("\"", playersPos) - playersPos);
		      playersFound = true;
		    }
		  } else if ( xmlLine.contains("<driver status") ) {
		    int statusPos = xmlLine.indexOf("driver status=\"") + 15;
		    if ( statusPos >= 0 ) {
		      gameStatus = xmlLine.mid(statusPos, xmlLine.indexOf("\"", statusPos) - statusPos);
		      statusFound = true;
		    }
		  }
		  endGame = xmlLine.contains(endMark) || (yearFound && manufacturerFound && hasROMs && hasCHDs && playersFound && statusFound);
		  i++;
		}

		if ( !gameCloneOf.isEmpty() )
		  qmc2HierarchyMap[gameCloneOf].append(gameName);
		else if ( !qmc2HierarchyMap.contains(gameName) )
		  qmc2HierarchyMap.insert(gameName, QStringList());

		gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_GAME, gameDescription);
		gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_YEAR, gameYear);
		gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_MANU, gameManufacturer);
		gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_NAME, gameName);
		gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_SRCFILE, gameSource);
		gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_RTYPES, romTypeNames[int(hasROMs) + int(hasCHDs) * 2]);
		if ( isDevice ) {
		  if ( gamePlayers != tr("?") )
		    gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_PLAYERS, gamePlayers);
		  else
		    gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_PLAYERS, tr("N/A"));
		  gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_DRVSTAT, tr("N/A"));
		} else {
		  gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_PLAYERS, gamePlayers);
		  gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_DRVSTAT, tr(gameStatus.toLocal8Bit()));
		}
		if ( useCatverIni || useCategoryIni ) {
		  QString *categoryString = categoryMap[gameName];
		  gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_CATEGORY, categoryString ? *categoryString : tr("Unknown"));
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
		  QString *versionString = versionMap[gameName];
		  gameDescriptionItem->setText(QMC2_GAMELIST_COLUMN_VERSION, versionString ? *versionString : tr("?"));
#endif
		}
		switch ( gameStatusHash[gameName] ) {
		  case 'C': 
		    numCorrectGames++;
		    if ( isBIOS ) {
			    if ( showROMStatusIcons )
				    gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
			    biosSets << gameName;
		    } else if ( isDevice ) {
			    if ( showROMStatusIcons )
				    gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectDeviceImageIcon);
			    deviceSets << gameName;
		    } else if ( showROMStatusIcons )
			    gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
		    break;

		  case 'M': 
		    numMostlyCorrectGames++;
		    if ( isBIOS ) {
			    if ( showROMStatusIcons )
				    gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectBIOSImageIcon);
			    biosSets << gameName;
		    } else if ( isDevice ) {
			    if ( showROMStatusIcons )
				    gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectDeviceImageIcon);
			    deviceSets << gameName;
		    } else if ( showROMStatusIcons )
			    gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectImageIcon);
		    break;

		  case 'I':
		    numIncorrectGames++;
		    if ( isBIOS ) {
			    if ( showROMStatusIcons )
				    gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectBIOSImageIcon);
			    biosSets << gameName;
		    } else if ( isDevice ) {
			    if ( showROMStatusIcons )
				    gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectDeviceImageIcon);
			    deviceSets << gameName;
		    } else if ( showROMStatusIcons )
			    gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectImageIcon);
		    break;

		  case 'N':
		    numNotFoundGames++;
		    if ( isBIOS ) {
			    if ( showROMStatusIcons )
				    gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundBIOSImageIcon);
			    biosSets << gameName;
		    } else if ( isDevice ) {
			    if ( showROMStatusIcons )
				    gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundDeviceImageIcon);
			    deviceSets << gameName;
		    } else if ( showROMStatusIcons )
			    gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundImageIcon);
		    break;

		  default:
		    numUnknownGames++;
		    gameStatusHash[gameName] = 'U';
		    if ( isBIOS ) {
			    if ( showROMStatusIcons )
				    gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownBIOSImageIcon);
			    biosSets << gameName;
		    } else if ( isDevice ) {
			    if ( showROMStatusIcons )
				    gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownDeviceImageIcon);
			    deviceSets << gameName;
		    } else if ( showROMStatusIcons )
			    gameDescriptionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownImageIcon);
		    break;
		}

		QTreeWidgetItem *nameItem = new QTreeWidgetItem(gameDescriptionItem);
		nameItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Waiting for data..."));
		nameItem->setText(QMC2_GAMELIST_COLUMN_ICON, gameName);
		qmc2GamelistItemMap[gameName] = gameDescriptionItem;
		qmc2GamelistNameMap[gameDescription] = gameName;
		loadIcon(gameName, gameDescriptionItem);

		if ( gamelistCache.isOpen() )
			tsGamelistCache << gameName << "\t" << gameDescription << "\t" << gameManufacturer << "\t"
					<< gameYear << "\t" << gameCloneOf << "\t" << (isBIOS ? "1": "0") << "\t"
					<< (hasROMs ? "1" : "0") << "\t" << (hasCHDs ? "1": "0") << "\t"
					<< gamePlayers << "\t" << gameStatus << "\t" << (isDevice ? "1": "0") << "\t"
					<< gameSource <<"\n";

		numGames++;
		if ( isDevice ) numDevices++;

		itemList << gameDescriptionItem;
	      }

	      if ( numGames % qmc2GamelistResponsiveness == 0 ) {
		qmc2MainWindow->progressBarGamelist->setValue(numGames);
		qmc2MainWindow->labelGamelistStatus->setText(status());
		qApp->processEvents();
	      }
      }
    }
    qmc2MainWindow->treeWidgetGamelist->addTopLevelItems(itemList);
    foreach (QTreeWidgetItem *hiddenItem, hideList)
	    hiddenItem->setHidden(true);
  }

  if ( gamelistCache.isOpen() )
    gamelistCache.close();

#if defined(QMC2_EMUTYPE_MAME)
  bool useCatverIni = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/UseCatverIni").toBool();
  bool useCategoryIni = false;
#elif defined(QMC2_EMUTYPE_UME)
  bool useCatverIni = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/UseCatverIni").toBool();
  bool useCategoryIni = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/UseCategoryIni").toBool();
#elif defined(QMC2_EMUTYPE_MESS)
  bool useCatverIni = false;
  bool useCategoryIni = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/UseCategoryIni").toBool();
#endif

  // create parent/clone hierarchy tree
  qmc2MainWindow->treeWidgetHierarchy->clear();
  QMapIterator<QString, QStringList> i(qmc2HierarchyMap);
  QList<QTreeWidgetItem *> itemList;
  QList<QTreeWidgetItem *> hideList;
  while ( i.hasNext() ) {
    i.next();
    QString iValue = i.key();
    QString iDescription = qmc2GamelistItemMap[iValue]->text(QMC2_GAMELIST_COLUMN_GAME);
    if ( iDescription.isEmpty() )
      continue;
    bool isBIOS = isBios(iValue);
    bool isDevice = this->isDevice(iValue);
    GamelistItem *hierarchyItem = new GamelistItem();
    if ( (isBIOS && !showBiosSets) || (isDevice && !showDeviceSets) )
	    hideList << hierarchyItem;
    hierarchyItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
    hierarchyItem->setCheckState(QMC2_GAMELIST_COLUMN_TAG, Qt::Unchecked);
    hierarchyItem->setText(QMC2_GAMELIST_COLUMN_GAME, iDescription);
    QTreeWidgetItem *baseItem = qmc2GamelistItemMap[iValue];
    hierarchyItem->setText(QMC2_GAMELIST_COLUMN_YEAR, baseItem->text(QMC2_GAMELIST_COLUMN_YEAR));
    hierarchyItem->setText(QMC2_GAMELIST_COLUMN_MANU, baseItem->text(QMC2_GAMELIST_COLUMN_MANU));
    hierarchyItem->setText(QMC2_GAMELIST_COLUMN_NAME, baseItem->text(QMC2_GAMELIST_COLUMN_NAME));
    hierarchyItem->setText(QMC2_GAMELIST_COLUMN_SRCFILE, baseItem->text(QMC2_GAMELIST_COLUMN_SRCFILE));
    hierarchyItem->setText(QMC2_GAMELIST_COLUMN_RTYPES, baseItem->text(QMC2_GAMELIST_COLUMN_RTYPES));
    hierarchyItem->setText(QMC2_GAMELIST_COLUMN_PLAYERS, baseItem->text(QMC2_GAMELIST_COLUMN_PLAYERS));
    hierarchyItem->setText(QMC2_GAMELIST_COLUMN_DRVSTAT, baseItem->text(QMC2_GAMELIST_COLUMN_DRVSTAT));
    if ( useCatverIni || useCategoryIni ) {
      hierarchyItem->setText(QMC2_GAMELIST_COLUMN_CATEGORY, baseItem->text(QMC2_GAMELIST_COLUMN_CATEGORY));
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
      hierarchyItem->setText(QMC2_GAMELIST_COLUMN_VERSION, baseItem->text(QMC2_GAMELIST_COLUMN_VERSION));
#endif
    }
    qmc2HierarchyItemMap[iValue] = hierarchyItem;
    switch ( gameStatusHash[iValue] ) {
      case 'C': 
        if ( isBIOS ) {
          if ( showROMStatusIcons ) hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
        } else if ( isDevice ) {
          if ( showROMStatusIcons ) hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectDeviceImageIcon);
	} else {
          if ( showROMStatusIcons ) hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
	}
        break;

      case 'M': 
        if ( isBIOS ) {
          if ( showROMStatusIcons ) hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectBIOSImageIcon);
        } else if ( isDevice ) {
          if ( showROMStatusIcons ) hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectDeviceImageIcon);
        } else {
          if ( showROMStatusIcons ) hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectImageIcon);
	}
        break;

      case 'I':
        if ( isBIOS ) {
          if ( showROMStatusIcons ) hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectBIOSImageIcon);
        } else if ( isDevice ) {
          if ( showROMStatusIcons ) hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectDeviceImageIcon);
	} else {
          if ( showROMStatusIcons ) hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectImageIcon);
	}
        break;

      case 'N':
        if ( isBIOS ) {
          if ( showROMStatusIcons ) hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundBIOSImageIcon);
        } else if ( isDevice ) {
          if ( showROMStatusIcons ) hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundDeviceImageIcon);
	} else {
          if ( showROMStatusIcons ) hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundImageIcon);
	}
        break;

      case 'U':
      default:
        if ( isBIOS ) {
          if ( showROMStatusIcons ) hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownBIOSImageIcon);
        } else if ( isDevice ) {
          if ( showROMStatusIcons ) hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownDeviceImageIcon);
	} else {
          if ( showROMStatusIcons ) hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownImageIcon);
	}
        break;
    }

    loadIcon(iValue, hierarchyItem);

    int j;
    for (j = 0; j < i.value().count(); j++) {
      QString jValue = i.value().at(j);
      QString jDescription = qmc2GamelistItemMap[jValue]->text(QMC2_GAMELIST_COLUMN_GAME);
      if ( jDescription.isEmpty() )
        continue;
      isBIOS = isBios(jValue);
      isDevice = this->isDevice(jValue);
      GamelistItem *hierarchySubItem = new GamelistItem(hierarchyItem);
      if ( (isBIOS && !showBiosSets) || (isDevice && !showDeviceSets) )
	      hideList << hierarchySubItem;
      hierarchySubItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
      hierarchySubItem->setCheckState(QMC2_GAMELIST_COLUMN_TAG, Qt::Unchecked);
      hierarchySubItem->setText(QMC2_GAMELIST_COLUMN_GAME, jDescription);
      baseItem = qmc2GamelistItemMap[jValue];
      hierarchySubItem->setText(QMC2_GAMELIST_COLUMN_YEAR, baseItem->text(QMC2_GAMELIST_COLUMN_YEAR));
      hierarchySubItem->setText(QMC2_GAMELIST_COLUMN_MANU, baseItem->text(QMC2_GAMELIST_COLUMN_MANU));
      hierarchySubItem->setText(QMC2_GAMELIST_COLUMN_NAME, baseItem->text(QMC2_GAMELIST_COLUMN_NAME));
      hierarchySubItem->setText(QMC2_GAMELIST_COLUMN_SRCFILE, baseItem->text(QMC2_GAMELIST_COLUMN_SRCFILE));
      hierarchySubItem->setText(QMC2_GAMELIST_COLUMN_RTYPES, baseItem->text(QMC2_GAMELIST_COLUMN_RTYPES));
      hierarchySubItem->setText(QMC2_GAMELIST_COLUMN_PLAYERS, baseItem->text(QMC2_GAMELIST_COLUMN_PLAYERS));
      hierarchySubItem->setText(QMC2_GAMELIST_COLUMN_DRVSTAT, baseItem->text(QMC2_GAMELIST_COLUMN_DRVSTAT));
      if ( useCatverIni || useCategoryIni ) {
        hierarchySubItem->setText(QMC2_GAMELIST_COLUMN_CATEGORY, baseItem->text(QMC2_GAMELIST_COLUMN_CATEGORY));
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
        hierarchySubItem->setText(QMC2_GAMELIST_COLUMN_VERSION, baseItem->text(QMC2_GAMELIST_COLUMN_VERSION));
#endif
      }
      qmc2HierarchyItemMap[jValue] = hierarchySubItem;
      qmc2ParentMap[jValue] = iValue;
      // "fill up" emulator info data for clones
      if ( !qmc2EmuInfoDB.isEmpty() ) {
        QByteArray *p = qmc2EmuInfoDB[hierarchyItem->text(QMC2_GAMELIST_COLUMN_NAME)];
        if ( p )
          if ( !qmc2EmuInfoDB.contains(baseItem->text(QMC2_GAMELIST_COLUMN_NAME)) )
            qmc2EmuInfoDB[baseItem->text(QMC2_GAMELIST_COLUMN_NAME)] = p;
      }
      switch ( gameStatusHash[jValue] ) {
        case 'C': 
          if ( isBIOS ) {
            if ( showROMStatusIcons ) hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
          } else if ( isDevice ) {
            if ( showROMStatusIcons ) hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectDeviceImageIcon);
	  } else {
            if ( showROMStatusIcons ) hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
	  }
          break;

        case 'M': 
          if ( isBIOS ) {
            if ( showROMStatusIcons ) hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectBIOSImageIcon);
          } else if ( isDevice ) {
            if ( showROMStatusIcons ) hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectDeviceImageIcon);
	  } else {
            if ( showROMStatusIcons ) hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectImageIcon);
	  }
          break;

        case 'I':
          if ( isBIOS ) {
            if ( showROMStatusIcons ) hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectBIOSImageIcon);
          } else if ( isDevice ) {
            if ( showROMStatusIcons ) hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectDeviceImageIcon);
	  } else {
            if ( showROMStatusIcons ) hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectImageIcon);
	  }
          break;

        case 'N':
          if ( isBIOS ) {
            if ( showROMStatusIcons ) hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundBIOSImageIcon);
          } else if ( isDevice ) {
            if ( showROMStatusIcons ) hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundDeviceImageIcon);
	  } else {
            if ( showROMStatusIcons ) hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundImageIcon);
	  }
          break;

        case 'U':
        default:
          if ( isBIOS ) {
            if ( showROMStatusIcons ) hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownBIOSImageIcon);
          } else if ( isDevice ) {
            if ( showROMStatusIcons ) hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownDeviceImageIcon);
	  } else {
            if ( showROMStatusIcons ) hierarchySubItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownImageIcon);
	  }
          break;
      }

      loadIcon(jValue, hierarchySubItem);
    }

    itemList << hierarchyItem;
  }
  qmc2MainWindow->treeWidgetHierarchy->addTopLevelItems(itemList);
  foreach (QTreeWidgetItem *hiddenItem, hideList)
	  hiddenItem->setHidden(true);

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

  // final update of progress bar and game/machine list stats
  qmc2MainWindow->progressBarGamelist->setValue(qmc2MainWindow->progressBarGamelist->maximum());
  qmc2MainWindow->labelGamelistStatus->setText(status());
  qApp->processEvents();

  // sort the master-list
  qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(false);
  qmc2MainWindow->treeWidgetHierarchy->setUpdatesEnabled(false);
  qmc2MainWindow->treeWidgetGamelist->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
  qmc2MainWindow->treeWidgetHierarchy->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
  qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(true);
  qmc2MainWindow->treeWidgetHierarchy->setUpdatesEnabled(true);

  QTreeWidgetItem *ci = qmc2MainWindow->treeWidgetGamelist->currentItem();
  if ( ci ) {
    if ( ci->isSelected() ) {
      QTimer::singleShot(0, qmc2MainWindow, SLOT(scrollToCurrentItem()));
    } else if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/RestoreGameSelection").toBool() ) {
      QString selectedGame = qmc2Config->value(QMC2_EMULATOR_PREFIX + "SelectedGame", QString()).toString();
      if ( !selectedGame.isEmpty() ) {
	      QTreeWidgetItem *glItem = qmc2GamelistItemMap[selectedGame];
	      if ( !glItem ) { // fallback for old value (game description)
		      QList<QTreeWidgetItem *> il = qmc2MainWindow->treeWidgetGamelist->findItems(selectedGame, Qt::MatchExactly, QMC2_GAMELIST_COLUMN_GAME);
		      if ( !il.isEmpty() )
			      glItem = il[0];
	      }
	      if ( glItem ) {
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
		      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("restoring game selection"));
#elif defined(QMC2_EMUTYPE_MESS)
		      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("restoring machine selection"));
#endif
		      qmc2MainWindow->treeWidgetGamelist->setCurrentItem(glItem);
		      QTimer::singleShot(0, qmc2MainWindow, SLOT(scrollToCurrentItem()));
	      }
      }
    }
  } else if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/RestoreGameSelection").toBool() ) {
    QString selectedGame = qmc2Config->value(QMC2_EMULATOR_PREFIX + "SelectedGame", QString()).toString();
    if ( !selectedGame.isEmpty() ) {
	    QTreeWidgetItem *glItem = qmc2GamelistItemMap[selectedGame];
	    if ( !glItem ) { // fallback for old value (game description)
		    QList<QTreeWidgetItem *> il = qmc2MainWindow->treeWidgetGamelist->findItems(selectedGame, Qt::MatchExactly, QMC2_GAMELIST_COLUMN_GAME);
		    if ( !il.isEmpty() )
			    glItem = il[0];
	    }
	    if ( glItem ) {
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
		    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("restoring game selection"));
#elif defined(QMC2_EMUTYPE_MESS)
		    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("restoring machine selection"));
#endif
		    qmc2MainWindow->treeWidgetGamelist->setCurrentItem(glItem);
		    QTimer::singleShot(0, qmc2MainWindow, SLOT(scrollToCurrentItem()));
	    }
    }
  }
  qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(true);
  qmc2MainWindow->labelGamelistStatus->setText(status());

  processGamelistElapsedTimer = processGamelistElapsedTimer.addMSecs(parseTimer.elapsed());
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (processing game list, elapsed time = %1)").arg(processGamelistElapsedTimer.toString("mm:ss.zzz")));
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("%n game(s)", "", numTotalGames - biosSets.count()) + tr(", %n BIOS set(s)", "", biosSets.count()) + tr(" and %n device(s) loaded", "", numDevices));
#elif defined(QMC2_EMUTYPE_MESS)
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (processing machine list, elapsed time = %1)").arg(processGamelistElapsedTimer.toString("mm:ss.zzz")));
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("%n machine(s)", "", numTotalGames - biosSets.count()) + tr(", %n BIOS set(s)", "", biosSets.count()) + tr(" and %n device(s) loaded", "", numDevices));
#endif

  if ( numGames - numDevices != numTotalGames ) {
    if ( reparseGamelist && qmc2StopParser ) {
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: game list not fully parsed, invalidating game list cache"));
      QFile f(qmc2Config->value("MAME/FilesAndDirectories/GamelistCacheFile").toString());
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: machine list not fully parsed, invalidating machine list cache"));
      QFile f(qmc2Config->value("MESS/FilesAndDirectories/GamelistCacheFile").toString());
#endif
      f.remove();
    } else if ( !qmc2StopParser) {
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: game list cache is out of date, invalidating game list cache"));
      QFile f(qmc2Config->value("MAME/FilesAndDirectories/GamelistCacheFile").toString());
#elif defined(QMC2_EMUTYPE_MESS)
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: machine list cache is out of date, invalidating machine list cache"));
      QFile f(qmc2Config->value("MESS/FilesAndDirectories/GamelistCacheFile").toString());
#endif
      f.remove();
    }
  }
  QString sL = numTotalGames + numDevices >= 0 ? QString::number(numTotalGames + numDevices) : tr("?");
  QString sC = numCorrectGames >= 0 ? QString::number(numCorrectGames) : tr("?");
  QString sM = numMostlyCorrectGames >= 0 ? QString::number(numMostlyCorrectGames) : tr("?");
  QString sI = numIncorrectGames >= 0 ? QString::number(numIncorrectGames) : tr("?");
  QString sN = numNotFoundGames >= 0 ? QString::number(numNotFoundGames) : tr("?");
  QString sU = numUnknownGames >= 0 ? QString::number(numUnknownGames) : tr("?");
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ROM state info: L:%1 C:%2 M:%3 I:%4 N:%5 U:%6").arg(sL).arg(sC).arg(sM).arg(sI).arg(sN).arg(sU));
  qmc2MainWindow->progressBarGamelist->reset();

  qmc2ReloadActive = false;
  qmc2StartingUp = false;

  if ( qmc2StopParser ) {
	  if ( loadProc )
		  loadProc->kill();
  } else {
	  if ( romStateCacheUpdate || cachedGamesCounter - numDevices != numTotalGames ) {
		  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/AutoTriggerROMCheck").toBool() ) {
			  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: ROM state cache is incomplete or not up to date, triggering an automatic ROM check"));
			  autoRomCheck = true;
		  } else {
			  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: ROM state cache is incomplete or not up to date, please re-check ROMs"));
		  }
	  }
  }

  verifyCurrentOnly = false;

  if ( autoRomCheck )
	  QTimer::singleShot(QMC2_AUTOROMCHECK_DELAY, qmc2MainWindow->actionCheckROMs, SLOT(trigger()));
  else if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/EnableRomStateFilter", true).toBool() )
	  filter(true);

  enableWidgets(true);
}

void Gamelist::filter(bool initial)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Gamelist::filter(initial = %1)").arg(initial));
#endif

  if ( qmc2FilterActive ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ROM state filter already active"));
    return;
  }

  if ( qmc2VerifyActive ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("please wait for ROM verification to finish and try again"));
    return;
  }

  if ( qmc2ReloadActive ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("please wait for reload to finish and try again"));
    return;
  }

  bool showC = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowC", true).toBool();
  bool showM = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowM", true).toBool();
  bool showI = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowI", true).toBool();
  bool showN = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowN", true).toBool();
  bool showU = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowU", true).toBool();
  bool showDeviceSets = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowDeviceSets", true).toBool();
  bool showBiosSets = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowBiosSets", true).toBool();

  if ( initial && showC && showM && showI && showN && showU && showDeviceSets && showBiosSets ) {
	  qmc2StatesTogglesEnabled = true;
	  return;
  }

  QTime elapsedTime(0, 0, 0, 0);
  qmc2StopParser = false;
  parseTimer.start();
  qmc2FilterActive = true;
  enableWidgets(false);
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("applying ROM state filter"));
  qmc2MainWindow->progressBarGamelist->reset();
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
    qmc2MainWindow->progressBarGamelist->setFormat(tr("State filter - %p%"));
  else
    qmc2MainWindow->progressBarGamelist->setFormat("%p%");
  qmc2MainWindow->progressBarGamelist->setRange(0, numGames - 1);
  qApp->processEvents();
  if ( verifyCurrentOnly && checkedItem ) {
	  QString gameName = checkedItem->text(QMC2_GAMELIST_COLUMN_NAME);
	  if ( !showBiosSets && isBios(gameName) )
		  checkedItem->setHidden(true);
	  else if ( !showDeviceSets && isDevice(gameName) )
		  checkedItem->setHidden(true);
	  else switch ( gameStatusHash[gameName] ) {
		  case 'C':
			  checkedItem->setHidden(!showC);
			  break;
		  case 'M':
			  checkedItem->setHidden(!showM);
			  break;
		  case 'I':
			  checkedItem->setHidden(!showI);
			  break;
		  case 'N':
			  checkedItem->setHidden(!showN);
			  break;
		  case 'U':
		  default:
			  checkedItem->setHidden(!showU);
			  break;
	  }
  } else {
	  qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(false);
	  int filterResponse = numGames / QMC2_STATEFILTER_UPDATES;
	  for (int i = 0; i < qmc2MainWindow->treeWidgetGamelist->topLevelItemCount() && !qmc2StopParser; i++) {
		  if ( i % filterResponse == 0 ) {
			  qmc2MainWindow->progressBarGamelist->setValue(i);
			  qApp->processEvents();
		  }
		  QTreeWidgetItem *item = qmc2MainWindow->treeWidgetGamelist->topLevelItem(i);
		  QString gameName = item->text(QMC2_GAMELIST_COLUMN_NAME);
		  if ( !showBiosSets && isBios(gameName) )
			  item->setHidden(true);
		  else if ( !showDeviceSets && isDevice(gameName) )
			  item->setHidden(true);
		  else switch ( gameStatusHash[gameName] ) {
			  case 'C':
				  item->setHidden(!showC);
				  break;
			  case 'M':
				  item->setHidden(!showM);
				  break;
			  case 'I':
				  item->setHidden(!showI);
				  break;
			  case 'N':
				  item->setHidden(!showN);
				  break;
			  case 'U':
			  default:
				  item->setHidden(!showU);
				  break;
		  }
	  }
	  qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(true);
  }
  qmc2MainWindow->progressBarGamelist->setValue(numGames - 1);
  qmc2FilterActive = false;
  elapsedTime = elapsedTime.addMSecs(parseTimer.elapsed());
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (applying ROM state filter, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
  qmc2MainWindow->progressBarGamelist->reset();
  QTimer::singleShot(0, qmc2MainWindow, SLOT(scrollToCurrentItem()));
  qApp->processEvents();
  enableWidgets(true);
  qmc2StatesTogglesEnabled = true;
}

void Gamelist::save()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::save()");
#endif

  // FIXME: there's no use in this method...
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("saving game list"));
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (saving game list)"));
#elif defined(QMC2_EMUTYPE_MESS)
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("saving machine list"));
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (saving machine list)"));
#endif
}

void Gamelist::loadFavorites()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::loadFavorites()");
#endif

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading favorites"));

  qmc2MainWindow->listWidgetFavorites->clear();
  QFile f(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/FavoritesFile").toString());
  if ( f.open(QIODevice::ReadOnly | QIODevice::Text) ) {
    QTextStream ts(&f);
    while ( !ts.atEnd() ) {
      QString gameName = ts.readLine();
      if ( !gameName.isEmpty() ) {
        QTreeWidgetItem *gameItem = qmc2GamelistItemMap[gameName];
        if ( gameItem ) {
          QListWidgetItem *item = new QListWidgetItem(qmc2MainWindow->listWidgetFavorites);
          item->setText(gameItem->text(QMC2_GAMELIST_COLUMN_GAME));
	  if ( gameItem->isSelected() )
		  item->setSelected(true);
        }
      }
    }
    f.close();
  }

  qmc2MainWindow->listWidgetFavorites->sortItems();
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading favorites)"));
  if ( qmc2MainWindow->tabWidgetGamelist->currentIndex() == QMC2_FAVORITES_INDEX )
    QTimer::singleShot(50, qmc2MainWindow, SLOT(checkCurrentFavoritesSelection()));
  else
    qmc2MainWindow->listWidgetFavorites->setCurrentIndex(QModelIndex());
}

void Gamelist::saveFavorites()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::saveFavorites()");
#endif

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("saving favorites"));

  QFile f(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/FavoritesFile").toString());
  if ( f.open(QIODevice::WriteOnly | QIODevice::Text) ) {
    QTextStream ts(&f);
    int i;
    for (i = 0; i < qmc2MainWindow->listWidgetFavorites->count(); i++) {
      ts << qmc2GamelistNameMap[qmc2MainWindow->listWidgetFavorites->item(i)->text()] << "\n";
    }
    f.close();
  } else
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open favorites file for writing, path = %1").arg(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/FavoritesFile").toString()));

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (saving favorites)"));
}

void Gamelist::loadPlayHistory()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::loadPlayHistory()");
#endif

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading play history"));

  qmc2MainWindow->listWidgetPlayed->clear();
  QFile f(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/HistoryFile").toString());
  if ( f.open(QIODevice::ReadOnly | QIODevice::Text) ) {
    QTextStream ts(&f);
    while ( !ts.atEnd() ) {
      QString gameName = ts.readLine();
      if ( !gameName.isEmpty() ) {
        QTreeWidgetItem *gameItem = qmc2GamelistItemMap[gameName];
        if ( gameItem ) {
          QListWidgetItem *item = new QListWidgetItem(qmc2MainWindow->listWidgetPlayed);
          item->setText(gameItem->text(QMC2_GAMELIST_COLUMN_GAME));
	  if ( gameItem->isSelected() )
		  item->setSelected(true);
        }
      }
    }
    f.close();
  }

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading play history)"));
  if ( qmc2MainWindow->tabWidgetGamelist->currentIndex() == QMC2_PLAYED_INDEX )
    QTimer::singleShot(50, qmc2MainWindow, SLOT(checkCurrentPlayedSelection()));
  else
    qmc2MainWindow->listWidgetPlayed->setCurrentIndex(QModelIndex());
}

void Gamelist::savePlayHistory()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::savePlayHistory()");
#endif

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("saving play history"));

  QFile f(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/HistoryFile").toString());
  if ( f.open(QIODevice::WriteOnly | QIODevice::Text) ) {
    QTextStream ts(&f);
    int i;
    for (i = 0; i < qmc2MainWindow->listWidgetPlayed->count(); i++) {
      ts << qmc2GamelistNameMap[qmc2MainWindow->listWidgetPlayed->item(i)->text()] << "\n";
    }
    f.close();
  } else {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open play history file for writing, path = %1").arg(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/HistoryFile").toString()));
  }

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (saving play history)"));
}

QString Gamelist::status()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::status()");
#endif

  QLocale locale;
  QString statusString = "<b>";
  statusString += "<font color=black>" + tr("L:") + QString(numGames > -1 ? locale.toString(numGames) : tr("?")) + "</font>\n";
  statusString += "<font color=#00cc00>" + tr("C:") + QString(numCorrectGames > -1 ? locale.toString(numCorrectGames) : tr("?")) + "</font>\n";
  statusString += "<font color=#a2c743>" + tr("M:") + QString(numMostlyCorrectGames > -1 ? locale.toString(numMostlyCorrectGames) : tr("?")) + "</font>\n";
  statusString += "<font color=#f90000>" + tr("I:") + QString(numIncorrectGames > -1 ? locale.toString(numIncorrectGames) : tr("?")) + "</font>\n";
  statusString += "<font color=#7f7f7f>" + tr("N:") + QString(numNotFoundGames > -1 ? locale.toString(numNotFoundGames) : tr("?")) + "</font>\n";
  statusString += "<font color=#0000f9>" + tr("U:") + QString(numUnknownGames > -1 ? locale.toString(numUnknownGames) : tr("?")) + "</font>\n";
  statusString += "<font color=chocolate>" + tr("S:") + QString(numSearchGames > -1 ? locale.toString(numSearchGames) : tr("?")) + "</font>\n";
  statusString += "<font color=sandybrown>" + tr("T:") + QString(numTaggedSets > -1 ? locale.toString(numTaggedSets) : tr("?")) + "</font>";
  statusString += "</b>";

  return statusString;
}

void Gamelist::loadStarted()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::loadStarted()");
#endif

  qmc2MainWindow->progressBarGamelist->setRange(0, numTotalGames);
  qmc2MainWindow->progressBarGamelist->reset();
}

void Gamelist::loadFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Gamelist::loadFinished(int exitCode = %1, QProcess::ExitStatus exitStatus = %2): proc = %3").arg(exitCode).arg(exitStatus).arg((qulonglong)loadProc));
#endif

	bool invalidateListXmlCache = false;
	if ( exitStatus != QProcess::NormalExit && !qmc2StopParser ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: emulator audit call didn't exit cleanly -- exitCode = %1, exitStatus = %2").arg(exitCode).arg(QString(exitStatus == QProcess::NormalExit ? tr("normal") : tr("crashed"))));
		qmc2StopParser = invalidateListXmlCache = true;
	} else if ( qmc2StopParser && exitStatus == QProcess::CrashExit )
		qmc2StopParser = invalidateListXmlCache = true;

	QTime elapsedTime(0, 0, 0, 0);
	elapsedTime = elapsedTime.addMSecs(loadTimer.elapsed());
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading XML data and recreating cache, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
	qmc2MainWindow->progressBarGamelist->reset();
	qmc2EarlyReloadActive = false;
	if ( loadProc )
		delete loadProc;
	loadProc = NULL;

	if ( romCache.isOpen() )
		romCache.close();

	xmlDb()->commitTransaction();
	uncommittedXmlDbRows = 0;
	if ( invalidateListXmlCache ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: XML data cache is incomplete, invalidating XML data cache"));
		xmlDb()->recreateDatabase();
	}

	parse();
	loadFavorites();
	loadPlayHistory();

	// show game list
	qmc2MainWindow->labelLoadingGamelist->setVisible(false);
	qmc2MainWindow->treeWidgetGamelist->setVisible(true);
	qmc2MainWindow->labelLoadingHierarchy->setVisible(false);
	qmc2MainWindow->treeWidgetHierarchy->setVisible(true);

	if ( qmc2MainWindow->tabWidgetGamelist->currentIndex() == QMC2_GAMELIST_INDEX ) {
		if ( qApp->focusWidget() != qmc2MainWindow->comboBoxToolbarSearch ) {
			switch ( qmc2MainWindow->stackedWidgetView->currentIndex() ) {
				case QMC2_VIEW_DETAIL_INDEX:
					qmc2MainWindow->treeWidgetGamelist->setFocus();
					break;
				case QMC2_VIEW_TREE_INDEX:
					qmc2MainWindow->treeWidgetHierarchy->setFocus();
					break;
				case QMC2_VIEW_CATEGORY_INDEX:
					qmc2MainWindow->treeWidgetCategoryView->setFocus();
					break;
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
				case QMC2_VIEW_VERSION_INDEX:
					qmc2MainWindow->treeWidgetVersionView->setFocus();
					break;
#endif
				default:
					qmc2MainWindow->treeWidgetGamelist->setFocus();
					break;
			}
		}
	}

	qApp->processEvents();
}

void Gamelist::loadReadyReadStandardOutput()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Gamelist::loadReadyReadStandardOutput(): proc = %1)").arg((qulonglong)loadProc));
#endif

	static bool lastCharacterWasSpace = false;
	static QString dtdBuffer;
	static QString setXmlBuffer;
	static QString currentSetName;

	// this makes the GUI much more responsive, but is HAS to be called before loadProc->readAllStandardOutput()!
	qApp->processEvents();

#if defined(QMC2_OS_WIN)
	QString readBuffer = QString::fromUtf8(loadProc->readAllStandardOutput());
#else
	QString readBuffer = loadProc->readAllStandardOutput();
#endif

	bool startsWithSpace = readBuffer.startsWith(" ") && !lastCharacterWasSpace;
	bool endsWithSpace = readBuffer.endsWith(" ");
	lastCharacterWasSpace = false;

	if ( uncommittedXmlDbRows == 0 )
		xmlDb()->beginTransaction();

	if ( qmc2StopParser )
		loadProc->kill();

	readBuffer = readBuffer.simplified();

	if ( startsWithSpace )
		readBuffer.prepend(" ");

	// ensure XML elements are on individual lines
	for (int i = 0; i < readBuffer.length(); i++) {
		if ( readBuffer[i] == '>' ) {
			if ( i + 1 < readBuffer.length() ) {
				if ( readBuffer[i + 1] == '<' )
					readBuffer.insert(i + 1, "\n");
				else if ( readBuffer[i + 1] == ' ' ) {
					if ( i + 2 < readBuffer.length() ) {
						if ( readBuffer[i + 2] == '<' )
							readBuffer.replace(i + 1, 1, "\n");
					}
				}
			}
		}
	}

	QStringList sl = readBuffer.split("\n");

	for (int l = 0; l < sl.count(); l++) {
		QString singleXMLLine = sl[l];
		bool newLine = singleXMLLine.endsWith(">");
		if ( newLine ) {
			if ( singleXMLLine.indexOf(QRegExp("\\<description\\>$|\\<year\\>$|\\<manufacturer\\>$")) >= 0 )
				newLine = false;
			if ( newLine ) {
				bool found = false;
				int i;
				for (i = singleXMLLine.length() - 2; i > 0 && !found; i--)
					found = (singleXMLLine[i] == '<');
				if ( found && i == 0 )
					newLine = false;
			}
		}
		bool needsSpace = singleXMLLine.endsWith("\"");
		if ( needsSpace ) {
			bool found = false;
			bool stop = false;
			for (int i = singleXMLLine.length() - 2; i > 1 && !found && !stop; i--) {
				if ( singleXMLLine[i] == '\"' ) {
					if ( singleXMLLine[i - 1] == '=' )
						found = true;
					else
						stop = true;
				}
			}
			if ( !found )
				needsSpace = false;
		}
		int i = singleXMLLine.length() - 1;
		while ( i >= 0 && singleXMLLine[i].isSpace() )
			singleXMLLine.remove(i--, 1);
		needsSpace |= endsWithSpace;
		if ( newLine )
			singleXMLLine += "\n";
		else if ( needsSpace ) {
			singleXMLLine += " ";
			lastCharacterWasSpace = true;
		}

		xmlLineBuffer += singleXMLLine;
		if ( xmlLineBuffer.endsWith("\n") ) {
			if ( !dtdBufferReady ) {
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
				dtdBufferReady = xmlLineBuffer.startsWith("<mame build=");
#elif defined(QMC2_EMUTYPE_MESS)
				dtdBufferReady = xmlLineBuffer.startsWith("<mess build=");
#endif
				if ( !dtdBufferReady ) {
					if ( !xmlLineBuffer.startsWith("<?xml version=") )
						dtdBuffer += xmlLineBuffer;
				} else {
					if ( dtdBuffer.endsWith("\n") )
						dtdBuffer.remove(dtdBuffer.length() - 1, 1);
					xmlDb()->setDtd(dtdBuffer);
					dtdBuffer.clear();
				}
			} else {
				if ( currentSetName.isEmpty() ) {
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
					int startIndex = xmlLineBuffer.indexOf("<game name=\"");
#elif defined(QMC2_EMUTYPE_MESS)
					int startIndex = xmlLineBuffer.indexOf("<machine name=\"");
#endif

					if ( startIndex >= 0 ) {
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
						startIndex += 12;
#elif defined(QMC2_EMUTYPE_MESS)
						startIndex += 15;
#endif
						int endIndex = xmlLineBuffer.indexOf("\"", startIndex);
						if ( endIndex >= 0 ) {
							currentSetName = xmlLineBuffer.mid(startIndex, endIndex - startIndex);
							setXmlBuffer += xmlLineBuffer;
						}
					}
				} else {
					setXmlBuffer += xmlLineBuffer;
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
					int index = xmlLineBuffer.indexOf("</game>");
#elif defined(QMC2_EMUTYPE_MESS)
					int index = xmlLineBuffer.indexOf("</machine>");
#endif
					if ( index >= 0 ) {
						if ( setXmlBuffer.endsWith("\n") )
							setXmlBuffer.remove(setXmlBuffer.length() - 1, 1);

						if ( xmlDb()->exists(currentSetName) ) {
#if defined(QMC2_EMUTYPE_MAME)
							qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: XML bug: the name '%1' is used for multiple sets -- please inform MAME developers").arg(currentSetName));
#elif defined(QMC2_EMUTYPE_MESS)
							qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: XML bug: the name '%1' is used for multiple sets -- please inform MESS developers").arg(currentSetName));
#elif defined(QMC2_EMUTYPE_UME)
							qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: XML bug: the name '%1' is used for multiple sets -- please inform MAME/MESS developers").arg(currentSetName));
#endif
						}

						xmlDb()->setXml(currentSetName, setXmlBuffer);
						uncommittedXmlDbRows++;
						currentSetName.clear();
						setXmlBuffer.clear();
					}
				}
			}
			xmlLineBuffer.clear();
		}
	}

	if ( uncommittedXmlDbRows >= QMC2_XMLCACHE_COMMIT ) {
		xmlDb()->commitTransaction();
		uncommittedXmlDbRows = 0;
	}

#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
	qmc2MainWindow->progressBarGamelist->setValue(qmc2MainWindow->progressBarGamelist->value() + readBuffer.count("<game name="));
#elif defined(QMC2_EMUTYPE_MESS)
	qmc2MainWindow->progressBarGamelist->setValue(qmc2MainWindow->progressBarGamelist->value() + readBuffer.count("<machine name="));
#endif
}

void Gamelist::loadError(QProcess::ProcessError processError)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::loadError(QProcess::ProcessError processError = " + QString::number(processError) + ")");
#endif

}

void Gamelist::loadStateChanged(QProcess::ProcessState processState)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::loadStateChanged(QProcess::ProcessState = " + QString::number(processState) + ")");
#endif

}

void Gamelist::verifyStarted()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::verifyStarted()");
#endif

	if ( !verifyCurrentOnly )
		qmc2MainWindow->progressBarGamelist->setValue(0);
}

void Gamelist::verifyFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Gamelist::verifyFinished(int exitCode = %1, QProcess::ExitStatus exitStatus = %2): proc = %3").arg(exitCode).arg(exitStatus).arg((qulonglong)verifyProc));
#endif

  if ( !verifyProc->atEnd() )
	  verifyReadyReadStandardOutput();

  bool cleanExit = true;
  if ( exitStatus != QProcess::NormalExit && !qmc2StopParser ) {
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: emulator audit call didn't exit cleanly -- exitCode = %1, exitStatus = %2").arg(exitCode).arg(QString(exitStatus == QProcess::NormalExit ? tr("normal") : tr("crashed"))));
    cleanExit = false;
  }

  bool showROMStatusIcons = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowROMStatusIcons", true).toBool();
  if ( !verifyCurrentOnly ) {
    // the progress text may have changed in the meantime...
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
	    qmc2MainWindow->progressBarGamelist->setFormat(tr("ROM check - %p%"));
    QSet<QString> gameSet = QSet<QString>::fromList(qmc2GamelistItemMap.uniqueKeys());
    QList<QString> remainingGames = gameSet.subtract(QSet<QString>::fromList(verifiedList)).values();
    int i;
    if ( qmc2StopParser || !cleanExit ) {
      for (i = 0; i < remainingGames.count(); i++) {
        qmc2MainWindow->progressBarGamelist->setValue(qmc2MainWindow->progressBarGamelist->value() + 1);
        qmc2MainWindow->labelGamelistStatus->setText(status());
	if ( i % QMC2_REMAINING_SETS_CHECK_RSP == 0 )
		qApp->processEvents();
        QString gameName = remainingGames[i];
	gameStatusHash[gameName] = 'U';
        bool isBIOS = isBios(gameName);
        bool isDevice = this->isDevice(gameName);
        QTreeWidgetItem *romItem = qmc2GamelistItemMap[gameName];
        QTreeWidgetItem *hierarchyItem = qmc2HierarchyItemMap[gameName];
	QTreeWidgetItem *categoryItem = qmc2CategoryItemMap[gameName];
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
	QTreeWidgetItem *versionItem = qmc2VersionItemMap[gameName];
#endif
        if ( romCache.isOpen() )
          tsRomCache << gameName << " U\n";
        numUnknownGames++;
        if ( romItem && hierarchyItem ) {
          if ( isBIOS ) {
            if ( showROMStatusIcons ) {
              romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownBIOSImageIcon);
              hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownBIOSImageIcon);
	      if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownBIOSImageIcon);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
	      if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownBIOSImageIcon);
#endif
	    }
          } else if ( isDevice ) {
            if ( showROMStatusIcons ) {
              romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownDeviceImageIcon);
              hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownDeviceImageIcon);
	      if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownDeviceImageIcon);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
	      if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownDeviceImageIcon);
#endif
	    }
          } else {
            if ( showROMStatusIcons ) {
              romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownImageIcon);
              hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownImageIcon);
	      if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownImageIcon);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
	      if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownImageIcon);
#endif
            }
          }
        } else
          qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: can't find item map entry for '%1' - ROM state cannot be determined").arg(gameName));
        if ( romItem == qmc2CurrentItem ) qmc2MainWindow->labelGameStatus->setPalette(MainWindow::qmc2StatusColorBlue);
      }
    } else {
      if ( !remainingGames.isEmpty() && !qmc2StopParser )
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("checking real status of %n set(s) not mentioned during full audit", "", remainingGames.count()));
      for (i = 0; i < remainingGames.count() && !qmc2StopParser; i++) {
        qmc2MainWindow->progressBarGamelist->setValue(qmc2MainWindow->progressBarGamelist->value() + 1);
        qmc2MainWindow->labelGamelistStatus->setText(status());
	if ( i % QMC2_REMAINING_SETS_CHECK_RSP == 0 )
		qApp->processEvents();
        QString gameName = remainingGames[i];
        bool isBIOS = isBios(gameName);
        bool isDevice = this->isDevice(gameName);
        QTreeWidgetItem *romItem = qmc2GamelistItemMap[gameName];
        QTreeWidgetItem *hierarchyItem = qmc2HierarchyItemMap[gameName];
	QTreeWidgetItem *categoryItem = qmc2CategoryItemMap[gameName];
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
	QTreeWidgetItem *versionItem = qmc2VersionItemMap[gameName];
#endif
	// there are quite a number of sets in MESS and MAME that don't require any ROMs... many/most device-sets in particular
	bool romRequired = true;
	int xmlCounter = 0;
	QStringList xmlLines = xmlDb()->xml(gameName).split("\n", QString::SkipEmptyParts);
	if ( xmlLines.count() > 0 ) {
		int romCounter = 0;
		int chdCounter = 0;
		bool endFound = false;
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
		QString endMark = "</game>";
#elif defined(QMC2_EMUTYPE_MESS)
		QString endMark = "</machine>";
#else
		QString endMark = "</undefined>";
#endif
		while ( !endFound && xmlCounter < xmlLines.count() ) {
			if ( xmlLines[xmlCounter].contains("<rom name=\"") ) {
				romCounter++;
				endFound = true;
			} else if ( xmlLines[xmlCounter].contains("<disk name=\"") ) {
				chdCounter++;
				endFound = true;
			} else if ( xmlLines[xmlCounter].contains(endMark) )
				endFound = true;
			xmlCounter++;
		}
		if ( romCounter == 0 && chdCounter > 0 )
			romRequired = true;
		else
			romRequired = (romCounter > 0);
	}
        if ( romItem && hierarchyItem ) {
	  if ( romCache.isOpen() ) {
		  if ( romRequired ) {
			  tsRomCache << gameName << " N\n";
			  numNotFoundGames++;
		  } else {
			  tsRomCache << gameName << " C\n";
			  numCorrectGames++;
		  }
	  }
          if ( isBIOS ) {
            if ( showROMStatusIcons ) {
              if ( romRequired ) {
                romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundBIOSImageIcon);
                hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundBIOSImageIcon);
	        if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundBIOSImageIcon);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
	        if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundBIOSImageIcon);
#endif
              } else {
                romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
                hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
	        if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
	        if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
#endif
              }
            }
          } else if ( isDevice ) {
              if ( romRequired ) {
                romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundDeviceImageIcon);
                hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundDeviceImageIcon);
	        if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundDeviceImageIcon);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
	        if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundDeviceImageIcon);
#endif
              } else {
                romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectDeviceImageIcon);
                hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectDeviceImageIcon);
	        if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectDeviceImageIcon);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
	        if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectDeviceImageIcon);
#endif
              }
          } else {
            if ( showROMStatusIcons ) {
              if ( romRequired ) {
                romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundImageIcon);
                hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundImageIcon);
                if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundImageIcon);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
                if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundImageIcon);
#endif
              } else {
                romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
                hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
                if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
                if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
#endif
              }
            }
          }
	  if ( romRequired ) {
		  gameStatusHash[gameName] = 'N';
		  if ( romItem == qmc2CurrentItem ) qmc2MainWindow->labelGameStatus->setPalette(MainWindow::qmc2StatusColorGrey);
	  } else {
		  gameStatusHash[gameName] = 'C';
		  if ( romItem == qmc2CurrentItem ) qmc2MainWindow->labelGameStatus->setPalette(MainWindow::qmc2StatusColorGreen);
	  }
        } else {
          qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: can't find item map entry for '%1' - ROM state cannot be determined").arg(gameName));
          if ( romCache.isOpen() )
            tsRomCache << gameName << " U\n";
          gameStatusHash[gameName] = 'U';
          numUnknownGames++;
        }
      }
      if ( !remainingGames.isEmpty() && !qmc2StopParser )
        qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (checking real status of %n set(s) not mentioned during full audit)", "", remainingGames.count()));
    }
    qmc2MainWindow->labelGamelistStatus->setText(status());
  }

  bool doFilter = true;
  if ( verifyCurrentOnly ) {
    QString gameName;
    if ( verifiedList.isEmpty() && checkedItem && exitCode == QMC2_MAME_ERROR_NO_SUCH_GAME ) {
	    // many device-sets that have no ROMs are declared as being "invalid" during the audit, but that isn't true :)
	    gameName = checkedItem->text(QMC2_GAMELIST_COLUMN_NAME);
	    QTreeWidgetItem *hierarchyItem = qmc2HierarchyItemMap[gameName];
	    if ( hierarchyItem ) {
		    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ROM status for '%1' is '%2'").arg(checkedItem->text(QMC2_GAMELIST_COLUMN_GAME)).arg(QObject::tr("correct")));
		    gameStatusHash[gameName] = 'C';
		    numUnknownGames--;
		    numCorrectGames++;
		    if ( checkedItem == qmc2CurrentItem ) qmc2MainWindow->labelGameStatus->setPalette(MainWindow::qmc2StatusColorGreen);
		    QTreeWidgetItem *categoryItem = qmc2CategoryItemMap[gameName];
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
		    QTreeWidgetItem *versionItem = qmc2VersionItemMap[gameName];
#endif
		    if ( isBios(gameName) ) {
			    if ( showROMStatusIcons ) {
				    checkedItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
				    hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
				    if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
				    if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
#endif
			    }
		    } else if ( isDevice(gameName) ) {
			    if ( showROMStatusIcons ) {
				    checkedItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectDeviceImageIcon);
				    hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectDeviceImageIcon);
				    if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectDeviceImageIcon);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
				    if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectDeviceImageIcon);
#endif
			    }
		    } else {
			    if ( showROMStatusIcons ) {
				    checkedItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
				    hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
				    if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
				    if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
#endif
			    }
		    }
	    }
	    qmc2MainWindow->labelGamelistStatus->setText(status());
    }
    if ( romCache.isOpen() ) {
	    QHashIterator<QString, char> it(gameStatusHash);
	    while ( it.hasNext() ) {
		    it.next();
		    tsRomCache << it.key() << " ";
		    switch ( it.value() ) {
			    case 'C':
				    tsRomCache << "C\n";
				    break;
			    case 'M':
				    tsRomCache << "M\n";
				    break;
			    case 'I':
				    tsRomCache << "I\n";
				    break;
			    case 'N':
				    tsRomCache << "N\n";
				    break;
			    case 'U':
			    default:
				    tsRomCache << "U\n";
				    break;
		    }
	    }
    }
    doFilter = oldRomState != gameStatusHash[gameName];
  }

  QTime elapsedTime(0, 0, 0, 0);
  elapsedTime = elapsedTime.addMSecs(verifyTimer.elapsed());
  if ( verifyCurrentOnly )
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (verifying ROM status for '%1', elapsed time = %2)").arg(checkedItem->text(QMC2_GAMELIST_COLUMN_GAME)).arg(elapsedTime.toString("mm:ss.zzz")));
  else
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (verifying ROM status for all sets, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));

  QString sL = numTotalGames + numDevices >= 0 ? QString::number(numTotalGames + numDevices) : tr("?");
  QString sC = numCorrectGames >= 0 ? QString::number(numCorrectGames) : tr("?");
  QString sM = numMostlyCorrectGames >= 0 ? QString::number(numMostlyCorrectGames) : tr("?");
  QString sI = numIncorrectGames >= 0 ? QString::number(numIncorrectGames) : tr("?");
  QString sN = numNotFoundGames >= 0 ? QString::number(numNotFoundGames) : tr("?");
  QString sU = numUnknownGames >= 0 ? QString::number(numUnknownGames) : tr("?");
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ROM state info: L:%1 C:%2 M:%3 I:%4 N:%5 U:%6").arg(sL).arg(sC).arg(sM).arg(sI).arg(sN).arg(sU));
  qmc2MainWindow->progressBarGamelist->reset();

  if ( verifyProc )
    delete verifyProc;
  verifyProc = NULL;
  qmc2VerifyActive = false;

  if ( romCache.isOpen() ) {
    tsRomCache.flush();
    romCache.close();
  }

  if ( qmc2SortCriteria == QMC2_SORT_BY_ROM_STATE ) {
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("sorting game list by %1 in %2 order").arg(tr("ROM state")).arg(qmc2SortOrder == Qt::AscendingOrder ? tr("ascending") : tr("descending")));
#elif defined(QMC2_EMUTYPE_MESS)
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("sorting machine list by %1 in %2 order").arg(tr("ROM state")).arg(qmc2SortOrder == Qt::AscendingOrder ? tr("ascending") : tr("descending")));
#endif
    qApp->processEvents();
    for (int i = 0; i < qmc2MainWindow->treeWidgetGamelist->topLevelItemCount(); i++) {
      QTreeWidgetItem *item = qmc2MainWindow->treeWidgetGamelist->topLevelItem(i);
      if ( item->childCount() > 1 ) {
        qmc2MainWindow->treeWidgetGamelist->collapseItem(item);
        QList<QTreeWidgetItem *> childrenList = item->takeChildren();
        int j;
        for (j = 0; j < childrenList.count(); j++)
          delete childrenList[j];
        QTreeWidgetItem *nameItem = new QTreeWidgetItem(item);
        nameItem->setText(QMC2_GAMELIST_COLUMN_GAME, tr("Waiting for data..."));
        nameItem->setText(QMC2_GAMELIST_COLUMN_ICON, qmc2GamelistNameMap[item->text(QMC2_GAMELIST_COLUMN_GAME)]);
        qApp->processEvents();
      }
    }
    qApp->processEvents();
    qmc2MainWindow->treeWidgetGamelist->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
    QTreeWidgetItem *ci = qmc2MainWindow->treeWidgetGamelist->currentItem();
    if ( ci )
      QTimer::singleShot(0, qmc2MainWindow, SLOT(scrollToCurrentItem()));
  }

  enableWidgets(true);

  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/EnableRomStateFilter", true).toBool() ){
	  if ( doFilter )
		  filter();
	  else
		  QTimer::singleShot(0, qmc2MainWindow, SLOT(scrollToCurrentItem()));
  }
}

void Gamelist::verifyReadyReadStandardOutput()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Gamelist::verifyReadyReadStandardOutput(): proc = %1").arg((qulonglong) verifyProc));
#endif

  // this makes the GUI much more responsive, but is HAS to be called before verifyProc->readAllStandardOutput()!
  qApp->processEvents();

  // process rom verification output
  int i;
  QString romName, romStateLong; 
  char romState;
  QString s = verifyLastLine + verifyProc->readAllStandardOutput();
#if defined(QMC2_OS_WIN)
  s.replace("\r\n", "\n"); // convert WinDOS's "0x0D 0x0A" to just "0x0A" 
#endif
  QStringList lines = s.split("\n");

  if ( s.endsWith("\n") ) {
    verifyLastLine.clear();
  } else {
    verifyLastLine = lines.last();
    lines.removeLast();
  }

  if ( !verifyCurrentOnly ) {
    // the progress text may have changed in the meantime...
    if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
	    qmc2MainWindow->progressBarGamelist->setFormat(tr("ROM check - %p%"));
  }

  bool showROMStatusIcons = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowROMStatusIcons", true).toBool();
  for (i = 0; i < lines.count(); i++) {
    if ( lines[i].startsWith("romset ") ) {
      QStringList words = lines[i].split(" ");
      numVerifyRoms++;
      if ( words.count() > 2 ) {
        romName = words[1].remove("\"");
        bool isBIOS = isBios(romName);
        bool isDevice = this->isDevice(romName);
        if ( qmc2GamelistItemMap.contains(romName) ) {
          QTreeWidgetItem *romItem = qmc2GamelistItemMap[romName];
          QTreeWidgetItem *hierarchyItem = qmc2HierarchyItemMap[romName];
          QTreeWidgetItem *categoryItem = qmc2CategoryItemMap[romName];
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
          QTreeWidgetItem *versionItem = qmc2VersionItemMap[romName];
#endif
          if ( romItem && hierarchyItem ) {
            if ( words.last() == "good" || lines[i].endsWith("has no roms!") ) {
              romState = 'C';
              romStateLong = QObject::tr("correct");
              numCorrectGames++;
              if ( isBIOS ) {
                if ( showROMStatusIcons ) {
                  romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
                  hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
                  if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
                  if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
#endif
                }
              } else if ( isDevice ) {
                if ( showROMStatusIcons ) {
                  romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectDeviceImageIcon);
                  hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectDeviceImageIcon);
                  if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectDeviceImageIcon);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
                  if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectDeviceImageIcon);
#endif
                }
              } else {
                if ( showROMStatusIcons ) {
                  romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
                  hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
                  if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
                  if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
#endif
                }
              }
	      if ( romItem == qmc2CurrentItem ) qmc2MainWindow->labelGameStatus->setPalette(MainWindow::qmc2StatusColorGreen);
            } else if ( words.last() == "bad" ) {
              romState = 'I';
              romStateLong = QObject::tr("incorrect");
              numIncorrectGames++;
              if ( isBIOS ) {
                if ( showROMStatusIcons ) {
                  romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectBIOSImageIcon);
                  hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectBIOSImageIcon);
                  if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectBIOSImageIcon);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
                  if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectBIOSImageIcon);
#endif
                }
              } else if ( isDevice ) {
                if ( showROMStatusIcons ) {
                  romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectDeviceImageIcon);
                  hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectDeviceImageIcon);
                  if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectDeviceImageIcon);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
                  if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectDeviceImageIcon);
#endif
                }
              } else {
                if ( showROMStatusIcons ) {
                  romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectImageIcon);
                  hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectImageIcon);
                  if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectImageIcon);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
                  if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectImageIcon);
#endif
                }
              }
	      if ( romItem == qmc2CurrentItem ) qmc2MainWindow->labelGameStatus->setPalette(MainWindow::qmc2StatusColorRed);
            } else if ( words.last() == "available" ) {
              romState = 'M';
              romStateLong = QObject::tr("mostly correct");
              numMostlyCorrectGames++;
              if ( isBIOS ) {
                if ( showROMStatusIcons ) {
                  romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectBIOSImageIcon);
                  hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectBIOSImageIcon);
                  if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectBIOSImageIcon);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
                  if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectBIOSImageIcon);
#endif
                }
              } else if ( isDevice ) {
                if ( showROMStatusIcons ) {
                  romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectDeviceImageIcon);
                  hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectDeviceImageIcon);
                  if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectDeviceImageIcon);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
                  if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectDeviceImageIcon);
#endif
                }
              } else {
                if ( showROMStatusIcons ) {
                  romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectImageIcon);
                  hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectImageIcon);
                  if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectImageIcon);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
                  if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectImageIcon);
#endif
                }
              }
	      if ( romItem == qmc2CurrentItem ) qmc2MainWindow->labelGameStatus->setPalette(MainWindow::qmc2StatusColorYellowGreen);
            } else if ( words.last() == "missing" || words.last() == "found!" ) {
              romState = 'N';
              romStateLong = QObject::tr("not found");
              numNotFoundGames++;
              if ( isBIOS ) {
                if ( showROMStatusIcons ) {
                  romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundBIOSImageIcon);
                  hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundBIOSImageIcon);
                  if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundBIOSImageIcon);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
                  if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundBIOSImageIcon);
#endif
                }
              } else if ( isDevice ) {
                if ( showROMStatusIcons ) {
                  romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundDeviceImageIcon);
                  hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundDeviceImageIcon);
                  if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundDeviceImageIcon);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
                  if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundDeviceImageIcon);
#endif
                }
              } else {
                if ( showROMStatusIcons ) {
                  romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundImageIcon);
                  hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundImageIcon);
                  if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundImageIcon);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
                  if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundImageIcon);
#endif
                }
              }
	      if ( romItem == qmc2CurrentItem ) qmc2MainWindow->labelGameStatus->setPalette(MainWindow::qmc2StatusColorGrey);
            } else {
              romState = 'U';
              romStateLong = QObject::tr("unknown");
              numUnknownGames++;
              if ( isBIOS ) {
                if ( showROMStatusIcons ) {
                  romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownBIOSImageIcon);
                  hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownBIOSImageIcon);
                  if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownBIOSImageIcon);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
                  if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownBIOSImageIcon);
#endif
                }
              } else if ( isDevice ) {
                if ( showROMStatusIcons ) {
                  romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownDeviceImageIcon);
                  hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownDeviceImageIcon);
                  if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownDeviceImageIcon);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
                  if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownDeviceImageIcon);
#endif
                }
              } else {
                if ( showROMStatusIcons ) {
                  romItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownImageIcon);
                  hierarchyItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownImageIcon);
                  if ( categoryItem ) categoryItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownImageIcon);
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
                  if ( versionItem ) versionItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownImageIcon);
#endif
                }
              }
	      if ( romItem == qmc2CurrentItem ) qmc2MainWindow->labelGameStatus->setPalette(MainWindow::qmc2StatusColorBlue);
            }
          } else {
            qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: can't find item map entry for '%1' - ROM state cannot be determined").arg(romName));
            romState = 'U';
            romStateLong = QObject::tr("unknown");
            numUnknownGames++;
          }

          gameStatusHash[romName] = romState;

          verifiedList << romName;

          if ( verifyCurrentOnly ) {
            qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ROM status for '%1' is '%2'").arg(checkedItem->text(QMC2_GAMELIST_COLUMN_GAME)).arg(romStateLong));
            numUnknownGames--;
          } else if ( romCache.isOpen() )
            tsRomCache << romName << " " << romState << "\n";
        }
      }
    }
  }

  if ( romCache.isOpen() && !verifyCurrentOnly )
    tsRomCache.flush();

  if ( qmc2StopParser && verifyProc )
    verifyProc->kill();

  if ( !verifyCurrentOnly )
	  qmc2MainWindow->progressBarGamelist->setValue(numVerifyRoms);

  qmc2MainWindow->labelGamelistStatus->setText(status());
}

void Gamelist::verifyError(QProcess::ProcessError processError)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::verifyError(QProcess::ProcessError processError = " + QString::number(processError) + ")");
#endif

}

void Gamelist::verifyStateChanged(QProcess::ProcessState processState)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::verifyStateChanged(QProcess::ProcessState = " + QString::number(processState) + ")");
#endif

}

bool Gamelist::loadIcon(QString gameName, QTreeWidgetItem *item, bool checkOnly, QString *fileName)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Gamelist::loadIcon(QString gameName = %1, QTreeWidgetItem *item = %2, bool checkOnly = %3, QString *fileName = %4)").arg(gameName).arg((qulonglong)item).arg(checkOnly).arg((qulonglong)fileName));
#endif

  QIcon icon;
  char imageBuffer[QMC2_ZIP_BUFFER_SIZE];

  if ( fileName )
    *fileName = gameName;

  if ( qmc2IconMap.contains(gameName) ) {
    // use cached icon
    if ( !checkOnly )
      item->setIcon(QMC2_GAMELIST_COLUMN_ICON, qmc2IconMap.value(gameName));
    else
      qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(true);

    return true;
  } else if ( qmc2IconsPreloaded ) {
    // icon wasn't found
    if ( !checkOnly ) {
      qmc2IconMap[gameName] = QIcon();
      item->setIcon(QMC2_GAMELIST_COLUMN_ICON, QIcon());
    } else
      qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(true);

    return false;
  }

  if ( qmc2UseIconFile ) {
    // use icon file
    QByteArray imageData;
    int len, i;
    if ( !qmc2IconsPreloaded ) {
      QTime preloadTimer, elapsedTime(0, 0, 0, 0);
      int iconCount = 0;
      preloadTimer.start();
      if ( QMC2_ICON_FILETYPE_ZIP ) {
	      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("pre-caching icons from ZIP archive"));
	      foreach (unzFile iconFile, qmc2IconFileMap) {
		      unz_global_info unzGlobalInfo;
		      if ( unzGetGlobalInfo(iconFile, &unzGlobalInfo) == UNZ_OK ) {
			      int currentMax = qmc2MainWindow->progressBarGamelist->maximum();
			      QString oldFormat = qmc2MainWindow->progressBarGamelist->format();
			      if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
				      qmc2MainWindow->progressBarGamelist->setFormat(tr("Icon cache - %p%"));
			      else
				      qmc2MainWindow->progressBarGamelist->setFormat("%p%");
			      qmc2MainWindow->progressBarGamelist->setRange(0, unzGlobalInfo.number_entry);
			      qmc2MainWindow->progressBarGamelist->reset();
			      qApp->processEvents();
			      if ( unzGoToFirstFile(iconFile) == UNZ_OK ) {
				      int index = 0;
				      do {
					      char unzFileName[QMC2_MAX_PATH_LENGTH];
					      if ( index % QMC2_ICONCACHE_RESPONSIVENESS == 0 ) {
						      qmc2MainWindow->progressBarGamelist->setValue(index);
						      qApp->processEvents();
					      }
					      if ( unzGetCurrentFileInfo(iconFile, NULL, unzFileName, QMC2_MAX_PATH_LENGTH, NULL, 0, NULL, 0) == UNZ_OK ) {
						      QFileInfo fi(unzFileName);
						      QString gameFileName = fi.fileName();
#ifdef QMC2_DEBUG
						      qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Gamelist::loadIcon(): loading %1").arg(gameFileName));
#endif
						      imageData.clear();
						      if ( unzOpenCurrentFile(iconFile) == UNZ_OK ) {
							      while ( (len = unzReadCurrentFile(iconFile, &imageBuffer, QMC2_ZIP_BUFFER_SIZE)) > 0 )
								      for (i = 0; i < len; i++)
									      imageData += imageBuffer[i];
							      unzCloseCurrentFile(iconFile);
							      QPixmap iconPixmap;
							      if ( iconPixmap.loadFromData(imageData) ) {
								      QFileInfo fi(gameFileName.toLower());
								      qmc2IconMap[fi.baseName()] = QIcon(iconPixmap);
								      iconCount++;
							      }
						      }
					      }
					      if ( index % qmc2GamelistResponsiveness == 0 ) {
						      qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(true);
						      qApp->processEvents();
						      qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(false);
					      }
					      index++;
				      } while ( unzGoToNextFile(iconFile) != UNZ_END_OF_LIST_OF_FILE );
			      }
			      qmc2MainWindow->progressBarGamelist->setRange(0, currentMax);
			      if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
				      qmc2MainWindow->progressBarGamelist->setFormat(oldFormat);
			      else
				      qmc2MainWindow->progressBarGamelist->setFormat("%p%");
		      }
	      }
	      elapsedTime = elapsedTime.addMSecs(preloadTimer.elapsed());
	      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (pre-caching icons from ZIP archive, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
	      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("%n icon(s) loaded", "", iconCount));
	      qmc2IconsPreloaded = true;
      } else if ( QMC2_ICON_FILETYPE_7Z ) {
	      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("pre-caching icons from 7z archive"));
	      foreach (SevenZipFile *sevenZipFile, qmc2IconFileMap7z) {
		      int currentMax = qmc2MainWindow->progressBarGamelist->maximum();
		      QString oldFormat = qmc2MainWindow->progressBarGamelist->format();
		      if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
			      qmc2MainWindow->progressBarGamelist->setFormat(tr("Icon cache - %p%"));
		      else
			      qmc2MainWindow->progressBarGamelist->setFormat("%p%");
		      qmc2MainWindow->progressBarGamelist->setRange(0, sevenZipFile->itemList().count());
		      qmc2MainWindow->progressBarGamelist->reset();
		      qApp->processEvents();
		      for (int index = 0; index < sevenZipFile->itemList().count(); index++) {
			      if ( index % QMC2_ICONCACHE_RESPONSIVENESS == 0 ) {
				      qmc2MainWindow->progressBarGamelist->setValue(index);
				      qApp->processEvents();
			      }
			      SevenZipMetaData metaData = sevenZipFile->itemList()[index];
			      QFileInfo fi(metaData.name());
			      QString gameFileName = fi.fileName();
#ifdef QMC2_DEBUG
			      qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Gamelist::loadIcon(): loading %1").arg(gameFileName));
#endif
			      sevenZipFile->read(index, &imageData);
			      if ( !sevenZipFile->hasError() ) {
				      QPixmap iconPixmap;
				      if ( iconPixmap.loadFromData(imageData) ) {
					      QFileInfo fi(gameFileName.toLower());
					      qmc2IconMap[fi.baseName()] = QIcon(iconPixmap);
					      iconCount++;
				      }
			      }
			      if ( index % qmc2GamelistResponsiveness == 0 ) {
				      qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(true);
				      qApp->processEvents();
				      qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(false);
			      }
		      }
		      qmc2MainWindow->progressBarGamelist->setRange(0, currentMax);
		      if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
			      qmc2MainWindow->progressBarGamelist->setFormat(oldFormat);
		      else
			      qmc2MainWindow->progressBarGamelist->setFormat("%p%");
	      }
	      elapsedTime = elapsedTime.addMSecs(preloadTimer.elapsed());
	      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (pre-caching icons from 7z archive, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
	      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("%n icon(s) loaded", "", iconCount));
	      qmc2IconsPreloaded = true;
      }

      if ( checkOnly )
        qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(true);

      return loadIcon(gameName, item, checkOnly);
    }
  } else {
    // use icon directory
    if ( !qmc2IconsPreloaded ) {
      QTime preloadTimer, elapsedTime(0, 0, 0, 0);
      preloadTimer.start();
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("pre-caching icons from directory"));
      int iconCount = 0;
      int currentMax = qmc2MainWindow->progressBarGamelist->maximum();
      QString oldFormat = qmc2MainWindow->progressBarGamelist->format();
      foreach(QString icoDir, qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/IconDirectory").toString().split(";", QString::SkipEmptyParts)) {
	      qApp->processEvents();
	      QDir iconDirectory(icoDir);
	      QStringList nameFilter;
	      nameFilter << "*.png";
	      QStringList iconFiles = iconDirectory.entryList(nameFilter, QDir::Files | QDir::Readable);
	      if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
		qmc2MainWindow->progressBarGamelist->setFormat(tr("Icon cache - %p%"));
	      else
		qmc2MainWindow->progressBarGamelist->setFormat("%p%");
	      qmc2MainWindow->progressBarGamelist->setRange(0, iconFiles.count());
	      qmc2MainWindow->progressBarGamelist->reset();
	      qApp->processEvents();
	      for (iconCount = 0; iconCount < iconFiles.count(); iconCount++) {
		qmc2MainWindow->progressBarGamelist->setValue(iconCount);
		if ( iconCount % 25 == 0 )
		  qApp->processEvents();
#ifdef QMC2_DEBUG
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Gamelist::loadIcon(): loading %1").arg(iconFiles[iconCount]));
#endif
		QPixmap iconPixmap;
		if ( iconPixmap.load(icoDir + iconFiles[iconCount]) )
		  icon = QIcon(iconPixmap);
		else
		  icon = QIcon();
		qmc2IconMap[iconFiles[iconCount].toLower().remove(".png")] = icon;
		if ( iconCount % qmc2GamelistResponsiveness == 0 ) {
		  qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(true);
		  qApp->processEvents();
		  qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(false);
		}
	      }
      }

      qmc2MainWindow->progressBarGamelist->setRange(0, currentMax);
      if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
	qmc2MainWindow->progressBarGamelist->setFormat(oldFormat);
      else
	qmc2MainWindow->progressBarGamelist->setFormat("%p%");
      elapsedTime = elapsedTime.addMSecs(preloadTimer.elapsed());
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (pre-caching icons from directory, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("%n icon(s) loaded", "", iconCount));
      qmc2IconsPreloaded = true;

      if ( checkOnly )
        qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(true);

      return loadIcon(gameName, item, checkOnly);
    }
  }

  if ( checkOnly )
    qmc2MainWindow->treeWidgetGamelist->setUpdatesEnabled(true);

  return false;
}

#if defined(QMC2_EMUTYPE_MESS) || defined(QMC2_EMUTYPE_UME)
void Gamelist::loadCategoryIni()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::loadCategoryIni()");
#endif

	if ( !mergeCategories ) {
		clearCategoryNames();
		categoryMap.clear();
	}

	QTime loadTimer, elapsedTime(0, 0, 0, 0);
	loadTimer.start();
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading category.ini"));
	qApp->processEvents();

	int currentMax = qmc2MainWindow->progressBarGamelist->maximum();
	QString oldFormat = qmc2MainWindow->progressBarGamelist->format();
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
		qmc2MainWindow->progressBarGamelist->setFormat(tr("Category.ini - %p%"));
	else
		qmc2MainWindow->progressBarGamelist->setFormat("%p%");

	qmc2MainWindow->progressBarGamelist->reset();
	qApp->processEvents();

	QFile categoryIniFile(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/CategoryIni").toString());
	int entryCounter = 0;
	if ( categoryIniFile.open(QFile::ReadOnly) ) {
		qmc2MainWindow->progressBarGamelist->setRange(0, categoryIniFile.size());
		QTextStream tsCategoryIni(&categoryIniFile);
		QString categoryName;
		while ( !tsCategoryIni.atEnd() ) {
			QString categoryLine = tsCategoryIni.readLine().simplified().trimmed();
			qmc2MainWindow->progressBarGamelist->setValue(categoryIniFile.pos());
			if ( categoryLine.isEmpty() )
				continue;
			if ( categoryLine.indexOf(QRegExp("^\\[.*\\]$")) == 0 )
				categoryName = categoryLine.mid(1, categoryLine.length() - 2);
			else if ( !categoryName.isEmpty() ) {
				if ( !categoryNames.contains(categoryName) )
					categoryNames[categoryName] = new QString(categoryName);
				categoryMap.insert(categoryLine, categoryNames[categoryName]);
				entryCounter++;
			}
		}
		categoryIniFile.close();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: can't open '%1' for reading -- no category.ini data available").arg(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/CategoryIni").toString()));

	qmc2MainWindow->progressBarGamelist->setRange(0, currentMax);
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
		qmc2MainWindow->progressBarGamelist->setFormat(oldFormat);
	else
		qmc2MainWindow->progressBarGamelist->setFormat("%p%");

	elapsedTime = elapsedTime.addMSecs(loadTimer.elapsed());
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading category.ini, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("%n category record(s) loaded", "", entryCounter));
}
#endif

void Gamelist::createCategoryView()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::createCategoryView()");
#endif

	qmc2CategoryItemMap.clear();
	qmc2MainWindow->treeWidgetCategoryView->hide();
	qmc2MainWindow->labelCreatingCategoryView->show();

	if ( qmc2ReloadActive && !qmc2StopParser && qmc2MainWindow->stackedWidgetView->currentIndex() == QMC2_VIEW_CATEGORY_INDEX ) {
		QTimer::singleShot(QMC2_RELOAD_POLL_INTERVAL, this, SLOT(createCategoryView()));
		qApp->processEvents();
		return;
	} else if ( qmc2MainWindow->stackedWidgetView->currentIndex() != QMC2_VIEW_CATEGORY_INDEX && qmc2MainWindow->stackedWidgetView->currentIndex() == QMC2_VIEW_CATEGORY_INDEX ) {
		qmc2MainWindow->stackedWidgetView->setCurrentIndex(QMC2_VIEW_CATEGORY_INDEX);
		qmc2MainWindow->stackedWidgetView->update();
		qApp->processEvents();
		QTimer::singleShot(QMC2_RELOAD_POLL_INTERVAL, this, SLOT(createCategoryView()));
		return;
	} else if ( qmc2ReloadActive )
		return;

	qmc2MainWindow->stackedWidgetView->setCurrentIndex(QMC2_VIEW_CATEGORY_INDEX);
	qmc2MainWindow->stackedWidgetView->update();
	qApp->processEvents();

	qmc2MainWindow->treeWidgetCategoryView->setColumnHidden(QMC2_GAMELIST_COLUMN_CATEGORY, true);

	if ( !qmc2StopParser ) {
		qmc2MainWindow->treeWidgetCategoryView->clear();
		QString oldFormat = qmc2MainWindow->progressBarGamelist->format();
		if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
			qmc2MainWindow->progressBarGamelist->setFormat(tr("Category view - %p%"));
		else
			qmc2MainWindow->progressBarGamelist->setFormat("%p%");
		qmc2MainWindow->progressBarGamelist->setRange(0, categoryMap.count());
		qmc2MainWindow->progressBarGamelist->reset();
		QMapIterator<QString, QString *> it(categoryMap);
		int counter = 0;
		bool showDeviceSets = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowDeviceSets", true).toBool();
		bool showBiosSets = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowBiosSets", true).toBool();
		QList<QTreeWidgetItem *> itemList;
		QList<QTreeWidgetItem *> hideList;
		while ( it.hasNext() ) {
			it.next();
			qmc2MainWindow->progressBarGamelist->setValue(counter++);
			QString gameName = it.key();
			if ( gameName.isEmpty() )
				continue;
			if ( !qmc2GamelistItemMap.contains(gameName) )
				continue;
			QTreeWidgetItem *baseItem = qmc2GamelistItemMap[gameName];
			QString *categoryPtr = it.value();
			QString category;
			if ( categoryPtr )
				category = *categoryPtr;
			else
				category = tr("?");
			QList<QTreeWidgetItem *> matchItems = qmc2MainWindow->treeWidgetCategoryView->findItems(category, Qt::MatchExactly);
			QTreeWidgetItem *categoryItem = NULL;
			if ( matchItems.count() > 0 )
				categoryItem = matchItems[0];
			if ( categoryItem == NULL ) {
				categoryItem = new QTreeWidgetItem();
				categoryItem->setText(QMC2_GAMELIST_COLUMN_GAME, category);
				itemList << categoryItem;
			}
			QTreeWidgetItem *gameItem = new GamelistItem(categoryItem);
			bool isBIOS = isBios(gameName);
			bool isDevice = this->isDevice(gameName);
			if ( (isBIOS && !showBiosSets) || (isDevice && !showDeviceSets) )
				hideList << gameItem;
			gameItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
			gameItem->setCheckState(QMC2_GAMELIST_COLUMN_TAG, baseItem->checkState(QMC2_GAMELIST_COLUMN_TAG));
			gameItem->setText(QMC2_GAMELIST_COLUMN_GAME, baseItem->text(QMC2_GAMELIST_COLUMN_GAME));
			gameItem->setText(QMC2_GAMELIST_COLUMN_YEAR, baseItem->text(QMC2_GAMELIST_COLUMN_YEAR));
			gameItem->setText(QMC2_GAMELIST_COLUMN_MANU, baseItem->text(QMC2_GAMELIST_COLUMN_MANU));
			gameItem->setText(QMC2_GAMELIST_COLUMN_NAME, baseItem->text(QMC2_GAMELIST_COLUMN_NAME));
			gameItem->setText(QMC2_GAMELIST_COLUMN_SRCFILE, baseItem->text(QMC2_GAMELIST_COLUMN_SRCFILE));
			gameItem->setText(QMC2_GAMELIST_COLUMN_RTYPES, baseItem->text(QMC2_GAMELIST_COLUMN_RTYPES));
			gameItem->setText(QMC2_GAMELIST_COLUMN_PLAYERS, baseItem->text(QMC2_GAMELIST_COLUMN_PLAYERS));
			gameItem->setText(QMC2_GAMELIST_COLUMN_DRVSTAT, baseItem->text(QMC2_GAMELIST_COLUMN_DRVSTAT));
			gameItem->setText(QMC2_GAMELIST_COLUMN_VERSION, baseItem->text(QMC2_GAMELIST_COLUMN_VERSION));
			if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowROMStatusIcons", true).toBool() ) {
				switch ( gameStatusHash[gameName] ) {
					case 'C':
						if ( isBIOS )
							gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
						else if ( isDevice )
							gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectDeviceImageIcon);
						else
							gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
						break;
					case 'M':
						if ( isBIOS )
							gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectBIOSImageIcon);
						else if ( isDevice )
							gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectDeviceImageIcon);
						else
							gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectImageIcon);
						break;
					case 'I':
						if ( isBIOS )
							gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectBIOSImageIcon);
						else if ( isDevice )
							gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectDeviceImageIcon);
						else
							gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectImageIcon);
						break;
					case 'N':
						if ( isBIOS )
							gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundBIOSImageIcon);
						else if ( isDevice )
							gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundDeviceImageIcon);
						else
							gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundImageIcon);
						break;
					case 'U':
					default:
						if ( isBIOS )
							gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownBIOSImageIcon);
						else if ( isDevice )
							gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownDeviceImageIcon);
						else
							gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownImageIcon);
						break;
				}
			}
			loadIcon(gameName, gameItem);
			qmc2CategoryItemMap[gameName] = gameItem;
			qmc2MainWindow->treeWidgetCategoryView->insertTopLevelItems(0, itemList);
		}
		foreach (QTreeWidgetItem *hiddenItem, hideList)
			hiddenItem->setHidden(true);
		qmc2MainWindow->treeWidgetCategoryView->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
		qmc2MainWindow->progressBarGamelist->reset();
		qmc2MainWindow->progressBarGamelist->setFormat(oldFormat);

		QTimer::singleShot(QMC2_RANK_UPDATE_DELAY, qmc2MainWindow, SLOT(treeWidgetCategoryView_verticalScrollChanged()));
	}

	qmc2MainWindow->labelCreatingCategoryView->hide();
	qmc2MainWindow->treeWidgetCategoryView->show();

	QTimer::singleShot(0, qmc2MainWindow, SLOT(scrollToCurrentItem()));
}

#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
void Gamelist::loadCatverIni()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::loadCatverIni()");
#endif

  clearCategoryNames();
  categoryMap.clear();
  clearVersionNames();
  versionMap.clear();

  QTime loadTimer, elapsedTime(0, 0, 0, 0);
  loadTimer.start();
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading catver.ini"));
  qApp->processEvents();

  int currentMax = qmc2MainWindow->progressBarGamelist->maximum();
  QString oldFormat = qmc2MainWindow->progressBarGamelist->format();
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
    qmc2MainWindow->progressBarGamelist->setFormat(tr("Catver.ini - %p%"));
  else
    qmc2MainWindow->progressBarGamelist->setFormat("%p%");
  qmc2MainWindow->progressBarGamelist->reset();
  qApp->processEvents();

  QFile catverIniFile(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/CatverIni").toString());
  if ( catverIniFile.open(QFile::ReadOnly) ) {
    qmc2MainWindow->progressBarGamelist->setRange(0, catverIniFile.size());
    QTextStream tsCatverIni(&catverIniFile);
    bool isVersion = false, isCategory = false;
    while ( !tsCatverIni.atEnd() ) {
      QString catverLine = tsCatverIni.readLine().simplified().trimmed();
      qmc2MainWindow->progressBarGamelist->setValue(catverIniFile.pos());
      if ( catverLine.isEmpty() )
        continue;
      if ( catverLine.contains("[Category]") ) {
        isCategory = true;
        isVersion = false;
      } else if ( catverLine.contains("[VerAdded]") ) {
        isCategory = false;
        isVersion = true;
      } else {
        QStringList tokens = catverLine.split("=");
        if ( tokens.count() >= 2 ) {
          if ( isCategory ) {
		  if ( !categoryNames.contains(tokens[1]) )
			  categoryNames[tokens[1]] = new QString(tokens[1]);
		  categoryMap.insert(tokens[0], categoryNames[tokens[1]]);
          } else if ( isVersion ) {
            QString verStr = tokens[1];
            if ( verStr.startsWith(".") ) verStr.prepend("0");
	    if ( !versionNames.contains(verStr) )
		    versionNames[verStr] = new QString(verStr);
            versionMap.insert(tokens[0], versionNames[verStr]);
          }
        }
      }
    }
    catverIniFile.close();
  } else
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: can't open '%1' for reading -- no catver.ini data available").arg(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/CatverIni").toString()));

  qmc2MainWindow->progressBarGamelist->setRange(0, currentMax);
  if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
    qmc2MainWindow->progressBarGamelist->setFormat(oldFormat);
  else
    qmc2MainWindow->progressBarGamelist->setFormat("%p%");

  elapsedTime = elapsedTime.addMSecs(loadTimer.elapsed());
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading catver.ini, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("%1 category / %2 version records loaded").arg(categoryMap.count()).arg(versionMap.count()));
}

void Gamelist::createVersionView()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Gamelist::createVersionView()");
#endif

	qmc2VersionItemMap.clear();
	qmc2MainWindow->treeWidgetVersionView->hide();
	qmc2MainWindow->labelCreatingVersionView->show();

	if ( qmc2ReloadActive && !qmc2StopParser && qmc2MainWindow->stackedWidgetView->currentIndex() == QMC2_VIEW_VERSION_INDEX ) {
		QTimer::singleShot(QMC2_RELOAD_POLL_INTERVAL, this, SLOT(createVersionView()));
		qApp->processEvents();
		return;
	} else if ( qmc2MainWindow->stackedWidgetView->currentIndex() != QMC2_VIEW_VERSION_INDEX && qmc2MainWindow->stackedWidgetView->currentIndex() == QMC2_VIEW_VERSION_INDEX ) {
		qmc2MainWindow->stackedWidgetView->setCurrentIndex(QMC2_VIEW_VERSION_INDEX);
		qmc2MainWindow->stackedWidgetView->update();
		qApp->processEvents();
		QTimer::singleShot(QMC2_RELOAD_POLL_INTERVAL, this, SLOT(createVersionView()));
		return;
	} else if ( qmc2ReloadActive )
		return;

	qmc2MainWindow->stackedWidgetView->setCurrentIndex(QMC2_VIEW_VERSION_INDEX);
	qmc2MainWindow->stackedWidgetView->update();
	qApp->processEvents();

	qmc2MainWindow->treeWidgetVersionView->setColumnHidden(QMC2_GAMELIST_COLUMN_VERSION, true);

	if ( !qmc2StopParser ) {
		qmc2MainWindow->treeWidgetVersionView->clear();
		QString oldFormat = qmc2MainWindow->progressBarGamelist->format();
		if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
			qmc2MainWindow->progressBarGamelist->setFormat(tr("Version view - %p%"));
		else
			qmc2MainWindow->progressBarGamelist->setFormat("%p%");
		qmc2MainWindow->progressBarGamelist->setRange(0, versionMap.count());
		qmc2MainWindow->progressBarGamelist->reset();
		QMapIterator<QString, QString *> it(versionMap);
		int counter = 0;
		bool showDeviceSets = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowDeviceSets", true).toBool();
		bool showBiosSets = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowBiosSets", true).toBool();
		QList<QTreeWidgetItem *> itemList;
		QList<QTreeWidgetItem *> hideList;
		while ( it.hasNext() ) {
			it.next();
			qmc2MainWindow->progressBarGamelist->setValue(counter++);
			QString gameName = it.key();
			if ( gameName.isEmpty() )
				continue;
			if ( !qmc2GamelistItemMap.contains(gameName) )
				continue;
			QTreeWidgetItem *baseItem = qmc2GamelistItemMap[gameName];
			QString *versionPtr = it.value();
			QString version;
			if ( versionPtr )
				version = *versionPtr;
			else
				version = tr("?");
			QList<QTreeWidgetItem *> matchItems = qmc2MainWindow->treeWidgetVersionView->findItems(version, Qt::MatchExactly);
			QTreeWidgetItem *versionItem = NULL;
			if ( matchItems.count() > 0 )
				versionItem = matchItems[0];
			if ( versionItem == NULL ) {
				versionItem = new QTreeWidgetItem();
				versionItem->setText(QMC2_GAMELIST_COLUMN_GAME, version);
				itemList << versionItem;
			}
			QTreeWidgetItem *gameItem = new GamelistItem(versionItem);
			bool isBIOS = isBios(gameName);
			bool isDevice = this->isDevice(gameName);
			if ( (isBIOS && !showBiosSets) || (isDevice && !showDeviceSets) )
				hideList << gameItem;
			gameItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
			gameItem->setCheckState(QMC2_GAMELIST_COLUMN_TAG, baseItem->checkState(QMC2_GAMELIST_COLUMN_TAG));
			gameItem->setText(QMC2_GAMELIST_COLUMN_GAME, baseItem->text(QMC2_GAMELIST_COLUMN_GAME));
			gameItem->setText(QMC2_GAMELIST_COLUMN_YEAR, baseItem->text(QMC2_GAMELIST_COLUMN_YEAR));
			gameItem->setText(QMC2_GAMELIST_COLUMN_MANU, baseItem->text(QMC2_GAMELIST_COLUMN_MANU));
			gameItem->setText(QMC2_GAMELIST_COLUMN_NAME, baseItem->text(QMC2_GAMELIST_COLUMN_NAME));
			gameItem->setText(QMC2_GAMELIST_COLUMN_SRCFILE, baseItem->text(QMC2_GAMELIST_COLUMN_SRCFILE));
			gameItem->setText(QMC2_GAMELIST_COLUMN_RTYPES, baseItem->text(QMC2_GAMELIST_COLUMN_RTYPES));
			gameItem->setText(QMC2_GAMELIST_COLUMN_PLAYERS, baseItem->text(QMC2_GAMELIST_COLUMN_PLAYERS));
			gameItem->setText(QMC2_GAMELIST_COLUMN_DRVSTAT, baseItem->text(QMC2_GAMELIST_COLUMN_DRVSTAT));
			gameItem->setText(QMC2_GAMELIST_COLUMN_CATEGORY, baseItem->text(QMC2_GAMELIST_COLUMN_CATEGORY));
			if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Gamelist/ShowROMStatusIcons", true).toBool() ) {
				switch ( gameStatusHash[gameName] ) {
					case 'C':
						if ( isBIOS )
							gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectBIOSImageIcon);
						else if ( isDevice )
							gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectDeviceImageIcon);
						else
							gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2CorrectImageIcon);
						break;
					case 'M':
						if ( isBIOS )
							gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectBIOSImageIcon);
						else if ( isDevice )
							gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectDeviceImageIcon);
						else
							gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2MostlyCorrectImageIcon);
						break;
					case 'I':
						if ( isBIOS )
							gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectBIOSImageIcon);
						else if ( isDevice )
							gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectDeviceImageIcon);
						else
							gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2IncorrectImageIcon);
						break;
					case 'N':
						if ( isBIOS )
							gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundBIOSImageIcon);
						else if ( isDevice )
							gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundDeviceImageIcon);
						else
							gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2NotFoundImageIcon);
						break;
					case 'U':
					default:
						if ( isBIOS )
							gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownBIOSImageIcon);
						else if ( isDevice )
							gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownDeviceImageIcon);
						else
							gameItem->setIcon(QMC2_GAMELIST_COLUMN_GAME, qmc2UnknownImageIcon);
						break;
				}
			}
			loadIcon(gameName, gameItem);
			qmc2VersionItemMap[gameName] = gameItem;
			qmc2MainWindow->treeWidgetVersionView->insertTopLevelItems(0, itemList);
		}
		foreach (QTreeWidgetItem *hiddenItem, hideList)
			hiddenItem->setHidden(true);
		qmc2MainWindow->treeWidgetVersionView->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
		qmc2MainWindow->progressBarGamelist->reset();
		qmc2MainWindow->progressBarGamelist->setFormat(oldFormat);

		QTimer::singleShot(QMC2_RANK_UPDATE_DELAY, qmc2MainWindow, SLOT(treeWidgetVersionView_verticalScrollChanged()));
	}

	qmc2MainWindow->labelCreatingVersionView->hide();
	qmc2MainWindow->treeWidgetVersionView->show();

	QTimer::singleShot(0, qmc2MainWindow, SLOT(scrollToCurrentItem()));
}
#endif

QString Gamelist::romStatus(QString systemName, bool translated)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Gamelist::romStatus(QString systemName = %1, bool translated = %2)").arg(systemName).arg(translated));
#endif

	switch ( gameStatusHash[systemName] ) {
		case 'C':
			if ( translated )
				return tr("correct");
			else
				return "correct";
			break;
		case 'M':
			if ( translated )
				return tr("mostly correct");
			else
				return "mostly correct";
			break;
		case 'I':
			if ( translated )
				return tr("incorrect");
			else
				return "incorrect";
			break;
		case 'N':
			if ( translated )
				return tr("not found");
			else
				return "not found";
			break;
		case 'U':
		default:
			if ( translated )
				return tr("unknown");
			else
				return "unknown";
			break;
	}

}

char Gamelist::romState(QString systemName)
{
	char state = gameStatusHash[systemName];
	return (state == 0 ? 'U' : state);
}

QString Gamelist::lookupDriverName(QString systemName)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Gamelist::lookupDriverName(QString systemName = %1)").arg(systemName));
#endif

	QString driverName = driverNameHash[systemName];

	if ( driverName.isEmpty() ) {
		QString xml = xmlDb()->xml(systemName).simplified();
		if ( !xml.isEmpty() ) {
			int startIndex = xml.indexOf("sourcefile=\"");
			if ( startIndex > 0 ) {
				startIndex += 12;
				int endIndex = xml.indexOf("\"", startIndex);
				driverName = xml.mid(startIndex, endIndex - startIndex);
				driverNameHash[systemName] = driverName;
			}
		}
	}

	return driverName;
}

void Gamelist::clearCategoryNames()
{
	foreach (QString *category, categoryNames)
		if ( category )
			delete category;
	categoryNames.clear();
}

#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
void Gamelist::clearVersionNames()
{
	foreach (QString *version, versionNames)
		if ( version )
			delete version;
	versionNames.clear();
}
#endif

bool GamelistItem::operator<(const QTreeWidgetItem &otherItem) const
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: GamelistItem::operator<(const GamelistItem &otherItem = ...)");
#endif
  
  switch ( qmc2SortCriteria ) {
    case QMC2_SORT_BY_DESCRIPTION:
      return (text(QMC2_GAMELIST_COLUMN_GAME).toUpper() < otherItem.text(QMC2_GAMELIST_COLUMN_GAME).toUpper());

    case QMC2_SORT_BY_ROM_STATE:
      return (qmc2Gamelist->gameStatusHash[text(QMC2_GAMELIST_COLUMN_NAME)] < qmc2Gamelist->gameStatusHash[otherItem.text(QMC2_GAMELIST_COLUMN_NAME)]);

    case QMC2_SORT_BY_TAG:
      return (int(checkState(QMC2_GAMELIST_COLUMN_TAG)) < int(otherItem.checkState(QMC2_GAMELIST_COLUMN_TAG)));

    case QMC2_SORT_BY_YEAR:
      return (text(QMC2_GAMELIST_COLUMN_YEAR) < otherItem.text(QMC2_GAMELIST_COLUMN_YEAR));

    case QMC2_SORT_BY_MANUFACTURER:
      return (text(QMC2_GAMELIST_COLUMN_MANU).toUpper() < otherItem.text(QMC2_GAMELIST_COLUMN_MANU).toUpper());

    case QMC2_SORT_BY_NAME:
      return (text(QMC2_GAMELIST_COLUMN_NAME).toUpper() < otherItem.text(QMC2_GAMELIST_COLUMN_NAME).toUpper());

    case QMC2_SORT_BY_ROMTYPES:
      return (text(QMC2_GAMELIST_COLUMN_RTYPES).toUpper() < otherItem.text(QMC2_GAMELIST_COLUMN_RTYPES).toUpper());

    case QMC2_SORT_BY_PLAYERS:
      return (text(QMC2_GAMELIST_COLUMN_PLAYERS).toUpper() < otherItem.text(QMC2_GAMELIST_COLUMN_PLAYERS).toUpper());

    case QMC2_SORT_BY_DRVSTAT:
      return (text(QMC2_GAMELIST_COLUMN_DRVSTAT).toUpper() < otherItem.text(QMC2_GAMELIST_COLUMN_DRVSTAT).toUpper());

    case QMC2_SORT_BY_SRCFILE:
      return (text(QMC2_GAMELIST_COLUMN_SRCFILE).toUpper() < otherItem.text(QMC2_GAMELIST_COLUMN_SRCFILE).toUpper());

    case QMC2_SORT_BY_CATEGORY:
      return (text(QMC2_GAMELIST_COLUMN_CATEGORY).toUpper() < otherItem.text(QMC2_GAMELIST_COLUMN_CATEGORY).toUpper());

#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
    case QMC2_SORT_BY_VERSION:
      return (text(QMC2_GAMELIST_COLUMN_VERSION).toUpper() < otherItem.text(QMC2_GAMELIST_COLUMN_VERSION).toUpper());
#endif

    default:
      return false;
  }
}
