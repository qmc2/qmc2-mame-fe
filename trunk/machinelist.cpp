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
#include <QFileInfoList>
#include <QFontMetrics>
#include <QFont>
#include <QTimer>
#include <QMap>
#include <QSet>
#include <QDir>
#include <QBitArray>
#include <QByteArray>
#include <QCryptographicHash>
#include <QApplication>
#include <QChar>

#include "machinelist.h"
#include "imagewidget.h"
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
#include "deviceconfigurator.h"
#include "demomode.h"
#include "softwarelist.h"
#if defined(QMC2_YOUTUBE_ENABLED)
#include "youtubevideoplayer.h"
#endif
#include "htmleditor/htmleditor.h"
#include "aspectratiolabel.h"
#include "processmanager.h"
#if defined(QMC2_LIBARCHIVE_ENABLED)
#include "archivefile.h"
#endif

// external global variables
extern MainWindow *qmc2MainWindow;
extern Options *qmc2Options;
extern Settings *qmc2Config;
extern EmulatorOptions *qmc2EmulatorOptions;
extern ROMStatusExporter *qmc2ROMStatusExporter;
extern ROMAlyzer *qmc2SystemROMAlyzer;
extern ROMAlyzer *qmc2SoftwareROMAlyzer;
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
extern bool qmc2ForceCacheRefresh;
extern bool qmc2SortingActive;
extern int qmc2MachineListResponsiveness;
extern Preview *qmc2Preview;
extern Flyer *qmc2Flyer;
extern Cabinet *qmc2Cabinet;
extern Controller *qmc2Controller;
extern Marquee *qmc2Marquee;
extern Title *qmc2Title;
extern PCB *qmc2PCB;
extern QTreeWidgetItem *qmc2CurrentItem;
extern QTreeWidgetItem *qmc2LastGameInfoItem;
extern QTreeWidgetItem *qmc2LastEmuInfoItem;
extern QTreeWidgetItem *qmc2LastSoftwareListItem;
extern QTreeWidgetItem *qmc2LastDeviceConfigItem;
extern DeviceConfigurator *qmc2DeviceConfigurator;
extern SoftwareList *qmc2SoftwareList;
extern QHash<QString, QStringList> systemSoftwareListHash;
extern QHash<QString, QStringList> systemSoftwareFilterHash;
extern QHash<QString, QTreeWidgetItem *> qmc2MachineListItemHash;
extern QHash<QString, QTreeWidgetItem *> qmc2HierarchyItemHash;
extern QHash<QString, QString> qmc2ParentHash;
extern int qmc2SortCriteria;
extern Qt::SortOrder qmc2SortOrder;
extern QBitArray qmc2Filter;
extern QMap<QString, unzFile> qmc2IconFileMap;
extern QMap<QString, SevenZipFile *> qmc2IconFileMap7z;
#if defined(QMC2_LIBARCHIVE_ENABLED)
extern QMap<QString, ArchiveFile *> qmc2IconArchiveMap;
#endif
extern QHash<QString, QIcon> qmc2IconHash;
extern QTreeWidgetItem *qmc2LastProjectMESSItem;
extern MiniWebBrowser *qmc2ProjectMESSLookup;
extern QHash<QString, QTreeWidgetItem *> qmc2CategoryItemHash;
extern QHash<QString, QTreeWidgetItem *> qmc2VersionItemHash;
extern DemoModeDialog *qmc2DemoModeDialog;
#if defined(QMC2_YOUTUBE_ENABLED)
extern YouTubeVideoPlayer *qmc2YouTubeWidget;
extern QTreeWidgetItem *qmc2LastYouTubeItem;
#endif
extern HtmlEditor *qmc2SystemNotesEditor;
extern HtmlEditor *qmc2SoftwareNotesEditor;
extern QList<QTreeWidgetItem *> qmc2ExpandedMachineListItems;
extern MachineList *qmc2MachineList;
extern bool qmc2TemplateCheck;
extern bool qmc2ParentImageFallback;
extern QTime qmc2StartupTimer;

QStringList MachineList::phraseTranslatorList;
QStringList MachineList::romTypeNames;
QHash<QString, QString> MachineList::reverseTranslation;
QHash<QString, QString> MachineList::machineStateTranslations;
bool MachineList::creatingCatView = false;
bool MachineList::creatingVerView = false;
QString MachineList::trQuestionMark;
QString MachineList::trWaitingForData;
Qt::ItemFlags MachineListItem::defaultItemFlags = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled;

MachineList::MachineList(QObject *parent)
	: QObject(parent)
{
	numMachines = numTotalMachines = numCorrectMachines = numMostlyCorrectMachines = numIncorrectMachines = numUnknownMachines = numNotFoundMachines = -1;
	uncommittedXmlDbRows = numTaggedSets = numMatchedMachines = numVerifyRoms = 0;
	loadProc = verifyProc = 0;
	checkedItem = 0;
	emulatorVersion = tr("unknown");
	mergeCategories = autoRomCheck = verifyCurrentOnly = dtdBufferReady = false;
	initialLoad = true;

	qmc2UnknownImageIcon = QIcon(QString::fromUtf8(":/data/img/sphere_blue.png"));
	qmc2UnknownBIOSImageIcon = QIcon(QString::fromUtf8(":/data/img/sphere_blue_bios.png"));
	qmc2UnknownDeviceImageIcon = QIcon(QString::fromUtf8(":/data/img/sphere_blue_device.png"));
	qmc2CorrectImageIcon = QIcon(QString::fromUtf8(":/data/img/sphere_green.png"));
	qmc2CorrectBIOSImageIcon = QIcon(QString::fromUtf8(":/data/img/sphere_green_bios.png"));
	qmc2CorrectDeviceImageIcon = QIcon(QString::fromUtf8(":/data/img/sphere_green_device.png"));
	qmc2MostlyCorrectImageIcon = QIcon(QString::fromUtf8(":/data/img/sphere_yellowgreen.png"));
	qmc2MostlyCorrectBIOSImageIcon = QIcon(QString::fromUtf8(":/data/img/sphere_yellowgreen_bios.png"));
	qmc2MostlyCorrectDeviceImageIcon = QIcon(QString::fromUtf8(":/data/img/sphere_yellowgreen_device.png"));
	qmc2IncorrectImageIcon = QIcon(QString::fromUtf8(":/data/img/sphere_red.png"));
	qmc2IncorrectBIOSImageIcon = QIcon(QString::fromUtf8(":/data/img/sphere_red_bios.png"));
	qmc2IncorrectDeviceImageIcon = QIcon(QString::fromUtf8(":/data/img/sphere_red_device.png"));
	qmc2NotFoundImageIcon = QIcon(QString::fromUtf8(":/data/img/sphere_grey.png"));
	qmc2NotFoundBIOSImageIcon = QIcon(QString::fromUtf8(":/data/img/sphere_grey_bios.png"));
	qmc2NotFoundDeviceImageIcon = QIcon(QString::fromUtf8(":/data/img/sphere_grey_device.png"));

	// translation look-up hashes/maps
	phraseTranslatorList << tr("good") << tr("bad") << tr("preliminary") << tr("supported") << tr("unsupported")
		<< tr("imperfect") << tr("yes") << tr("no") << tr("baddump") << tr("nodump")
		<< tr("vertical") << tr("horizontal") << tr("raster") << tr("unknown") << tr("Unknown") 
		<< tr("On") << tr("Off") << tr("audio") << tr("unused") << tr("Unused") << tr("cpu")
		<< tr("vector") << tr("lcd") << tr("joy4way") << tr("joy8way") << tr("trackball")
		<< tr("joy2way") << tr("doublejoy8way") << tr("dial") << tr("paddle") << tr("pedal")
		<< tr("stick") << tr("vjoy2way") << tr("lightgun") << tr("doublejoy4way") << tr("vdoublejoy2way")
		<< tr("doublejoy2way") << tr("printer") << tr("cdrom") << tr("cartridge") << tr("cassette")
		<< tr("quickload") << tr("floppydisk") << tr("serial") << tr("snapshot") << tr("original")
		<< tr("compatible") << tr("N/A");
	reverseTranslation[tr("good")] = "good";
	reverseTranslation[tr("bad")] = "bad";
	reverseTranslation[tr("preliminary")] = "preliminary";
	reverseTranslation[tr("supported")] = "supported";
	reverseTranslation[tr("unsupported")] = "unsupported";
	reverseTranslation[tr("imperfect")] = "imperfect";
	reverseTranslation[QObject::tr("yes")] = "yes";
	reverseTranslation[QObject::tr("no")] = "no";
	reverseTranslation[QObject::tr("partially")] = "partially";
	trQuestionMark = tr("?");
	trWaitingForData = tr("Waiting for data...");
	machineStateTranslations["good"] = tr("good");
	machineStateTranslations["preliminary"] = tr("preliminary");
	machineStateTranslations["imperfect"] = tr("imperfect");
	machineStateTranslations["N/A"] = tr("N/A");
	romTypeNames << "--" << tr("ROM") << tr("CHD") << tr("ROM, CHD");

	// identifier strings (see "mame -help") that we support - others will produce a warning
	emulatorIdentifiers << "MAME" << "M.A.M.E." << "HBMAME" << "HB.M.A.M.E." << "MESS" << "M.E.S.S.";

	switch ( qmc2Options->iconFileType() ) {
		case QMC2_ICON_FILETYPE_ZIP:
			foreach (QString filePath, qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/IconFile").toString().split(";", QString::SkipEmptyParts)) {
				unzFile iconFile = unzOpen(filePath.toUtf8().constData());
				if ( iconFile == 0 )
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open icon file, please check access permissions for %1").arg(filePath));
				else
					qmc2IconFileMap[filePath] = iconFile;
			}
			break;
		case QMC2_ICON_FILETYPE_7Z:
			foreach (QString filePath, qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/IconFile").toString().split(";", QString::SkipEmptyParts)) {
				SevenZipFile *iconFile = new SevenZipFile(filePath);
				if ( !iconFile->open() ) {
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open icon file %1").arg(filePath) + " - " + tr("7z error") + ": " + iconFile->lastError());
					delete iconFile;
				} else
					qmc2IconFileMap7z[filePath] = iconFile;
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
					qmc2IconArchiveMap[filePath] = archiveFile;
			}
			break;
#endif
	}

	m_xmlDb = new XmlDatabaseManager(this);
	xmlDb()->setSyncMode(QMC2_DB_SYNC_MODE_OFF);
	xmlDb()->setJournalMode(QMC2_DB_JOURNAL_MODE_MEMORY);
	m_userDataDb = new UserDataDatabaseManager(this);
	userDataDb()->setSyncMode(QMC2_DB_SYNC_MODE_OFF);
	userDataDb()->setJournalMode(QMC2_DB_JOURNAL_MODE_MEMORY);
	m_datInfoDb = new DatInfoDatabaseManager(this);
	datInfoDb()->setSyncMode(QMC2_DB_SYNC_MODE_OFF);
	datInfoDb()->setJournalMode(QMC2_DB_JOURNAL_MODE_MEMORY);
}

MachineList::~MachineList()
{
	if ( loadProc )
		loadProc->kill();
	if ( verifyProc )
		verifyProc->kill();

	clearCategoryNames();
	categoryHash.clear();
	clearVersionNames();
	versionHash.clear();

	foreach (unzFile iconFile, qmc2IconFileMap)
		unzClose(iconFile);
	foreach (SevenZipFile *iconFile, qmc2IconFileMap7z) {
		iconFile->close();
		delete iconFile;
	}
#if defined(QMC2_LIBARCHIVE_ENABLED)
	foreach (ArchiveFile *iconFile, qmc2IconArchiveMap) {
		iconFile->close();
		delete iconFile;
	}
#endif

	QString connectionName;

	connectionName = m_xmlDb->connectionName();
	delete m_xmlDb;
	QSqlDatabase::removeDatabase(connectionName);

	connectionName = m_userDataDb->connectionName();
	delete m_userDataDb;
	QSqlDatabase::removeDatabase(connectionName);
}

void MachineList::enableWidgets(bool enable)
{
	static bool lastEnable = true;

	// store widget enablement flag for later dialog setups
	qmc2WidgetsEnabled = enable;

	if ( enable && qmc2MainWindow->labelLoadingMachineList->isVisible() ) {
		// show machine list / hide loading animation
		qmc2MainWindow->loadAnimMovie->setPaused(true);
		qmc2MainWindow->labelLoadingMachineList->setVisible(false);
		qmc2MainWindow->treeWidgetMachineList->setVisible(true);
		qmc2MainWindow->labelLoadingHierarchy->setVisible(false);
		qmc2MainWindow->treeWidgetHierarchy->setVisible(true);
	}

	// avoid redundant operations
	if ( lastEnable == enable )
		return;
	lastEnable = enable;

	qmc2Options->toolButtonBrowseStyleSheet->setEnabled(enable);
	qmc2Options->toolButtonBrowseFont->setEnabled(enable);
	qmc2Options->toolButtonBrowseLogFont->setEnabled(enable);
	qmc2Options->toolButtonBrowseTemporaryFile->setEnabled(enable);
	qmc2Options->toolButtonBrowseFrontendLogFile->setEnabled(enable);
	qmc2Options->toolButtonBrowsePreviewDirectory->setEnabled(enable);
	qmc2Options->toolButtonBrowsePreviewFile->setEnabled(enable);
	qmc2Options->toolButtonBrowseDataDirectory->setEnabled(enable);
	qmc2Options->toolButtonBrowseDatInfoDatabase->setEnabled(enable);
	qmc2Options->toolButtonBrowseMameHistoryDat->setEnabled(enable);
	qmc2Options->toolButtonBrowseMessSysinfoDat->setEnabled(enable);
	qmc2Options->checkBoxProcessMameHistoryDat->setEnabled(enable);
	qmc2Options->checkBoxProcessMessSysinfoDat->setEnabled(enable);
	qmc2Options->toolButtonBrowseMameInfoDat->setEnabled(enable);
	qmc2Options->toolButtonBrowseMessInfoDat->setEnabled(enable);
	qmc2Options->checkBoxProcessMameInfoDat->setEnabled(enable);
	qmc2Options->checkBoxProcessMessInfoDat->setEnabled(enable);
	qmc2Options->toolButtonBrowseSoftwareInfoDB->setEnabled(enable);
	qmc2Options->checkBoxProcessSoftwareInfoDB->setEnabled(enable);
	qmc2Options->toolButtonImportGameInfo->setEnabled(enable);
	qmc2Options->toolButtonImportMachineInfo->setEnabled(enable);
	qmc2Options->toolButtonImportMameInfo->setEnabled(enable);
	qmc2Options->toolButtonImportMessInfo->setEnabled(enable);
	qmc2Options->toolButtonImportSoftwareInfo->setEnabled(enable);
	qmc2Options->toolButtonBrowseCatverIniFile->setEnabled(enable);
	qmc2Options->checkBoxUseCatverIni->setEnabled(enable);
	qmc2Options->toolButtonBrowseCategoryIniFile->setEnabled(enable);
	qmc2Options->checkBoxUseCategoryIni->setEnabled(enable);
	qmc2Options->checkBoxShowROMStatusIcons->setEnabled(enable);
	qmc2Options->checkBoxRomStateFilter->setEnabled(enable);
	qmc2Options->checkBoxShowBiosSets->setEnabled(enable);
	qmc2Options->checkBoxShowDeviceSets->setEnabled(enable);
	qmc2Options->toolButtonBrowseSoftwareListCacheDb->setEnabled(enable);
	qmc2Options->toolButtonBrowseSoftwareStateCache->setEnabled(enable);
	qmc2Options->toolButtonBrowseGeneralSoftwareFolder->setEnabled(enable);
	qmc2Options->toolButtonBrowseExecutableFile->setEnabled(enable);
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
	qmc2Options->toolButtonBrowseMachineListCacheFile->setEnabled(enable);
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
	qmc2Options->toolButtonBrowseVideoSnapFolder->setEnabled(enable);
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
	qmc2Options->checkBoxParentImageFallback->setEnabled(enable);
	qmc2Options->pushButtonIndividualFallbackSettings->setEnabled(enable && qmc2Options->checkBoxParentImageFallback->isChecked());
	qmc2Options->checkBoxStandardColorPalette->setEnabled(enable);
	qmc2Options->pushButtonEditPalette->setEnabled(enable && !qmc2Options->checkBoxStandardColorPalette->isChecked());
	qmc2Options->pushButtonAdditionalArtworkSetup->setEnabled(enable);
	qmc2Options->pushButtonImageFormats->setEnabled(enable);
	for (int row = 0; row < qmc2Options->tableWidgetRegisteredEmulators->rowCount(); row++) {
		QWidget *w = qmc2Options->tableWidgetRegisteredEmulators->cellWidget(row, QMC2_ADDTLEMUS_COLUMN_ICON);
		if ( w )
			w->setEnabled(enable);
		w = qmc2Options->tableWidgetRegisteredEmulators->cellWidget(row, QMC2_ADDTLEMUS_COLUMN_CUID);
		if ( w )
			w->setEnabled(enable);
	}
#if QMC2_USE_PHONON_API || QMC2_MULTIMEDIA_ENABLED
	qmc2MainWindow->toolButtonAudioAddTracks->setEnabled(enable);
	qmc2MainWindow->toolButtonAudioAddURL->setEnabled(enable);
#endif
	if ( qmc2ROMStatusExporter )
		qmc2ROMStatusExporter->pushButtonExport->setEnabled(enable);
	if ( qmc2SystemROMAlyzer ) {
		qmc2SystemROMAlyzer->pushButtonAnalyze->setEnabled(enable);
		qmc2SystemROMAlyzer->toolButtonToolsMenu->setEnabled(enable);
		qmc2SystemROMAlyzer->toolButtonBrowseBackupFolder->setEnabled(qmc2SystemROMAlyzer->checkBoxCreateBackups->isChecked() && enable);
		if ( qmc2SystemROMAlyzer->groupBoxCHDManager->isChecked() ) {
			qmc2SystemROMAlyzer->toolButtonBrowseCHDManagerExecutableFile->setEnabled(enable);
			qmc2SystemROMAlyzer->toolButtonBrowseTemporaryWorkingDirectory->setEnabled(enable);
		}
		if ( qmc2SystemROMAlyzer->groupBoxSetRewriter->isChecked() ) {
			qmc2SystemROMAlyzer->toolButtonBrowseSetRewriterOutputPath->setEnabled(enable);
			qmc2SystemROMAlyzer->toolButtonBrowseSetRewriterAdditionalRomPath->setEnabled(qmc2SystemROMAlyzer->checkBoxSetRewriterUseAdditionalRomPath->isChecked() && enable);
		}
		if ( qmc2SystemROMAlyzer->groupBoxCheckSumDatabase->isChecked() ) {
			qmc2SystemROMAlyzer->toolButtonBrowseCheckSumDbDatabasePath->setEnabled(enable);
			qmc2SystemROMAlyzer->toolButtonCheckSumDbAddPath->setEnabled(enable);
		}
	} else {
		qmc2MainWindow->actionSystemROMAlyzer->setEnabled(enable);
		qmc2MainWindow->actionAnalyseCurrentROM->setEnabled(enable);
		qmc2MainWindow->actionAnalyseROMTagged->setEnabled(enable);
		foreach (QAction *action, qmc2MainWindow->criticalActions)
			action->setEnabled(enable);
	}
	if ( qmc2SoftwareROMAlyzer ) {
		qmc2SoftwareROMAlyzer->pushButtonAnalyze->setEnabled(enable);
		qmc2SoftwareROMAlyzer->toolButtonToolsMenu->setEnabled(enable);
		qmc2SoftwareROMAlyzer->toolButtonBrowseBackupFolder->setEnabled(qmc2SoftwareROMAlyzer->checkBoxCreateBackups->isChecked() && enable);
		if ( qmc2SoftwareROMAlyzer->groupBoxCHDManager->isChecked() ) {
			qmc2SoftwareROMAlyzer->toolButtonBrowseCHDManagerExecutableFile->setEnabled(enable);
			qmc2SoftwareROMAlyzer->toolButtonBrowseTemporaryWorkingDirectory->setEnabled(enable);
		}
		if ( qmc2SoftwareROMAlyzer->groupBoxSetRewriter->isChecked() ) {
			qmc2SoftwareROMAlyzer->toolButtonBrowseSetRewriterOutputPath->setEnabled(enable);
			qmc2SoftwareROMAlyzer->toolButtonBrowseSetRewriterAdditionalRomPath->setEnabled(qmc2SoftwareROMAlyzer->checkBoxSetRewriterUseAdditionalRomPath->isChecked() && enable);
		}
		if ( qmc2SoftwareROMAlyzer->groupBoxCheckSumDatabase->isChecked() ) {
			qmc2SoftwareROMAlyzer->toolButtonBrowseCheckSumDbDatabasePath->setEnabled(enable);
			qmc2SoftwareROMAlyzer->toolButtonCheckSumDbAddPath->setEnabled(enable);
		}
	} else
		qmc2MainWindow->actionSoftwareROMAlyzer->setEnabled(enable);
	qmc2MainWindow->toolButtonSelectRomFilter->setEnabled(enable);
	qmc2MainWindow->actionLaunchArcade->setEnabled(enable);
	qmc2MainWindow->actionArcadeSetup->setEnabled(enable);
}

