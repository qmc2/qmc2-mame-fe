#include <QCryptographicHash>
#include <QHeaderView>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QScrollBar>
#include <QTest>
#include <QMap>
#include <QHash>
#include <QHashIterator>
#include <QFileDialog>
#include <QClipboard>
#include <QDateTime>
#include <QDate>
#include <QTime>
#include <QXmlQuery>
#include <QPalette>
#include <QRegExp>
#if defined(QMC2_OS_WIN)
#include <windows.h>
#endif

#include "romalyzer.h"
#include "qmc2main.h"
#include "options.h"
#include "machinelist.h"
#include "macros.h"
#include "unzip.h"
#include "zip.h"
#include "sevenzipfile.h"
#include "softwarelist.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;
extern Options *qmc2Options;
extern MachineList *qmc2MachineList;
extern bool qmc2ReloadActive;
extern bool qmc2CleaningUp;
extern bool qmc2EarlyStartup;
extern bool qmc2StopParser;
extern SoftwareList *qmc2SoftwareList;
extern QHash<QString, QTreeWidgetItem *> qmc2MachineListItemHash;
extern QHash<QString, QTreeWidgetItem *> qmc2HierarchyItemHash;
extern QHash<QString, QTreeWidgetItem *> qmc2CategoryItemHash;
extern QHash<QString, QTreeWidgetItem *> qmc2VersionItemHash;
extern QAbstractItemView::ScrollHint qmc2CursorPositioningMode;

/*
  HOWTO: Calculate the 32-bit CRC of a QByteArray with zlib:

  #include <QByteArray>
  #include <zlib.h>
  ...
  QByteArray ba("This is a test -- 123 :)!");
  ulong crc = crc32(0, NULL, 0);
  crc = crc32(crc, (const Bytef *)ba.data(), ba.size());
  printf("CRC-32 = %x\n", crc);
*/

/*
  INFO: How MAME searches for ROMs & CHDs of a game

  <rompath> = <first_rompath>
  foreach <file> {
    1) try <rompath>/<machine>/<file> - if okay skip to 7)
    2) try <file> from <rompath>/<machine>.7z/.zip - if okay skip to 7)
    3) if more <rompaths> exists, retry 1) and 2) for <rompath> = <next_rompath>
    4a) if a <merge> exists, retry 1), 2) and 3) for <romof>/<merge> instead of <machine>/<file>
    4b) if <merge> is empty, retry 1), 2) and 3) for <romof>/<file> instead of <machine>/<file>
    6) <file> was not found - stop
    7) load <file> and check CRC
    8) ...
  }

  Backward engineering powered by strace :)! 
*/

ROMAlyzer::ROMAlyzer(QWidget *parent, int romalyzerMode)
#if defined(QMC2_OS_WIN)
	: QDialog(parent, Qt::Dialog)
#else
	: QDialog(parent, Qt::Dialog | Qt::SubWindow)
#endif
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::ROMAlyzer(QWidget *parent = %1)").arg((qulonglong)parent));
#endif
  
	setupUi(this);
	setMode(romalyzerMode);
	setActive(false);
	setPaused(false);

	m_checkSumDbQueryStatusPixmap = QPixmap(QString::fromUtf8(":/data/img/database.png"));

	treeWidgetChecksums->header()->setSortIndicatorShown(false);
	treeWidgetChecksums->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/" + m_settingsKey + "/ReportHeaderState", QByteArray()).toByteArray());
	treeWidgetChecksumWizardSearchResult->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/" + m_settingsKey + "/ChecksumWizardHeaderState", QByteArray()).toByteArray());
	move(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/" + m_settingsKey + "/Position", QPoint(0, 0)).toPoint());
	resize(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/" + m_settingsKey + "/Size", QSize(600, 800)).toSize());

	// compression types for CHD v3 and v4
	chdCompressionTypes << tr("none") << tr("zlib") << tr("zlib+") << tr("A/V codec");

	// compression types for CHD v5
	chdCompressionTypesV5["avhu"] = tr("avhu (A/V Huffman)");
	chdCompressionTypesV5["cdfl"] = tr("cdfl (CD FLAC)");
	chdCompressionTypesV5["cdlz"] = tr("cdlz (CD LZMA)");
	chdCompressionTypesV5["cdzl"] = tr("cdzl (CD Deflate)");
	chdCompressionTypesV5["flac"] = tr("flac (FLAC)");
	chdCompressionTypesV5["huff"] = tr("huff (Huffman)");
	chdCompressionTypesV5["lzma"] = tr("lzma (LZMA)");
	chdCompressionTypesV5["zlib"] = tr("zlib (Deflate)");

	chdManagerRunning = chdManagerMD5Success = chdManagerSHA1Success = false;
#if QMC2_CHD_CURRENT_VERSION < 5
	chdManagerCurrentHunk = chdManagerTotalHunks = 0;
#else
	chdManagerCurrentHunk = 0;
	chdManagerTotalHunks = 100;
	checkBoxVerifyCHDs->setToolTip(tr("Verify CHDs through 'chdman verify'"));
	checkBoxUpdateCHDs->setToolTip(tr("Try to update CHDs if their header indicates an older version ('chdman copy')"));
#endif
	lastRowCount = -1;

	adjustIconSizes();

	widgetCheckSumDbQueryStatus->setVisible(false);
	pushButtonPause->setVisible(false);

	wizardSearch = quickSearch = false;

	QFont logFont;
	logFont.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/LogFont").toString());
	textBrowserLog->setFont(logFont);

	connect(&animTimer, SIGNAL(timeout()), this, SLOT(animationTimeout()));

	QString s;
	QAction *action;

	romFileContextMenu = new QMenu(this);
	romFileContextMenu->hide();
  
	s = tr("Search check-sum");
	action = romFileContextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/filefind.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(runChecksumWizard()));

	romSetContextMenu = new QMenu(this);
	romSetContextMenu->hide();
  
	s = tr("Rewrite set");
	action = romSetContextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/filesave.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(runSetRewriter()));
	actionRewriteSet = action;

	s = tr("Analyse referenced devices");
	action = romSetContextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/romalyzer.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(analyzeDeviceRefs()));
	actionAnalyzeDeviceRefs = action;

	s = tr("Copy to clipboard");
	action = romSetContextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/editcopy.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(copyToClipboard()));

	s = tr("Copy to clipboard (bad / missing dumps)");
	action = romSetContextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/editcopybad.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(copyBadToClipboard()));
	actionCopyBadToClipboard = action;

	// setup tools-menu
	toolsMenu = new QMenu(this);
	actionImportFromDataFile = toolsMenu->addAction(QIcon(QString::fromUtf8(":/data/img/fileopen.png")), tr("Import from data file"), this, SLOT(importFromDataFile()));
	actionExportToDataFile = toolsMenu->addAction(QIcon(QString::fromUtf8(":/data/img/filesaveas.png")), tr("Export to data file"), this, SLOT(exportToDataFile()));
	actionExportToDataFile->setEnabled(false);
	toolButtonToolsMenu->setMenu(toolsMenu);

	// check-sum DB related
	m_checkSumScannerLog = new CheckSumScannerLog(mode() == QMC2_ROMALYZER_MODE_SYSTEM ? "CheckSumScannerLog" : "SoftwareCheckSumScannerLog" , 0);
	connect(checkSumScannerLog(), SIGNAL(windowOpened()), this, SLOT(checkSumScannerLog_windowOpened()));
	connect(checkSumScannerLog(), SIGNAL(windowClosed()), this, SLOT(checkSumScannerLog_windowClosed()));
	m_checkSumDb = new CheckSumDatabaseManager(this, m_settingsKey);
	connect(checkSumDb(), SIGNAL(log(const QString &)), this, SLOT(log(const QString &)));
	m_checkSumScannerThread = new CheckSumScannerThread(checkSumScannerLog(), m_settingsKey, this);
	connect(checkSumScannerThread(), SIGNAL(log(const QString &)), checkSumScannerLog(), SLOT(log(const QString &)));
	connect(checkSumScannerThread(), SIGNAL(scanStarted()), this, SLOT(checkSumScannerThread_scanStarted()));
	connect(checkSumScannerThread(), SIGNAL(scanFinished()), this, SLOT(checkSumScannerThread_scanFinished()));
	connect(checkSumScannerThread(), SIGNAL(scanPaused()), this, SLOT(checkSumScannerThread_scanPaused()));
	connect(checkSumScannerThread(), SIGNAL(scanResumed()), this, SLOT(checkSumScannerThread_scanResumed()));
	connect(checkSumScannerThread(), SIGNAL(scanResumed()), this, SLOT(checkSumScannerThread_scanResumed()));
	connect(checkSumScannerThread(), SIGNAL(progressTextChanged(const QString &)), checkSumScannerLog(), SLOT(progressTextChanged(const QString &)));
	connect(checkSumScannerThread(), SIGNAL(progressRangeChanged(int, int)), checkSumScannerLog(), SLOT(progressRangeChanged(int, int)));
	connect(checkSumScannerThread(), SIGNAL(progressChanged(int)), checkSumScannerLog(), SLOT(progressChanged(int)));
	connect(&checkSumDbStatusTimer, SIGNAL(timeout()), this, SLOT(updateCheckSumDbStatus()));
	updateCheckSumDbStatus();
	checkSumDbStatusTimer.start(QMC2_CHECKSUM_DB_STATUS_UPDATE_LONG);
	pushButtonCheckSumDbPauseResumeScan->hide();
	connect(&m_checkSumTextChangedTimer, SIGNAL(timeout()), this, SLOT(lineEditChecksumWizardHash_textChanged_delayed()));

	// we create the collection rebuilder on demand
	m_collectionRebuilder = NULL;

	currentFilesSize = 0;
	if ( mode() == QMC2_ROMALYZER_MODE_SOFTWARE ) {
		if ( !qmc2SoftwareList ) {
			QLayout *vbl = qmc2MainWindow->tabSoftwareList->layout();
			if ( vbl )
				delete vbl;
			int left, top, right, bottom;
			qmc2MainWindow->gridLayout->getContentsMargins(&left, &top, &right, &bottom);
			QVBoxLayout *layout = new QVBoxLayout;
			layout->setContentsMargins(left, top, right, bottom);
			qmc2SoftwareList = new SoftwareList("qmc2_romalyzer_dummy", qmc2MainWindow->tabSoftwareList);
			layout->addWidget(qmc2SoftwareList);
			qmc2MainWindow->tabSoftwareList->setLayout(layout);
			qmc2MainWindow->isCreatingSoftList = false;
			setEnabled(false);
			connect(qmc2SoftwareList, SIGNAL(loadFinished(bool)), this, SLOT(softwareListLoadFinished(bool)));
			QTimer::singleShot(0, qmc2SoftwareList, SLOT(load()));
		}
	}

#if defined(QMC2_OS_MAC)
	setParent(qmc2MainWindow, Qt::Dialog);
#endif

	tabWidgetAnalysis->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/" + m_settingsKey + "/AnalysisTab", 0).toInt());
}

ROMAlyzer::~ROMAlyzer()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::~ROMAlyzer()");
#endif

	if ( checkSumDb() )
		delete checkSumDb();
	if ( checkSumScannerLog() )
		delete checkSumScannerLog();
	if ( checkSumScannerThread() )
		delete checkSumScannerThread();
	if ( collectionRebuilder() )
		delete collectionRebuilder();
}

void ROMAlyzer::adjustIconSizes()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::adjustIconSizes()"));
#endif

	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFontMetrics fm(f);
	QSize iconSize = QSize(fm.height() - 2, fm.height() - 2);
	QSize iconSizeTreeWidgets = iconSize + QSize(2, 2);
	iconSizeTreeWidgets.setWidth(iconSizeTreeWidgets.width() * 2);
	pushButtonAnalyze->setIconSize(iconSize);
	pushButtonPause->setIconSize(iconSize);
	pushButtonClose->setIconSize(iconSize);
	pushButtonSearchForward->setIconSize(iconSize);
	pushButtonSearchBackward->setIconSize(iconSize);
	toolButtonSaveLog->setIconSize(iconSize);
	toolButtonBrowseCHDManagerExecutableFile->setIconSize(iconSize);
	toolButtonBrowseTemporaryWorkingDirectory->setIconSize(iconSize);
	toolButtonBrowseSetRewriterOutputPath->setIconSize(iconSize);
	toolButtonBrowseSetRewriterAdditionalRomPath->setIconSize(iconSize);
	toolButtonBrowseBackupFolder->setIconSize(iconSize);
	QTabBar *tabBar = tabWidgetAnalysis->findChild<QTabBar *>();
	if ( tabBar )
		tabBar->setIconSize(iconSize);
	toolButtonToolsMenu->setIconSize(iconSize);
	pushButtonChecksumWizardAnalyzeSelectedSets->setIconSize(iconSize);
	pushButtonChecksumWizardRepairBadSets->setIconSize(iconSize);
	treeWidgetChecksums->setIconSize(iconSizeTreeWidgets);
	treeWidgetChecksumWizardSearchResult->setIconSize(iconSizeTreeWidgets);
	pushButtonChecksumWizardSearch->setIconSize(iconSize);
	toolButtonCheckSumDbAddPath->setIconSize(iconSize);
	toolButtonBrowseCheckSumDbDatabasePath->setIconSize(iconSize);
	toolButtonCheckSumDbRemovePath->setIconSize(iconSize);
	pushButtonCheckSumDbScan->setIconSize(iconSize);
	pushButtonCheckSumDbPauseResumeScan->setIconSize(iconSize);
	widgetCheckSumDbQueryStatus->setFixedWidth(widgetCheckSumDbQueryStatus->height());
	QPalette pal = widgetCheckSumDbQueryStatus->palette();
	pal.setBrush(QPalette::Window, m_checkSumDbQueryStatusPixmap.scaled(widgetCheckSumDbQueryStatus->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
	widgetCheckSumDbQueryStatus->setPalette(pal);
}

void ROMAlyzer::on_pushButtonClose_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_pushButtonClose_clicked()");
#endif

	if ( active() )
		on_pushButtonAnalyze_clicked();
}

void ROMAlyzer::on_pushButtonAnalyze_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_pushButtonAnalyze_clicked()");
#endif

	if ( qmc2ReloadActive ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("please wait for reload to finish and try again"));
		return;
	}

	if ( active() ) {
		// stop ROMAlyzer
		log(tr("stopping analysis"));
		qmc2StopParser = true;
	} else if ( qmc2MachineList->numGames > 0 ) {
		// start ROMAlyzer
		log(tr("starting analysis"));
		qmc2StopParser = false;
		QTimer::singleShot(0, this, SLOT(analyze()));
	}
}

void ROMAlyzer::on_pushButtonPause_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_pushButtonPause_clicked()");
#endif

	setPaused(!paused());
	if ( paused() ) {
		log(tr("pausing analysis"));
		pushButtonPause->setEnabled(false);
	} else {
		log(tr("resuming analysis"));
		pushButtonPause->setText(tr("&Pause"));
	}
}

void ROMAlyzer::on_pushButtonSearchForward_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_pushButtonSearchForward_clicked()");
#endif

	if ( !textBrowserLog->find(lineEditSearchString->text()) ) {
		lineEditSearchString->setEnabled(false);
		QTimer::singleShot(QMC2_ROMALYZER_FLASH_TIME, this, SLOT(enableSearchEdit()));
	}
}

void ROMAlyzer::on_pushButtonSearchBackward_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_pushButtonSearchBackward_clicked()");
#endif

	if ( !textBrowserLog->find(lineEditSearchString->text(), QTextDocument::FindBackward) ) {
		lineEditSearchString->setEnabled(false);
		QTimer::singleShot(QMC2_ROMALYZER_FLASH_TIME, this, SLOT(enableSearchEdit()));
	}
}

void ROMAlyzer::on_lineEditSoftwareLists_textChanged(QString text)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::on_lineEditSoftwareLists_textChanged(QString text = %1)").arg(text));
#endif

	if ( !active() )
		pushButtonAnalyze->setEnabled(!text.isEmpty() && !lineEditSets->text().isEmpty());
}

void ROMAlyzer::on_lineEditSets_textChanged(QString text)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::on_lineEditSets_textChanged(QString text = %1)").arg(text));
#endif

	if ( !active() )
		pushButtonAnalyze->setEnabled(mode() == QMC2_ROMALYZER_MODE_SYSTEM ? !text.isEmpty() : !text.isEmpty() && !lineEditSoftwareLists->text().isEmpty());
}

void ROMAlyzer::on_checkBoxCalculateCRC_toggled(bool enable)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::on_checkBoxCalculateCRC_toggled(bool enable = %1)").arg(enable));
#endif

	treeWidgetChecksums->setColumnHidden(QMC2_ROMALYZER_COLUMN_CRC, !enable);
}

void ROMAlyzer::on_checkBoxCalculateMD5_toggled(bool enable)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::on_checkBoxCalculateMD5_toggled(bool enable = %1)").arg(enable));
#endif

	treeWidgetChecksums->setColumnHidden(QMC2_ROMALYZER_COLUMN_MD5, !enable);
}

void ROMAlyzer::on_checkBoxCalculateSHA1_toggled(bool enable)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::on_checkBoxCalculateSHA1_toggled(bool enable = %1)").arg(enable));
#endif

	treeWidgetChecksums->setColumnHidden(QMC2_ROMALYZER_COLUMN_SHA1, !enable);
	tabChecksumWizard->setEnabled(enable);
}

void ROMAlyzer::animationTimeout()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::animationTimeout()");
#endif

	switch ( ++animSeq ) {
		case 0:
			pushButtonAnalyze->setIcon(QIcon(QString::fromUtf8(":/data/img/romalyzer.png")));
			break;

		case 2:
			pushButtonAnalyze->setIcon(QIcon(QString::fromUtf8(":/data/img/romalyzer_flipped.png")));
			break;

		default:
			break;
	}
	if ( animSeq > 2 )
		animSeq = -1;
}

void ROMAlyzer::closeEvent(QCloseEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::closeEvent(QCloseEvent *e = %1)").arg((qulonglong)e));
#endif

	if ( e && active() )
		on_pushButtonAnalyze_clicked();

	// save settings
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/AppendReport", checkBoxAppendReport->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/ExpandFiles", checkBoxExpandFiles->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/ExpandChecksums", checkBoxExpandChecksums->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/AutoScroll", checkBoxAutoScroll->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/CalculateCRC", checkBoxCalculateCRC->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/CalculateSHA1", checkBoxCalculateSHA1->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/CalculateMD5", checkBoxCalculateMD5->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SelectGame", checkBoxSelectGame->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/MaxFileSize", spinBoxMaxFileSize->value());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/MaxLogSize", spinBoxMaxLogSize->value());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/MaxReports", spinBoxMaxReports->value());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/CreateBackups", checkBoxCreateBackups->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/BackupFolder", lineEditBackupFolder->text());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/EnableCHDManager", groupBoxCHDManager->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/CHDManagerExecutableFile", lineEditCHDManagerExecutableFile->text());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/TemporaryWorkingDirectory", lineEditTemporaryWorkingDirectory->text());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/VerifyCHDs", checkBoxVerifyCHDs->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/UpdateCHDs", checkBoxUpdateCHDs->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/EnableSetRewriter", groupBoxSetRewriter->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetRewriterWhileAnalyzing", checkBoxSetRewriterWhileAnalyzing->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetRewriterSelfContainedSets", checkBoxSetRewriterSelfContainedSets->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetRewriterGoodDumpsOnly", checkBoxSetRewriterGoodDumpsOnly->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetRewriterAbortOnError", checkBoxSetRewriterAbortOnError->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetRewriterZipArchives", radioButtonSetRewriterZipArchives->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetRewriterZipLevel", spinBoxSetRewriterZipLevel->value());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetRewriterUniqueCRCs", checkBoxSetRewriterUniqueCRCs->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetRewriterAddZipComment", checkBoxAddZipComment->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetRewriterIndividualDirectories", radioButtonSetRewriterIndividualDirectories->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetRewriterOutputPath", lineEditSetRewriterOutputPath->text());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetRewriterUseAdditionalRomPath", checkBoxSetRewriterUseAdditionalRomPath->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetRewriterAdditionalRomPath", lineEditSetRewriterAdditionalRomPath->text());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/ChecksumWizardHashType", comboBoxChecksumWizardHashType->currentIndex());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/ChecksumWizardAutomationLevel", comboBoxChecksumWizardAutomationLevel->currentIndex());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/EnableCheckSumDb", groupBoxCheckSumDatabase->isChecked());
	QStringList checkSumDbScannedPaths;
	QStringList checkSumDbScannedPathsEnabled;
	for (int i = 0; i < listWidgetCheckSumDbScannedPaths->count(); i++) {
		checkSumDbScannedPaths << listWidgetCheckSumDbScannedPaths->item(i)->text();
		checkSumDbScannedPathsEnabled << (listWidgetCheckSumDbScannedPaths->item(i)->checkState() == Qt::Checked ? "true" : "false");
	}
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/CheckSumDbScannedPaths", checkSumDbScannedPaths);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/CheckSumDbScannedPathsEnabled", checkSumDbScannedPathsEnabled);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/CheckSumDbDatabasePath", lineEditCheckSumDbDatabasePath->text());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/CheckSumDbScanIncrementally", toolButtonCheckSumDbScanIncrementally->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/CheckSumDbDeepScan", toolButtonCheckSumDbDeepScan->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/" + m_settingsKey + "/ReportHeaderState", treeWidgetChecksums->header()->saveState());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/" + m_settingsKey + "/AnalysisTab", tabWidgetAnalysis->currentIndex());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/" + m_settingsKey + "/ChecksumWizardHeaderState", treeWidgetChecksumWizardSearchResult->header()->saveState());
	if ( !qmc2CleaningUp ) {
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/" + m_settingsKey + "/Position", pos());
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/" + m_settingsKey + "/Size", size());
	}
	if ( checkSumScannerLog() )
		checkSumScannerLog()->close();
	if ( e )
		e->accept();
}

void ROMAlyzer::hideEvent(QHideEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::hideEvent(QHideEvent *e = %1)").arg((qulonglong)e));
#endif

	closeEvent(NULL);

	if ( e )
		e->accept();
}

void ROMAlyzer::showEvent(QShowEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::showEvent(QShowEvent *e = %1)").arg((qulonglong)e));
#endif

	static bool initialCall = true;

	QString userScopePath = QMC2_DYNAMIC_DOT_PATH;
	QString variantName = QMC2_VARIANT_NAME.toLower().replace(QRegExp("\\..*$"), "");

	// restore settings
	checkBoxAppendReport->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/AppendReport", false).toBool());
	checkBoxExpandFiles->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/ExpandFiles", false).toBool());
	checkBoxExpandChecksums->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/ExpandChecksums", false).toBool());
	checkBoxAutoScroll->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/AutoScroll", true).toBool());
	checkBoxCalculateCRC->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/CalculateCRC", true).toBool());
	checkBoxCalculateSHA1->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/CalculateSHA1", true).toBool());
	checkBoxCalculateMD5->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/CalculateMD5", false).toBool());
	checkBoxSelectGame->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SelectGame", true).toBool());
	spinBoxMaxFileSize->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/MaxFileSize", 0).toInt());
	spinBoxMaxLogSize->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/MaxLogSize", 10000).toInt());
	spinBoxMaxReports->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/MaxReports", 1000).toInt());
	checkBoxCreateBackups->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/CreateBackups", false).toBool());
	lineEditBackupFolder->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/BackupFolder", QString()).toString());
	groupBoxCHDManager->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/EnableCHDManager", false).toBool());
	checkBoxVerifyCHDs->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/VerifyCHDs", true).toBool());
	checkBoxUpdateCHDs->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/UpdateCHDs", false).toBool());
	lineEditCHDManagerExecutableFile->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/CHDManagerExecutableFile", QString()).toString());
	lineEditTemporaryWorkingDirectory->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/TemporaryWorkingDirectory", QString()).toString());
	lineEditSetRewriterOutputPath->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetRewriterOutputPath", QString()).toString());
	checkBoxSetRewriterUseAdditionalRomPath->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetRewriterUseAdditionalRomPath", false).toBool());
	lineEditSetRewriterAdditionalRomPath->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetRewriterAdditionalRomPath", QString()).toString());
	groupBoxSetRewriter->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/EnableSetRewriter", false).toBool());
	checkBoxSetRewriterWhileAnalyzing->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetRewriterWhileAnalyzing", false).toBool());
	checkBoxSetRewriterSelfContainedSets->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetRewriterSelfContainedSets", false).toBool());
	checkBoxSetRewriterGoodDumpsOnly->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetRewriterGoodDumpsOnly", true).toBool());
	checkBoxSetRewriterAbortOnError->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetRewriterAbortOnError", true).toBool());
	radioButtonSetRewriterZipArchives->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetRewriterZipArchives", true).toBool());
	spinBoxSetRewriterZipLevel->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetRewriterZipLevel", Z_DEFAULT_COMPRESSION).toInt());
	checkBoxSetRewriterUniqueCRCs->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetRewriterUniqueCRCs", false).toBool());
	checkBoxAddZipComment->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetRewriterAddZipComment", true).toBool());
	comboBoxChecksumWizardHashType->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/ChecksumWizardHashType", QMC2_ROMALYZER_CSWIZ_HASHTYPE_SHA1).toInt());
	comboBoxChecksumWizardAutomationLevel->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/ChecksumWizardAutomationLevel", QMC2_ROMALYZER_CSWIZ_AMLVL_NONE).toInt());
	radioButtonSetRewriterIndividualDirectories->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetRewriterIndividualDirectories", false).toBool());
	groupBoxCheckSumDatabase->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/EnableCheckSumDb", false).toBool());
	on_groupBoxCheckSumDatabase_toggled(groupBoxCheckSumDatabase->isChecked());
	if ( initialCall || !checkSumScannerThread() )
		tabWidgetAnalysis->setTabEnabled(QMC2_ROMALYZER_PAGE_RCR, groupBoxCheckSumDatabase->isChecked() && groupBoxSetRewriter->isChecked());
	else
		tabWidgetAnalysis->setTabEnabled(QMC2_ROMALYZER_PAGE_RCR, groupBoxCheckSumDatabase->isChecked() && groupBoxSetRewriter->isChecked() && !checkSumScannerThread()->isActive);
	listWidgetCheckSumDbScannedPaths->clear();
	QStringList checkSumDbScannedPaths = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/CheckSumDbScannedPaths", QStringList()).toStringList();
	QStringList checkSumDbScannedPathsEnabled = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/CheckSumDbScannedPathsEnabled", QStringList()).toStringList();
	for (int i = 0; i < checkSumDbScannedPaths.count(); i++) {
		QListWidgetItem *item = new QListWidgetItem(checkSumDbScannedPaths[i]);
		if ( i < checkSumDbScannedPathsEnabled.count() )
			item->setCheckState(checkSumDbScannedPathsEnabled[i] == "true" ? Qt::Checked : Qt::Unchecked);
		else
			item->setCheckState(Qt::Checked);
		listWidgetCheckSumDbScannedPaths->addItem(item);
	}
	pushButtonCheckSumDbScan->setEnabled(listWidgetCheckSumDbScannedPaths->count() > 0);
	switch ( mode() ) {
		case QMC2_ROMALYZER_MODE_SOFTWARE:
			lineEditCheckSumDbDatabasePath->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/CheckSumDbDatabasePath", QString(userScopePath + "/%1-software-checksum.db").arg(variantName)).toString());
			break;
		case QMC2_ROMALYZER_MODE_SYSTEM:
		default:
			lineEditCheckSumDbDatabasePath->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/CheckSumDbDatabasePath", QString(userScopePath + "/%1-checksum.db").arg(variantName)).toString());
			break;
	}
	lineEditCheckSumDbDatabasePath->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/CheckSumDbDatabasePath", QString(userScopePath + "/%1-checksum.db").arg(variantName)).toString());
	toolButtonCheckSumDbScanIncrementally->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/CheckSumDbScanIncrementally", true).toBool());
	toolButtonCheckSumDbDeepScan->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/CheckSumDbDeepScan", true).toBool());

	if ( e )
		e->accept();

	initialCall = false;
}

void ROMAlyzer::on_spinBoxMaxLogSize_valueChanged(int value)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::on_spinBoxMaxLogSize_valueChanged(int value = %1)").arg(value));
#endif

	textBrowserLog->setMaximumBlockCount(spinBoxMaxLogSize->value());
}

void ROMAlyzer::moveEvent(QMoveEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::moveEvent(QMoveEvent *e = %1)").arg((qulonglong)e));
#endif

	if ( !qmc2CleaningUp && !qmc2EarlyStartup )
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/" + m_settingsKey + "/Position", pos());

	if ( e )
		e->accept();
}

void ROMAlyzer::resizeEvent(QResizeEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::resizeEvent(QResizeEvent *e = %1)").arg((qulonglong)e));
#endif

	if ( !qmc2CleaningUp && !qmc2EarlyStartup )
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/" + m_settingsKey +"/Size", size());

	if ( e )
		e->accept();
}

