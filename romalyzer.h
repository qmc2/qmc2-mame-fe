#ifndef _ROMALYZER_H_
#define _ROMALYZER_H_

#include <QtGui>
#include <QtXml>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QTime>
#include <QTimer>
#include <QHash>
#include <QPixmap>
#include <QStringList>

#include "macros.h"
#include "checksumdbmgr.h"
#include "checksumscannerlog.h"
#include "collectionrebuilder.h"
#include "ui_romalyzer.h"

#define QMC2_ROMALYZER_PAGE_REPORT		0
#define QMC2_ROMALYZER_PAGE_LOG			1
#define QMC2_ROMALYZER_PAGE_SETTINGS		2
#define QMC2_ROMALYZER_PAGE_CSWIZ		3
#define QMC2_ROMALYZER_PAGE_RCR			4

#define QMC2_ROMALYZER_COLUMN_GAME		0
#define QMC2_ROMALYZER_COLUMN_MERGE		1
#define QMC2_ROMALYZER_COLUMN_TYPE		2
#define QMC2_ROMALYZER_COLUMN_EMUSTATUS		3
#define QMC2_ROMALYZER_COLUMN_FILESTATUS	4
#define QMC2_ROMALYZER_COLUMN_SIZE		5
#define QMC2_ROMALYZER_COLUMN_CRC		6
#define QMC2_ROMALYZER_COLUMN_SHA1		7
#define QMC2_ROMALYZER_COLUMN_MD5		8

#define QMC2_ROMALYZER_CSWIZ_COLUMN_ID		0
#define QMC2_ROMALYZER_CSWIZ_COLUMN_FILENAME	1
#define QMC2_ROMALYZER_CSWIZ_COLUMN_STATUS	2
#define QMC2_ROMALYZER_CSWIZ_COLUMN_TYPE	3
#define QMC2_ROMALYZER_CSWIZ_COLUMN_PATH	4

#define QMC2_ROMALYZER_CSWIZ_AMLVL_0		0
#define QMC2_ROMALYZER_CSWIZ_AMLVL_NONE		QMC2_ROMALYZER_CSWIZ_AMLVL_0
#define QMC2_ROMALYZER_CSWIZ_AMLVL_1		1
#define QMC2_ROMALYZER_CSWIZ_AMLVL_SELECT	QMC2_ROMALYZER_CSWIZ_AMLVL_1
#define QMC2_ROMALYZER_CSWIZ_AMLVL_2		2
#define QMC2_ROMALYZER_CSWIZ_AMLVL_ANALYZE	QMC2_ROMALYZER_CSWIZ_AMLVL_2
#define QMC2_ROMALYZER_CSWIZ_AMLVL_3		3
#define QMC2_ROMALYZER_CSWIZ_AMLVL_REPAIR	QMC2_ROMALYZER_CSWIZ_AMLVL_3

#define QMC2_ROMALYZER_CSWIZ_HASHTYPE_SHA1	0
#define QMC2_ROMALYZER_CSWIZ_HASHTYPE_CRC	1

#define QMC2_ROMALYZER_MERGE_STATUS_OK		0
#define QMC2_ROMALYZER_MERGE_STATUS_WARN	1
#define QMC2_ROMALYZER_MERGE_STATUS_CRIT	2

#define QMC2_ROMALYZER_EMUSTATUS_GOOD		0x00000001
#define QMC2_ROMALYZER_EMUSTATUS_NODUMP		0x00000020
#define QMC2_ROMALYZER_EMUSTATUS_BADDUMP	0x00000400
#define QMC2_ROMALYZER_EMUSTATUS_UNKNOWN	0x00008000

#define QMC2_ROMALYZER_PAUSE_TIMEOUT		250
#define QMC2_ROMALYZER_FLASH_TIME		100
#define QMC2_ROMALYZER_XMLPOSCACHE_SIZE		1000
#define QMC2_ROMALYZER_SEARCH_RESPONSE		5000
#define QMC2_ROMALYZER_EXPORT_RESPONSE		10
#define QMC2_ROMALYZER_CKSUM_SEARCH_RESPONSE	500

#define QMC2_ROMALYZER_FILE_TOO_BIG		"QMC2_FILE_TOO_BIG"
#define QMC2_ROMALYZER_FILE_ERROR		"QMC2_FILE_ERROR"
#define QMC2_ROMALYZER_FILE_NOT_SUPPORTED	"QMC2_FILE_NOT_SUPPORTED"
#define QMC2_ROMALYZER_FILE_NOT_FOUND		"QMC2_FILE_NOT_FOUND"
#define QMC2_ROMALYZER_NO_DUMP			"QMC2_NO_DUMP"

