#include <QSettings>
#include "imagechecker.h"
#include "preview.h"
#include "flyer.h"
#include "cabinet.h"
#include "controller.h"
#include "marquee.h"
#include "title.h"
#include "pcb.h"
#include "gamelist.h"
#include "qmc2main.h"
#include "options.h"
#include "macros.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern QMap<QString, QTreeWidgetItem *> qmc2GamelistItemMap;
extern QMap<QString, QTreeWidgetItem *> qmc2HierarchyItemMap;
extern QMap<QString, QTreeWidgetItem *> qmc2CategoryItemMap;
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
extern QMap<QString, QTreeWidgetItem *> qmc2VersionItemMap;
#endif
extern QMap<QString, QIcon> qmc2IconMap;
extern QSettings *qmc2Config;
extern Options *qmc2Options;
extern Gamelist *qmc2Gamelist;
extern QAbstractItemView::ScrollHint qmc2CursorPositioningMode;
extern bool qmc2ImageCheckActive;
extern bool qmc2StopParser;
extern bool qmc2IconsPreloaded;
extern Preview *qmc2Preview;
extern Flyer *qmc2Flyer;
extern Cabinet *qmc2Cabinet;
extern Controller *qmc2Controller;
extern Marquee *qmc2Marquee;
extern Title *qmc2Title;
extern PCB *qmc2PCB;

//#define QMC2_DEBUG

ImageCheckerThread::ImageCheckerThread(int tNum, ImageWidget *imgWidget, QObject *parent)
	: QThread(parent)
{
	threadNumber = tNum;
	imageWidget = imgWidget;
	isActive = exitThread = false;
	zip = NULL;
}

ImageCheckerThread::~ImageCheckerThread()
{
	// NOP
}

QString ImageCheckerThread::humanReadable(quint64 value)
{
	QString humanReadableString;
	qreal humanReadableValue;
	QLocale locale;

#if __WORDSIZE == 64
	if ( (qreal)value / (qreal)QMC2_ONE_KILOBYTE < (qreal)QMC2_ONE_KILOBYTE ) {
		humanReadableValue = (qreal)value / (qreal)QMC2_ONE_KILOBYTE;
		humanReadableString = locale.toString(humanReadableValue, 'f', 2) + QString(tr(" KB"));
	} else if ( (qreal)value / (qreal)QMC2_ONE_MEGABYTE < (qreal)QMC2_ONE_KILOBYTE ) {
		humanReadableValue = (qreal)value / (qreal)QMC2_ONE_MEGABYTE;
		humanReadableString = locale.toString(humanReadableValue, 'f', 2) + QString(tr(" MB"));
	} else if ( (qreal)value / (qreal)QMC2_ONE_GIGABYTE < (qreal)QMC2_ONE_KILOBYTE ) {
		humanReadableValue = (qreal)value / (qreal)QMC2_ONE_GIGABYTE;
		humanReadableString = locale.toString(humanReadableValue, 'f', 2) + QString(tr(" GB"));
	} else {
		humanReadableValue = (qreal)value / (qreal)QMC2_ONE_TERABYTE;
		humanReadableString = locale.toString(humanReadableValue, 'f', 2) + QString(tr(" TB"));
	}
#else
	if ( (qreal)value / (qreal)QMC2_ONE_KILOBYTE < (qreal)QMC2_ONE_KILOBYTE ) {
		humanReadableValue = (qreal)value / (qreal)QMC2_ONE_KILOBYTE;
		humanReadableString = locale.toString(humanReadableValue, 'f', 2) + QString(tr(" KB"));
	} else if ( (qreal)value / (qreal)QMC2_ONE_MEGABYTE < (qreal)QMC2_ONE_KILOBYTE ) {
		humanReadableValue = (qreal)value / (qreal)QMC2_ONE_MEGABYTE;
		humanReadableString = locale.toString(humanReadableValue, 'f', 2) + QString(tr(" MB"));
	} else {
		humanReadableValue = (qreal)value / (qreal)QMC2_ONE_GIGABYTE;
		humanReadableString = locale.toString(humanReadableValue, 'f', 2) + QString(tr(" GB"));
	}
#endif

	return humanReadableString;
}

