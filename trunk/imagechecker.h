#ifndef _IMAGECHECKER_H_
#define _IMAGECHECKER_H_

#include <QMap>
#include <QThread>
#include <QTimer>
#include <QMutex>
#include <QWaitCondition>
#include <QStringList>
#include "unzip.h"

#include "ui_imagechecker.h"
#include "imagewidget.h"

class ImageCheckerThread : public QThread
{
	Q_OBJECT

	public:
		bool exitThread;
		bool isActive;
		int threadNumber;
		unzFile zip;
		QStringList workUnit;
		QStringList foundList;
		QStringList missingList;
		ImageWidget *imageWidget;
		QMutex workUnitMutex;
		QMutex mutex;
		QWaitCondition waitCondition;

		ImageCheckerThread(int, ImageWidget *, QObject *parent = 0);
		~ImageCheckerThread();

	protected:
		void run();

	signals:
		void log(const QString &);
		void resultsReady(const QStringList &, const QStringList &);
};

class ImageChecker : public QDialog, public Ui::ImageChecker
{
	Q_OBJECT

	public:
		bool isRunning;
		QStringList bufferedFoundList;
		QStringList bufferedMissingList;
		QTimer updateTimer;
		QMap<int, ImageCheckerThread *> threadMap;

		ImageChecker(QWidget *parent = 0);
		~ImageChecker();

	public slots:
		void on_listWidgetFound_itemSelectionChanged();
		void on_listWidgetMissing_itemSelectionChanged();
		void on_toolButtonStartStop_clicked();
		void on_toolButtonRemoveObsolete_clicked();
		void on_comboBoxImageType_currentIndexChanged(int);
		void selectItem(QString);
		void adjustIconSizes();
		void log(const QString &);
		void resultsReady(const QStringList &, const QStringList &);
		void feedWorkerThreads();
		void updateResults();
		void enableWidgets(bool);

	protected:
		void closeEvent(QCloseEvent *);
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);
};

#endif
