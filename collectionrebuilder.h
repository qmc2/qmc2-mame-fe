#ifndef _COLLECTIONREBUILDER_H_
#define _COLLECTIONREBUILDER_H_

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QFile>
#include <QIcon>
#include <QHash>
#include <QTimer>
#include <QRegExp>
#include <QString>
#include <QStringList>
#include <QPixmap>

#include "checksumdbmgr.h"
#include "xmldbmgr.h"
#include "swldbmgr.h"
#include "missingdumpsviewer.h"
#include "ui_collectionrebuilder.h"

class CollectionRebuilder;
class ROMAlyzer;

#define QMC2_COLLECTIONREBUILDER_FILETYPE_ZIP	0
#define QMC2_COLLECTIONREBUILDER_FILETYPE_7Z	1
#define QMC2_COLLECTIONREBUILDER_FILETYPE_CHD	2
#define QMC2_COLLECTIONREBUILDER_FILETYPE_FILE	3

#define QMC2_COLLECTIONREBUILDER_CHD_IGNORE	0
#define QMC2_COLLECTIONREBUILDER_CHD_HARDLINK	1
#define QMC2_COLLECTIONREBUILDER_CHD_SYMLINK	2
#define QMC2_COLLECTIONREBUILDER_CHD_COPY	3
#define QMC2_COLLECTIONREBUILDER_CHD_MOVE	4

class CollectionRebuilderThread : public QThread
{
	Q_OBJECT

	public:
		bool exitThread;
		bool isActive;
		bool isWaiting;
		bool isPaused;
		bool pauseRequested;
		bool stopRebuilding;
		bool doFilter;
		bool doFilterSoftware;
		bool isIncludeFilter;
		bool isIncludeFilterSoftware;
		bool doFilterState;
		bool includeStateC;
		bool includeStateM;
		bool includeStateI;
		bool includeStateN;
		bool includeStateU;
		bool exactMatch;
		bool exactMatchSoftware;
		bool useHashCache;
		bool dryRun;
		quint64 missingDisks;
		quint64 missingROMs;
		quint64 setsProcessed;
		QRegExp filterRx;
		QRegExp filterRxSoftware;
		QMutex mutex;
		QWaitCondition waitCondition;

		CollectionRebuilderThread(QObject *parent = 0);
		~CollectionRebuilderThread();

		CheckSumDatabaseManager *checkSumDb() { return m_checkSumDb; }
		CollectionRebuilder *rebuilderDialog() { return m_rebuilderDialog; }
		XmlDatabaseManager *xmlDb() { return m_xmlDb; }
		SoftwareListXmlDatabaseManager *swlDb() { return m_swlDb; }
		qint64 checkpoint() { return m_checkpoint; }
		QString currentListName() { return m_currentListName; }
		void setCheckpoint(qint64 cp, int xmlSourceIndex);
		void setListCheckpoint(QString list, int xmlSourceIndex);
		void reopenCheckSumDb();
		bool parseXml(QString, QString *, QStringList *, QStringList *, QStringList *, QStringList *, QStringList *, QStringList *, QStringList *);
		bool nextId(QString *, QStringList *, QStringList *, QStringList *, QStringList *, QStringList *, QStringList *, QStringList *);
		void checkpointRestart(qint64 checkpoint);
		bool rewriteSet(QString *, QStringList *, QStringList *, QStringList *, QStringList *, QStringList *, QStringList *);
		bool writeAllFileData(QString, QString, QStringList *, QStringList *, QStringList *, QStringList *, QStringList *, QStringList *);
		bool writeAllZipData(QString, QString, QStringList *, QStringList *, QStringList *, QStringList *, QStringList *, QStringList *);
#if defined(QMC2_LIBARCHIVE_ENABLED)
		bool writeAllArchiveData(QString, QString, QStringList *, QStringList *, QStringList *, QStringList *, QStringList *, QStringList *);
#endif
		bool readFileData(QString, QByteArray *);
		bool readSevenZipFileData(QString, QString, QString, QByteArray *);
		bool readZipFileData(QString, QString, QString, QByteArray *);
		bool hardlinkChds(QString, QString, QStringList *, QStringList *);
		bool symlinkChds(QString, QString, QStringList *, QStringList *);
		bool copyChds(QString, QString, QStringList *, QStringList *);
		bool moveChds(QString, QString, QStringList *, QStringList *);
		bool sameFileSystem(QString, QString);
		bool createBackup(QString filePath);
		void setFilterExpression(QString, int, int, bool);
		void setFilterExpressionSoftware(QString, int, int, bool);
		void clearFilterExpression() { setFilterExpression(QString(), 0, 0, false); }
		void clearFilterExpressionSoftware() { setFilterExpressionSoftware(QString(), 0, 0, false); }
		void setStateFilter(bool enableFilter, bool stateC = true, bool stateM = true, bool stateI = true, bool stateN = true, bool stateU = true) {
			doFilterState = enableFilter;
			includeStateC = stateC;
			includeStateM = stateM;
			includeStateI = stateI;
			includeStateN = stateN;
			includeStateU = stateU;
		}
		void clearStateFilter() { setStateFilter(false); }
		void setSetEntityPattern(QString pattern) { m_setEntityPattern = pattern; }
		QString &setEntityPattern() { return m_setEntityPattern; }
		void setRomEntityPattern(QString pattern) { m_romEntityPattern = pattern; }
		QString &romEntityPattern() { return m_romEntityPattern; }
		void setDiskEntityPattern(QString pattern) { m_diskEntityPattern = pattern; }
		QString &diskEntityPattern() { return m_diskEntityPattern; }
		void setSetEntityStartPattern(QString pattern) { m_setEntityStartPattern = pattern; }
		QString &setEntityStartPattern() { return m_setEntityStartPattern; }
		void setListEntityStartPattern(QString pattern) { m_listEntityStartPattern = pattern; }
		QString &listEntityStartPattern() { return m_listEntityStartPattern; }
		void setMerge(bool merge) { m_merge = merge; }
		bool merge() { return m_merge; }
		bool evaluateFilters(QString &setKey);