void ROMAlyzer::analyze()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::analyze()");
#endif

	setActive(true);

	QString myRomPath;
	if ( qmc2Config->contains(QMC2_EMULATOR_PREFIX + "Configuration/Global/rompath") )
		myRomPath = qmc2Config->value(QMC2_EMULATOR_PREFIX + "Configuration/Global/rompath").toString();
	else
		myRomPath = "roms";

	if ( groupBoxSetRewriter->isChecked() && checkBoxSetRewriterUseAdditionalRomPath->isChecked() && !lineEditSetRewriterAdditionalRomPath->text().isEmpty() )
		myRomPath.prepend(lineEditSetRewriterAdditionalRomPath->text() + ";");

	myRomPath.replace("~", QDir::homePath());
	myRomPath.replace("$HOME", QDir::homePath());
	romPaths = myRomPath.split(";", QString::SkipEmptyParts);

	QStringList analyzerList;
	QStringList softwareListPatternList = lineEditSoftwareLists->text().simplified().split(" ", QString::SkipEmptyParts);
	QStringList setPatternList = lineEditSets->text().simplified().split(" ", QString::SkipEmptyParts);

	if ( !checkBoxAppendReport->isChecked() ) {
		treeWidgetChecksums->clear();
		textBrowserLog->clear();
		analyzerBadSets.clear();
	}

	if ( mode() == QMC2_ROMALYZER_MODE_SOFTWARE && qmc2SoftwareList ) {
		qmc2SoftwareList->analyzeMenuAction->setEnabled(false);
		qmc2SoftwareList->actionAnalyzeSoftware->setEnabled(false);
		qmc2SoftwareList->actionAnalyzeSoftwareList->setEnabled(false);
		qmc2SoftwareList->actionAnalyzeSoftwareLists->setEnabled(false);
		qmc2SoftwareList->toolButtonAnalyzeSoftware->setEnabled(false);
	}

	setPaused(false);
	animSeq = -1;
	animationTimeout();
	animTimer.start(QMC2_ANIMATION_TIMEOUT);
	pushButtonAnalyze->setText(tr("&Stop"));
	pushButtonPause->setVisible(true);
	pushButtonPause->setEnabled(true);
	pushButtonPause->setText(tr("&Pause"));
	lineEditSoftwareLists->setEnabled(false);
	lineEditSets->setEnabled(false);
	toolButtonToolsMenu->setEnabled(false);
	if ( checkBoxCalculateSHA1->isChecked() )
		tabChecksumWizard->setEnabled(false);
	QTime analysisTimer, elapsedTime(0, 0, 0, 0);
	analysisTimer.start();
	log(tr("analysis started"));
	log(tr("determining list of sets to analyze"));

	int i = 0;
	QRegExp wildcardRx("(\\*|\\?)");
	switch ( mode() ) {
		case QMC2_ROMALYZER_MODE_SOFTWARE:
			if ( wizardSearch || quickSearch || (wildcardRx.indexIn(lineEditSets->text().simplified()) == -1 && wildcardRx.indexIn(lineEditSoftwareLists->text().simplified()) == -1 )) {
				// no wild-cards => no need to search!
				if ( wizardSearch ) {
					for (int j = 0; j < softwareListPatternList.count(); j++) {
						QString list = softwareListPatternList[j];
						QString set = setPatternList[j];
						if ( qmc2MainWindow->swlDb->exists(list, set) )
							analyzerList << list + ":" + set;
					}
				} else {
					foreach (QString list, softwareListPatternList)
						foreach (QString set, setPatternList)
							if ( qmc2MainWindow->swlDb->exists(list, set) )
								analyzerList << list + ":" + set;
				}
			} else {
				if ( softwareListPatternList.count() == 1 && setPatternList.count() == 1 ) {
					// special case for exactly ONE matching softlist + set -- no need to search
					if ( qmc2MainWindow->swlDb->exists(softwareListPatternList.first(), setPatternList.first()) )
						analyzerList << softwareListPatternList.first() + ":" + setPatternList.first();
				}
				if ( analyzerList.empty() ) {
					// determine list of softlists + sets to analyze
					QStringList uniqueSoftwareLists = qmc2MainWindow->swlDb->uniqueSoftwareLists();
					if ( !uniqueSoftwareLists.isEmpty() ) {
						labelStatus->setText(tr("Searching sets"));
						i = 0;
						bool matchAllSoftwareLists = (lineEditSoftwareLists->text().simplified() == "*");
						bool matchAllSets = (lineEditSets->text().simplified() == "*");
						progressBar->setRange(0, uniqueSoftwareLists.count());
						progressBar->reset();
						foreach (QString softList, uniqueSoftwareLists) {
							progressBar->setValue(++i);
							bool swlMatched = matchAllSoftwareLists;
							if ( !swlMatched ) {
								foreach (QString pattern, softwareListPatternList) {
									QRegExp regexp(pattern, Qt::CaseSensitive, QRegExp::Wildcard);
									if ( regexp.exactMatch(softList) ) {
										swlMatched = true;
										break;
									}
								}
							}
							if ( swlMatched ) {
								QStringList uniqueSoftwareSets = qmc2MainWindow->swlDb->uniqueSoftwareSets(softList);
								foreach (QString setName, uniqueSoftwareSets) {
									QString softwareKey = softList + ":" + setName;
									if ( matchAllSets )
										analyzerList << softwareKey;
									else foreach (QString pattern, setPatternList) {
										QRegExp regexp(pattern, Qt::CaseSensitive, QRegExp::Wildcard);
										if ( regexp.exactMatch(setName) )
											analyzerList << softwareKey;
									}
								}
							}
							qApp->processEvents();
						}
						progressBar->reset();
						labelStatus->setText(tr("Idle"));
					}
				}
			}
			break;
		case QMC2_ROMALYZER_MODE_SYSTEM:
		default:
			if ( wizardSearch || quickSearch || wildcardRx.indexIn(lineEditSets->text().simplified()) == -1 ) {
				// no wild-cards => no need to search!
				foreach (QString id, setPatternList)
					if ( qmc2MachineList->xmlDb()->exists(id) )
						analyzerList << id;
			} else {
				if ( setPatternList.count() == 1 ) {
					// special case for exactly ONE matching set -- no need to search
					if ( qmc2MachineListItemHash.contains(setPatternList[0]) )
						analyzerList << setPatternList[0];
				}
				if ( analyzerList.empty() ) {
					// determine list of sets to analyze
					labelStatus->setText(tr("Searching sets"));
					progressBar->setRange(0, qmc2MachineList->numGames);
					progressBar->reset();
					QHashIterator<QString, QTreeWidgetItem *> it(qmc2MachineListItemHash);
					i = 0;
					bool matchAll = (lineEditSets->text().simplified() == "*");
					while ( it.hasNext() && !qmc2StopParser ) {
						it.next();
						progressBar->setValue(++i);
						QString gameID = it.key();
						if ( matchAll )
							analyzerList << gameID;
						else foreach (QString pattern, setPatternList) {
							QRegExp regexp(pattern, Qt::CaseSensitive, QRegExp::Wildcard);
							if ( regexp.exactMatch(gameID) )
								if ( !analyzerList.contains(gameID) )
									analyzerList << gameID;
						}
						if ( i % QMC2_ROMALYZER_SEARCH_RESPONSE == 0 )
							qApp->processEvents();
					}
					progressBar->reset();
					labelStatus->setText(tr("Idle"));
				}
			}
			break;
	}

	analyzerList.sort();
	quickSearch = false;

	if ( !qmc2StopParser ) {
		log(tr("done (determining list of sets to analyze)"));
		log(tr("%n set(s) to analyze", "", analyzerList.count()));
		i = 0;
		int setsInMemory = 0;
		foreach (QString setKey, analyzerList) {
			QString gameName, softListName;
			QStringList setKeyTokens;
			switch ( mode() ) {
				case QMC2_ROMALYZER_MODE_SOFTWARE:
					setKeyTokens = setKey.split(":", QString::SkipEmptyParts);
					if ( setKeyTokens.count() < 2 )
						continue;
					softListName = setKeyTokens[0];
					gameName = setKeyTokens[1];
					break;
				case QMC2_ROMALYZER_MODE_SYSTEM:
				default:
					gameName = setKey;
					break;
			}
			// wait if paused...
			for (quint64 waitCounter = 0; paused() && !qmc2StopParser; waitCounter++) {
				if ( waitCounter == 0 ) {
					log(tr("analysis paused"));
					pushButtonPause->setText(tr("&Resume"));
					pushButtonPause->setEnabled(true);
					progressBar->reset();
					labelStatus->setText(tr("Paused"));
				}
				QTest::qWait(QMC2_ROMALYZER_PAUSE_TIMEOUT);
			}

			bool filesSkipped = false;
			bool filesUnknown = false;
			bool filesError = false;

			if ( qmc2StopParser )
				break;

			// remove the 'oldest' sets from the report if the report limit has been reached
			int reportLimit = spinBoxMaxReports->value();
			if ( reportLimit > 0 ) {
				if ( setsInMemory >= reportLimit ) {
					int setsToBeRemoved = setsInMemory - reportLimit + 1;
					log(tr("report limit reached, removing %n set(s) from the report", "", setsToBeRemoved));
					for (int j = 0; j < setsToBeRemoved; j++) {
						QTreeWidgetItem *ti = treeWidgetChecksums->topLevelItem(0);
						if ( ti ) {
							analyzerBadSets.removeAll(ti->text(QMC2_ROMALYZER_COLUMN_GAME).split(" ", QString::SkipEmptyParts)[0]);
							if ( ti->isSelected() )
								treeWidgetChecksums->selectionModel()->clear();
							delete treeWidgetChecksums->takeTopLevelItem(0);
							setsInMemory--;
						}
					}
				}
			}
			setsInMemory++;

			QLocale locale;

			// analyze set
			log(tr("analyzing '%1'").arg(setKey));
			setRewriterSetCount = analyzerList.count() - i;
			labelStatus->setText(tr("Analyzing '%1'").arg(setKey) + QString(" - %1").arg(locale.toString(setRewriterSetCount)));

			// step 1: retrieve XML data, insert item with game name
			QTreeWidgetItem *item = new QTreeWidgetItem(treeWidgetChecksums);
			item->setText(QMC2_ROMALYZER_COLUMN_GAME, setKey);
			QString xmlBuffer;
			switch ( mode() ) {
				case QMC2_ROMALYZER_MODE_SOFTWARE:
					xmlBuffer = getSoftwareXmlData(softListName, gameName);
					break;
				case QMC2_ROMALYZER_MODE_SYSTEM:
				default:
					xmlBuffer = getXmlData(gameName);
					break;
			}

			if ( qmc2StopParser )
				break;

			// step 2: parse XML data, insert ROMs / CHDs and check-sums as they *should* be
			log(tr("parsing XML data for '%1'").arg(setKey));
			QXmlInputSource xmlInputSource;
			xmlInputSource.setData(xmlBuffer);
			ROMAlyzerXmlHandler xmlHandler(item, checkBoxExpandFiles->isChecked(), checkBoxAutoScroll->isChecked(), mode());
			QXmlSimpleReader xmlReader;
			xmlReader.setContentHandler(&xmlHandler);
			if ( xmlReader.parse(xmlInputSource) )
				log(tr("done (parsing XML data for '%1')").arg(setKey));
			else
				log(tr("error (parsing XML data for '%1')").arg(setKey));

			if ( qmc2StopParser )
				break;

			if ( !xmlHandler.deviceReferences.isEmpty() )
				item->setWhatsThis(QMC2_ROMALYZER_COLUMN_GAME, xmlHandler.deviceReferences.join(","));

			int numWizardFiles = 1;
			if ( wizardSearch )
				numWizardFiles = treeWidgetChecksumWizardSearchResult->findItems(setKey, Qt::MatchExactly, QMC2_ROMALYZER_CSWIZ_COLUMN_ID).count();

			// step 3: check file status of ROMs and CHDs, recalculate check-sums
			log(tr("checking %n file(s) for '%1'", "", wizardSearch ? numWizardFiles : xmlHandler.fileCounter).arg(setKey));
			progressBar->reset();
			progressBar->setRange(0, xmlHandler.fileCounter);
			int fileCounter;
			int notFoundCounter = 0;
			int noDumpCounter = 0;
			bool gameOkay = true;
			int mergeStatus = QMC2_ROMALYZER_MERGE_STATUS_OK;

			setRewriterFileMap.clear();
			setRewriterSetName = setKey;
			setRewriterItem = item;

			for (fileCounter = 0; fileCounter < xmlHandler.fileCounter && !qmc2StopParser; fileCounter++) {
				progressBar->setValue(fileCounter + 1);
				progressBar->setFormat(QString("%1 / %2").arg(fileCounter + 1).arg(xmlHandler.fileCounter));
				qApp->processEvents();
				QByteArray data;
				bool sevenZipped = false;
				bool zipped = false;
				bool merged = false;
				bool fromCheckSumDb = false;
				QTreeWidgetItem *childItem = xmlHandler.childItems.at(fileCounter);
				QTreeWidgetItem *parentItem = xmlHandler.parentItem;
				QString sha1Calculated, md5Calculated, fallbackPath;
				bool optionalRom = xmlHandler.optionalROMs.contains(childItem->text(QMC2_ROMALYZER_COLUMN_CRC));

				if ( wizardSearch ) {
					QList<QTreeWidgetItem *> il = treeWidgetChecksumWizardSearchResult->findItems(setKey, Qt::MatchExactly, QMC2_ROMALYZER_CSWIZ_COLUMN_ID);
					bool itemFound = false;
					foreach (QTreeWidgetItem *it, il)
						if ( it->text(QMC2_ROMALYZER_CSWIZ_COLUMN_FILENAME) == childItem->text(QMC2_ROMALYZER_COLUMN_GAME) ) {
							itemFound = true;
							break;
						}
					if ( !itemFound ) {
						childItem->setText(QMC2_ROMALYZER_COLUMN_FILESTATUS, tr("skipped"));
						childItem->setForeground(QMC2_ROMALYZER_COLUMN_FILESTATUS, xmlHandler.blueBrush);
						continue;
					}
				}

				QString effectiveFile = getEffectiveFile(childItem, softListName, gameName, childItem->text(QMC2_ROMALYZER_COLUMN_GAME), childItem->text(QMC2_ROMALYZER_COLUMN_CRC),
									 parentItem->text(QMC2_ROMALYZER_COLUMN_MERGE), childItem->text(QMC2_ROMALYZER_COLUMN_MERGE), childItem->text(QMC2_ROMALYZER_COLUMN_TYPE),
									 &data, &sha1Calculated, &md5Calculated, &zipped, &sevenZipped, &merged, fileCounter, &fallbackPath, optionalRom, &fromCheckSumDb); 

				if ( qmc2StopParser )
					continue;

				if ( effectiveFile.isEmpty() ) {
					childItem->setText(QMC2_ROMALYZER_COLUMN_FILESTATUS, tr("not found"));
					childItem->setForeground(QMC2_ROMALYZER_COLUMN_FILESTATUS, xmlHandler.greyBrush);
					if ( xmlHandler.optionalROMs.contains(childItem->text(QMC2_ROMALYZER_COLUMN_CRC)) )
						childItem->setIcon(QMC2_ROMALYZER_COLUMN_GAME, QIcon(QIcon(QString::fromUtf8(":/data/img/remove.png")).pixmap(QSize(64, 64), QIcon::Disabled)));
					else
						childItem->setIcon(QMC2_ROMALYZER_COLUMN_GAME, QIcon(QString::fromUtf8(":/data/img/remove.png")));
					notFoundCounter++;
				} else {
					QString fileStatus;
					bool somethingsWrong = false;
					bool goodDump = false;
					bool isCHD = childItem->text(QMC2_ROMALYZER_COLUMN_TYPE).split(" ", QString::SkipEmptyParts)[0] == QObject::tr("CHD");
					bool isROM = childItem->text(QMC2_ROMALYZER_COLUMN_TYPE).startsWith(tr("ROM"));
					bool hasDump = childItem->text(QMC2_ROMALYZER_COLUMN_EMUSTATUS) != QObject::tr("no dump");
					QTreeWidgetItem *fileItem = NULL;

					if ( effectiveFile != QMC2_ROMALYZER_FILE_NOT_FOUND ) {
						QIcon icon;
						if ( isROM )
							icon = QIcon(QString::fromUtf8(":/data/img/rom.png"));
						else if ( isCHD )
							icon = QIcon(QString::fromUtf8(":/data/img/disk2.png"));
						else if ( hasDump )
							icon = QIcon(QString::fromUtf8(":/data/img/fileopen.png"));
						else
							icon = QIcon(QString::fromUtf8(":/data/img/wip.png"));
						if ( fromCheckSumDb ) {
							QPainter p;
							QPixmap pm(128, 64);
							QPixmap pmIcon = icon.pixmap(64, 64);
							QPixmap pmDb = QIcon(QString::fromUtf8(":/data/img/database.png")).pixmap(64, 64);
							pm.fill(Qt::transparent);
							p.begin(&pm);
							p.setBackgroundMode(Qt::TransparentMode);
							p.drawPixmap(0, 0, pmIcon);
							p.drawPixmap(64, 0, pmDb);
							p.end();
							icon = QIcon(pm);
						}
						childItem->setIcon(QMC2_ROMALYZER_COLUMN_GAME, icon);
					}

					if ( effectiveFile == QMC2_ROMALYZER_FILE_TOO_BIG ) {
						fileStatus = tr("skipped");
						filesSkipped = true;
					} else if ( effectiveFile == QMC2_ROMALYZER_FILE_NOT_SUPPORTED ) {
						fileStatus = tr("skipped");
						filesUnknown = true;
					} else if ( effectiveFile == QMC2_ROMALYZER_FILE_ERROR ) {
						fileStatus = tr("error");
						childItem->setIcon(QMC2_ROMALYZER_COLUMN_GAME, QIcon(QString::fromUtf8(":/data/img/warning.png")));
						if ( mode() == QMC2_ROMALYZER_MODE_SYSTEM ) {
							QString mergeName = childItem->text(QMC2_ROMALYZER_COLUMN_MERGE);
							if ( !mergeName.isEmpty() ) {
								if ( mergeStatus < QMC2_ROMALYZER_MERGE_STATUS_CRIT )
									mergeStatus = QMC2_ROMALYZER_MERGE_STATUS_CRIT;
								childItem->setIcon(QMC2_ROMALYZER_COLUMN_MERGE, QIcon(QString::fromUtf8(":/data/img/merge_nok.png")));
							}
						}
						filesError = true;
						if ( wizardSelectedSets.contains(setKey) ) {
							QList<QTreeWidgetItem *> il = treeWidgetChecksumWizardSearchResult->findItems(setKey, Qt::MatchExactly, QMC2_ROMALYZER_CSWIZ_COLUMN_ID);
							foreach (QTreeWidgetItem *item, il)
								if ( item->text(QMC2_ROMALYZER_CSWIZ_COLUMN_FILENAME) == childItem->text(QMC2_ROMALYZER_COLUMN_GAME) ||
										item->whatsThis(QMC2_ROMALYZER_CSWIZ_COLUMN_FILENAME) == childItem->text(QMC2_ROMALYZER_COLUMN_CRC) ) {
									item->setText(QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS, tr("bad"));
									item->setForeground(QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS, xmlHandler.redBrush);
									item->setText(QMC2_ROMALYZER_CSWIZ_COLUMN_PATH, fallbackPath);
								}
							on_treeWidgetChecksumWizardSearchResult_itemSelectionChanged();
						}
					} else if ( effectiveFile == QMC2_ROMALYZER_FILE_NOT_FOUND || effectiveFile == QMC2_ROMALYZER_NO_DUMP ) {
						if ( hasDump ) {
							fileStatus = tr("not found");
							if ( xmlHandler.optionalROMs.contains(childItem->text(QMC2_ROMALYZER_COLUMN_CRC)) )
								childItem->setIcon(QMC2_ROMALYZER_COLUMN_GAME, QIcon(QIcon(QString::fromUtf8(":/data/img/remove.png")).pixmap(QSize(64, 64), QIcon::Disabled)));
							else
								childItem->setIcon(QMC2_ROMALYZER_COLUMN_GAME, QIcon(QString::fromUtf8(":/data/img/remove.png")));
							notFoundCounter++;
							if ( mode() == QMC2_ROMALYZER_MODE_SYSTEM ) {
								QString mergeName = childItem->text(QMC2_ROMALYZER_COLUMN_MERGE);
								if ( !mergeName.isEmpty() ) {
									if ( mergeStatus < QMC2_ROMALYZER_MERGE_STATUS_CRIT )
										mergeStatus = QMC2_ROMALYZER_MERGE_STATUS_CRIT;
									childItem->setIcon(QMC2_ROMALYZER_COLUMN_MERGE, QIcon(QString::fromUtf8(":/data/img/merge_nok.png")));
								}
							}
							if ( wizardSelectedSets.contains(setKey) ) {
								QList<QTreeWidgetItem *> il = treeWidgetChecksumWizardSearchResult->findItems(setKey, Qt::MatchExactly, QMC2_ROMALYZER_CSWIZ_COLUMN_ID);
								foreach (QTreeWidgetItem *item, il) {
									if ( item->text(QMC2_ROMALYZER_CSWIZ_COLUMN_FILENAME) == childItem->text(QMC2_ROMALYZER_COLUMN_GAME) || item->whatsThis(QMC2_ROMALYZER_CSWIZ_COLUMN_FILENAME) == childItem->text(QMC2_ROMALYZER_COLUMN_CRC) ) {
										item->setText(QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS, tr("bad"));
										item->setForeground(QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS, xmlHandler.redBrush);
										item->setText(QMC2_ROMALYZER_CSWIZ_COLUMN_PATH, fallbackPath);
									}
								}
								on_treeWidgetChecksumWizardSearchResult_itemSelectionChanged();
							}
						} else {
							fileStatus = tr("no dump");
							childItem->setIcon(QMC2_ROMALYZER_COLUMN_GAME, QIcon(QString::fromUtf8(":/data/img/wip.png")));
							noDumpCounter++;
						}
					} else {
						fileItem = new QTreeWidgetItem(childItem);
						fileItem->setIcon(QMC2_ROMALYZER_COLUMN_GAME, QIcon(QString::fromUtf8(":/data/img/hash.png")));
						fileItem->setText(QMC2_ROMALYZER_COLUMN_GAME, tr("Calculated check-sums"));
						childItem->setExpanded(false);
						if ( mode() == QMC2_ROMALYZER_MODE_SYSTEM ) {
							QString mergeName = childItem->text(QMC2_ROMALYZER_COLUMN_MERGE);
							if ( !mergeName.isEmpty() ) {
								if ( !parentItem->text(QMC2_ROMALYZER_COLUMN_MERGE).isEmpty() ) {
									if ( !merged ) {
										log(tr("WARNING: %1 file '%2' loaded from '%3' may be obsolete, should be merged from parent set '%4'").arg(isCHD ? tr("CHD") : tr("ROM")).arg(childItem->text(QMC2_ROMALYZER_COLUMN_GAME)).arg(effectiveFile).arg(parentItem->text(QMC2_ROMALYZER_COLUMN_MERGE)));
										childItem->setIcon(QMC2_ROMALYZER_COLUMN_MERGE, QIcon(QString::fromUtf8(":/data/img/merge.png")));
										if ( mergeStatus < QMC2_ROMALYZER_MERGE_STATUS_WARN )
											mergeStatus = QMC2_ROMALYZER_MERGE_STATUS_WARN;
									} else
										childItem->setIcon(QMC2_ROMALYZER_COLUMN_MERGE, QIcon(QString::fromUtf8(":/data/img/merge_ok.png")));
								} else {
									// this is actually an XML bug in the driver, inform via log and ignore...
									log(tr("INFORMATION: %1 file '%2' has a named merge ('%3') but no parent set -- ignored, but should be reported to the MAME developers as a possible XML bug of the respective driver").arg(isCHD ? tr("CHD") : tr("ROM")).arg(childItem->text(QMC2_ROMALYZER_COLUMN_GAME)).arg(mergeName));
									childItem->setIcon(QMC2_ROMALYZER_COLUMN_MERGE, QIcon(QString::fromUtf8(":/data/img/merge_ok.png")));
								}
							}
						}

						// Size
						QString sizeStr = childItem->text(QMC2_ROMALYZER_COLUMN_SIZE);
						if ( !sizeStr.isEmpty() ){
							QString sizeItemStr;
							if ( fromCheckSumDb )
								sizeItemStr = sizeStr;
							else
								sizeItemStr = QString::number(data.size());
							fileItem->setText(QMC2_ROMALYZER_COLUMN_SIZE, sizeItemStr);
							if ( !fileStatus.isEmpty() )
								fileStatus += " ";
							if ( sizeItemStr == sizeStr )
								fileStatus += tr("SIZE");
							else {
								fileStatus += tr("size");
								if ( hasDump ) {
									somethingsWrong = true;
									fileItem->setForeground(QMC2_ROMALYZER_COLUMN_SIZE, xmlHandler.redBrush);
								}
							}
							qApp->processEvents();
						}

						// CRC
						QString crcStr = childItem->text(QMC2_ROMALYZER_COLUMN_CRC);
						if ( !crcStr.isEmpty() && checkBoxCalculateCRC->isChecked() ) {
							QString crcItemStr;
							if ( fromCheckSumDb )
								crcItemStr = crcStr;
							else {
								ulong crc = crc32(0, NULL, 0);
								crc = crc32(crc, (const Bytef *)data.data(), data.size());
								crcItemStr = crcToString(crc);
							}
							fileItem->setText(QMC2_ROMALYZER_COLUMN_CRC, crcItemStr);
							if ( !fileStatus.isEmpty() )
								fileStatus += " ";
							if ( crcItemStr == crcStr )
								fileStatus += tr("CRC");
							else {
								fileStatus += tr("crc");
							        if ( hasDump ) {
									somethingsWrong = true;
									fileItem->setForeground(QMC2_ROMALYZER_COLUMN_CRC, xmlHandler.redBrush);
								}
							}
							qApp->processEvents();
						}

						// SHA1
						QString sha1Str = childItem->text(QMC2_ROMALYZER_COLUMN_SHA1);
						if ( !sha1Str.isEmpty() && checkBoxCalculateSHA1->isChecked() ) {
							fileItem->setText(QMC2_ROMALYZER_COLUMN_SHA1, sha1Calculated);
							if ( !fileStatus.isEmpty() )
								fileStatus += " ";
							if ( sha1Str == sha1Calculated )
								fileStatus += tr("SHA-1");
							else
								fileStatus += tr("sha-1");
							if ( sha1Str != sha1Calculated && hasDump ) {
								somethingsWrong = true;
								fileItem->setForeground(QMC2_ROMALYZER_COLUMN_SHA1, xmlHandler.redBrush);
							} else if ( hasDump )
								goodDump = true;
							if ( wizardSelectedSets.contains(setKey) ) {
								QList<QTreeWidgetItem *> il = treeWidgetChecksumWizardSearchResult->findItems(setKey, Qt::MatchExactly, QMC2_ROMALYZER_CSWIZ_COLUMN_ID);
								foreach (QTreeWidgetItem *item, il) {
									if ( item->text(QMC2_ROMALYZER_CSWIZ_COLUMN_FILENAME) == childItem->text(QMC2_ROMALYZER_COLUMN_GAME) ||
									     item->whatsThis(QMC2_ROMALYZER_CSWIZ_COLUMN_FILENAME) == childItem->text(QMC2_ROMALYZER_COLUMN_CRC) ) {
										if ( fromCheckSumDb ) {
											item->setIcon(QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS, QIcon(QString::fromUtf8(":/data/img/database_good.png")));
											item->setText(QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS, tr("bad"));
											item->setForeground(QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS, xmlHandler.redBrush);
										} else {
											item->setText(QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS, goodDump ? tr("good") : tr("bad"));
											item->setForeground(QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS, goodDump ? xmlHandler.greenBrush : xmlHandler.redBrush);
										}
										item->setText(QMC2_ROMALYZER_CSWIZ_COLUMN_PATH, effectiveFile);
										if ( goodDump || fromCheckSumDb )
											item->setWhatsThis(QMC2_ROMALYZER_CSWIZ_COLUMN_FILENAME, childItem->text(QMC2_ROMALYZER_COLUMN_CRC));
									}
								}
								on_treeWidgetChecksumWizardSearchResult_itemSelectionChanged();
							}
							qApp->processEvents();
						}

						// MD5
						QString md5Str = childItem->text(QMC2_ROMALYZER_COLUMN_MD5);
						if ( !md5Str.isEmpty() && checkBoxCalculateMD5->isChecked() ) {
							fileItem->setText(QMC2_ROMALYZER_COLUMN_MD5, md5Calculated);
							if ( !fileStatus.isEmpty() )
								fileStatus += " ";
							if ( md5Str == md5Calculated )
								fileStatus += tr("MD5");
							else
								fileStatus += tr("md5");
							if ( md5Str != md5Calculated && hasDump ) {
								somethingsWrong = true;
								fileItem->setForeground(QMC2_ROMALYZER_COLUMN_MD5, xmlHandler.redBrush);
							}
							qApp->processEvents();
						}
					}

					if ( somethingsWrong ) {
						quint64 size = childItem->text(QMC2_ROMALYZER_COLUMN_SIZE).toULongLong();
						if ( !fromCheckSumDb && groupBoxCheckSumDatabase->isChecked() && checkSumDb()->exists(childItem->text(QMC2_ROMALYZER_COLUMN_SHA1), childItem->text(QMC2_ROMALYZER_COLUMN_CRC), size) ) {
							QString pathFromDb, memberFromDb, typeFromDb;
							if ( checkSumDb()->getData(childItem->text(QMC2_ROMALYZER_COLUMN_SHA1), childItem->text(QMC2_ROMALYZER_COLUMN_CRC), &size, &pathFromDb, &memberFromDb, &typeFromDb) ) {
								fileItem = new QTreeWidgetItem(childItem);
								QStringList sl;
								switch ( checkSumDb()->nameToType(typeFromDb) ) {
									case QMC2_CHECKSUM_SCANNER_FILE_ZIP:
										//    fromName        fromPath      toName                                      fromZip
										sl << memberFromDb << pathFromDb << childItem->text(QMC2_ROMALYZER_COLUMN_GAME) << "zip";
										log(tr("check-sum database") + ": " + tr("using member '%1' from archive '%2' with SHA-1 '%3' and CRC '%4' as '%5'").arg(memberFromDb).arg(pathFromDb).arg(childItem->text(QMC2_ROMALYZER_COLUMN_SHA1)).arg(childItem->text(QMC2_ROMALYZER_COLUMN_CRC)).arg(childItem->text(QMC2_ROMALYZER_COLUMN_GAME)));
										break;
									case QMC2_CHECKSUM_SCANNER_FILE_7Z:
										//    fromName        fromPath      toName                                      fromZip
										sl << memberFromDb << pathFromDb << childItem->text(QMC2_ROMALYZER_COLUMN_GAME) << "7z";
										log(tr("check-sum database") + ": " + tr("using member '%1' from archive '%2' with SHA-1 '%3' and CRC '%4' as '%5'").arg(memberFromDb).arg(pathFromDb).arg(childItem->text(QMC2_ROMALYZER_COLUMN_SHA1)).arg(childItem->text(QMC2_ROMALYZER_COLUMN_CRC)).arg(childItem->text(QMC2_ROMALYZER_COLUMN_GAME)));
										break;
									case QMC2_CHECKSUM_SCANNER_FILE_CHD:
										//    fromName        fromPath      toName                                      fromZip
										sl << memberFromDb << pathFromDb << childItem->text(QMC2_ROMALYZER_COLUMN_GAME) << "chd";
										log(tr("check-sum database") + ": " + tr("using CHD '%1' with SHA-1 '%2' as '%3'").arg(pathFromDb).arg(childItem->text(QMC2_ROMALYZER_COLUMN_SHA1)).arg(childItem->text(QMC2_ROMALYZER_COLUMN_GAME)));
										break;
									case QMC2_CHECKSUM_SCANNER_FILE_REGULAR:
										//    fromName        fromPath      toName                                      fromZip
										sl << memberFromDb << pathFromDb << childItem->text(QMC2_ROMALYZER_COLUMN_GAME) << "file";
										log(tr("check-sum database") + ": " + tr("using file '%1' with SHA-1 '%2' and CRC '%3' as '%4'").arg(pathFromDb).arg(childItem->text(QMC2_ROMALYZER_COLUMN_SHA1)).arg(childItem->text(QMC2_ROMALYZER_COLUMN_CRC)).arg(childItem->text(QMC2_ROMALYZER_COLUMN_GAME)));
										break;
									default:
										break;
								}
								if ( !sl.isEmpty() ) {
									setRewriterFileMap.insert(childItem->text(QMC2_ROMALYZER_COLUMN_CRC), sl); 
									QIcon icon;
									if ( isROM )
										icon = QIcon(QString::fromUtf8(":/data/img/rom.png"));
									else if ( isCHD )
										icon = QIcon(QString::fromUtf8(":/data/img/disk2.png"));
									else if ( hasDump )
										icon = QIcon(QString::fromUtf8(":/data/img/fileopen.png"));
									else
										icon = QIcon(QString::fromUtf8(":/data/img/wip.png"));
									QPainter p;
									QPixmap pm(128, 64);
									QPixmap pmIcon = icon.pixmap(64, 64);
									QPixmap pmDb = QIcon(QString::fromUtf8(":/data/img/database.png")).pixmap(64, 64);
									pm.fill(Qt::transparent);
									p.begin(&pm);
									p.setBackgroundMode(Qt::TransparentMode);
									p.drawPixmap(0, 0, pmIcon);
									p.drawPixmap(64, 0, pmDb);
									p.end();
									icon = QIcon(pm);
									childItem->setForeground(QMC2_ROMALYZER_COLUMN_FILESTATUS, xmlHandler.greenBrush);
									childItem->setIcon(QMC2_ROMALYZER_COLUMN_GAME, icon);
									fileStatus.clear();
									if ( !childItem->text(QMC2_ROMALYZER_COLUMN_SIZE).isEmpty() ) {
										fileStatus = tr("SIZE");
										fileItem->setText(QMC2_ROMALYZER_COLUMN_SIZE, childItem->text(QMC2_ROMALYZER_COLUMN_SIZE));
									}
									if ( checkBoxCalculateCRC->isChecked() && !childItem->text(QMC2_ROMALYZER_COLUMN_CRC).isEmpty() ) {
										if ( !fileStatus.isEmpty() )
											fileStatus += " ";
										fileStatus += tr("CRC");
										fileItem->setText(QMC2_ROMALYZER_COLUMN_CRC, childItem->text(QMC2_ROMALYZER_COLUMN_CRC));
									}
									if ( checkBoxCalculateSHA1->isChecked() && !childItem->text(QMC2_ROMALYZER_COLUMN_SHA1).isEmpty() ) {
										if ( !fileStatus.isEmpty() )
											fileStatus += " ";
										fileStatus += tr("SHA-1");
										fileItem->setText(QMC2_ROMALYZER_COLUMN_SHA1, childItem->text(QMC2_ROMALYZER_COLUMN_SHA1));
									}
								}
							}
						} else {
							gameOkay = false;
							childItem->setForeground(QMC2_ROMALYZER_COLUMN_FILESTATUS, xmlHandler.redBrush);
							childItem->setIcon(QMC2_ROMALYZER_COLUMN_GAME, QIcon(QString::fromUtf8(":/data/img/warning.png")));
							log(tr("WARNING: %1 file '%2' loaded from '%3' has incorrect / unexpected check-sums").arg(isCHD ? tr("CHD") : tr("ROM")).arg(childItem->text(QMC2_ROMALYZER_COLUMN_GAME)).arg(effectiveFile));
						}
					} else {
						if ( fileStatus == tr("skipped") )
							childItem->setForeground(QMC2_ROMALYZER_COLUMN_FILESTATUS, xmlHandler.blueBrush);
						else if ( fileStatus == tr("not found") )
							childItem->setForeground(QMC2_ROMALYZER_COLUMN_FILESTATUS, xmlHandler.greyBrush);
						else if ( fileStatus == tr("no dump") )
							childItem->setForeground(QMC2_ROMALYZER_COLUMN_FILESTATUS, xmlHandler.brownBrush);
						else
							childItem->setForeground(QMC2_ROMALYZER_COLUMN_FILESTATUS, xmlHandler.greenBrush);
					}

					childItem->setText(QMC2_ROMALYZER_COLUMN_FILESTATUS, fileStatus);

					if ( checkBoxExpandChecksums->isChecked() && checkBoxExpandFiles->isChecked() )
						childItem->setExpanded(true);

					qApp->processEvents();
				}
			}

			if ( xmlHandler.fileCounter == 0 )
				progressBar->setRange(0, 1);
			progressBar->setFormat(QString());
			progressBar->reset();
			qApp->processEvents();

			if ( qmc2StopParser ) 
				log(tr("interrupted (checking %n file(s) for '%1')", "", wizardSearch ? numWizardFiles : xmlHandler.fileCounter).arg(setKey));
			else {
				gameOkay |= filesError;
				filesSkipped |= filesUnknown;
				if ( !wizardSearch ) {
					if ( gameOkay ) {
						if ( notFoundCounter == xmlHandler.fileCounter ) {
							if ( xmlHandler.fileCounter == 0 ) {
								xmlHandler.parentItem->setText(QMC2_ROMALYZER_COLUMN_FILESTATUS, tr("good"));
								xmlHandler.parentItem->setForeground(QMC2_ROMALYZER_COLUMN_FILESTATUS, xmlHandler.greenBrush);
							} else {
								xmlHandler.parentItem->setText(QMC2_ROMALYZER_COLUMN_FILESTATUS, tr("not found"));
								xmlHandler.parentItem->setForeground(QMC2_ROMALYZER_COLUMN_FILESTATUS, xmlHandler.greyBrush);
								gameOkay = false;
							}
						} else if ( notFoundCounter > 0 ) {
							if ( filesSkipped )
								xmlHandler.parentItem->setText(QMC2_ROMALYZER_COLUMN_FILESTATUS, tr("good / not found / skipped"));
							else
								xmlHandler.parentItem->setText(QMC2_ROMALYZER_COLUMN_FILESTATUS, tr("good / not found"));
							xmlHandler.parentItem->setForeground(QMC2_ROMALYZER_COLUMN_FILESTATUS, xmlHandler.brownBrush);
							gameOkay = false;
						} else if ( noDumpCounter > 0 ) {
							if ( filesSkipped )
								xmlHandler.parentItem->setText(QMC2_ROMALYZER_COLUMN_FILESTATUS, tr("good / no dump / skipped"));
							else
								xmlHandler.parentItem->setText(QMC2_ROMALYZER_COLUMN_FILESTATUS, tr("good / no dump"));
							xmlHandler.parentItem->setForeground(QMC2_ROMALYZER_COLUMN_FILESTATUS, xmlHandler.brownBrush);
						} else {
							if ( filesSkipped )
								xmlHandler.parentItem->setText(QMC2_ROMALYZER_COLUMN_FILESTATUS, tr("good / skipped"));
							else
								xmlHandler.parentItem->setText(QMC2_ROMALYZER_COLUMN_FILESTATUS, tr("good"));
							xmlHandler.parentItem->setForeground(QMC2_ROMALYZER_COLUMN_FILESTATUS, xmlHandler.greenBrush);
						}
					} else {
						if ( notFoundCounter > 0 ) {
							if ( filesSkipped )
								xmlHandler.parentItem->setText(QMC2_ROMALYZER_COLUMN_FILESTATUS, tr("bad / not found / skipped"));
							else
								xmlHandler.parentItem->setText(QMC2_ROMALYZER_COLUMN_FILESTATUS, tr("bad / not found"));
							xmlHandler.parentItem->setForeground(QMC2_ROMALYZER_COLUMN_FILESTATUS, xmlHandler.redBrush);
						} else if ( noDumpCounter > 0 ) {
							if ( filesSkipped )
								xmlHandler.parentItem->setText(QMC2_ROMALYZER_COLUMN_FILESTATUS, tr("bad / no dump / skipped"));
							else
								xmlHandler.parentItem->setText(QMC2_ROMALYZER_COLUMN_FILESTATUS, tr("bad / no dump"));
							xmlHandler.parentItem->setForeground(QMC2_ROMALYZER_COLUMN_FILESTATUS, xmlHandler.redBrush);
						} else {
							if ( filesSkipped )
								xmlHandler.parentItem->setText(QMC2_ROMALYZER_COLUMN_FILESTATUS, tr("bad / skipped"));
							else
								xmlHandler.parentItem->setText(QMC2_ROMALYZER_COLUMN_FILESTATUS, tr("bad"));
							xmlHandler.parentItem->setForeground(QMC2_ROMALYZER_COLUMN_FILESTATUS, xmlHandler.redBrush);
						}
					}
					if ( mode() == QMC2_ROMALYZER_MODE_SYSTEM ) {
						if ( !xmlHandler.parentItem->text(QMC2_ROMALYZER_COLUMN_MERGE).isEmpty() ) {
							switch ( mergeStatus ) {
								case QMC2_ROMALYZER_MERGE_STATUS_OK:
									xmlHandler.parentItem->setIcon(QMC2_ROMALYZER_COLUMN_MERGE, QIcon(QString::fromUtf8(":/data/img/merge_ok.png")));
									break;
								case QMC2_ROMALYZER_MERGE_STATUS_WARN:
									xmlHandler.parentItem->setIcon(QMC2_ROMALYZER_COLUMN_MERGE, QIcon(QString::fromUtf8(":/data/img/merge.png")));
									break;
								case QMC2_ROMALYZER_MERGE_STATUS_CRIT:
									xmlHandler.parentItem->setIcon(QMC2_ROMALYZER_COLUMN_MERGE, QIcon(QString::fromUtf8(":/data/img/merge_nok.png")));
									break;
								default:
									break;
							}
						}
					}
				}

				log(tr("done (checking %n file(s) for '%1')", "", wizardSearch ? numWizardFiles : xmlHandler.fileCounter).arg(setKey));

				if ( !gameOkay )
					analyzerBadSets << setKey;

				if ( gameOkay || !checkBoxSetRewriterGoodDumpsOnly->isChecked() )
					if ( groupBoxSetRewriter->isChecked() )
						if ( checkBoxSetRewriterWhileAnalyzing->isChecked() && !qmc2StopParser && !wizardSearch )
							runSetRewriter();
			}
			if ( qmc2StopParser )
				break;

			treeWidgetChecksums->update();

			i++;
			log(tr("done (analyzing '%1')").arg(setKey));
			log(tr("%n set(s) remaining", "", analyzerList.count() - i));
		}
	}

	animTimer.stop();
	pushButtonAnalyze->setText(tr("&Analyze"));
	pushButtonPause->setVisible(false);
	pushButtonAnalyze->setIcon(QIcon(QString::fromUtf8(":/data/img/romalyzer.png")));
	lineEditSoftwareLists->setEnabled(true);
	lineEditSets->setEnabled(true);
	toolButtonToolsMenu->setEnabled(true);
	if ( checkBoxCalculateSHA1->isChecked() )
		tabChecksumWizard->setEnabled(true);

	progressBar->reset();
	progressBar->setFormat(QString());
	labelStatus->setText(tr("Idle"));
	qApp->processEvents();
	elapsedTime = elapsedTime.addMSecs(analysisTimer.elapsed());
	log(tr("analysis ended") + " - " + tr("elapsed time = %1").arg(elapsedTime.toString("hh:mm:ss.zzz")));

	actionExportToDataFile->setEnabled(!analyzerBadSets.isEmpty());

	if ( wizardSearch && wizardAutomationLevel >= QMC2_ROMALYZER_CSWIZ_AMLVL_REPAIR && !qmc2StopParser ) {
		if ( pushButtonChecksumWizardRepairBadSets->isEnabled() )
			on_pushButtonChecksumWizardRepairBadSets_clicked();
	}

	setActive(false);
	wizardSearch = false;

	if ( mode() == QMC2_ROMALYZER_MODE_SOFTWARE && qmc2SoftwareList ) {
		qmc2SoftwareList->analyzeMenuAction->setEnabled(true);
		qmc2SoftwareList->actionAnalyzeSoftware->setEnabled(true);
		qmc2SoftwareList->actionAnalyzeSoftwareList->setEnabled(true);
		qmc2SoftwareList->actionAnalyzeSoftwareLists->setEnabled(true);
		qmc2SoftwareList->toolButtonAnalyzeSoftware->setEnabled(true);
	}
}