void ImageCheckerThread::run()
{
	emit log(tr("Thread[%1]: started").arg(threadNumber));

	if ( imageWidget->useZip() ) {
		QString zipFileName = imageWidget->imageZip();
		zip = unzOpen(zipFileName.toLocal8Bit());
		if ( zip ) {
			emit log(tr("Thread[%1]: ZIP file '%2' successfully opened").arg(threadNumber).arg(zipFileName));
		} else {
			emit log(tr("Thread[%1]: failed opening ZIP file '%2'").arg(threadNumber).arg(zipFileName));
			exitThread = true;
		}
	}

	while ( !exitThread && !qmc2StopParser ) {
		emit log(tr("Thread[%1]: waiting for work").arg(threadNumber));

		mutex.lock();
		isActive = false;
		waitCondition.wait(&mutex);
		isActive = true;
		mutex.unlock();

		if ( !exitThread && !qmc2StopParser ) {
			if ( workUnitMutex.tryLock() ) {
				emit log(tr("Thread[%1]: processing work unit with %n entries", "", workUnit.count()).arg(threadNumber));
				foundList.clear();
				missingList.clear();
				foreach (QString gameName, workUnit) {
					if ( exitThread || qmc2StopParser )
						break;
					QString fileName;
					QSize imageSize;
					int byteCount;
					if ( imageWidget->checkImage(gameName, zip, &imageSize, &byteCount, &fileName) ) {
						foundList << gameName;
						emit log(tr("Thread[%1]: image for '%2' found, loaded from '%3', size = %4x%5, bytes = %6").arg(threadNumber).arg(gameName).arg(fileName).arg(imageSize.width()).arg(imageSize.height()).arg(humanReadable(byteCount)));
					} else {
						missingList << gameName;
						emit log(tr("Thread[%1]: image for '%2' is missing").arg(threadNumber).arg(gameName));
					}
				}
				workUnit.clear();
				workUnitMutex.unlock();
				emit resultsReady(foundList, missingList);
			}
		}
	}

	if ( zip ) {
		unzClose(zip);
		emit log(tr("Thread[%1]: ZIP file '%2' closed").arg(threadNumber).arg(imageWidget->imageZip()));
	}

	emit log(tr("Thread[%1]: ended").arg(threadNumber));
}

ImageChecker::ImageChecker(QWidget *parent)
#if defined(Q_WS_WIN)
	: QDialog(parent, Qt::Dialog)
#else
	: QDialog(parent, Qt::Dialog | Qt::SubWindow)
#endif
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ImageChecker::ImageChecker(QWidget *parent = %1)").arg((qulonglong)parent));
#endif

	setupUi(this);

	startStopClicked = isRunning = false;
	passNumber = 0;
	labelFound->setText(tr("Found:") + " 0");
	labelMissing->setText(tr("Missing:") + " 0");
	labelObsolete->setText(tr("Obsolete:") + " 0");
	connect(&updateTimer, SIGNAL(timeout()), this, SLOT(updateResults()));

	comboBoxImageType->clear();
#if defined(QMC2_EMUTYPE_MESS)
	comboBoxImageType->insertItem(QMC2_IMGCHK_INDEX_PREVIEW, QIcon(QString::fromUtf8(":/data/img/camera.png")), tr("Previews"));
	comboBoxImageType->insertItem(QMC2_IMGCHK_INDEX_FLYER, QIcon(QString::fromUtf8(":/data/img/thumbnail.png")), tr("Flyers"));
	comboBoxImageType->insertItem(QMC2_IMGCHK_INDEX_CABINET, QIcon(QString::fromUtf8(":/data/img/arcadecabinet.png")), tr("Cabinets"));
	comboBoxImageType->insertItem(QMC2_IMGCHK_INDEX_LOGO, QIcon(QString::fromUtf8(":/data/img/marquee.png")), tr("Logos"));
	comboBoxImageType->insertItem(QMC2_IMGCHK_INDEX_PCB, QIcon(QString::fromUtf8(":/data/img/circuit.png")), tr("PCBs"));
	comboBoxImageType->insertSeparator(QMC2_IMGCHK_INDEX_SEPARATOR);
	comboBoxImageType->insertItem(QMC2_IMGCHK_INDEX_ICON, QIcon(QString::fromUtf8(":/data/img/icon.png")), tr("Icons"));
