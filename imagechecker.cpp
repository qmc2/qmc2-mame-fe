#include <QFileDialog>
#include <QTest>
#include <QMap>
#include <QHash>
#include <QHashIterator>

#include "settings.h"
#include "imagechecker.h"
#include "preview.h"
#include "flyer.h"
#include "cabinet.h"
#include "controller.h"
#include "marquee.h"
#include "title.h"
#include "pcb.h"
#include "machinelist.h"
#include "qmc2main.h"
#include "options.h"
#include "toolexec.h"
#include "unzip.h"
#include "sevenzipfile.h"
#include "softwarelist.h"
#include "macros.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern QHash<QString, QTreeWidgetItem *> qmc2MachineListItemHash;
extern QHash<QString, QTreeWidgetItem *> qmc2HierarchyItemHash;
extern QHash<QString, QTreeWidgetItem *> qmc2CategoryItemHash;
extern QHash<QString, QTreeWidgetItem *> qmc2VersionItemHash;
extern QHash<QString, QIcon> qmc2IconHash;
extern Settings *qmc2Config;
extern Options *qmc2Options;
extern MachineList *qmc2MachineList;
extern QAbstractItemView::ScrollHint qmc2CursorPositioningMode;
extern bool qmc2ImageCheckActive;
extern bool qmc2StopParser;
extern bool qmc2IconsPreloaded;
extern bool qmc2UseIconFile;
extern Preview *qmc2Preview;
extern Flyer *qmc2Flyer;
extern Cabinet *qmc2Cabinet;
extern Controller *qmc2Controller;
extern Marquee *qmc2Marquee;
extern Title *qmc2Title;
extern PCB *qmc2PCB;
extern QMap<QString, unzFile> qmc2IconFileMap;
extern QMap<QString, SevenZipFile *> qmc2IconFileMap7z;
#if defined(QMC2_LIBARCHIVE_ENABLED)
extern QMap<QString, ArchiveFile *> qmc2IconArchiveMap;
#endif
extern SoftwareList *qmc2SoftwareList;

#define swlDb qmc2MainWindow->swlDb