QString &ROMAlyzer::getXmlData(QString gameName, bool includeDTD)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::getXmlData(QString gameName = %1, bool includeDTD = %2)").arg(gameName).arg(includeDTD));
#endif

	static QString xmlBuffer;

	xmlBuffer.clear();

	if ( includeDTD ) {
		xmlBuffer = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
		xmlBuffer += qmc2MachineList->xmlDb()->dtd();
	}
	xmlBuffer += qmc2MachineList->xmlDb()->xml(gameName);

	return xmlBuffer;
}

QString &ROMAlyzer::getSoftwareXmlData(QString listName, QString setName, bool includeDTD)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::getSoftwareXmlData(QString listName = %1, QString setName = %2, bool includeDTD = %3)").arg(listName).arg(setName).arg(includeDTD));
#endif

	static QString swlBuffer;

	swlBuffer.clear();

	if ( includeDTD ) {
		swlBuffer = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
		swlBuffer += qmc2MainWindow->swlDb->dtd();
	}
	swlBuffer += qmc2MainWindow->swlDb->xml(listName, setName);

	return swlBuffer;
}

QString &ROMAlyzer::getEffectiveFile(QTreeWidgetItem *myItem, QString listName, QString gameName, QString fileName, QString wantedCRC, QString merge, QString mergeFile, QString type, QByteArray *fileData, QString *sha1Str, QString *md5Str, bool *isZipped, bool *isSevenZipped, bool *mergeUsed, int fileCounter, QString *fallbackPath, bool isOptionalROM, bool *fromCheckSumDb)
{
	static QCryptographicHash sha1Hash(QCryptographicHash::Sha1);
	static QCryptographicHash md5Hash(QCryptographicHash::Md5);
	static QString effectiveFile;
	static char buffer[QMC2_MAX(QMC2_ROMALYZER_ZIP_BUFFER_SIZE, QMC2_ROMALYZER_FILE_BUFFER_SIZE)];

	effectiveFile.clear();
	fileData->clear();

	bool calcMD5 = checkBoxCalculateMD5->isChecked();
	bool calcSHA1 = checkBoxCalculateSHA1->isChecked();
	bool isCHD = type.split(" ", QString::SkipEmptyParts)[0] == tr("CHD");
	bool sizeLimited = spinBoxMaxFileSize->value() > 0;
	bool chdManagerVerifyCHDs = checkBoxVerifyCHDs->isChecked();
	bool chdManagerUpdateCHDs = checkBoxUpdateCHDs->isChecked();
	bool chdManagerEnabled = groupBoxCHDManager->isChecked() && (chdManagerVerifyCHDs || chdManagerUpdateCHDs);
	bool needProgressWidget;
	QProgressBar *progressWidget;
	qint64 totalSize, myProgress, sizeLeft, len;

	// search for file in ROM paths (first search for "machine/file", then search for "file" in "machine.7z", then in "machine.zip"), load file data when found
	int romPathCount = 0;
	QStringList actualRomPaths;
	foreach (QString romPath, romPaths) {
		if ( mode() == QMC2_ROMALYZER_MODE_SOFTWARE )
			actualRomPaths << romPath + "/" + listName;
		actualRomPaths << romPath;
	}
	foreach (QString romPath, actualRomPaths) {
		romPathCount++;
		progressWidget = NULL;
		needProgressWidget = false;
		QString filePath(romPath + "/" + gameName + "/" + fileName);
		if ( isCHD ) {
			filePath += ".chd";
			if ( fallbackPath->isEmpty() )
				*fallbackPath = filePath;
		}
		if ( QFile::exists(filePath) ) {
			QFileInfo fi(filePath);
			if ( fi.isReadable() ) {
				totalSize = fi.size();
				// load data from a regular file
				if ( sizeLimited ) {
					if ( totalSize > (qint64) spinBoxMaxFileSize->value() * QMC2_ONE_MEGABYTE ) {
						log(tr("size of '%1' is greater than allowed maximum -- skipped").arg(filePath));
						*isZipped = false;
						progressBarFileIO->reset();
						effectiveFile = QMC2_ROMALYZER_FILE_TOO_BIG;
						continue;
					}
				}
				QFile romFile(filePath);
				if ( romFile.open(QIODevice::ReadOnly) ) {
					log(tr("loading '%1'%2").arg(filePath).arg(*mergeUsed ? tr(" (merged)") : ""));
					progressBarFileIO->setRange(0, totalSize);
					progressBarFileIO->reset();
					if ( totalSize > QMC2_ROMALYZER_PROGRESS_THRESHOLD ) {
						if ( isCHD && !chdManagerEnabled )
							needProgressWidget = false;
						else
							needProgressWidget = true;
						if ( needProgressWidget ) {
							progressWidget = new QProgressBar(0);
#if QMC2_CHD_CURRENT_VERSION >= 5
							if ( isCHD ) {
								progressWidget->setRange(0, 100);
								progressWidget->setValue(0);
							} else {
								progressWidget->setRange(0, totalSize);
								progressWidget->setValue(0);
							}
#else
							progressWidget->setRange(0, totalSize);
							progressWidget->setValue(0);
#endif
							treeWidgetChecksums->setItemWidget(myItem, QMC2_ROMALYZER_COLUMN_FILESTATUS, progressWidget);
						}
					} else
						needProgressWidget = false;
					sizeLeft = totalSize;
					if ( calcSHA1 )
						sha1Hash.reset();
					if ( calcMD5 )
						md5Hash.reset();
					if ( isCHD ) {
						QString chdFilePath;
						quint32 chdTotalHunks = 0;
						if ( (len = romFile.read(buffer, QMC2_CHD_HEADER_V3_LENGTH)) > 0 ) {
							if ( len >= QMC2_CHD_HEADER_V3_LENGTH ) {
								log(tr("CHD header information:"));
								QByteArray chdTag(buffer + QMC2_CHD_HEADER_TAG_OFFSET, QMC2_CHD_HEADER_TAG_LENGTH);
								log(tr("  tag: %1").arg(chdTag.constData()));
								quint32 chdVersion = QMC2_TO_UINT32(buffer + QMC2_CHD_HEADER_VERSION_OFFSET);
								log(tr("  version: %1").arg(chdVersion));
								myItem->setText(QMC2_ROMALYZER_COLUMN_TYPE, tr("CHD v%1").arg(chdVersion));
								QLocale locale;
								switch ( chdVersion ) {
									case 3: {
											quint32 chdCompression = QMC2_TO_UINT32(buffer + QMC2_CHD_HEADER_COMPRESSION_OFFSET);
											log(tr("  compression: %1").arg(chdCompressionTypes[chdCompression]));
											quint32 chdFlags = QMC2_TO_UINT32(buffer + QMC2_CHD_HEADER_FLAGS_OFFSET);
											log(tr("  flags: %1, %2").arg(chdFlags & QMC2_CHD_HEADER_FLAG_HASPARENT ? tr("has parent") : tr("no parent")).arg(chdFlags & QMC2_CHD_HEADER_FLAG_ALLOWSWRITES ? tr("allows writes") : tr("read only")));
											chdTotalHunks = QMC2_TO_UINT32(buffer + QMC2_CHD_HEADER_V3_TOTALHUNKS_OFFSET);
											log(tr("  number of total hunks: %1").arg(locale.toString(chdTotalHunks)));
											quint32 chdHunkBytes = QMC2_TO_UINT32(buffer + QMC2_CHD_HEADER_V3_HUNKBYTES_OFFSET);
											log(tr("  number of bytes per hunk: %1").arg(locale.toString(chdHunkBytes)));
											quint64 chdLogicalBytes = QMC2_TO_UINT64(buffer + QMC2_CHD_HEADER_V3_LOGICALBYTES_OFFSET);
											log(tr("  logical size: %1 (%2 B)").arg(humanReadable(chdLogicalBytes)).arg(locale.toString(chdLogicalBytes)));
											log(tr("  real size: %1 (%2 B)").arg(humanReadable(fi.size())).arg(locale.toString(fi.size())));
											QByteArray md5Data((const char *)(buffer + QMC2_CHD_HEADER_V3_MD5_OFFSET), QMC2_CHD_HEADER_V3_MD5_LENGTH);
											log(tr("  MD5 check-sum: %1").arg(QString(md5Data.toHex())));
											QByteArray sha1Data((const char *)(buffer + QMC2_CHD_HEADER_V3_SHA1_OFFSET), QMC2_CHD_HEADER_V3_SHA1_LENGTH);
											log(tr("  SHA-1 check-sum: %1").arg(QString(sha1Data.toHex())));
											if ( chdFlags & QMC2_CHD_HEADER_FLAG_HASPARENT ) {
												QByteArray md5Data((const char *)(buffer + QMC2_CHD_HEADER_V3_PARENTMD5_OFFSET), QMC2_CHD_HEADER_V3_PARENTMD5_LENGTH);
												log(tr("  parent CHD's MD5 check-sum: %1").arg(QString(md5Data.toHex())));
												QByteArray sha1Data((const char *)(buffer + QMC2_CHD_HEADER_V3_PARENTSHA1_OFFSET), QMC2_CHD_HEADER_V3_PARENTSHA1_LENGTH);
												log(tr("  parent CHD's SHA-1 check-sum: %1").arg(QString(sha1Data.toHex())));
											}
										}
										break;

									case 4: {
											quint32 chdCompression = QMC2_TO_UINT32(buffer + QMC2_CHD_HEADER_COMPRESSION_OFFSET);
											log(tr("  compression: %1").arg(chdCompressionTypes[chdCompression]));
											quint32 chdFlags = QMC2_TO_UINT32(buffer + QMC2_CHD_HEADER_FLAGS_OFFSET);
											log(tr("  flags: %1, %2").arg(chdFlags & QMC2_CHD_HEADER_FLAG_HASPARENT ? tr("has parent") : tr("no parent")).arg(chdFlags & QMC2_CHD_HEADER_FLAG_ALLOWSWRITES ? tr("allows writes") : tr("read only")));
											chdTotalHunks = QMC2_TO_UINT32(buffer + QMC2_CHD_HEADER_V4_TOTALHUNKS_OFFSET);
											log(tr("  number of total hunks: %1").arg(locale.toString(chdTotalHunks)));
											quint32 chdHunkBytes = QMC2_TO_UINT32(buffer + QMC2_CHD_HEADER_V4_HUNKBYTES_OFFSET);
											log(tr("  number of bytes per hunk: %1").arg(locale.toString(chdHunkBytes)));
											quint64 chdLogicalBytes = QMC2_TO_UINT64(buffer + QMC2_CHD_HEADER_V4_LOGICALBYTES_OFFSET);
											log(tr("  logical size: %1 (%2 B)").arg(humanReadable(chdLogicalBytes)).arg(locale.toString(chdLogicalBytes)));
											log(tr("  real size: %1 (%2 B)").arg(humanReadable(fi.size())).arg(locale.toString(fi.size())));
											QByteArray sha1Data((const char *)(buffer + QMC2_CHD_HEADER_V4_SHA1_OFFSET), QMC2_CHD_HEADER_V4_SHA1_LENGTH);
											log(tr("  SHA-1 check-sum: %1").arg(QString(sha1Data.toHex())));
											if ( chdFlags & QMC2_CHD_HEADER_FLAG_HASPARENT ) {
												QByteArray sha1Data((const char *)(buffer + QMC2_CHD_HEADER_V4_PARENTSHA1_OFFSET), QMC2_CHD_HEADER_V4_PARENTSHA1_LENGTH);
												log(tr("  parent CHD's SHA-1 check-sum: %1").arg(QString(sha1Data.toHex())));
											}
											QByteArray rawsha1Data((const char *)(buffer + QMC2_CHD_HEADER_V4_RAWSHA1_OFFSET), QMC2_CHD_HEADER_V4_SHA1_LENGTH);
											log(tr("  raw SHA-1 check-sum: %1").arg(QString(rawsha1Data.toHex())));
										}
										break;

									case 5: {
											QString chdCompressors;
											for (int i = 0; i < QMC2_CHD_HEADER_V5_COMPRESSORS_COUNT; i++) {
												QString compressor = QString::fromLocal8Bit(buffer + i * 4 + QMC2_CHD_HEADER_V5_COMPRESSORS_OFFSET, 4);
												if ( chdCompressionTypesV5.contains(compressor) ) {
													if ( i > 0 ) chdCompressors += ", ";
													chdCompressors += chdCompressionTypesV5[compressor];
												} else if ( i == 0 ) {
													chdCompressors = tr("none (uncompressed)");
													break;
												} else
													break;
											}
											log(tr("  compressors: %1").arg(chdCompressors));
											quint32 chdHunkBytes = QMC2_TO_UINT32(buffer + QMC2_CHD_HEADER_V5_HUNKBYTES_OFFSET);
											log(tr("  number of bytes per hunk: %1").arg(locale.toString(chdHunkBytes)));
											quint32 chdUnitBytes = QMC2_TO_UINT32(buffer + QMC2_CHD_HEADER_V5_UNITBYTES_OFFSET);
											log(tr("  number of bytes per unit: %1").arg(locale.toString(chdUnitBytes)));
											quint64 chdLogicalBytes = QMC2_TO_UINT64(buffer + QMC2_CHD_HEADER_V5_LOGICALBYTES_OFFSET);
											log(tr("  logical size: %1 (%2 B)").arg(humanReadable(chdLogicalBytes)).arg(locale.toString(chdLogicalBytes)));
											log(tr("  real size: %1 (%2 B)").arg(humanReadable(fi.size())).arg(locale.toString(fi.size())));
											QByteArray sha1Data((const char *)(buffer + QMC2_CHD_HEADER_V5_SHA1_OFFSET), QMC2_CHD_HEADER_V5_SHA1_LENGTH);
											log(tr("  SHA-1 check-sum: %1").arg(QString(sha1Data.toHex())));
											QByteArray parentSha1DataHex = QByteArray((const char *)(buffer + QMC2_CHD_HEADER_V5_PARENTSHA1_OFFSET), QMC2_CHD_HEADER_V5_PARENTSHA1_LENGTH).toHex();
											if ( !QMC2_CHD_CHECK_NULL_SHA1(parentSha1DataHex) )
												log(tr("  parent CHD's SHA-1 check-sum: %1").arg(QString(parentSha1DataHex)));
											QByteArray rawsha1Data((const char *)(buffer + QMC2_CHD_HEADER_V5_RAWSHA1_OFFSET), QMC2_CHD_HEADER_V5_SHA1_LENGTH);
											log(tr("  raw SHA-1 check-sum: %1").arg(QString(rawsha1Data.toHex())));
										}
										break;

									default:
										log(tr("CHD v%1 not supported -- rest of header information skipped").arg(chdVersion));
										break;
								}

								if ( calcSHA1 || calcMD5 || chdManagerEnabled ) {
									chdFilePath = fi.absoluteFilePath();
									QString chdTempFilePath = QDir::cleanPath(lineEditTemporaryWorkingDirectory->text());
									if ( !chdTempFilePath.endsWith("/") )
										chdTempFilePath += "/";
									chdTempFilePath += fi.baseName() + "-chdman-update.chd";
									if ( chdManagerEnabled ) {
										romFile.close();
										chdManagerCurrentHunk = 0;
#if QMC2_CHD_CURRENT_VERSION >= 5
										chdTotalHunks = 100;
#endif
										chdManagerTotalHunks = chdTotalHunks;
										if ( progressWidget ) {
											progressWidget->setRange(0, chdTotalHunks);
											progressWidget->setValue(0);
										}
										progressBarFileIO->setRange(0, chdTotalHunks);
										progressBarFileIO->reset();
										int step;
										for (step = 0; step < 2 && !qmc2StopParser; step++) {
											QStringList args;
											QString oldFormat;
											if ( progressWidget ) oldFormat = progressWidget->format();
											switch ( step ) {
												case 0:
													if ( chdManagerVerifyCHDs ) {
														if ( progressWidget ) progressWidget->setFormat(tr("Verify - %p%"));
#if QMC2_CHD_CURRENT_VERSION >= 5
														log(tr("CHD manager: verifying CHD"));
														args << "verify" << "--input" << chdFilePath;
#else
														log(tr("CHD manager: verifying CHD"));
														args << "-verify" << chdFilePath;
#endif
													} else
														continue;
													break;

												case 1:
													if ( chdManagerUpdateCHDs ) {
														if ( chdVersion < QMC2_CHD_CURRENT_VERSION ) {
															if ( progressWidget ) progressWidget->setFormat(tr("Update - %p%"));
															log(tr("CHD manager: updating CHD (v%1 -> v%2)").arg(chdVersion).arg(QMC2_CHD_CURRENT_VERSION));
#if QMC2_CHD_CURRENT_VERSION >= 5
															args << "copy" << "--input" << chdFilePath << "--output" << chdTempFilePath;
#else
															args << "-update" << chdFilePath << chdTempFilePath;
#endif
														} else if ( !chdManagerVerifyCHDs ) {
															switch ( chdVersion ) {
																case 3:
																	log(tr("CHD manager: using CHD v%1 header check-sums for CHD verification").arg(chdVersion));
																	if ( calcSHA1 ) {
																		QByteArray sha1Data((const char *)(buffer + QMC2_CHD_HEADER_V3_SHA1_OFFSET), QMC2_CHD_HEADER_V3_SHA1_LENGTH);
																		*sha1Str = QString(sha1Data.toHex());
																	}
																	if ( calcMD5 ) {
																		QByteArray md5Data((const char *)(buffer + QMC2_CHD_HEADER_V3_MD5_OFFSET), QMC2_CHD_HEADER_V3_MD5_LENGTH);
																		*md5Str = QString(md5Data.toHex());
																	}
																	break;

																case 4:
																	log(tr("CHD manager: using CHD v%1 header check-sums for CHD verification").arg(chdVersion));
																	if ( calcSHA1 ) {
																		QByteArray sha1Data((const char *)(buffer + QMC2_CHD_HEADER_V4_SHA1_OFFSET), QMC2_CHD_HEADER_V4_SHA1_LENGTH);
																		*sha1Str = QString(sha1Data.toHex());
																	}
																	break;

																case 5:
																	log(tr("CHD manager: using CHD v%1 header check-sums for CHD verification").arg(chdVersion));
																	if ( calcSHA1 ) {
																		QByteArray sha1Data((const char *)(buffer + QMC2_CHD_HEADER_V5_SHA1_OFFSET), QMC2_CHD_HEADER_V5_SHA1_LENGTH);
																		*sha1Str = QString(sha1Data.toHex());
																	}
																	break;

																default:
																	log(tr("CHD manager: no header check-sums available for CHD verification"));
																	effectiveFile = QMC2_ROMALYZER_FILE_NOT_SUPPORTED;
																	if ( fallbackPath->isEmpty() )
																		*fallbackPath = chdFilePath;
																	break;
															}
															continue;
														} else
															continue;
													} else
														continue;
													break;
											}
											QString command = lineEditCHDManagerExecutableFile->text();
											QProcess *chdManagerProc = new QProcess(this);
											connect(chdManagerProc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(chdManagerError(QProcess::ProcessError)));
											connect(chdManagerProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(chdManagerFinished(int, QProcess::ExitStatus)));
											connect(chdManagerProc, SIGNAL(readyReadStandardOutput()), this, SLOT(chdManagerReadyReadStandardOutput()));
											connect(chdManagerProc, SIGNAL(readyReadStandardError()), this, SLOT(chdManagerReadyReadStandardError()));
											connect(chdManagerProc, SIGNAL(started()), this, SLOT(chdManagerStarted()));
											chdManagerProc->start(command, args);
											chdManagerRunning = true;
											chdManagerMD5Success = chdManagerSHA1Success = false;

											// wait for CHD manager to finish...
											while ( chdManagerRunning && !qmc2StopParser ) {
												QTest::qWait(QMC2_ROMALYZER_PAUSE_TIMEOUT);
												if ( qmc2StopParser ) {
													log(tr("CHD manager: terminating external process"));
													chdManagerProc->kill();
													chdManagerProc->waitForFinished();
													disconnect(chdManagerProc);
													chdManagerProc->deleteLater();
												} else {
													if ( progressWidget ) {
														if ( chdManagerTotalHunks != (quint64)progressWidget->maximum() )
															progressWidget->setRange(0, chdManagerTotalHunks);
														if ( chdManagerCurrentHunk != (quint64)progressWidget->value() )
															progressWidget->setValue(chdManagerCurrentHunk);
													}
													if ( chdManagerTotalHunks != (quint64)progressBarFileIO->maximum() )
														progressBarFileIO->setRange(0, chdManagerTotalHunks);
													if ( chdManagerCurrentHunk != (quint64)progressBarFileIO->value() )
														progressBarFileIO->setValue(chdManagerCurrentHunk);
													progressBarFileIO->update();
													qApp->processEvents();
												}
											}
											chdManagerRunning = false;
											if ( !qmc2StopParser ) {
												if ( chdManagerMD5Success && calcMD5 )
													log(tr("CHD manager: CHD file integrity is good"));
												else if ( chdManagerSHA1Success && calcSHA1 )
													log(tr("CHD manager: CHD file integrity is good"));
												else
													log(tr("CHD manager: WARNING: CHD file integrity is bad"));

												if ( step == 1 && (chdManagerMD5Success || chdManagerSHA1Success) ) {
													log(tr("CHD manager: replacing CHD"));
													if ( progressWidget ) {
														progressWidget->setFormat(tr("Copy"));
														progressWidget->setRange(-1, -1);
														progressWidget->setValue(-1);
													}
													QFile::remove(chdFilePath);
													if ( QFile::rename(chdTempFilePath, chdFilePath) ) {
														log(tr("CHD manager: CHD replaced"));
														myItem->setText(QMC2_ROMALYZER_COLUMN_TYPE, tr("CHD v%1").arg(QMC2_CHD_CURRENT_VERSION));
														QFile romFileTmp(chdFilePath);
														if ( romFileTmp.open(QIODevice::ReadOnly) ) {
															char bufferCopy[QMC2_ROMALYZER_FILE_BUFFER_SIZE];
															strncpy(bufferCopy, buffer, QMC2_ROMALYZER_FILE_BUFFER_SIZE);
															if ( romFileTmp.read(buffer, QMC2_CHD_HEADER_V3_LENGTH) <= 0 ) {
																log(tr("CHD manager: WARNING: failed updating CHD header information"));
																strncpy(buffer, bufferCopy, QMC2_ROMALYZER_FILE_BUFFER_SIZE);
															} else
																chdVersion = QMC2_CHD_CURRENT_VERSION;
															romFileTmp.close();
														}
													} else
														log(tr("CHD manager: FATAL: failed to replace CHD -- updated CHD preserved as '%1', please copy it to '%2' manually!").arg(chdTempFilePath).arg(chdFilePath));
												}

												switch ( chdVersion ) {
													case 3:
														log(tr("CHD manager: using CHD v%1 header check-sums for CHD verification").arg(chdVersion));
														if ( chdManagerSHA1Success && calcSHA1 ) {
															QByteArray sha1Data((const char *)(buffer + QMC2_CHD_HEADER_V3_SHA1_OFFSET), QMC2_CHD_HEADER_V3_SHA1_LENGTH);
															*sha1Str = QString(sha1Data.toHex());
														}
														if ( chdManagerMD5Success && calcMD5 ) {
															QByteArray md5Data((const char *)(buffer + QMC2_CHD_HEADER_V3_MD5_OFFSET), QMC2_CHD_HEADER_V3_MD5_LENGTH);
															*md5Str = QString(md5Data.toHex());
														}
														break;

													case 4:
														log(tr("CHD manager: using CHD v%1 header check-sums for CHD verification").arg(chdVersion));
														if ( chdManagerSHA1Success && calcSHA1 ) {
															QByteArray sha1Data((const char *)(buffer + QMC2_CHD_HEADER_V4_SHA1_OFFSET), QMC2_CHD_HEADER_V4_SHA1_LENGTH);
															*sha1Str = QString(sha1Data.toHex());
														}
														break;

													case 5:
														log(tr("CHD manager: using CHD v%1 header check-sums for CHD verification").arg(chdVersion));
														if ( chdManagerSHA1Success && calcSHA1 ) {
															QByteArray sha1Data((const char *)(buffer + QMC2_CHD_HEADER_V5_SHA1_OFFSET), QMC2_CHD_HEADER_V5_SHA1_LENGTH);
															*sha1Str = QString(sha1Data.toHex());
														}
														break;

													default:
														log(tr("CHD manager: WARNING: no header check-sums available for CHD verification"));
														effectiveFile = QMC2_ROMALYZER_FILE_NOT_SUPPORTED;
														if ( fallbackPath->isEmpty() )
															*fallbackPath = chdFilePath;
														break;
												}
											}
											if ( QFile::exists(chdTempFilePath) ) {
												log(tr("CHD manager: cleaning up"));
												QFile::remove(chdTempFilePath);
											}
											if ( progressWidget ) {
												progressWidget->setFormat(oldFormat);
												progressWidget->reset();
											}
											progressBarFileIO->reset();
										}
									} else {
										switch ( chdVersion ) {
											case 3:
												log(tr("using CHD v%1 header check-sums for CHD verification").arg(chdVersion));
												if ( calcSHA1 ) {
													QByteArray sha1Data((const char *)(buffer + QMC2_CHD_HEADER_V3_SHA1_OFFSET), QMC2_CHD_HEADER_V3_SHA1_LENGTH);
													*sha1Str = QString(sha1Data.toHex());
												}
												if ( calcMD5 ) {
													QByteArray md5Data((const char *)(buffer + QMC2_CHD_HEADER_V3_MD5_OFFSET), QMC2_CHD_HEADER_V3_MD5_LENGTH);
													*md5Str = QString(md5Data.toHex());
												}
												break;

											case 4:
												log(tr("using CHD v%1 header check-sums for CHD verification").arg(chdVersion));
												if ( calcSHA1 ) {
													QByteArray sha1Data((const char *)(buffer + QMC2_CHD_HEADER_V4_SHA1_OFFSET), QMC2_CHD_HEADER_V4_SHA1_LENGTH);
													*sha1Str = QString(sha1Data.toHex());
												}
												break;

											case 5:
												log(tr("using CHD v%1 header check-sums for CHD verification").arg(chdVersion));
												if ( calcSHA1 ) {
													QByteArray sha1Data((const char *)(buffer + QMC2_CHD_HEADER_V5_SHA1_OFFSET), QMC2_CHD_HEADER_V5_SHA1_LENGTH);
													*sha1Str = QString(sha1Data.toHex());
												}
												break;

											default:
												log(tr("WARNING: no header check-sums available for CHD verification"));
												effectiveFile = QMC2_ROMALYZER_FILE_NOT_SUPPORTED;
												if ( fallbackPath->isEmpty() )
													*fallbackPath = chdFilePath;
												break;
										}
									}
								}
							} else {
								log(tr("WARNING: can't read CHD header information"));
								effectiveFile = QMC2_ROMALYZER_FILE_ERROR;
								if ( fallbackPath->isEmpty() )
									*fallbackPath = chdFilePath;
							}
						} else {
							log(tr("WARNING: can't read CHD header information"));
							effectiveFile = QMC2_ROMALYZER_FILE_ERROR;
							if ( fallbackPath->isEmpty() )
								*fallbackPath = chdFilePath;
						}
					} else {
						while ( (len = romFile.read(buffer, QMC2_ROMALYZER_FILE_BUFFER_SIZE)) > 0 && !qmc2StopParser ) {
							QByteArray readData((const char *)buffer, len);
							fileData->append(readData);
							if ( calcSHA1 )
								sha1Hash.addData(readData);
							if ( calcMD5 )
								md5Hash.addData(readData);
							sizeLeft -= len;
							myProgress = totalSize - sizeLeft;
							progressBarFileIO->setValue(myProgress);
							if ( needProgressWidget )
								if ( progressWidget ) progressWidget->setValue(myProgress);
							progressBarFileIO->update();
							qApp->processEvents();
						}

						ulong crc = crc32(0, NULL, 0);
						crc = crc32(crc, (const Bytef *)fileData->data(), fileData->size());
						QFileInfo fi(filePath);
						QStringList sl;
						//    fromName         fromPath    toName           fromZip
						sl << fi.fileName() << filePath << fi.fileName() << "file";
						setRewriterFileMap.insert(crcToString(crc), sl); 

						if ( calcSHA1 )
							*sha1Str = sha1Hash.result().toHex();
						if ( calcMD5 )
							*md5Str = md5Hash.result().toHex();
					}
					romFile.close();
					effectiveFile = filePath;
					*isZipped = false;
					if ( needProgressWidget ) {
						if ( progressWidget ) {
							progressWidget->reset();
							treeWidgetChecksums->removeItemWidget(myItem, QMC2_ROMALYZER_COLUMN_FILESTATUS);
						}
						if ( progressWidget )
							delete progressWidget;
					}
					progressBarFileIO->reset();
				} else {
					log(tr("WARNING: found '%1' but can't read from it although permissions seem okay - check file integrity").arg(filePath));
				}
			} else
				log(tr("WARNING: found '%1' but can't read from it - check permission").arg(filePath));
		} else {
			if ( isCHD ) {
				if ( romPathCount == actualRomPaths.count() ) {
					QString baseName = QFileInfo(filePath).baseName();
					QStringList chdPaths = actualRomPaths;
					for (int i = 0; i < chdPaths.count(); i++)
						chdPaths[i] = chdPaths[i] + QDir::separator() + gameName + QDir::separator() + baseName + ".chd";
					QString sP;
					if ( actualRomPaths.count() > 1 )
						sP = tr("searched paths: %1").arg(chdPaths.join(", "));
					else
						sP = tr("searched path: %1").arg(chdPaths[0]);
					if ( myItem->text(QMC2_ROMALYZER_COLUMN_EMUSTATUS) == tr("no dump") )
						sP += " (" + tr("no dump exists") + " / " + tr("SHA-1") + " " + tr("unknown") + ")";
					log(tr("WARNING: CHD file '%1' not found").arg(baseName) + " -- " + sP);
				}
				if ( mergeFile.isEmpty() && merge.isEmpty() )
					if ( fallbackPath->isEmpty() )
						*fallbackPath = filePath;
			}
		}

		if ( effectiveFile.isEmpty() && !qmc2StopParser ) {
			filePath = romPath + "/" + gameName + ".7z";
			if ( QFile::exists(filePath) ) {
				QFileInfo fi(filePath);
				if ( fi.isReadable() ) {
					// try loading data from a 7z archive
					SevenZipFile sevenZipFile(filePath);
					if ( sevenZipFile.open() ) {
						// identify file by CRC
						int index = sevenZipFile.indexOfCrc(wantedCRC);
						if ( index >= 0 ) {
							SevenZipMetaData metaData = sevenZipFile.itemList()[index];
							if ( sizeLimited ) {
								if ( metaData.size() > (quint64) spinBoxMaxFileSize->value() * QMC2_ONE_MEGABYTE ) {
									log(tr("size of '%1' from '%2' is greater than allowed maximum -- skipped").arg(metaData.name()).arg(filePath));
									*isSevenZipped = true;
									effectiveFile = QMC2_ROMALYZER_FILE_TOO_BIG;
									if ( fallbackPath->isEmpty() )
										*fallbackPath = filePath;
									continue;
								}
							}
							log(tr("loading '%1' with CRC '%2' from '%3' as '%4'%5").arg(metaData.name()).arg(wantedCRC).arg(filePath).arg(myItem->text(QMC2_ROMALYZER_COLUMN_GAME)).arg(*mergeUsed ? tr(" (merged)") : ""));
							QByteArray data;
							qApp->processEvents();
							quint64 readLength = sevenZipFile.read(index, &data);
							qApp->processEvents();
							if ( sevenZipFile.hasError() )
								log(tr("ERROR") + ": " + sevenZipFile.lastError());
							if ( readLength != metaData.size() )
								log(tr("WARNING") + ": " + tr("actual bytes read != file size in header") + " - " + tr("check archive integrity"));
							ulong crc = crc32(0, NULL, 0);
							crc = crc32(crc, (const Bytef *)data.data(), data.size());
							QString crcString = crcToString(crc);
							if ( crcString != wantedCRC )
								log(tr("WARNING") + ": " + tr("actual CRC != CRC in header") + " - " + tr("check archive integrity"));
							else {
								if ( !wantedCRC.isEmpty() ) {
									QStringList sl;
									//    fromName           fromPath    toName                                      fromZip
									sl << metaData.name() << filePath << myItem->text(QMC2_ROMALYZER_COLUMN_GAME) << "7z";
									setRewriterFileMap.insert(wantedCRC, sl); 
								} else {
									if ( !crcString.isEmpty() ) {
										QStringList sl;
										//    fromName           fromPath    toName                                      fromZip
										sl << metaData.name() << filePath << myItem->text(QMC2_ROMALYZER_COLUMN_GAME) << "7z";
										setRewriterFileMap.insert(crcString, sl);
										if ( groupBoxSetRewriter->isChecked() )
											log(tr("WARNING: the CRC for '%1' from '%2' is unknown to the emulator, the set rewriter will use the recalculated CRC '%3' to qualify the file").arg(metaData.name()).arg(filePath).arg(crcString));
									} else if ( groupBoxSetRewriter->isChecked() )
										log(tr("WARNING: unable to determine the CRC for '%1' from '%2', the set rewriter will NOT store this file in the new set").arg(metaData.name()).arg(filePath));
								}

								if ( calcSHA1 ) {
									sha1Hash.reset();
									sha1Hash.addData(data);
									*sha1Str = sha1Hash.result().toHex();
								}

								if ( calcMD5 ) {
									md5Hash.reset();
									md5Hash.addData(data);
									*md5Str = md5Hash.result().toHex();
								}

								if ( !isCHD )
									*fileData = data;

								effectiveFile = filePath;
								*isSevenZipped = true;
							}
						} else if ( mergeFile.isEmpty() ) {
							if ( !isCHD ) {
								if ( wantedCRC.isEmpty() ) {
									log(tr("WARNING: unable to identify '%1' from '%2' by CRC (no dump exists / CRC unknown)").arg(fileName).arg(filePath));
									effectiveFile = QMC2_ROMALYZER_NO_DUMP;
								} else
									log(tr("WARNING: unable to identify '%1' from '%2' by CRC '%3'").arg(fileName).arg(filePath).arg(wantedCRC) + QString(isOptionalROM ? " (" + tr("optional") + ")" : ""));
							}
						}
					} else
						log(tr("WARNING: found '%1' but can't open it for decompression - check file integrity").arg(filePath));
				} else
					log(tr("WARNING: found '%1' but can't read from it - check permission").arg(filePath));
			}

			if ( !effectiveFile.isEmpty() || qmc2StopParser )
				break;

			filePath = romPath + "/" + gameName + ".zip";
			if ( fallbackPath->isEmpty() )
				*fallbackPath = filePath;
			if ( QFile::exists(filePath) ) {
				QFileInfo fi(filePath);
				if ( fi.isReadable() ) {
					// try loading data from a ZIP archive
					unzFile zipFile = unzOpen(filePath.toUtf8().constData());
					if ( zipFile ) {
						// identify file by CRC
						unz_file_info zipInfo;
						QMap<uLong, QString> crcIdentMap;
						uLong ulCRC = wantedCRC.toULong(0, 16);
						do {
							if ( unzGetCurrentFileInfo(zipFile, &zipInfo, buffer, QMC2_ROMALYZER_ZIP_BUFFER_SIZE, 0, 0, 0, 0) == UNZ_OK )
								crcIdentMap[zipInfo.crc] = QString((const char *)buffer);
						} while ( unzGoToNextFile(zipFile) == UNZ_OK && !crcIdentMap.contains(ulCRC) );
						unzGoToFirstFile(zipFile);
						QString fn = "QMC2_DUMMY_FILENAME";
						if ( crcIdentMap.contains(ulCRC) )
							fn = crcIdentMap[ulCRC];
						else if ( mergeFile.isEmpty() ) {
							if ( !isCHD ) {
								if ( wantedCRC.isEmpty() ) {
									log(tr("WARNING: unable to identify '%1' from '%2' by CRC (no dump exists / CRC unknown)").arg(fileName).arg(filePath));
									effectiveFile = QMC2_ROMALYZER_NO_DUMP;
								} else
									log(tr("WARNING: unable to identify '%1' from '%2' by CRC '%3'").arg(fileName).arg(filePath).arg(wantedCRC) + QString(isOptionalROM ? " (" + tr("optional") + ")" : ""));
							}
							fn = fileName;
						}

						if ( unzLocateFile(zipFile, fn.toUtf8().constData(), 2) == UNZ_OK ) { // NOT case-sensitive filename compare!
							totalSize = 0;
							if ( unzGetCurrentFileInfo(zipFile, &zipInfo, 0, 0, 0, 0, 0, 0) == UNZ_OK ) 
								totalSize = zipInfo.uncompressed_size;
							if ( sizeLimited ) {
								if ( totalSize > (qint64) spinBoxMaxFileSize->value() * QMC2_ONE_MEGABYTE ) {
									log(tr("size of '%1' from '%2' is greater than allowed maximum -- skipped").arg(fn).arg(filePath));
									*isZipped = true;
									progressBarFileIO->reset();
									effectiveFile = QMC2_ROMALYZER_FILE_TOO_BIG;
									if ( fallbackPath->isEmpty() )
										*fallbackPath = filePath;
									continue;
								}
							}
							if ( unzOpenCurrentFile(zipFile) == UNZ_OK ) {
								log(tr("loading '%1' with CRC '%2' from '%3' as '%4'%5").arg(fn).arg(wantedCRC).arg(filePath).arg(myItem->text(QMC2_ROMALYZER_COLUMN_GAME)).arg(*mergeUsed ? tr(" (merged)") : ""));
								progressBarFileIO->setRange(0, totalSize);
								progressBarFileIO->reset();
								needProgressWidget = totalSize > QMC2_ROMALYZER_PROGRESS_THRESHOLD;
								if ( needProgressWidget ) {
									progressWidget = new QProgressBar(0);
									progressWidget->setRange(0, totalSize);
									progressWidget->setValue(0);
									treeWidgetChecksums->setItemWidget(myItem, QMC2_ROMALYZER_COLUMN_FILESTATUS, progressWidget);
								}
								sizeLeft = totalSize;
								if ( calcSHA1 )
									sha1Hash.reset();
								if ( calcMD5 )
									md5Hash.reset();
								while ( (len = unzReadCurrentFile(zipFile, buffer, QMC2_ROMALYZER_ZIP_BUFFER_SIZE)) > 0 && !qmc2StopParser ) {
									QByteArray readData((const char *)buffer, len);
									if ( !isCHD )
										fileData->append(readData);
									if ( calcSHA1 )
										sha1Hash.addData(readData);
									if ( calcMD5 )
										md5Hash.addData(readData);
									sizeLeft -= len;
									myProgress = totalSize - sizeLeft;
									progressBarFileIO->setValue(myProgress);
									if ( needProgressWidget )
										progressWidget->setValue(myProgress);
									progressBarFileIO->update();
									qApp->processEvents();
								}
								unzCloseCurrentFile(zipFile);
								effectiveFile = filePath;
								if ( fallbackPath->isEmpty() )
									*fallbackPath = filePath;
								QString fromName = fileName;
								if ( fn != "QMC2_DUMMY_FILENAME" )
									fromName = fn;
								if ( !wantedCRC.isEmpty() ) {
									QStringList sl;
									//    fromName    fromPath    toName                                      fromZip
									sl << fromName << filePath << myItem->text(QMC2_ROMALYZER_COLUMN_GAME) << "zip";
									setRewriterFileMap.insert(wantedCRC, sl); 
								} else {
									ulong crc = crc32(0, NULL, 0);
									crc = crc32(crc, (const Bytef *)fileData->data(), fileData->size());
									QString fallbackCRC = crcToString(crc);
									if ( !fallbackCRC.isEmpty() ) {
										QStringList sl;
										//    fromName    fromPath    toName                                      fromZip
										sl << fromName << filePath << myItem->text(QMC2_ROMALYZER_COLUMN_GAME) << "zip";
										setRewriterFileMap.insert(fallbackCRC, sl); 
										if ( groupBoxSetRewriter->isChecked() )
											log(tr("WARNING: the CRC for '%1' from '%2' is unknown to the emulator, the set rewriter will use the recalculated CRC '%3' to qualify the file").arg(fileName).arg(filePath).arg(fallbackCRC));
									} else if ( groupBoxSetRewriter->isChecked() )
										log(tr("WARNING: unable to determine the CRC for '%1' from '%2', the set rewriter will NOT store this file in the new set").arg(fileName).arg(filePath));
								}
								*isZipped = true;
								if ( calcSHA1 )
									*sha1Str = sha1Hash.result().toHex();
								if ( calcMD5 )
									*md5Str = md5Hash.result().toHex();
								if ( needProgressWidget ) {
									if ( progressWidget ) {
										progressWidget->reset();
										treeWidgetChecksums->removeItemWidget(myItem, QMC2_ROMALYZER_COLUMN_FILESTATUS);
									}
									if ( progressWidget )
										delete progressWidget;
								}
								progressBarFileIO->reset();
							} else
								log(tr("WARNING: unable to decompress '%1' from '%2' - check file integrity").arg(fn).arg(filePath));
						}
						unzClose(zipFile);
					} else
						log(tr("WARNING: found '%1' but can't open it for decompression - check file integrity").arg(filePath));
				} else
					log(tr("WARNING: found '%1' but can't read from it - check permission").arg(filePath));
			}
		}

		if ( !effectiveFile.isEmpty() || qmc2StopParser )
			break;
	}

	// try merging if applicable...
	if ( effectiveFile.isEmpty() && !qmc2StopParser ) {
		if ( mergeFile.isEmpty() && !merge.isEmpty() ) {
			// romof/cloneof is set, but the merge's file name is missing... use the same file name for the merge
			mergeFile = fileName;
		}
		if ( !mergeFile.isEmpty() && !qmc2StopParser ) {
			// romof/clonef is set, and the merge's file name is given
			*mergeUsed = true;
			switch ( mode() ) {
				case QMC2_ROMALYZER_MODE_SOFTWARE:
					effectiveFile = getEffectiveFile(myItem, listName, merge, mergeFile, wantedCRC, "", "", type, fileData, sha1Str, md5Str, isZipped, isSevenZipped, mergeUsed, fileCounter, fallbackPath, isOptionalROM, fromCheckSumDb);
					break;
				case QMC2_ROMALYZER_MODE_SYSTEM:
				default: {
						QString nextMerge = getXmlData(merge).split("\n")[0];
						int position = nextMerge.indexOf("romof=");
						if ( position > -1 ) {
							nextMerge = nextMerge.mid(position + 7);
							nextMerge = nextMerge.left(nextMerge.indexOf("\""));
							effectiveFile = getEffectiveFile(myItem, listName, merge, mergeFile, wantedCRC, nextMerge, mergeFile, type, fileData, sha1Str, md5Str, isZipped, isSevenZipped, mergeUsed, fileCounter, fallbackPath, isOptionalROM, fromCheckSumDb);
						} else
							effectiveFile = getEffectiveFile(myItem, listName, merge, mergeFile, wantedCRC, "", "", type, fileData, sha1Str, md5Str, isZipped, isSevenZipped, mergeUsed, fileCounter, fallbackPath, isOptionalROM, fromCheckSumDb);
					}
					break;
			}
		}
	}

	// try check-sum database if available/applicable...
	if ( effectiveFile.isEmpty() && !qmc2StopParser && groupBoxCheckSumDatabase->isChecked() ) {
		QString wantedSHA1 = myItem->text(QMC2_ROMALYZER_COLUMN_SHA1);
		quint64 size = myItem->text(QMC2_ROMALYZER_COLUMN_SIZE).toULongLong();
		if ( checkSumDb()->exists(wantedSHA1, wantedCRC, size) ) {
			QString pathFromDb, memberFromDb, typeFromDb;
			if ( checkSumDb()->getData(wantedSHA1, wantedCRC, &size, &pathFromDb, &memberFromDb, &typeFromDb) ) {
				QStringList sl;
				switch ( checkSumDb()->nameToType(typeFromDb) ) {
					case QMC2_CHECKSUM_SCANNER_FILE_ZIP:
						//    fromName        fromPath      toName                                      fromZip
						sl << memberFromDb << pathFromDb << myItem->text(QMC2_ROMALYZER_COLUMN_GAME) << "zip";
						log(tr("check-sum database") + ": " + tr("using member '%1' from archive '%2' with SHA-1 '%3' and CRC '%4' as '%5'").arg(memberFromDb).arg(pathFromDb).arg(wantedSHA1).arg(wantedCRC).arg(myItem->text(QMC2_ROMALYZER_COLUMN_GAME)));
						*isZipped = true;
						break;
					case QMC2_CHECKSUM_SCANNER_FILE_7Z:
						//    fromName        fromPath      toName                                      fromZip
						sl << memberFromDb << pathFromDb << myItem->text(QMC2_ROMALYZER_COLUMN_GAME) << "7z";
						log(tr("check-sum database") + ": " + tr("using member '%1' from archive '%2' with SHA-1 '%3' and CRC '%4' as '%5'").arg(memberFromDb).arg(pathFromDb).arg(wantedSHA1).arg(wantedCRC).arg(myItem->text(QMC2_ROMALYZER_COLUMN_GAME)));
						*isSevenZipped = true;
						break;
					case QMC2_CHECKSUM_SCANNER_FILE_CHD:
						//    fromName        fromPath      toName                                      fromZip
						sl << memberFromDb << pathFromDb << myItem->text(QMC2_ROMALYZER_COLUMN_GAME) << "chd";
						log(tr("check-sum database") + ": " + tr("using CHD '%1' with SHA-1 '%2' as '%3'").arg(pathFromDb).arg(wantedSHA1).arg(myItem->text(QMC2_ROMALYZER_COLUMN_GAME) + ".chd"));
						break;
					case QMC2_CHECKSUM_SCANNER_FILE_REGULAR:
						//    fromName        fromPath      toName                                      fromZip
						sl << memberFromDb << pathFromDb << myItem->text(QMC2_ROMALYZER_COLUMN_GAME) << "file";
						log(tr("check-sum database") + ": " + tr("using file '%1' with SHA-1 '%2' and CRC '%3' as '%4'").arg(pathFromDb).arg(wantedSHA1).arg(wantedCRC).arg(myItem->text(QMC2_ROMALYZER_COLUMN_GAME)));
						break;
					default:
						break;
				}
				if ( !sl.isEmpty() ) {
					*sha1Str = wantedSHA1;
					*fromCheckSumDb = true;
					effectiveFile = pathFromDb;
					setRewriterFileMap.insert(wantedCRC, sl); 
				}
			}
		}
	}

	if ( effectiveFile.isEmpty() )
		effectiveFile = QMC2_ROMALYZER_FILE_NOT_FOUND;

	return effectiveFile;
}

