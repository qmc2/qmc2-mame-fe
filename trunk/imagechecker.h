#ifndef _IMAGECHECKER_H_
#define _IMAGECHECKER_H_

#include <QMap>
#include <QThread>
#include <QTimer>
#include <QElapsedTimer>
#include <QMutex>
#include <QWaitCondition>
#include <QStringList>
#include <QRegExp>

#include "ui_imagechecker.h"
#include "imagewidget.h"
#include "softwareimagewidget.h"
#include "unzip.h"
#include "sevenzipfile.h"
#if defined(QMC2_LIBARCHIVE_ENABLED)
#include "archivefile.h"
#endif
#include "comboboxwidget.h"

class ImageCheckerThread : public QThread
{
	Q_OBJECT

	public:
		bool exitThread;
		bool isActive;
		bool isWaiting;
		int threadNumber;
		quint64 scanCount;
		quint64 foundCount;
		quint64 missingCount;
		QMap<QString, unzFile> zipMap;
		QMap<QString, SevenZipFile *> sevenZipMap;
#if defined(QMC2_LIBARCHIVE_ENABLED)
		QMap<QString, ArchiveFile *> archiveMap;
#endif
		QStringList workUnit;
		QStringList foundList;
		QStringList missingList;
		QStringList badList;
		QStringList badFileList;
		ImageWidget *imageWidget;
		SoftwareImageWidget *softwareImageWidget;
		QMutex workUnitMutex;
		QMutex mutex;
		QWaitCondition waitCondition;

		ImageCheckerThread(int, ImageWidget *, SoftwareImageWidget *, QObject *parent = 0);
		~ImageCheckerThread();

		QString humanReadable(quint64);
		bool isFillingDictionary() { return m_isFillingDictionary; }

		void runSystemArtworkCheck();
		void runSoftwareArtworkCheck();

	public slots:
		void sevenZipDataReady();

	protected:
		void run();

	signals:
		void log(const QString &);
		void resultsReady(const QStringList &, const QStringList &, const QStringList &, const QStringList &);

	private:
		bool m_async;
		bool m_isFillingDictionary;
};

class ImageChecker : public QDialog, public Ui::ImageChecker
{
	Q_OBJECT

	public:
		bool isRunning;
		bool startStopClicked;
		double avgScanSpeed;
		quint64 foundCount;
		quint64 missingCount;
		quint64 badCount;
		QStringList bufferedFoundList;
		QStringList bufferedMissingList;
		QStringList bufferedObsoleteList;
		QStringList bufferedBadList;
		QStringList bufferedBadFileList;
		QTimer updateTimer;
		QTimer listWidgetFoundSelectionTimer;
		QTimer listWidgetMissingSelectionTimer;
		QElapsedTimer checkTimer;
		QMap<int, ImageCheckerThread *> threadMap;
		int passNumber;
		int currentImageType;
		QRegExp rxFourDigits;
		QRegExp rxCharsToEscape;
		QRegExp rxColonSepStr;
		ComboBoxWidget *comboBoxWidgetSoftwareLists;

		ImageChecker(QWidget *parent = 0);

		void recursiveFileList(const QString &, QStringList *);
		void recursiveZipList(unzFile, QStringList *, QString prependString = QString());
		void recursiveSevenZipList(SevenZipFile *, QStringList *, QString prependString = QString());
#if defined(QMC2_LIBARCHIVE_ENABLED)
		void recursiveArchiveList(ArchiveFile *, QStringList *, QString prependString = QString());
#endif

	public slots:
		void on_listWidgetFound_itemSelectionChanged();
		void listWidgetFound_itemSelectionChanged_delayed();
		void on_listWidgetMissing_itemSelectionChanged();
		void listWidgetMissing_itemSelectionChanged_delayed();
		void on_toolButtonStartStop_clicked();
		void on_toolButtonClear_clicked();
		void on_toolButtonSaveLog_clicked();
		void on_toolButtonRemoveObsolete_clicked();
		void on_toolButtonBad_toggled(bool);
		void on_toolButtonRemoveBad_clicked();
		void on_comboBoxImageType_currentIndexChanged(int);
		void selectItem(QString);
		void adjustIconSizes();
		void log(const QString &);
		void resultsReady(const QStringList &, const QStringList &, const QStringList &, const QStringList &);
		void feedWorkerThreads();
		void updateResults();
		void checkObsoleteFiles();
		void enableWidgets(bool);
		void startStop();
		void updateSoftwareLists();
		void updateCornerWidget();
		void softwareListLoadFinished(bool);

	protected:
		void closeEvent(QCloseEvent *);
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);
};

#endif