#else
	comboBoxImageType->insertItem(QMC2_IMGCHK_INDEX_PREVIEW, QIcon(QString::fromUtf8(":/data/img/camera.png")), tr("Previews"));
	comboBoxImageType->insertItem(QMC2_IMGCHK_INDEX_FLYER, QIcon(QString::fromUtf8(":/data/img/thumbnail.png")), tr("Flyers"));
	comboBoxImageType->insertItem(QMC2_IMGCHK_INDEX_CABINET, QIcon(QString::fromUtf8(":/data/img/arcadecabinet.png")), tr("Cabinets"));
	comboBoxImageType->insertItem(QMC2_IMGCHK_INDEX_CONTROLLER, QIcon(QString::fromUtf8(":/data/img/joystick.png")), tr("Controllers"));
	comboBoxImageType->insertItem(QMC2_IMGCHK_INDEX_MARQUEE, QIcon(QString::fromUtf8(":/data/img/marquee.png")), tr("Marquees"));
	comboBoxImageType->insertItem(QMC2_IMGCHK_INDEX_TITLE, QIcon(QString::fromUtf8(":/data/img/arcademode.png")), tr("Titles"));
	comboBoxImageType->insertItem(QMC2_IMGCHK_INDEX_PCB, QIcon(QString::fromUtf8(":/data/img/circuit.png")), tr("PCBs"));
	comboBoxImageType->insertSeparator(QMC2_IMGCHK_INDEX_SEPARATOR);
	comboBoxImageType->insertItem(QMC2_IMGCHK_INDEX_ICON, QIcon(QString::fromUtf8(":/data/img/icon.png")), tr("Icons"));
#endif
}

ImageChecker::~ImageChecker()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageChecker::~ImageChecker()");
#endif

}

void ImageChecker::adjustIconSizes()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageChecker::adjustIconSizes()");
#endif

	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFontMetrics fm(f);
	QSize iconSize = QSize(fm.height() - 2, fm.height() - 2);
	QSize iconSizeMiddle = iconSize + QSize(2, 2);
	QSize iconSizeLarge = iconSize + QSize(4, 4);

	comboBoxImageType->setIconSize(iconSize);
	toolButtonSelectSets->setIconSize(iconSize);
	toolButtonStartStop->setIconSize(iconSize);
	toolButtonRemoveObsolete->setIconSize(iconSize);

	QFont logFont;
	logFont.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/LogFont").toString());
	plainTextEditLog->setFont(logFont);
}

void ImageChecker::on_listWidgetFound_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageChecker::on_listWidgetFound_itemSelectionChanged()");
#endif

	if ( toolButtonSelectSets->isChecked() ) {
		QList<QListWidgetItem *> items = listWidgetFound->selectedItems();
		if ( items.count() > 0 )
			selectItem(items[0]->text());
	}
}

void ImageChecker::on_listWidgetMissing_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageChecker::on_listWidgetMissing_itemSelectionChanged()");
#endif

	if ( toolButtonSelectSets->isChecked() ) {
		QList<QListWidgetItem *> items = listWidgetMissing->selectedItems();
		if ( items.count() > 0 )
			selectItem(items[0]->text());
	}
}