bool ROMAlyzer::createBackup(QString filePath)
{
	if ( !checkBoxCreateBackups->isChecked() || lineEditBackupFolder->text().isEmpty() )
		return true;
	QFile sourceFile(filePath);
	if ( !sourceFile.exists() )
		return true;
	QDir backupDir(lineEditBackupFolder->text());
	QFileInfo backupDirInfo(backupDir.absolutePath());
	if ( backupDirInfo.exists() ) {
		if ( backupDirInfo.isWritable() ) {
#if defined(QMC2_OS_WIN)
			QString filePathCopy = filePath;
			QString destinationPath = QDir::cleanPath(QString(backupDir.absolutePath() + "/" + filePathCopy.replace(":", "")));
#else
			QString destinationPath = QDir::cleanPath(backupDir.absolutePath() + "/" + filePath);
#endif
			QFileInfo destinationPathInfo(destinationPath);
			if ( !destinationPathInfo.dir().exists() ) {
				if ( !backupDir.mkpath(destinationPathInfo.dir().absolutePath()) ) {
					log(tr("backup") + ": " + tr("FATAL: target path '%1' cannot be created").arg(destinationPathInfo.dir().absolutePath()));
					return false;
				}
			}
			if ( !sourceFile.open(QIODevice::ReadOnly) ) {
				log(tr("backup") + ": " + tr("FATAL: source file '%1' cannot be opened for reading").arg(filePath));
				return false;
			}
			QFile destinationFile(destinationPath);
			if ( destinationFile.open(QIODevice::WriteOnly) ) {
				log(tr("backup") + ": " + tr("creating backup copy of '%1' as '%2'").arg(filePath).arg(destinationPath));
				char ioBuffer[QMC2_ROMALYZER_FILE_BUFFER_SIZE];
				int count = 0;
				int len = 0;
				bool success = true;
				while ( success && (len = sourceFile.read(ioBuffer, QMC2_ROMALYZER_FILE_BUFFER_SIZE)) > 0 ) {
					if ( count++ % QMC2_BACKUP_IO_RESPONSE == 0 )
						qApp->processEvents();
					if ( destinationFile.write(ioBuffer, len) != len ) {
						log(tr("backup") + ": " + tr("FATAL: I/O error while writing to '%1'").arg(destinationPath));
						success = false;
					}
				}
				sourceFile.close();
				destinationFile.close();
				if ( success ) {
					log(tr("backup") + ": " + tr("done (creating backup copy of '%1' as '%2')").arg(filePath).arg(destinationPath));
					return true;
				} else
					return false;
			} else {
				log(tr("backup") + ": " + tr("FATAL: destination file '%1' cannot be opened for writing").arg(destinationPath));
				sourceFile.close();
				return false;
			}
		} else {
			log(tr("backup") + ": " + tr("FATAL: backup folder '%1' isn't writable").arg(backupDir.absolutePath()));
			return false;
		}
	} else {
		log(tr("backup") + ": " + tr("FATAL: backup folder '%1' doesn't exist").arg(backupDir.absolutePath()));
		return false;
	}
}

void ROMAlyzer::setMode(int mode)
{
	switch ( mode ) {
		case QMC2_ROMALYZER_MODE_SOFTWARE:
			m_currentMode = QMC2_ROMALYZER_MODE_SOFTWARE;
			checkBoxAutoScroll->setToolTip(tr("Automatically scroll to the currently analyzed software"));
			tabWidgetAnalysis->setTabText(QMC2_ROMALYZER_PAGE_RCR, tr("Software collection rebuilder"));
			setWindowTitle(tr("ROMAlyzer") + " [" + tr("software mode") + "]");
			m_settingsKey = "SoftwareROMAlyzer";
			lineEditSoftwareLists->setVisible(true);
			toolButtonToolsMenu->setVisible(false);
			checkBoxSelectGame->setVisible(false);
			break;
		case QMC2_ROMALYZER_MODE_SYSTEM:
		default:
			m_currentMode = QMC2_ROMALYZER_MODE_SYSTEM;
			checkBoxSelectGame->setText(tr("Select machine"));
			checkBoxSelectGame->setToolTip(tr("Select machine in machine list if selected in analysis report?"));
			checkBoxAutoScroll->setToolTip(tr("Automatically scroll to the currently analyzed machine"));
			tabWidgetAnalysis->setTabText(QMC2_ROMALYZER_PAGE_RCR, tr("ROM collection rebuilder"));
			setWindowTitle(tr("ROMAlyzer") + " [" + tr("system mode") + "]");
			m_settingsKey = "ROMAlyzer";
			lineEditSoftwareLists->setVisible(false);
			toolButtonToolsMenu->setVisible(true);
			checkBoxSelectGame->setVisible(true);
			break;
	}
}

void ROMAlyzer::on_tabWidgetAnalysis_currentChanged(int index)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::on_tabWidgetAnalysis_currentChanged(int index = %1)").arg(index));
#endif

	switch ( index ) {
		case QMC2_ROMALYZER_PAGE_LOG:
			textBrowserLog->horizontalScrollBar()->setValue(0);
			break;
		case QMC2_ROMALYZER_PAGE_CSWIZ:
			on_groupBoxCheckSumDatabase_toggled(groupBoxCheckSumDatabase->isChecked());
			break;
		case QMC2_ROMALYZER_PAGE_RCR:
			switchToCollectionRebuilder();
			break;
		default:
			break;
	}
}

void ROMAlyzer::on_treeWidgetChecksums_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_treeWidgetChecksums_itemSelectionChanged()");
#endif

	if ( checkBoxSelectGame->isChecked() ) {
		QList<QTreeWidgetItem *> items = treeWidgetChecksums->selectedItems();
		if ( items.count() > 0 ) {
			QTreeWidgetItem *item = items[0];
			while ( (void*)item->parent() != (void *)treeWidgetChecksums && item->parent() != 0 )
				item = item->parent();
			QStringList words = item->text(QMC2_ROMALYZER_COLUMN_GAME).split(" ", QString::SkipEmptyParts);
			selectItem(words[0]);
		}
	}
}

