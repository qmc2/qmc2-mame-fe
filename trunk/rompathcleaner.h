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

	public slots:
		void requestExit() { m_exit = true; }

	signals:
		void log(const QString &);

	protected:
		void run();

	private:
		bool m_exit;
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

		RomPathCleanerThread *romPathCleanerThread() { return m_romPathCleanerThread; }

	public slots:
		void adjustIconSizes();
		void log(const QString &);
		void on_pushButtonStartStop_clicked();
		void on_comboBoxCheckedPath_activated(int);

	signals:

	private:
		RomPathCleanerThread *m_romPathCleanerThread;
		QString m_settingsKey;
};

#endif