void ImageChecker::startStop()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageChecker::startStop()");
#endif

	if ( isRunning ) {
		foreach (ImageCheckerThread *thread, threadMap) {
			thread->exitThread = true;
			thread->waitCondition.wakeAll();
			thread->quit();
			thread->wait();
			delete thread;
		}
		threadMap.clear();
		if ( passNumber == 2 && !startStopClicked ) {
			progressBar->setRange(0, qmc2GamelistItemMap.count());
			progressBar->setValue(0);
			progressBar->setFormat(tr("Pass #%1").arg(passNumber));
			bufferedObsoleteList.clear();
			QTimer::singleShot(0, this, SLOT(checkObsoleteFiles()));
			updateTimer.start(QMC2_CHECK_UPDATE_FAST);
		} else if ( passNumber == -1 || startStopClicked ) {
			isRunning = false;
			qmc2ImageCheckActive = false;
			toolButtonStartStop->setIcon(QIcon(QString::fromUtf8(":/data/img/refresh.png")));
			updateResults();
			progressBar->setRange(0, 100);
			progressBar->setValue(0);
			progressBar->setFormat(tr("Idle"));
			updateTimer.stop();
			enableWidgets(true);
			passNumber = 0;
		}
	} else {
		threadMap.clear();
		passNumber = 0;
		plainTextEditLog->clear();
		ImageWidget *imageWidget;
		switch ( comboBoxImageType->currentIndex() ) {
			case QMC2_IMGCHK_INDEX_PREVIEW:
				imageWidget = qmc2Preview;
				break;
			case QMC2_IMGCHK_INDEX_FLYER:
				imageWidget = qmc2Flyer;
				break;
			case QMC2_IMGCHK_INDEX_CABINET:
				imageWidget = qmc2Cabinet;
				break;
			case QMC2_IMGCHK_INDEX_MARQUEE:
				imageWidget = qmc2Marquee;
				break;
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
			case QMC2_IMGCHK_INDEX_CONTROLLER:
				imageWidget = qmc2Controller;
				break;
			case QMC2_IMGCHK_INDEX_TITLE:
				imageWidget = qmc2Title;
				break;
#endif
			case QMC2_IMGCHK_INDEX_PCB:
				imageWidget = qmc2PCB;
				break;
			case QMC2_IMGCHK_INDEX_ICON:
			default:
				imageWidget = NULL;
				break;
		}
		qmc2StopParser = false;
		enableWidgets(false);
		if ( imageWidget ) {
			// images
			for (int t = 0; t < spinBoxThreads->value(); t++) {
				ImageCheckerThread *thread = new ImageCheckerThread(t, imageWidget, this);
				connect(thread, SIGNAL(log(const QString &)), this, SLOT(log(const QString &)));
				connect(thread, SIGNAL(resultsReady(const QStringList &, const QStringList &)), this, SLOT(resultsReady(const QStringList &, const QStringList &)));
				threadMap[t] = thread;
				thread->start();
			}
		}
		isRunning = true;
		toolButtonStartStop->setIcon(QIcon(QString::fromUtf8(":/data/img/halt.png")));
		qmc2ImageCheckActive = true;
		listWidgetFound->clear();
		labelFound->setText(tr("Found:") + " 0");
		listWidgetMissing->clear();
		labelMissing->setText(tr("Missing:") + " 0");
		listWidgetObsolete->clear();
		labelObsolete->setText(tr("Obsolete:") + " 0");
		progressBar->setRange(0, qmc2GamelistItemMap.count());
		progressBar->setValue(0);
		passNumber = 1;
		progressBar->setFormat(tr("Pass #%1").arg(passNumber));
		bufferedFoundList.clear();
		bufferedMissingList.clear();
		QTimer::singleShot(0, this, SLOT(feedWorkerThreads()));
		updateTimer.start(QMC2_CHECK_UPDATE_FAST);
	}
}

void ImageChecker::on_toolButtonStartStop_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageChecker::on_toolButtonStartStop_clicked()");
#endif

	startStopClicked = true;
	startStop();
	startStopClicked = false;
}