void ROMAlyzer::selectItem(QString gameName)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::selectItem(QString gameName = %1)").arg(gameName));
#endif

	switch ( qmc2MainWindow->stackedWidgetView->currentIndex() ) {
		case QMC2_VIEWMACHINELIST_INDEX: {
			      QTreeWidgetItem *gameItem = qmc2MachineListItemHash[gameName];
			      if ( gameItem ) {
				      qmc2MainWindow->treeWidgetMachineList->clearSelection();
				      qmc2MainWindow->treeWidgetMachineList->setCurrentItem(gameItem);
				      qmc2MainWindow->treeWidgetMachineList->scrollToItem(gameItem, qmc2CursorPositioningMode);
				      gameItem->setSelected(true);
			      }
			      break;
		      }
		case QMC2_VIEWHIERARCHY_INDEX: {
			       QTreeWidgetItem *hierarchyItem = qmc2HierarchyItemHash[gameName];
			       if ( hierarchyItem ) {
				       qmc2MainWindow->treeWidgetHierarchy->clearSelection();
				       qmc2MainWindow->treeWidgetHierarchy->setCurrentItem(hierarchyItem);
				       qmc2MainWindow->treeWidgetHierarchy->scrollToItem(hierarchyItem, qmc2CursorPositioningMode);
				       hierarchyItem->setSelected(true);
			       }
			       break;
		       }
		case QMC2_VIEWCATEGORY_INDEX: {
			      QTreeWidgetItem *categoryItem = qmc2CategoryItemHash[gameName];
			      if ( categoryItem ) {
				      qmc2MainWindow->treeWidgetCategoryView->clearSelection();
				      qmc2MainWindow->treeWidgetCategoryView->setCurrentItem(categoryItem);
				      qmc2MainWindow->treeWidgetCategoryView->scrollToItem(categoryItem, qmc2CursorPositioningMode);
				      categoryItem->setSelected(true);
			      }
			      break;
		      }
		case QMC2_VIEWVERSION_INDEX: {
			     QTreeWidgetItem *versionItem = qmc2VersionItemHash[gameName];
			     if ( versionItem ) {
				     qmc2MainWindow->treeWidgetVersionView->clearSelection();
				     qmc2MainWindow->treeWidgetVersionView->setCurrentItem(versionItem);
				     qmc2MainWindow->treeWidgetVersionView->scrollToItem(versionItem, qmc2CursorPositioningMode);
				     versionItem->setSelected(true);
			     }
			     break;
		     }
	}
}

QString ROMAlyzer::humanReadable(quint64 value, int digits)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::humanReadable(quint64 value = %1, int digits = %2)").arg(value).arg(digits));
#endif

	static QString humanReadableString;
	static qreal humanReadableValue;
	QLocale locale;

#if __WORDSIZE == 64
	if ( (qreal)value / (qreal)QMC2_ONE_KILOBYTE < (qreal)QMC2_ONE_KILOBYTE ) {
		humanReadableValue = (qreal)value / (qreal)QMC2_ONE_KILOBYTE;
		humanReadableString = locale.toString(humanReadableValue, 'f', digits) + QString(tr(" KB"));
	} else if ( (qreal)value / (qreal)QMC2_ONE_MEGABYTE < (qreal)QMC2_ONE_KILOBYTE ) {
		humanReadableValue = (qreal)value / (qreal)QMC2_ONE_MEGABYTE;
		humanReadableString = locale.toString(humanReadableValue, 'f', digits) + QString(tr(" MB"));
	} else if ( (qreal)value / (qreal)QMC2_ONE_GIGABYTE < (qreal)QMC2_ONE_KILOBYTE ) {
		humanReadableValue = (qreal)value / (qreal)QMC2_ONE_GIGABYTE;
		humanReadableString = locale.toString(humanReadableValue, 'f', digits) + QString(tr(" GB"));
	} else {
		humanReadableValue = (qreal)value / (qreal)QMC2_ONE_TERABYTE;
		humanReadableString = locale.toString(humanReadableValue, 'f', digits) + QString(tr(" TB"));
	}
#else
	if ( (qreal)value / (qreal)QMC2_ONE_KILOBYTE < (qreal)QMC2_ONE_KILOBYTE ) {
		humanReadableValue = (qreal)value / (qreal)QMC2_ONE_KILOBYTE;
		humanReadableString = locale.toString(humanReadableValue, 'f', digits) + QString(tr(" KB"));
	} else if ( (qreal)value / (qreal)QMC2_ONE_MEGABYTE < (qreal)QMC2_ONE_KILOBYTE ) {
		humanReadableValue = (qreal)value / (qreal)QMC2_ONE_MEGABYTE;
		humanReadableString = locale.toString(humanReadableValue, 'f', digits) + QString(tr(" MB"));
	} else {
		humanReadableValue = (qreal)value / (qreal)QMC2_ONE_GIGABYTE;
		humanReadableString = locale.toString(humanReadableValue, 'f', digits) + QString(tr(" GB"));
	}
#endif

	return humanReadableString;
}

void ROMAlyzer::log(const QString &msg)
{
	QString message = msg;
	message.prepend(QTime::currentTime().toString("hh:mm:ss.zzz") + ": ");
	bool scrollBarMaximum = (textBrowserLog->verticalScrollBar()->value() == textBrowserLog->verticalScrollBar()->maximum());
	textBrowserLog->appendPlainText(message);
	if ( scrollBarMaximum ) {
		textBrowserLog->update();
		qApp->processEvents();
		textBrowserLog->verticalScrollBar()->setValue(textBrowserLog->verticalScrollBar()->maximum());
	}
}

void ROMAlyzer::on_toolButtonBrowseBackupFolder_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_toolButtonBrowseBackupFolder_clicked()");
#endif

	QString s = QFileDialog::getExistingDirectory(this, tr("Choose backup folder"), lineEditBackupFolder->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog));
	if ( !s.isNull() )
		lineEditBackupFolder->setText(s);
	raise();
}

void ROMAlyzer::on_toolButtonBrowseCHDManagerExecutableFile_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_toolButtonBrowseCHDManagerExecutableFile_clicked()");
#endif

	QString s = QFileDialog::getOpenFileName(this, tr("Choose CHD manager executable file"), lineEditCHDManagerExecutableFile->text(), tr("All files (*)"), 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		lineEditCHDManagerExecutableFile->setText(s);
	raise();
}

void ROMAlyzer::on_toolButtonBrowseTemporaryWorkingDirectory_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_toolButtonBrowseTemporaryWorkingDirectory_clicked()");
#endif

	QString s = QFileDialog::getExistingDirectory(this, tr("Choose temporary working directory"), lineEditTemporaryWorkingDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog));
	if ( !s.isNull() )
		lineEditTemporaryWorkingDirectory->setText(s);
	raise();
}

void ROMAlyzer::on_toolButtonBrowseSetRewriterOutputPath_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_toolButtonBrowseSetRewriterOutputPath_clicked()");
#endif

	QString s = QFileDialog::getExistingDirectory(this, tr("Choose output directory"), lineEditSetRewriterOutputPath->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog));
	if ( !s.isNull() )
		lineEditSetRewriterOutputPath->setText(s);
	raise();
}

void ROMAlyzer::on_toolButtonBrowseSetRewriterAdditionalRomPath_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_toolButtonBrowseSetRewriterAdditionalRomPath_clicked()");
#endif

	QString s = QFileDialog::getExistingDirectory(this, tr("Choose additional ROM path"), lineEditSetRewriterAdditionalRomPath->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog));
	if ( !s.isNull() )
		lineEditSetRewriterAdditionalRomPath->setText(s);
	raise();
}

void ROMAlyzer::chdManagerStarted()
{
#ifdef QMC2_DEBUG
	QProcess *proc = (QProcess *)sender();
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::chdManagerStarted(), proc = %1").arg((qulonglong)proc));
#endif

	chdManagerRunning = true;
	chdManagerCurrentHunk = 0;
	log(tr("CHD manager: external process started"));
}

void ROMAlyzer::chdManagerFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
#ifdef QMC2_DEBUG
	QProcess *proc = (QProcess *)sender();
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::chdManagerFinished(int exitCode = %1, QProcess::ExitStatus exitStatus = %2), proc = %3").arg(exitCode).arg((int)exitStatus).arg((qulonglong)proc));
#endif

	chdManagerRunning = false;
	QString statusString = tr("unknown");
	switch ( exitStatus ) {
		case QProcess::NormalExit:
			statusString = tr("normal");
			break;
		case QProcess::CrashExit:
			statusString = tr("crashed");
			break;
	}
	log(tr("CHD manager: external process finished (exit code = %1, exit status = %2)").arg(exitCode).arg(statusString));
}

void ROMAlyzer::chdManagerReadyReadStandardOutput()
{
	QProcess *proc = (QProcess *)sender();

#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::chdManagerReadyReadStandardOutput(), proc = %1").arg((qulonglong)proc));
#endif

	QString output = proc->readAllStandardOutput();
	QStringList sl = output.split("\n");
	foreach (QString s, sl) {
		s = s.trimmed();
		if ( !s.isEmpty() ) {
			log(tr("CHD manager: stdout: %1").arg(s));
			if ( s.contains("MD5 verification successful") )
				chdManagerMD5Success = true;
			if ( s.contains("SHA1 verification successful") )
				chdManagerSHA1Success = true;
		}
	}
}

void ROMAlyzer::chdManagerReadyReadStandardError()
{
	QProcess *proc = (QProcess *)sender();

#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::chdManagerReadyReadStandardError(), proc = %1").arg((qulonglong)proc));
#endif

	QString output = proc->readAllStandardError();
	QStringList sl = output.split("\n");
	foreach (QString s, sl) {
		s = s.trimmed();
		if ( !s.isEmpty() ) {
			log(tr("CHD manager: stderr: %1").arg(s));
#if QMC2_CHD_CURRENT_VERSION >= 5
			if ( s.contains(QRegExp(", \\d+\\.\\d+\\%\\ complete\\.\\.\\.")) ) {
				QRegExp rx(", (\\d+)\\.(\\d+)\\%\\ complete\\.\\.\\.");
				int pos = rx.indexIn(s);
				if ( pos > -1 ) {
					chdManagerCurrentHunk = rx.cap(1).toInt(); // 'current hunk' is misused as a percentage value, and 'total hunks' is thus set to 100 constantly
					int decimal = rx.cap(2).toInt();
					if ( decimal >= 5 ) chdManagerCurrentHunk += 1;
				}
			} else {
				if ( s.contains("Compression complete ... final ratio =") )
					chdManagerSHA1Success = true;
			}
#else
			if ( s.contains(QRegExp("hunk \\d+/\\d+\\.\\.\\.")) ) {
				QRegExp rx("(\\d+)/(\\d+)");
				int pos = rx.indexIn(s);
				if ( pos > -1 ) {
					chdManagerCurrentHunk = rx.cap(1).toInt();
					chdManagerTotalHunks = rx.cap(2).toInt();
				}
			} else {
				if ( s.contains("Input MD5 verified") )
					chdManagerMD5Success = true;
				if ( s.contains("Input SHA-1 verified") )
					chdManagerSHA1Success = true;
			}
#endif
		}
	}
}

void ROMAlyzer::chdManagerError(QProcess::ProcessError processError)
{
#ifdef QMC2_DEBUG
	QProcess *proc = (QProcess *)sender();
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::chdManagerError(QProcess::ProcessError processError = %1), proc = %2").arg((int)processError).arg((qulonglong)proc));
#endif

	switch ( processError ) {
		case QProcess::FailedToStart:
			log(tr("CHD manager: failed to start"));
			break;

		case QProcess::Crashed:
			log(tr("CHD manager: crashed"));
			break;

		case QProcess::WriteError:
			log(tr("CHD manager: write error"));
			break;

		case QProcess::ReadError:
			log(tr("CHD manager: read error"));
			break;

		default:
			log(tr("CHD manager: unknown error %1").arg(processError));
			break;
	}

	chdManagerRunning = false;
}

void ROMAlyzer::on_lineEditChecksumWizardHash_textChanged(const QString &/*text*/)
{
	if ( groupBoxCheckSumDatabase->isChecked() )
		QTimer::singleShot(0, this, SLOT(indicateCheckSumDbQueryStatusUnknown()));
	m_checkSumTextChangedTimer.start(QMC2_SEARCH_DELAY);
}

void ROMAlyzer::lineEditChecksumWizardHash_textChanged_delayed()
{
	m_checkSumTextChangedTimer.stop();
	QString sha1, crc;
	switch ( comboBoxChecksumWizardHashType->currentIndex() ) {
		case QMC2_ROMALYZER_CSWIZ_HASHTYPE_CRC:
			crc = lineEditChecksumWizardHash->text();
			if ( !groupBoxCheckSumDatabase->isChecked() )
				currentFilesCrcChecksum = crc;
			break;

		default:
		case QMC2_ROMALYZER_CSWIZ_HASHTYPE_SHA1:
			sha1 = lineEditChecksumWizardHash->text();
			if ( !groupBoxCheckSumDatabase->isChecked() )
				currentFilesSHA1Checksum = sha1;
			break;
	}
	if ( groupBoxCheckSumDatabase->isChecked() ) {
		if ( sha1.length() != 40 && crc.length() != 8 )
			QTimer::singleShot(0, this, SLOT(indicateCheckSumDbQueryStatusUnknown()));
		else {
			if ( checkSumDb()->exists(sha1, crc, currentFilesSize) ) {
				if ( crc.isEmpty() )
					currentFilesCrcChecksum = checkSumDb()->getCrc(sha1);
				else if ( sha1.isEmpty() )
					currentFilesSHA1Checksum = checkSumDb()->getSha1(crc);
				QTimer::singleShot(0, this, SLOT(indicateCheckSumDbQueryStatusGood()));
			} else
				QTimer::singleShot(0, this, SLOT(indicateCheckSumDbQueryStatusBad()));
		}
	}
}

void ROMAlyzer::on_groupBoxSetRewriter_toggled(bool enable)
{
	tabWidgetAnalysis->setTabEnabled(QMC2_ROMALYZER_PAGE_RCR, groupBoxCheckSumDatabase->isChecked() && enable && !checkSumScannerThread()->isActive);
	if ( mode() == QMC2_ROMALYZER_MODE_SYSTEM )
		qmc2MainWindow->update_rebuildRomActions_visibility();
	else if ( qmc2SoftwareList )
		qmc2SoftwareList->updateRebuildSoftwareMenuVisibility();
	if ( !enable ) {
		if ( collectionRebuilder() ) {
			delete collectionRebuilder();
			m_collectionRebuilder = NULL;
		}
	}
}

void ROMAlyzer::on_groupBoxCheckSumDatabase_toggled(bool enable)
{
	widgetCheckSumDbQueryStatus->setVisible(enable);
	if ( checkSumScannerThread() )
		tabWidgetAnalysis->setTabEnabled(QMC2_ROMALYZER_PAGE_RCR, groupBoxSetRewriter->isChecked() && enable && !checkSumScannerThread()->isActive);
	else
		tabWidgetAnalysis->setTabEnabled(QMC2_ROMALYZER_PAGE_RCR, groupBoxSetRewriter->isChecked() && enable);
	if ( mode() == QMC2_ROMALYZER_MODE_SYSTEM )
		qmc2MainWindow->update_rebuildRomActions_visibility();
	else if ( qmc2SoftwareList )
		qmc2SoftwareList->updateRebuildSoftwareMenuVisibility();
	if ( enable )
		QTimer::singleShot(0, this, SLOT(lineEditChecksumWizardHash_textChanged_delayed()));
	else {
		if ( collectionRebuilder() ) {
			delete collectionRebuilder();
			m_collectionRebuilder = NULL;
		}
	}
}

void ROMAlyzer::on_comboBoxChecksumWizardHashType_currentIndexChanged(int index)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::on_comboBoxChecksumWizardHashType_currentIndexChanged(int index)").arg(index));
#endif

	switch ( index ) {
		case QMC2_ROMALYZER_CSWIZ_HASHTYPE_CRC:
			if ( !currentFilesCrcChecksum.isEmpty() && lineEditChecksumWizardHash->text().length() == 40 )
				lineEditChecksumWizardHash->setText(currentFilesCrcChecksum);
			else
				lineEditChecksumWizardHash->clear();
			break;

		default:
		case QMC2_ROMALYZER_CSWIZ_HASHTYPE_SHA1:
			if ( !currentFilesSHA1Checksum.isEmpty() && lineEditChecksumWizardHash->text().length() == 8 )
				lineEditChecksumWizardHash->setText(currentFilesSHA1Checksum);
			else
				lineEditChecksumWizardHash->clear();
			break;
	}
}

void ROMAlyzer::on_pushButtonChecksumWizardSearch_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_pushButtonChecksumWizardSearch_clicked()");
#endif

	qmc2StopParser = false;

	treeWidgetChecksumWizardSearchResult->clear();
	QString searchedChecksum = lineEditChecksumWizardHash->text().toLower();
	if ( searchedChecksum.isEmpty() )
		return;

	pushButtonChecksumWizardSearch->setEnabled(false);
	lineEditChecksumWizardHash->setReadOnly(true);
	pushButtonAnalyze->setEnabled(false);
	toolButtonToolsMenu->setEnabled(false);
	lineEditSoftwareLists->setEnabled(false);
	lineEditSets->setEnabled(false);
	labelStatus->setText(tr("Check-sum search"));

	QString hashStartString;
	int hashStartOffset;
	switch ( comboBoxChecksumWizardHashType->currentIndex() ) {
		case QMC2_ROMALYZER_CSWIZ_HASHTYPE_CRC:
			hashStartString = "crc=\"";
			hashStartOffset = 5;
			break;

		default:
		case QMC2_ROMALYZER_CSWIZ_HASHTYPE_SHA1:
			hashStartString = "sha1=\"";
			hashStartOffset = 6;
			break;
	}

	switch ( mode() ) {
		case QMC2_ROMALYZER_MODE_SOFTWARE: {
				QStringList uniqueSoftwareLists = qmc2MainWindow->swlDb->uniqueSoftwareLists();
				progressBar->setRange(0, uniqueSoftwareLists.count());
				int progressCount = 0, updateCount = 0;
				foreach (QString list, uniqueSoftwareLists) {
					foreach (QString set, qmc2MainWindow->swlDb->uniqueSoftwareSets(list)) {
						QStringList xmlLines = qmc2MainWindow->swlDb->xml(list, set).split("\n", QString::SkipEmptyParts);
						for (int i = 0; i < xmlLines.count(); i++) {
							QString xmlLine = xmlLines[i];
							int hashIndex = xmlLine.indexOf(hashStartString);
							if ( hashIndex >= 0 ) {
								int hashPos = hashIndex + hashStartOffset;
								QString currentChecksum = xmlLine.mid(hashPos, xmlLine.indexOf("\"", hashPos) - hashPos).toLower();
								if ( currentChecksum == searchedChecksum ) {
									int fileNamePos;
									QString fileType;
									if ( xmlLine.startsWith("<disk name=\"") ) {
										fileType = tr("CHD");
										fileNamePos = xmlLine.indexOf("<disk name=\"") + 12;
									} else {
										fileType = tr("ROM");
										fileNamePos = xmlLine.indexOf("<rom name=\"") + 11;
									}
									QString fileName = xmlLine.mid(fileNamePos, xmlLine.indexOf("\"", fileNamePos) - fileNamePos);
									QTreeWidgetItem *item = new QTreeWidgetItem(treeWidgetChecksumWizardSearchResult);
									item->setText(QMC2_ROMALYZER_CSWIZ_COLUMN_ID, list + ":" + set);
									item->setText(QMC2_ROMALYZER_CSWIZ_COLUMN_FILENAME, fileName.replace("&amp;", "&").replace("&lt;", "<").replace("&gt;", ">").replace("&quot;", "\"").replace("&apos;", "'"));
									item->setText(QMC2_ROMALYZER_CSWIZ_COLUMN_TYPE, fileType);
									item->setText(QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS, tr("unknown"));
									if ( wizardAutomationLevel >= QMC2_ROMALYZER_CSWIZ_AMLVL_SELECT )
										item->setSelected(true);
								}
							}
						}
						if ( updateCount++ % QMC2_ROMALYZER_CKSUM_SEARCH_RESPONSE ) {
							progressBar->setValue(progressCount);
							qApp->processEvents();
						}
					}
					progressBar->setValue(progressCount++);
					qApp->processEvents();
				}
			}
			break;
		case QMC2_ROMALYZER_MODE_SYSTEM:
		default:
			progressBar->setRange(0, qmc2MainWindow->treeWidgetMachineList->topLevelItemCount());
			for (int i = 0; i < qmc2MainWindow->treeWidgetMachineList->topLevelItemCount(); i++) {
				if ( i % QMC2_ROMALYZER_CKSUM_SEARCH_RESPONSE ) {
					progressBar->setValue(i);
					qApp->processEvents();
				}
				QString currentGame = qmc2MainWindow->treeWidgetMachineList->topLevelItem(i)->text(QMC2_MACHINELIST_COLUMN_NAME);
				QStringList xmlLines = qmc2MachineList->xmlDb()->xml(currentGame).split("\n", QString::SkipEmptyParts);
				for (int j = 0; j < xmlLines.count(); j++) {
					QString xmlLine = xmlLines[j];
					int hashIndex = xmlLine.indexOf(hashStartString);
					if ( hashIndex >= 0 ) {
						int hashPos = hashIndex + hashStartOffset;
						QString currentChecksum = xmlLine.mid(hashPos, xmlLine.indexOf("\"", hashPos) - hashPos).toLower();
						if ( currentChecksum == searchedChecksum ) {
							int fileNamePos;
							QString fileType;
							if ( xmlLine.startsWith("<disk name=\"") ) {
								fileType = tr("CHD");
								fileNamePos = xmlLine.indexOf("<disk name=\"") + 12;
							} else {
								fileType = tr("ROM");
								fileNamePos = xmlLine.indexOf("<rom name=\"") + 11;
							}
							QString fileName = xmlLine.mid(fileNamePos, xmlLine.indexOf("\"", fileNamePos) - fileNamePos);
							QTreeWidgetItem *item = new QTreeWidgetItem(treeWidgetChecksumWizardSearchResult);
							item->setText(QMC2_ROMALYZER_CSWIZ_COLUMN_ID, currentGame);
							item->setText(QMC2_ROMALYZER_CSWIZ_COLUMN_FILENAME, fileName.replace("&amp;", "&").replace("&lt;", "<").replace("&gt;", ">").replace("&quot;", "\"").replace("&apos;", "'"));
							item->setText(QMC2_ROMALYZER_CSWIZ_COLUMN_TYPE, fileType);
							item->setText(QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS, tr("unknown"));
							if ( wizardAutomationLevel >= QMC2_ROMALYZER_CSWIZ_AMLVL_SELECT )
								item->setSelected(true);
						}
					}
				}
			}
			break;
	}

	progressBar->reset();
	labelStatus->setText(tr("Idle"));
	pushButtonAnalyze->setEnabled(true);
	toolButtonToolsMenu->setEnabled(true);
	lineEditSoftwareLists->setEnabled(true);
	lineEditSets->setEnabled(true);
	pushButtonChecksumWizardSearch->setEnabled(true);
	lineEditChecksumWizardHash->setReadOnly(false);
	qApp->processEvents();

	if ( wizardAutomationLevel >= QMC2_ROMALYZER_CSWIZ_AMLVL_ANALYZE && !qmc2StopParser ) {
		if ( pushButtonChecksumWizardAnalyzeSelectedSets->isEnabled() )
			on_pushButtonChecksumWizardAnalyzeSelectedSets_clicked();
	}

	currentFilesSize = 0;
}

void ROMAlyzer::runChecksumWizard()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::runChecksumWizard()");
#endif

	if ( !currentFilesSHA1Checksum.isEmpty() ) {
		comboBoxChecksumWizardHashType->setCurrentIndex(QMC2_ROMALYZER_CSWIZ_HASHTYPE_SHA1);
		lineEditChecksumWizardHash->setText(currentFilesSHA1Checksum);
		tabWidgetAnalysis->setCurrentWidget(tabChecksumWizard);
		pushButtonChecksumWizardSearch->animateClick();
	} else if ( !currentFilesCrcChecksum.isEmpty() ) {
		comboBoxChecksumWizardHashType->setCurrentIndex(QMC2_ROMALYZER_CSWIZ_HASHTYPE_CRC);
		lineEditChecksumWizardHash->setText(currentFilesCrcChecksum);
		tabWidgetAnalysis->setCurrentWidget(tabChecksumWizard);
		pushButtonChecksumWizardSearch->animateClick();
	}
}

void ROMAlyzer::runSetRewriter()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::runSetRewriter()");
#endif

	// when called through the context menu and multiple sets are in the report, rerun the analyzer on the respective set (to make sure that pre-check data is current)
	if ( setRewriterItem == NULL ) {
		QList<QTreeWidgetItem *> il = treeWidgetChecksums->selectedItems();
		if ( il.count() > 0 ) {
			QTreeWidgetItem *item = il[0];
			while ( item->parent() != NULL ) item = item->parent();
			if ( item != NULL ) {
				if ( treeWidgetChecksums->topLevelItemCount() > 1 || qmc2StopParser ) {
					groupBoxSetRewriter->setEnabled(false);
					bool savedSRWA = checkBoxSetRewriterWhileAnalyzing->isChecked();
					checkBoxSetRewriterWhileAnalyzing->setChecked(false);
					QStringList setKeyTokens = item->text(QMC2_ROMALYZER_COLUMN_GAME).split(" ", QString::SkipEmptyParts)[0].split(":", QString::SkipEmptyParts);
					switch ( mode() ) {
						case QMC2_ROMALYZER_MODE_SOFTWARE:
							lineEditSoftwareLists->setText(setKeyTokens[0]);
							lineEditSets->setText(setKeyTokens[1]);
							break;
						case QMC2_ROMALYZER_MODE_SYSTEM:
						default:
							lineEditSets->setText(setKeyTokens[0]);
							break;
					}
					qmc2StopParser = false;
					analyze();
					checkBoxSetRewriterWhileAnalyzing->setChecked(savedSRWA);
					groupBoxSetRewriter->setEnabled(true);
					setRewriterItem = treeWidgetChecksums->topLevelItem(0);
					if ( setRewriterItem == NULL ) return;
				} else
					setRewriterItem = item;
			} else
				return;
		} else
			return;
	}

	// check output path and write permission
	QString outPath = lineEditSetRewriterOutputPath->text();
	if ( !outPath.isEmpty() ) {
		QDir dir(outPath);
		if ( dir.exists() ) {
			QFileInfo dirInfo(outPath);
			if ( !dirInfo.isDir() ) {
				log(tr("set rewriter: WARNING: can't rewrite set '%1', output path is not a directory").arg(setRewriterSetName));
				return;
			}
			if ( !dirInfo.isWritable() ) {
				log(tr("set rewriter: WARNING: can't rewrite set '%1', output path is not writable").arg(setRewriterSetName));
				return;
			}
		} else {
			log(tr("set rewriter: WARNING: can't rewrite set '%1', output path does not exist").arg(setRewriterSetName));
			return;
		}
	} else {
		log(tr("set rewriter: WARNING: can't rewrite set '%1', output path is empty").arg(setRewriterSetName));
		return;
	}

	QString setName, listName;
	QStringList setKeyTokens;
	switch ( mode() ) {
		case QMC2_ROMALYZER_MODE_SOFTWARE:
			setKeyTokens = setRewriterSetName.split(":", QString::SkipEmptyParts);
			if ( setKeyTokens.count() > 1 ) {
				listName = setKeyTokens[0];
				setName = setKeyTokens[1];
				outPath += "/" + listName;
				bool success = true;
				QDir d(QDir::cleanPath(outPath));
				if ( !d.exists() )
					success = d.mkdir(QDir::cleanPath(outPath));
				if ( !success ) {
					log(tr("set rewriter: WARNING: can't rewrite set '%1', output path is not writable").arg(setRewriterSetName));
					return;
				}
			}
			break;
		case QMC2_ROMALYZER_MODE_SYSTEM:
		default:
			setName = setRewriterSetName;
			break;
	}

	if ( !outPath.endsWith("/") )
		outPath += "/";

	if ( radioButtonSetRewriterZipArchives->isChecked() )
		outPath += setName + ".zip";
	else
		outPath += setName;

	QLocale locale;

	QString savedStatusText = labelStatus->text();
	labelStatus->setText(tr("Reading '%1' - %2").arg(setRewriterSetName).arg(locale.toString(setRewriterSetCount)));
	progressBar->reset();
	progressBar->setFormat(QString("%1 / %2").arg(0).arg(setRewriterFileMap.count()));
	qApp->processEvents();
	QString modeString = tr("space-efficient");
	if ( checkBoxSetRewriterSelfContainedSets->isChecked() )
		modeString = tr("self-contained");

	log(tr("set rewriter: rewriting %1 set '%2' to '%3'").arg(modeString).arg(setRewriterSetName).arg(outPath));

	bool loadOkay = true;
	bool ignoreErrors = !checkBoxSetRewriterAbortOnError->isChecked();
	QMapIterator<QString, QStringList> it(setRewriterFileMap);
	QMap<QString, QByteArray> outputDataMap;
	int count = 0;
	QStringList uniqueCRCs;
	while ( it.hasNext() && loadOkay ) {
		progressBar->setValue(++count);
		progressBar->setFormat(QString("%1 / %2").arg(count).arg(setRewriterFileMap.count()));
		it.next();

		QString fileCRC = it.key();
		QString fileName = it.value()[0];
		QString filePath = it.value()[1];
		QString outputFileName = it.value()[2];
		bool fromZip = (it.value()[3] == "zip");
		bool fromSevenZip = (it.value()[3] == "7z");

		if ( checkBoxSetRewriterUniqueCRCs->isChecked() ) {
			if ( uniqueCRCs.contains(fileCRC) ) {
				log(tr("set rewriter: skipping '%1' with CRC '%2' from '%3' as '%4'").arg(fileName).arg(fileCRC).arg(filePath).arg(outputFileName));
				continue;
			}
		}

		log(tr("set rewriter: loading '%1' with CRC '%2' from '%3' as '%4'").arg(fileName).arg(fileCRC).arg(filePath).arg(outputFileName));

		QByteArray fileData;
		if ( fromZip ) {
			if ( readZipFileData(filePath, fileCRC, &fileData) ) {
				outputDataMap[outputFileName] = fileData;
				uniqueCRCs << fileCRC;
			} else {
				if ( checkBoxSetRewriterGoodDumpsOnly->isChecked() ) {
					log(tr("set rewriter: FATAL: can't load '%1' with CRC '%2' from '%3', aborting").arg(fileName).arg(fileCRC).arg(filePath));
					loadOkay = ignoreErrors ? true : false;
				} else
					log(tr("set rewriter: WARNING: can't load '%1' with CRC '%2' from '%3', ignoring this file").arg(fileName).arg(fileCRC).arg(filePath));
			}
		} else if ( fromSevenZip ) {
			if ( readSevenZipFileData(filePath, fileCRC, &fileData) ) {
				outputDataMap[outputFileName] = fileData;
				uniqueCRCs << fileCRC;
			} else {
				if ( checkBoxSetRewriterGoodDumpsOnly->isChecked() ) {
					log(tr("set rewriter: FATAL: can't load '%1' with CRC '%2' from '%3', aborting").arg(fileName).arg(fileCRC).arg(filePath));
					loadOkay = ignoreErrors ? true : false;
				} else
					log(tr("set rewriter: WARNING: can't load '%1' with CRC '%2' from '%3', ignoring this file").arg(fileName).arg(fileCRC).arg(filePath));
			}
		} else {
			if ( readFileData(filePath, fileCRC, &fileData) ) {
				outputDataMap[outputFileName] = fileData;
				uniqueCRCs << fileCRC;
			} else {
				if ( checkBoxSetRewriterGoodDumpsOnly->isChecked() ) {
					log(tr("set rewriter: FATAL: can't load '%1' with CRC '%2', aborting").arg(filePath).arg(fileCRC));
					loadOkay = ignoreErrors ? true : false;
				} else
					log(tr("set rewriter: WARNING: can't load '%1' with CRC '%2', ignoring this file").arg(filePath).arg(fileCRC));
			}
		}
	}
	progressBar->reset();
	progressBar->setFormat(QString());

	if ( loadOkay ) {
		if ( !checkBoxSetRewriterSelfContainedSets->isChecked() ) {
			// remove redundant files (if applicable)
			bool hasValidParent = !setRewriterItem->text(QMC2_ROMALYZER_COLUMN_MERGE).isEmpty();
			for (int i = 0; i < setRewriterItem->childCount(); i++) {
				QTreeWidgetItem *childItem = setRewriterItem->child(i);
				if ( !childItem->text(QMC2_ROMALYZER_COLUMN_MERGE).isEmpty() ) {
					if ( !childItem->text(QMC2_ROMALYZER_COLUMN_CRC).isEmpty() ) {
						QList<QStringList> fileEntries = setRewriterFileMap.values(childItem->text(QMC2_ROMALYZER_COLUMN_CRC));
						foreach (QStringList fileEntry, fileEntries) {
							if ( fileEntry.count() == 4 ) { // valid entry?
								QString localName = fileEntry[2];
								if ( outputDataMap.contains(localName) && hasValidParent ) {
									log(tr("set rewriter: removing redundant file '%1' with CRC '%2' from output data").arg(localName).arg(childItem->text(QMC2_ROMALYZER_COLUMN_CRC)));
									outputDataMap.remove(localName);
								}
							}
						}
					}
				}
			}
		}
		if ( !outputDataMap.isEmpty() ) {
			log(tr("set rewriter: writing new %1 set '%2' in '%3'").arg(modeString).arg(setRewriterSetName).arg(outPath));
			labelStatus->setText(tr("Writing '%1' - %2").arg(setRewriterSetName).arg(locale.toString(setRewriterSetCount)));
			if ( radioButtonSetRewriterZipArchives->isChecked() ) {
				if ( writeAllZipData(outPath, &outputDataMap, true, progressBar) )
					log(tr("set rewriter: new %1 set '%2' in '%3' successfully created").arg(modeString).arg(setRewriterSetName).arg(outPath));
				else {
					log(tr("set rewriter: FATAL: failed to create new %1 set '%2' in '%3'").arg(modeString).arg(setRewriterSetName).arg(outPath));
					loadOkay = ignoreErrors ? true : false;
				}
			} else {
				if ( writeAllFileData(outPath, &outputDataMap, true, progressBar) ) {
					log(tr("set rewriter: new %1 set '%2' in '%3' successfully created").arg(modeString).arg(setRewriterSetName).arg(outPath));
				} else {
					log(tr("set rewriter: FATAL: failed to create new %1 set '%2' in '%3'").arg(modeString).arg(setRewriterSetName).arg(outPath));
					loadOkay = ignoreErrors ? true : false;
				}
			}
		} else {
			log(tr("set rewriter: INFORMATION: no output data available, thus not rewriting set '%1' to '%2'").arg(setRewriterSetName).arg(outPath));
			setRewriterItem->setIcon(QMC2_ROMALYZER_COLUMN_GAME, QIcon(QIcon(QString::fromUtf8(":/data/img/filesaveas.png")).pixmap(QSize(64, 64), QIcon::Disabled)));
			loadOkay = false;
		}
	}

	if ( loadOkay ) {
		log(tr("set rewriter: done (rewriting %1 set '%2' to '%3')").arg(modeString).arg(setRewriterSetName).arg(outPath));
		setRewriterItem->setIcon(QMC2_ROMALYZER_COLUMN_GAME, QIcon(QString::fromUtf8(":/data/img/filesaveas.png")));
	}

	labelStatus->setText(savedStatusText);
	progressBar->setRange(0, 100);
	progressBar->setValue(0);
	progressBar->reset();
	progressBar->setFormat(QString());
	qApp->processEvents();
}