void MachineList::load()
{
	QString userScopePath(Options::configPath());
	QString machineName;
	if ( qmc2CurrentItem )
		machineName = qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_NAME);
	if ( qmc2DemoModeDialog )
		qmc2DemoModeDialog->saveCategoryFilter();
	qmc2ReloadActive = qmc2EarlyReloadActive = true;
	qmc2StopParser = false;
	machineStatusHash.clear();
	qmc2MachineListItemHash.clear();
	qmc2HierarchyItemHash.clear();
	qmc2CategoryItemHash.clear();
	qmc2VersionItemHash.clear();
	qmc2ExpandedMachineListItems.clear();
	biosSets.clear();
	deviceSets.clear();
	userDataDb()->clearRankCache();
	userDataDb()->clearCommentCache();
	enableWidgets(false);
	qmc2MainWindow->stackedWidgetSpecial_setCurrentIndex(QMC2_SPECIAL_DEFAULT_PAGE);
	numMachines = numTotalMachines = numCorrectMachines = numMostlyCorrectMachines = numIncorrectMachines = numUnknownMachines = numNotFoundMachines = -1;
	numTaggedSets = numMatchedMachines = 0;
	qmc2MainWindow->treeWidgetMachineList->clear();
	qmc2MainWindow->treeWidgetHierarchy->clear();
	qmc2MainWindow->treeWidgetCategoryView->clear();
	qmc2MainWindow->treeWidgetVersionView->clear();
	qmc2MainWindow->listWidgetSearch->clear();
	qmc2MainWindow->listWidgetFavorites->clear();
	qmc2MainWindow->listWidgetPlayed->clear();
	qmc2MainWindow->textBrowserGameInfo->clear();
	qmc2MainWindow->textBrowserEmuInfo->clear();
	qmc2MainWindow->labelMachineStatus->setPalette(MainWindow::qmc2StatusColorBlue);
	qmc2CurrentItem = 0;
	QTreeWidgetItem *dummyItem;
	dummyItem = new QTreeWidgetItem(qmc2MainWindow->treeWidgetMachineList);
	dummyItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, trWaitingForData);
	dummyItem = new QTreeWidgetItem(qmc2MainWindow->treeWidgetHierarchy);
	dummyItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, trWaitingForData);
	dummyItem = new QTreeWidgetItem(qmc2MainWindow->treeWidgetCategoryView);
	dummyItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, trWaitingForData);
	dummyItem = new QTreeWidgetItem(qmc2MainWindow->treeWidgetVersionView);
	dummyItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, trWaitingForData);
	qmc2MainWindow->labelMachineListStatus->setText(status());
	ImageWidget::updateArtwork();
	if ( qmc2DeviceConfigurator ) {
		qmc2DeviceConfigurator->save();
		qmc2DeviceConfigurator->saveSetup();
		qmc2DeviceConfigurator->setVisible(false);
		QLayout *vbl = qmc2MainWindow->tabDevices->layout();
		if ( vbl )
			delete vbl;
		delete qmc2DeviceConfigurator;
		qmc2DeviceConfigurator = 0;
	}
	qmc2LastDeviceConfigItem = 0;
	DeviceConfigurator::systemSlotHash.clear();
	DeviceConfigurator::slotNameHash.clear();
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
			qmc2LastSoftwareListItem = 0;
			QTimer::singleShot(0, this, SLOT(load()));
			return;
		}
		qmc2SoftwareList->save();
		qmc2SoftwareList->setVisible(false);
		QLayout *vbl = qmc2MainWindow->tabSoftwareList->layout();
		if ( vbl )
			delete vbl;
		delete qmc2SoftwareList;
		qmc2SoftwareList = 0;
	}
	qmc2LastSoftwareListItem = 0;
	SoftwareList::swlSupported = true;
	systemSoftwareListHash.clear();
	systemSoftwareFilterHash.clear();
	qmc2LastGameInfoItem = 0;
	qmc2LastEmuInfoItem = 0;
	if ( qmc2ProjectMESSLookup ) {
		qmc2ProjectMESSLookup->setVisible(false);
		QLayout *vbl = qmc2MainWindow->tabProjectMESS->layout();
		if ( vbl )
			delete vbl;
		delete qmc2ProjectMESSLookup;
		qmc2ProjectMESSLookup = 0;
	}
	qmc2LastProjectMESSItem = 0;
#if defined(QMC2_YOUTUBE_ENABLED)
	qmc2LastYouTubeItem = 0;
	if ( qmc2YouTubeWidget ) {
		qmc2YouTubeWidget->setVisible(false);
		QLayout *vbl = qmc2MainWindow->tabYouTube->layout();
		if ( vbl )
			delete vbl;
		delete qmc2YouTubeWidget;
		qmc2YouTubeWidget = 0;
	}
#endif
	if ( qmc2EmulatorOptions ) {
		qmc2EmulatorOptions->save();
		QLayout *vbl = qmc2MainWindow->tabConfiguration->layout();
		if ( vbl )
			delete vbl;
		delete qmc2MainWindow->labelEmuSelector;
		if ( !machineName.isEmpty() ) {
			QString selectedEmulator = qmc2MainWindow->comboBoxEmuSelector->currentText();
			if ( selectedEmulator == tr("Default") || selectedEmulator.isEmpty() )
				qmc2Config->remove(QString(QMC2_EMULATOR_PREFIX + "Configuration/%1/SelectedEmulator").arg(machineName));
			else
				qmc2Config->setValue(QString(QMC2_EMULATOR_PREFIX + "Configuration/%1/SelectedEmulator").arg(machineName), selectedEmulator);
		}
		delete qmc2MainWindow->comboBoxEmuSelector;
		qmc2MainWindow->comboBoxEmuSelector = 0;
		delete qmc2EmulatorOptions;
		delete qmc2MainWindow->pushButtonCurrentEmulatorOptionsExportToFile;
		delete qmc2MainWindow->pushButtonCurrentEmulatorOptionsImportFromFile;
		qmc2EmulatorOptions = 0;
	}
	if ( qmc2MainWindow->tabWidgetMachineList->indexOf(qmc2MainWindow->tabMachineList) == qmc2MainWindow->tabWidgetMachineList->currentIndex() ) {
		switch ( qmc2MainWindow->stackedWidgetView->currentIndex() ) {
			case QMC2_VIEWCATEGORY_INDEX:
				QTimer::singleShot(0, qmc2MainWindow, SLOT(viewByCategory()));
				break;
			case QMC2_VIEWVERSION_INDEX:
				QTimer::singleShot(0, qmc2MainWindow, SLOT(viewByVersion()));
				break;
			default:
				break;
		}
	}
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("determining emulator version and supported sets"));
	QStringList args;
	QTime elapsedTime(0, 0, 0, 0);
	parseTimer.start();
	// emulator version
	QProcess commandProc;
	args << "-help";
	bool commandProcStarted = false;
	int retries = 0;
	QString execFile(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ExecutableFile").toString());
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
			while ( commandProcRunning && !commandProc.waitForFinished(QMC2_PROCESS_POLL_TIME) ) {
				qApp->processEvents();
				commandProcRunning = (commandProc.state() == QProcess::Running);
			}
		} else {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't start MAME executable within a reasonable time frame, giving up") + " (" + tr("error text = %1").arg(ProcessManager::errorText(commandProc.error())) + ")");
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
	if ( commandProcStarted ) {
		QString s(commandProc.readAllStandardOutput());
#if defined(QMC2_OS_WIN)
		s.replace("\r\n", "\n"); // convert WinDOS's "0x0D 0x0A" to just "0x0A" 
#endif
		QStringList versionLines(s.split('\n'));
		QStringList versionWords(versionLines.first().split(' '));
		if ( versionWords.count() > 1 ) {
			emulatorVersion = versionWords[1].remove('v');
			if ( emulatorIdentifiers.contains(versionWords.at(0)) )
				emulatorType = "MAME";
			else {
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the selected emulator executable cannot be identified as MAME"));
				emulatorType = versionWords.at(0);
			}
		} else {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the selected emulator executable cannot be identified as MAME"));
			emulatorVersion = tr("unknown");
			emulatorType = tr("unknown");
		}
	} else {
		emulatorVersion = tr("unknown");
		emulatorType = tr("unknown");
	}
	// supported (non-device) sets
	args.clear();
	args << "-listfull";
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
		while ( commandProcRunning && !commandProc.waitForFinished(QMC2_PROCESS_POLL_TIME) ) {
			qApp->processEvents();
			commandProcRunning = (commandProc.state() == QProcess::Running);
		}
	} else {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't start MAME executable within a reasonable time frame, giving up") + " (" + tr("error text = %1").arg(ProcessManager::errorText(commandProc.error())) + ")");
		qmc2ReloadActive = qmc2EarlyReloadActive = false;
		qmc2StopParser = true;
		return;
	}
	QString listfullSha1;
	if ( commandProcStarted ) {
		QCryptographicHash sha1(QCryptographicHash::Sha1);
		QString lfOutput(commandProc.readAllStandardOutput());
		numTotalMachines = lfOutput.count('\n') - 1;
		sha1.addData(lfOutput.toUtf8().constData());
		listfullSha1 = sha1.result().toHex();
		elapsedTime = elapsedTime.addMSecs(parseTimer.elapsed());
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (determining emulator version and supported sets, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
	}
	qmc2MainWindow->labelMachineListStatus->setText(status());
	if ( emulatorVersion != tr("unknown") )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("emulator info: type = %1, version = %2").arg(emulatorType).arg(emulatorVersion));
	else {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: couldn't determine emulator type and version"));
		qmc2ReloadActive = false;
		enableWidgets(true);
		return;
	}
	if ( numTotalMachines > 0 )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("%n supported (non-device) set(s)", "", numTotalMachines));
	else {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: couldn't determine the number of supported sets"));
		qmc2ReloadActive = false;
		enableWidgets(true);
		return;
	}
	if ( qmc2Config->contains(QMC2_EMULATOR_PREFIX + "ListfullSha1") && qmc2Config->value(QMC2_EMULATOR_PREFIX + "ListfullSha1", QString()).toString() != listfullSha1 ) {
		if ( !QMC2_CLI_OPT_CLEAR_ALL_CACHES && qmc2Config->value(QMC2_EMULATOR_PREFIX + "AutoClearEmuCaches", true).toBool() ) {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the output from -listfull changed, forcing a refresh of all emulator caches"));
			qmc2ForceCacheRefresh = true;
			qmc2MainWindow->on_actionClearAllEmulatorCaches_triggered();
			qmc2ForceCacheRefresh = false;
		}
	}
	qmc2Config->setValue(QMC2_EMULATOR_PREFIX + "ListfullSha1", listfullSha1);
	categoryHash.clear();
	versionHash.clear();
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "MachineList/UseCatverIni", false).toBool() ) {
		loadCatverIni();
		mergeCategories = true;
	}
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "MachineList/UseCategoryIni", false).toBool() )
		loadCategoryIni();
	mergeCategories = false;
	if ( qmc2DemoModeDialog )
		QTimer::singleShot(0, qmc2DemoModeDialog, SLOT(updateCategoryFilter()));
	qmc2EarlyReloadActive = false;
	if ( qmc2StopParser ) {
		qmc2MainWindow->progressBarMachineList->reset();
		qmc2ReloadActive = false;
		enableWidgets(true);
		return;
	}
	if ( !initialLoad ) {
		// hide machine list / show loading animation
		qmc2MainWindow->treeWidgetMachineList->setVisible(false);
		((AspectRatioLabel *)qmc2MainWindow->labelLoadingMachineList)->setLabelText(tr("Loading, please wait..."));
		qmc2MainWindow->labelLoadingMachineList->setVisible(true);
		qmc2MainWindow->treeWidgetHierarchy->setVisible(false);
		((AspectRatioLabel *)qmc2MainWindow->labelLoadingHierarchy)->setLabelText(tr("Loading, please wait..."));
		qmc2MainWindow->labelLoadingHierarchy->setVisible(true);
		if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ShowLoadingAnimation", true).toBool() )
			qmc2MainWindow->loadAnimMovie->start();
		qApp->processEvents();
	}
	if ( emulatorVersion == xmlDb()->emulatorVersion() && xmlDb()->xmlRowCount() > 0 ) {
		parse();
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading favorites and play history"));
		loadFavorites();
		loadPlayHistory();
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading favorites and play history)"));
		if ( initialLoad ) {
			QTime startupTime(0, 0, 0, 0);
			startupTime = startupTime.addMSecs(qmc2StartupTimer.elapsed());
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("total start-up time: %1").arg(startupTime.toString("mm:ss.zzz")));
			initialLoad = false;
		}
		// show machine list / hide loading animation
		qmc2MainWindow->loadAnimMovie->setPaused(true);
		qmc2MainWindow->labelLoadingMachineList->setVisible(false);
		qmc2MainWindow->treeWidgetMachineList->setVisible(true);
		qmc2MainWindow->labelLoadingHierarchy->setVisible(false);
		qmc2MainWindow->treeWidgetHierarchy->setVisible(true);
		if ( qmc2MainWindow->tabWidgetMachineList->indexOf(qmc2MainWindow->tabMachineList) == qmc2MainWindow->tabWidgetMachineList->currentIndex() ) {
			if ( qApp->focusWidget() != qmc2MainWindow->comboBoxToolbarSearch ) {
				switch ( qmc2MainWindow->stackedWidgetView->currentIndex() ) {
					case QMC2_VIEWHIERARCHY_INDEX:
						qmc2MainWindow->treeWidgetHierarchy->setFocus();
						break;
					case QMC2_VIEWCATEGORY_INDEX:
						qmc2MainWindow->treeWidgetCategoryView->setFocus();
						break;
					case QMC2_VIEWVERSION_INDEX:
						qmc2MainWindow->treeWidgetVersionView->setFocus();
						break;
					case QMC2_VIEWMACHINELIST_INDEX:
					default:
						qmc2MainWindow->treeWidgetMachineList->setFocus();
						break;
				}
			}
			switch ( qmc2MainWindow->stackedWidgetView->currentIndex() ) {
				case QMC2_VIEWHIERARCHY_INDEX:
					qmc2MainWindow->treeWidgetHierarchy_verticalScrollChanged();
					break;
				case QMC2_VIEWCATEGORY_INDEX:
					qmc2MainWindow->treeWidgetCategoryView_verticalScrollChanged();
					break;
				case QMC2_VIEWVERSION_INDEX:
					qmc2MainWindow->treeWidgetVersionView_verticalScrollChanged();
					break;
				case QMC2_VIEWMACHINELIST_INDEX:
				default:
					qmc2MainWindow->treeWidgetMachineList_verticalScrollChanged();
					break;
			}
		}
	} else {
		loadTimer.start();
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading XML data and recreating cache"));
		if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
			qmc2MainWindow->progressBarMachineList->setFormat(tr("XML data - %p%"));
		else
			qmc2MainWindow->progressBarMachineList->setFormat("%p%");
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
		connect(loadProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(loadFinished(int, QProcess::ExitStatus)));
		connect(loadProc, SIGNAL(readyReadStandardOutput()), this, SLOT(loadReadyReadStandardOutput()));
		connect(loadProc, SIGNAL(started()), this, SLOT(loadStarted()));
		loadProc->setProcessChannelMode(QProcess::MergedChannels);
		loadProc->start(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ExecutableFile").toString(), args);
	}
	userDataDb()->setEmulatorVersion(emulatorVersion);
	userDataDb()->setQmc2Version(XSTR(QMC2_VERSION));
	userDataDb()->setUserDataVersion(QMC2_USERDATA_VERSION);
}

void MachineList::verify(bool currentOnly)
{
	if ( currentOnly )
		if ( !qmc2CurrentItem )
			return;
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
		romStateCache.setFileName(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ROMStateCacheFile").toString());
		romStateCache.open(QIODevice::WriteOnly | QIODevice::Text);
		if ( !romStateCache.isOpen() ) {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: can't open ROM state cache for writing, path = %1").arg(romStateCache.fileName()));
			qmc2VerifyActive = false;
			enableWidgets(true);
			return;
		} else {
			tsRomCache.setDevice(&romStateCache);
			tsRomCache.reset();
			tsRomCache << "# THIS FILE IS AUTO-GENERATED - PLEASE DO NOT EDIT!\n";
		}
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("verifying ROM status for '%1'").arg(checkedItem->text(QMC2_MACHINELIST_COLUMN_MACHINE)));
		oldRomState = machineStatusHash.value(checkedItem->text(QMC2_MACHINELIST_COLUMN_NAME));
		// decrease counter for current game's/machine's state
		switch ( oldRomState ) {
			case 'C':
				numCorrectMachines--;
				numUnknownMachines++;
				break;
			case 'M':
				numMostlyCorrectMachines--;
				numUnknownMachines++;
				break;
			case 'I':
				numIncorrectMachines--;
				numUnknownMachines++;
				break;
			case 'N':
				numNotFoundMachines--;
				numUnknownMachines++;
				break;
			case 'U':
			default:
				break;
		}
	} else {
		checkedItem = 0;
		romStateCache.setFileName(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ROMStateCacheFile").toString());
		romStateCache.open(QIODevice::WriteOnly | QIODevice::Text);
		if ( !romStateCache.isOpen() ) {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: can't open ROM state cache for writing, path = %1").arg(romStateCache.fileName()));
			qmc2VerifyActive = false;
			enableWidgets(true);
			return;
		} else {
			tsRomCache.setDevice(&romStateCache);
			tsRomCache.reset();
			tsRomCache << "# THIS FILE IS AUTO-GENERATED - PLEASE DO NOT EDIT!\n";
		}
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("verifying ROM status for all sets"));
		numCorrectMachines = numMostlyCorrectMachines = numIncorrectMachines = numNotFoundMachines = numUnknownMachines = 0;
		qmc2MainWindow->labelMachineListStatus->setText(status());
		if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
			qmc2MainWindow->progressBarMachineList->setFormat(tr("ROM check - %p%"));
		else
			qmc2MainWindow->progressBarMachineList->setFormat("%p%");
		qmc2MainWindow->progressBarMachineList->setRange(0, numTotalMachines + deviceSets.count());
		qmc2MainWindow->progressBarMachineList->reset();
	}
	QStringList args;
	QString command = qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ExecutableFile").toString();
	if ( qmc2Config->contains(QMC2_EMULATOR_PREFIX + "Configuration/Global/rompath") )
		args << "-rompath" << QString("\"%1\"").arg(qmc2Config->value(QMC2_EMULATOR_PREFIX + "Configuration/Global/rompath").toString().replace("~", "$HOME"));
	args << "-verifyroms";
	if ( verifyCurrentOnly )
		args << checkedItem->text(QMC2_MACHINELIST_COLUMN_NAME);
	verifyProc = new QProcess(this);
	connect(verifyProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(verifyFinished(int, QProcess::ExitStatus)));
	connect(verifyProc, SIGNAL(readyReadStandardOutput()), this, SLOT(verifyReadyReadStandardOutput()));
	connect(verifyProc, SIGNAL(started()), this, SLOT(verifyStarted()));
	if ( !qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory", QString()).toString().isEmpty() )
		verifyProc->setWorkingDirectory(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/WorkingDirectory").toString());
	verifyProc->setProcessChannelMode(QProcess::MergedChannels);
	verifyProc->start(command, args);
}

QString MachineList::value(QString element, QString attribute, bool translate)
{
	QString attributePattern(" " + attribute + "=\"");
	if ( element.contains(attributePattern) ) {
		QString valueString(element.remove(0, element.indexOf(attributePattern) + attributePattern.length()));
		valueString = valueString.remove(valueString.indexOf("\""), valueString.lastIndexOf(">")).replace("&amp;", "&").replace("&lt;", "<").replace("&gt;", ">").replace("&quot;", "\"").replace("&apos;", "'");
		if ( valueString == ">" )
			return QString::null;
		if ( translate )
			return tr(valueString.toUtf8().constData());
		else
			return valueString;
	} else
		return QString::null;
}

void MachineList::insertAttributeItems(QTreeWidgetItem *parent, QString element, QStringList attributes, QStringList descriptions, bool translate)
{
	QList<QTreeWidgetItem *> itemList;
	for (int i = 0; i < attributes.count(); i++) {
		QString valueString = value(element, attributes.at(i), translate);
		if ( !valueString.isEmpty() ) {
			QTreeWidgetItem *attributeItem = new QTreeWidgetItem();
			attributeItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, descriptions.at(i));
			attributeItem->setText(QMC2_MACHINELIST_COLUMN_ICON, tr(valueString.toUtf8().constData()));
			itemList << attributeItem;
		}
	}
	parent->addChildren(itemList);
}

void MachineList::insertAttributeItems(QList<QTreeWidgetItem *> *itemList, QString element, QStringList attributes, QStringList descriptions, bool translate)
{
	for (int i = 0; i < attributes.count(); i++) {
		QString valueString = value(element, attributes.at(i), translate);
		if ( !valueString.isEmpty() ) {
			QTreeWidgetItem *attributeItem = new QTreeWidgetItem();
			attributeItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, descriptions.at(i));
			attributeItem->setText(QMC2_MACHINELIST_COLUMN_ICON, tr(valueString.toUtf8().constData()));
			itemList->append(attributeItem);
		}
	}
}