ImageCheckerThread::ImageCheckerThread(int tNum, ImageWidget *imgWidget, SoftwareImageWidget *swImgWidget, QObject *parent)
	: QThread(parent)
{
	threadNumber = tNum;
	imageWidget = imgWidget;
	softwareImageWidget = swImgWidget;
	isActive = exitThread = isWaiting = false;
	scanCount = foundCount = missingCount = 0;
	m_async = false;
	m_isFillingDictionary = true;
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

void ImageCheckerThread::runSystemArtworkCheck()
{
	m_isFillingDictionary = false;
	if ( imageWidget->useZip() ) {
		foreach (QString zipFileName, imageWidget->imageZip().split(";", QString::SkipEmptyParts)) {
			zipMap[zipFileName] = unzOpen(zipFileName.toUtf8().constData());
			if ( zipMap[zipFileName] ) {
				emit log(tr("Thread[%1]: ZIP file '%2' successfully opened").arg(threadNumber).arg(zipFileName));
			} else {
				emit log(tr("Thread[%1]: failed opening ZIP file '%2'").arg(threadNumber).arg(zipFileName));
				exitThread = true;
			}
		}
	} else if ( imageWidget->useSevenZip() ) {
		foreach (QString sevenZipFileName, imageWidget->imageZip().split(";", QString::SkipEmptyParts)) {
			SevenZipFile *sevenZipFile = new SevenZipFile(sevenZipFileName);
			if ( !sevenZipFile->open() ) {
				emit log(tr("Thread[%1]: failed opening 7z file '%2'").arg(threadNumber).arg(sevenZipFileName));
				delete sevenZipFile;
				exitThread = true;
			} else {
				connect(sevenZipFile, SIGNAL(dataReady()), this, SLOT(sevenZipDataReady()));
				sevenZipMap[sevenZipFileName] = sevenZipFile;
				emit log(tr("Thread[%1]: 7z file '%2' successfully opened").arg(threadNumber).arg(sevenZipFileName));
			}
		}
		m_isFillingDictionary = true;
	}
#if defined(QMC2_LIBARCHIVE_ENABLED)
	else if ( imageWidget->useArchive() ) {
		foreach (QString archiveFileName, imageWidget->imageZip().split(";", QString::SkipEmptyParts)) {
			ArchiveFile *archiveFile = new ArchiveFile(archiveFileName);
			if ( !archiveFile->open() ) {
				emit log(tr("Thread[%1]: failed opening archive file '%2'").arg(threadNumber).arg(archiveFileName));
				delete archiveFile;
				exitThread = true;
			} else {
				archiveMap[archiveFileName] = archiveFile;
				emit log(tr("Thread[%1]: archive file '%2' successfully opened").arg(threadNumber).arg(archiveFileName));
			}
		}
	}
#endif
	while ( !exitThread && !qmc2StopParser ) {
		emit log(tr("Thread[%1]: waiting for work").arg(threadNumber));
		mutex.lock();
		isWaiting = true;
		isActive = false;
		waitCondition.wait(&mutex);
		isActive = true;
		isWaiting = false;
		mutex.unlock();
		if ( !exitThread && !qmc2StopParser ) {
			if ( workUnitMutex.tryLock(QMC2_IMGCHK_WU_MUTEX_LOCK_TIMEOUT) ) {
				emit log(tr("Thread[%1]: processing work unit with %n entries", "", workUnit.count()).arg(threadNumber));
				foundList.clear();
				missingList.clear();
				foreach (QString gameName, workUnit) {
					if ( exitThread || qmc2StopParser )
						break;
					QString fileName;
					QSize imageSize;
					int byteCount;
					QString readerError;
					if ( imageWidget->useZip() ) {
						// zipped images
						int zlCount = 0;
						foreach (unzFile zip, zipMap) {
							zlCount++;
							QString zipFilePath = zipMap.key(zip);
							readerError.clear();
#if defined(QMC2_LIBARCHIVE_ENABLED)
							bool ok = imageWidget->checkImage(gameName, zip, NULL, NULL, &imageSize, &byteCount, &fileName, &readerError);
#else
							bool ok = imageWidget->checkImage(gameName, zip, NULL, &imageSize, &byteCount, &fileName, &readerError);
#endif
							if ( ok ) {
								foundList << gameName;
								emit log(tr("Thread[%1]: image for '%2' found, loaded from '%3', size = %4x%5, bytes = %6").arg(threadNumber).arg(gameName).arg(zipFilePath + ": " + fileName).arg(imageSize.width()).arg(imageSize.height()).arg(humanReadable(byteCount)));
								foundCount++;
								break;
							} else {
								if ( zlCount < zipMap.count() ) {
									if ( !readerError.isEmpty() ) {
										emit log(tr("Thread[%1]: image for '%2' loaded from '%3' is bad, error = '%4'").arg(threadNumber).arg(gameName).arg(zipFilePath + ": " + fileName).arg(readerError));
										badList << gameName;
										badFileList << zipFilePath + ": " + fileName;
									}
								} else {
									missingList << gameName;
									if ( !readerError.isEmpty() ) {
										emit log(tr("Thread[%1]: image for '%2' loaded from '%3' is bad, error = '%4'").arg(threadNumber).arg(gameName).arg(zipFilePath + ": " + fileName).arg(readerError));
										badList << gameName;
										badFileList << zipFilePath + ": " + fileName;
									} else
										emit log(tr("Thread[%1]: image for '%2' is missing").arg(threadNumber).arg(gameName));
									missingCount++;
								}
							}
						}
					} else if ( imageWidget->useSevenZip() ) {
						// 7-zipped images
						int szlCount = 0;
						foreach (SevenZipFile *sevenZipFile, sevenZipMap) {
							szlCount++;
							QString sevenZipFilePath = sevenZipMap.key(sevenZipFile);
							readerError.clear();
							m_async = true;
							bool checkReturn = false;
							int waitCounter = 0;
#if defined(QMC2_LIBARCHIVE_ENABLED)
							if ( !m_isFillingDictionary )
								checkReturn = imageWidget->checkImage(gameName, NULL, sevenZipFile, NULL, &imageSize, &byteCount, &fileName, &readerError, &m_async, &m_isFillingDictionary);
#else
							if ( !m_isFillingDictionary )
								checkReturn = imageWidget->checkImage(gameName, NULL, sevenZipFile, &imageSize, &byteCount, &fileName, &readerError, &m_async, &m_isFillingDictionary);
#endif
							else while ( m_async && m_isFillingDictionary && !exitThread ) {
#if defined(QMC2_LIBARCHIVE_ENABLED)
								checkReturn = imageWidget->checkImage(gameName, NULL, sevenZipFile, NULL, &imageSize, &byteCount, &fileName, &readerError, &m_async, &m_isFillingDictionary);
#else
								checkReturn = imageWidget->checkImage(gameName, NULL, sevenZipFile, &imageSize, &byteCount, &fileName, &readerError, &m_async, &m_isFillingDictionary);
#endif
								if ( checkReturn && m_isFillingDictionary && m_async ) {
									if ( waitCounter++ % 100 == 0 )
										emit log(tr("Thread[%1]: decompressing archive").arg(threadNumber));
									QTest::qWait(25);
								} else
									break;
							}
							if ( checkReturn ) {
								foundList << gameName;
								emit log(tr("Thread[%1]: image for '%2' found, loaded from '%3', size = %4x%5, bytes = %6").arg(threadNumber).arg(gameName).arg(sevenZipFilePath + ": " + fileName).arg(imageSize.width()).arg(imageSize.height()).arg(humanReadable(byteCount)));
								foundCount++;
								break;
							} else {
								if ( szlCount < sevenZipMap.count() ) {
									if ( !readerError.isEmpty() ) {
										emit log(tr("Thread[%1]: image for '%2' loaded from '%3' is bad, error = '%4'").arg(threadNumber).arg(gameName).arg(sevenZipFilePath + ": " + fileName).arg(readerError));
										badList << gameName;
										badFileList << sevenZipFilePath + ": " + fileName;
									}
								} else {
									missingList << gameName;
									if ( !readerError.isEmpty() ) {
										emit log(tr("Thread[%1]: image for '%2' loaded from '%3' is bad, error = '%4'").arg(threadNumber).arg(gameName).arg(sevenZipFilePath + ": " + fileName).arg(readerError));
										badList << gameName;
										badFileList << sevenZipFilePath + ": " + fileName;
									} else
										emit log(tr("Thread[%1]: image for '%2' is missing").arg(threadNumber).arg(gameName));
									missingCount++;
								}
							}
						}
					}
#if defined(QMC2_LIBARCHIVE_ENABLED)
					else if ( imageWidget->useArchive() ) {
						// archived images
						int alCount = 0;
						foreach (ArchiveFile *archiveFile, archiveMap) {
							alCount++;
							QString archiveFilePath = archiveMap.key(archiveFile);
							readerError.clear();
							if ( imageWidget->checkImage(gameName, NULL, NULL, archiveFile, &imageSize, &byteCount, &fileName, &readerError) ) {
								foundList << gameName;
								emit log(tr("Thread[%1]: image for '%2' found, loaded from '%3', size = %4x%5, bytes = %6").arg(threadNumber).arg(gameName).arg(archiveFilePath + ": " + fileName).arg(imageSize.width()).arg(imageSize.height()).arg(humanReadable(byteCount)));
								foundCount++;
								break;
							} else {
								if ( alCount < archiveMap.count() ) {
									if ( !readerError.isEmpty() ) {
										emit log(tr("Thread[%1]: image for '%2' loaded from '%3' is bad, error = '%4'").arg(threadNumber).arg(gameName).arg(archiveFilePath + ": " + fileName).arg(readerError));
										badList << gameName;
										badFileList << archiveFilePath + ": " + fileName;
									}
								} else {
									missingList << gameName;
									if ( !readerError.isEmpty() ) {
										emit log(tr("Thread[%1]: image for '%2' loaded from '%3' is bad, error = '%4'").arg(threadNumber).arg(gameName).arg(archiveFilePath + ": " + fileName).arg(readerError));
										badList << gameName;
										badFileList << archiveFilePath + ": " + fileName;
									} else
										emit log(tr("Thread[%1]: image for '%2' is missing").arg(threadNumber).arg(gameName));
									missingCount++;
								}
							}
						}
					}
#endif
					else {
						// unzipped images
#if defined(QMC2_LIBARCHIVE_ENABLED)
						bool ok = imageWidget->checkImage(gameName, NULL, NULL, NULL, &imageSize, &byteCount, &fileName, &readerError);
#else
						bool ok = imageWidget->checkImage(gameName, NULL, NULL, &imageSize, &byteCount, &fileName, &readerError);
#endif
						if ( ok ) {
							foundList << gameName;
							emit log(tr("Thread[%1]: image for '%2' found, loaded from '%3', size = %4x%5, bytes = %6").arg(threadNumber).arg(gameName).arg(fileName).arg(imageSize.width()).arg(imageSize.height()).arg(humanReadable(byteCount)));
							foundCount++;
						} else {
							missingList << gameName;
							if ( !readerError.isEmpty() ) {
								emit log(tr("Thread[%1]: image for '%2' loaded from '%3' is bad, error = '%4'").arg(threadNumber).arg(gameName).arg(fileName).arg(readerError));
								badList << gameName;
								badFileList << fileName;
							} else
								emit log(tr("Thread[%1]: image for '%2' is missing").arg(threadNumber).arg(gameName));
							missingCount++;
						}
					}
					scanCount++;
					// it's possible that the work-unit grows above the 'limit' so we need to report our intermediate results in order to update the GUI
					if ( foundList.count() > QMC2_IMGCHK_WORKUNIT_SIZE || missingList.count() > QMC2_IMGCHK_WORKUNIT_SIZE ) {
						emit resultsReady(foundList, missingList, badList, badFileList);
						foundList.clear();
						missingList.clear();
						badList.clear();
						badFileList.clear();
					}
				}
				workUnit.clear();
				workUnitMutex.unlock();
				if ( !foundList.isEmpty() || !missingList.isEmpty() ) {
					emit resultsReady(foundList, missingList, badList, badFileList);
					foundList.clear();
					missingList.clear();
					badList.clear();
					badFileList.clear();
				}
			}
		}
	}
}

void ImageCheckerThread::runSoftwareArtworkCheck()
{
	// FIXME
}

void ImageCheckerThread::run()
{
	emit log(tr("Thread[%1]: started").arg(threadNumber));
	if ( imageWidget )
		runSystemArtworkCheck();
	else if ( softwareImageWidget )
		runSoftwareArtworkCheck();
	foreach (unzFile zip, zipMap) {
		unzClose(zip);
		emit log(tr("Thread[%1]: ZIP file '%2' closed").arg(threadNumber).arg(zipMap.key(zip)));
	}
	foreach (SevenZipFile *sevenZipFile, sevenZipMap) {
		sevenZipFile->close();
		emit log(tr("Thread[%1]: 7z file '%2' closed").arg(threadNumber).arg(sevenZipMap.key(sevenZipFile)));
		delete sevenZipFile;
	}
#if defined(QMC2_LIBARCHIVE_ENABLED)
	foreach (ArchiveFile *archiveFile, archiveMap) {
		archiveFile->close();
		emit log(tr("Thread[%1]: archive file '%2' closed").arg(threadNumber).arg(archiveMap.key(archiveFile)));
		delete archiveFile;
	}
#endif
	emit log(tr("Thread[%1]: ended").arg(threadNumber));
}

void ImageCheckerThread::sevenZipDataReady()
{
	SevenZipFile *sevenZipFile = (SevenZipFile *)sender();
	if ( sevenZipFile ) {
		m_async = false;
		emit log(tr("Thread[%1]: finished decompressing archive").arg(threadNumber));
	}
}

ImageChecker::ImageChecker(QWidget *parent)
#if defined(QMC2_OS_WIN)
	: QDialog(parent, Qt::Dialog)
#else
	: QDialog(parent, Qt::Dialog | Qt::SubWindow)
#endif
{
	setupUi(this);

	labelStatus->setText(tr("Idle"));
	progressBar->setRange(-1, -1);
	progressBar->setValue(-1);

	rxFourDigits = QRegExp("^\\d{4}$");
	rxCharsToEscape = QRegExp("(\\s|\\\\|\\(|\\))");
	rxColonSepStr = QRegExp("^.*\\: ");

	startStopClicked = isRunning = false;
	currentImageType = QMC2_IMGCHK_INDEX_NONE;
	avgScanSpeed = 0.0;
	foundCount = missingCount = badCount = passNumber = 0;
	labelFound->setText(tr("Found:") + " 0");
	labelMissing->setText(tr("Missing:") + " 0");
	toolButtonBad->setText(tr("Bad:") + " 0");
	toolButtonBad->setChecked(false);
	toolButtonBad->setEnabled(false);
	toolButtonRemoveBad->setEnabled(false);
	toolButtonRemoveObsolete->setEnabled(false);
	labelObsolete->setText(tr("Obsolete:") + " 0");
	connect(&updateTimer, SIGNAL(timeout()), this, SLOT(updateResults()));

	listWidgetFoundSelectionTimer.setSingleShot(true);
	connect(&listWidgetFoundSelectionTimer, SIGNAL(timeout()), this, SLOT(listWidgetFound_itemSelectionChanged_delayed()));
	listWidgetMissingSelectionTimer.setSingleShot(true);
	connect(&listWidgetMissingSelectionTimer, SIGNAL(timeout()), this, SLOT(listWidgetMissing_itemSelectionChanged_delayed()));
 
	if ( !swlDb ) {
		swlDb = new SoftwareListXmlDatabaseManager(qmc2MainWindow);
		swlDb->setSyncMode(QMC2_DB_SYNC_MODE_OFF);
		swlDb->setJournalMode(QMC2_DB_JOURNAL_MODE_MEMORY);
	}

	QStringList softwareLists = swlDb->uniqueSoftwareLists();
	if ( softwareLists.isEmpty() )
		comboBoxWidgetSoftwareLists = new ComboBoxWidget(QStringList(), tr("No software information"), QString(), tr("Update database"), tr("Click to update the software list XML database now"));
	else
		comboBoxWidgetSoftwareLists = new ComboBoxWidget(softwareLists, tr("Select software list"), tr("Select the software list for which the image check should run"), QString(), QString());
	connect(comboBoxWidgetSoftwareLists->pushButton, SIGNAL(clicked()), this, SLOT(updateSoftwareLists()));
	tabWidget->setCornerWidget(comboBoxWidgetSoftwareLists);

#if !defined(QMC2_WIP_ENABLED)
	// FIXME
	comboBoxWidgetSoftwareLists->setVisible(false);
#endif

	comboBoxImageType->clear();
	comboBoxImageType->insertItem(QMC2_IMGCHK_INDEX_PREVIEW, QIcon(QString::fromUtf8(":/data/img/camera.png")), tr("Previews"));
	comboBoxImageType->insertItem(QMC2_IMGCHK_INDEX_FLYER, QIcon(QString::fromUtf8(":/data/img/thumbnail.png")), tr("Flyers"));
	comboBoxImageType->insertItem(QMC2_IMGCHK_INDEX_CABINET, QIcon(QString::fromUtf8(":/data/img/arcadecabinet.png")), tr("Cabinets"));
	comboBoxImageType->insertItem(QMC2_IMGCHK_INDEX_CONTROLLER, QIcon(QString::fromUtf8(":/data/img/joystick.png")), tr("Controllers"));
	comboBoxImageType->insertItem(QMC2_IMGCHK_INDEX_MARQUEE, QIcon(QString::fromUtf8(":/data/img/marquee.png")), tr("Marquees"));
	comboBoxImageType->insertItem(QMC2_IMGCHK_INDEX_TITLE, QIcon(QString::fromUtf8(":/data/img/arcademode.png")), tr("Titles"));
	comboBoxImageType->insertItem(QMC2_IMGCHK_INDEX_PCB, QIcon(QString::fromUtf8(":/data/img/circuit.png")), tr("PCBs"));
	comboBoxImageType->insertSeparator(QMC2_IMGCHK_INDEX_SEPARATOR);
	comboBoxImageType->insertItem(QMC2_IMGCHK_INDEX_ICON, QIcon(QString::fromUtf8(":/data/img/icon.png")), tr("Icons"));
}

void ImageChecker::updateSoftwareLists()
{
	if ( !qmc2SoftwareList ) {
		QLayout *vbl = qmc2MainWindow->tabSoftwareList->layout();
		if ( vbl )
			delete vbl;
		int left, top, right, bottom;
		qmc2MainWindow->gridLayout->getContentsMargins(&left, &top, &right, &bottom);
		QVBoxLayout *layout = new QVBoxLayout;
		layout->setContentsMargins(left, top, right, bottom);
		qmc2SoftwareList = new SoftwareList("qmc2_imagechecker_dummy", qmc2MainWindow->tabSoftwareList);
		layout->addWidget(qmc2SoftwareList);
		qmc2MainWindow->tabSoftwareList->setLayout(layout);
	}
	qmc2MainWindow->isCreatingSoftList = false;
	setEnabled(false);
	disconnect(qmc2SoftwareList);
	connect(qmc2SoftwareList, SIGNAL(loadFinished(bool)), this, SLOT(softwareListLoadFinished(bool)));
	comboBoxWidgetSoftwareLists->clearAll();
	comboBoxWidgetSoftwareLists->setLabel(tr("Updating software information, please wait..."));
	QTimer::singleShot(0, qmc2SoftwareList, SLOT(load()));
}

void ImageChecker::updateCornerWidget()
{
	comboBoxWidgetSoftwareLists->clearAll();
	QStringList softwareLists = swlDb->uniqueSoftwareLists();
	if ( softwareLists.isEmpty() ) {
		comboBoxWidgetSoftwareLists->setLabel(tr("No software information"));
		comboBoxWidgetSoftwareLists->setPushButton(tr("Update database"), tr("Click to update the software list XML database now"));
	} else {
		comboBoxWidgetSoftwareLists->setLabel(tr("Select software list"));
		comboBoxWidgetSoftwareLists->setComboBox(softwareLists, tr("Select the software list for which the image check should run"));
	}
}

void ImageChecker::softwareListLoadFinished(bool /* success */)
{
	QTimer::singleShot(0, this, SLOT(updateCornerWidget()));
	setEnabled(true);
}

void ImageChecker::adjustIconSizes()
{
	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFontMetrics fm(f);
	QSize iconSize = QSize(fm.height() - 2, fm.height() - 2);

	comboBoxImageType->setIconSize(iconSize);
	toolButtonSelectSets->setIconSize(iconSize);
	toolButtonStartStop->setIconSize(iconSize);
	toolButtonClear->setIconSize(iconSize);
	toolButtonSaveLog->setIconSize(iconSize);
	toolButtonRemoveObsolete->setIconSize(iconSize);
	toolButtonBad->setIconSize(iconSize);
	toolButtonRemoveBad->setIconSize(iconSize);
	listWidgetFound->setIconSize(iconSize);
	listWidgetMissing->setIconSize(iconSize);
	listWidgetObsolete->setIconSize(iconSize);

	QFont logFont;
	logFont.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/LogFont").toString());
	plainTextEditLog->setFont(logFont);
}

void ImageChecker::on_toolButtonBad_toggled(bool checked)
{
	int row = listWidgetMissing->currentItem() ? listWidgetMissing->currentRow() : 0;
	for (int i = listWidgetMissing->count() - 1; i >= 0; i--) {
		QListWidgetItem *item = listWidgetMissing->item(i);
		if ( checked )
			item->setHidden(item->icon().isNull());
		else
			item->setHidden(false);
	}
	listWidgetMissing->blockSignals(true);
	listWidgetMissing->setCurrentRow(row);
	listWidgetMissing->blockSignals(false);
	listWidgetMissing->scrollToItem(listWidgetMissing->item(row), QAbstractItemView::PositionAtCenter);
}

void ImageChecker::on_listWidgetFound_itemSelectionChanged()
{
	listWidgetFoundSelectionTimer.start(qmc2Config->value(QMC2_FRONTEND_PREFIX + "MachineList/UpdateDelay", 10).toInt());
}

void ImageChecker::listWidgetFound_itemSelectionChanged_delayed()
{
	if ( toolButtonSelectSets->isChecked() ) {
		QList<QListWidgetItem *> items = listWidgetFound->selectedItems();
		if ( items.count() > 0 )
			selectItem(items[0]->text());
	}
}

void ImageChecker::on_listWidgetMissing_itemSelectionChanged()
{
	listWidgetMissingSelectionTimer.start(qmc2Config->value(QMC2_FRONTEND_PREFIX + "MachineList/UpdateDelay", 10).toInt());
}

void ImageChecker::listWidgetMissing_itemSelectionChanged_delayed()
{
	if ( toolButtonSelectSets->isChecked() ) {
		QList<QListWidgetItem *> items = listWidgetMissing->selectedItems();
		if ( items.count() > 0 )
			selectItem(items[0]->text());
	}
}

void ImageChecker::startStop()
{
	if ( isRunning ) {
		foreach (ImageCheckerThread *thread, threadMap) {
			thread->exitThread = true;
			thread->waitCondition.wakeAll();
			thread->wait();
			qApp->processEvents();
			avgScanSpeed += thread->scanCount;
			foundCount += thread->foundCount;
			missingCount += thread->missingCount;
			delete thread;
		}
		threadMap.clear();
		if ( passNumber == 2 && !startStopClicked ) {
			if ( avgScanSpeed > 0.0 ) {
				int scannedItems = int(avgScanSpeed);
				avgScanSpeed *= 1000.0 / (double)checkTimer.elapsed();
				if ( currentImageType == QMC2_IMGCHK_INDEX_ICON ) {
					log(QString("%1, %2, %3 (%4)").arg(tr("%n icon(s) scanned", "", scannedItems)).arg(tr("%n valid icon file(s) found", "", foundCount)).arg(tr("%n icon file(s) missing", "", missingCount)).arg(tr("%n bad file(s)", "", badCount)));
					log(tr("Average scanning speed = %n icon(s) per second", "", int(avgScanSpeed)));
				} else {
					log(QString("%1, %2, %3 (%4)").arg(tr("%n image(s) scanned", "", scannedItems)).arg(tr("%n valid image file(s) found", "", foundCount)).arg(tr("%n image file(s) missing", "", missingCount)).arg(tr("%n bad file(s)", "", badCount)));
					log(tr("Average scanning speed = %n image(s) per second", "", int(avgScanSpeed)));
				}
				avgScanSpeed = 0.0;
				foundCount = missingCount = 0;
			}
			bufferedObsoleteList.clear();
			QTimer::singleShot(0, this, SLOT(checkObsoleteFiles()));
			updateTimer.start(QMC2_CHECK_UPDATE_FAST);
		} else if ( passNumber == -1 || startStopClicked ) {
			if ( avgScanSpeed > 0.0 ) {
				int scannedItems = int(avgScanSpeed);
				avgScanSpeed *= 1000.0 / (double)checkTimer.elapsed();
				log(QString("%1, %2, %3 (%4)").arg(tr("%n image(s) scanned", "", scannedItems)).arg(tr("%n valid image file(s) found", "", foundCount)).arg(tr("%n image file(s) missing", "", missingCount)).arg(tr("%n bad file(s)", "", badCount)));
				log(tr("Average scanning speed = %n image(s) per second", "", int(avgScanSpeed)));
			}
			isRunning = false;
			qmc2ImageCheckActive = false;
			toolButtonStartStop->setIcon(QIcon(QString::fromUtf8(":/data/img/refresh.png")));
			QTime elapsedTime(0, 0, 0, 0);
			elapsedTime = elapsedTime.addMSecs(checkTimer.elapsed());
			log(tr("%1 check ended -- elapsed time = %2").arg(currentImageType == QMC2_IMGCHK_INDEX_ICON ? tr("Icon") : tr("Image")).arg(elapsedTime.toString("mm:ss.zzz")));
			updateResults();
			progressBar->setRange(0, 100);
			progressBar->setValue(0);
			labelStatus->setText(tr("Idle"));
			progressBar->setRange(-1, -1);
			progressBar->setValue(-1);
			updateTimer.stop();
			enableWidgets(true);
			avgScanSpeed = 0.0;
			toolButtonBad->setEnabled(badCount > 0);
			toolButtonRemoveBad->setEnabled(badCount > 0);
			passNumber = foundCount = missingCount = badCount = 0;
		}
	} else {
		threadMap.clear();
		passNumber = 0;
		plainTextEditLog->clear();
		ImageWidget *imageWidget = 0;
		SoftwareImageWidget *softwareImageWidget = 0;
		currentImageType = comboBoxImageType->currentIndex();
		QString imageType;
		switch ( currentImageType ) {
			case QMC2_IMGCHK_INDEX_PREVIEW:
				imageWidget = qmc2Preview;
				imageType = tr("preview");
				break;
			case QMC2_IMGCHK_INDEX_FLYER:
				imageWidget = qmc2Flyer;
				imageType = tr("flyer");
				break;
			case QMC2_IMGCHK_INDEX_CABINET:
				imageWidget = qmc2Cabinet;
				imageType = tr("cabinet");
				break;
			case QMC2_IMGCHK_INDEX_MARQUEE:
				imageWidget = qmc2Marquee;
				imageType = tr("marquee");
				break;
			case QMC2_IMGCHK_INDEX_CONTROLLER:
				imageWidget = qmc2Controller;
				imageType = tr("controller");
				break;
			case QMC2_IMGCHK_INDEX_TITLE:
				imageWidget = qmc2Title;
				imageType = tr("title");
				break;
			case QMC2_IMGCHK_INDEX_PCB:
				imageWidget = qmc2PCB;
				imageType = tr("PCB");
				break;
			case QMC2_IMGCHK_INDEX_ICON:
			default:
				imageWidget = 0;
				break;
		}
		qmc2StopParser = false;
		enableWidgets(false);
		log(tr("%1 check started").arg(imageWidget ? tr("Image") : tr("Icon")));
		if ( imageWidget ) {
			for (int t = 0; t < spinBoxThreads->value(); t++) {
				ImageCheckerThread *thread = new ImageCheckerThread(t, imageWidget, softwareImageWidget, this);
				connect(thread, SIGNAL(log(const QString &)), this, SLOT(log(const QString &)));
				connect(thread, SIGNAL(resultsReady(const QStringList &, const QStringList &, const QStringList &, const QStringList &)), this, SLOT(resultsReady(const QStringList &, const QStringList &, const QStringList &, const QStringList &)));
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
		toolButtonBad->setText(tr("Bad:") + " 0");
		toolButtonBad->setChecked(false);
		toolButtonBad->setEnabled(false);
		toolButtonRemoveBad->setEnabled(false);
		toolButtonRemoveObsolete->setEnabled(false);
		listWidgetObsolete->clear();
		labelObsolete->setText(tr("Obsolete:") + " 0");
		progressBar->setRange(0, qmc2MachineListItemHash.count());
		progressBar->setValue(0);
		avgScanSpeed = 0.0;
		foundCount = missingCount = badCount = 0;
		passNumber = 1;
		labelStatus->setText(imageWidget || softwareImageWidget ? tr("Checking %1 images").arg(imageType) : tr("Checking icons"));
		bufferedFoundList.clear();
		bufferedMissingList.clear();
		bufferedBadList.clear();
		bufferedBadFileList.clear();
		QTimer::singleShot(0, this, SLOT(feedWorkerThreads()));
		updateTimer.start(QMC2_CHECK_UPDATE_FAST);
		checkTimer.start();
	}
}

void ImageChecker::on_toolButtonStartStop_clicked()
{
	startStopClicked = true;
	startStop();
	startStopClicked = false;
}

void ImageChecker::enableWidgets(bool enable)
{
	switch ( currentImageType ) {
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
		case QMC2_IMGCHK_INDEX_CONTROLLER:
			qmc2Options->stackedWidgetController->setEnabled(enable);
			qmc2Options->radioButtonControllerSelect->setEnabled(enable);
			break;
		case QMC2_IMGCHK_INDEX_TITLE:
			qmc2Options->stackedWidgetTitle->setEnabled(enable);
			qmc2Options->radioButtonTitleSelect->setEnabled(enable);
			break;
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
	spinBoxThreads->setEnabled(currentImageType == QMC2_IMGCHK_INDEX_ICON ? false : enable);
	if ( enable ) {
		toolButtonClear->setEnabled(listWidgetFound->count() > 0 || listWidgetMissing->count() > 0 || listWidgetObsolete->count() > 0 || plainTextEditLog->blockCount() > 0);
		toolButtonRemoveObsolete->setEnabled(listWidgetObsolete->count() > 0);
		toolButtonSaveLog->setEnabled(plainTextEditLog->blockCount() > 1);
	} else {
		toolButtonClear->setEnabled(false);
		toolButtonRemoveObsolete->setEnabled(false);
		toolButtonSaveLog->setEnabled(false);
	}
}

void ImageChecker::feedWorkerThreads()
{
	QHashIterator<QString, QTreeWidgetItem *> it(qmc2MachineListItemHash);
	int lastThreadID = -1;
#ifdef QMC2_DEBUG
	int count = 0;
#endif

	QString imageType;
	switch ( currentImageType ) {
		case QMC2_IMGCHK_INDEX_PREVIEW:
			imageType = tr("preview");
			break;
		case QMC2_IMGCHK_INDEX_FLYER:
			imageType = tr("flyer");
			break;
		case QMC2_IMGCHK_INDEX_CABINET:
			imageType = tr("cabinet");
			break;
		case QMC2_IMGCHK_INDEX_MARQUEE:
			imageType = tr("marquee");
			break;
		case QMC2_IMGCHK_INDEX_CONTROLLER:
			imageType = tr("controller");
			break;
		case QMC2_IMGCHK_INDEX_TITLE:
			imageType = tr("title");
			break;
		case QMC2_IMGCHK_INDEX_PCB:
			imageType = tr("PCB");
			break;
		case QMC2_IMGCHK_INDEX_ICON:
		default:
			break;
	}

	// synchronize with worker threads
	bool waitForThreads;
	do {
		waitForThreads = false;
		foreach (ImageCheckerThread *thread, threadMap) {
			if ( !thread->isWaiting ) {
				waitForThreads = true;
				break;
			}
		}
		if ( waitForThreads )
			QTest::qWait(25);
	} while ( waitForThreads );

	bool decompressionDone = false;
	while ( it.hasNext() && qmc2ImageCheckActive && !qmc2StopParser ) {
		if ( !threadMap.isEmpty() ) {
			// images
			bool enableStartStop = true;
			if ( !decompressionDone ) {
				foreach (ImageCheckerThread *thread, threadMap) {
					if ( thread->isFillingDictionary() ) {
						enableStartStop = false;
						break;
					}
				}
				if ( enableStartStop ) {
					toolButtonStartStop->setEnabled(true);
					qmc2MainWindow->actionExitStop->setEnabled(true);
					labelStatus->setText(tr("Checking %1 images").arg(imageType));
					progressBar->setRange(0, qmc2MachineListItemHash.count());
					decompressionDone = true;
				} else {
					toolButtonStartStop->setEnabled(false);
					qmc2MainWindow->actionExitStop->setEnabled(false);
					labelStatus->setText(tr("Decompressing archive"));
					progressBar->setRange(0, 0);
					progressBar->setValue(-1);
				}
			}

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
				if ( threadMap[selectedThread]->workUnitMutex.tryLock(QMC2_IMGCHK_WU_MUTEX_LOCK_TIMEOUT) ) {
					QStringList workUnit;
					while ( it.hasNext() && qmc2ImageCheckActive && workUnit.count() < QMC2_IMGCHK_WORKUNIT_SIZE && !qmc2StopParser ) {
						it.next();
						if ( qmc2MachineListItemHash[it.key()] )
							workUnit << it.key();
					}
					threadMap[selectedThread]->workUnitMutex.unlock();
					if ( qmc2ImageCheckActive ) {
						threadMap[selectedThread]->workUnit += workUnit;
						threadMap[selectedThread]->waitCondition.wakeAll();
						QTest::qWait(25);
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
			qmc2IconHash.clear();
			qmc2IconsPreloaded = false;
			int itemCount = 0;
			bool firstCheck = true;
			qmc2MainWindow->progressBarMachineList->setRange(0, qmc2MachineListItemHash.count());
			qmc2MainWindow->progressBarMachineList->setFormat(QString());
			while ( it.hasNext() && qmc2ImageCheckActive && !qmc2StopParser ) {
				it.next();
				QString gameName = it.key();
				if ( !qmc2MachineListItemHash[gameName] )
					continue;
				if ( qmc2MachineList->loadIcon(gameName, NULL, true, NULL) ) {
					log(tr("Thread[%1]: Icon for '%2' found").arg(0).arg(gameName));
					bufferedFoundList << gameName;
					foundCount++;
				} else {
					log(tr("Thread[%1]: Icon for '%2' is missing").arg(0).arg(gameName));
					bufferedMissingList << gameName;
					missingCount++;
				}
				if ( firstCheck ) {
					qmc2MainWindow->progressBarMachineList->reset();
					firstCheck = false;
				}
				if ( itemCount++ % 50 == 0 )
					qApp->processEvents();
				avgScanSpeed += 1.0;
			}
			qmc2MainWindow->progressBarMachineList->reset();
		}
		qApp->processEvents();
	}
}

void ImageChecker::on_toolButtonClear_clicked()
{
	listWidgetFound->clear();
	labelFound->setText(tr("Found:") + " 0");
	listWidgetMissing->clear();
	labelMissing->setText(tr("Missing:") + " 0");
	toolButtonBad->setText(tr("Bad:") + " 0");
	toolButtonBad->setChecked(false);
	toolButtonBad->setEnabled(false);
	toolButtonRemoveBad->setEnabled(false);
	toolButtonRemoveObsolete->setEnabled(false);
	listWidgetObsolete->clear();
	labelObsolete->setText(tr("Obsolete:") + " 0");
	plainTextEditLog->clear();
	progressBar->setRange(0, qmc2MachineListItemHash.count());
	progressBar->setValue(0);
	avgScanSpeed = 0.0;
	foundCount = missingCount = badCount = passNumber = 0;
	currentImageType = QMC2_IMGCHK_INDEX_NONE;
	toolButtonClear->setEnabled(false);
	toolButtonSaveLog->setEnabled(false);
	toolButtonRemoveObsolete->setEnabled(false);
}

void ImageChecker::on_toolButtonSaveLog_clicked()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Choose file to store the image checker log"), "qmc2-imagechecker.log", tr("All files (*)"), 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !fileName.isEmpty() ) {
		QFile f(fileName);
		if ( f.open(QIODevice::WriteOnly | QIODevice::Text) ) {
			QTextStream ts(&f);
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("saving image checker log to '%1'").arg(fileName));
			log(tr("saving image checker log to '%1'").arg(fileName));
			ts << plainTextEditLog->toPlainText();
			f.close();
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("done (saving image checker log to '%1')").arg(fileName));
			log(tr("done (saving image checker log to '%1')").arg(fileName));
		} else {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: can't open file '%1' for writing, please check permissions").arg(fileName));
			log(tr("WARNING: can't open file '%1' for writing, please check permissions").arg(fileName));
		}
	}
}

void ImageChecker::on_toolButtonRemoveBad_clicked()
{
	ImageWidget *imageWidget = 0;
	SoftwareImageWidget *softwareImageWidget = 0;
	switch ( currentImageType ) {
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
		case QMC2_IMGCHK_INDEX_CONTROLLER:
			imageWidget = qmc2Controller;
			break;
		case QMC2_IMGCHK_INDEX_TITLE:
			imageWidget = qmc2Title;
			break;
		case QMC2_IMGCHK_INDEX_PCB:
			imageWidget = qmc2PCB;
			break;
		case QMC2_IMGCHK_INDEX_ICON:
		default:
			imageWidget = 0;
			break;
	}

	if ( imageWidget ) {
		QStringList pathsToRemove;
		QList<int> badImageRows;
		for (int i = 0; i < listWidgetMissing->count(); i++) {
			QListWidgetItem *item = listWidgetMissing->item(i);
			if ( !item->icon().isNull() ) {
				badImageRows << i;
				pathsToRemove << item->whatsThis().split("\t", QString::SkipEmptyParts);
			}
		}

		int itemCount = 0;
		if ( imageWidget->useZip() ) {
			// zipped images
			foreach (QString filePath, imageWidget->imageZip().split(";", QString::SkipEmptyParts)) {
				labelStatus->setText(tr("Executing ZIP tool"));
				progressBar->setRange(0, 0);
				progressBar->setValue(-1);
#if defined(Q_OS_WIN)
				QString command = "cmd.exe";
				QStringList args;
				args << "/c" << qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/ZipTool").toString().replace('/', '\\')
				     << qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/ZipToolRemovalArguments").toString().split(" ", QString::SkipEmptyParts);
#else
				QString command = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/ZipTool").toString();
				QStringList args = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/ZipToolRemovalArguments").toString().split(" ", QString::SkipEmptyParts);
#endif
				QString fullCommandString = command;
				QStringList pathsToRemoveLocal;
				foreach (QString p, pathsToRemove) {
					if ( p.startsWith(filePath + ": ") ) {
						p.remove(0, filePath.length() + 2);
						pathsToRemoveLocal << p;
					}
				}
				for (int i = 0; i < args.count(); i++) {
					if ( args[i] == "$ARCHIVE$" ) {
#if defined(Q_OS_WIN)
						QString zipFile = filePath;
						args[i] = zipFile.replace('/', '\\');
#else
						args[i] = filePath;
#endif
					} else if ( args[i] == "$FILELIST$" ) {
						args.removeAt(i);
						int j = i;
						foreach (QString file, pathsToRemoveLocal)
							args.insert(j++, file);
					}
				}
				if ( !pathsToRemoveLocal.isEmpty() ) {
					foreach (QString s, args) {
						if ( s.contains(rxCharsToEscape) )
							s = "\"" + s + "\"";
						fullCommandString += " " + s;
					}
					log(tr("Running ZIP tool to remove bad image files, command = '%1'").arg(fullCommandString));
					unzClose(imageWidget->imageFileMap[filePath]);
					ToolExecutor zipRemovalTool(this, command, args);
					zipRemovalTool.exec();
					imageWidget->imageFileMap[filePath] = unzOpen(filePath.toUtf8().constData());
					if ( zipRemovalTool.toolExitStatus == QProcess::NormalExit && zipRemovalTool.toolExitCode == 0 ) {
						listWidgetMissing->setUpdatesEnabled(false);
						int filesRemoved = 0;
						for (int i = badImageRows.count() - 1; i >= 0; i--) {
							QListWidgetItem *itemToDelete = listWidgetMissing->takeItem(badImageRows[i]);
							if ( itemToDelete ) {
								filesRemoved++;
								delete itemToDelete;
							}
						}
						listWidgetMissing->setUpdatesEnabled(true);
						int filesRemaining = badImageRows.count() - filesRemoved;
						toolButtonBad->setText(tr("Bad:") + " " + QString::number(filesRemaining));
						if ( filesRemaining <= 0 && toolButtonBad->isChecked() ) on_toolButtonBad_toggled(false);
						toolButtonRemoveBad->setEnabled(false);
						toolButtonBad->setEnabled(false);
						toolButtonBad->setChecked(false);
					} else
						log(tr("WARNING: ZIP tool didn't exit cleanly: exitCode = %1, exitStatus = %2").arg(zipRemovalTool.toolExitCode).arg(zipRemovalTool.toolExitStatus == QProcess::NormalExit ? tr("normal") : tr("crashed")));
				}
				labelStatus->setText(tr("Idle"));
				progressBar->setRange(-1, -1);
				progressBar->setValue(-1);
			}
		} else if ( imageWidget->useSevenZip() ) {
			// 7-zipped images
			foreach (QString filePath, imageWidget->imageZip().split(";", QString::SkipEmptyParts)) {
				labelStatus->setText(tr("Executing 7z tool"));
				progressBar->setRange(0, 0);
				progressBar->setValue(-1);
#if defined(Q_OS_WIN)
				QString command = "cmd.exe";
				QStringList args;
				args << "/c" << qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/SevenZipTool").toString().replace('/', '\\')
				     << qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/SevenZipToolRemovalArguments").toString().split(" ", QString::SkipEmptyParts);
#else
				QString command = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/SevenZipTool").toString();
				QStringList args = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/SevenZipToolRemovalArguments").toString().split(" ", QString::SkipEmptyParts);
#endif
				QString fullCommandString = command;
				QStringList pathsToRemoveLocal;
				foreach (QString p, pathsToRemove) {
					if ( p.startsWith(filePath + ": ") ) {
						p.remove(0, filePath.length() + 2);
						pathsToRemoveLocal << p;
					}
				}
				for (int i = 0; i < args.count(); i++) {
					if ( args[i] == "$ARCHIVE$" ) {
#if defined(Q_OS_WIN)
						QString sevenZipFilePath = filePath;
						args[i] = sevenZipFilePath.replace('/', '\\');
#else
						args[i] = filePath;
#endif
					} else if ( args[i] == "$FILELIST$" ) {
						args.removeAt(i);
						int j = i;
						foreach (QString file, pathsToRemoveLocal)
							args.insert(j++, file);
					}
				}
				if ( !pathsToRemoveLocal.isEmpty() ) {
					foreach (QString s, args) {
						if ( s.contains(rxCharsToEscape) )
							s = "\"" + s + "\"";
						fullCommandString += " " + s;
					}
					log(tr("Running 7z tool to remove bad image files, command = '%1'").arg(fullCommandString));
					imageWidget->imageFileMap7z[filePath]->close();
					ToolExecutor sevenZipRemovalTool(this, command, args);
					sevenZipRemovalTool.exec();
					imageWidget->imageFileMap7z[filePath]->open();
					if ( sevenZipRemovalTool.toolExitStatus == QProcess::NormalExit && sevenZipRemovalTool.toolExitCode == 0 ) {
						listWidgetMissing->setUpdatesEnabled(false);
						int filesRemoved = 0;
						for (int i = badImageRows.count() - 1; i >= 0; i--) {
							QListWidgetItem *itemToDelete = listWidgetMissing->takeItem(badImageRows[i]);
							if ( itemToDelete ) {
								filesRemoved++;
								delete itemToDelete;
							}
						}
						listWidgetMissing->setUpdatesEnabled(true);
						int filesRemaining = badImageRows.count() - filesRemoved;
						toolButtonBad->setText(tr("Bad:") + " " + QString::number(filesRemaining));
						if ( filesRemaining <= 0 && toolButtonBad->isChecked() ) on_toolButtonBad_toggled(false);
						toolButtonRemoveBad->setEnabled(false);
						toolButtonBad->setEnabled(false);
						toolButtonBad->setChecked(false);
					} else
						log(tr("WARNING: 7z tool didn't exit cleanly: exitCode = %1, exitStatus = %2").arg(sevenZipRemovalTool.toolExitCode).arg(sevenZipRemovalTool.toolExitStatus == QProcess::NormalExit ? tr("normal") : tr("crashed")));
				}
				labelStatus->setText(tr("Idle"));
				progressBar->setRange(-1, -1);
				progressBar->setValue(-1);
			}
		} else {
			// unzipped images
			listWidgetMissing->setUpdatesEnabled(false);
			int filesRemoved = 0;
			for (int i = badImageRows.count() - 1; i >= 0; i--) {
				QString fileName = pathsToRemove[i];
				QFile f(fileName);
				if ( f.remove() ) {
					log(tr("Bad image file '%1' removed").arg(fileName));
					filesRemoved++;
					QListWidgetItem *itemToDelete = listWidgetMissing->takeItem(badImageRows[i]);
					if ( itemToDelete )
						delete itemToDelete;
				} else
					log(tr("Bad image file '%1' cannot be removed, please check permissions").arg(fileName));
				if ( itemCount++ % 25 )
					qApp->processEvents();
			}
			listWidgetMissing->setUpdatesEnabled(true);
			int filesRemaining = badImageRows.count() - filesRemoved;
			toolButtonBad->setText(tr("Bad:") + " " + QString::number(filesRemaining));
			if ( filesRemaining <= 0 && toolButtonBad->isChecked() ) on_toolButtonBad_toggled(false);
			toolButtonBad->setEnabled(filesRemaining > 0);
			toolButtonRemoveBad->setEnabled(filesRemaining > 0);
		}
	}
}

void ImageChecker::on_toolButtonRemoveObsolete_clicked()
{
	ImageWidget *imageWidget = 0;
	SoftwareImageWidget *softwareImageWidget = 0;
	switch ( currentImageType ) {
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
		case QMC2_IMGCHK_INDEX_CONTROLLER:
			imageWidget = qmc2Controller;
			break;
		case QMC2_IMGCHK_INDEX_TITLE:
			imageWidget = qmc2Title;
			break;
		case QMC2_IMGCHK_INDEX_PCB:
			imageWidget = qmc2PCB;
			break;
		case QMC2_IMGCHK_INDEX_ICON:
		default:
			imageWidget = 0;
			break;
	}

	toolButtonRemoveObsolete->setEnabled(false);
	int itemCount = 0;
	if ( imageWidget ) {
		// images
		if ( imageWidget->useZip() ) {
			// zipped images
			foreach (QString filePath, imageWidget->imageZip().split(";", QString::SkipEmptyParts)) {
				labelStatus->setText(tr("Executing ZIP tool"));
				progressBar->setRange(0, 0);
				progressBar->setValue(-1);
#if defined(Q_OS_WIN)
				QString command = "cmd.exe";
				QStringList args;
				args << "/c" << qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/ZipTool").toString().replace('/', '\\')
				     << qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/ZipToolRemovalArguments").toString().split(" ", QString::SkipEmptyParts);
#else
				QString command = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/ZipTool").toString();
				QStringList args = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/ZipToolRemovalArguments").toString().split(" ", QString::SkipEmptyParts);
#endif
				QString fullCommandString = command;
				int i, j;
				QStringList addArgs;
				for (i = 0; i < args.count(); i++) {
					if ( args[i] == "$ARCHIVE$" ) {
#if defined(Q_OS_WIN)
						QString zipFile = filePath;
						args[i] = zipFile.replace('/', '\\');
#else
						args[i] = filePath;
#endif
					} else if ( args[i] == "$FILELIST$" ) {
						for (j = 0; j < listWidgetObsolete->count(); j++) {
							QString itemText = listWidgetObsolete->item(j)->text();
							if ( itemText.startsWith(filePath + ": ") ) {
								itemText.remove(0, filePath.length() + 2);
								addArgs << itemText;
							}
						}
						args.removeAt(i);
						j = i;
						foreach (QString file, addArgs)
							args.insert(j++, file);
					}
				}
				if ( !addArgs.isEmpty() ) {
					foreach (QString s, args) {
						if ( s.contains(rxCharsToEscape) )
							s = "\"" + s + "\"";
						fullCommandString += " " + s;
					}
					log(tr("Running ZIP tool to remove obsolete files, command = '%1'").arg(fullCommandString));
					unzClose(imageWidget->imageFileMap[filePath]);
					ToolExecutor zipRemovalTool(this, command, args);
					zipRemovalTool.exec();
					imageWidget->imageFileMap[filePath] = unzOpen(filePath.toUtf8().constData());
					if ( zipRemovalTool.toolExitStatus != QProcess::NormalExit || zipRemovalTool.toolExitCode != 0 )
						log(tr("WARNING: ZIP tool didn't exit cleanly: exitCode = %1, exitStatus = %2").arg(zipRemovalTool.toolExitCode).arg(zipRemovalTool.toolExitStatus == QProcess::NormalExit ? tr("normal") : tr("crashed")));
				}
				labelStatus->setText(tr("Idle"));
				progressBar->setRange(-1, -1);
				progressBar->setValue(-1);
			}
			listWidgetObsolete->setUpdatesEnabled(false);
			listWidgetObsolete->clear();
			checkObsoleteFiles();
			listWidgetObsolete->setUpdatesEnabled(true);
		} else if ( imageWidget->useSevenZip() ) {
			// 7-zipped images
			foreach (QString filePath, imageWidget->imageZip().split(";", QString::SkipEmptyParts)) {
				labelStatus->setText(tr("Executing 7z tool"));
				progressBar->setRange(0, 0);
				progressBar->setValue(-1);
#if defined(Q_OS_WIN)
				QString command = "cmd.exe";
				QStringList args;
				args << "/c" << qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/SevenZipTool").toString().replace('/', '\\')
				     << qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/SevenZipToolRemovalArguments").toString().split(" ", QString::SkipEmptyParts);
#else
				QString command = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/SevenZipTool").toString();
				QStringList args = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/SevenZipToolRemovalArguments").toString().split(" ", QString::SkipEmptyParts);
#endif
				QString fullCommandString = command;
				int i, j;
				QStringList addArgs;
				for (i = 0; i < args.count(); i++) {
					if ( args[i] == "$ARCHIVE$" ) {
#if defined(Q_OS_WIN)
						QString sevenZipFilePath = filePath;
						args[i] = sevenZipFilePath.replace('/', '\\');
#else
						args[i] = filePath;
#endif
					} else if ( args[i] == "$FILELIST$" ) {
						for (j = 0; j < listWidgetObsolete->count(); j++) {
							QString itemText = listWidgetObsolete->item(j)->text();
							if ( itemText.startsWith(filePath + ": ") ) {
								itemText.remove(0, filePath.length() + 2);
								addArgs << itemText;
							}
						}
						args.removeAt(i);
						j = i;
						foreach (QString file, addArgs)
							args.insert(j++, file);
					}
				}
				if ( !addArgs.isEmpty() ) {
					foreach (QString s, args) {
						if ( s.contains(rxCharsToEscape) )
							s = "\"" + s + "\"";
						fullCommandString += " " + s;
					}
					log(tr("Running 7z tool to remove obsolete files, command = '%1'").arg(fullCommandString));
					imageWidget->imageFileMap7z[filePath]->close();
					ToolExecutor sevenZipRemovalTool(this, command, args);
					sevenZipRemovalTool.exec();
					imageWidget->imageFileMap7z[filePath]->open();
					if ( sevenZipRemovalTool.toolExitStatus != QProcess::NormalExit || sevenZipRemovalTool.toolExitCode != 0 )
						log(tr("WARNING: 7z tool didn't exit cleanly: exitCode = %1, exitStatus = %2").arg(sevenZipRemovalTool.toolExitCode).arg(sevenZipRemovalTool.toolExitStatus == QProcess::NormalExit ? tr("normal") : tr("crashed")));
				}
				labelStatus->setText(tr("Idle"));
				progressBar->setRange(-1, -1);
				progressBar->setValue(-1);
			}
			listWidgetObsolete->setUpdatesEnabled(false);
			listWidgetObsolete->clear();
			checkObsoleteFiles();
			listWidgetObsolete->setUpdatesEnabled(true);
		} else {
			// unzipped images
			labelStatus->setText(tr("Removing obsolete files / folders"));
			progressBar->setRange(0, listWidgetObsolete->count());
			listWidgetObsolete->setUpdatesEnabled(false);
			for (int i = 0; i < listWidgetObsolete->count(); i++) {
				QListWidgetItem *item = listWidgetObsolete->item(i);
				progressBar->setValue(++itemCount);
				QString path = item->text();
				QFileInfo fi(path);
				if ( fi.isDir() ) {
					QDir d(path);
					if ( d.rmdir(path) ) {
						log(tr("Obsolete image folder '%1' removed").arg(d.absolutePath()));
						QListWidgetItem *itemToDelete = listWidgetObsolete->takeItem(listWidgetObsolete->row(item));
						if ( itemToDelete )
							delete itemToDelete;
						i--;
					} else
						log(tr("Obsolete image folder '%1' cannot be removed, please check permissions").arg(d.absolutePath()));
				} else {
					QFile f(path);
					if ( f.remove() ) {
						log(tr("Obsolete image file '%1' removed").arg(fi.absoluteFilePath()));
						QListWidgetItem *itemToDelete = listWidgetObsolete->takeItem(listWidgetObsolete->row(item));
						if ( itemToDelete )
							delete itemToDelete;
						i--;
					} else
						log(tr("Obsolete image file '%1' cannot be removed, please check permissions").arg(fi.absoluteFilePath()));
				}
				if ( itemCount % QMC2_CHECK_UPDATE_MEDIUM == 0 ) {
					listWidgetObsolete->setUpdatesEnabled(true);
					qApp->processEvents();
					listWidgetObsolete->setUpdatesEnabled(false);
				}
			}
			listWidgetObsolete->setUpdatesEnabled(true);
		}
	} else {
		// icons
		if ( QMC2_ICON_FILETYPE_ZIP ) {
			// zipped icons
			foreach (QString filePath, qmc2IconFileMap.keys()) {
				labelStatus->setText(tr("Executing ZIP tool"));
				progressBar->setRange(0, 0);
				progressBar->setValue(-1);
#if defined(Q_OS_WIN)
				QString command = "cmd.exe";
				QStringList args;
				args << "/c" << qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/ZipTool").toString().replace('/', '\\')
				     << qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/ZipToolRemovalArguments").toString().split(" ", QString::SkipEmptyParts);
#else
				QString command = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/ZipTool").toString();
				QStringList args = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/ZipToolRemovalArguments").toString().split(" ", QString::SkipEmptyParts);
#endif
				QString fullCommandString = command;
				int i, j;
				QStringList addArgs;
				for (i = 0; i < args.count(); i++) {
					if ( args[i] == "$ARCHIVE$" ) {
#if defined(Q_OS_WIN)
						QString zipFile = filePath;
						args[i] = zipFile.replace('/', '\\');
#else
						args[i] = filePath;
#endif
					} else if ( args[i] == "$FILELIST$" ) {
						for (j = 0; j < listWidgetObsolete->count(); j++) {
							QString itemText = listWidgetObsolete->item(j)->text();
							if ( itemText.startsWith(filePath + ": ") ) {
								itemText.remove(0, filePath.length() + 2);
								addArgs << itemText;
							}
						}
						args.removeAt(i);
						j = i;
						foreach (QString file, addArgs)
							args.insert(j++, file);
					}
				}
				if ( !addArgs.isEmpty() ) {
					foreach (QString s, args) {
						if ( s.contains(rxCharsToEscape) )
							s = "\"" + s + "\"";
						fullCommandString += " " + s;
					}
					log(tr("Running ZIP tool to remove obsolete files, command = '%1'").arg(fullCommandString));
					unzClose(qmc2IconFileMap[filePath]);
					ToolExecutor zipRemovalTool(this, command, args);
					zipRemovalTool.exec();
					qmc2IconFileMap[filePath] = unzOpen(filePath.toUtf8().constData());
					if ( zipRemovalTool.toolExitStatus != QProcess::NormalExit || zipRemovalTool.toolExitCode != 0 )
						log(tr("WARNING: ZIP tool didn't exit cleanly: exitCode = %1, exitStatus = %2").arg(zipRemovalTool.toolExitCode).arg(zipRemovalTool.toolExitStatus == QProcess::NormalExit ? tr("normal") : tr("crashed")));
				}
				labelStatus->setText(tr("Idle"));
				progressBar->setRange(-1, -1);
				progressBar->setValue(-1);
			}
			listWidgetObsolete->setUpdatesEnabled(false);
			listWidgetObsolete->clear();
			checkObsoleteFiles();
			listWidgetObsolete->setUpdatesEnabled(true);
		} else if ( QMC2_ICON_FILETYPE_7Z ) {
			// 7-zipped icons
			foreach (QString filePath, qmc2IconFileMap7z.keys()) {
				labelStatus->setText(tr("Executing 7z tool"));
				progressBar->setRange(0, 0);
				progressBar->setValue(-1);
#if defined(Q_OS_WIN)
				QString command = "cmd.exe";
				QStringList args;
				args << "/c" << qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/SevenZipTool").toString().replace('/', '\\')
				     << qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/SevenZipToolRemovalArguments").toString().split(" ", QString::SkipEmptyParts);
#else
				QString command = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/SevenZipTool").toString();
				QStringList args = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/SevenZipToolRemovalArguments").toString().split(" ", QString::SkipEmptyParts);
#endif
				QString fullCommandString = command;
				int i, j;
				QStringList addArgs;
				for (i = 0; i < args.count(); i++) {
					if ( args[i] == "$ARCHIVE$" ) {
#if defined(Q_OS_WIN)
						QString sevenZipFilePath = filePath;
						args[i] = sevenZipFilePath.replace('/', '\\');
#else
						args[i] = filePath;
#endif
					} else if ( args[i] == "$FILELIST$" ) {
						for (j = 0; j < listWidgetObsolete->count(); j++) {
							QString itemText = listWidgetObsolete->item(j)->text();
							if ( itemText.startsWith(filePath + ": ") ) {
								itemText.remove(0, filePath.length() + 2);
								addArgs << itemText;
							}
						}
						args.removeAt(i);
						j = i;
						foreach (QString file, addArgs)
							args.insert(j++, file);
					}
				}
				if ( !addArgs.isEmpty() ) {
					foreach (QString s, args) {
						if ( s.contains(rxCharsToEscape) )
							s = "\"" + s + "\"";
						fullCommandString += " " + s;
					}
					log(tr("Running 7z tool to remove obsolete files, command = '%1'").arg(fullCommandString));
					qmc2IconFileMap7z[filePath]->close();
					ToolExecutor sevenZipRemovalTool(this, command, args);
					sevenZipRemovalTool.exec();
					qmc2IconFileMap7z[filePath]->open();
					if ( sevenZipRemovalTool.toolExitStatus != QProcess::NormalExit || sevenZipRemovalTool.toolExitCode != 0 )
						log(tr("WARNING: 7z tool didn't exit cleanly: exitCode = %1, exitStatus = %2").arg(sevenZipRemovalTool.toolExitCode).arg(sevenZipRemovalTool.toolExitStatus == QProcess::NormalExit ? tr("normal") : tr("crashed")));
				}
				labelStatus->setText(tr("Idle"));
				progressBar->setRange(-1, -1);
				progressBar->setValue(-1);
			}
			listWidgetObsolete->setUpdatesEnabled(false);
			listWidgetObsolete->clear();
			checkObsoleteFiles();
			listWidgetObsolete->setUpdatesEnabled(true);
		} else {
			// unzipped icons
			labelStatus->setText(tr("Removing obsolete files / folders"));
			progressBar->setRange(0, listWidgetObsolete->count());
			listWidgetObsolete->setUpdatesEnabled(false);
			for (int i = 0; i < listWidgetObsolete->count(); i++) {
				QListWidgetItem *item = listWidgetObsolete->item(i);
				progressBar->setValue(++itemCount);
				QString path = item->text();
				QFileInfo fi(path);
				if ( fi.isDir() ) {
					QDir d(path);
					if ( d.rmdir(path) ) {
						log(tr("Obsolete icon folder '%1' removed").arg(d.absolutePath()));
						QListWidgetItem *itemToDelete = listWidgetObsolete->takeItem(listWidgetObsolete->row(item));
						if ( itemToDelete )
							delete itemToDelete;
						i--;
					} else
						log(tr("Obsolete icon folder '%1' cannot be removed, please check permissions").arg(d.absolutePath()));
				} else {
					QFile f(path);
					if ( f.remove() ) {
						log(tr("Obsolete icon file '%1' removed").arg(path));
						QListWidgetItem *itemToDelete = listWidgetObsolete->takeItem(listWidgetObsolete->row(item));
						if ( itemToDelete )
							delete itemToDelete;
						i--;
					} else
						log(tr("Obsolete icon file '%1' cannot be removed, please check permissions").arg(path));
				}
				if ( itemCount % QMC2_CHECK_UPDATE_MEDIUM == 0 ) {
					listWidgetObsolete->setUpdatesEnabled(true);
					qApp->processEvents();
					listWidgetObsolete->setUpdatesEnabled(false);
				}
			}
			listWidgetObsolete->setUpdatesEnabled(true);
		}
	}
	labelObsolete->setText(tr("Obsolete:") + " " + QString::number(listWidgetObsolete->count()));
	toolButtonRemoveObsolete->setEnabled(listWidgetObsolete->count() > 0);
	labelStatus->setText(tr("Idle"));
	progressBar->setRange(-1, -1);
	progressBar->setValue(-1);
}

void ImageChecker::on_comboBoxImageType_currentIndexChanged(int index)
{
	spinBoxThreads->setEnabled(index != QMC2_IMGCHK_INDEX_ICON);
	toolButtonBad->setVisible(index != QMC2_IMGCHK_INDEX_ICON);
	toolButtonRemoveBad->setVisible(index != QMC2_IMGCHK_INDEX_ICON);
}

void ImageChecker::log(const QString &message)
{
	plainTextEditLog->appendPlainText(QTime::currentTime().toString("hh:mm:ss.zzz") + ": " + message);
}

void ImageChecker::resultsReady(const QStringList &foundList, const QStringList &missingList, const QStringList &badList, const QStringList &badFileList)
{
	bufferedFoundList += foundList;
	bufferedMissingList += missingList;
	bufferedBadList += badList;
	bufferedBadFileList += badFileList;
	progressBar->setValue(progressBar->value() + foundList.count() + missingList.count());
	progressBar->update();
}

void ImageChecker::checkObsoleteFiles()
{
	ImageWidget *imageWidget = 0;
	SoftwareImageWidget *softwareImageWidget = 0;
	log(tr("Checking for obsolete files"));
	switch ( currentImageType ) {
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
		case QMC2_IMGCHK_INDEX_CONTROLLER:
			imageWidget = qmc2Controller;
			break;
		case QMC2_IMGCHK_INDEX_TITLE:
			imageWidget = qmc2Title;
			break;
		case QMC2_IMGCHK_INDEX_PCB:
			imageWidget = qmc2PCB;
			break;
		case QMC2_IMGCHK_INDEX_ICON:
		default:
			imageWidget = 0;
			break;
	}

	labelStatus->setText(tr("Checking for obsolete files / folder"));
	progressBar->setRange(0, 0);
	progressBar->setValue(-1);

	QStringList fileList;
	QStringList dirList;

	if ( imageWidget ) {
		// images
		if ( imageWidget->useZip() ) {
			log(tr("Reading ZIP directory recursively"));
			foreach (unzFile imageFile, imageWidget->imageFileMap)
				recursiveZipList(imageFile, &fileList, imageWidget->imageFileMap.key(imageFile) + ": ");
		} else if ( imageWidget->useSevenZip() ) {
			log(tr("Reading 7z directory recursively"));
			foreach (SevenZipFile *imageFile, imageWidget->imageFileMap7z)
				recursiveSevenZipList(imageFile, &fileList, imageWidget->imageFileMap7z.key(imageFile) + ": ");
		}
#if defined(QMC2_LIBARCHIVE_ENABLED)
		else if ( imageWidget->useArchive() ) {
			log(tr("Reading archive directory recursively"));
			foreach (ArchiveFile *imageFile, imageWidget->imageArchiveMap)
				recursiveArchiveList(imageFile, &fileList, imageWidget->imageArchiveMap.key(imageFile) + ": ");
		}
#endif
		else {
			dirList = imageWidget->imageDir().split(";", QString::SkipEmptyParts);
			foreach (QString path, dirList) {
				log(tr("Reading image directory '%1' recursively").arg(QDir::toNativeSeparators(path)));
				recursiveFileList(path, &fileList);
			}
		}
	} else {
		// icons
		if ( QMC2_ICON_FILETYPE_ZIP ) {
			log(tr("Reading ZIP directory recursively"));
			foreach (unzFile iconFile, qmc2IconFileMap)
				recursiveZipList(iconFile, &fileList, qmc2IconFileMap.key(iconFile) + ": ");
		} else if ( QMC2_ICON_FILETYPE_7Z ) {
			log(tr("Reading 7z directory recursively"));
			foreach (SevenZipFile *imageFile, qmc2IconFileMap7z)
				recursiveSevenZipList(imageFile, &fileList, qmc2IconFileMap7z.key(imageFile) + ": ");
		}
#if defined(QMC2_LIBARCHIVE_ENABLED)
		else if ( QMC2_ICON_FILETYPE_ARCHIVE ) {
			log(tr("Reading archive directory recursively"));
			foreach (ArchiveFile *imageFile, qmc2IconArchiveMap)
				recursiveArchiveList(imageFile, &fileList, qmc2IconArchiveMap.key(imageFile) + ": ");
		}
#endif
		else {
			dirList = qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/IconDirectory").toString().split(";", QString::SkipEmptyParts);
			foreach (QString path, dirList) {
				log(tr("Reading icon directory '%1' recursively").arg(QDir::toNativeSeparators(path)));
				recursiveFileList(path, &fileList);
			}
		}
	}
	log(tr("%n directory entries to check", "", fileList.count()));
	progressBar->setRange(0, fileList.count());
	int itemCount = 0;
	int obsoleteCount = 0;
	QStringList imageFormats;
	foreach (QByteArray format, QImageReader::supportedImageFormats())
		imageFormats << QString(format).toLower();
	foreach (QString path, fileList) {
		progressBar->setValue(itemCount++);
		if ( qmc2StopParser || !isRunning )
			break;
		QFileInfo fi(path);
		bool isValidPath = false;
		if ( imageWidget ) {
			// images
			if ( imageWidget->useZip() || imageWidget->useSevenZip() || imageWidget->useArchive() ) {
				// archived images
				QString pathCopy = path;
				pathCopy.remove(rxColonSepStr);
				fi.setFile(pathCopy);
				foreach (int format, imageWidget->activeFormats) {
					foreach (QString extension, ImageWidget::formatExtensions[format].split(", ", QString::SkipEmptyParts)) {
#if defined(Q_OS_WIN)
						if ( pathCopy == fi.filePath() && fi.completeSuffix().toLower() == extension )
							if ( qmc2MachineListItemHash.contains(fi.baseName().toLower()) )
								isValidPath = true;
#else
						if ( pathCopy == fi.filePath() && fi.completeSuffix() == extension )
							if ( qmc2MachineListItemHash.contains(fi.baseName()) )
								isValidPath = true;
#endif
						if ( !isValidPath ) {
							QString subPath = fi.dir().dirName();
							QString imageFile = fi.baseName();
							if ( !subPath.isEmpty() && !imageFile.isEmpty() ) {
#if defined(Q_OS_WIN)
								if ( qmc2MachineListItemHash.contains(subPath.toLower()) ) {
									if ( imageFile.indexOf(rxFourDigits) == 0 )
										isValidPath = fi.completeSuffix().toLower() == extension;
								}
#else
								if ( qmc2MachineListItemHash.contains(subPath) ) {
									if ( imageFile.indexOf(rxFourDigits) == 0 )
										isValidPath = fi.completeSuffix() == extension;
								}
#endif
							} else if ( !subPath.isEmpty() ) {
#if defined(Q_OS_WIN)
								if ( qmc2MachineListItemHash.contains(subPath.toLower()) )
									isValidPath = true;
#else
								if ( qmc2MachineListItemHash.contains(subPath) )
									isValidPath = true;
#endif
							}
						}
					}
				}
			} else {
				// unzipped images
				foreach (QString dirPath, dirList) {
					dirPath = QDir::toNativeSeparators(dirPath);
					if ( isValidPath )
						break;
					QString pathCopy = path;
					pathCopy.remove(dirPath);
					fi.setFile(pathCopy);
					foreach (int format, imageWidget->activeFormats) {
						foreach (QString extension, ImageWidget::formatExtensions[format].split(", ", QString::SkipEmptyParts)) {
							if ( pathCopy.endsWith(QDir::separator()) ) {
								pathCopy.remove(pathCopy.length() - 1, 1);
#if defined(Q_OS_WIN)
								if ( qmc2MachineListItemHash.contains(pathCopy.toLower()) )
									isValidPath = true;
#else
								if ( qmc2MachineListItemHash.contains(pathCopy) )
									isValidPath = true;
#endif
							} else {
#if defined(Q_OS_WIN)
								if ( pathCopy == fi.fileName() && fi.completeSuffix().toLower() == extension )
									if ( qmc2MachineListItemHash.contains(fi.baseName().toLower()) )
										isValidPath = true;
#else
								if ( pathCopy == fi.fileName() && fi.completeSuffix() == extension )
									if ( qmc2MachineListItemHash.contains(fi.baseName()) )
										isValidPath = true;
#endif
							}

							if ( !isValidPath ) {
								QString subPath = fi.dir().dirName();
								QString imageFile = fi.baseName();
								if ( !subPath.isEmpty() && !imageFile.isEmpty() ) {
#if defined(Q_OS_WIN)
									if ( qmc2MachineListItemHash.contains(subPath.toLower()) ) {
										if ( imageFile.indexOf(rxFourDigits) == 0 )
											isValidPath = fi.completeSuffix().toLower() == extension;
									}
#else
									if ( qmc2MachineListItemHash.contains(subPath) ) {
										if ( imageFile.indexOf(rxFourDigits) == 0 )
											isValidPath = fi.completeSuffix() == extension;
									}
#endif
								}
							}
						}
					}
				}
			}
		} else {
			// icons
			if ( QMC2_ICON_FILETYPE_ZIP || QMC2_ICON_FILETYPE_7Z || QMC2_ICON_FILETYPE_ARCHIVE ) {
				// for archived icons, only the lower-case basenames and image-type suffixes count (.ico, .png, ...)
				QString pathCopy = path;
				pathCopy.remove(rxColonSepStr);
				fi.setFile(pathCopy);
				if ( imageFormats.contains(fi.completeSuffix().toLower()) ) {
					if ( qmc2MachineListItemHash.contains(fi.baseName().toLower()) )
						isValidPath = true;
				}
			} else {
				// for unzipped icons, only PNG images are supported
#if defined(Q_OS_WIN)
				if ( qmc2MachineListItemHash.contains(fi.baseName().toLower()) && fi.completeSuffix().toLower() == "png" )
					isValidPath = true;
#else
				if ( qmc2MachineListItemHash.contains(fi.baseName()) && fi.completeSuffix() == "png" )
					isValidPath = true;
#endif
			}
		}

		if ( !isValidPath ) {
			bool isZip = false;
			if ( imageWidget )
				if ( imageWidget->useZip() || imageWidget->useSevenZip() )
					isZip = true;
			QFileInfo fi(path);
			if ( fi.isDir() ) {
				log(tr("%1 folder '%2' is obsolete").arg(imageWidget ? tr("Image") : tr("Icon")).arg(path));
				if ( isZip )
					bufferedObsoleteList << path;
				else
					bufferedObsoleteList << fi.dir().absolutePath() + QDir::separator();
			} else {
				log(tr("%1 file '%2' is obsolete").arg(imageWidget ? tr("Image") : tr("Icon")).arg(path));
				if ( isZip )
					bufferedObsoleteList << path;
				else
					bufferedObsoleteList << fi.absoluteFilePath();
			}
			obsoleteCount++;
		}

		if ( itemCount % QMC2_CHECK_UPDATE_FAST == 0 )
			qApp->processEvents();
	}

	if ( passNumber > 0 && isRunning ) {
		passNumber = -1;
		QTimer::singleShot(0, this, SLOT(startStop()));
	}

	log(tr("%n obsolete file(s) found", "", obsoleteCount));
}

void ImageChecker::updateResults()
{
	listWidgetFound->insertItems(listWidgetFound->count(), bufferedFoundList);
	listWidgetMissing->insertItems(listWidgetMissing->count(), bufferedMissingList);
	listWidgetObsolete->insertItems(listWidgetObsolete->count(), bufferedObsoleteList);
	bufferedFoundList.clear();
	bufferedMissingList.clear();
	bufferedObsoleteList.clear();
	if ( !bufferedBadList.isEmpty() ) {
		QString searchRegExp = "(" + bufferedBadList.join("|") + ")";
		foreach (QListWidgetItem *item, listWidgetMissing->findItems(searchRegExp, Qt::MatchRegExp)) {
			item->setIcon(QIcon(QString::fromUtf8(":/data/img/warning.png")));
			QStringList blTemp;
			for (int i = 0; i < bufferedBadList.count(); i++)
				if ( bufferedBadList[i] == item->text() )
					 blTemp << bufferedBadFileList[i];
			item->setWhatsThis(blTemp.join("\t"));
		}
		badCount += bufferedBadList.count();
	}
	bufferedBadList.clear();
	bufferedBadFileList.clear();
	labelFound->setText(tr("Found:") + " " + QString::number(listWidgetFound->count()));
	labelMissing->setText(tr("Missing:") + " " + QString::number(listWidgetMissing->count()));
	if ( badCount > 0 )
		toolButtonBad->setText(tr("Bad:") + " " + QString::number(badCount));
	labelObsolete->setText(tr("Obsolete:") + " " + QString::number(listWidgetObsolete->count()));

	qApp->processEvents();

	if ( listWidgetFound->count() + listWidgetMissing->count() >= qmc2MainWindow->treeWidgetMachineList->topLevelItemCount() && isRunning && passNumber != 2 ) {
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
	switch ( qmc2MainWindow->stackedWidgetView->currentIndex() ) {
		case QMC2_VIEWMACHINELIST_INDEX: {
			QTreeWidgetItem *gameItem = qmc2MachineListItemHash[setName];
			if ( gameItem ) {
				qmc2MainWindow->treeWidgetMachineList->clearSelection();
				qmc2MainWindow->treeWidgetMachineList->setCurrentItem(gameItem);
				qmc2MainWindow->treeWidgetMachineList->scrollToItem(gameItem, qmc2CursorPositioningMode);
				gameItem->setSelected(true);
			}
			break;
		}
		case QMC2_VIEWHIERARCHY_INDEX: {
			QTreeWidgetItem *hierarchyItem = qmc2HierarchyItemHash[setName];
			if ( hierarchyItem ) {
				qmc2MainWindow->treeWidgetHierarchy->clearSelection();
				qmc2MainWindow->treeWidgetHierarchy->setCurrentItem(hierarchyItem);
				qmc2MainWindow->treeWidgetHierarchy->scrollToItem(hierarchyItem, qmc2CursorPositioningMode);
				hierarchyItem->setSelected(true);
			}
			break;
		}
		case QMC2_VIEWCATEGORY_INDEX: {
			QTreeWidgetItem *categoryItem = qmc2CategoryItemHash[setName];
			if ( categoryItem ) {
				qmc2MainWindow->treeWidgetCategoryView->clearSelection();
				qmc2MainWindow->treeWidgetCategoryView->setCurrentItem(categoryItem);
				qmc2MainWindow->treeWidgetCategoryView->scrollToItem(categoryItem, qmc2CursorPositioningMode);
				categoryItem->setSelected(true);
			}
			break;
		}
		case QMC2_VIEWVERSION_INDEX: {
			QTreeWidgetItem *versionItem = qmc2VersionItemHash[setName];
			if ( versionItem ) {
				qmc2MainWindow->treeWidgetVersionView->clearSelection();
				qmc2MainWindow->treeWidgetVersionView->setCurrentItem(versionItem);
				qmc2MainWindow->treeWidgetVersionView->scrollToItem(versionItem, qmc2CursorPositioningMode);
				versionItem->setSelected(true);
			}
			break;
		}
	}
}

void ImageChecker::closeEvent(QCloseEvent *e)
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/ImageChecker/Geometry", saveGeometry());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ImageChecker/ImageType", comboBoxImageType->currentIndex());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ImageChecker/Threads", spinBoxThreads->value());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "ImageChecker/SelectSets", toolButtonSelectSets->isChecked());

	if ( e )
		QDialog::closeEvent(e);
}

void ImageChecker::hideEvent(QHideEvent *e)
{
	closeEvent(NULL);
	QDialog::hideEvent(e);
}

void ImageChecker::showEvent(QShowEvent *e)
{
	adjustIconSizes();
	restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/ImageChecker/Geometry", QByteArray()).toByteArray());
	comboBoxImageType->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ImageChecker/ImageType", QMC2_IMGCHK_INDEX_PREVIEW).toInt());
	spinBoxThreads->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ImageChecker/Threads", 1).toInt());
	toolButtonSelectSets->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "ImageChecker/SelectSets", true).toBool());
	QTimer::singleShot(0, this, SLOT(updateCornerWidget()));
	QDialog::showEvent(e);
}