void ROMAlyzer::analyzeDeviceRefs()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::analyzeDeviceRefs()");
#endif

	QList<QTreeWidgetItem *> il = treeWidgetChecksums->selectedItems();
	if ( !il.isEmpty() ) {
		QStringList deviceRefs = il[0]->whatsThis(QMC2_ROMALYZER_COLUMN_GAME).split(",", QString::SkipEmptyParts);
		deviceRefs.removeDuplicates();
		if ( !deviceRefs.isEmpty() ) {
			lineEditSets->setText(deviceRefs.join(" "));
			QTimer::singleShot(0, this, SLOT(analyze()));
		}
	}
}

void ROMAlyzer::importFromDataFile()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::importFromDataFile()");
#endif

	QString storedPath;
	if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + m_settingsKey + "/LastDataFilePath") )
		storedPath = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/LastDataFilePath").toString();
	QString dataFilePath = QFileDialog::getOpenFileName(this, tr("Choose data file to import from"), storedPath, tr("Data files (*.dat)") + ";;" + tr("All files (*)"), 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !dataFilePath.isNull() ) {
		QStringList nameList;
		QFile dataFile(dataFilePath);
		if ( dataFile.open(QIODevice::ReadOnly | QIODevice::Text) ) {
			setActive(true);
			QTextStream ts(&dataFile);
			QString pattern1 = "<machine name=\"";
			QString pattern2 = "<game name=\"";
			while ( !ts.atEnd() ) {
				QString line = ts.readLine().trimmed();
				if ( line.startsWith(pattern1) )
					nameList << line.mid(pattern1.length(), line.indexOf("\"", pattern1.length()) - pattern1.length());
				else if ( line.startsWith(pattern2) )
					nameList << line.mid(pattern2.length(), line.indexOf("\"", pattern2.length()) - pattern2.length());
			}
			dataFile.close();
			if ( !nameList.isEmpty() )
				lineEditSets->setText(nameList.join(" "));
			setActive(false);
		} else
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open data file '%1' for reading").arg(dataFilePath));
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/LastDataFilePath", dataFilePath);
	}
}

void ROMAlyzer::exportToDataFile()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::exportToDataFile()");
#endif

	QString storedPath;
	if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + m_settingsKey + "/LastDataFilePath") )
		storedPath = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/LastDataFilePath").toString();
	QString dataFilePath = QFileDialog::getSaveFileName(this, tr("Choose data file to export to"), storedPath, tr("Data files (*.dat)") + ";;" + tr("All files (*)"), 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !dataFilePath.isNull() ) {
		QFile dataFile(dataFilePath);
		QFileInfo fi(dataFilePath);
		if ( dataFile.open(QIODevice::WriteOnly | QIODevice::Text) ) {
			setActive(true);
			progressBar->setRange(0, qmc2MainWindow->treeWidgetMachineList->topLevelItemCount());
			labelStatus->setText(tr("Exporting"));
			QTextStream ts(&dataFile);
			ts << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
			ts << "<!DOCTYPE datafile PUBLIC \"-//Logiqx//DTD ROM Management Datafile//EN\" \"http://www.logiqx.com/Dats/datafile.dtd\">\n\n";
			ts << "<datafile>\n";
			ts << "\t<header>\n";
			ts << "\t\t<name>" << fi.completeBaseName() << "</name>\n";
			ts << "\t\t<description>" << fi.completeBaseName() << "</description>\n";
			ts << "\t\t<category>FIXDATFILE</category>\n";
			ts << "\t\t<version>" << QDateTime::currentDateTime().toString("MM/dd/yy hh:mm:ss") << "</version>\n";
			ts << "\t\t<date>" << QDateTime::currentDateTime().toString("yyyy-MM-dd") << "</date>\n";
			ts << "\t\t<author>auto-create</author>\n";
			ts << "\t\t<email></email>\n";
			ts << "\t\t<homepage></homepage>\n";
			ts << "\t\t<url></url>\n";
			ts << "\t\t<comment>" << tr("Created by QMC2 v%1").arg(XSTR(QMC2_VERSION)) << "</comment>\n";
			ts << "\t</header>\n";
			QString mainEntityName = "machine";
			for (int i = 0; i < treeWidgetChecksums->topLevelItemCount(); i++) {
				if ( i % QMC2_ROMALYZER_EXPORT_RESPONSE ) {
					progressBar->setValue(i);
					qApp->processEvents();
				}
				QTreeWidgetItem *item = treeWidgetChecksums->topLevelItem(i);
				QString name = item->text(QMC2_ROMALYZER_COLUMN_GAME).split(" ", QString::SkipEmptyParts)[0];
				if ( analyzerBadSets.contains(name) ) {
					QString sourcefile, isbios, cloneof, romof, sampleof;
					QByteArray xmlDocument(ROMAlyzer::getXmlData(name, true).toUtf8());
					QBuffer xmlQueryBuffer(&xmlDocument);
					xmlQueryBuffer.open(QIODevice::ReadOnly);
					QXmlQuery xmlQuery(QXmlQuery::XQuery10);
					xmlQuery.bindVariable("xmlDocument", &xmlQueryBuffer);
					xmlQuery.setQuery(QString("doc($xmlDocument)//%1/@sourcefile/string()").arg(mainEntityName));
					xmlQuery.evaluateTo(&sourcefile);
					sourcefile = sourcefile.trimmed();
					xmlQuery.setQuery(QString("doc($xmlDocument)//%1/@isbios/string()").arg(mainEntityName));
					xmlQuery.evaluateTo(&isbios);
					isbios = isbios.trimmed();
					xmlQuery.setQuery(QString("doc($xmlDocument)//%1/@cloneof/string()").arg(mainEntityName));
					xmlQuery.evaluateTo(&cloneof);
					cloneof = cloneof.trimmed();
					xmlQuery.setQuery(QString("doc($xmlDocument)//%1/@romof/string()").arg(mainEntityName));
					xmlQuery.evaluateTo(&romof);
					romof = romof.trimmed();
					xmlQuery.setQuery(QString("doc($xmlDocument)//%1/@sampleof/string()").arg(mainEntityName));
					xmlQuery.evaluateTo(&sampleof);
					sampleof = sampleof.trimmed();
					ts << "\t<machine name=\"" << name << "\"";
					if ( !sourcefile.isEmpty() )
						ts << " sourcefile=\"" << sourcefile << "\"";
					if ( !isbios.isEmpty() && isbios != "no" )
						ts << " isbios=\"" << isbios << "\"";
					if ( !cloneof.isEmpty() )
						ts << " cloneof=\"" << cloneof << "\"";
					if ( !romof.isEmpty() )
						ts << " romof=\"" << romof << "\"";
					if ( !sampleof.isEmpty() )
						ts << " sampleof=\"" << sampleof << "\"";
					ts << ">\n";
					QString description, year, manufacturer;
					xmlQuery.setQuery(QString("doc($xmlDocument)//%1/description/string()").arg(mainEntityName));
					xmlQuery.evaluateTo(&description);
					description = description.trimmed();
					ts << "\t\t<description>" << description << "</description>\n";
					xmlQuery.setQuery(QString("doc($xmlDocument)//%1/year/string()").arg(mainEntityName));
					xmlQuery.evaluateTo(&year);
					year = year.trimmed();
					ts << "\t\t<year>" << year << "</year>\n";
					xmlQuery.setQuery(QString("doc($xmlDocument)//%1/manufacturer/string()").arg(mainEntityName));
					xmlQuery.evaluateTo(&manufacturer);
					manufacturer = manufacturer.trimmed();
					ts << "\t\t<manufacturer>" << manufacturer << "</manufacturer>\n";
					for (int j = 0; j < item->childCount(); j++) {
						QTreeWidgetItem *childItem = item->child(j);
						QString filestatus = childItem->text(QMC2_ROMALYZER_COLUMN_FILESTATUS);
						if ( filestatus == tr("not found") || (filestatus.toUpper() != filestatus && filestatus != tr("no dump")) ) {
							QString type = childItem->text(QMC2_ROMALYZER_COLUMN_TYPE);
							QString filename = childItem->text(QMC2_ROMALYZER_COLUMN_GAME);
							QString size = childItem->text(QMC2_ROMALYZER_COLUMN_SIZE);
							QString crc = childItem->text(QMC2_ROMALYZER_COLUMN_CRC);
							QString sha1 = childItem->text(QMC2_ROMALYZER_COLUMN_SHA1);
							QString merge = childItem->text(QMC2_ROMALYZER_COLUMN_MERGE);
							if ( type == "ROM" ) {
								ts << "\t\t<rom name=\"" << filename << "\"";
								if ( !merge.isEmpty() )
									ts << " merge=\"" << merge << "\"";
							       	if ( !size.isEmpty() )
									ts << " size=\"" << size << "\"";
								if ( !crc.isEmpty() )
									ts << " crc=\"" << crc << "\"";
								if ( !sha1.isEmpty() )
									ts << " sha1=\"" << sha1 << "\"";
								ts << "/>\n";
							} else {
								ts << "\t\t<disk name=\"" << filename << "\"";
								if ( !merge.isEmpty() )
									ts << " merge=\"" << merge << "\"";
								if ( !sha1.isEmpty() )
									ts << " sha1=\"" << sha1 << "\"";
								ts << "/>\n";
							}
						}
					}
					ts << "\t</machine>\n";
				}
			}
			ts << "</datafile>\n";
			dataFile.close();
			progressBar->reset();
			labelStatus->setText(tr("Idle"));
			setActive(false);
		} else
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open data file '%1' for writing").arg(dataFilePath));
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/LastDataFilePath", dataFilePath);
	}
}

void ROMAlyzer::copyToClipboard(bool onlyBadOrMissing)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::copyToClipboard(bool onlyBadOrMissing = %1)").arg(onlyBadOrMissing));
#endif

	static QStringList excludedColumnsBadOrMissing = QStringList() << tr("Merge") << tr("Emu status") << tr("File status");

	QList<QTreeWidgetItem *> il = treeWidgetChecksums->selectedItems();
	if ( !il.isEmpty() ) {
		QHeaderView *header = treeWidgetChecksums->header();
		QTreeWidgetItem *headerItem = treeWidgetChecksums->headerItem();
		QTreeWidgetItem *item = il[0];
		QStringList columnTitles;
		QList<int> columnWidths;
		QList<QStringList> rows;
		QStringList firstRow;
		for (int i = 0; i < treeWidgetChecksums->columnCount(); i++) {
			if ( !treeWidgetChecksums->isColumnHidden(header->logicalIndex(i)) ) {
				QString h = headerItem->text(header->logicalIndex(i));
				if ( onlyBadOrMissing )
					if ( excludedColumnsBadOrMissing.contains(h) )
						continue;
				columnTitles << h;
				QString t = item->text(header->logicalIndex(i));
				if ( i == 0 )
					if ( onlyBadOrMissing )
						t = t.split(" ", QString::SkipEmptyParts)[0];
				firstRow << t;
				columnWidths.append(QMC2_MAX(t.length(), h.length()));
			}
		}
		rows.append(firstRow);
		for (int i = 0; i < item->childCount(); i++) {
			QTreeWidgetItem *childItem = item->child(i);
			if ( onlyBadOrMissing ) {
				QString filestatus = childItem->text(QMC2_ROMALYZER_COLUMN_FILESTATUS);
				if ( !(filestatus == tr("not found") || (filestatus.toUpper() != filestatus && filestatus != tr("no dump"))) )
					continue;
			}
			QStringList row;
			int columnCount = 0;
			for (int j = 0; j < treeWidgetChecksums->columnCount(); j++) {
				if ( !treeWidgetChecksums->isColumnHidden(header->logicalIndex(j)) ) {
					if ( onlyBadOrMissing ) {
						if ( excludedColumnsBadOrMissing.contains(headerItem->text(header->logicalIndex(j))) )
							continue;
					}
					QString t = childItem->text(header->logicalIndex(j));
					if ( j == 0 )
						t.prepend("\\ ");
					row << t;
					if ( columnWidths[columnCount] < t.length() )
						columnWidths[columnCount] = t.length();
				}
				columnCount++;
			}
			rows.append(row);
		}

		QString cbText, cbLine;
		QRegExp removeTrailingSpacesRx("\\s+$");
		for (int i = 0; i < columnTitles.count(); i++) {
			if ( i == columnTitles.count() - 1 )
				cbLine += columnTitles[i].leftJustified(columnWidths[i], ' ');
			else
				cbLine += columnTitles[i].leftJustified(columnWidths[i] + 2, ' ');
		}
		cbLine.replace(removeTrailingSpacesRx, QString());
		cbText += cbLine + "\n";
		cbLine.clear();
		for (int i = 0; i < columnTitles.count(); i++) {
			if ( i == columnTitles.count() - 1 )
				cbLine += QString().fill('-', columnWidths[i]);
			else
				cbLine += QString().fill('-', columnWidths[i]) + "  ";
		}
		cbLine.replace(removeTrailingSpacesRx, QString());
		cbText += cbLine + "\n";
		foreach (QStringList row, rows) {
			cbLine.clear();
			for (int i = 0; i < row.count(); i++) {
				if ( i == row.count() - 1 )
					cbLine += row[i].leftJustified(columnWidths[i], ' ');
				else
					cbLine += row[i].leftJustified(columnWidths[i] + 2, ' ');
			}
			cbLine.replace(removeTrailingSpacesRx, QString());
			cbText += cbLine + "\n";
		}

		qApp->clipboard()->setText(cbText);
	}
}

void ROMAlyzer::copyBadToClipboard()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::copyBadToClipboard()");
#endif

	copyToClipboard(true);
}

void ROMAlyzer::on_treeWidgetChecksumWizardSearchResult_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_treeWidgetChecksumWizardSearchResult_itemSelectionChanged()");
#endif

	QList<QTreeWidgetItem *> il = treeWidgetChecksumWizardSearchResult->selectedItems();
	pushButtonChecksumWizardAnalyzeSelectedSets->setEnabled(!il.isEmpty());
	wizardSelectedSets.clear();
	int selectedGoodSets = 0;
	int selectedBadSets = 0;
	foreach (QTreeWidgetItem *item, il) {
		wizardSelectedSets << item->text(QMC2_ROMALYZER_CSWIZ_COLUMN_ID);
		if ( item->text(QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS) == tr("good") )
			selectedGoodSets++;
		if ( item->text(QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS) == tr("bad") ) {
			if ( !item->icon(QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS).isNull() )
				selectedGoodSets++;
			selectedBadSets++;
		}
	}
	wizardSelectedSets.removeDuplicates();
	pushButtonChecksumWizardRepairBadSets->setEnabled(selectedBadSets > 0 && selectedGoodSets > 0);
}

void ROMAlyzer::on_pushButtonChecksumWizardAnalyzeSelectedSets_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_pushButtonChecksumWizardAnalyzeSelectedSets_clicked()");
#endif

	switch ( mode() ) {
		case QMC2_ROMALYZER_MODE_SOFTWARE: {
				QStringList lists, sets;
				foreach (QString setKey, wizardSelectedSets) {
					QStringList setKeyTokens = setKey.split(":", QString::SkipEmptyParts);
					if ( setKeyTokens.count() < 2 )
						continue;
					lists << setKeyTokens[0];
					sets << setKeyTokens[1];
				}
				lineEditSoftwareLists->setText(lists.join(" "));
				lineEditSets->setText(sets.join(" "));
			}
			break;
		case QMC2_ROMALYZER_MODE_SYSTEM:
		default:
			lineEditSets->setText(wizardSelectedSets.join(" "));
			break;
	}
	wizardSearch = true;
	pushButtonAnalyze->animateClick();
}

// reads all files in the ZIP 'fileName' and maps the data:
// - CRC codes are mapped to their data in 'dataMap'
// - CRC codes are mapped to their file names in 'fileMap'
// - existing filenames will be appended to fileList (if != NULL)
// - will also read files with incorrect CRCs (compared to their header CRCs)
bool ROMAlyzer::readAllZipData(QString fileName, QMap<QString, QByteArray> *dataMap, QMap<QString, QString> *fileMap, QStringList *fileList)
{
	bool success = true;
	unzFile zipFile = unzOpen(fileName.toUtf8().constData());

	if ( zipFile ) {
  		char ioBuffer[QMC2_ROMALYZER_ZIP_BUFFER_SIZE];
		unz_file_info zipInfo;
		do {
			if ( unzGetCurrentFileInfo(zipFile, &zipInfo, ioBuffer, QMC2_ROMALYZER_ZIP_BUFFER_SIZE, 0, 0, 0, 0) == UNZ_OK ) {
				QString fn = QString((const char *)ioBuffer);
				fileMap->insert(crcToString(zipInfo.crc), fn);
				if ( fileList )
					fileList->append(fn);
				if ( unzOpenCurrentFile(zipFile) == UNZ_OK ) {
					qint64 len;
					QByteArray fileData;
					progressBarFileIO->setRange(0, zipInfo.uncompressed_size);
					progressBarFileIO->reset();
					while ( (len = unzReadCurrentFile(zipFile, ioBuffer, QMC2_ROMALYZER_ZIP_BUFFER_SIZE)) > 0 ) {
						QByteArray readData((const char *)ioBuffer, len);
						fileData += readData;
						progressBarFileIO->setValue(fileData.length());
						progressBarFileIO->update();
						progressBar->update();
						if ( fileData.length() % QMC2_128K == 0 || (uLong)fileData.length() == zipInfo.uncompressed_size ) qApp->processEvents();
					}
					dataMap->insert(crcToString(zipInfo.crc), fileData);
					unzCloseCurrentFile(zipFile);
				}
			}
		} while ( unzGoToNextFile(zipFile) == UNZ_OK );
		unzClose(zipFile);
	} else
		success = false;

	progressBarFileIO->reset();
	return success;
}

// reads the file 'fileName' with the expected CRC 'crc' and returns its data in 'data'
bool ROMAlyzer::readFileData(QString fileName, QString crc, QByteArray *data)
{
	QFile romFile(fileName);
	data->clear();
	if ( romFile.open(QIODevice::ReadOnly) ) {
		quint64 sizeLeft = romFile.size();
  		char ioBuffer[QMC2_ROMALYZER_FILE_BUFFER_SIZE];
		int len = 0;
		progressBarFileIO->setRange(0, sizeLeft);
		progressBarFileIO->reset();
		while ( (len = romFile.read(ioBuffer, QMC2_ROMALYZER_FILE_BUFFER_SIZE)) > 0 ) {
			QByteArray readData((const char *)ioBuffer, len);
			data->append(readData);
			sizeLeft -= len;
			progressBarFileIO->setValue(romFile.size() - sizeLeft);
			progressBarFileIO->update();
			if ( data->length() % QMC2_128K == 0 || data->length() == romFile.size() ) qApp->processEvents();
		}
		romFile.close();
		progressBarFileIO->reset();
		ulong calculatedCrc = crc32(0, NULL, 0);
		calculatedCrc = crc32(calculatedCrc, (const Bytef *)data->data(), data->size());
		return ( crcToString(calculatedCrc) == crc );
	} else
		return false;
}

// reads the file with the CRC 'crc' in the 7z archive 'fileName' and returns its data in 'data'
bool ROMAlyzer::readSevenZipFileData(QString fileName, QString crc, QByteArray *data)
{
	SevenZipFile sevenZipFile(fileName);
	if ( sevenZipFile.open() ) {
		int index = sevenZipFile.indexOfCrc(crc);
		if ( index >= 0 ) {
			SevenZipMetaData metaData = sevenZipFile.itemList()[index];
			qApp->processEvents();
			quint64 readLength = sevenZipFile.read(index, data);
			qApp->processEvents();
			if ( sevenZipFile.hasError() )
				return false;
			if ( readLength != metaData.size() )
				return false;
			ulong ulCrc = crc32(0, NULL, 0);
			ulCrc = crc32(ulCrc, (const Bytef *)data->data(), data->size());
			if ( crcToString(ulCrc) != crc )
				return false;
			return true;
		} else
			return false;
	} else
		return false;
}

// reads the file with the CRC 'crc' in the ZIP 'fileName' and returns its data in 'data'
bool ROMAlyzer::readZipFileData(QString fileName, QString crc, QByteArray *data)
{
	bool success = true;
	unzFile zipFile = unzOpen(fileName.toUtf8().constData());

	if ( zipFile ) {
  		char ioBuffer[QMC2_ROMALYZER_ZIP_BUFFER_SIZE];

		// identify file by CRC
		unz_file_info zipInfo;
		QMap<uLong, QString> crcIdentMap;
		uLong ulCRC = crc.toULong(0, 16);
		do {
			if ( unzGetCurrentFileInfo(zipFile, &zipInfo, ioBuffer, QMC2_ROMALYZER_ZIP_BUFFER_SIZE, 0, 0, 0, 0) == UNZ_OK )
				crcIdentMap[zipInfo.crc] = QString((const char *)ioBuffer);
		} while ( unzGoToNextFile(zipFile) == UNZ_OK && !crcIdentMap.contains(ulCRC) );
		unzGoToFirstFile(zipFile);
		if ( crcIdentMap.contains(ulCRC) ) {
			QString fn = crcIdentMap[ulCRC];
			if ( unzLocateFile(zipFile, fn.toUtf8().constData(), 2) == UNZ_OK ) { // NOT case-sensitive filename compare!
				if ( unzOpenCurrentFile(zipFile) == UNZ_OK ) {
					qint64 len;
					progressBarFileIO->setRange(0, zipInfo.uncompressed_size);
					progressBarFileIO->reset();
					while ( (len = unzReadCurrentFile(zipFile, ioBuffer, QMC2_ROMALYZER_ZIP_BUFFER_SIZE)) > 0 ) {
						QByteArray readData((const char *)ioBuffer, len);
						*data += readData;
						progressBarFileIO->setValue(data->length());
						progressBarFileIO->update();
						progressBar->update();
						if ( data->length() % QMC2_128K == 0 || (uLong)data->length() == zipInfo.uncompressed_size ) qApp->processEvents();
					}
					unzCloseCurrentFile(zipFile);
				} else
					success = false;
			} else
				success = false;
		} else
			success = false;

		unzClose(zipFile);
	} else
		success = false;

	progressBarFileIO->reset();
	return success;
}

// creates the directory 'dirName'
// and stores the data found in 'fileDataMap' into individual files
// - 'fileDataMap' maps file names to their data
bool ROMAlyzer::writeAllFileData(QString dirName, QMap<QString, QByteArray> *fileDataMap, bool writeLog, QProgressBar *pBar)
{
	bool success = true;

	if ( pBar ) {
		pBar->setRange(0, fileDataMap->count());
		pBar->reset();
	}

	QDir d(dirName);
	if ( !d.exists() )
		success = d.mkdir(dirName);

	QMapIterator<QString, QByteArray> it(*fileDataMap);
	int count = 0;
	while ( it.hasNext() && success ) {
		if ( pBar )
			pBar->setValue(++count);
		it.next();
		QString file = dirName + "/" + it.key();
		QFile f(file);
		QByteArray data = it.value();
		if ( writeLog )
			log(tr("set rewriter: writing '%1' (size: %2)").arg(file).arg(humanReadable(data.length())));
		createBackup(file);
		if ( f.open(QIODevice::WriteOnly) ) {
			quint64 bytesWritten = 0;
			progressBarFileIO->setInvertedAppearance(true);
			progressBarFileIO->setRange(0, data.length());
			progressBarFileIO->reset();
			qApp->processEvents();
			while ( bytesWritten < (quint64)data.length() && success ) {
				quint64 bufferLength = QMC2_ZIP_BUFFER_SIZE;
				if ( bytesWritten + bufferLength > (quint64)data.length() )
					bufferLength = data.length() - bytesWritten;
				qint64 len = f.write(data.mid(bytesWritten, bufferLength));
				success = (len >= 0);
				if ( success ) {
					bytesWritten += len;
					progressBarFileIO->setValue(bytesWritten);
					progressBarFileIO->update();
					progressBar->update();
					if ( bytesWritten % QMC2_128K == 0 || bytesWritten == (quint64)data.length() ) qApp->processEvents();
				} else if ( writeLog )
					log(tr("set rewriter: WARNING: failed to write '%1'").arg(file));
			}
			f.close();
		} else
			success = false;
	}

	if ( pBar )
		pBar->reset();
	progressBarFileIO->reset();
	progressBarFileIO->setInvertedAppearance(false);
	return success;
}

// creates the new ZIP 'fileName'
// and stores the data found in 'fileDataMap' into the ZIP:
// - 'fileDataMap' maps file names to their data
bool ROMAlyzer::writeAllZipData(QString fileName, QMap<QString, QByteArray> *fileDataMap, bool writeLog, QProgressBar *pBar)
{
	bool success = true;

	QFile f(fileName);
	if ( f.exists() )
		success = createBackup(fileName) && f.remove();

	zipFile zip = NULL;
	if ( success )
		zip = zipOpen(fileName.toUtf8().constData(), APPEND_STATUS_CREATE);

	if ( zip ) {
		zip_fileinfo zipInfo;
		QDateTime cDT = QDateTime::currentDateTime();
		zipInfo.tmz_date.tm_sec = cDT.time().second();
		zipInfo.tmz_date.tm_min = cDT.time().minute();
		zipInfo.tmz_date.tm_hour = cDT.time().hour();
		zipInfo.tmz_date.tm_mday = cDT.date().day();
		zipInfo.tmz_date.tm_mon = cDT.date().month() - 1;
		zipInfo.tmz_date.tm_year = cDT.date().year();
		zipInfo.dosDate = zipInfo.internal_fa = zipInfo.external_fa = 0;
		if ( pBar ) {
			pBar->setRange(0, fileDataMap->count());
			pBar->reset();
		}
		QMapIterator<QString, QByteArray> it(*fileDataMap);
		int count = 0;
		while ( it.hasNext() && success ) {
			if ( pBar ) {
				pBar->setValue(++count);
				pBar->setFormat(QString("%1 / %2").arg(count).arg(fileDataMap->count()));
			}
			it.next();
			QString file = it.key();
			QByteArray data = it.value();
			if ( writeLog )
				log(tr("set rewriter: deflating '%1' (uncompressed size: %2)").arg(file).arg(humanReadable(data.length())));
			if ( zipOpenNewFileInZip(zip, file.toUtf8().constData(), &zipInfo, file.toUtf8().constData(), file.length(), 0, 0, 0, Z_DEFLATED, spinBoxSetRewriterZipLevel->value()) == ZIP_OK ) {
				quint64 bytesWritten = 0;
				progressBarFileIO->setInvertedAppearance(true);
				progressBarFileIO->setRange(0, data.length());
				progressBarFileIO->reset();
				qApp->processEvents();
				while ( bytesWritten < (quint64)data.length() && success ) {
					quint64 bufferLength = QMC2_ZIP_BUFFER_SIZE;
					if ( bytesWritten + bufferLength > (quint64)data.length() )
						bufferLength = data.length() - bytesWritten;
					QByteArray writeBuffer = data.mid(bytesWritten, bufferLength);
					success = (zipWriteInFileInZip(zip, (const void *)writeBuffer.data(), bufferLength) == ZIP_OK);
					if ( success ) {
						bytesWritten += bufferLength;
						progressBarFileIO->setValue(bytesWritten);
						progressBarFileIO->update();
						progressBar->update();
						if ( bytesWritten % QMC2_128K == 0 || bytesWritten == (quint64)data.length() ) qApp->processEvents();
					} else if ( writeLog )
						log(tr("set rewriter: WARNING: failed to deflate '%1'").arg(file));
				}
				zipCloseFileInZip(zip);
			} else
				success = false;
		}
		if ( checkBoxAddZipComment->isChecked() )
			zipClose(zip, tr("Created by QMC2 v%1 (%2)").arg(XSTR(QMC2_VERSION)).arg(cDT.toString(Qt::SystemLocaleShortDate)).toUtf8().constData());
		else
			zipClose(zip, "");
	} else
		success = false;

	if ( pBar ) {
		pBar->reset();
		pBar->setFormat(QString());
	}

	progressBarFileIO->reset();
	progressBarFileIO->setInvertedAppearance(false);
	return success;
}