void MachineList::parseMachineDetail(QTreeWidgetItem *item)
{
	QString machineName = item->text(QMC2_MACHINELIST_COLUMN_NAME);
	QStringList xmlLines = xmlDb()->xml(machineName).split("\n", QString::SkipEmptyParts);
	if ( xmlLines.count() < 2 ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: couldn't find machine information for '%1'").arg(machineName));
		return;
	}
	int gamePos = 1;
	item->child(0)->setText(QMC2_MACHINELIST_COLUMN_MACHINE, tr("Updating"));
	qmc2MainWindow->treeWidgetMachineList->viewport()->repaint();
	qApp->processEvents();
	QString element, content;
	QStringList attributes, descriptions;
	QTreeWidgetItem *childItem = 0;
	QList<QTreeWidgetItem *> itemList;

	attributes << "name" << "sourcefile" << "isbios" << "isdevice" << "runnable" << "cloneof" << "romof" << "sampleof";
	descriptions << tr("Name") << tr("Source file") << tr("Is BIOS?") << tr("Is device?") << tr("Runnable") << tr("Clone of") << tr("ROM of") << tr("Sample of");
	element = xmlLines.at(gamePos - 1).simplified();
	insertAttributeItems(&itemList, element, attributes, descriptions, true);
	QString endMark("</machine>");

	while ( !xmlLines[gamePos].contains(endMark) ) {
		childItem = 0;
		element = xmlLines[gamePos].simplified();
		if ( element.contains("<year>") ) {
			content = element.remove("<year>").remove("</year>");
			childItem = new QTreeWidgetItem();
			childItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, tr("Year"));
			childItem->setText(QMC2_MACHINELIST_COLUMN_ICON, content);
		}
		if ( element.contains("<manufacturer>") ) {
			content = element.remove("<manufacturer>").remove("</manufacturer>");
			content.replace("&amp;", "&").replace("&lt;", "<").replace("&gt;", ">").replace("&quot;", "\"").replace("&apos;", "'");
			childItem = new QTreeWidgetItem();
			childItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, tr("Manufacturer"));
			childItem->setText(QMC2_MACHINELIST_COLUMN_ICON, content);
		}
		if ( element.contains("<rom ") ) {
			childItem = new QTreeWidgetItem();
			childItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, tr("ROM"));
			childItem->setText(QMC2_MACHINELIST_COLUMN_ICON, value(element, "name"));
			attributes.clear();
			attributes << "bios" << "size" << "crc" << "sha1" << "merge" << "region" << "offset" << "status" << "optional";
			descriptions.clear();
			descriptions << tr("BIOS") << tr("Size") << tr("CRC") << tr("SHA-1") << tr("Merge") << tr("Region") << tr("Offset") << tr("Status") << tr("Optional");
			insertAttributeItems(childItem, element, attributes, descriptions, true);
		}
		if ( element.contains("<device_ref ") ) {
			childItem = new QTreeWidgetItem();
			childItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, tr("Device reference"));
			childItem->setText(QMC2_MACHINELIST_COLUMN_ICON, value(element, "name"));
		}
		if ( element.contains("<chip ") ) {
			childItem = new QTreeWidgetItem();
			childItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, tr("Chip"));
			childItem->setText(QMC2_MACHINELIST_COLUMN_ICON, value(element, "name"));
			attributes.clear();
			attributes << "tag" << "type" << "clock";
			descriptions.clear();
			descriptions << tr("Tag") << tr("Type") << tr("Clock");
			insertAttributeItems(childItem, element, attributes, descriptions, true);
		}
		if ( element.contains("<display ") ) {
			childItem = new QTreeWidgetItem();
			childItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, tr("Display"));
			attributes.clear();
			attributes << "type" << "rotate" << "flipx" << "width" << "height" << "refresh" << "pixclock" << "htotal" << "hbend" << "hbstart" << "vtotal" << "vbend" << "vbstart";
			descriptions.clear();
			descriptions << tr("Type") << tr("Rotate") << tr("Flip-X") << tr("Width") << tr("Height") << tr("Refresh") << tr("Pixel clock") << tr("H-Total") << tr("H-Bend") << tr("HB-Start") << tr("V-Total") << tr("V-Bend") << tr("VB-Start");
			insertAttributeItems(childItem, element, attributes, descriptions, true);
		}
		if ( element.contains("<sound ") ) {
			childItem = new QTreeWidgetItem();
			childItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, tr("Sound"));
			attributes.clear();
			attributes << "channels";
			descriptions.clear();
			descriptions << tr("Channels");
			insertAttributeItems(childItem, element, attributes, descriptions, true);
		}
		if ( element.contains("<input ") ) {
			childItem = new QTreeWidgetItem();
			childItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, tr("Input"));
			attributes.clear();
			attributes << "service" << "tilt" << "players" << "buttons" << "coins";
			descriptions.clear();
			descriptions << tr("Service") << tr("Tilt") << tr("Players") << tr("Buttons") << tr("Coins");
			insertAttributeItems(childItem, element, attributes, descriptions, true);
			gamePos++;
			while ( xmlLines[gamePos].contains("<control ") ) {
				QString subElement = xmlLines[gamePos].simplified();
				QTreeWidgetItem *nextChildItem = new QTreeWidgetItem(childItem);
				nextChildItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, tr("Control"));
				nextChildItem->setText(QMC2_MACHINELIST_COLUMN_ICON, value(subElement, "type", true));
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
			childItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, tr("DIP switch"));
			childItem->setText(QMC2_MACHINELIST_COLUMN_ICON, value(element, "name", true));
			gamePos++;
			while ( xmlLines[gamePos].contains("<dipvalue ") ) {
				QString subElement = xmlLines[gamePos].simplified();
				QTreeWidgetItem *secondChildItem = new QTreeWidgetItem(childItem);
				secondChildItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, tr("DIP value"));
				secondChildItem->setText(QMC2_MACHINELIST_COLUMN_ICON, value(subElement, "name", true));
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
			childItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, tr("Configuration"));
			childItem->setText(QMC2_MACHINELIST_COLUMN_ICON, value(element, "name", true));
			attributes.clear();
			attributes << "tag" << "mask";
			descriptions.clear();
			descriptions << tr("Tag") << tr("Mask");
			insertAttributeItems(childItem, element, attributes, descriptions, true);
			gamePos++;
			while ( xmlLines[gamePos].contains("<confsetting ") ) {
				QString subElement = xmlLines[gamePos].simplified();
				QTreeWidgetItem *secondChildItem = new QTreeWidgetItem(childItem);
				secondChildItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, tr("Setting"));
				secondChildItem->setText(QMC2_MACHINELIST_COLUMN_ICON, value(subElement, "name", true));
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
			childItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, tr("Driver"));
			attributes.clear();
			attributes << "status" << "emulation" << "color" << "sound" << "graphic" << "cocktail" << "protection" << "savestate" << "palettesize";
			descriptions.clear();
			descriptions << tr("Status") << tr("Emulation") << tr("Color") << tr("Sound") << tr("Graphic") << tr("Cocktail") << tr("Protection") << tr("Save state") << tr("Palette size");
			insertAttributeItems(childItem, element, attributes, descriptions, true);
		}
		if ( element.contains("<biosset ") ) {
			childItem = new QTreeWidgetItem();
			childItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, tr("BIOS set"));
			childItem->setText(QMC2_MACHINELIST_COLUMN_ICON, value(element, "name"));
			attributes.clear();
			attributes << "description" << "default";
			descriptions.clear();
			descriptions << tr("Description") << tr("Default");
			insertAttributeItems(childItem, element, attributes, descriptions, true);
		}
		if ( element.contains("<sample ") ) {
			childItem = new QTreeWidgetItem();
			childItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, tr("Sample"));
			childItem->setText(QMC2_MACHINELIST_COLUMN_ICON, value(element, "name"));
		}
		if ( element.contains("<disk ") ) {
			childItem = new QTreeWidgetItem();
			childItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, tr("Disk"));
			childItem->setText(QMC2_MACHINELIST_COLUMN_ICON, value(element, "name"));
			attributes.clear();
			attributes << "md5" << "sha1" << "merge" << "region" << "index" << "status" << "optional";
			descriptions.clear();
			descriptions << tr("MD5") << tr("SHA-1") << tr("Merge") << tr("Region") << tr("Index") << tr("Status") << tr("Optional");
			insertAttributeItems(childItem, element, attributes, descriptions, true);
		}
		if ( element.contains("<adjuster ") ) {
			childItem = new QTreeWidgetItem();
			childItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, tr("Adjuster"));
			childItem->setText(QMC2_MACHINELIST_COLUMN_ICON, value(element, "name"));
			attributes.clear();
			attributes << "default";
			descriptions.clear();
			descriptions << tr("Default");
			insertAttributeItems(childItem, element, attributes, descriptions, true);
		}
		if ( element.contains("<softwarelist ") ) {
			childItem = new QTreeWidgetItem();
			childItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, tr("Software list"));
			childItem->setText(QMC2_MACHINELIST_COLUMN_ICON, value(element, "name"));
			attributes.clear();
			attributes << "status";
			descriptions.clear();
			descriptions << tr("Status");
			insertAttributeItems(childItem, element, attributes, descriptions, true);
		}
		if ( element.contains("<category ") ) {
			childItem = new QTreeWidgetItem();
			childItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, tr("Category"));
			childItem->setText(QMC2_MACHINELIST_COLUMN_ICON, value(element, "name", true));
			gamePos++;
			while ( xmlLines[gamePos].contains("<item ") ) {
				QString subElement = xmlLines[gamePos].simplified();
				QTreeWidgetItem *secondChildItem = new QTreeWidgetItem(childItem);
				secondChildItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, tr("Item"));
				secondChildItem->setText(QMC2_MACHINELIST_COLUMN_ICON, value(subElement, "name", true));
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
			childItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, tr("Device"));
			childItem->setText(QMC2_MACHINELIST_COLUMN_ICON, value(element, "type", true));
			attributes.clear();
			attributes << "tag" << "mandatory" << "interface";
			descriptions.clear();
			descriptions << tr("Tag") << tr("Mandatory") << tr("Interface");
			insertAttributeItems(childItem, element, attributes, descriptions, false);
			gamePos++;
			while ( xmlLines[gamePos].contains("<instance ") ) {
				QString subElement = xmlLines[gamePos].simplified();
				QTreeWidgetItem *secondChildItem = new QTreeWidgetItem(childItem);
				secondChildItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, tr("Instance"));
				secondChildItem->setText(QMC2_MACHINELIST_COLUMN_ICON, value(subElement, "name", false));
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
				secondChildItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, tr("Extension"));
				secondChildItem->setText(QMC2_MACHINELIST_COLUMN_ICON, value(subElement, "name", false));
				gamePos++;
			}
		}
		if ( element.contains("<ramoption") ) {
			childItem = new QTreeWidgetItem();
			childItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, tr("RAM options"));
			while ( xmlLines[gamePos].contains("<ramoption") ) {
				QString subElement = xmlLines[gamePos].simplified();
				QTreeWidgetItem *secondChildItem = new QTreeWidgetItem(childItem);
				secondChildItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, tr("Option"));
				int fromIndex = subElement.indexOf('>') + 1;
				int toIndex = subElement.indexOf('<', fromIndex);
				QString ramOptionValue = subElement.mid(fromIndex, toIndex - fromIndex);
				secondChildItem->setText(QMC2_MACHINELIST_COLUMN_ICON, ramOptionValue);
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
	qmc2MainWindow->treeWidgetMachineList->setUpdatesEnabled(false);
	delete item->takeChild(0);
	item->addChildren(itemList);
	qmc2MainWindow->treeWidgetMachineList->setUpdatesEnabled(true);
}