#define QMC2_ROMALYZER_ZIP_BUFFER_SIZE		QMC2_ZIP_BUFFER_SIZE
#define QMC2_ROMALYZER_FILE_BUFFER_SIZE		QMC2_FILE_BUFFER_SIZE
#define QMC2_ROMALYZER_PROGRESS_THRESHOLD	QMC2_ONE_MEGABYTE

#define QMC2_CHD_HEADER_TAG_OFFSET		0
#define QMC2_CHD_HEADER_TAG_LENGTH		8
#define QMC2_CHD_HEADER_VERSION_OFFSET		12
#define QMC2_CHD_HEADER_FLAGS_OFFSET		16
#define QMC2_CHD_HEADER_FLAG_HASPARENT		0x00000001
#define QMC2_CHD_HEADER_FLAG_ALLOWSWRITES	0x00000002
#define QMC2_CHD_HEADER_COMPRESSION_OFFSET	20
#define QMC2_CHD_HEADER_COMPRESSION_NONE	0
#define QMC2_CHD_HEADER_COMPRESSION_ZLIB	1
#define QMC2_CHD_HEADER_COMPRESSION_ZLIB_PLUS	2
#define QMC2_CHD_HEADER_COMPRESSION_AV		3

#define QMC2_CHD_HEADER_V3_TAG_OFFSET		QMC2_CHD_HEADER_TAG_OFFSET
#define QMC2_CHD_HEADER_V3_TAG_LENGTH		QMC2_CHD_HEADER_TAG_LENGTH
#define QMC2_CHD_HEADER_V3_VERSION_OFFSET	QMC2_CHD_HEADER_VERSION_OFFSET
#define QMC2_CHD_HEADER_V3_TOTALHUNKS_OFFSET	24
#define QMC2_CHD_HEADER_V3_LOGICALBYTES_OFFSET	28
#define QMC2_CHD_HEADER_V3_MD5_OFFSET		44
#define QMC2_CHD_HEADER_V3_MD5_LENGTH		16
#define QMC2_CHD_HEADER_V3_PARENTMD5_OFFSET	60
#define QMC2_CHD_HEADER_V3_PARENTMD5_LENGTH	16
#define QMC2_CHD_HEADER_V3_HUNKBYTES_OFFSET	76
#define QMC2_CHD_HEADER_V3_SHA1_OFFSET		80
#define QMC2_CHD_HEADER_V3_SHA1_LENGTH		20
#define QMC2_CHD_HEADER_V3_PARENTSHA1_OFFSET	100
#define QMC2_CHD_HEADER_V3_PARENTSHA1_LENGTH	20
#define QMC2_CHD_HEADER_V3_LENGTH		120

#define QMC2_CHD_HEADER_V4_TAG_OFFSET		QMC2_CHD_HEADER_TAG_OFFSET
#define QMC2_CHD_HEADER_V4_TAG_LENGTH		QMC2_CHD_HEADER_TAG_LENGTH
#define QMC2_CHD_HEADER_V4_VERSION_OFFSET	QMC2_CHD_HEADER_VERSION_OFFSET
#define QMC2_CHD_HEADER_V4_TOTALHUNKS_OFFSET	24
#define QMC2_CHD_HEADER_V4_LOGICALBYTES_OFFSET	28
#define QMC2_CHD_HEADER_V4_HUNKBYTES_OFFSET	44
#define QMC2_CHD_HEADER_V4_SHA1_OFFSET		48
#define QMC2_CHD_HEADER_V4_SHA1_LENGTH		20
#define QMC2_CHD_HEADER_V4_PARENTSHA1_OFFSET	68
#define QMC2_CHD_HEADER_V4_PARENTSHA1_LENGTH	20
#define QMC2_CHD_HEADER_V4_RAWSHA1_OFFSET	88
#define QMC2_CHD_HEADER_V4_RAWSHA1_LENGTH	20
#define QMC2_CHD_HEADER_V4_LENGTH		108

