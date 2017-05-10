#ifndef _ROMPATHCLEANER_H_
#define _ROMPATHCLEANER_H_

#include <QString>
#include <QStringList>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#include "ui_rompathcleaner.h"

#define QMC2_RPC_PATH_INDEX_ROMPATH		0
#define QMC2_RPC_PATH_INDEX_SEPARATOR		1
#define QMC2_RPC_PATH_INDEX_SELECT		2
#define QMC2_RPC_PATH_INDEX_CUSTOMPATH		3

#define QMC2_RPC_MODE_INDEX_DELETE		0
#define QMC2_RPC_MODE_INDEX_MOVE		1
#define QMC2_RPC_MODE_INDEX_DRYRUN		2

#define QMC2_RPC_STATUS_UPDATE			100 // update stats every 100 processed files

class RomPathCleanerThread : public QThread
{
	Q_OBJECT

	public:
		RomPathCleanerThread(QObject *parent = 0);
		~RomPathCleanerThread();

		bool active() { return m_active; }
		bool waiting() { return m_waiting; }
		bool paused() { return m_paused; }
		QWaitCondition &waitCondition() { return m_waitCondition; }
		void setCheckedPaths(const QStringList &cp) { m_checkedPaths = cp; }

	public slots:
		void requestExit() { m_exit = true; }
		void requestStop() { m_stop = true; }
		void pause() { m_paused = true; }
		void resume() { m_paused = false; }

	signals:
		void log(const QString &);
		void checkStarted();
		void checkFinished();
		void checkPaused();
		void checkResumed();
		void progressTextChanged(const QString &);
		void progressRangeChanged(int, int);
		void progressChanged(int);
		void statusUpdated(quint64, quint64, quint64, quint64, quint64);

	protected:
		void run();

	private:
		bool m_exit, m_stop, m_active, m_waiting, m_paused;
		quint64 m_filesProcessed, m_renamedFiles, m_obsoleteROMs, m_obsoleteDisks, m_invalidFiles;
		QMutex m_mutex;
		QWaitCondition m_waitCondition;
		QStringList m_checkedPaths;
 
		void recursiveFileList(const QString &, QStringList *);
};

class RomPathCleaner : public QWidget, public Ui::RomPathCleaner
{
	Q_OBJECT

       	public:
		RomPathCleaner(const QString &settingsKey, QWidget *parent = 0);
		~RomPathCleaner();

		RomPathCleanerThread *cleanerThread() { return m_cleanerThread; }

	public slots:
		void adjustIconSizes();
		void log(const QString &);
		void cleanerThread_checkStarted();
		void cleanerThread_checkFinished();
		void cleanerThread_checkPaused();
		void cleanerThread_checkResumed();
		void cleanerThread_progressTextChanged(const QString &);
		void cleanerThread_progressRangeChanged(int, int);
		void cleanerThread_progressChanged(int);
		void cleanerThread_statusUpdated(quint64, quint64, quint64, quint64, quint64);
		void on_pushButtonStartStop_clicked();
		void on_pushButtonPauseResume_clicked();
		void on_comboBoxCheckedPath_activated(int);
		void on_spinBoxMaxLogSize_valueChanged(int);

	protected:
		void hideEvent(QHideEvent *);

	private:
		RomPathCleanerThread *m_cleanerThread;
		QString m_settingsKey;
};

#endif