void MachineList::parse()
{
	if ( qmc2StopParser ) {
		qmc2ReloadActive = false;
		enableWidgets(true);
		return;
	}
	bool showROMStatusIcons = qmc2Config->value(QMC2_FRONTEND_PREFIX + "MachineList/ShowROMStatusIcons", true).toBool();
	bool showDeviceSets = qmc2Config->value(QMC2_FRONTEND_PREFIX + "MachineList/ShowDeviceSets", true).toBool();
	bool showBiosSets = qmc2Config->value(QMC2_FRONTEND_PREFIX + "MachineList/ShowBiosSets", true).toBool();
	QTime elapsedTime(0, 0, 0, 0);
	qmc2MainWindow->progressBarMachineList->setRange(0, numTotalMachines);
	romStateCache.setFileName(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/ROMStateCacheFile").toString());
	romStateCache.open(QIODevice::ReadOnly | QIODevice::Text);
	if ( !romStateCache.isOpen() ) {
		if ( !autoRomCheck )
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: can't open ROM state cache, please check ROMs"));
	} else {
		parseTimer.start();
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading ROM state from cache"));
		tsRomCache.setDevice(&romStateCache);
		tsRomCache.reset();
		tsRomCache.readLine(); // ignore the first line
		QChar splitChar(' ');
		while ( !tsRomCache.atEnd() ) {
			QStringList words(tsRomCache.readLine().split(splitChar, QString::SkipEmptyParts));
			machineStatusHash.insert(words.at(QMC2_RSC_INDEX_NAME), words.at(QMC2_RSC_INDEX_STATE).at(0).toLatin1());
		}
		numCorrectMachines = numMostlyCorrectMachines = numIncorrectMachines = numNotFoundMachines = 0;
		elapsedTime = elapsedTime.addMSecs(parseTimer.elapsed());
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading ROM state from cache, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("%n cached ROM state(s) loaded", "", machineStatusHash.count()));
		romStateCache.close();
	}
	QTime processMachineListElapsedTimer(0, 0, 0, 0);
	parseTimer.start();
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("processing machine list"));
	qmc2MainWindow->treeWidgetMachineList->clear();
	qmc2ParentHash.clear();
	hierarchyHash.clear();
	qmc2MainWindow->progressBarMachineList->reset();
	machineListCache.setFileName(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/MachineListCacheFile").toString());
	machineListCache.open(QIODevice::ReadOnly | QIODevice::Text);
	bool reparseMachineList = true;
	bool romStateCacheUpdate = false;
	bool loadedFromCache = false;
	QTime machineListCacheElapsedTime(0, 0, 0, 0);
	QList<QTreeWidgetItem *> itemList;
	QHash<QTreeWidgetItem *, bool> hiddenItemHash;
	QString trSystemBios(tr("System / BIOS"));
	QString trSystemDevice(tr("System / Device"));
	QChar columnSplitChar('\t');
	QChar lineSplitChar('\n');
	if ( machineListCache.isOpen() ) {
		tsMachineListCache.setDevice(&machineListCache);
		tsMachineListCache.setCodec(QTextCodec::codecForName("UTF-8"));
		tsMachineListCache.seek(0);
		if ( !tsMachineListCache.atEnd() ) {
			QString line(tsMachineListCache.readLine());
			while ( line.startsWith('#') && !tsMachineListCache.atEnd() )
				line = tsMachineListCache.readLine();
			QStringList words(line.split(columnSplitChar));
			if ( words.count() >= 2 ) {
				if ( words.at(0).compare("MAME_VERSION") == 0 )
					romStateCacheUpdate = reparseMachineList = (words.at(1).compare(emulatorVersion) != 0);
				else
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: couldn't determine emulator version of machine list cache"));
				if ( words.count() < 4 ) {
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("INFORMATION: the machine list cache will now be updated due to a new format"));
					reparseMachineList = true;
				} else {
					if ( words.at(3).toInt() < QMC2_MLC_VERSION ) {
						qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("INFORMATION: the machine list cache will now be updated due to a new format"));
						reparseMachineList = true;
					}
				}
			}
		} else {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the machine list cache is invalid, forcing a refresh"));
			reparseMachineList = true;
		}
		bool useCatverIni = qmc2Config->value(QMC2_FRONTEND_PREFIX + "MachineList/UseCatverIni", false).toBool();
		bool useCategories = useCatverIni | qmc2Config->value(QMC2_FRONTEND_PREFIX + "MachineList/UseCategoryIni", false).toBool();
		if ( !reparseMachineList && !qmc2StopParser ) {
			loadIcon(QString(), 0); // initiates icon pre-caching
			qmc2MainWindow->progressBarMachineList->setRange(0, numTotalMachines * 2);
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading machine data from machine list cache"));
			if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
				qmc2MainWindow->progressBarMachineList->setFormat(tr("Machine data - %p%"));
			else
				qmc2MainWindow->progressBarMachineList->setFormat("%p%");
			miscTimer.start();
			numMachines = numUnknownMachines = 0;
			qmc2MainWindow->progressBarMachineList->setValue(0);
			QString readBuffer;
			QChar one('1');
			while ( (!tsMachineListCache.atEnd() || !readBuffer.isEmpty()) && !qmc2StopParser ) {
				readBuffer.append(tsMachineListCache.read(QMC2_FILE_BUFFER_SIZE));
				bool endsWithNewLine = readBuffer.endsWith(lineSplitChar);
				QStringList lines(readBuffer.split(lineSplitChar, QString::SkipEmptyParts));
				int lc = endsWithNewLine ? lines.count() : lines.count() - 1;
				for (int l = 0; l < lc; l++) {
					QStringList machineData(lines.at(l).split(columnSplitChar));
					QString machineName(machineData.at(QMC2_MLC_INDEX_NAME));
					QString machineCloneOf(machineData.at(QMC2_MLC_INDEX_CLONEOF));
					QString machinePlayers(machineData.at(QMC2_MLC_INDEX_PLAYERS));
					QString machineDrvStat(machineData.at(QMC2_MLC_INDEX_DRVSTAT));
					int machineType = int(machineData.at(QMC2_MLC_INDEX_IS_BIOS).compare(one) == 0) + int(machineData.at(QMC2_MLC_INDEX_IS_DEVICE).compare(one) == 0) * 2; // 0: normal, 1: BIOS, 2: device
					MachineListItem *machineItem = new MachineListItem();
					qmc2MachineListItemHash.insert(machineName, machineItem);
					machineItem->setFlags(MachineListItem::defaultItemFlags);
					machineItem->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, Qt::Unchecked);
					if ( machineCloneOf.isEmpty() ) {
						if ( !hierarchyHash.contains(machineName) )
							hierarchyHash.insert(machineName, QStringList());
					} else
						hierarchyHash[machineCloneOf].append(machineName);
					machineItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, machineData.at(QMC2_MLC_INDEX_MACHINE));
					machineItem->setText(QMC2_MACHINELIST_COLUMN_YEAR, machineData.at(QMC2_MLC_INDEX_YEAR));
					machineItem->setText(QMC2_MACHINELIST_COLUMN_MANU, machineData.at(QMC2_MLC_INDEX_MANU));
					machineItem->setText(QMC2_MACHINELIST_COLUMN_NAME, machineName);
					machineItem->setText(QMC2_MACHINELIST_COLUMN_SRCFILE, machineData.at(QMC2_MLC_INDEX_SRCFILE));
					machineItem->setText(QMC2_MACHINELIST_COLUMN_RTYPES, romTypeNames.at(int(machineData.at(QMC2_MLC_INDEX_HAS_ROM).compare(one) == 0) + int(machineData.at(QMC2_MLC_INDEX_HAS_CHD).compare(one) == 0) * 2));
					if ( useCatverIni ) {
						QString *versionString = versionHash.value(machineName);
						machineItem->setText(QMC2_MACHINELIST_COLUMN_VERSION, versionString ? *versionString : trQuestionMark);
					}
					switch ( machineStatusHash.value(machineName) ) {
						case 'C': 
							numCorrectMachines++;
							switch ( machineType ) {
								case QMC2_MACHINETYPE_NORMAL:
									if ( useCategories ) {
										QString *categoryString = categoryHash.value(machineName);
										machineItem->setText(QMC2_MACHINELIST_COLUMN_CATEGORY, categoryString ? *categoryString : trQuestionMark);
									}
									machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectImageIcon);
									break;
								case QMC2_MACHINETYPE_BIOS:
									if ( showROMStatusIcons )
										machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectBIOSImageIcon);
									if ( !showBiosSets )
										hiddenItemHash.insert(machineItem, true);
									if ( useCategories )
										machineItem->setText(QMC2_MACHINELIST_COLUMN_CATEGORY, trSystemBios);
									biosSets.insert(machineName, true);
									break;
								case QMC2_MACHINETYPE_DEVICE:
									if ( showROMStatusIcons )
										machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectDeviceImageIcon);
									if ( !showDeviceSets )
										hiddenItemHash.insert(machineItem, true);
									if ( useCategories )
										machineItem->setText(QMC2_MACHINELIST_COLUMN_CATEGORY, trSystemDevice);
									deviceSets.insert(machineName, true);
									machinePlayers = machineDrvStat = "N/A";
									break;
							}
							break;
						case 'M': 
							numMostlyCorrectMachines++;
							switch ( machineType ) {
								case QMC2_MACHINETYPE_NORMAL:
									if ( useCategories ) {
										QString *categoryString = categoryHash.value(machineName);
										machineItem->setText(QMC2_MACHINELIST_COLUMN_CATEGORY, categoryString ? *categoryString : trQuestionMark);
									}
									machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2MostlyCorrectImageIcon);
									break;
								case QMC2_MACHINETYPE_BIOS:
									if ( showROMStatusIcons )
										machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2MostlyCorrectBIOSImageIcon);
									if ( !showBiosSets )
										hiddenItemHash.insert(machineItem, true);
									if ( useCategories )
										machineItem->setText(QMC2_MACHINELIST_COLUMN_CATEGORY, trSystemBios);
									biosSets.insert(machineName, true);
									break;
								case QMC2_MACHINETYPE_DEVICE:
									if ( showROMStatusIcons )
										machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2MostlyCorrectDeviceImageIcon);
									if ( !showDeviceSets )
										hiddenItemHash.insert(machineItem, true);
									if ( useCategories )
										machineItem->setText(QMC2_MACHINELIST_COLUMN_CATEGORY, trSystemDevice);
									deviceSets.insert(machineName, true);
									machinePlayers = machineDrvStat = "N/A";
									break;
							}
							break;
						case 'I':
							numIncorrectMachines++;
							switch ( machineType ) {
								case QMC2_MACHINETYPE_NORMAL:
									if ( useCategories ) {
										QString *categoryString = categoryHash.value(machineName);
										machineItem->setText(QMC2_MACHINELIST_COLUMN_CATEGORY, categoryString ? *categoryString : trQuestionMark);
									}
									machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2IncorrectImageIcon);
									break;
								case QMC2_MACHINETYPE_BIOS:
									if ( showROMStatusIcons )
										machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2IncorrectBIOSImageIcon);
									if ( !showBiosSets )
										hiddenItemHash.insert(machineItem, true);
									if ( useCategories )
										machineItem->setText(QMC2_MACHINELIST_COLUMN_CATEGORY, trSystemBios);
									biosSets.insert(machineName, true);
									break;
								case QMC2_MACHINETYPE_DEVICE:
									if ( showROMStatusIcons )
										machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2IncorrectDeviceImageIcon);
									if ( !showDeviceSets )
										hiddenItemHash.insert(machineItem, true);
									if ( useCategories )
										machineItem->setText(QMC2_MACHINELIST_COLUMN_CATEGORY, trSystemDevice);
									deviceSets.insert(machineName, true);
									machinePlayers = machineDrvStat = "N/A";
									break;
							}
							break;
						case 'N':
							numNotFoundMachines++;
							switch ( machineType ) {
								case QMC2_MACHINETYPE_NORMAL:
									if ( useCategories ) {
										QString *categoryString = categoryHash.value(machineName);
										machineItem->setText(QMC2_MACHINELIST_COLUMN_CATEGORY, categoryString ? *categoryString : trQuestionMark);
									}
									machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2NotFoundImageIcon);
									break;
								case QMC2_MACHINETYPE_BIOS:
									if ( showROMStatusIcons )
										machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2NotFoundBIOSImageIcon);
									if ( !showBiosSets )
										hiddenItemHash.insert(machineItem, true);
									if ( useCategories )
										machineItem->setText(QMC2_MACHINELIST_COLUMN_CATEGORY, trSystemBios);
									biosSets.insert(machineName, true);
									break;
								case QMC2_MACHINETYPE_DEVICE:
									if ( showROMStatusIcons )
										machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2NotFoundDeviceImageIcon);
									if ( !showDeviceSets )
										hiddenItemHash.insert(machineItem, true);
									if ( useCategories )
										machineItem->setText(QMC2_MACHINELIST_COLUMN_CATEGORY, trSystemDevice);
									deviceSets.insert(machineName, true);
									machinePlayers = machineDrvStat = "N/A";
									break;
							}
							break;
						default:
							numUnknownMachines++;
							machineStatusHash.insert(machineName, 'U');
							switch ( machineType ) {
								case QMC2_MACHINETYPE_NORMAL:
									if ( useCategories ) {
										QString *categoryString = categoryHash.value(machineName);
										machineItem->setText(QMC2_MACHINELIST_COLUMN_CATEGORY, categoryString ? *categoryString : trQuestionMark);
									}
									machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2UnknownImageIcon);
									break;
								case QMC2_MACHINETYPE_BIOS:
									if ( showROMStatusIcons )
										machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2UnknownBIOSImageIcon);
									if ( !showBiosSets )
										hiddenItemHash.insert(machineItem, true);
									if ( useCategories )
										machineItem->setText(QMC2_MACHINELIST_COLUMN_CATEGORY, trSystemBios);
									biosSets.insert(machineName, true);
									break;
								case QMC2_MACHINETYPE_DEVICE:
									if ( showROMStatusIcons )
										machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2UnknownDeviceImageIcon);
									if ( !showDeviceSets )
										hiddenItemHash.insert(machineItem, true);
									if ( useCategories )
										machineItem->setText(QMC2_MACHINELIST_COLUMN_CATEGORY, trSystemDevice);
									deviceSets.insert(machineName, true);
									machinePlayers = machineDrvStat = "N/A";
									break;
							}
							break;
					}
					machineItem->setText(QMC2_MACHINELIST_COLUMN_PLAYERS, machinePlayers);
					machineItem->setText(QMC2_MACHINELIST_COLUMN_DRVSTAT, machineStateTranslations.value(machineDrvStat));
					QTreeWidgetItem *nameItem = new QTreeWidgetItem(machineItem);
					nameItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, trWaitingForData);
					loadIcon(machineName, machineItem);
					itemList << machineItem;
					if ( numMachines++ % qmc2MachineListResponsiveness == 0 ) {
						qmc2MainWindow->progressBarMachineList->setValue(numMachines);
						qmc2MainWindow->labelMachineListStatus->setText(status());
					}
				}
				if ( endsWithNewLine )
					readBuffer.clear();
				else
					readBuffer = lines.last();
			}
			qmc2MainWindow->progressBarMachineList->setValue(numMachines);
			loadedFromCache = true;
		}
	} 
	if ( machineListCache.isOpen() )
		machineListCache.close();
	if ( reparseMachineList && !qmc2StopParser ) {
		loadIcon(QString(), 0); // initiates icon pre-caching
		qmc2MainWindow->progressBarMachineList->setRange(0, numTotalMachines * 2);
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("parsing machine data and recreating machine list cache"));
		if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
			qmc2MainWindow->progressBarMachineList->setFormat(tr("Machine data - %p%"));
		else
			qmc2MainWindow->progressBarMachineList->setFormat("%p%");
		machineListCache.open(QIODevice::WriteOnly | QIODevice::Text);
		if ( !machineListCache.isOpen() )
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: can't open machine list cache for writing, path = %1").arg(machineListCache.fileName()));
		else {
			tsMachineListCache.setDevice(&machineListCache);
			tsMachineListCache.setCodec(QTextCodec::codecForName("UTF-8"));
			tsMachineListCache.reset();
			tsMachineListCache << "# THIS FILE IS AUTO-GENERATED - PLEASE DO NOT EDIT!\n";
			tsMachineListCache << "MAME_VERSION\t" + emulatorVersion + "\tMLC_VERSION\t" + QString::number(QMC2_MLC_VERSION) + "\n";
		}
		bool useCatverIni = qmc2Config->value(QMC2_FRONTEND_PREFIX + "MachineList/UseCatverIni", false).toBool();
		bool useCategories = useCatverIni | qmc2Config->value(QMC2_FRONTEND_PREFIX + "MachineList/UseCategoryIni", false).toBool();;
		// parse XML data
		numMachines = numUnknownMachines = 0;
		qint64 xmlRowCount = xmlDb()->xmlRowCount();
		for (qint64 rowCounter = 1; rowCounter <= xmlRowCount && !qmc2StopParser; rowCounter++) {
			QStringList xmlLines = xmlDb()->xml(rowCounter).split(lineSplitChar, QString::SkipEmptyParts);
			for (int lineCounter = 0; lineCounter < xmlLines.count() && !qmc2StopParser; lineCounter++) {
				while ( lineCounter < xmlLines.count() && !xmlLines[lineCounter].contains("<description>") )
					lineCounter++;
				if ( !qmc2StopParser && lineCounter < xmlLines.count() ) {
					QString machineElement(xmlLines.at(lineCounter - 1).simplified());
					if ( !machineElement.contains(" name=\"") )
						continue;
					bool isBIOS = value(machineElement, "isbios").compare("yes") == 0;
					bool isDev = value(machineElement, "isdevice").compare("yes") == 0;
					QString machineName(value(machineElement, "name"));
					if ( machineName.isEmpty() ) {
						qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: name attribute empty on XML line %1 (set will be ignored!) -- please inform MAME developers and include the offending output from -listxml").arg(lineCounter + 2));
						qApp->processEvents();
						continue;
					}
					QString machineSource(value(machineElement, "sourcefile"));
					QString machineCloneOf(value(machineElement, "cloneof"));
					QString descriptionElement(xmlLines.at(lineCounter).simplified());
					QString machineDescription(descriptionElement.remove("<description>").remove("</description>").replace("&amp;", "&").replace("&lt;", "<").replace("&gt;", ">").replace("&quot;", "\"").replace("&apos;", "'"));
					MachineListItem *machineItem = new MachineListItem();
					qmc2MachineListItemHash.insert(machineName, machineItem);
					machineItem->setFlags(MachineListItem::defaultItemFlags);
					machineItem->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, Qt::Unchecked);
					if ( (isBIOS && !showBiosSets) || (isDev && !showDeviceSets) )
						hiddenItemHash.insert(machineItem, true);
					// find year & manufacturer and determine ROM/CHD requirements
					bool endGame = false;
					int i = lineCounter;
					QString machineYear(trQuestionMark), machineManufacturer(trQuestionMark), machinePlayers(trQuestionMark), machineDrvStat(trQuestionMark);
					bool yearFound = false, manufacturerFound = false, hasROMs = false, hasCHDs = false, playersFound = false, statusFound = false;
					QString endMark("</machine>");
					while ( !endGame ) {
						QString xmlLine(xmlLines[i]);
						if ( xmlLine.contains("<year>") ) {
							machineYear = xmlLine.simplified().remove("<year>").remove("</year>");
							yearFound = true;
						} else if ( xmlLine.contains("<manufacturer>") ) {
							machineManufacturer = xmlLine.simplified().remove("<manufacturer>").remove("</manufacturer>").replace("&amp;", "&").replace("&lt;", "<").replace("&gt;", ">").replace("&quot;", "\"").replace("&apos;", "'");
							manufacturerFound = true;
						} else if ( xmlLine.contains("<rom name") ) {
							hasROMs = true;
						} else if ( xmlLine.contains("<disk name") ) {
							hasCHDs = true;
						} else if ( xmlLine.contains("<input players") ) {
							int playersPos = xmlLine.indexOf("input players=\"") + 15;
							if ( playersPos >= 0 ) {
								machinePlayers = xmlLine.mid(playersPos, xmlLine.indexOf("\"", playersPos) - playersPos);
								playersFound = true;
							}
						} else if ( xmlLine.contains("<driver status") ) {
							int statusPos = xmlLine.indexOf("driver status=\"") + 15;
							if ( statusPos >= 0 ) {
								machineDrvStat = xmlLine.mid(statusPos, xmlLine.indexOf("\"", statusPos) - statusPos);
								statusFound = true;
							}
						}
						endGame = xmlLine.contains(endMark) || (yearFound && manufacturerFound && hasROMs && hasCHDs && playersFound && statusFound);
						i++;
					}
					if ( machineCloneOf.isEmpty() ) {
						if ( !hierarchyHash.contains(machineName) )
							hierarchyHash.insert(machineName, QStringList());
					} else
						hierarchyHash[machineCloneOf].append(machineName);
					machineItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, machineDescription);
					machineItem->setText(QMC2_MACHINELIST_COLUMN_YEAR, machineYear);
					machineItem->setText(QMC2_MACHINELIST_COLUMN_MANU, machineManufacturer);
					machineItem->setText(QMC2_MACHINELIST_COLUMN_NAME, machineName);
					machineItem->setText(QMC2_MACHINELIST_COLUMN_SRCFILE, machineSource);
					machineItem->setText(QMC2_MACHINELIST_COLUMN_RTYPES, romTypeNames.at(int(hasROMs) + int(hasCHDs) * 2));
					if ( isDev ) {
						if ( machinePlayers.compare(trQuestionMark) != 0 )
							machineItem->setText(QMC2_MACHINELIST_COLUMN_PLAYERS, machinePlayers);
						else
							machineItem->setText(QMC2_MACHINELIST_COLUMN_PLAYERS, tr("N/A"));
						machineItem->setText(QMC2_MACHINELIST_COLUMN_DRVSTAT, tr("N/A"));
					} else {
						machineItem->setText(QMC2_MACHINELIST_COLUMN_PLAYERS, machinePlayers);
						machineItem->setText(QMC2_MACHINELIST_COLUMN_DRVSTAT, machineStateTranslations.value(machineDrvStat));
					}
					if ( useCategories ) {
						if ( isBIOS )
							machineItem->setText(QMC2_MACHINELIST_COLUMN_CATEGORY, trSystemBios);
						else if ( isDev )
							machineItem->setText(QMC2_MACHINELIST_COLUMN_CATEGORY, trSystemDevice);
						else {
							QString *categoryString = categoryHash.value(machineName);
							machineItem->setText(QMC2_MACHINELIST_COLUMN_CATEGORY, categoryString ? *categoryString : trQuestionMark);
						}
					}
					if ( useCatverIni ) {
						QString *versionString = versionHash.value(machineName);
						machineItem->setText(QMC2_MACHINELIST_COLUMN_VERSION, versionString ? *versionString : trQuestionMark);
					}
					switch ( machineStatusHash.value(machineName) ) {
						case 'C': 
							numCorrectMachines++;
							if ( isBIOS ) {
								if ( showROMStatusIcons )
									machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectBIOSImageIcon);
								biosSets.insert(machineName, true);
							} else if ( isDev ) {
								if ( showROMStatusIcons )
									machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectDeviceImageIcon);
								deviceSets.insert(machineName, true);
							} else if ( showROMStatusIcons )
								machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectImageIcon);
							break;

						case 'M': 
							numMostlyCorrectMachines++;
							if ( isBIOS ) {
								if ( showROMStatusIcons )
									machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2MostlyCorrectBIOSImageIcon);
								biosSets.insert(machineName, true);
							} else if ( isDev ) {
								if ( showROMStatusIcons )
									machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2MostlyCorrectDeviceImageIcon);
								deviceSets.insert(machineName, true);
							} else if ( showROMStatusIcons )
								machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2MostlyCorrectImageIcon);
							break;

						case 'I':
							numIncorrectMachines++;
							if ( isBIOS ) {
								if ( showROMStatusIcons )
									machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2IncorrectBIOSImageIcon);
								biosSets.insert(machineName, true);
							} else if ( isDev ) {
								if ( showROMStatusIcons )
									machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2IncorrectDeviceImageIcon);
								deviceSets.insert(machineName, true);
							} else if ( showROMStatusIcons )
								machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2IncorrectImageIcon);
							break;

						case 'N':
							numNotFoundMachines++;
							if ( isBIOS ) {
								if ( showROMStatusIcons )
									machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2NotFoundBIOSImageIcon);
								biosSets.insert(machineName, true);
							} else if ( isDev ) {
								if ( showROMStatusIcons )
									machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2NotFoundDeviceImageIcon);
								deviceSets.insert(machineName, true);
							} else if ( showROMStatusIcons )
								machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2NotFoundImageIcon);
							break;

						default:
							numUnknownMachines++;
							machineStatusHash[machineName] = 'U';
							if ( isBIOS ) {
								if ( showROMStatusIcons )
									machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2UnknownBIOSImageIcon);
								biosSets.insert(machineName, true);
							} else if ( isDev ) {
								if ( showROMStatusIcons )
									machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2UnknownDeviceImageIcon);
								deviceSets.insert(machineName, true);
							} else if ( showROMStatusIcons )
								machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2UnknownImageIcon);
							break;
					}
					QTreeWidgetItem *nameItem = new QTreeWidgetItem(machineItem);
					nameItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, trWaitingForData);
					loadIcon(machineName, machineItem);
					if ( machineListCache.isOpen() )
						tsMachineListCache << machineName << columnSplitChar << machineDescription << columnSplitChar << machineManufacturer << columnSplitChar
							<< machineYear << columnSplitChar << machineCloneOf << columnSplitChar << (isBIOS ? '1': '0') << columnSplitChar
							<< (hasROMs ? '1' : '0') << columnSplitChar << (hasCHDs ? '1': '0') << columnSplitChar
							<< machinePlayers << columnSplitChar << machineDrvStat << columnSplitChar << (isDev ? '1': '0') << columnSplitChar
							<< machineSource << lineSplitChar;
					numMachines++;
					itemList << machineItem;
				}
				if ( numMachines % qmc2MachineListResponsiveness == 0 ) {
					qmc2MainWindow->progressBarMachineList->setValue(numMachines);
					qmc2MainWindow->labelMachineListStatus->setText(status());
					qApp->processEvents();
				}
			}
		}
		qmc2MainWindow->progressBarMachineList->setValue(numMachines);
	}
	if ( machineListCache.isOpen() )
		machineListCache.close();
	// create hierarchical view
	bool useCatverIni = qmc2Config->value(QMC2_FRONTEND_PREFIX + "MachineList/UseCatverIni", false).toBool();
	bool useCategories = useCatverIni | qmc2Config->value(QMC2_FRONTEND_PREFIX + "MachineList/UseCategoryIni", false).toBool();
	bool iconFallback = qmc2ParentImageFallback && qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/IconFallback", 0).toInt() == 0;
	qmc2MainWindow->labelMachineListStatus->setText(status());
	qmc2MainWindow->treeWidgetHierarchy->clear();
	QHashIterator<QString, QStringList> itHierarchyHash(hierarchyHash);
	QList<QTreeWidgetItem *> hierarchyItemList;
	QHash<QTreeWidgetItem *, bool> hierarchyHiddenItemHash;
	int counter = numMachines;
	qmc2HierarchyItemHash.reserve(numMachines);
	while ( itHierarchyHash.hasNext() && !qmc2StopParser ) {
		itHierarchyHash.next();
		const QString &parentName = itHierarchyHash.key();
		if ( counter++ % qmc2MachineListResponsiveness == 0 ) {
			qmc2MainWindow->progressBarMachineList->setValue(counter);
			qApp->processEvents();
		}
		QTreeWidgetItem *baseItem = qmc2MachineListItemHash.value(parentName);
		MachineListItem *hierarchyItem = new MachineListItem();
		qmc2HierarchyItemHash.insert(parentName, hierarchyItem);
		if ( hiddenItemHash.contains(baseItem) )
			hierarchyHiddenItemHash.insert(hierarchyItem, true);
		hierarchyItemList << hierarchyItem;
		hierarchyItem->setFlags(MachineListItem::defaultItemFlags);
		hierarchyItem->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, Qt::Unchecked);
		hierarchyItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, baseItem->text(QMC2_MACHINELIST_COLUMN_MACHINE));
		hierarchyItem->setText(QMC2_MACHINELIST_COLUMN_YEAR, baseItem->text(QMC2_MACHINELIST_COLUMN_YEAR));
		hierarchyItem->setText(QMC2_MACHINELIST_COLUMN_MANU, baseItem->text(QMC2_MACHINELIST_COLUMN_MANU));
		hierarchyItem->setText(QMC2_MACHINELIST_COLUMN_NAME, baseItem->text(QMC2_MACHINELIST_COLUMN_NAME));
		hierarchyItem->setText(QMC2_MACHINELIST_COLUMN_SRCFILE, baseItem->text(QMC2_MACHINELIST_COLUMN_SRCFILE));
		hierarchyItem->setText(QMC2_MACHINELIST_COLUMN_RTYPES, baseItem->text(QMC2_MACHINELIST_COLUMN_RTYPES));
		hierarchyItem->setText(QMC2_MACHINELIST_COLUMN_PLAYERS, baseItem->text(QMC2_MACHINELIST_COLUMN_PLAYERS));
		hierarchyItem->setText(QMC2_MACHINELIST_COLUMN_DRVSTAT, baseItem->text(QMC2_MACHINELIST_COLUMN_DRVSTAT));
		if ( useCategories )
			hierarchyItem->setText(QMC2_MACHINELIST_COLUMN_CATEGORY, baseItem->text(QMC2_MACHINELIST_COLUMN_CATEGORY));
		if ( useCatverIni )
			hierarchyItem->setText(QMC2_MACHINELIST_COLUMN_VERSION, baseItem->text(QMC2_MACHINELIST_COLUMN_VERSION));
		if ( showROMStatusIcons )
			hierarchyItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, baseItem->icon(QMC2_MACHINELIST_COLUMN_MACHINE));
		hierarchyItem->setIcon(QMC2_MACHINELIST_COLUMN_ICON, baseItem->icon(QMC2_MACHINELIST_COLUMN_ICON));
		// sub-items
		const QStringList &children = itHierarchyHash.value();
		for (int j = 0; j < children.count(); j++) {
			if ( counter++ % qmc2MachineListResponsiveness == 0 ) {
				qmc2MainWindow->progressBarMachineList->setValue(counter);
				qApp->processEvents();
			}
			const QString &cloneName = children.at(j);
			baseItem = qmc2MachineListItemHash.value(cloneName);
			MachineListItem *hierarchySubItem = new MachineListItem(hierarchyItem);
			qmc2HierarchyItemHash.insert(cloneName, hierarchySubItem);
			if ( hiddenItemHash.contains(baseItem) )
				hierarchyHiddenItemHash.insert(hierarchyItem, true);
			hierarchySubItem->setFlags(MachineListItem::defaultItemFlags);
			hierarchySubItem->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, Qt::Unchecked);
			hierarchySubItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, baseItem->text(QMC2_MACHINELIST_COLUMN_MACHINE));
			hierarchySubItem->setText(QMC2_MACHINELIST_COLUMN_YEAR, baseItem->text(QMC2_MACHINELIST_COLUMN_YEAR));
			hierarchySubItem->setText(QMC2_MACHINELIST_COLUMN_MANU, baseItem->text(QMC2_MACHINELIST_COLUMN_MANU));
			hierarchySubItem->setText(QMC2_MACHINELIST_COLUMN_NAME, cloneName);
			hierarchySubItem->setText(QMC2_MACHINELIST_COLUMN_SRCFILE, baseItem->text(QMC2_MACHINELIST_COLUMN_SRCFILE));
			hierarchySubItem->setText(QMC2_MACHINELIST_COLUMN_RTYPES, baseItem->text(QMC2_MACHINELIST_COLUMN_RTYPES));
			hierarchySubItem->setText(QMC2_MACHINELIST_COLUMN_PLAYERS, baseItem->text(QMC2_MACHINELIST_COLUMN_PLAYERS));
			hierarchySubItem->setText(QMC2_MACHINELIST_COLUMN_DRVSTAT, baseItem->text(QMC2_MACHINELIST_COLUMN_DRVSTAT));
			if ( useCategories ) {
				QString category(baseItem->text(QMC2_MACHINELIST_COLUMN_CATEGORY));
				if ( category.compare(trQuestionMark) == 0 ) {
					category = hierarchyItem->text(QMC2_MACHINELIST_COLUMN_CATEGORY);
					baseItem->setText(QMC2_MACHINELIST_COLUMN_CATEGORY, category);
				}
				hierarchySubItem->setText(QMC2_MACHINELIST_COLUMN_CATEGORY, category);
			}
			if ( useCatverIni )
				hierarchySubItem->setText(QMC2_MACHINELIST_COLUMN_VERSION, baseItem->text(QMC2_MACHINELIST_COLUMN_VERSION));
			if ( showROMStatusIcons )
				hierarchySubItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, baseItem->icon(QMC2_MACHINELIST_COLUMN_MACHINE));
			QIcon icon(baseItem->icon(QMC2_MACHINELIST_COLUMN_ICON));
			if ( icon.isNull() ) {
				if ( iconFallback ) {
					QIcon icon(hierarchyItem->icon(QMC2_MACHINELIST_COLUMN_ICON));
					if ( !icon.isNull() ) {
						baseItem->setIcon(QMC2_MACHINELIST_COLUMN_ICON, icon);
						hierarchySubItem->setIcon(QMC2_MACHINELIST_COLUMN_ICON, icon);
					}
				}
			} else
				hierarchySubItem->setIcon(QMC2_MACHINELIST_COLUMN_ICON, icon);
			qmc2ParentHash.insert(cloneName, parentName);
		}
	}
	qmc2MainWindow->treeWidgetMachineList->setUpdatesEnabled(false);
	qmc2MainWindow->treeWidgetMachineList->addTopLevelItems(itemList);
	QHashIterator<QTreeWidgetItem *, bool> itHiddenItemHash(hiddenItemHash);
	while ( itHiddenItemHash.hasNext() ) {
		itHiddenItemHash.next();
		itHiddenItemHash.key()->setHidden(true);
	}
	qmc2MainWindow->treeWidgetHierarchy->setUpdatesEnabled(false);
	qmc2MainWindow->treeWidgetHierarchy->addTopLevelItems(hierarchyItemList);
	QHashIterator<QTreeWidgetItem *, bool> itHierarchyHiddenItemHash(hierarchyHiddenItemHash);
	while ( itHierarchyHiddenItemHash.hasNext() ) {
		itHierarchyHiddenItemHash.next();
		itHierarchyHiddenItemHash.key()->setHidden(true);
	}
	if ( loadedFromCache ) {
		machineListCacheElapsedTime = machineListCacheElapsedTime.addMSecs(miscTimer.elapsed());
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading machine data from machine list cache, elapsed time = %1)").arg(machineListCacheElapsedTime.toString("mm:ss.zzz")));
	}
	QString sortCriteria(trQuestionMark);
	switch ( qmc2SortCriteria ) {
		case QMC2_SORT_BY_DESCRIPTION:
			sortCriteria = QObject::tr("machine description");
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
			sortCriteria = QObject::tr("machine name");
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
		case QMC2_SORT_BY_VERSION:
			sortCriteria = QObject::tr("version");
			break;
	}
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("sorting machine list by %1 in %2 order").arg(sortCriteria).arg(qmc2SortOrder == Qt::AscendingOrder ? tr("ascending") : tr("descending")));
	qmc2MainWindow->progressBarMachineList->setValue(qmc2MainWindow->progressBarMachineList->maximum());
	if ( !qmc2MachineList->userDataDb()->rankCacheComplete() )
		qmc2MachineList->userDataDb()->fillUpRankCache();
	qmc2MainWindow->treeWidgetMachineList->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
	qmc2MainWindow->treeWidgetHierarchy->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
	QTreeWidgetItem *ci = qmc2MainWindow->treeWidgetMachineList->currentItem();
	if ( ci ) {
		if ( ci->isSelected() ) {
			QTimer::singleShot(0, qmc2MainWindow, SLOT(scrollToCurrentItem()));
		} else if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/RestoreGameSelection").toBool() ) {
			QString selectedMachine = qmc2Config->value(QMC2_EMULATOR_PREFIX + "SelectedGame", QString()).toString();
			if ( !selectedMachine.isEmpty() ) {
				QTreeWidgetItem *mlItem = qmc2MachineListItemHash.value(selectedMachine);
				if ( mlItem ) {
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("restoring machine selection"));
					qmc2MainWindow->treeWidgetMachineList->setCurrentItem(mlItem);
					QTimer::singleShot(0, qmc2MainWindow, SLOT(scrollToCurrentItem()));
				} else
					QTimer::singleShot(0, qmc2MainWindow, SLOT(updateUserData()));
			} else
				QTimer::singleShot(0, qmc2MainWindow, SLOT(updateUserData()));
		}
	} else if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/RestoreGameSelection").toBool() ) {
		QString selectedMachine = qmc2Config->value(QMC2_EMULATOR_PREFIX + "SelectedGame", QString()).toString();
		if ( !selectedMachine.isEmpty() ) {
			QTreeWidgetItem *mlItem = qmc2MachineListItemHash.value(selectedMachine);
			if ( mlItem ) {
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("restoring machine selection"));
				qmc2MainWindow->treeWidgetMachineList->setCurrentItem(mlItem);
				QTimer::singleShot(0, qmc2MainWindow, SLOT(scrollToCurrentItem()));
			} else
				QTimer::singleShot(0, qmc2MainWindow, SLOT(updateUserData()));
		} else
			QTimer::singleShot(0, qmc2MainWindow, SLOT(updateUserData()));
	} else
		QTimer::singleShot(0, qmc2MainWindow, SLOT(updateUserData()));
	qmc2MainWindow->treeWidgetMachineList->setUpdatesEnabled(true);
	qmc2MainWindow->treeWidgetHierarchy->setUpdatesEnabled(true);
	processMachineListElapsedTimer = processMachineListElapsedTimer.addMSecs(parseTimer.elapsed());
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (processing machine list, elapsed time = %1)").arg(processMachineListElapsedTimer.toString("mm:ss.zzz")));
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("%n machine(s)", "", numTotalMachines - biosSets.count()) + tr(", %n BIOS set(s)", "", biosSets.count()) + tr(" and %n device(s) loaded", "", deviceSets.count()));
	if ( numMachines - deviceSets.count() != numTotalMachines ) {
		if ( reparseMachineList && qmc2StopParser ) {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: machine list not fully parsed, invalidating machine list cache"));
			QFile f(qmc2Config->value("MAME/FilesAndDirectories/MachineListCacheFile").toString());
			f.remove();
		} else if ( !qmc2StopParser) {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: machine list cache is out of date, invalidating machine list cache"));
			QFile f(qmc2Config->value("MAME/FilesAndDirectories/MachineListCacheFile").toString());
			f.remove();
		}
	}
	QString sL(numTotalMachines + deviceSets.count() >= 0 ? QString::number(numTotalMachines + deviceSets.count()) : trQuestionMark);
	QString sC(numCorrectMachines >= 0 ? QString::number(numCorrectMachines) : trQuestionMark);
	QString sM(numMostlyCorrectMachines >= 0 ? QString::number(numMostlyCorrectMachines) : trQuestionMark);
	QString sI(numIncorrectMachines >= 0 ? QString::number(numIncorrectMachines) : trQuestionMark);
	QString sN(numNotFoundMachines >= 0 ? QString::number(numNotFoundMachines) : trQuestionMark);
	QString sU(numUnknownMachines >= 0 ? QString::number(numUnknownMachines) : trQuestionMark);
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ROM state info: L:%1 C:%2 M:%3 I:%4 N:%5 U:%6").arg(sL).arg(sC).arg(sM).arg(sI).arg(sN).arg(sU));
	qmc2MainWindow->progressBarMachineList->reset();
	qmc2ReloadActive = qmc2StartingUp = false;
	if ( qmc2StopParser ) {
		if ( loadProc )
			loadProc->kill();
		autoRomCheck = false;
	} else {
		if ( romStateCacheUpdate || machineStatusHash.count() - deviceSets.count() != numTotalMachines ) {
			if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "MachineList/AutoTriggerROMCheck").toBool() ) {
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: ROM state cache is incomplete or not up to date, triggering an automatic ROM check"));
				autoRomCheck = true;
			} else
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: ROM state cache is incomplete or not up to date, please re-check ROMs"));
		}
	}
	verifyCurrentOnly = false;
	if ( autoRomCheck )
		QTimer::singleShot(QMC2_AUTOROMCHECK_DELAY, qmc2MainWindow->actionCheckROMs, SLOT(trigger()));
	else if ( !qmc2StopParser && qmc2Config->value(QMC2_FRONTEND_PREFIX + "RomStateFilter/Enabled", true).toBool() )
		filter(true);
	enableWidgets(true);
}