#define QMC2_CHD_HEADER_V5_TAG_OFFSET		QMC2_CHD_HEADER_TAG_OFFSET
#define QMC2_CHD_HEADER_V5_TAG_LENGTH		QMC2_CHD_HEADER_TAG_LENGTH
#define QMC2_CHD_HEADER_V5_VERSION_OFFSET	QMC2_CHD_HEADER_VERSION_OFFSET
#define QMC2_CHD_HEADER_V5_COMPRESSORS_OFFSET	16
#define QMC2_CHD_HEADER_V5_COMPRESSORS_COUNT	4	// 4 x UINT32
#define QMC2_CHD_HEADER_V5_LOGICALBYTES_OFFSET	32
#define QMC2_CHD_HEADER_V5_HUNKBYTES_OFFSET	56
#define QMC2_CHD_HEADER_V5_UNITBYTES_OFFSET	60
#define QMC2_CHD_HEADER_V5_RAWSHA1_OFFSET	64
#define QMC2_CHD_HEADER_V5_RAWSHA1_LENGTH	20
#define QMC2_CHD_HEADER_V5_SHA1_OFFSET		84
#define QMC2_CHD_HEADER_V5_SHA1_LENGTH		20
#define QMC2_CHD_HEADER_V5_PARENTSHA1_OFFSET	104
#define QMC2_CHD_HEADER_V5_PARENTSHA1_LENGTH	20
#define QMC2_CHD_HEADER_V5_LENGTH		124

#define QMC2_CHD_CURRENT_VERSION		5
#define QMC2_CHD_CHECK_NULL_SHA1(ba)		((ba).startsWith(QByteArray("00000000000000000000")))

#define wizardAutomationLevel			comboBoxChecksumWizardAutomationLevel->currentIndex()
#define crcToString(crc)			QString::number((crc), 16).rightJustified(8, '0')

#define QMC2_CHECKSUM_SCANNER_FILE_UNKNOWN	-2
#define QMC2_CHECKSUM_SCANNER_FILE_NO_ACCESS	-1
#define QMC2_CHECKSUM_SCANNER_FILE_ZIP		0
#define QMC2_CHECKSUM_SCANNER_FILE_7Z		1
#define QMC2_CHECKSUM_SCANNER_FILE_CHD		2
#define QMC2_CHECKSUM_SCANNER_FILE_REGULAR	3
#define QMC2_CHECKSUM_SCANNER_FILE_ARCHIVE	4

#define QMC2_CHECKSUM_DB_QUERY_STATUS_UNKNOWN	-1
#define QMC2_CHECKSUM_DB_QUERY_STATUS_GOOD	0
#define QMC2_CHECKSUM_DB_QUERY_STATUS_BAD	1

#define QMC2_ROMALYZER_REBUILD_ANIM_SPEED	500

class CheckSumScannerThread : public QThread
{
	Q_OBJECT

	public:
		bool exitThread;
		bool stopScan;
		bool isActive;
		bool isWaiting;
		bool isPaused;
		bool pauseRequested;
		bool scanIncrementally;
		bool deepScan;
		bool useHashCache;
#if defined(QMC2_LIBARCHIVE_ENABLED)
		bool useLibArchive;
#endif
		QMutex mutex;
		QMutex logSyncMutex;
		QWaitCondition waitCondition;
		QStringList scannedPaths;
		QTime scanTimer;

		CheckSumScannerThread(CheckSumScannerLog *scannerLog, QString settingsKey, QObject *parent = 0);
		~CheckSumScannerThread();

		CheckSumDatabaseManager *checkSumDb() { return m_checkSumDb; }
		quint64 pendingUpdates() { return m_pendingUpdates; }
		QString status();
		void reopenCheckSumDb();
		int fileType(QString);
		void prepareIncrementalScan(QStringList *fileList);
		QString scanTime();
		void emitlog(QString);
		void flushMessageQueue();

	public slots:
		void pause();
		void resume();

	signals:
		void log(const QString &);
		void scanStarted();
		void scanFinished();
		void scanPaused();
		void scanResumed();
		void progressTextChanged(const QString &);
		void progressRangeChanged(int, int);
		void progressChanged(int);

	protected:
		void run();

	private:
		CheckSumDatabaseManager *m_checkSumDb;
		CheckSumScannerLog *m_scannerLog;
		quint64 m_pendingUpdates;
		bool m_preparingIncrementalScan;
		QString m_settingsKey;
		QHash<QString, bool> m_hashCache;
		QStringList m_queuedMessages;
		bool checkSumExists(QString sha1, QString crc, quint64 size = 0);
		void recursiveFileList(const QString &, QStringList *);
		bool scanZip(QString, QStringList *, QList<quint64> *, QStringList *, QStringList *);
		bool scanSevenZip(QString, QStringList *, QList<quint64> *, QStringList *, QStringList *);
#if defined(QMC2_LIBARCHIVE_ENABLED)
		bool scanArchive(QString, QStringList *, QList<quint64> *, QStringList *, QStringList *);
#endif
		bool scanChd(QString, quint64 *, QString *);
		bool scanRegularFile(QString, quint64 *, QString *, QString *);
};

