#ifndef _FILEEDITWIDGET_H_
#define _FILEEDITWIDGET_H_

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#include "checksumdbmgr.h"
#include "ui_collectionrebuilder.h"

class CollectionRebuilder;

class CollectionRebuilderThread : public QThread
{
	Q_OBJECT

	public:
		bool exitThread;
		bool isActive;
		bool isWaiting;
		bool isPaused;
		bool pauseRequested;
		QMutex mutex;
		QWaitCondition waitCondition;

		CollectionRebuilderThread(QObject *parent = 0);
		~CollectionRebuilderThread();

		CheckSumDatabaseManager *checkSumDb() { return m_checkSumDb; }
		void reopenDatabase();

	public slots:
		void pause();
		void resume();

	signals:
		void log(const QString &);
		void rebuildStarted();
		void rebuildFinished();
		void rebuildPaused();
		void rebuildResumed();

	protected:
		void run();

	private:
		CheckSumDatabaseManager *m_checkSumDb;
		CollectionRebuilder *m_rebuilderDialog;
};

class CollectionRebuilder : public QDialog, public Ui::CollectionRebuilder
{
	Q_OBJECT

       	public:
		CollectionRebuilder(QWidget *parent = 0);
		~CollectionRebuilder();

		CollectionRebuilderThread *rebuilderThread() { return m_rebuilderThread; }

	public slots:
		void on_spinBoxMaxLogSize_valueChanged(int);
		void log(const QString &);
		void clear() { plainTextEditLog->clear(); }
		void scrollToEnd();
		void adjustIconSizes();
		void on_pushButtonStartStop_clicked();
		void on_pushButtonPauseResume_clicked();
		void on_comboBoxXmlSource_currentIndexChanged(int);
		void rebuilderThread_rebuildStarted();
		void rebuilderThread_rebuildFinished();
		void rebuilderThread_rebuildPaused();
		void rebuilderThread_rebuildResumed();

	protected:
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);
		void closeEvent(QCloseEvent *);
		void keyPressEvent(QKeyEvent *);

	private:
		CollectionRebuilderThread *m_rebuilderThread;
		QString m_defaultSetEntity, m_defaultRomEntity, m_defaultDiskEntity;
};

#endif
