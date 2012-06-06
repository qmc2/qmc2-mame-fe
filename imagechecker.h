#ifndef _IMAGECHECKER_H_
#define _IMAGECHECKER_H_

#include <QMap>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#include "ui_imagechecker.h"

#include "preview.h"
#include "flyer.h"
#include "cabinet.h"
#include "controller.h"
#include "marquee.h"
#include "title.h"
#include "pcb.h"
#include "gamelist.h"

class ImageCheckerThread : public QThread
{
	Q_OBJECT

	public:
		bool exitThread;
		int threadNumber;
		QMutex mutex;
		QWaitCondition waitCondition;
		QStringList checkList;
		QStringList foundList;
		QStringList missingList;

		ImageCheckerThread(int tNum, QObject *parent = 0);
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

	protected:
		void closeEvent(QCloseEvent *);
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);
};

#endif
