#ifndef _COLLECTIONREBUILDER_H_
#define _COLLECTIONREBUILDER_H_

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QFile>
#include <QIcon>
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
		void reopenDatabase();
		bool parseXml(QString, QString *, QStringList *, QStringList *, QStringList *, QStringList *, QStringList *, QStringList *, QStringList *);
		bool nextId(QString *, QStringList *, QStringList *, QStringList *, QStringList *, QStringList *, QStringList *, QStringList *);
		void checkpointRestart(qint64 checkpoint);
		bool rewriteSet(QString *, QStringList *, QStringList *, QStringList *, QStringList *, QStringList *, QStringList *);
		bool writeAllFileData(QString, QString, QStringList *, QStringList *, QStringList *, QStringList *, QStringList *, QStringList *);
		bool writeAllZipData(QString, QString, QStringList *, QStringList *, QStringList *, QStringList *, QStringList *, QStringList *);
		bool readFileData(QString, QByteArray *);
		bool readSevenZipFileData(QString, QString, QByteArray *);
		bool readZipFileData(QString, QString, QByteArray *);
		bool createBackup(QString filePath);
		void setFilterExpression(QString, int, int);
		void setFilterExpressionSoftware(QString, int, int);
		void clearFilterExpression() { setFilterExpression(QString(), 0, 0); }
		void clearFilterExpressionSoftware() { setFilterExpressionSoftware(QString(), 0, 0); }
		void setStateFilter(bool enableFilter, bool stateC = true, bool stateM = true, bool stateI = true, bool stateN = true, bool stateU = true) {
			doFilterState = enableFilter; includeStateC = stateC; includeStateM = stateM; includeStateI = stateI; includeStateN = stateN; includeStateU = stateU;
		}
		void clearStateFilter() { setStateFilter(false); }

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
};

class CollectionRebuilder : public QDialog, public Ui::CollectionRebuilder
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

	protected:
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);
		void closeEvent(QCloseEvent *);
		void keyPressEvent(QKeyEvent *);

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
};

#endif

#include "romalyzer.h"