void MachineList::filter(bool initial)
{
	if ( !initial ) {
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
	}
	bool showC = qmc2Config->value(QMC2_FRONTEND_PREFIX + "RomStateFilter/ShowCorrect", true).toBool();
	bool showM = qmc2Config->value(QMC2_FRONTEND_PREFIX + "RomStateFilter/ShowMostlyCorrect", true).toBool();
	bool showI = qmc2Config->value(QMC2_FRONTEND_PREFIX + "RomStateFilter/ShowIncorrect", true).toBool();
	bool showN = qmc2Config->value(QMC2_FRONTEND_PREFIX + "RomStateFilter/ShowNotFound", true).toBool();
	bool showU = qmc2Config->value(QMC2_FRONTEND_PREFIX + "RomStateFilter/ShowUnknown", true).toBool();
	if ( initial && showC && showM && showI && showN && showU ) {
		qmc2StatesTogglesEnabled = true;
		return;
	}
	bool showDeviceSets = qmc2Config->value(QMC2_FRONTEND_PREFIX + "MachineList/ShowDeviceSets", true).toBool();
	bool showBiosSets = qmc2Config->value(QMC2_FRONTEND_PREFIX + "MachineList/ShowBiosSets", true).toBool();
	QTime elapsedTime(0, 0, 0, 0);
	qmc2StopParser = false;
	parseTimer.start();
	qmc2FilterActive = true;
	enableWidgets(false);
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("applying ROM state filter"));
	qmc2MainWindow->progressBarMachineList->reset();
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
		qmc2MainWindow->progressBarMachineList->setFormat(tr("State filter - %p%"));
	else
		qmc2MainWindow->progressBarMachineList->setFormat("%p%");
	int itemCount = qmc2MainWindow->treeWidgetMachineList->topLevelItemCount();
	qmc2MainWindow->progressBarMachineList->setRange(0, itemCount - 1);
	if ( verifyCurrentOnly && checkedItem ) {
		QString machineName = checkedItem->text(QMC2_MACHINELIST_COLUMN_NAME);
		if ( !showBiosSets && isBios(machineName) )
			checkedItem->setHidden(true);
		else if ( !showDeviceSets && isDevice(machineName) )
			checkedItem->setHidden(true);
		else switch ( machineStatusHash.value(machineName) ) {
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
		QTreeWidgetItem *curItem = qmc2MainWindow->treeWidgetMachineList->currentItem();
		QWidget *currentFocusWidget = qApp->focusWidget();
		qmc2MainWindow->treeWidgetMachineList->setVisible(false);
		// note: reset()'ing the tree-widget is essential to avoid an apparent Qt bug that slows down filtering under certain circumstances
		qmc2MainWindow->treeWidgetMachineList->reset();
		((AspectRatioLabel *)qmc2MainWindow->labelLoadingMachineList)->setLabelText(tr("Filtering, please wait..."));
		qmc2MainWindow->labelLoadingMachineList->setVisible(true);
		if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ShowLoadingAnimation", true).toBool() )
			qmc2MainWindow->loadAnimMovie->start();
		int filterResponse = itemCount / QMC2_STATEFILTER_UPDATES;
		for (int i = 0; i < itemCount && !qmc2StopParser; i++) {
			QTreeWidgetItem *item = qmc2MainWindow->treeWidgetMachineList->topLevelItem(i);
			QString machineName(item->text(QMC2_MACHINELIST_COLUMN_NAME));
			if ( !showBiosSets && isBios(machineName) )
				item->setHidden(true);
			else if ( !showDeviceSets && isDevice(machineName) )
				item->setHidden(true);
			else switch ( machineStatusHash.value(machineName) ) {
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
			if ( i % filterResponse == 0 ) {
				qmc2MainWindow->progressBarMachineList->setValue(i);
				qApp->processEvents();
			}
		}
		qmc2MainWindow->loadAnimMovie->setPaused(true);
		qmc2MainWindow->treeWidgetMachineList->setVisible(true);
		qmc2MainWindow->labelLoadingMachineList->setVisible(false);
		if ( curItem )
			qmc2MainWindow->treeWidgetMachineList->setCurrentItem(curItem);
		if ( currentFocusWidget )
			currentFocusWidget->setFocus();
	}
	qmc2MainWindow->progressBarMachineList->setValue(itemCount - 1);
	qmc2FilterActive = false;
	elapsedTime = elapsedTime.addMSecs(parseTimer.elapsed());
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (applying ROM state filter, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
	qmc2MainWindow->progressBarMachineList->reset();
	enableWidgets(true);
	QTimer::singleShot(0, qmc2MainWindow, SLOT(scrollToCurrentItem()));
	qmc2StatesTogglesEnabled = true;
}

void MachineList::loadFavorites()
{
	qmc2MainWindow->listWidgetFavorites->clear();
	QFile f(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/FavoritesFile").toString());
	if ( f.open(QIODevice::ReadOnly | QIODevice::Text) ) {
		QTextStream ts(&f);
		while ( !ts.atEnd() ) {
			QString machineName = ts.readLine();
			if ( !machineName.isEmpty() ) {
				QTreeWidgetItem *machineItem = qmc2MachineListItemHash.value(machineName);
				if ( machineItem ) {
					QListWidgetItem *item = new QListWidgetItem(qmc2MainWindow->listWidgetFavorites);
					item->setText(machineItem->text(QMC2_MACHINELIST_COLUMN_MACHINE));
					item->setWhatsThis(machineItem->text(QMC2_MACHINELIST_COLUMN_NAME));
					if ( machineItem->isSelected() )
						item->setSelected(true);
				}
			}
		}
		f.close();
	}
	qmc2MainWindow->listWidgetFavorites->sortItems();
	if ( qmc2MainWindow->tabWidgetMachineList->indexOf(qmc2MainWindow->tabFavorites) == qmc2MainWindow->tabWidgetMachineList->currentIndex() )
		QTimer::singleShot(50, qmc2MainWindow, SLOT(checkCurrentFavoritesSelection()));
	else
		qmc2MainWindow->listWidgetFavorites->setCurrentIndex(QModelIndex());
}

void MachineList::saveFavorites()
{
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("saving favorites"));
	QFile f(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/FavoritesFile").toString());
	if ( f.open(QIODevice::WriteOnly | QIODevice::Text) ) {
		QTextStream ts(&f);
		for (int i = 0; i < qmc2MainWindow->listWidgetFavorites->count(); i++)
			ts << qmc2MainWindow->listWidgetFavorites->item(i)->whatsThis() << "\n";
		f.close();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open favorites file for writing, path = %1").arg(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/FavoritesFile").toString()));
}

void MachineList::loadPlayHistory()
{
	qmc2MainWindow->listWidgetPlayed->clear();
	QFile f(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/HistoryFile").toString());
	if ( f.open(QIODevice::ReadOnly | QIODevice::Text) ) {
		QTextStream ts(&f);
		while ( !ts.atEnd() ) {
			QString machineName = ts.readLine();
			if ( !machineName.isEmpty() ) {
				QTreeWidgetItem *machineItem = qmc2MachineListItemHash.value(machineName);
				if ( machineItem ) {
					QListWidgetItem *item = new QListWidgetItem(qmc2MainWindow->listWidgetPlayed);
					item->setText(machineItem->text(QMC2_MACHINELIST_COLUMN_MACHINE));
					item->setWhatsThis(machineItem->text(QMC2_MACHINELIST_COLUMN_NAME));
					if ( machineItem->isSelected() )
						item->setSelected(true);
				}
			}
		}
		f.close();
	}
	if ( qmc2MainWindow->tabWidgetMachineList->indexOf(qmc2MainWindow->tabPlayed) == qmc2MainWindow->tabWidgetMachineList->currentIndex() )
		QTimer::singleShot(50, qmc2MainWindow, SLOT(checkCurrentPlayedSelection()));
	else
		qmc2MainWindow->listWidgetPlayed->setCurrentIndex(QModelIndex());
}

void MachineList::savePlayHistory()
{
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("saving play history"));
	QFile f(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/HistoryFile").toString());
	if ( f.open(QIODevice::WriteOnly | QIODevice::Text) ) {
		QTextStream ts(&f);
		for (int i = 0; i < qmc2MainWindow->listWidgetPlayed->count(); i++)
			ts << qmc2MainWindow->listWidgetPlayed->item(i)->whatsThis() << "\n";
		f.close();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open play history file for writing, path = %1").arg(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/HistoryFile").toString()));
}

QString &MachineList::status()
{
	static QLocale locale;
	statusString = "<b><font color=\"black\">" + tr("L:") + QString(numMachines > -1 ? locale.toString(numMachines) : trQuestionMark) + "</font>&nbsp;"
		"<font color=\"#00cc00\">" + tr("C:") + QString(numCorrectMachines > -1 ? locale.toString(numCorrectMachines) : trQuestionMark) + "</font>&nbsp;"
		"<font color=\"#799632\">" + tr("M:") + QString(numMostlyCorrectMachines > -1 ? locale.toString(numMostlyCorrectMachines) : trQuestionMark) + "</font>&nbsp;"
		"<font color=\"#f90000\">" + tr("I:") + QString(numIncorrectMachines > -1 ? locale.toString(numIncorrectMachines) : trQuestionMark) + "</font>&nbsp;"
		"<font color=\"#7f7f7f\">" + tr("N:") + QString(numNotFoundMachines > -1 ? locale.toString(numNotFoundMachines) : trQuestionMark) + "</font>&nbsp;"
		"<font color=\"#0000f9\">" + tr("U:") + QString(numUnknownMachines > -1 ? locale.toString(numUnknownMachines) : trQuestionMark) + "</font>&nbsp;"
		"<font color=\"chocolate\">" + tr("S:") + QString(numMatchedMachines > -1 ? locale.toString(numMatchedMachines) : trQuestionMark) + "</font>&nbsp;"
		"<font color=\"sandybrown\">" + tr("T:") + QString(numTaggedSets > -1 ? locale.toString(numTaggedSets) : trQuestionMark) + "</font></b>";
	return statusString;
}

void MachineList::loadStarted()
{
	qmc2MainWindow->progressBarMachineList->setRange(0, numTotalMachines);
	qmc2MainWindow->progressBarMachineList->reset();
}

void MachineList::loadFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	bool invalidateListXmlCache = false;
	if ( exitStatus != QProcess::NormalExit && !qmc2StopParser ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: emulator audit call didn't exit cleanly -- exitCode = %1, exitStatus = %2").arg(exitCode).arg(QString(exitStatus == QProcess::NormalExit ? tr("normal") : tr("crashed"))));
		qmc2StopParser = invalidateListXmlCache = true;
	} else if ( qmc2StopParser && exitStatus == QProcess::CrashExit )
		qmc2StopParser = invalidateListXmlCache = true;
	QTime elapsedTime(0, 0, 0, 0);
	elapsedTime = elapsedTime.addMSecs(loadTimer.elapsed());
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading XML data and recreating cache, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
	qmc2MainWindow->progressBarMachineList->reset();
	qmc2EarlyReloadActive = false;
	if ( loadProc )
		delete loadProc;
	loadProc = 0;
	if ( romStateCache.isOpen() )
		romStateCache.close();
	xmlDb()->commitTransaction();
	uncommittedXmlDbRows = 0;
	if ( invalidateListXmlCache ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: XML data cache is incomplete, invalidating XML data cache"));
		xmlDb()->recreateDatabase();
	}
	parse();
  	QTimer::singleShot(0, qmc2MainWindow, SLOT(updateUserData()));
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading favorites and play history"));
	loadFavorites();
	loadPlayHistory();
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading favorites and play history)"));
	if ( initialLoad ) {
		QTime startupTime(0, 0, 0, 0);
		startupTime = startupTime.addMSecs(qmc2StartupTimer.elapsed());
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("total start-up time: %1").arg(startupTime.toString("mm:ss.zzz")));
		initialLoad = false;
	}
	// show machine list / hide loading animation
	qmc2MainWindow->loadAnimMovie->setPaused(true);
	qmc2MainWindow->labelLoadingMachineList->setVisible(false);
	qmc2MainWindow->treeWidgetMachineList->setVisible(true);
	qmc2MainWindow->labelLoadingHierarchy->setVisible(false);
	qmc2MainWindow->treeWidgetHierarchy->setVisible(true);
	if ( qmc2MainWindow->tabWidgetMachineList->indexOf(qmc2MainWindow->tabMachineList) == qmc2MainWindow->tabWidgetMachineList->currentIndex() ) {
		if ( qApp->focusWidget() != qmc2MainWindow->comboBoxToolbarSearch ) {
			switch ( qmc2MainWindow->stackedWidgetView->currentIndex() ) {
				case QMC2_VIEWHIERARCHY_INDEX:
					qmc2MainWindow->treeWidgetHierarchy->setFocus();
					break;
				case QMC2_VIEWCATEGORY_INDEX:
					qmc2MainWindow->treeWidgetCategoryView->setFocus();
					break;
				case QMC2_VIEWVERSION_INDEX:
					qmc2MainWindow->treeWidgetVersionView->setFocus();
					break;
				case QMC2_VIEWMACHINELIST_INDEX:
				default:
					qmc2MainWindow->treeWidgetMachineList->setFocus();
					break;
			}
		}
	}
}