void ImageChecker::enableWidgets(bool enable)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ImageChecker::enableWidgets(bool enable = %1)").arg(enable ? "true" : "false"));
#endif

	switch ( comboBoxImageType->currentIndex() ) {
		case QMC2_IMGCHK_INDEX_PREVIEW:
			qmc2Options->stackedWidgetPreview->setEnabled(enable);
			qmc2Options->radioButtonPreviewSelect->setEnabled(enable);
			break;
		case QMC2_IMGCHK_INDEX_FLYER:
			qmc2Options->stackedWidgetFlyer->setEnabled(enable);
			qmc2Options->radioButtonFlyerSelect->setEnabled(enable);
			break;
		case QMC2_IMGCHK_INDEX_CABINET:
			qmc2Options->stackedWidgetCabinet->setEnabled(enable);
			qmc2Options->radioButtonCabinetSelect->setEnabled(enable);
			break;
		case QMC2_IMGCHK_INDEX_MARQUEE:
			qmc2Options->stackedWidgetMarquee->setEnabled(enable);
			qmc2Options->radioButtonMarqueeSelect->setEnabled(enable);
			break;
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
		case QMC2_IMGCHK_INDEX_CONTROLLER:
			qmc2Options->stackedWidgetController->setEnabled(enable);
			qmc2Options->radioButtonControllerSelect->setEnabled(enable);
			break;
		case QMC2_IMGCHK_INDEX_TITLE:
			qmc2Options->stackedWidgetTitle->setEnabled(enable);
			qmc2Options->radioButtonTitleSelect->setEnabled(enable);
			break;
#endif
		case QMC2_IMGCHK_INDEX_PCB:
			qmc2Options->stackedWidgetPCB->setEnabled(enable);
			qmc2Options->radioButtonPCBSelect->setEnabled(enable);
			break;
		case QMC2_IMGCHK_INDEX_ICON:
			qmc2Options->stackedWidgetIcon->setEnabled(enable);
			qmc2Options->radioButtonIconSelect->setEnabled(enable);
			break;
	}
	comboBoxImageType->setEnabled(enable);
	if ( comboBoxImageType->currentIndex() == QMC2_IMGCHK_INDEX_ICON )
		spinBoxThreads->setEnabled(false);
	else
		spinBoxThreads->setEnabled(enable);
}

void ImageChecker::feedWorkerThreads()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageChecker::feedWorkerThreads()");
#endif

	QMapIterator<QString, QTreeWidgetItem *> it(qmc2GamelistItemMap);
	int lastThreadID = -1;
#ifdef QMC2_DEBUG
	int count = 0;
#endif
	while ( it.hasNext() && qmc2ImageCheckActive && !qmc2StopParser ) {
		if ( !threadMap.isEmpty() ) {
			// images
			int selectedThread = -1;
			if ( threadMap.count() > 1 ) {
				for (int t = lastThreadID + 1; t < threadMap.count() && selectedThread == -1; t++)
					if ( !threadMap[t]->isActive )
						selectedThread = t;
				for (int t = 0; t < lastThreadID && selectedThread == -1; t++)
					if ( !threadMap[t]->isActive )
						selectedThread = t;
			} else
				selectedThread = 0;
			if ( selectedThread >= 0 ) {
				if ( threadMap[selectedThread]->workUnitMutex.tryLock() ) {
					QStringList workUnit;
					while ( it.hasNext() && qmc2ImageCheckActive && workUnit.count() < QMC2_IMGCHK_WORKUNIT_SIZE && !qmc2StopParser ) {
						it.next();
						workUnit << it.key();
					}
					threadMap[selectedThread]->workUnitMutex.unlock();
					if ( qmc2ImageCheckActive ) {
						threadMap[selectedThread]->workUnit += workUnit;
						threadMap[selectedThread]->waitCondition.wakeAll();
#ifdef QMC2_DEBUG
						count += workUnit.count();
						log(QString("DEBUG: ImageChecker::feedWorkerThreads(): count = %1").arg(count));
#endif
					}
				}
				lastThreadID = selectedThread;
			}
		} else {
			// icons
			qmc2IconMap.clear();
			qmc2IconsPreloaded = false;
			int itemCount = 0;
			bool firstCheck = true;
			qmc2MainWindow->progressBarGamelist->setRange(0, qmc2GamelistItemMap.count());
			qmc2MainWindow->progressBarGamelist->setFormat("");
			log(tr("Thread[%1]: Icon check started").arg(0));
			while ( it.hasNext() && qmc2ImageCheckActive && !qmc2StopParser ) {
				it.next();
				QString gameName = it.key();
				if ( qmc2Gamelist->loadIcon(gameName, NULL, true, NULL) ) {
					log(tr("Thread[%1]: Icon for '%2' found").arg(0).arg(gameName));
					bufferedFoundList << gameName;
				} else {
					log(tr("Thread[%1]: Icon for '%2' is missing").arg(0).arg(gameName));
					bufferedMissingList << gameName;
				}
				if ( firstCheck ) {
					qmc2MainWindow->progressBarGamelist->reset();
					firstCheck = false;
				}
				if ( itemCount++ % 50 == 0 )
					qApp->processEvents();
			}
			while ( qmc2ImageCheckActive && !qmc2StopParser )
				qApp->processEvents();
			log(tr("Thread[%1]: Icon check ended").arg(0));
			qmc2MainWindow->progressBarGamelist->reset();
		}
		qApp->processEvents();
	}
}