		static QString &toHumanReadable(QString &text) {
			foreach (QString old, m_replacementHash.keys())
				text.replace(old, m_replacementHash.value(old));
			return text;
		}

	public slots:
		void pause();
		void resume();

	signals:
		void log(const QString &);
		void rebuildStarted();
		void rebuildFinished();
		void rebuildPaused();
		void rebuildResumed();
		void progressTextChanged(const QString &);
		void progressRangeChanged(int, int);
		void progressChanged(int);
		void statusUpdated(quint64, quint64, quint64);
		void newMissing(QString, QString, QString, QString, QString, QString, QString);

	protected:
		void run();

	private:
		CheckSumDatabaseManager *m_checkSumDb;
		XmlDatabaseManager *m_xmlDb;
		SoftwareListXmlDatabaseManager *m_swlDb;
		CollectionRebuilder *m_rebuilderDialog;
		qint64 m_xmlIndex, m_xmlIndexCount, m_checkpoint;
		QFile m_xmlFile;
		QString m_currentListName;
		QString m_setEntityPattern;
		QString m_romEntityPattern;
		QString m_diskEntityPattern;
		QString m_setEntityStartPattern;
		QString m_listEntityStartPattern;
		bool m_merge;
		static QHash<QString, QString> m_replacementHash;
		static QStringList m_fileTypes;
		QHash<QString, bool> m_hashCache;
		uint m_hashCacheUpdateTime;
		bool checkSumExists(QString sha1, QString crc, quint64 size = 0);
		void updateHashCache();
};

class CollectionRebuilder : public QWidget, public Ui::CollectionRebuilder
{
	Q_OBJECT

       	public:
		CollectionRebuilder(ROMAlyzer *romAlyzer, QWidget *parent = 0);
		~CollectionRebuilder();

		CollectionRebuilderThread *rebuilderThread() { return m_rebuilderThread; }
		ROMAlyzer *romAlyzer() { return m_romAlyzer; }
		QString settingsKey() { return m_settingsKey; }
		QStringList &newMissingList() { return m_newMissingList; }
		MissingDumpsViewer *missingDumpsViewer() { return m_missingDumpsViewer; }

		bool defaultEmulator() { return m_defaultEmulator; }
		void setDefaultEmulator(bool enable) { m_defaultEmulator = enable; }
		void setStateFilterVisibility(bool visible);
		void showStateFilter() { setStateFilterVisibility(true); }
		void hideStateFilter() { setStateFilterVisibility(false); }
		bool active() { return rebuilderThread() ? rebuilderThread()->isActive : false; }
		void setIgnoreCheckpoint(bool ignore) { m_ignoreCheckpoint = ignore; }
		bool ignoreCheckpoint() { return m_ignoreCheckpoint; }

	public slots:
		void on_spinBoxMaxLogSize_valueChanged(int);
		void log(const QString &);
		void clear() { plainTextEditLog->clear(); }
		void scrollToEnd();
		void adjustIconSizes();
		void on_pushButtonStartStop_clicked();
		void on_pushButtonPauseResume_clicked();
		void on_comboBoxXmlSource_currentIndexChanged(int);
		void on_toolButtonRemoveXmlSource_clicked();
		void on_toolButtonViewMissingList_clicked();
		void rebuilderThread_rebuildStarted();
		void rebuilderThread_rebuildFinished();
		void rebuilderThread_rebuildPaused();
		void rebuilderThread_rebuildResumed();
		void rebuilderThread_progressTextChanged(const QString &);
		void rebuilderThread_progressRangeChanged(int, int);
		void rebuilderThread_progressChanged(int);
		void rebuilderThread_statusUpdated(quint64, quint64, quint64);
		void rebuilderThread_newMissing(QString, QString, QString, QString, QString, QString, QString);
		void animationTimer_timeout();
		void updateMissingList();
		void updateModeSetup();
		void on_comboBoxModeSwitch_currentIndexChanged(int);

	protected:
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);

	private:
		CollectionRebuilderThread *m_rebuilderThread;
		QString m_defaultSetEntity, m_defaultRomEntity, m_defaultDiskEntity;
		QIcon m_iconCheckpoint, m_iconNoCheckpoint;
		QTimer m_animationTimer;
		int m_animationSequence;
		ROMAlyzer *m_romAlyzer;
		QString m_settingsKey;
		QPixmap m_correctIconPixmap;
		QPixmap m_mostlyCorrectIconPixmap;
		QPixmap m_incorrectIconPixmap;
		QPixmap m_notFoundIconPixmap;
		QPixmap m_unknownIconPixmap;
		QStringList m_newMissingList;
		MissingDumpsViewer *m_missingDumpsViewer;
		bool m_defaultEmulator;
		bool m_ignoreCheckpoint;
};

#endif

#include "romalyzer.h"