void MachineList::loadReadyReadStandardOutput()
{
	static bool lastCharacterWasSpace = false;
	static QString dtdBuffer;
	static QString setXmlBuffer;
	static QString currentSetName;
	static QRegExp rxDescYearManu("\\<description\\>$|\\<year\\>$|\\<manufacturer\\>$");

	// this makes the GUI much more responsive, but is HAS to be called before loadProc->readAllStandardOutput()!
	if ( QCoreApplication::hasPendingEvents() )
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
			if ( singleXMLLine.indexOf(rxDescYearManu) >= 0 )
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
				dtdBufferReady = xmlLineBuffer.startsWith("<mame build=");
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
					int startIndex = xmlLineBuffer.indexOf("<machine name=\"");
					if ( startIndex >= 0 ) {
						startIndex += 15;
						int endIndex = xmlLineBuffer.indexOf("\"", startIndex);
						if ( endIndex >= 0 ) {
							currentSetName = xmlLineBuffer.mid(startIndex, endIndex - startIndex);
							setXmlBuffer += xmlLineBuffer;
						}
					}
				} else {
					setXmlBuffer += xmlLineBuffer;
					int index = xmlLineBuffer.indexOf("</machine>");
					if ( index >= 0 ) {
						if ( setXmlBuffer.endsWith("\n") )
							setXmlBuffer.remove(setXmlBuffer.length() - 1, 1);
						if ( xmlDb()->exists(currentSetName) )
							qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: XML bug: the name '%1' is used for multiple sets -- please inform MAME developers").arg(currentSetName));
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
	qmc2MainWindow->progressBarMachineList->setValue(qmc2MainWindow->progressBarMachineList->value() + readBuffer.count("<machine name="));
}

void MachineList::verifyStarted()
{
	if ( !verifyCurrentOnly )
		qmc2MainWindow->progressBarMachineList->setValue(0);
}

void MachineList::verifyFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	if ( !verifyProc->atEnd() )
		verifyReadyReadStandardOutput();
	bool cleanExit = true;
	if ( exitStatus != QProcess::NormalExit && !qmc2StopParser ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: emulator audit call didn't exit cleanly -- exitCode = %1, exitStatus = %2").arg(exitCode).arg(QString(exitStatus == QProcess::NormalExit ? tr("normal") : tr("crashed"))));
		cleanExit = false;
	}
	bool showROMStatusIcons = qmc2Config->value(QMC2_FRONTEND_PREFIX + "MachineList/ShowROMStatusIcons", true).toBool();
	if ( !verifyCurrentOnly ) {
		// the progress text may have changed in the meantime...
		if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
			qmc2MainWindow->progressBarMachineList->setFormat(tr("ROM check - %p%"));
		QSet<QString> gameSet = QSet<QString>::fromList(qmc2MachineListItemHash.uniqueKeys());
		QList<QString> remainingGames = gameSet.subtract(QSet<QString>::fromList(verifiedList)).values();
		int counter = qmc2MainWindow->progressBarMachineList->value();
		if ( qmc2StopParser || !cleanExit ) {
			for (int i = 0; i < remainingGames.count(); i++) {
				counter++;
				if ( i % QMC2_REMAINING_SETS_CHECK_RSP == 0 || i == remainingGames.count() - 1 ) {
					qmc2MainWindow->progressBarMachineList->setValue(counter);
					qmc2MainWindow->labelMachineListStatus->setText(status());
					qApp->processEvents();
				}
				QString machineName = remainingGames[i];
				QTreeWidgetItem *romItem = qmc2MachineListItemHash.value(machineName);
				QTreeWidgetItem *hierarchyItem = qmc2HierarchyItemHash.value(machineName);
				if ( romItem && hierarchyItem ) {
					QTreeWidgetItem *categoryItem = qmc2CategoryItemHash.value(machineName);
					QTreeWidgetItem *versionItem = qmc2VersionItemHash.value(machineName);
					machineStatusHash[machineName] = 'U';
					bool isBIOS = isBios(machineName);
					bool isDev = isDevice(machineName);
					if ( romStateCache.isOpen() )
						tsRomCache << machineName << " U\n";
					numUnknownMachines++;
					if ( isBIOS ) {
						if ( showROMStatusIcons ) {
							romItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2UnknownBIOSImageIcon);
							hierarchyItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2UnknownBIOSImageIcon);
							if ( categoryItem )
								categoryItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2UnknownBIOSImageIcon);
							if ( versionItem )
								versionItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2UnknownBIOSImageIcon);
						}
					} else if ( isDev ) {
						if ( showROMStatusIcons ) {
							romItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2UnknownDeviceImageIcon);
							hierarchyItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2UnknownDeviceImageIcon);
							if ( categoryItem )
								categoryItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2UnknownDeviceImageIcon);
							if ( versionItem )
								versionItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2UnknownDeviceImageIcon);
						}
					} else {
						if ( showROMStatusIcons ) {
							romItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2UnknownImageIcon);
							hierarchyItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2UnknownImageIcon);
							if ( categoryItem )
								categoryItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2UnknownImageIcon);
							if ( versionItem )
								versionItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2UnknownImageIcon);
						}
					}
				}
				if ( romItem == qmc2CurrentItem )
					qmc2MainWindow->labelMachineStatus->setPalette(MainWindow::qmc2StatusColorBlue);
			}
		} else {
			if ( !remainingGames.isEmpty() && !qmc2StopParser )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("checking real status of %n set(s) not mentioned during full audit", "", remainingGames.count()));
			for (int i = 0; i < remainingGames.count() && !qmc2StopParser; i++) {
				counter++;
				if ( i % QMC2_REMAINING_SETS_CHECK_RSP == 0 || i == remainingGames.count() - 1 ) {
					qmc2MainWindow->progressBarMachineList->setValue(counter);
					qmc2MainWindow->labelMachineListStatus->setText(status());
					qApp->processEvents();
				}
				QString machineName = remainingGames[i];
				bool isBIOS = isBios(machineName);
				bool isDev = isDevice(machineName);
				QTreeWidgetItem *romItem = qmc2MachineListItemHash.value(machineName);
				QTreeWidgetItem *hierarchyItem = qmc2HierarchyItemHash.value(machineName);
				QTreeWidgetItem *categoryItem = qmc2CategoryItemHash.value(machineName);
				QTreeWidgetItem *versionItem = qmc2VersionItemHash.value(machineName);
				// there are quite a number of sets in MESS and MAME that don't require any ROMs... many/most device-sets in particular
				bool romRequired = true;
				int xmlCounter = 0;
				QStringList xmlLines = xmlDb()->xml(machineName).split("\n", QString::SkipEmptyParts);
				if ( xmlLines.count() > 0 ) {
					int romCounter = 0;
					int chdCounter = 0;
					bool endFound = false;
					QString endMark = "</machine>";
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
					if ( romStateCache.isOpen() ) {
						if ( romRequired ) {
							tsRomCache << machineName << " N\n";
							numNotFoundMachines++;
						} else {
							tsRomCache << machineName << " C\n";
							numCorrectMachines++;
						}
					}
					if ( isBIOS ) {
						if ( showROMStatusIcons ) {
							if ( romRequired ) {
								romItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2NotFoundBIOSImageIcon);
								hierarchyItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2NotFoundBIOSImageIcon);
								if ( categoryItem )
									categoryItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2NotFoundBIOSImageIcon);
								if ( versionItem )
									versionItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2NotFoundBIOSImageIcon);
							} else {
								romItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectBIOSImageIcon);
								hierarchyItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectBIOSImageIcon);
								if ( categoryItem )
									categoryItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectBIOSImageIcon);
								if ( versionItem )
									versionItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectBIOSImageIcon);
							}
						}
					} else if ( isDev ) {
						if ( romRequired ) {
							romItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2NotFoundDeviceImageIcon);
							hierarchyItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2NotFoundDeviceImageIcon);
							if ( categoryItem )
								categoryItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2NotFoundDeviceImageIcon);
							if ( versionItem )
								versionItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2NotFoundDeviceImageIcon);
						} else {
							romItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectDeviceImageIcon);
							hierarchyItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectDeviceImageIcon);
							if ( categoryItem )
								categoryItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectDeviceImageIcon);
							if ( versionItem )
								versionItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectDeviceImageIcon);
						}
					} else {
						if ( showROMStatusIcons ) {
							if ( romRequired ) {
								romItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2NotFoundImageIcon);
								hierarchyItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2NotFoundImageIcon);
								if ( categoryItem )
									categoryItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2NotFoundImageIcon);
								if ( versionItem )
									versionItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2NotFoundImageIcon);
							} else {
								romItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectImageIcon);
								hierarchyItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectImageIcon);
								if ( categoryItem )
									categoryItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectImageIcon);
								if ( versionItem )
									versionItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectImageIcon);
							}
						}
					}
					if ( romRequired ) {
						machineStatusHash[machineName] = 'N';
						if ( romItem == qmc2CurrentItem )
							qmc2MainWindow->labelMachineStatus->setPalette(MainWindow::qmc2StatusColorGrey);
					} else {
						machineStatusHash[machineName] = 'C';
						if ( romItem == qmc2CurrentItem )
							qmc2MainWindow->labelMachineStatus->setPalette(MainWindow::qmc2StatusColorGreen);
					}
				}
			}
			if ( !remainingGames.isEmpty() && !qmc2StopParser )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (checking real status of %n set(s) not mentioned during full audit)", "", remainingGames.count()));
		}
		qmc2MainWindow->labelMachineListStatus->setText(status());
	}
	bool doFilter = true;
	if ( verifyCurrentOnly ) {
		QString machineName;
		if ( verifiedList.isEmpty() && checkedItem && exitCode == QMC2_MAME_ERROR_NO_SUCH_MACHINE ) {
			// many device-sets that have no ROMs are declared as being "invalid" during the audit, but that isn't true :)
			machineName = checkedItem->text(QMC2_MACHINELIST_COLUMN_NAME);
			QTreeWidgetItem *hierarchyItem = qmc2HierarchyItemHash.value(machineName);
			if ( hierarchyItem ) {
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ROM status for '%1' is '%2'").arg(checkedItem->text(QMC2_MACHINELIST_COLUMN_MACHINE)).arg(QObject::tr("correct")));
				machineStatusHash[machineName] = 'C';
				numUnknownMachines--;
				numCorrectMachines++;
				if ( checkedItem == qmc2CurrentItem )
					qmc2MainWindow->labelMachineStatus->setPalette(MainWindow::qmc2StatusColorGreen);
				QTreeWidgetItem *categoryItem = qmc2CategoryItemHash.value(machineName);
				QTreeWidgetItem *versionItem = qmc2VersionItemHash.value(machineName);
				if ( isBios(machineName) ) {
					if ( showROMStatusIcons ) {
						checkedItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectBIOSImageIcon);
						hierarchyItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectBIOSImageIcon);
						if ( categoryItem )
							categoryItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectBIOSImageIcon);
						if ( versionItem )
							versionItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectBIOSImageIcon);
					}
				} else if ( isDevice(machineName) ) {
					if ( showROMStatusIcons ) {
						checkedItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectDeviceImageIcon);
						hierarchyItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectDeviceImageIcon);
						if ( categoryItem )
							categoryItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectDeviceImageIcon);
						if ( versionItem )
							versionItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectDeviceImageIcon);
					}
				} else {
					if ( showROMStatusIcons ) {
						checkedItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectImageIcon);
						hierarchyItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectImageIcon);
						if ( categoryItem )
							categoryItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectImageIcon);
						if ( versionItem )
							versionItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectImageIcon);
					}
				}
			}
			qmc2MainWindow->labelMachineListStatus->setText(status());
		} else if ( checkedItem )
			machineName = checkedItem->text(QMC2_MACHINELIST_COLUMN_NAME);
		if ( romStateCache.isOpen() ) {
			QHashIterator<QString, char> it(machineStatusHash);
			while ( it.hasNext() ) {
				it.next();
				QString machineName = it.key();
				if ( !machineName.isEmpty() ) {
					tsRomCache << machineName << " ";
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
		}
		doFilter = (oldRomState != machineStatusHash.value(machineName));
	}
	QTime elapsedTime(0, 0, 0, 0);
	elapsedTime = elapsedTime.addMSecs(verifyTimer.elapsed());
	if ( verifyCurrentOnly )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (verifying ROM status for '%1', elapsed time = %2)").arg(checkedItem->text(QMC2_MACHINELIST_COLUMN_MACHINE)).arg(elapsedTime.toString("mm:ss.zzz")));
	else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (verifying ROM status for all sets, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
	QString sL(numTotalMachines + deviceSets.count() >= 0 ? QString::number(numTotalMachines + deviceSets.count()) : trQuestionMark);
	QString sC(numCorrectMachines >= 0 ? QString::number(numCorrectMachines) : trQuestionMark);
	QString sM(numMostlyCorrectMachines >= 0 ? QString::number(numMostlyCorrectMachines) : trQuestionMark);
	QString sI(numIncorrectMachines >= 0 ? QString::number(numIncorrectMachines) : trQuestionMark);
	QString sN(numNotFoundMachines >= 0 ? QString::number(numNotFoundMachines) : trQuestionMark);
	QString sU(numUnknownMachines >= 0 ? QString::number(numUnknownMachines) : trQuestionMark);
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ROM state info: L:%1 C:%2 M:%3 I:%4 N:%5 U:%6").arg(sL).arg(sC).arg(sM).arg(sI).arg(sN).arg(sU));
	qmc2MainWindow->progressBarMachineList->reset();
	if ( verifyProc )
		delete verifyProc;
	verifyProc = 0;
	qmc2VerifyActive = false;
	if ( romStateCache.isOpen() ) {
		tsRomCache.flush();
		romStateCache.close();
	}
	if ( qmc2SortCriteria == QMC2_SORT_BY_ROM_STATE ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("sorting machine list by %1 in %2 order").arg(tr("ROM state")).arg(qmc2SortOrder == Qt::AscendingOrder ? tr("ascending") : tr("descending")));
		qmc2SortingActive = true;
		qApp->processEvents();
		foreach (QTreeWidgetItem *ti, qmc2ExpandedMachineListItems) {
			qmc2MainWindow->treeWidgetMachineList->collapseItem(ti);
			QList<QTreeWidgetItem *> childrenList = ti->takeChildren();
			foreach (QTreeWidgetItem *ci, ti->takeChildren())
				delete ci;
			QTreeWidgetItem *nameItem = new QTreeWidgetItem(ti);
			nameItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, trWaitingForData);
			nameItem->setText(QMC2_MACHINELIST_COLUMN_ICON, ti->text(QMC2_MACHINELIST_COLUMN_NAME));
		}
		qmc2ExpandedMachineListItems.clear();
		qmc2MainWindow->treeWidgetMachineList->setUpdatesEnabled(false);
		qmc2MainWindow->treeWidgetHierarchy->setUpdatesEnabled(false);
		qmc2MainWindow->treeWidgetCategoryView->setUpdatesEnabled(false);
		qmc2MainWindow->treeWidgetVersionView->setUpdatesEnabled(false);
		qmc2MainWindow->treeWidgetMachineList->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
		qApp->processEvents();
		qmc2MainWindow->treeWidgetHierarchy->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
		qApp->processEvents();
		if ( qmc2MainWindow->treeWidgetCategoryView->topLevelItemCount() > 0 ) {
			qmc2MainWindow->treeWidgetCategoryView->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
			qApp->processEvents();
		}
		if ( qmc2MainWindow->treeWidgetVersionView->topLevelItemCount() > 0 ) {
			qmc2MainWindow->treeWidgetVersionView->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
			qApp->processEvents();
		}
		qmc2MainWindow->treeWidgetMachineList->setUpdatesEnabled(true);
		qmc2MainWindow->treeWidgetHierarchy->setUpdatesEnabled(true);
		qmc2MainWindow->treeWidgetCategoryView->setUpdatesEnabled(true);
		qmc2MainWindow->treeWidgetVersionView->setUpdatesEnabled(true);
		qmc2SortingActive = false;
		QTimer::singleShot(0, qmc2MainWindow, SLOT(scrollToCurrentItem()));
	}
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "RomStateFilter/Enabled", true).toBool() ){
		if ( doFilter )
			QTimer::singleShot(0, this, SLOT(filter()));
		else {
			QTimer::singleShot(0, this, SLOT(enableWidgets()));
			QTimer::singleShot(0, qmc2MainWindow, SLOT(scrollToCurrentItem()));
		}
	} else
		QTimer::singleShot(0, this, SLOT(enableWidgets()));
}