void ImageChecker::on_toolButtonRemoveObsolete_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageChecker::on_toolButtonRemoveObsolete_clicked()");
#endif

	// FIXME
}

void ImageChecker::on_comboBoxImageType_currentIndexChanged(int index)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ImageChecker::on_comboBoxImageType_currentIndexChanged(int index = %1)").arg(index));
#endif

	spinBoxThreads->setEnabled(index != QMC2_IMGCHK_INDEX_ICON);
}

void ImageChecker::log(const QString &message)
{
	plainTextEditLog->appendPlainText(QTime::currentTime().toString("hh:mm:ss.zzz") + ": " + message);
}

void ImageChecker::resultsReady(const QStringList &foundList, const QStringList &missingList)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageChecker::resultsReady(const QStringList &foundList = ..., const QStringList &missingList = ...)");
#endif

	bufferedFoundList += foundList;
	bufferedMissingList += missingList;
	progressBar->setValue(progressBar->value() + foundList.count() + missingList.count());
	qApp->processEvents();
}

void ImageChecker::checkObsoleteFiles()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageChecker::checkObsoleteFiles()");
#endif

	passNumber = -1;
	QTimer::singleShot(0, this, SLOT(startStop()));
}

void ImageChecker::updateResults()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageChecker::updateResults()");
#endif

	listWidgetFound->insertItems(listWidgetFound->count(), bufferedFoundList);
	listWidgetMissing->insertItems(listWidgetMissing->count(), bufferedMissingList);
	listWidgetObsolete->insertItems(listWidgetObsolete->count(), bufferedObsoleteList);
	bufferedFoundList.clear();
	bufferedMissingList.clear();
	bufferedObsoleteList.clear();
	labelFound->setText(tr("Found:") + " " + QString::number(listWidgetFound->count()));
	labelMissing->setText(tr("Missing:") + " " + QString::number(listWidgetMissing->count()));
	labelObsolete->setText(tr("Obsolete:") + " " + QString::number(listWidgetObsolete->count()));

	qApp->processEvents();

	if ( listWidgetFound->count() + listWidgetMissing->count() >= qmc2GamelistItemMap.count() && isRunning ) {
		passNumber = 2;
		QTimer::singleShot(0, this, SLOT(startStop()));
	} else {
		int runCount = 0;
		foreach (ImageCheckerThread *thread, threadMap)
			if ( !thread->exitThread )
				runCount++;
		if ( (runCount == 0 || qmc2StopParser) && isRunning && !threadMap.isEmpty() ) {
			passNumber = -1;
			QTimer::singleShot(0, this, SLOT(startStop()));
		}
	}

	if ( threadMap.isEmpty() && passNumber == 1 )
		progressBar->setValue(listWidgetFound->count() + listWidgetMissing->count());
}