void ROMAlyzer::on_pushButtonChecksumWizardRepairBadSets_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_pushButtonChecksumWizardRepairBadSets_clicked()");
#endif

	QList<QTreeWidgetItem *> badList;
	QTreeWidgetItem *goodItem = NULL;
	foreach (QTreeWidgetItem *item, treeWidgetChecksumWizardSearchResult->selectedItems()) {
		if ( item->text(QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS) == tr("bad") ) {
			badList << item;
			if ( !item->icon(QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS).isNull() )
				goodItem = item;
		} else if ( goodItem == NULL && item->text(QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS) == tr("good") )
			goodItem = item;
	}
	int numBadSets = badList.count();

	// this shouldn't happen, but you never know :)
	if ( numBadSets <= 0  || goodItem == NULL)
		return;

	// only one repair at a time!
	if ( !pushButtonChecksumWizardRepairBadSets->isEnabled() )
		return;
	pushButtonChecksumWizardRepairBadSets->setEnabled(false);

	log(tr("check-sum wizard: repairing %n bad set(s)", "", numBadSets));
	if ( goodItem != NULL ) {
		QString sourceType = goodItem->text(QMC2_ROMALYZER_CSWIZ_COLUMN_TYPE);
		QString sourceFile = goodItem->text(QMC2_ROMALYZER_CSWIZ_COLUMN_FILENAME);
		QString sourceCRC  = goodItem->whatsThis(QMC2_ROMALYZER_CSWIZ_COLUMN_FILENAME); // we need the CRC for file identification in ZIPs
		QString sourcePath = goodItem->text(QMC2_ROMALYZER_CSWIZ_COLUMN_PATH);
		log(tr("check-sum wizard: using %1 file '%2' from '%3' as repro template").arg(sourceType).arg(sourceFile).arg(sourcePath));

		bool loadOkay = true;
		QByteArray templateData;
		QString fn;
		if ( sourceType == tr("ROM") ) {
			// load ROM image
			if ( sourcePath.indexOf(QRegExp("^.*\\.[zZ][iI][pP]$")) == 0 ) {
				// file from a ZIP archive
				if ( !readZipFileData(sourcePath, sourceCRC, &templateData) ) {
					log(tr("check-sum wizard: FATAL: can't open ZIP archive '%1' for reading").arg(sourcePath));
					loadOkay = false;
				}
			} else if ( sourcePath.indexOf(QRegExp("^.*\\.7[zZ]$")) == 0 ) {
				// file from a 7Z archive
				if ( !readSevenZipFileData(sourcePath, sourceCRC, &templateData) ) {
					log(tr("check-sum wizard: FATAL: can't load repro template data from '%1' with expected CRC '%2'").arg(sourcePath).arg(sourceCRC));
					loadOkay = false;
				}
			} else {
				// read a regular file
				if ( !readFileData(sourcePath, sourceCRC, &templateData) ) {
					log(tr("check-sum wizard: FATAL: can't load repro template data from '%1' with expected CRC '%2'").arg(sourcePath).arg(sourceCRC));
					loadOkay = false;
				}
			}
		} else {
			// FIXME: no support for CHDs yet (probably not necessary)
			log(tr("check-sum wizard: sorry, no support for CHD files yet"));
			loadOkay = false;
		}

		if ( loadOkay ) {
			progressBar->setRange(0, badList.count());
			progressBar->reset();
			quint32 counter = 0;
			foreach (QTreeWidgetItem *badItem, badList) {
				bool saveOkay = true;
				QString targetType = badItem->text(QMC2_ROMALYZER_CSWIZ_COLUMN_TYPE);
				QString targetFile = badItem->text(QMC2_ROMALYZER_CSWIZ_COLUMN_FILENAME);
				QString targetPath = badItem->text(QMC2_ROMALYZER_CSWIZ_COLUMN_PATH);
				QString badSetName = badItem->text(QMC2_ROMALYZER_CSWIZ_COLUMN_ID);
				if ( !badItem->icon(QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS).isNull() ) {
					QString myRomPath;
					if ( qmc2Config->contains(QMC2_EMULATOR_PREFIX + "Configuration/Global/rompath") )
						myRomPath = qmc2Config->value(QMC2_EMULATOR_PREFIX + "Configuration/Global/rompath").toString();
					else
						myRomPath = "roms";
					if ( groupBoxSetRewriter->isChecked() && checkBoxSetRewriterUseAdditionalRomPath->isChecked() && !lineEditSetRewriterAdditionalRomPath->text().isEmpty() )
						myRomPath.prepend(lineEditSetRewriterAdditionalRomPath->text() + ";");
					targetPath = QDir::cleanPath(myRomPath.split(";", QString::SkipEmptyParts).first()) + "/" + badSetName + ".zip";
				}
				labelStatus->setText(tr("Repairing set '%1' - %2").arg(badSetName).arg(badList.count() - counter));
				log(tr("check-sum wizard: repairing %1 file '%2' in '%3' from repro template").arg(targetType).arg(targetFile).arg(targetPath));
				qApp->processEvents();

				if ( targetType == tr("ROM") ) {
					// save ROM image
					if ( targetPath.indexOf(QRegExp("^.*\\.[zZ][iI][pP]$")) == 0 ) {
						bool copyTargetData = false;
						QMap <QString, QByteArray> targetDataMap;
						QMap <QString, QString> targetFileMap;
						QFile f(targetPath);
						int appendType = APPEND_STATUS_ADDINZIP;
						if ( f.exists() ) {
							log(tr("check-sum wizard: target ZIP exists, loading complete data and structure"));
							QStringList fileNameList;
							if ( readAllZipData(targetPath, &targetDataMap, &targetFileMap, &fileNameList) ) {
								log(tr("check-sum wizard: target ZIP successfully loaded"));
								bool crcExists = targetDataMap.contains(sourceCRC);
								bool fileExists = fileNameList.contains(targetFile);
								if ( crcExists || fileExists ) {
									if ( crcExists ) {
										log(tr("check-sum wizard: an entry with the CRC '%1' already exists, recreating the ZIP from scratch to replace the bad file").arg(sourceCRC));
										targetDataMap.remove(sourceCRC);
										targetFileMap.remove(sourceCRC);
									}
									if ( fileExists ) {
										log(tr("check-sum wizard: an entry with the name '%1' already exists, recreating the ZIP from scratch to replace the bad file").arg(targetFile));
										targetDataMap.remove(targetDataMap.key(targetFile.toUtf8()));
										targetFileMap.remove(targetFileMap.key(targetFile.toUtf8()));
									}
									// we need to make sure that only 'valid' (aka 'accepted') CRCs are reproduced
									QStringList acceptedCRCs;
									QList<QTreeWidgetItem *> itemList = treeWidgetChecksums->findItems(badSetName + " ", Qt::MatchStartsWith, QMC2_ROMALYZER_COLUMN_GAME);
									if ( !itemList.isEmpty() ) {
										QTreeWidgetItem *item = itemList[0];
										for (int i = 0; i < item->childCount(); i++) {
											QString crc = item->child(i)->text(QMC2_ROMALYZER_COLUMN_CRC);
											if ( !crc.isEmpty() )
												acceptedCRCs << crc;
										}
									}
									QMapIterator<QString, QByteArray> it(targetDataMap);
									while ( it.hasNext() ) {
										it.next();
										QString crc = it.key();
										if ( !acceptedCRCs.contains(crc) ) {
											targetDataMap.remove(crc);
											targetFileMap.remove(crc);
										}
									}
									if ( createBackup(targetPath) ) {
										copyTargetData = true;
										appendType = APPEND_STATUS_CREATE;
									} else {
										log(tr("check-sum wizard: FATAL: backup creation failed, aborting"));
										saveOkay = false;
									}
								} else {
									if ( createBackup(targetPath) ) {
										log(tr("check-sum wizard: no entry with the CRC '%1' or name '%2' was found, adding the missing file to the existing ZIP").arg(sourceCRC).arg(targetFile));
									} else {
										log(tr("check-sum wizard: FATAL: backup creation failed, aborting"));
										saveOkay = false;
									}
								}
							} else {
								log(tr("check-sum wizard: FATAL: failed to load target ZIP, aborting"));
								saveOkay = false;
							}
						} else {
							appendType = APPEND_STATUS_CREATE;
							log(tr("check-sum wizard: the target ZIP does not exist, creating a new ZIP with just the missing file"));
						}

						zipFile zip = NULL;
					        if ( saveOkay )
							zip = zipOpen(targetPath.toUtf8().constData(), appendType);

						if ( zip ) {
							if ( !copyTargetData ) {
								targetDataMap.clear();
								targetFileMap.clear();
							}

							targetDataMap[sourceCRC] = templateData;
							targetFileMap[sourceCRC] = targetFile;

							zip_fileinfo zipInfo;
							QDateTime cDT = QDateTime::currentDateTime();
							zipInfo.tmz_date.tm_sec = cDT.time().second();
							zipInfo.tmz_date.tm_min = cDT.time().minute();
							zipInfo.tmz_date.tm_hour = cDT.time().hour();
							zipInfo.tmz_date.tm_mday = cDT.date().day();
							zipInfo.tmz_date.tm_mon = cDT.date().month() - 1;
							zipInfo.tmz_date.tm_year = cDT.date().year();
							zipInfo.dosDate = zipInfo.internal_fa = zipInfo.external_fa = 0;

							QMapIterator<QString, QByteArray> it(targetDataMap);
							while ( it.hasNext() ) {
								it.next();
								QString tFile = targetFileMap[it.key()];
								QByteArray tData = it.value();
								if ( zipOpenNewFileInZip(zip, tFile.toUtf8().constData(), &zipInfo, tFile.toUtf8().constData(), tFile.length(), 0, 0, 0, Z_DEFLATED, Z_DEFAULT_COMPRESSION) == ZIP_OK ) {
									quint64 bytesWritten = 0;
									progressBarFileIO->setRange(0, tData.length());
									progressBarFileIO->reset();
									while ( bytesWritten < (quint64)tData.length() && saveOkay ) {
										quint64 bufferLength = QMC2_ZIP_BUFFER_SIZE;
										if ( bytesWritten + bufferLength > (quint64)tData.length() )
											bufferLength = tData.length() - bytesWritten;
										QByteArray writeBuffer = tData.mid(bytesWritten, bufferLength);
										saveOkay = (zipWriteInFileInZip(zip, (const void *)writeBuffer.data(), bufferLength) == ZIP_OK);
										if ( saveOkay )
											bytesWritten += bufferLength;
										progressBarFileIO->setValue(bytesWritten);
										progressBarFileIO->update();
									}
									progressBarFileIO->reset();
									qApp->processEvents();
									zipCloseFileInZip(zip);
								} else {
									log(tr("check-sum wizard: FATAL: can't open file '%1' in ZIP archive '%2' for writing").arg(targetFile).arg(targetPath));
									saveOkay = false;
								}
							}
							if ( saveOkay )
								if ( checkBoxAddZipComment->isChecked() ) {
									if ( appendType == APPEND_STATUS_ADDINZIP )
										zipClose(zip, tr("Fixed by QMC2 v%1 (%2)").arg(XSTR(QMC2_VERSION)).arg(cDT.toString(Qt::SystemLocaleShortDate)).toUtf8().constData());
									else
										zipClose(zip, tr("Created by QMC2 v%1 (%2)").arg(XSTR(QMC2_VERSION)).arg(cDT.toString(Qt::SystemLocaleShortDate)).toUtf8().constData());
								} else
									zipClose(zip, "");
							else
								zipClose(zip, 0);
						} else {
							log(tr("check-sum wizard: FATAL: can't open ZIP archive '%1' for writing").arg(targetPath));
							saveOkay = false;
						}
					} else {
						// FIXME: no support for regular files yet
						log(tr("check-sum wizard: sorry, no support for regular files yet"));
						saveOkay = false;
					}
				} else {
					// FIXME: no support for CHDs yet (probably not necessary)
					log(tr("check-sum wizard: sorry, no support for CHD files yet"));
					saveOkay = false;
				}

				if ( saveOkay ) {
					badItem->setText(QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS, tr("repaired"));
					badItem->setForeground(QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS, QBrush(QColor(0, 255, 0))); // green
					log(tr("check-sum wizard: successfully repaired %1 file '%2' in '%3' from repro template").arg(targetType).arg(targetFile).arg(targetPath));
				} else {
					badItem->setText(QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS, tr("repair failed"));
					badItem->setForeground(QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS, QBrush(QColor(255, 0, 0))); // red
					log(tr("check-sum wizard: FATAL: failed to repair %1 file '%2' in '%3' from repro template").arg(targetType).arg(targetFile).arg(targetPath));
				}

				progressBar->setValue(++counter);
				qApp->processEvents();
			}
			labelStatus->setText(tr("Idle"));
			progressBar->reset();
		}
	} else
		log(tr("check-sum wizard: FATAL: can't find any good set"));

	log(tr("check-sum wizard: done (repairing %n bad set(s))", "", numBadSets));
	on_treeWidgetChecksumWizardSearchResult_itemSelectionChanged();
}

void ROMAlyzer::on_treeWidgetChecksums_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_treeWidgetChecksums_customContextMenuRequested(const QPoint &p = ...)");
#endif

	if ( active() )
		return;

	QTreeWidgetItem *item = treeWidgetChecksums->itemAt(p);
	if ( item ) {
		if ( item->parent() != NULL ) {
			currentFilesSHA1Checksum = item->text(QMC2_ROMALYZER_COLUMN_SHA1);
			currentFilesCrcChecksum = item->text(QMC2_ROMALYZER_COLUMN_CRC);
			currentFilesSize = item->text(QMC2_ROMALYZER_COLUMN_SIZE).toULongLong();
			if ( !currentFilesSHA1Checksum.isEmpty() || !currentFilesCrcChecksum.isEmpty() ) {
				treeWidgetChecksums->setItemSelected(item, true);
				romFileContextMenu->move(qmc2MainWindow->adjustedWidgetPosition(treeWidgetChecksums->viewport()->mapToGlobal(p), romFileContextMenu));
				romFileContextMenu->show();
			}
		} else {
			bool hasBadOrMissingDumps = analyzerBadSets.contains(item->text(QMC2_ROMALYZER_COLUMN_GAME).split(" ", QString::SkipEmptyParts)[0]);
			actionCopyBadToClipboard->setVisible(hasBadOrMissingDumps);
			actionCopyBadToClipboard->setEnabled(hasBadOrMissingDumps);
			actionRewriteSet->setVisible(groupBoxSetRewriter->isChecked());
			actionRewriteSet->setEnabled(groupBoxSetRewriter->isChecked());
			QStringList deviceRefs = item->whatsThis(QMC2_ROMALYZER_COLUMN_GAME).split(",", QString::SkipEmptyParts);
			deviceRefs.removeDuplicates();
			actionAnalyzeDeviceRefs->setText(tr("Analyse referenced devices") + QString(" [%1]").arg(deviceRefs.count()));
			actionAnalyzeDeviceRefs->setVisible(!deviceRefs.isEmpty());
			actionAnalyzeDeviceRefs->setEnabled(!deviceRefs.isEmpty());
			treeWidgetChecksums->setItemSelected(item, true);
			setRewriterItem = NULL;
			romSetContextMenu->move(qmc2MainWindow->adjustedWidgetPosition(treeWidgetChecksums->viewport()->mapToGlobal(p), romSetContextMenu));
			romSetContextMenu->show();
		}
	}
}

void ROMAlyzer::on_toolButtonSaveLog_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_toolButtonSaveLog_clicked()");
#endif

	QString fileName = QFileDialog::getSaveFileName(this, tr("Choose file to store the ROMAlyzer log"), "qmc2-romalyzer.log", tr("All files (*)"), 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !fileName.isEmpty() ) {
		QFile f(fileName);
		if ( f.open(QIODevice::WriteOnly | QIODevice::Text) ) {
			QTextStream ts(&f);
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("saving ROMAlyzer log to '%1'").arg(fileName));
			log(tr("saving ROMAlyzer log to '%1'").arg(fileName));
			ts << textBrowserLog->toPlainText();
			f.close();
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (saving ROMAlyzer log to '%1')").arg(fileName));
			log(tr("done (saving ROMAlyzer log to '%1')").arg(fileName));
		} else {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: can't open file '%1' for writing, please check permissions").arg(fileName));
			log(tr("WARNING: can't open file '%1' for writing, please check permissions").arg(fileName));
		}
	}
}

void ROMAlyzer::on_toolButtonCheckSumDbAddPath_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_toolButtonCheckSumDbAddPath_clicked()");
#endif

	QString newPath = QFileDialog::getExistingDirectory(this, tr("Choose path to be added to scan-list"), QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog));
	if ( !newPath.isEmpty() ) {
		QStringList checkSumDbScannedPaths;
		for (int i = 0; i < listWidgetCheckSumDbScannedPaths->count(); i++)
			checkSumDbScannedPaths << listWidgetCheckSumDbScannedPaths->item(i)->text();
		if ( !checkSumDbScannedPaths.contains(newPath) ) {
			QListWidgetItem *item = new QListWidgetItem(newPath);
			item->setCheckState(Qt::Checked);
			listWidgetCheckSumDbScannedPaths->addItem(item);
		}
	}

	pushButtonCheckSumDbScan->setEnabled(listWidgetCheckSumDbScannedPaths->count() > 0);
}

void ROMAlyzer::on_toolButtonCheckSumDbRemovePath_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_toolButtonCheckSumDbRemovePath_clicked()");
#endif

	foreach (QListWidgetItem *item, listWidgetCheckSumDbScannedPaths->selectedItems()) {
		listWidgetCheckSumDbScannedPaths->takeItem(listWidgetCheckSumDbScannedPaths->row(item));
		delete item;
	}

	pushButtonCheckSumDbScan->setEnabled(listWidgetCheckSumDbScannedPaths->count() > 0);
}

void ROMAlyzer::on_lineEditCheckSumDbDatabasePath_textChanged(const QString &text)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzer::on_lineEditCheckSumDbDatabasePath_textChanged(const QString &text = %1)").arg(text));
#endif

	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/CheckSumDbDatabasePath", text);
}

void ROMAlyzer::on_toolButtonBrowseCheckSumDbDatabasePath_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_toolButtonBrowseCheckSumDbDatabasePath_clicked()");
#endif

	QString fileName = QFileDialog::getSaveFileName(this, tr("Choose check-sum database file"), lineEditCheckSumDbDatabasePath->text(), tr("All files (*)"), 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !fileName.isEmpty() ) {
		lineEditCheckSumDbDatabasePath->setText(fileName);
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/CheckSumDbDatabasePath", fileName);
	}
}

void ROMAlyzer::on_toolButtonCheckSumDbViewLog_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_toolButtonCheckSumDbViewLog_clicked()");
#endif

	if ( checkSumScannerLog()->isMinimized() ) {
		checkSumScannerLog()->showNormal();
		checkSumScannerLog()->scrollToEnd();
		checkSumScannerLog()->raise();
	} else if ( checkSumScannerLog()->isVisible() ) {
		checkSumScannerLog()->close();
	} else {
		checkSumScannerLog()->show();
		checkSumScannerLog()->scrollToEnd();
		checkSumScannerLog()->raise();
	}
}

void ROMAlyzer::on_pushButtonCheckSumDbScan_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_pushButtonCheckSumDbScan_clicked()");
#endif

	pushButtonCheckSumDbScan->setEnabled(false);
	pushButtonCheckSumDbScan->update();
	pushButtonCheckSumDbPauseResumeScan->setEnabled(false);
	pushButtonCheckSumDbPauseResumeScan->update();
	qApp->processEvents();
	if ( checkSumScannerThread()->isActive )
		checkSumScannerThread()->stopScan = true;
	else if ( checkSumScannerThread()->isWaiting ) {
		checkSumDb()->disconnect(this);
		delete checkSumDb();
		m_checkSumDb = new CheckSumDatabaseManager(this, m_settingsKey);
		connect(checkSumDb(), SIGNAL(log(const QString &)), this, SLOT(log(const QString &)));
		checkSumScannerThread()->reopenCheckSumDb();
		checkSumScannerThread()->scannedPaths.clear();
		for (int i = 0; i < listWidgetCheckSumDbScannedPaths->count(); i++)
			if ( listWidgetCheckSumDbScannedPaths->item(i)->checkState() == Qt::Checked )
				checkSumScannerThread()->scannedPaths << listWidgetCheckSumDbScannedPaths->item(i)->text();
		checkSumScannerThread()->scanIncrementally = toolButtonCheckSumDbScanIncrementally->isChecked();
		checkSumScannerThread()->deepScan = toolButtonCheckSumDbDeepScan->isChecked();
		checkSumScannerThread()->waitCondition.wakeAll();
	}
	qApp->processEvents();
}

void ROMAlyzer::on_pushButtonCheckSumDbPauseResumeScan_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_pushButtonCheckSumDbPauseResumeScan_clicked()");
#endif

	pushButtonCheckSumDbPauseResumeScan->setEnabled(false);
	if ( checkSumScannerThread()->isPaused )
		QTimer::singleShot(0, checkSumScannerThread(), SLOT(resume()));
	else
		QTimer::singleShot(0, checkSumScannerThread(), SLOT(pause()));
}

void ROMAlyzer::on_listWidgetCheckSumDbScannedPaths_customContextMenuRequested(const QPoint &/*p*/)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_listWidgetCheckSumDbScannedPaths_customContextMenuRequested(const QPoint &p = ...)");
#endif

	// FIXME
}

void ROMAlyzer::on_listWidgetCheckSumDbScannedPaths_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::on_listWidgetCheckSumDbScannedPaths_itemSelectionChanged()");
#endif

	toolButtonCheckSumDbRemovePath->setEnabled(!listWidgetCheckSumDbScannedPaths->selectedItems().isEmpty());
}

void ROMAlyzer::checkSumScannerLog_windowOpened()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::checkSumScannerLog_windowClosed()");
#endif

	toolButtonCheckSumDbViewLog->setText(tr("Close log"));
}

void ROMAlyzer::checkSumScannerLog_windowClosed()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::checkSumScannerLog_windowClosed()");
#endif

	toolButtonCheckSumDbViewLog->setText(tr("Open log"));
}

void ROMAlyzer::checkSumScannerThread_scanStarted()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::checkSumScannerThread_scanStarted()");
#endif

	if ( collectionRebuilder() ) {
		delete collectionRebuilder();
		m_collectionRebuilder = NULL;
	}
	pushButtonCheckSumDbScan->setIcon(QIcon(QString::fromUtf8(":/data/img/halt.png")));
	pushButtonCheckSumDbScan->setText(tr("Stop scanner"));
	pushButtonCheckSumDbPauseResumeScan->setText(tr("Pause"));
	pushButtonCheckSumDbPauseResumeScan->show();
	checkSumDbStatusTimer.stop();
	checkSumDbStatusTimer.start(QMC2_CHECKSUM_DB_STATUS_UPDATE_SHORT);
	pushButtonCheckSumDbScan->setEnabled(true);
	pushButtonCheckSumDbPauseResumeScan->setEnabled(true);
	tabWidgetAnalysis->setTabEnabled(QMC2_ROMALYZER_PAGE_RCR, false);
	qApp->processEvents();
	QTimer::singleShot(0, this, SLOT(updateCheckSumDbStatus()));
}

void ROMAlyzer::checkSumScannerThread_scanFinished()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::checkSumScannerThread_scanFinished()");
#endif

	pushButtonCheckSumDbScan->setIcon(QIcon(QString::fromUtf8(":/data/img/refresh.png")));
	pushButtonCheckSumDbScan->setText(tr("Start scanner"));
	pushButtonCheckSumDbPauseResumeScan->hide();
	checkSumDbStatusTimer.stop();
	checkSumDbStatusTimer.start(QMC2_CHECKSUM_DB_STATUS_UPDATE_LONG);
	pushButtonCheckSumDbScan->setEnabled(true);
	pushButtonCheckSumDbPauseResumeScan->setEnabled(true);
	tabWidgetAnalysis->setTabEnabled(QMC2_ROMALYZER_PAGE_RCR, groupBoxCheckSumDatabase->isChecked() && groupBoxSetRewriter->isChecked());
	qApp->processEvents();
	updateCheckSumDbStatus();
	QTimer::singleShot(50, this, SLOT(updateCheckSumDbStatus()));
}

void ROMAlyzer::checkSumScannerThread_scanPaused()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::checkSumScannerThread_scanPaused()");
#endif

	pushButtonCheckSumDbPauseResumeScan->setText(tr("Resume"));
	pushButtonCheckSumDbPauseResumeScan->setEnabled(true);
}

void ROMAlyzer::checkSumScannerThread_scanResumed()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::checkSumScannerThread_scanResumed()");
#endif

	pushButtonCheckSumDbPauseResumeScan->setText(tr("Pause"));
	pushButtonCheckSumDbPauseResumeScan->setEnabled(true);
}

void ROMAlyzer::updateCheckSumDbStatus()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzer::updateCheckSumDbStatus()");
#endif

	bool isScanning = checkSumScannerThread()->status() == tr("scanning");

	QDateTime now = QDateTime::currentDateTime();
	QString statusString = "<center><table border=\"0\" cellpadding=\"2\" cellspacing=\"2\">";
	if ( isScanning ) {
		qint64 currentRowCount = checkSumDb()->checkSumRowCount();
		if ( currentRowCount >= 0 ) {
			if ( lastRowCount > 0 && currentRowCount >= lastRowCount )
				statusString += "<tr><td nowrap width=\"50%\" valign=\"top\" align=\"right\"><b>" + tr("Objects in database") + "</b></td><td nowrap width=\"50%\" valign=\"top\">" + QString::number(currentRowCount) + " | &Delta; " + QString::number(currentRowCount - lastRowCount) + "</td></tr>";
			else
				statusString += "<tr><td nowrap width=\"50%\" valign=\"top\" align=\"right\"><b>" + tr("Objects in database") + "</b></td><td nowrap width=\"50%\" valign=\"top\">" + QString::number(currentRowCount) + "</td></tr>";
			lastRowCount = currentRowCount;
		} else {
			if ( lastRowCount >= 0 )
				statusString += "<tr><td nowrap width=\"50%\" valign=\"top\" align=\"right\"><b>" + tr("Objects in database") + "</b></td><td nowrap width=\"50%\" valign=\"top\">" + QString::number(lastRowCount) + "</td></tr>";
			else
				statusString += "<tr><td nowrap width=\"50%\" valign=\"top\" align=\"right\"><b>" + tr("Objects in database") + "</b></td><td nowrap width=\"50%\" valign=\"top\">?</td></tr>";
		}
	} else {
		statusString += "<tr><td nowrap width=\"50%\" valign=\"top\" align=\"right\"><b>" + tr("Objects in database") + "</b></td><td nowrap width=\"50%\" valign=\"top\">" + QString::number(checkSumDb()->checkSumRowCount()) + "</td></tr>";
		lastRowCount = 0;
	}
	statusString += "<tr><td nowrap width=\"50%\" valign=\"top\" align=\"right\"><b>" + tr("Database size") + "</b></td><td nowrap width=\"50%\" valign=\"top\">" + humanReadable(checkSumDb()->databaseSize()) + "</td></tr>";
	QDateTime scanTime = QDateTime::fromTime_t(checkSumDb()->scanTime());
	QString ageString;
	int days = scanTime.daysTo(now);
	if ( days > 0 )
		ageString = tr("%n day(s)", "", days);
	else {
		int seconds = scanTime.secsTo(now);
		if ( seconds < 60 ) {
			ageString = tr("%n second(s)", "", seconds);
			if ( !checkSumScannerThread()->isActive )
				QTimer::singleShot(QMC2_CHECKSUM_DB_STATUS_UPDATE_SHORT, this, SLOT(updateCheckSumDbStatus()));
		} else {
			int hours = seconds / 3600;
			if ( hours > 0 )
				ageString = tr("%n hour(s)", "", hours);
			else
				ageString = tr("%n minute(s)", "", seconds / 60);
		}
	}
	statusString += "<tr><td nowrap width=\"50%\" valign=\"top\" align=\"right\"><b>" + tr("Age of stored data") + "</b></td><td nowrap width=\"50%\" valign=\"top\">" + ageString + "</td></tr>";
	statusString += "<tr><td nowrap width=\"50%\" valign=\"top\" align=\"right\"><b>" + tr("Pending updates") + "</b></td><td nowrap width=\"50%\" valign=\"top\">" + QString::number(checkSumScannerThread()->pendingUpdates()) + "</td></tr>";
	if ( checkSumScannerLog()->progress() > 0 && isScanning )
		statusString += "<tr><td nowrap width=\"50%\" valign=\"top\" align=\"right\"><b>" + tr("Scanner status") + "</b></td><td nowrap width=\"50%\" valign=\"top\">" + checkSumScannerThread()->status() + " | " + QString::number(checkSumScannerLog()->progress(), 'f', 1) + "%</td></tr>";
	else
		statusString += "<tr><td nowrap width=\"50%\" valign=\"top\" align=\"right\"><b>" + tr("Scanner status") + "</b></td><td nowrap width=\"50%\" valign=\"top\">" + checkSumScannerThread()->status() + "</td></tr>";
	statusString += "</table></center>";
	labelCheckSumDbStatusDisplay->setText(statusString);
	qApp->processEvents();
}