void MachineList::verifyReadyReadStandardOutput()
{
	// process rom verification output
	char romState;
	QString romName, romStateLong; 
	QString s = verifyLastLine + verifyProc->readAllStandardOutput();
#if defined(QMC2_OS_WIN)
	s.replace("\r\n", "\n"); // convert WinDOS's "0x0D 0x0A" to just "0x0A" 
#endif
	QStringList lines = s.split("\n");
	if ( s.endsWith("\n") )
		verifyLastLine.clear();
	else {
		verifyLastLine = lines.last();
		lines.removeLast();
	}
	if ( !verifyCurrentOnly ) {
		// the progress text may have changed in the meantime...
		if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
			qmc2MainWindow->progressBarMachineList->setFormat(tr("ROM check - %p%"));
	}
	bool showROMStatusIcons = qmc2Config->value(QMC2_FRONTEND_PREFIX + "MachineList/ShowROMStatusIcons", true).toBool();
	QChar splitChar(' ');
	for (int i = 0; i < lines.count(); i++) {
		if ( lines[i].startsWith("romset ") ) {
			QStringList words = lines[i].split(splitChar);
			numVerifyRoms++;
			if ( words.count() > 2 ) {
				romName = words[1].remove("\"");
				QTreeWidgetItem *romItem = qmc2MachineListItemHash.value(romName);
				QTreeWidgetItem *hierarchyItem = qmc2HierarchyItemHash.value(romName);
				if ( romItem && hierarchyItem ) {
					QTreeWidgetItem *categoryItem = qmc2CategoryItemHash.value(romName);
					QTreeWidgetItem *versionItem = qmc2VersionItemHash.value(romName);
					bool isBIOS = isBios(romName);
					bool isDev = isDevice(romName);
					if ( words.last() == "good" || lines[i].endsWith("has no roms!") ) {
						romState = 'C';
						romStateLong = QObject::tr("correct");
						numCorrectMachines++;
						if ( showROMStatusIcons ) {
							if ( isBIOS ) {
								romItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectBIOSImageIcon);
								hierarchyItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectBIOSImageIcon);
								if ( categoryItem )
									categoryItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectBIOSImageIcon);
								if ( versionItem )
									versionItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectBIOSImageIcon);
							} else if ( isDev ) {
								romItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectDeviceImageIcon);
								hierarchyItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectDeviceImageIcon);
								if ( categoryItem )
									categoryItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectDeviceImageIcon);
								if ( versionItem )
									versionItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectDeviceImageIcon);
							} else {
								romItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectImageIcon);
								hierarchyItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectImageIcon);
								if ( categoryItem )
									categoryItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectImageIcon);
								if ( versionItem )
									versionItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectImageIcon);
							}
						}
						if ( romItem == qmc2CurrentItem )
							qmc2MainWindow->labelMachineStatus->setPalette(MainWindow::qmc2StatusColorGreen);
					} else if ( words.last() == "bad" ) {
						romState = 'I';
						romStateLong = QObject::tr("incorrect");
						numIncorrectMachines++;
						if ( showROMStatusIcons ) {
							if ( isBIOS ) {
								romItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2IncorrectBIOSImageIcon);
								hierarchyItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2IncorrectBIOSImageIcon);
								if ( categoryItem )
									categoryItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2IncorrectBIOSImageIcon);
								if ( versionItem )
									versionItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2IncorrectBIOSImageIcon);
							} else if ( isDev ) {
								romItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2IncorrectDeviceImageIcon);
								hierarchyItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2IncorrectDeviceImageIcon);
								if ( categoryItem )
									categoryItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2IncorrectDeviceImageIcon);
								if ( versionItem )
									versionItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2IncorrectDeviceImageIcon);
							} else {
								romItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2IncorrectImageIcon);
								hierarchyItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2IncorrectImageIcon);
								if ( categoryItem )
									categoryItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2IncorrectImageIcon);
								if ( versionItem )
									versionItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2IncorrectImageIcon);
							}
						}
						if ( romItem == qmc2CurrentItem )
							qmc2MainWindow->labelMachineStatus->setPalette(MainWindow::qmc2StatusColorRed);
					} else if ( words.last() == "available" ) {
						romState = 'M';
						romStateLong = QObject::tr("mostly correct");
						numMostlyCorrectMachines++;
						if ( showROMStatusIcons ) {
							if ( isBIOS ) {
								romItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2MostlyCorrectBIOSImageIcon);
								hierarchyItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2MostlyCorrectBIOSImageIcon);
								if ( categoryItem )
									categoryItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2MostlyCorrectBIOSImageIcon);
								if ( versionItem )
									versionItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2MostlyCorrectBIOSImageIcon);
							} else if ( isDev ) {
								romItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2MostlyCorrectDeviceImageIcon);
								hierarchyItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2MostlyCorrectDeviceImageIcon);
								if ( categoryItem )
									categoryItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2MostlyCorrectDeviceImageIcon);
								if ( versionItem )
									versionItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2MostlyCorrectDeviceImageIcon);
							} else {
								romItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2MostlyCorrectImageIcon);
								hierarchyItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2MostlyCorrectImageIcon);
								if ( categoryItem )
									categoryItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2MostlyCorrectImageIcon);
								if ( versionItem )
									versionItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2MostlyCorrectImageIcon);
							}
						}
						if ( romItem == qmc2CurrentItem )
							qmc2MainWindow->labelMachineStatus->setPalette(MainWindow::qmc2StatusColorYellowGreen);
					} else if ( words.last() == "missing" || words.last() == "found!" ) {
						romState = 'N';
						romStateLong = QObject::tr("not found");
						numNotFoundMachines++;
						if ( showROMStatusIcons ) {
							if ( isBIOS ) {
								romItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2NotFoundBIOSImageIcon);
								hierarchyItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2NotFoundBIOSImageIcon);
								if ( categoryItem )
									categoryItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2NotFoundBIOSImageIcon);
								if ( versionItem )
									versionItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2NotFoundBIOSImageIcon);
							} else if ( isDev ) {
								romItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2NotFoundDeviceImageIcon);
								hierarchyItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2NotFoundDeviceImageIcon);
								if ( categoryItem )
									categoryItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2NotFoundDeviceImageIcon);
								if ( versionItem )
									versionItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2NotFoundDeviceImageIcon);
							} else {
								romItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2NotFoundImageIcon);
								hierarchyItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2NotFoundImageIcon);
								if ( categoryItem )
									categoryItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2NotFoundImageIcon);
								if ( versionItem )
									versionItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2NotFoundImageIcon);
							}
						}
						if ( romItem == qmc2CurrentItem )
							qmc2MainWindow->labelMachineStatus->setPalette(MainWindow::qmc2StatusColorGrey);
					} else {
						romState = 'U';
						romStateLong = QObject::tr("unknown");
						numUnknownMachines++;
						if ( showROMStatusIcons ) {
							if ( isBIOS ) {
								romItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2UnknownBIOSImageIcon);
								hierarchyItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2UnknownBIOSImageIcon);
								if ( categoryItem )
									categoryItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2UnknownBIOSImageIcon);
								if ( versionItem )
									versionItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2UnknownBIOSImageIcon);
							} else if ( isDev ) {
								romItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2UnknownDeviceImageIcon);
								hierarchyItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2UnknownDeviceImageIcon);
								if ( categoryItem )
									categoryItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2UnknownDeviceImageIcon);
								if ( versionItem )
									versionItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2UnknownDeviceImageIcon);
							} else {
								romItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2UnknownImageIcon);
								hierarchyItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2UnknownImageIcon);
								if ( categoryItem )
									categoryItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2UnknownImageIcon);
								if ( versionItem )
									versionItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2UnknownImageIcon);
							}
						}
						if ( romItem == qmc2CurrentItem )
							qmc2MainWindow->labelMachineStatus->setPalette(MainWindow::qmc2StatusColorBlue);
					}
					machineStatusHash[romName] = romState;
					verifiedList << romName;
					if ( verifyCurrentOnly ) {
						qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ROM status for '%1' is '%2'").arg(checkedItem->text(QMC2_MACHINELIST_COLUMN_MACHINE)).arg(romStateLong));
						numUnknownMachines--;
					} else if ( romStateCache.isOpen() )
						tsRomCache << romName << " " << romState << "\n";
				}
			}
		}
	}
	if ( romStateCache.isOpen() && !verifyCurrentOnly )
		tsRomCache.flush();
	if ( qmc2StopParser && verifyProc )
		verifyProc->kill();
	if ( !verifyCurrentOnly )
		qmc2MainWindow->progressBarMachineList->setValue(numVerifyRoms);
	qmc2MainWindow->labelMachineListStatus->setText(status());
}

bool MachineList::loadIcon(const QString &machineName, QTreeWidgetItem *item)
{
	QIcon cachedIcon(qmc2IconHash.value(machineName));
	if ( !cachedIcon.isNull() ) {
		// use the cached icon
		if ( item )
			item->setIcon(QMC2_MACHINELIST_COLUMN_ICON, cachedIcon);
		else
			qmc2MainWindow->treeWidgetMachineList->setUpdatesEnabled(true);
		return true;
	}
	if ( qmc2IconsPreloaded ) {
		// an icon wasn't found
		if ( item ) {
			qmc2IconHash.insert(machineName, QIcon());
			item->setIcon(QMC2_MACHINELIST_COLUMN_ICON, QIcon());
		} else
			qmc2MainWindow->treeWidgetMachineList->setUpdatesEnabled(true);
		return false;
	}
	if ( qmc2UseIconFile ) {
		// read icons from an archive
		QByteArray imageData;
		QTime preloadTimer, elapsedTime(0, 0, 0, 0);
		preloadTimer.start();

		switch ( qmc2Options->iconFileType() ) {
			case QMC2_ICON_FILETYPE_ZIP:
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("pre-caching icons from ZIP archive"));
				foreach (unzFile iconFile, qmc2IconFileMap) {
					unz_global_info unzGlobalInfo;
					if ( unzGetGlobalInfo(iconFile, &unzGlobalInfo) == UNZ_OK ) {
						int currentMax = qmc2MainWindow->progressBarMachineList->maximum();
						QString oldFormat = qmc2MainWindow->progressBarMachineList->format();
						if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
							qmc2MainWindow->progressBarMachineList->setFormat(tr("Icon cache - %p%"));
						else
							qmc2MainWindow->progressBarMachineList->setFormat("%p%");
						qmc2MainWindow->progressBarMachineList->setRange(0, unzGlobalInfo.number_entry);
						qmc2MainWindow->progressBarMachineList->reset();
						qApp->processEvents();
						char imageBuffer[QMC2_ZIP_BUFFER_SIZE];
						if ( unzGoToFirstFile(iconFile) == UNZ_OK ) {
							int counter = 0;
							char unzFileName[QMC2_MAX_PATH_LENGTH];
							unz_file_info unzFileInfo;
							do {
								if ( unzGetCurrentFileInfo(iconFile, &unzFileInfo, unzFileName, QMC2_MAX_PATH_LENGTH, 0, 0, 0, 0) == UNZ_OK ) {
									QFileInfo fi(unzFileName);
									imageData.clear();
									if ( unzOpenCurrentFile(iconFile) == UNZ_OK ) {
										int len = 0;
										while ( (len = unzReadCurrentFile(iconFile, &imageBuffer, QMC2_ZIP_BUFFER_SIZE)) > 0 )
											imageData.append(imageBuffer, len);
										unzCloseCurrentFile(iconFile);
										QPixmap iconPixmap;
										if ( iconPixmap.loadFromData(imageData) ) {
											QFileInfo fi2(fi.fileName().toLower());
											qmc2IconHash.insert(fi2.baseName(), QIcon(iconPixmap));
										}
									}
								}
								if ( counter++ % QMC2_ICONCACHE_RESPONSIVENESS == 0 ) {
									qmc2MainWindow->progressBarMachineList->setValue(counter);
									qApp->processEvents();
								}
							} while ( unzGoToNextFile(iconFile) != UNZ_END_OF_LIST_OF_FILE );
						}
						qmc2MainWindow->progressBarMachineList->setRange(0, currentMax);
						if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
							qmc2MainWindow->progressBarMachineList->setFormat(oldFormat);
						else
							qmc2MainWindow->progressBarMachineList->setFormat("%p%");
					}
				}
				elapsedTime = elapsedTime.addMSecs(preloadTimer.elapsed());
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (pre-caching icons from ZIP archive, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
				break;

			case QMC2_ICON_FILETYPE_7Z:
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("pre-caching icons from 7z archive"));
				foreach (SevenZipFile *sevenZipFile, qmc2IconFileMap7z) {
					int currentMax = qmc2MainWindow->progressBarMachineList->maximum();
					QString oldFormat = qmc2MainWindow->progressBarMachineList->format();
					if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
						qmc2MainWindow->progressBarMachineList->setFormat(tr("Icon cache - %p%"));
					else
						qmc2MainWindow->progressBarMachineList->setFormat("%p%");
					qmc2MainWindow->progressBarMachineList->setRange(0, sevenZipFile->entryList().count());
					qmc2MainWindow->progressBarMachineList->reset();
					for (int index = 0; index < sevenZipFile->entryList().count(); index++) {
						SevenZipMetaData metaData = sevenZipFile->entryList()[index];
						QFileInfo fi(metaData.name());
						sevenZipFile->read(index, &imageData);
						if ( !sevenZipFile->hasError() ) {
							QPixmap iconPixmap;
							if ( iconPixmap.loadFromData(imageData) ) {
								QFileInfo fi2(fi.fileName().toLower());
								qmc2IconHash.insert(fi2.baseName(), QIcon(iconPixmap));
							}
						}
						if ( index % QMC2_ICONCACHE_RESPONSIVENESS == 0 ) {
							qmc2MainWindow->progressBarMachineList->setValue(index);
							qApp->processEvents();
						}
					}
					qmc2MainWindow->progressBarMachineList->setRange(0, currentMax);
					if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
						qmc2MainWindow->progressBarMachineList->setFormat(oldFormat);
					else
						qmc2MainWindow->progressBarMachineList->setFormat("%p%");
				}
				elapsedTime = elapsedTime.addMSecs(preloadTimer.elapsed());
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (pre-caching icons from 7z archive, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
				break;

#if defined(QMC2_LIBARCHIVE_ENABLED)
			case QMC2_ICON_FILETYPE_ARCHIVE:
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("pre-caching icons from archive"));
				foreach (ArchiveFile *archiveFile, qmc2IconArchiveMap) {
					int currentMax = qmc2MainWindow->progressBarMachineList->maximum();
					QString oldFormat = qmc2MainWindow->progressBarMachineList->format();
					if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
						qmc2MainWindow->progressBarMachineList->setFormat(tr("Icon cache - %p%"));
					else
						qmc2MainWindow->progressBarMachineList->setFormat("%p%");
					qmc2MainWindow->progressBarMachineList->setRange(0, 0);
					qmc2MainWindow->progressBarMachineList->reset();
					ArchiveEntryMetaData metaData;
					int counter = 0;
					while ( archiveFile->seekNextEntry(&metaData) ) {
						QFileInfo fi(metaData.name());
						if ( archiveFile->readEntry(imageData) ) {
							QPixmap iconPixmap;
							if ( iconPixmap.loadFromData(imageData) ) {
								QFileInfo fi2(fi.fileName().toLower());
								qmc2IconHash.insert(fi2.baseName(), QIcon(iconPixmap));
							}
						}
						if ( counter++ % QMC2_ICONCACHE_RESPONSIVENESS == 0 )
							qApp->processEvents();
					}
					qmc2MainWindow->progressBarMachineList->setRange(0, currentMax);
					if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
						qmc2MainWindow->progressBarMachineList->setFormat(oldFormat);
					else
						qmc2MainWindow->progressBarMachineList->setFormat("%p%");
				}
				elapsedTime = elapsedTime.addMSecs(preloadTimer.elapsed());
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (pre-caching icons from archive, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
				break;
#endif
		}
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("%n icon(s) loaded", "", qmc2IconHash.count()));
		qmc2IconsPreloaded = true;
		if ( !item )
			qmc2MainWindow->treeWidgetMachineList->setUpdatesEnabled(true);
		return loadIcon(machineName, item);
	} else {
		// load icons from a directory
		QTime preloadTimer, elapsedTime(0, 0, 0, 0);
		preloadTimer.start();
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("pre-caching icons from directory"));
		int currentMax = qmc2MainWindow->progressBarMachineList->maximum();
		QString oldFormat = qmc2MainWindow->progressBarMachineList->format();
		foreach(QString icoDir, qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/IconDirectory").toString().split(";", QString::SkipEmptyParts)) {
			qApp->processEvents();
			QDir iconDirectory(icoDir);
			QFileInfoList iconFiles = iconDirectory.entryInfoList();
			if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
				qmc2MainWindow->progressBarMachineList->setFormat(tr("Icon cache - %p%"));
			else
				qmc2MainWindow->progressBarMachineList->setFormat("%p%");
			qmc2MainWindow->progressBarMachineList->setRange(0, iconFiles.count());
			qmc2MainWindow->progressBarMachineList->reset();
			qApp->processEvents();
			for (int fileCount = 0; fileCount < iconFiles.count(); fileCount++) {
				QFileInfo fi = iconFiles[fileCount];
				if ( fi.isFile() ) {
					QPixmap iconPixmap;
					if ( iconPixmap.load(fi.absoluteFilePath()) ) {
						qmc2IconHash.insert(fi.baseName().toLower(), QIcon(iconPixmap));
					}
				}
				if ( fileCount % QMC2_ICONCACHE_RESPONSIVENESS == 0 ) {
					qmc2MainWindow->progressBarMachineList->setValue(fileCount);
					qApp->processEvents();
				}
			}
		}
		qmc2MainWindow->progressBarMachineList->setRange(0, currentMax);
		if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
			qmc2MainWindow->progressBarMachineList->setFormat(oldFormat);
		else
			qmc2MainWindow->progressBarMachineList->setFormat("%p%");
		elapsedTime = elapsedTime.addMSecs(preloadTimer.elapsed());
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (pre-caching icons from directory, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("%n icon(s) loaded", "", qmc2IconHash.count()));
		qmc2IconsPreloaded = true;
		if ( !item )
			qmc2MainWindow->treeWidgetMachineList->setUpdatesEnabled(true);
		return loadIcon(machineName, item);
	}
}

void MachineList::loadCategoryIni()
{
	if ( !mergeCategories ) {
		clearCategoryNames();
		categoryHash.clear();
	}
	QTime loadTimer, elapsedTime(0, 0, 0, 0);
	loadTimer.start();
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading category.ini"));
	int currentMax = qmc2MainWindow->progressBarMachineList->maximum();
	QString oldFormat = qmc2MainWindow->progressBarMachineList->format();
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
		qmc2MainWindow->progressBarMachineList->setFormat(tr("Category.ini - %p%"));
	else
		qmc2MainWindow->progressBarMachineList->setFormat("%p%");
	qmc2MainWindow->progressBarMachineList->reset();
	QFile categoryIniFile(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/CategoryIni").toString());
	int entryCounter = 0;
	if ( categoryIniFile.open(QFile::ReadOnly) ) {
		qmc2MainWindow->progressBarMachineList->setRange(0, categoryIniFile.size());
		QTextStream tsCategoryIni(&categoryIniFile);
		QString categoryName;
		QRegExp rxCategoryName("^\\[.*\\]$");
		QString guiLanguage = qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Language", "us").toString();
		bool trFound = false;
		while ( !tsCategoryIni.atEnd() ) {
			QString categoryLine(tsCategoryIni.readLine().simplified().trimmed());
			qmc2MainWindow->progressBarMachineList->setValue(categoryIniFile.pos());
			if ( categoryLine.isEmpty() )
				continue;
			if ( categoryLine.indexOf(rxCategoryName) == 0 ) {
				categoryName = categoryLine.mid(1, categoryLine.length() - 2);
				QHash<QString, QString> translations;
				categoryLine = tsCategoryIni.readLine().simplified().trimmed();
				trFound = false;
				while ( !categoryLine.isEmpty() && categoryLine.startsWith("tr[") ) {
					int endIndex = categoryLine.indexOf(']', 3);
					QString trLanguage(categoryLine.mid(3, endIndex - 3));
					translations.insert(trLanguage, categoryLine.mid(endIndex + 2, categoryLine.length() - endIndex - 2));
					trFound = (trLanguage.compare(guiLanguage) == 0);
					if ( trFound )
						break;
					categoryLine = tsCategoryIni.readLine().simplified().trimmed();
				}
				if ( trFound )
					categoryName = translations[guiLanguage];
			} else if ( !categoryName.isEmpty() ) {
				if ( !categoryNames.contains(categoryName) )
					categoryNames[categoryName] = new QString(categoryName);
				categoryHash.insert(categoryLine, categoryNames.value(categoryName));
				entryCounter++;
			}
		}
		categoryIniFile.close();
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: can't open '%1' for reading -- no category.ini data available").arg(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/CategoryIni").toString()));
	qmc2MainWindow->progressBarMachineList->setRange(0, currentMax);
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
		qmc2MainWindow->progressBarMachineList->setFormat(oldFormat);
	else
		qmc2MainWindow->progressBarMachineList->setFormat("%p%");
	elapsedTime = elapsedTime.addMSecs(loadTimer.elapsed());
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading category.ini, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("%n category record(s) loaded", "", entryCounter));
}

void MachineList::createCategoryView()
{
	if ( creatingCatView || qmc2MainWindow->stackedWidgetView->currentIndex() != QMC2_VIEWCATEGORY_INDEX )
		return;
	qmc2CategoryItemHash.clear();
	qmc2MainWindow->treeWidgetCategoryView->setVisible(false);
	((AspectRatioLabel *)qmc2MainWindow->labelCreatingCategoryView)->setLabelText(tr("Loading, please wait..."));
	qmc2MainWindow->labelCreatingCategoryView->setVisible(true);
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ShowLoadingAnimation", true).toBool() )
		qmc2MainWindow->loadAnimMovie->start();
	if ( qmc2ReloadActive ) {
		if ( !qmc2StopParser )
			QTimer::singleShot(QMC2_RELOAD_POLL_INTERVAL, this, SLOT(createCategoryView()));
		return;
	}
	creatingCatView = true;
	qmc2MainWindow->treeWidgetCategoryView->setColumnHidden(QMC2_MACHINELIST_COLUMN_CATEGORY, true);
	if ( !qmc2StopParser ) {
		if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ShowLoadingAnimation", true).toBool() )
			qmc2MainWindow->loadAnimMovie->start();
		qmc2MainWindow->treeWidgetCategoryView->clear();
		QString oldFormat = qmc2MainWindow->progressBarMachineList->format();
		if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
			qmc2MainWindow->progressBarMachineList->setFormat(tr("Category view - %p%"));
		else
			qmc2MainWindow->progressBarMachineList->setFormat("%p%");
		qmc2MainWindow->progressBarMachineList->setRange(0, qmc2MainWindow->treeWidgetMachineList->topLevelItemCount());
		qmc2MainWindow->progressBarMachineList->reset();
		bool showDeviceSets = qmc2Config->value(QMC2_FRONTEND_PREFIX + "MachineList/ShowDeviceSets", true).toBool();
		bool showBiosSets = qmc2Config->value(QMC2_FRONTEND_PREFIX + "MachineList/ShowBiosSets", true).toBool();
		QList<QTreeWidgetItem *> itemList, hideList;
		QHash<QString, QTreeWidgetItem *> itemHash;
		int loadResponse = qmc2MainWindow->treeWidgetMachineList->topLevelItemCount() / QMC2_GENERAL_LOADING_UPDATES;
		if ( loadResponse == 0 )
			loadResponse = 25;
		QHash<QTreeWidgetItem *, int> childCountHash;
		QString trSystemBios(tr("System / BIOS"));
		QString trSystemDevice(tr("System / Device"));
		for (int i = 0; i < qmc2MainWindow->treeWidgetMachineList->topLevelItemCount(); i++) {
			if ( i % loadResponse == 0 ) {
				qmc2MainWindow->progressBarMachineList->setValue(i);
				qApp->processEvents();
			}
			QTreeWidgetItem *baseItem = qmc2MainWindow->treeWidgetMachineList->topLevelItem(i);
			QString machineName = baseItem->text(QMC2_MACHINELIST_COLUMN_NAME);
			QString category;
			bool isBIOS = isBios(machineName);
			bool isDev = isDevice(machineName);
			if ( isBIOS )
				category = trSystemBios;
			else if ( isDev )
				category = trSystemDevice;
			else
				category = baseItem->text(QMC2_MACHINELIST_COLUMN_CATEGORY);
			QTreeWidgetItem *categoryItem = itemHash[category];
			if ( !categoryItem ) {
				categoryItem = new QTreeWidgetItem();
				categoryItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, category);
				itemList << categoryItem;
				itemHash[category] = categoryItem;
				childCountHash[categoryItem] = 0;
			}
			QTreeWidgetItem *machineItem = new MachineListItem(categoryItem);
			childCountHash[categoryItem]++;
			if ( (isBIOS && !showBiosSets) || (isDev && !showDeviceSets) ) {
				hideList << machineItem;
				childCountHash[categoryItem]--;
			}
			machineItem->setFlags(MachineListItem::defaultItemFlags);
			machineItem->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, baseItem->checkState(QMC2_MACHINELIST_COLUMN_TAG));
			machineItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, baseItem->text(QMC2_MACHINELIST_COLUMN_MACHINE));
			machineItem->setText(QMC2_MACHINELIST_COLUMN_YEAR, baseItem->text(QMC2_MACHINELIST_COLUMN_YEAR));
			machineItem->setText(QMC2_MACHINELIST_COLUMN_MANU, baseItem->text(QMC2_MACHINELIST_COLUMN_MANU));
			machineItem->setText(QMC2_MACHINELIST_COLUMN_NAME, baseItem->text(QMC2_MACHINELIST_COLUMN_NAME));
			machineItem->setText(QMC2_MACHINELIST_COLUMN_SRCFILE, baseItem->text(QMC2_MACHINELIST_COLUMN_SRCFILE));
			machineItem->setText(QMC2_MACHINELIST_COLUMN_RTYPES, baseItem->text(QMC2_MACHINELIST_COLUMN_RTYPES));
			machineItem->setText(QMC2_MACHINELIST_COLUMN_PLAYERS, baseItem->text(QMC2_MACHINELIST_COLUMN_PLAYERS));
			machineItem->setText(QMC2_MACHINELIST_COLUMN_DRVSTAT, baseItem->text(QMC2_MACHINELIST_COLUMN_DRVSTAT));
			machineItem->setText(QMC2_MACHINELIST_COLUMN_VERSION, baseItem->text(QMC2_MACHINELIST_COLUMN_VERSION));
			machineItem->setWhatsThis(QMC2_MACHINELIST_COLUMN_RANK, baseItem->whatsThis(QMC2_MACHINELIST_COLUMN_RANK));
			machineItem->setIcon(QMC2_MACHINELIST_COLUMN_ICON, baseItem->icon(QMC2_MACHINELIST_COLUMN_ICON));
			if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "MachineList/ShowROMStatusIcons", true).toBool() ) {
				switch ( machineStatusHash.value(machineName) ) {
					case 'C':
						if ( isBIOS )
							machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectBIOSImageIcon);
						else if ( isDev )
							machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectDeviceImageIcon);
						else
							machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectImageIcon);
						break;
					case 'M':
						if ( isBIOS )
							machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2MostlyCorrectBIOSImageIcon);
						else if ( isDev )
							machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2MostlyCorrectDeviceImageIcon);
						else
							machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2MostlyCorrectImageIcon);
						break;
					case 'I':
						if ( isBIOS )
							machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2IncorrectBIOSImageIcon);
						else if ( isDev )
							machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2IncorrectDeviceImageIcon);
						else
							machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2IncorrectImageIcon);
						break;
					case 'N':
						if ( isBIOS )
							machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2NotFoundBIOSImageIcon);
						else if ( isDev )
							machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2NotFoundDeviceImageIcon);
						else
							machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2NotFoundImageIcon);
						break;
					case 'U':
					default:
						if ( isBIOS )
							machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2UnknownBIOSImageIcon);
						else if ( isDev )
							machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2UnknownDeviceImageIcon);
						else
							machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2UnknownImageIcon);
						break;
				}
			}
			qmc2CategoryItemHash[machineName] = machineItem;
		}
		foreach (QTreeWidgetItem *item, itemList) {
			if ( childCountHash.contains(item) ) {
				if ( childCountHash[item] <= 0 )
					hideList << item;
			} else
				hideList << item;
		}
		qmc2MainWindow->treeWidgetCategoryView->insertTopLevelItems(0, itemList);
		for (int i = 0; i < hideList.count(); i++)
			hideList[i]->setHidden(true);
		qmc2MainWindow->treeWidgetCategoryView->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
		qmc2MainWindow->progressBarMachineList->reset();
		qmc2MainWindow->progressBarMachineList->setFormat(oldFormat);
		if ( qmc2MainWindow->stackedWidgetView->currentIndex() == QMC2_VIEWCATEGORY_INDEX )
			QTimer::singleShot(QMC2_RANK_UPDATE_DELAY, qmc2MainWindow, SLOT(treeWidgetCategoryView_verticalScrollChanged()));
	}
	qmc2MainWindow->loadAnimMovie->setPaused(true);
	qmc2MainWindow->labelCreatingCategoryView->setVisible(false);
	qmc2MainWindow->treeWidgetCategoryView->setVisible(true);
	QTimer::singleShot(0, qmc2MainWindow, SLOT(scrollToCurrentItem()));
	qmc2MainWindow->treeWidgetCategoryView->setFocus();
	creatingCatView = false;
}

