#ifndef _ROMPATHCLEANER_H_
#define _ROMPATHCLEANER_H_

#include <QString>
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

	protected:
		void run();

	private:
		bool m_exit;
		bool m_stop;
		bool m_active;
		bool m_waiting;
		bool m_paused;
		QMutex m_mutex;
		QWaitCondition m_waitCondition;
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
		void on_pushButtonStartStop_clicked();
		void on_pushButtonPauseResume_clicked();
		void on_comboBoxCheckedPath_activated(int);
		void on_spinBoxMaxLogSize_valueChanged(int);

	signals:

	private:
		RomPathCleanerThread *m_cleanerThread;
		QString m_settingsKey;
};

#endif