class ROMAlyzerXmlHandler : public QXmlDefaultHandler
{
	public:
		QString currentText;
		QTreeWidgetItem *parentItem;
		QTreeWidgetItem *childItem;
		QList<QTreeWidgetItem *> childItems;
		QStringList deviceReferences;
		QStringList optionalROMs;
		bool autoExpand;
		bool autoScroll;
		int emuStatus;
		int fileCounter;
		int romalyzerMode;
		QBrush redBrush;
		QBrush greenBrush;
		QBrush blueBrush;
		QBrush yellowBrush;
		QBrush brownBrush;
		QBrush greyBrush;

		ROMAlyzerXmlHandler(QTreeWidgetItem *, bool expand = false, bool scroll = false, int mode = QMC2_ROMALYZER_MODE_SYSTEM);

		bool startElement(const QString &, const QString &, const QString &, const QXmlAttributes &);
		bool endElement(const QString &, const QString &, const QString &);
		bool characters(const QString &);
};

class ROMAlyzer : public QDialog, public Ui::ROMAlyzer
{
	Q_OBJECT

	public:
		QTimer animTimer;
		QTimer checkSumDbStatusTimer;
		QTime miscTimer;
		int animSeq;
		QStringList romPaths;
		QStringList chdCompressionTypes;
		QMap<QString, QString> chdCompressionTypesV5;
		bool chdManagerRunning;
		bool chdManagerMD5Success;
		bool chdManagerSHA1Success;
		quint64 chdManagerCurrentHunk;
		quint64 chdManagerTotalHunks;
		QMenu *romFileContextMenu;
		QMenu *romSetContextMenu;
		QMenu *toolsMenu;
		QAction *actionRewriteSet;
		QAction *actionAnalyzeDeviceRefs;
		QAction *actionImportFromDataFile;
		QAction *actionExportToDataFile;
		QAction *actionCopyBadToClipboard;
		QString currentFilesSHA1Checksum;
		QString currentFilesCrcChecksum;
		quint64 currentFilesSize;
		QStringList wizardSelectedSets;
		QStringList analyzerBadSets;
		QMultiMap<QString, QStringList> setRewriterFileMap;
		QString setRewriterSetName;
		QTreeWidgetItem *setRewriterItem;
		int setRewriterSetCount;
		bool wizardSearch;
		bool quickSearch;
		qint64 lastRowCount;

		ROMAlyzer(QWidget *, int romalyzerMode = QMC2_ROMALYZER_MODE_SYSTEM);
		~ROMAlyzer();

		void saveState() { closeEvent(NULL); }
		bool readAllZipData(QString, QMap<QString, QByteArray> *, QMap<QString, QString> *, QStringList *fileList = NULL);
		bool readSevenZipFileData(QString, QString, QByteArray *);
		bool readZipFileData(QString, QString, QByteArray *);
		bool readFileData(QString, QString, QByteArray *);
		bool writeAllZipData(QString, QMap<QString, QByteArray> *, bool writeLog = false, QProgressBar *pBar = NULL);
		bool writeAllFileData(QString, QMap<QString, QByteArray> *, bool writeLog = false, QProgressBar *pBar = NULL);
		static QString humanReadable(quint64, int digits = 2);
		static QString &getXmlData(QString, bool includeDTD = false);
		static QString &getSoftwareXmlData(QString, QString, bool includeDTD = false);
		QString &getEffectiveFile(QTreeWidgetItem *, QString, QString, QString, QString, QString, QString, QString, QByteArray *, QString *, QString *, bool *, bool *, bool *, int, QString *, bool, bool *);
		bool createBackup(QString filePath);
		CheckSumDatabaseManager *checkSumDb() { return m_checkSumDb; }
		CheckSumScannerLog *checkSumScannerLog() { return m_checkSumScannerLog; }
		CheckSumScannerThread *checkSumScannerThread() { return m_checkSumScannerThread; }
		CollectionRebuilder *collectionRebuilder() { return m_collectionRebuilder; }
		int mode() { return m_currentMode; }
		void setMode(int mode);
		bool active() { return m_active; }
		void setActive(bool active) { m_active = active; }
		bool paused() { return m_paused; }
		void setPaused(bool paused) { m_paused = paused; }
		QString settingsKey() { return m_settingsKey; }
		bool rebuilderActive() { return collectionRebuilder() ? collectionRebuilder()->active() : false; }