void ImageChecker::recursiveFileList(const QString &sDir, QStringList *fileNames)
{
	QDir dir(sDir);
	foreach (QFileInfo info, dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::Hidden | QDir::System)) {
		QString path = QDir::toNativeSeparators(info.filePath());
		if ( fileNames->count() % 25 == 0 )
			qApp->processEvents();
		if ( info.isDir() ) {
			// directory recursion
			if ( info.fileName() != ".." && info.fileName() != "." ) {
				recursiveFileList(path, fileNames);
				fileNames->append(path + QDir::separator());
			}
		} else
			fileNames->append(path);
	}
}

void ImageChecker::recursiveZipList(unzFile zip, QStringList *fileNames, QString prependString)
{
	if ( zip ) {
		unz_global_info unzGlobalInfo;
		int i = 0;
		if ( unzGetGlobalInfo(zip, &unzGlobalInfo) == UNZ_OK ) {
			if ( unzGoToFirstFile(zip) == UNZ_OK ) {
				do {
					char unzFileName[QMC2_MAX_PATH_LENGTH];
					if ( i % 25 == 0 )
						qApp->processEvents();
					unz_file_info unzFileInfo;
					if ( unzGetCurrentFileInfo(zip, &unzFileInfo, unzFileName, QMC2_MAX_PATH_LENGTH, NULL, 0, NULL, 0) == UNZ_OK )
						if ( unzFileName != NULL )
							fileNames->append(prependString + unzFileName);
					i++;
				} while ( unzGoToNextFile(zip) != UNZ_END_OF_LIST_OF_FILE );
			}
		}
	}
}

void ImageChecker::recursiveSevenZipList(SevenZipFile *sevenZipFile, QStringList *fileNames, QString prependString)
{
	if ( sevenZipFile ) {
		for (int index = 0; index < sevenZipFile->itemList().count(); index++) {
			fileNames->append(prependString + sevenZipFile->itemList()[index].name());
			if ( index % 25 == 0 )
				qApp->processEvents();
		}
	}
}

#if defined(QMC2_LIBARCHIVE_ENABLED)
void ImageChecker::recursiveArchiveList(ArchiveFile *archiveFile, QStringList *fileNames, QString prependString)
{
	if ( archiveFile ) {
		for (int index = 0; index < archiveFile->entryList().count(); index++) {
			fileNames->append(prependString + archiveFile->entryList()[index].name());
			if ( index % 25 == 0 )
				qApp->processEvents();
		}
	}
}
#endif