void ImageChecker::selectItem(QString setName)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ImageChecker::selectItem(QString setName = %1)").arg(setName));
#endif

	switch ( qmc2MainWindow->stackedWidgetView->currentIndex() ) {
		case QMC2_VIEWGAMELIST_INDEX: {
			QTreeWidgetItem *gameItem = qmc2GamelistItemMap[setName];
			if ( gameItem ) {
				qmc2MainWindow->treeWidgetGamelist->clearSelection();
				qmc2MainWindow->treeWidgetGamelist->setCurrentItem(gameItem);
				qmc2MainWindow->treeWidgetGamelist->scrollToItem(gameItem, qmc2CursorPositioningMode);
				gameItem->setSelected(true);
			}
			break;
		}

		case QMC2_VIEWHIERARCHY_INDEX: {
			QTreeWidgetItem *hierarchyItem = qmc2HierarchyItemMap[setName];
			if ( hierarchyItem ) {
				qmc2MainWindow->treeWidgetHierarchy->clearSelection();
				qmc2MainWindow->treeWidgetHierarchy->setCurrentItem(hierarchyItem);
				qmc2MainWindow->treeWidgetHierarchy->scrollToItem(hierarchyItem, qmc2CursorPositioningMode);
				hierarchyItem->setSelected(true);
			}
			break;
		}

		case QMC2_VIEWCATEGORY_INDEX: {
			QTreeWidgetItem *categoryItem = qmc2CategoryItemMap[setName];
			if ( categoryItem ) {
				qmc2MainWindow->treeWidgetCategoryView->clearSelection();
				qmc2MainWindow->treeWidgetCategoryView->setCurrentItem(categoryItem);
				qmc2MainWindow->treeWidgetCategoryView->scrollToItem(categoryItem, qmc2CursorPositioningMode);
				categoryItem->setSelected(true);
			}
			break;
		}

#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
		case QMC2_VIEWVERSION_INDEX: {
			QTreeWidgetItem *versionItem = qmc2VersionItemMap[setName];
			if ( versionItem ) {
				qmc2MainWindow->treeWidgetVersionView->clearSelection();
				qmc2MainWindow->treeWidgetVersionView->setCurrentItem(versionItem);
				qmc2MainWindow->treeWidgetVersionView->scrollToItem(versionItem, qmc2CursorPositioningMode);
				versionItem->setSelected(true);
			}
			break;
		}
#endif
	}
}

void ImageChecker::closeEvent(QCloseEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ImageChecker::closeEvent(QCloseEvent *e = %1)").arg((qulonglong)e));
#endif

	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/ImageChecker/Geometry", saveGeometry());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ImageChecker/ImageType", comboBoxImageType->currentIndex());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ImageChecker/Threads", spinBoxThreads->value());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ImageChecker/SelectSets", toolButtonSelectSets->isChecked());

	if ( e )
		QDialog::closeEvent(e);
}

void ImageChecker::hideEvent(QHideEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ImageChecker::hideEvent(QHideEvent *e = %1)").arg((qulonglong)e));
#endif

	closeEvent(NULL);
	QDialog::hideEvent(e);
}

void ImageChecker::showEvent(QShowEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ImageChecker::showEvent(QShowEvent *e = %1)").arg((qulonglong)e));
#endif

	adjustIconSizes();

	restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/ImageChecker/Geometry", QByteArray()).toByteArray());
	comboBoxImageType->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ImageChecker/ImageType", QMC2_IMGCHK_INDEX_PREVIEW).toInt());
	spinBoxThreads->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ImageChecker/Threads", 1).toInt());
	toolButtonSelectSets->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ImageChecker/SelectSets", true).toBool());

	QDialog::showEvent(e);
}

void ImageChecker::recursiveFileList(const QString &sDir, QStringList &fileNames)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ImageChecker::recursiveFileList(const QString& sDir = %1, QStringList &fileNames)").arg(sDir));
#endif

	QDir dir(sDir);
	QFileInfoList list = dir.entryInfoList();
	int i;
	for (i = 0; i < list.count(); i++) {
		QFileInfo info = list[i];
		QString path = info.filePath();
		if ( info.isDir() ) {
			// directory recursion
			if ( info.fileName() != ".." && info.fileName() != "." ) {
				recursiveFileList(path, fileNames);
				qApp->processEvents();
			}
		} else
			fileNames << path;
	}
}