void ROMAlyzer::indicateCheckSumDbQueryStatusGood()
{
	m_checkSumDbQueryStatusPixmap = QPixmap(QString::fromUtf8(":/data/img/database_good.png"));
	widgetCheckSumDbQueryStatus->setFixedWidth(widgetCheckSumDbQueryStatus->height());
	QPalette pal = widgetCheckSumDbQueryStatus->palette();
	pal.setBrush(QPalette::Window, m_checkSumDbQueryStatusPixmap.scaled(widgetCheckSumDbQueryStatus->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
	widgetCheckSumDbQueryStatus->setPalette(pal);
}

void ROMAlyzer::indicateCheckSumDbQueryStatusBad()
{
	m_checkSumDbQueryStatusPixmap = QPixmap(QString::fromUtf8(":/data/img/database_bad.png"));
	widgetCheckSumDbQueryStatus->setFixedWidth(widgetCheckSumDbQueryStatus->height());
	QPalette pal = widgetCheckSumDbQueryStatus->palette();
	pal.setBrush(QPalette::Window, m_checkSumDbQueryStatusPixmap.scaled(widgetCheckSumDbQueryStatus->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
	widgetCheckSumDbQueryStatus->setPalette(pal);
}

void ROMAlyzer::indicateCheckSumDbQueryStatusUnknown()
{
	m_checkSumDbQueryStatusPixmap = QPixmap(QString::fromUtf8(":/data/img/database.png"));
	widgetCheckSumDbQueryStatus->setFixedWidth(widgetCheckSumDbQueryStatus->height());
	QPalette pal = widgetCheckSumDbQueryStatus->palette();
	pal.setBrush(QPalette::Window, m_checkSumDbQueryStatusPixmap.scaled(widgetCheckSumDbQueryStatus->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
	widgetCheckSumDbQueryStatus->setPalette(pal);
}

void ROMAlyzer::softwareListLoadFinished(bool /* success */)
{
	setEnabled(true);
}

void ROMAlyzer::switchToCollectionRebuilder()
{
	if ( !collectionRebuilder() ) {
		m_collectionRebuilder = new CollectionRebuilder(this, tabCollectionRebuilder);
		gridLayoutCollectionRebuilder->addWidget(collectionRebuilder(), 0, 0);
	}
	tabWidgetAnalysis->setCurrentIndex(QMC2_ROMALYZER_PAGE_RCR);
	QTimer::singleShot(0, collectionRebuilder(), SLOT(scrollToEnd()));
}

ROMAlyzerXmlHandler::ROMAlyzerXmlHandler(QTreeWidgetItem *parent, bool expand, bool scroll, int mode)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ROMAlyzerXmlHandler::ROMAlyzerXmlHandler(QTreeWidgetItem *parent = %1, bool expand = %2, bool scroll = %3, int mode = %4)").arg((qulonglong)parent).arg(expand).arg(scroll).arg(mode));
#endif

	parentItem = parent;
	autoExpand = expand;
	autoScroll = scroll;
	romalyzerMode = mode;

	redBrush = QBrush(QColor(255, 0, 0));
	greenBrush = QBrush(QColor(0, 255, 0));
	blueBrush = QBrush(QColor(0, 0, 255));
	yellowBrush = QBrush(QColor(255, 255, 0));
	brownBrush = QBrush(QColor(128, 128, 0));
	greyBrush = QBrush(QColor(128, 128, 128));
}

bool ROMAlyzerXmlHandler::startElement(const QString &/*namespaceURI*/, const QString &/*localName*/, const QString &qName, const QXmlAttributes &attributes)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzerXmlHandler::startElement(...)");
#endif

	QString s;
	QString mainEntityName;

	switch ( romalyzerMode ) {
		case QMC2_ROMALYZER_MODE_SOFTWARE:
			mainEntityName = "software";
			break;
		case QMC2_ROMALYZER_MODE_SYSTEM:
		default:
			mainEntityName = "machine";
			break;
	}

	if ( qName == mainEntityName ) {
		switch ( romalyzerMode ) {
			case QMC2_ROMALYZER_MODE_SOFTWARE:
				parentItem->setText(QMC2_ROMALYZER_COLUMN_MERGE, attributes.value("cloneof"));
				break;
			case QMC2_ROMALYZER_MODE_SYSTEM:
			default:
				parentItem->setText(QMC2_ROMALYZER_COLUMN_MERGE, attributes.value("romof"));
				break;
		}
		parentItem->setExpanded(false);
		emuStatus = 0;
		fileCounter = 0;
		currentText.clear();
		childItems.clear();
		deviceReferences.clear();
		optionalROMs.clear();
	} else if ( qName == "rom" || qName == "disk" ) {
		if ( !attributes.value("name").isEmpty() ) {
			fileCounter++;
			childItem = new QTreeWidgetItem(parentItem);
			childItems << childItem;
			childItem->setText(QMC2_ROMALYZER_COLUMN_GAME, attributes.value("name"));
			childItem->setText(QMC2_ROMALYZER_COLUMN_TYPE, qName == "rom" ? QObject::tr("ROM") : QObject::tr("CHD"));
			childItem->setText(QMC2_ROMALYZER_COLUMN_MERGE, attributes.value("merge"));
			s = attributes.value("status");
			if ( s.isEmpty() || s == "good" ) {
				childItem->setText(QMC2_ROMALYZER_COLUMN_EMUSTATUS, QObject::tr("good"));
				childItem->setForeground(QMC2_ROMALYZER_COLUMN_EMUSTATUS, greenBrush);
				emuStatus |= QMC2_ROMALYZER_EMUSTATUS_GOOD;
			} else if ( s == "nodump" ) {
				childItem->setText(QMC2_ROMALYZER_COLUMN_EMUSTATUS, QObject::tr("no dump"));
				childItem->setForeground(QMC2_ROMALYZER_COLUMN_EMUSTATUS, brownBrush);
				emuStatus |= QMC2_ROMALYZER_EMUSTATUS_NODUMP;
			} else if ( s == "baddump" ) {
				childItem->setText(QMC2_ROMALYZER_COLUMN_EMUSTATUS, QObject::tr("bad dump"));
				childItem->setForeground(QMC2_ROMALYZER_COLUMN_EMUSTATUS, brownBrush);
				emuStatus |= QMC2_ROMALYZER_EMUSTATUS_BADDUMP;
			} else {
				childItem->setText(QMC2_ROMALYZER_COLUMN_EMUSTATUS, QObject::tr("unknown"));
				childItem->setForeground(QMC2_ROMALYZER_COLUMN_EMUSTATUS, blueBrush);
				emuStatus |= QMC2_ROMALYZER_EMUSTATUS_UNKNOWN;
			}
			childItem->setText(QMC2_ROMALYZER_COLUMN_SIZE, attributes.value("size"));
			childItem->setText(QMC2_ROMALYZER_COLUMN_CRC, attributes.value("crc"));
			childItem->setText(QMC2_ROMALYZER_COLUMN_SHA1, attributes.value("sha1"));
			childItem->setText(QMC2_ROMALYZER_COLUMN_MD5, attributes.value("md5"));
			if ( attributes.value("optional") == "yes" )
				optionalROMs << attributes.value("crc");
		}
	} else if ( qName == "device_ref" )
		deviceReferences << attributes.value("name");

	return true;
}

bool ROMAlyzerXmlHandler::endElement(const QString &/*namespaceURI*/, const QString &/*localName*/, const QString &qName)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzerXmlHandler::endElement(...)");
#endif

	QString mainEntityName;

	switch ( romalyzerMode ) {
		case QMC2_ROMALYZER_MODE_SOFTWARE:
			mainEntityName = "software";
			break;
		case QMC2_ROMALYZER_MODE_SYSTEM:
		default:
			mainEntityName = "machine";
			break;
	}

	if ( qName == mainEntityName ) {
		QString s(parentItem->text(QMC2_ROMALYZER_COLUMN_GAME));
		s += " [" + QString::number(fileCounter) + "]";
		parentItem->setText(QMC2_ROMALYZER_COLUMN_GAME, s);
		QString emuStatusStr;
		QBrush myBrush;
		if ( emuStatus == QMC2_ROMALYZER_EMUSTATUS_GOOD ) {
			emuStatusStr = QObject::tr("good");
			myBrush = greenBrush;
		} else if ( emuStatus & QMC2_ROMALYZER_EMUSTATUS_UNKNOWN ) {
			emuStatusStr = QObject::tr("unknown");
			myBrush = blueBrush;
		} else if ( emuStatus & QMC2_ROMALYZER_EMUSTATUS_NODUMP && emuStatus & QMC2_ROMALYZER_EMUSTATUS_BADDUMP ) {
			emuStatusStr = QObject::tr("no / bad dump");
			myBrush = brownBrush;
		} else if ( emuStatus & QMC2_ROMALYZER_EMUSTATUS_NODUMP ) {
			emuStatusStr = QObject::tr("no dump");
			myBrush = brownBrush;
		} else if ( emuStatus & QMC2_ROMALYZER_EMUSTATUS_BADDUMP ) {
			emuStatusStr = QObject::tr("bad dump");
			myBrush = brownBrush;
		} else {
			emuStatusStr = QObject::tr("unknown");
			myBrush = blueBrush;
		}
		if ( fileCounter == 0 ) {
			emuStatusStr = QObject::tr("good");
			myBrush = greenBrush;
		}
		parentItem->setText(QMC2_ROMALYZER_COLUMN_EMUSTATUS, emuStatusStr);
		parentItem->setForeground(QMC2_ROMALYZER_COLUMN_EMUSTATUS, myBrush);
		if ( autoExpand )
			parentItem->setExpanded(true);
		if ( autoScroll )
			parentItem->treeWidget()->scrollToItem(parentItem, QAbstractItemView::PositionAtTop);
	}

	return true;
}

bool ROMAlyzerXmlHandler::characters(const QString &str)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ROMAlyzerXmlHandler::characters(...)");
#endif

	currentText += QString::fromUtf8(str.toUtf8());
	return true;
}

CheckSumScannerThread::CheckSumScannerThread(CheckSumScannerLog *scannerLog, QString settingsKey, QObject *parent)
	: QThread(parent)
{
	isActive = exitThread = isWaiting = isPaused = pauseRequested = stopScan = scanIncrementally = deepScan = false;
	m_preparingIncrementalScan = false;
	m_checkSumDb = NULL;
	m_scannerLog = scannerLog;
	m_settingsKey = settingsKey;
	m_pendingUpdates = 0;
	reopenCheckSumDb();
	start();
}

CheckSumScannerThread::~CheckSumScannerThread()
{
	exitThread = true;
	waitCondition.wakeAll();
	wait();
	if ( checkSumDb() ) {
		checkSumDb()->disconnect(m_scannerLog);
		delete checkSumDb();
	}
}

QString CheckSumScannerThread::status()
{
	if ( m_preparingIncrementalScan )
		return tr("preparing");
	if ( exitThread )
		return tr("exiting");
	if ( stopScan )
		return tr("stopping");
	if ( isPaused )
		return tr("paused");
	if ( isActive )
		return tr("scanning");
	return tr("idle");
}

void CheckSumScannerThread::prepareIncrementalScan(QStringList *fileList)
{
	m_preparingIncrementalScan = true;
	emit log(tr("preparing incremental scan"));
	// step 1: remove entries from the database that "point to nowhere" (a.k.a. aren't contained in 'fileList'), storing all paths kept in the database
	//         in the 'pathsInDatabase' hash for use in steps 2 and 3 so we don't need to query the database again
	QHash<QString, bool> pathsInDatabase;
	qint64 row = checkSumDb()->nextRowId(true);
	emit progressTextChanged(tr("Preparing") + " - " + tr("Step %1 of %2").arg(1).arg(3));
	emit progressRangeChanged(0, checkSumDb()->checkSumRowCount() - 1);
	emit progressChanged(0);
	int count = 0;
	qint64 pathsRemoved = 0;
	checkSumDb()->beginTransaction();
	QHash<QString, bool> fileHash;
	foreach (QString file, *fileList)
		fileHash.insert(file, true);
	while ( row > 0 && !exitThread && !stopScan ) {
		emit progressChanged(count++);
		QString path = checkSumDb()->pathOfRow(row);
		if ( !path.isEmpty() ) {
			if ( !fileHash.contains(path) ) {
				checkSumDb()->pathRemove(path);
				pathsRemoved++;
			} else
				pathsInDatabase[path] = true;
		}
		row = checkSumDb()->nextRowId();
		if ( exitThread || stopScan )
			break;
	}
	checkSumDb()->commitTransaction();
	emit log(tr("%n obsolete path(s) removed from database", "", pathsRemoved));
	if ( !exitThread && !stopScan ) {
		// step 2: remove entries from 'fileList' where 'scanTime' is later than the file's modification time *and* the database has entries for it
		emit progressTextChanged(tr("Preparing") + " - " + tr("Step %1 of %2").arg(2).arg(3));
		int oldFileListCount = fileList->count();
		emit progressRangeChanged(0, oldFileListCount - 1);
		emit progressChanged(0);
		int filesRemoved = 0;
		uint scanTime = checkSumDb()->scanTime();
		count = 0;
		for (int i = 0; i < fileList->count() && !exitThread && !stopScan; i++) {
			emit progressChanged(count++);
			QFileInfo fi(fileList->at(i));
			if ( fi.lastModified().toTime_t() < scanTime && pathsInDatabase.contains(fileList->at(i)) ) {
				fileList->removeAt(i);
				filesRemoved++;
				i--;
			}
		}
		emit log(tr("%n unchanged file(s) removed from scan", "", filesRemoved));
		fileHash.clear();
		foreach (QString file, *fileList)
			fileHash.insert(file, true);
		if ( !exitThread && !stopScan ) {
			// step 3: remove entries from the database that "point to new stuff" (a.k.a. are still contained in the modified 'fileList')
			emit progressTextChanged(tr("Preparing") + " - " + tr("Step %1 of %2").arg(3).arg(3));
			emit progressRangeChanged(0, pathsInDatabase.count());
			emit progressChanged(0);
			pathsRemoved = 0;
			count = 0;
			checkSumDb()->beginTransaction();
			foreach (QString path, pathsInDatabase.keys()) {
				emit progressChanged(count++);
				if ( fileHash.contains(path) ) {
					checkSumDb()->pathRemove(path);
					pathsRemoved++;
				}
				if ( exitThread || stopScan )
					break;
			}
			checkSumDb()->commitTransaction();
			emit log(tr("%n outdated path(s) removed from database", "", pathsRemoved));

			if ( !exitThread && !stopScan ) {
				emit progressTextChanged(tr("Preparing"));
				emit progressRangeChanged(0, 0);
				emit progressChanged(-1);
				emit log(tr("freeing unused space previously occupied by database"));
				checkSumDb()->vacuum();
			}
		}
	}
	m_preparingIncrementalScan = false;
}

void CheckSumScannerThread::reopenCheckSumDb()
{
	if ( checkSumDb() ) {
		checkSumDb()->disconnect(m_scannerLog);
		delete checkSumDb();
	}
	m_checkSumDb = new CheckSumDatabaseManager(this, m_settingsKey);
	checkSumDb()->setSyncMode(QMC2_DB_SYNC_MODE_OFF);
	checkSumDb()->setJournalMode(QMC2_DB_JOURNAL_MODE_MEMORY);
	connect(checkSumDb(), SIGNAL(log(const QString &)), m_scannerLog, SLOT(log(const QString &)));
}

void CheckSumScannerThread::pause()
{
	pauseRequested = true;
}

void CheckSumScannerThread::resume()
{
	isPaused = false;
}

void CheckSumScannerThread::run()
{
	emit log(tr("scanner thread started"));
	while ( !exitThread ) {
		emit log(tr("waiting for work"));
		mutex.lock();
		isWaiting = true;
		isActive = stopScan = isPaused = false;
		waitCondition.wait(&mutex);
		isActive = true;
		isWaiting = false;
		mutex.unlock();
		if ( !exitThread && !stopScan ) {
			emit scanStarted();
			QTime scanTimer, elapsedTime(0, 0, 0, 0);
			scanTimer.start();
			QStringList fileList;
			emit progressTextChanged(tr("Scanning"));
			emit progressRangeChanged(0, 0);
			emit progressChanged(-1);
			foreach (QString path, scannedPaths) {
				emit log(tr("searching available files for path '%1'").arg(path));
				QStringList pathFileList;
				recursiveFileList(path, &pathFileList);
				fileList.append(pathFileList);
				emit log(tr("found %n file(s) for path '%1'", "", pathFileList.count()).arg(path));
				QTest::qWait(0);
				if ( exitThread || stopScan )
					break;
			}
			if ( scanIncrementally )
				prepareIncrementalScan(&fileList);
			else
				checkSumDb()->recreateDatabase();
			emit progressTextChanged(tr("Scanning"));
			emit progressRangeChanged(0, fileList.count());
			emit progressChanged(0);
			emit log(tr("starting database transaction"));
			checkSumDb()->beginTransaction();
			int counter = 0;
			foreach (QString filePath, fileList) {
				QTest::qWait(0);
				if ( exitThread || stopScan )
					break;
				emit log(tr("scan started for file '%1'").arg(filePath));
				emit progressChanged(counter++);
				QStringList memberList, sha1List, crcList;
				QString sha1, crc;
				QList<quint64> sizeList;
				quint64 size;
				int type = fileType(filePath);
				bool doDbUpdate = true;
				switch ( type ) {
					case QMC2_CHECKSUM_SCANNER_FILE_ZIP:
						if ( !scanZip(filePath, &memberList, &sizeList, &sha1List, &crcList) ) {
							emit log(tr("WARNING: scan failed for file '%1'").arg(filePath));
							doDbUpdate = false;
						}
						break;
					case QMC2_CHECKSUM_SCANNER_FILE_7Z:
						if ( !scanSevenZip(filePath, &memberList, &sizeList, &sha1List, &crcList) ) {
							emit log(tr("WARNING: scan failed for file '%1'").arg(filePath));
							doDbUpdate = false;
						}
						break;
					case QMC2_CHECKSUM_SCANNER_FILE_CHD:
						if ( !scanChd(filePath, &size, &sha1) ) {
							emit log(tr("WARNING: scan failed for file '%1'").arg(filePath));
							doDbUpdate = false;
						}
						break;
					case QMC2_CHECKSUM_SCANNER_FILE_REGULAR:
						if ( !scanRegularFile(filePath, &size, &sha1, &crc) ) {
							emit log(tr("WARNING: scan failed for file '%1'").arg(filePath));
							doDbUpdate = false;
						}
						break;
					default:
					case QMC2_CHECKSUM_SCANNER_FILE_NO_ACCESS:
						emit log(tr("WARNING: can't access file '%1', please check permissions").arg(filePath));
						doDbUpdate = false;
						break;
				}
				if ( exitThread || stopScan )
					break;
				if ( doDbUpdate ) {
					switch ( type ) {
						case QMC2_CHECKSUM_SCANNER_FILE_ZIP:
						case QMC2_CHECKSUM_SCANNER_FILE_7Z:
							for (int i = 0; i < memberList.count(); i++) {
								if ( !checkSumDb()->exists(sha1List[i], crcList[i], sizeList[i]) ) {
									emit log(tr("database update") + ": " + tr("adding member '%1' from archive '%2' with SHA-1 '%3' and CRC '%4' to database").arg(memberList[i]).arg(filePath).arg(sha1List[i]).arg(crcList[i]));
									checkSumDb()->setData(sha1List[i], crcList[i], sizeList[i], filePath, memberList[i], checkSumDb()->typeToName(type));
									m_pendingUpdates++;
								} else
									emit log(tr("database update") + ": " + tr("an object with SHA-1 '%1' and CRC '%2' already exists in the database").arg(sha1List[i]).arg(crcList[i]) + ", " + tr("member '%1' from archive '%2' ignored").arg(memberList[i]).arg(filePath));
								if ( m_pendingUpdates >= QMC2_CHECKSUM_DB_MAX_TRANSACTIONS ) {
									emit log(tr("committing database transaction"));
									checkSumDb()->setScanTime(QDateTime::currentDateTime().toTime_t());
									checkSumDb()->commitTransaction();
									m_pendingUpdates = 0;
									emit log(tr("starting database transaction"));
									checkSumDb()->beginTransaction();
								}
							}
							break;
						case QMC2_CHECKSUM_SCANNER_FILE_CHD:
							if ( !checkSumDb()->exists(sha1, crc, size) ) {
								emit log(tr("database update") + ": " + tr("adding CHD '%1' with SHA-1 '%2' to database").arg(filePath).arg(sha1));
								checkSumDb()->setData(sha1, QString(), size, filePath, QString(), checkSumDb()->typeToName(type));
								m_pendingUpdates++;
							} else
								emit log(tr("database update") + ": " + tr("an object with SHA-1 '%1' and CRC '%2' already exists in the database").arg(sha1).arg(crc) + ", " + tr("CHD '%1' ignored").arg(filePath));
							break;
						case QMC2_CHECKSUM_SCANNER_FILE_REGULAR:
							if ( !checkSumDb()->exists(sha1, crc, size) ) {
								emit log(tr("database update") + ": " + tr("adding file '%1' with SHA-1 '%2' and CRC '%3' to database").arg(filePath).arg(sha1).arg(crc));
								checkSumDb()->setData(sha1, crc, size, filePath, QString(), checkSumDb()->typeToName(type));
								m_pendingUpdates++;
							} else
								emit log(tr("database update") + ": " + tr("an object with SHA-1 '%1' and CRC '%2' already exists in the database").arg(sha1).arg(crc) + ", " + tr("file '%1' ignored").arg(filePath));
							break;
						default:
							break;
					}
				}
				if ( m_pendingUpdates >= QMC2_CHECKSUM_DB_MAX_TRANSACTIONS ) {
					emit log(tr("committing database transaction"));
					checkSumDb()->setScanTime(QDateTime::currentDateTime().toTime_t());
					checkSumDb()->commitTransaction();
					m_pendingUpdates = 0;
					emit log(tr("starting database transaction"));
					checkSumDb()->beginTransaction();
				}
				bool pauseMessageLogged = false;
				while ( (pauseRequested || isPaused) && !exitThread && !stopScan ) {
					if ( !pauseMessageLogged ) {
						pauseMessageLogged = true;
						isPaused = true;
						pauseRequested = false;
						emit scanPaused();
						emit log(tr("scanner paused"));
						emit progressTextChanged(tr("Paused"));
					}
					QTest::qWait(100);
				}
				if ( pauseMessageLogged && !exitThread && !stopScan ) {
					isPaused = false;
					emit scanResumed();
					emit log(tr("scanner resumed"));
					emit progressTextChanged(tr("Scanning"));
				}
				if ( exitThread || stopScan )
					break;
				else
					emit log(tr("scan finished for file '%1'").arg(filePath));

			}
			if ( exitThread || stopScan )
				emit log(tr("scanner interrupted"));
			emit log(tr("committing database transaction"));
			checkSumDb()->setScanTime(QDateTime::currentDateTime().toTime_t());
			checkSumDb()->commitTransaction();
			m_pendingUpdates = 0;
			emit progressTextChanged(tr("Idle"));
			emit progressRangeChanged(0, 100);
			emit progressChanged(0);
			elapsedTime = elapsedTime.addMSecs(scanTimer.elapsed());
			emit log(tr("scan finished - total scanning time = %1, objects in database = %2, database size = %3").arg(elapsedTime.toString("hh:mm:ss.zzz")).arg(checkSumDb()->checkSumRowCount()).arg(ROMAlyzer::humanReadable(checkSumDb()->databaseSize())));
			emit scanFinished();
		}
	}
	emit log(tr("scanner thread ended"));
}

void CheckSumScannerThread::recursiveFileList(const QString &sDir, QStringList *fileNames)
{
	if ( exitThread || stopScan )
		return;
#if defined(QMC2_OS_WIN)
	WIN32_FIND_DATA ffd;
	QString dirName = QDir::toNativeSeparators(QDir::cleanPath(sDir + "/*"));
#ifdef UNICODE
	HANDLE hFind = FindFirstFile((TCHAR *)dirName.utf16(), &ffd);
#else
	HANDLE hFind = FindFirstFile((TCHAR *)dirName.toUtf8().constData(), &ffd);
#endif
	if ( !exitThread && !stopScan && hFind != INVALID_HANDLE_VALUE ) {
		do {
#ifdef UNICODE
			QString fName = QString::fromUtf16((ushort*)ffd.cFileName);
#else
			QString fName = QString::fromLocal8Bit(ffd.cFileName);
#endif
			if ( ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
				if ( fName != ".." && fName != "." )
					recursiveFileList(sDir + "/" + fName, fileNames);
			} else
				fileNames->append(sDir + "/" + fName);
		} while ( !exitThread && !stopScan && FindNextFile(hFind, &ffd) != 0 );
	}
#else
	QDir dir(sDir);
	foreach (QFileInfo info, dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::Hidden | QDir::System)) {
		if ( exitThread || stopScan )
			break;
		QString path = info.filePath();
		if ( info.isDir() ) {
			// directory recursion
			if ( info.fileName() != ".." && info.fileName() != "." )
				recursiveFileList(path, fileNames);
		} else
			fileNames->append(path);
	}
#endif
}

int CheckSumScannerThread::fileType(QString fileName)
{
	static QRegExp zipRx("[Zz][Ii][Pp]");
	static QRegExp sevenZipRx("7[Zz]");
	static QRegExp chdRx("[Cc][Hh][Dd]");

	QFileInfo fileInfo(fileName);
	if ( fileInfo.isReadable() ) {
		if ( fileInfo.suffix().indexOf(zipRx) == 0 )
			return QMC2_CHECKSUM_SCANNER_FILE_ZIP;
		else if ( fileInfo.suffix().indexOf(sevenZipRx) == 0 )
			return QMC2_CHECKSUM_SCANNER_FILE_7Z;
		else if ( fileInfo.suffix().indexOf(chdRx) == 0 )
			return QMC2_CHECKSUM_SCANNER_FILE_CHD;
		else
			return QMC2_CHECKSUM_SCANNER_FILE_REGULAR;
	} else
		return QMC2_CHECKSUM_SCANNER_FILE_NO_ACCESS;
}

bool CheckSumScannerThread::scanZip(QString fileName, QStringList *memberList, QList<quint64> *sizeList, QStringList *sha1List, QStringList *crcList)
{
	unzFile zipFile = unzOpen(fileName.toUtf8().constData());
	if ( zipFile ) {
  		char ioBuffer[QMC2_ROMALYZER_ZIP_BUFFER_SIZE];
		unz_file_info zipInfo;
		do {
			if ( exitThread || stopScan )
				break;
			if ( unzGetCurrentFileInfo(zipFile, &zipInfo, ioBuffer, QMC2_ROMALYZER_ZIP_BUFFER_SIZE, 0, 0, 0, 0) == UNZ_OK ) {
				QString fn((const char *)ioBuffer);
				if ( exitThread || stopScan )
					break;
				if ( deepScan ) {
					if ( unzOpenCurrentFile(zipFile) == UNZ_OK ) {
						quint64 memberSize = 0;
						qint64 len;
						QCryptographicHash sha1Hash(QCryptographicHash::Sha1);
						ulong crc1 = crc32(0, NULL, 0);
						while ( (len = unzReadCurrentFile(zipFile, ioBuffer, QMC2_ROMALYZER_ZIP_BUFFER_SIZE)) > 0 ) {
							QByteArray fileData((const char *)ioBuffer, len);
							sha1Hash.addData(fileData);
							if ( crc1 > 0 ) {
								ulong crc2 = crc32(0, NULL, 0);
								crc2 = crc32(crc2, (const Bytef *)fileData.data(), fileData.size());
								crc1 = crc32_combine(crc1, crc2, fileData.size());
							} else
								crc1 = crc32(crc1, (const Bytef *)fileData.data(), fileData.size());
							memberSize += len;
							if ( exitThread || stopScan )
								break;
						}
						unzCloseCurrentFile(zipFile);
						memberList->append(fn);
						sizeList->append(memberSize);
						sha1List->append(sha1Hash.result().toHex());
						crcList->append(crcToString(crc1));
						emit log(tr("ZIP scan") + ": " + tr("member '%1' from archive '%2' has SHA-1 '%3' and CRC '%4'").arg(fn).arg(fileName).arg(sha1List->last()).arg(crcList->last()));
					} else
						emit log(tr("ZIP scan") + ": " + tr("WARNING: can't open member '%1' from archive '%2'").arg(fn).arg(fileName));
				} else {
					memberList->append(fn);
					sizeList->append(zipInfo.uncompressed_size);
					sha1List->append(QString());
					crcList->append(crcToString(zipInfo.crc));
					emit log(tr("ZIP scan") + ": " + tr("member '%1' from archive '%2' has SHA-1 '%3' and CRC '%4'").arg(fn).arg(fileName).arg(sha1List->last()).arg(crcList->last()));
				}
			}
		} while ( unzGoToNextFile(zipFile) == UNZ_OK );
		unzClose(zipFile);
		return true;
	} else
		return false;
}

bool CheckSumScannerThread::scanSevenZip(QString fileName, QStringList *memberList, QList<quint64> *sizeList, QStringList *sha1List, QStringList *crcList)
{
	SevenZipFile sevenZipFile(fileName);
	if ( sevenZipFile.open() ) {
		foreach (SevenZipMetaData metaData, sevenZipFile.itemList()) {
			if ( exitThread || stopScan )
				break;
			if ( deepScan ) {
				QByteArray fileData;
				quint64 readLength = sevenZipFile.read(metaData.name(), &fileData);
				if ( readLength > 0 ) {
					if ( exitThread || stopScan )
						break;
					memberList->append(metaData.name());
					sizeList->append(metaData.size());
					QCryptographicHash sha1Hash(QCryptographicHash::Sha1);
					sha1Hash.addData(fileData);
					sha1List->append(sha1Hash.result().toHex());
					ulong crc = crc32(0, NULL, 0);
					crc = crc32(crc, (const Bytef *)fileData.data(), fileData.size());
					crcList->append(crcToString(crc));
					emit log(tr("7Z scan") + ": " + tr("member '%1' from archive '%2' has SHA-1 '%3' and CRC '%4'").arg(metaData.name()).arg(fileName).arg(sha1List->last()).arg(crcList->last()));
				} else
					emit log(tr("7Z scan") + ": " + tr("WARNING: can't read member '%1' from archive '%2'").arg(metaData.name()).arg(fileName));
			} else {
				memberList->append(metaData.name());
				sizeList->append(metaData.size());
				sha1List->append(QString());
				crcList->append(metaData.crc());
				emit log(tr("7Z scan") + ": " + tr("member '%1' from archive '%2' has SHA-1 '%3' and CRC '%4'").arg(metaData.name()).arg(fileName).arg(sha1List->last()).arg(crcList->last()));
			}
		}
		sevenZipFile.close();
		return true;
	} else
		return false;
}

bool CheckSumScannerThread::scanChd(QString fileName, quint64 *size, QString *sha1)
{
	QFile file(fileName);
	if ( file.open(QIODevice::ReadOnly) ) {
  		char ioBuffer[QMC2_ROMALYZER_FILE_BUFFER_SIZE];
		bool success = true;
		int len = 0;
		quint32 chdVersion = 0;
		if ( (len = file.read(ioBuffer, QMC2_CHD_HEADER_V3_LENGTH)) > 0 ) {
			if ( len < QMC2_CHD_HEADER_V3_LENGTH ) {
				emit log(tr("CHD scan") + ": " + tr("WARNING: can't read CHD '%1'").arg(fileName));
				success = false;
			} else {
				chdVersion = QMC2_TO_UINT32(ioBuffer + QMC2_CHD_HEADER_VERSION_OFFSET);
				switch ( chdVersion ) {
					case 3: {
						QByteArray sha1Data((const char *)(ioBuffer + QMC2_CHD_HEADER_V3_SHA1_OFFSET), QMC2_CHD_HEADER_V3_SHA1_LENGTH);
						*sha1 = QString(sha1Data.toHex());
						break;
					}
					case 4: {
						QByteArray sha1Data((const char *)(ioBuffer + QMC2_CHD_HEADER_V4_SHA1_OFFSET), QMC2_CHD_HEADER_V4_SHA1_LENGTH);
						*sha1 = QString(sha1Data.toHex());
						break;
					}
					case 5: {
						QByteArray sha1Data((const char *)(ioBuffer + QMC2_CHD_HEADER_V5_SHA1_OFFSET), QMC2_CHD_HEADER_V5_SHA1_LENGTH);
						*sha1 = QString(sha1Data.toHex());
						break;
					}
					default: {
						emit log(tr("CHD scan") + ": " + tr("WARNING: version '%1' of CHD '%2' unknown").arg(fileName));
						success = false;
						break;
					}
				}
				*size = file.size();
			}
		} else {
			emit log(tr("CHD scan") + ": " + tr("WARNING: can't read CHD '%1'").arg(fileName));
			success = false;
		}
		file.close();
		if ( !success )
			return false;
		if ( exitThread || stopScan )
			return true;
		emit log(tr("CHD scan") + ": " + tr("CHD '%1' has SHA-1 '%2' (CHD v%3)").arg(fileName).arg(*sha1).arg(chdVersion));
		return true;
	} else
		return false;
}

bool CheckSumScannerThread::scanRegularFile(QString fileName, quint64 *size, QString *sha1, QString *crc)
{
	QFile file(fileName);
	if ( file.open(QIODevice::ReadOnly) ) {
  		char ioBuffer[QMC2_ROMALYZER_FILE_BUFFER_SIZE];
		QCryptographicHash sha1Hash(QCryptographicHash::Sha1);
		ulong crc1 = crc32(0, NULL, 0);
		int len = 0;
		while ( (len = file.read(ioBuffer, QMC2_ROMALYZER_FILE_BUFFER_SIZE)) > 0 ) {
			QByteArray fileData((const char *)ioBuffer, len);
			sha1Hash.addData(fileData);
			if ( crc1 > 0 ) {
				ulong crc2 = crc32(0, NULL, 0);
				crc2 = crc32(crc2, (const Bytef *)fileData.data(), fileData.size());
				crc1 = crc32_combine(crc1, crc2, fileData.size());
			} else
				crc1 = crc32(crc1, (const Bytef *)fileData.data(), fileData.size());
			if ( exitThread || stopScan )
				break;
		}
		*size = file.size();
		file.close();
		if ( exitThread || stopScan )
			return true;
		*sha1 = sha1Hash.result().toHex();
		*crc = crcToString(crc1);
		emit log(tr("file scan") + ": " + tr("file '%1' has SHA-1 '%2' and CRC '%3'").arg(fileName).arg(*sha1).arg(*crc));
		return true;
	} else
		return false;
}