void MachineList::loadCatverIni()
{
	clearCategoryNames();
	categoryHash.clear();
	clearVersionNames();
	versionHash.clear();
	QTime loadTimer, elapsedTime(0, 0, 0, 0);
	loadTimer.start();
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("loading catver.ini"));
	int currentMax = qmc2MainWindow->progressBarMachineList->maximum();
	QString oldFormat(qmc2MainWindow->progressBarMachineList->format());
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
		qmc2MainWindow->progressBarMachineList->setFormat(tr("Catver.ini - %p%"));
	else
		qmc2MainWindow->progressBarMachineList->setFormat("%p%");
	qmc2MainWindow->progressBarMachineList->reset();
	QFile catverIniFile(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/CatverIni").toString());
	if ( catverIniFile.open(QFile::ReadOnly) ) {
		qmc2MainWindow->progressBarMachineList->setRange(0, catverIniFile.size());
		QTextStream tsCatverIni(&catverIniFile);
		bool categoryDone = false, versionDone = false;
		int lineCounter = 0;
		char catVerSwitch = 0;
		QChar splitChar('='), dotChar('.'), zeroChar('0');
		QString catStr("[Category]"), verStr("[VerAdded]");
		while ( !tsCatverIni.atEnd() ) {
			QString catverLine(tsCatverIni.readLine());
			if ( lineCounter++ % QMC2_CATVERINI_LOAD_RESPONSE == 0 ) {
				qmc2MainWindow->progressBarMachineList->setValue(catverIniFile.pos());
				qApp->processEvents();
			}
			if ( catverLine.isEmpty() )
				continue;
			if ( !categoryDone && catVerSwitch != 1 ) {
				if ( catverLine.indexOf(catStr) >= 0 ) {
					categoryDone = true;
					catVerSwitch = 1;
				}
			}
			if ( !versionDone && catVerSwitch != 2 ) {
				if ( catverLine.indexOf(verStr) >= 0 ) {
					versionDone = true;
					catVerSwitch = 2;
				}
			}
			QStringList tokens(catverLine.split(splitChar, QString::SkipEmptyParts));
			if ( tokens.count() > 1 ) {
				QString token1(tokens.at(1).trimmed());
				switch ( catVerSwitch ) {
					case 1: // category
						if ( !categoryNames.contains(token1) )
							categoryNames.insert(token1, new QString(token1));
						categoryHash.insert(tokens.at(0).trimmed(), categoryNames.value(token1));
						break;
					case 2: // version
						if ( token1.startsWith(dotChar) )
							token1.prepend(zeroChar);
						if ( !versionNames.contains(token1) )
							versionNames.insert(token1, new QString(token1));
						versionHash.insert(tokens.at(0).trimmed(), versionNames.value(token1));
						break;
				}
			}
		}
		catverIniFile.close();
		qmc2MainWindow->progressBarMachineList->setValue(qmc2MainWindow->progressBarMachineList->maximum());
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("ERROR: can't open '%1' for reading -- no catver.ini data available").arg(qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/CatverIni").toString()));
	qmc2MainWindow->progressBarMachineList->setRange(0, currentMax);
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
		qmc2MainWindow->progressBarMachineList->setFormat(oldFormat);
	else
		qmc2MainWindow->progressBarMachineList->setFormat("%p%");
	elapsedTime = elapsedTime.addMSecs(loadTimer.elapsed());
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (loading catver.ini, elapsed time = %1)").arg(elapsedTime.toString("mm:ss.zzz")));
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("%1 category / %2 version records loaded").arg(categoryHash.count()).arg(versionHash.count()));
}

void MachineList::createVersionView()
{
	if ( creatingVerView || qmc2MainWindow->stackedWidgetView->currentIndex() != QMC2_VIEWVERSION_INDEX )
		return;
	qmc2VersionItemHash.clear();
	qmc2MainWindow->treeWidgetVersionView->setVisible(false);
	((AspectRatioLabel *)qmc2MainWindow->labelCreatingVersionView)->setLabelText(tr("Loading, please wait..."));
	qmc2MainWindow->labelCreatingVersionView->setVisible(true);
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ShowLoadingAnimation", true).toBool() )
		qmc2MainWindow->loadAnimMovie->start();
	if ( qmc2ReloadActive ) {
		if ( !qmc2StopParser )
			QTimer::singleShot(QMC2_RELOAD_POLL_INTERVAL, this, SLOT(createVersionView()));
		return;
	}
	creatingVerView = true;
	qmc2MainWindow->treeWidgetVersionView->setColumnHidden(QMC2_MACHINELIST_COLUMN_VERSION, true);
	if ( !qmc2StopParser ) {
		if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ShowLoadingAnimation", true).toBool() )
			qmc2MainWindow->loadAnimMovie->start();
		qmc2MainWindow->treeWidgetVersionView->clear();
		QString oldFormat = qmc2MainWindow->progressBarMachineList->format();
		if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/ProgressTexts").toBool() )
			qmc2MainWindow->progressBarMachineList->setFormat(tr("Version view - %p%"));
		else
			qmc2MainWindow->progressBarMachineList->setFormat("%p%");
		qmc2MainWindow->progressBarMachineList->setRange(0, versionHash.count());
		qmc2MainWindow->progressBarMachineList->reset();
		bool showDeviceSets = qmc2Config->value(QMC2_FRONTEND_PREFIX + "MachineList/ShowDeviceSets", true).toBool();
		bool showBiosSets = qmc2Config->value(QMC2_FRONTEND_PREFIX + "MachineList/ShowBiosSets", true).toBool();
		QList<QTreeWidgetItem *> itemList, hideList;
		QHash<QString, QTreeWidgetItem *> itemHash;
		int loadResponse = numMachines / QMC2_GENERAL_LOADING_UPDATES;
		if ( loadResponse == 0 )
			loadResponse = 25;
		QHash<QTreeWidgetItem *, int> childCountHash;
		for (int i = 0; i < qmc2MainWindow->treeWidgetMachineList->topLevelItemCount(); i++) {
			if ( i % loadResponse == 0 ) {
				qmc2MainWindow->progressBarMachineList->setValue(i);
				qApp->processEvents();
			}
			QTreeWidgetItem *baseItem = qmc2MainWindow->treeWidgetMachineList->topLevelItem(i);
			QString machineName = baseItem->text(QMC2_MACHINELIST_COLUMN_NAME);
			QString version = baseItem->text(QMC2_MACHINELIST_COLUMN_VERSION);
			QTreeWidgetItem *versionItem = itemHash[version];
			if ( !versionItem ) {
				versionItem = new QTreeWidgetItem();
				versionItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, version);
				itemList << versionItem;
				itemHash[version] = versionItem;
			}
			QTreeWidgetItem *machineItem = new MachineListItem(versionItem);
			bool isBIOS = isBios(machineName);
			bool isDev = isDevice(machineName);
			childCountHash[versionItem]++;
			if ( (isBIOS && !showBiosSets) || (isDev && !showDeviceSets) ) {
				hideList << machineItem;
				childCountHash[versionItem]--;
			}
			machineItem->setFlags(MachineListItem::defaultItemFlags);
			machineItem->setCheckState(QMC2_MACHINELIST_COLUMN_TAG, baseItem->checkState(QMC2_MACHINELIST_COLUMN_TAG));
			machineItem->setText(QMC2_MACHINELIST_COLUMN_MACHINE, baseItem->text(QMC2_MACHINELIST_COLUMN_MACHINE));
			machineItem->setText(QMC2_MACHINELIST_COLUMN_YEAR, baseItem->text(QMC2_MACHINELIST_COLUMN_YEAR));
			machineItem->setText(QMC2_MACHINELIST_COLUMN_MANU, baseItem->text(QMC2_MACHINELIST_COLUMN_MANU));
			machineItem->setText(QMC2_MACHINELIST_COLUMN_NAME, baseItem->text(QMC2_MACHINELIST_COLUMN_NAME));
			machineItem->setText(QMC2_MACHINELIST_COLUMN_SRCFILE, baseItem->text(QMC2_MACHINELIST_COLUMN_SRCFILE));
			machineItem->setText(QMC2_MACHINELIST_COLUMN_RTYPES, baseItem->text(QMC2_MACHINELIST_COLUMN_RTYPES));
			machineItem->setText(QMC2_MACHINELIST_COLUMN_PLAYERS, baseItem->text(QMC2_MACHINELIST_COLUMN_PLAYERS));
			machineItem->setText(QMC2_MACHINELIST_COLUMN_DRVSTAT, baseItem->text(QMC2_MACHINELIST_COLUMN_DRVSTAT));
			machineItem->setText(QMC2_MACHINELIST_COLUMN_CATEGORY, baseItem->text(QMC2_MACHINELIST_COLUMN_CATEGORY));
			machineItem->setWhatsThis(QMC2_MACHINELIST_COLUMN_RANK, baseItem->whatsThis(QMC2_MACHINELIST_COLUMN_RANK));
			machineItem->setIcon(QMC2_MACHINELIST_COLUMN_ICON, baseItem->icon(QMC2_MACHINELIST_COLUMN_ICON));
			if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "MachineList/ShowROMStatusIcons", true).toBool() ) {
				switch ( machineStatusHash.value(machineName) ) {
					case 'C':
						if ( isBIOS )
							machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectBIOSImageIcon);
						else if ( isDev )
							machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectDeviceImageIcon);
						else
							machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2CorrectImageIcon);
						break;
					case 'M':
						if ( isBIOS )
							machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2MostlyCorrectBIOSImageIcon);
						else if ( isDev )
							machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2MostlyCorrectDeviceImageIcon);
						else
							machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2MostlyCorrectImageIcon);
						break;
					case 'I':
						if ( isBIOS )
							machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2IncorrectBIOSImageIcon);
						else if ( isDev )
							machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2IncorrectDeviceImageIcon);
						else
							machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2IncorrectImageIcon);
						break;
					case 'N':
						if ( isBIOS )
							machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2NotFoundBIOSImageIcon);
						else if ( isDev )
							machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2NotFoundDeviceImageIcon);
						else
							machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2NotFoundImageIcon);
						break;
					case 'U':
					default:
						if ( isBIOS )
							machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2UnknownBIOSImageIcon);
						else if ( isDev )
							machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2UnknownDeviceImageIcon);
						else
							machineItem->setIcon(QMC2_MACHINELIST_COLUMN_MACHINE, qmc2UnknownImageIcon);
						break;
				}
			}
			qmc2VersionItemHash[machineName] = machineItem;
		}
		foreach (QTreeWidgetItem *item, itemList) {
			if ( childCountHash.contains(item) ) {
				if ( childCountHash[item] <= 0 )
					hideList << item;
			} else
				hideList << item;
		}
		qmc2MainWindow->treeWidgetVersionView->insertTopLevelItems(0, itemList);
		for (int i = 0; i < hideList.count(); i++)
			hideList[i]->setHidden(true);
		qmc2MainWindow->treeWidgetVersionView->sortItems(qmc2MainWindow->sortCriteriaLogicalIndex(), qmc2SortOrder);
		qmc2MainWindow->progressBarMachineList->reset();
		qmc2MainWindow->progressBarMachineList->setFormat(oldFormat);
		if ( qmc2MainWindow->stackedWidgetView->currentIndex() == QMC2_VIEWVERSION_INDEX )
			QTimer::singleShot(QMC2_RANK_UPDATE_DELAY, qmc2MainWindow, SLOT(treeWidgetVersionView_verticalScrollChanged()));
	}
	qmc2MainWindow->loadAnimMovie->setPaused(true);
	qmc2MainWindow->labelCreatingVersionView->setVisible(false);
	qmc2MainWindow->treeWidgetVersionView->setVisible(true);
	QTimer::singleShot(0, qmc2MainWindow, SLOT(scrollToCurrentItem()));
	qmc2MainWindow->treeWidgetVersionView->setFocus();
	creatingVerView = false;
}

QString MachineList::romStatus(const QString &systemName, bool translated)
{
	switch ( machineStatusHash.value(systemName) ) {
		case 'C':
			return translated ? tr("correct") : "correct";
		case 'M':
			return translated ? tr("mostly correct") : "mostly correct";
		case 'I':
			return translated ? tr("incorrect") : "incorrect";
		case 'N':
			return translated ? tr("not found") : "not found";
		default:
			return translated ? tr("unknown") : "unknown";
	}
}

QString MachineList::lookupDriverName(const QString &systemName)
{
	QString driverName(driverNameHash.value(systemName));
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

void MachineList::clearCategoryNames()
{
	foreach (QString *category, categoryNames)
		if ( category )
			delete category;
	categoryNames.clear();
}

void MachineList::clearVersionNames()
{
	foreach (QString *version, versionNames)
		if ( version )
			delete version;
	versionNames.clear();
}

bool MachineListItem::operator<(const QTreeWidgetItem &otherItem) const
{
	switch ( qmc2SortCriteria ) {
		case QMC2_SORT_BY_DESCRIPTION:
			return (text(QMC2_MACHINELIST_COLUMN_MACHINE).compare(otherItem.text(QMC2_MACHINELIST_COLUMN_MACHINE), Qt::CaseInsensitive) < 0);

		case QMC2_SORT_BY_ROM_STATE:
			return (qmc2MachineList->machineStatusHash.value(text(QMC2_MACHINELIST_COLUMN_NAME)) < qmc2MachineList->machineStatusHash.value(otherItem.text(QMC2_MACHINELIST_COLUMN_NAME)));

		case QMC2_SORT_BY_TAG:
			return (int(checkState(QMC2_MACHINELIST_COLUMN_TAG)) < int(otherItem.checkState(QMC2_MACHINELIST_COLUMN_TAG)));

		case QMC2_SORT_BY_YEAR:
			return (text(QMC2_MACHINELIST_COLUMN_YEAR).compare(otherItem.text(QMC2_MACHINELIST_COLUMN_YEAR), Qt::CaseInsensitive) < 0);

		case QMC2_SORT_BY_MANUFACTURER:
			return (text(QMC2_MACHINELIST_COLUMN_MANU).compare(otherItem.text(QMC2_MACHINELIST_COLUMN_MANU), Qt::CaseInsensitive) < 0);

		case QMC2_SORT_BY_NAME:
			return (text(QMC2_MACHINELIST_COLUMN_NAME).compare(otherItem.text(QMC2_MACHINELIST_COLUMN_NAME), Qt::CaseInsensitive) < 0);

		case QMC2_SORT_BY_ROMTYPES:
			return (text(QMC2_MACHINELIST_COLUMN_RTYPES).compare(otherItem.text(QMC2_MACHINELIST_COLUMN_RTYPES), Qt::CaseInsensitive) < 0);

		case QMC2_SORT_BY_PLAYERS:
			return (text(QMC2_MACHINELIST_COLUMN_PLAYERS).compare(otherItem.text(QMC2_MACHINELIST_COLUMN_PLAYERS), Qt::CaseInsensitive) < 0);

		case QMC2_SORT_BY_DRVSTAT:
			return (text(QMC2_MACHINELIST_COLUMN_DRVSTAT).compare(otherItem.text(QMC2_MACHINELIST_COLUMN_DRVSTAT), Qt::CaseInsensitive) < 0);

		case QMC2_SORT_BY_SRCFILE:
			return (text(QMC2_MACHINELIST_COLUMN_SRCFILE).compare(otherItem.text(QMC2_MACHINELIST_COLUMN_SRCFILE), Qt::CaseInsensitive) < 0);

		case QMC2_SORT_BY_RANK:
			return (whatsThis(QMC2_MACHINELIST_COLUMN_RANK).toInt() > otherItem.whatsThis(QMC2_MACHINELIST_COLUMN_RANK).toInt());

		case QMC2_SORT_BY_CATEGORY:
			return (text(QMC2_MACHINELIST_COLUMN_CATEGORY).compare(otherItem.text(QMC2_MACHINELIST_COLUMN_CATEGORY), Qt::CaseInsensitive) < 0);

		case QMC2_SORT_BY_VERSION:
			return (text(QMC2_MACHINELIST_COLUMN_VERSION).compare(otherItem.text(QMC2_MACHINELIST_COLUMN_VERSION), Qt::CaseInsensitive) < 0);

		default:
			return false;
	}
}

bool MachineListItem::isBios()
{
	return qmc2MachineList->isBios(id());
}

bool MachineListItem::isDevice()
{
	return qmc2MachineList->isDevice(id());
}

char MachineListItem::romStatus()
{
	return qmc2MachineList->machineStatusHash.value(id());
}

QString MachineListItem::parentId()
{
	return qmc2ParentHash.value(id());
}