	public slots:
		// callback functions
		void on_pushButtonAnalyze_clicked();
		void on_pushButtonPause_clicked();
		void on_pushButtonClose_clicked();
		void on_pushButtonSearchForward_clicked();
		void on_pushButtonSearchBackward_clicked();
		void on_lineEditSoftwareLists_textChanged(QString);
		void on_lineEditSets_textChanged(QString);
		void on_treeWidgetChecksums_itemSelectionChanged();
		void on_spinBoxMaxLogSize_valueChanged(int);
		void on_toolButtonBrowseBackupFolder_clicked();
		void on_toolButtonBrowseCHDManagerExecutableFile_clicked();
		void on_toolButtonBrowseTemporaryWorkingDirectory_clicked();
		void on_toolButtonBrowseSetRewriterOutputPath_clicked();
		void on_toolButtonBrowseSetRewriterAdditionalRomPath_clicked();
		void on_toolButtonSaveLog_clicked();
		void on_checkBoxCalculateCRC_toggled(bool);
		void on_checkBoxCalculateMD5_toggled(bool);
		void on_checkBoxCalculateSHA1_toggled(bool);
		void on_comboBoxChecksumWizardHashType_currentIndexChanged(int);
		void on_lineEditChecksumWizardHash_textChanged(const QString &);
		void lineEditChecksumWizardHash_textChanged_delayed();
		void on_pushButtonChecksumWizardSearch_clicked();
		void on_treeWidgetChecksums_customContextMenuRequested(const QPoint &);
		void on_treeWidgetChecksumWizardSearchResult_itemSelectionChanged();
		void on_pushButtonChecksumWizardAnalyzeSelectedSets_clicked();
		void on_pushButtonChecksumWizardRepairBadSets_clicked();
		void on_tabWidgetAnalysis_currentChanged(int);
		void on_toolButtonCheckSumDbAddPath_clicked();
		void on_toolButtonCheckSumDbRemovePath_clicked();
		void on_lineEditCheckSumDbDatabasePath_textChanged(const QString &);
		void on_toolButtonBrowseCheckSumDbDatabasePath_clicked();
		void on_toolButtonCheckSumDbViewLog_clicked();
		void on_pushButtonCheckSumDbScan_clicked();
		void on_pushButtonCheckSumDbPauseResumeScan_clicked();
		void on_listWidgetCheckSumDbScannedPaths_customContextMenuRequested(const QPoint &);
		void on_listWidgetCheckSumDbScannedPaths_itemSelectionChanged();
		void checkSumScannerLog_windowClosed();
		void checkSumScannerLog_windowOpened();
		void checkSumScannerThread_scanStarted();
		void checkSumScannerThread_scanFinished();
		void checkSumScannerThread_scanPaused();
		void checkSumScannerThread_scanResumed();
		void switchToCollectionRebuilder();

		// miscellaneous slots
		void animationTimeout();
		void analyze();
		void selectItem(QString);
		void enableSearchEdit() { lineEditSearchString->setEnabled(true); }
		void adjustIconSizes();
		void runChecksumWizard();
		void runSetRewriter();
		void copyToClipboard(bool onlyBadOrMissing = false);
		void copyBadToClipboard();
		void analyzeDeviceRefs();
		void importFromDataFile();
		void exportToDataFile();
		void updateCheckSumDbStatus();
		void log(const QString &);
		void indicateCheckSumDbQueryStatusGood();
		void indicateCheckSumDbQueryStatusBad();
		void indicateCheckSumDbQueryStatusUnknown();
		void softwareListLoadFinished(bool);
		void on_groupBoxSetRewriter_toggled(bool);
		void on_groupBoxCheckSumDatabase_toggled(bool);

		// CHD manager process control
		void chdManagerStarted();
		void chdManagerFinished(int, QProcess::ExitStatus);
		void chdManagerReadyReadStandardOutput();
		void chdManagerReadyReadStandardError();
		void chdManagerError(QProcess::ProcessError);

	protected:
		void closeEvent(QCloseEvent *);
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);
		void moveEvent(QMoveEvent *);
		void resizeEvent(QResizeEvent *);

	private:
		CheckSumDatabaseManager *m_checkSumDb;
		CheckSumScannerLog *m_checkSumScannerLog;
		CheckSumScannerThread *m_checkSumScannerThread;
		QPixmap m_checkSumDbQueryStatusPixmap;
		QTimer m_checkSumTextChangedTimer;
		CollectionRebuilder *m_collectionRebuilder;
		int m_currentMode;
		bool m_active;
		bool m_paused;
		QString m_settingsKey;
};

#endif
