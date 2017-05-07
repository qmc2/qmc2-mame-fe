#include <QApplication>
#include <QTest>
#include <QFont>
#include <QFontInfo>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QMap>
#include <QMultiMap>
#include <QDateTime>
#include <QMessageBox>
#include <QScrollBar>
#include <QChar>

#include "collectionrebuilder.h"
#include "settings.h"
#include "options.h"
#include "unzip.h"
#include "zip.h"
#include "sevenzipfile.h"
#if defined(QMC2_LIBARCHIVE_ENABLED)
#include "archivefile.h"
#endif
#include "machinelist.h"
#include "macros.h"
#include "softwarelist.h"

#if defined(QMC2_OS_WIN)
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>

extern Settings *qmc2Config;
extern Options *qmc2Options;
extern MachineList *qmc2MachineList;
extern SoftwareList *qmc2SoftwareList;

QHash<QString, QString> CollectionRebuilderThread::m_replacementHash;
QStringList CollectionRebuilderThread::m_fileTypes;

CollectionRebuilder::CollectionRebuilder(ROMAlyzer *myROMAlyzer, QWidget *parent) :
	QWidget(parent),
	m_romAlyzer(myROMAlyzer)
{
	setupUi(this);

	switch ( romAlyzer()->mode() ) {
		case QMC2_ROMALYZER_MODE_SOFTWARE:
			m_settingsKey = "SoftwareCollectionRebuilder";
			m_defaultSetEntity = "software";
			m_defaultRomEntity = "rom";
			m_defaultDiskEntity = "disk";
			checkBoxFilterExpressionSoftwareLists->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/UseFilterExpressionSoftwareLists", false).toBool());
			comboBoxFilterSyntaxSoftwareLists->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterSyntaxSoftwareLists", 0).toInt());
			comboBoxFilterTypeSoftwareLists->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterTypeSoftwareLists", 0).toInt());
			toolButtonExactMatchSoftwareLists->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/ExactMatchSoftwareLists", false).toBool());
			lineEditFilterExpressionSoftwareLists->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterExpressionSoftwareLists", QString()).toString());
			checkBoxFilterExpression->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/UseFilterExpression", false).toBool());
			comboBoxFilterSyntax->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterSyntax", 0).toInt());
			comboBoxFilterType->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterType", 0).toInt());
			toolButtonExactMatch->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/ExactMatch", false).toBool());
			lineEditFilterExpression->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterExpression", QString()).toString());
			m_correctIconPixmap = QPixmap(QString::fromUtf8(":/data/img/software_correct.png"));
			m_mostlyCorrectIconPixmap = QPixmap(QString::fromUtf8(":/data/img/software_mostlycorrect.png"));
			m_incorrectIconPixmap = QPixmap(QString::fromUtf8(":/data/img/software_incorrect.png"));
			m_notFoundIconPixmap = QPixmap(QString::fromUtf8(":/data/img/software_notfound.png"));
			m_unknownIconPixmap = QPixmap(QString::fromUtf8(":/data/img/software_unknown.png"));
			hideStateFilter(); // FIXME: add software-state filtering
			break;
		case QMC2_ROMALYZER_MODE_SYSTEM:
		default:
			m_settingsKey = "CollectionRebuilder";
			m_defaultSetEntity = "machine";
			m_defaultRomEntity = "rom";
			m_defaultDiskEntity = "disk";
			checkBoxFilterExpressionSoftwareLists->setVisible(false);
			comboBoxFilterSyntaxSoftwareLists->setVisible(false);
			comboBoxFilterTypeSoftwareLists->setVisible(false);
			lineEditFilterExpressionSoftwareLists->setVisible(false);
			toolButtonClearFilterExpressionSoftwareLists->setVisible(false);
			toolButtonExactMatchSoftwareLists->setVisible(false);
			checkBoxFilterExpression->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/UseFilterExpression", false).toBool());
			comboBoxFilterSyntax->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterSyntax", 0).toInt());
			comboBoxFilterType->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterType", 0).toInt());
			toolButtonExactMatch->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/ExactMatch", false).toBool());
			lineEditFilterExpression->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterExpression", QString()).toString());
			m_correctIconPixmap = QPixmap(QString::fromUtf8(":/data/img/sphere_green.png"));
			m_mostlyCorrectIconPixmap = QPixmap(QString::fromUtf8(":/data/img/sphere_yellowgreen.png"));
			m_incorrectIconPixmap = QPixmap(QString::fromUtf8(":/data/img/sphere_red.png"));
			m_notFoundIconPixmap = QPixmap(QString::fromUtf8(":/data/img/sphere_grey.png"));
			m_unknownIconPixmap = QPixmap(QString::fromUtf8(":/data/img/sphere_blue.png"));
			break;
	}
	checkBoxFilterStates->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/UseStateFilter", false).toBool());
	toolButtonStateC->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/IncludeStateC", true).toBool());
	toolButtonStateM->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/IncludeStateM", true).toBool());
	toolButtonStateI->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/IncludeStateI", true).toBool());
	toolButtonStateN->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/IncludeStateN", true).toBool());
	toolButtonStateU->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/IncludeStateU", true).toBool());

	pushButtonPauseResume->setVisible(false);
	setIgnoreCheckpoint(false);

	QFont logFont;
	logFont.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/LogFont").toString());
	plainTextEditLog->setFont(logFont);
	spinBoxMaxLogSize->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/MaxLogSize", 10000).toInt());
	adjustIconSizes();

	m_rebuilderThread = new CollectionRebuilderThread(this);
	connect(rebuilderThread(), SIGNAL(log(const QString &)), this, SLOT(log(const QString &)));
	connect(rebuilderThread(), SIGNAL(rebuildStarted()), this, SLOT(rebuilderThread_rebuildStarted()));
	connect(rebuilderThread(), SIGNAL(rebuildFinished()), this, SLOT(rebuilderThread_rebuildFinished()));
	connect(rebuilderThread(), SIGNAL(rebuildPaused()), this, SLOT(rebuilderThread_rebuildPaused()));
	connect(rebuilderThread(), SIGNAL(rebuildResumed()), this, SLOT(rebuilderThread_rebuildResumed()));
	connect(rebuilderThread(), SIGNAL(progressTextChanged(const QString &)), this, SLOT(rebuilderThread_progressTextChanged(const QString &)));
	connect(rebuilderThread(), SIGNAL(progressRangeChanged(int, int)), this, SLOT(rebuilderThread_progressRangeChanged(int, int)));
	connect(rebuilderThread(), SIGNAL(progressChanged(int)), this, SLOT(rebuilderThread_progressChanged(int)));
	connect(rebuilderThread(), SIGNAL(statusUpdated(quint64, quint64, quint64)), this, SLOT(rebuilderThread_statusUpdated(quint64, quint64, quint64)));
	connect(rebuilderThread(), SIGNAL(newMissing(QString, QString, QString, QString, QString, QString, QString)), this, SLOT(rebuilderThread_newMissing(QString, QString, QString, QString, QString, QString, QString)));

	m_missingDumpsViewer = 0;
	setDefaultEmulator(true);

	m_iconCheckpoint = QIcon(QString::fromUtf8(":/data/img/checkpoint.png"));
	m_iconNoCheckpoint = QIcon(QString::fromUtf8(":/data/img/no_checkpoint.png"));

	m_animationSequence = 0;
	connect(&m_animationTimer, SIGNAL(timeout()), this, SLOT(animationTimer_timeout()));

	frameEntities->setVisible(false);
	toolButtonRemoveXmlSource->setVisible(false);
	comboBoxXmlSource->blockSignals(true);
	comboBoxXmlSource->clear();
	comboBoxXmlSource->insertItem(0, tr("Current default emulator"));
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/Checkpoint", -1).toLongLong() >= 0 ) {
		comboBoxXmlSource->setItemIcon(0, m_iconCheckpoint);
		comboBoxXmlSource->setItemData(0, true);
	} else {
		comboBoxXmlSource->setItemIcon(0, m_iconNoCheckpoint);
		comboBoxXmlSource->setItemData(0, false);
	}
	QStringList xmlSources = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSources", QStringList()).toStringList();
	QStringList setEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetEntities", QStringList()).toStringList();
	QStringList romEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/RomEntities", QStringList()).toStringList();
	QStringList diskEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/DiskEntities", QStringList()).toStringList();
	QStringList checkpointList = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceCheckpoints", QStringList()).toStringList();
	QStringList softwareCheckpointList = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceListCheckpoints", QStringList()).toStringList();
	bool softwareCheckpointListOk = romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE ? (softwareCheckpointList.count() == xmlSources.count()) : true;
	QList<qint64> checkpoints;
	foreach (QString cp, checkpointList)
		checkpoints << cp.toLongLong();
	int index = 1;
	if ( xmlSources.count() > 0 && setEntities.count() == xmlSources.count() && romEntities.count() == xmlSources.count() && diskEntities.count() == xmlSources.count() && checkpointList.count() == xmlSources.count() && softwareCheckpointListOk ) {
		for (int i = 0; i < xmlSources.count(); i++) {
			QString xmlSource(xmlSources.at(i));
			QFileInfo fi(xmlSource);
			if ( fi.exists() && fi.isReadable() ) {
				comboBoxXmlSource->insertItem(index, xmlSource);
				if ( checkpoints[i] >= 0 ) {
					comboBoxXmlSource->setItemIcon(index, m_iconCheckpoint);
					comboBoxXmlSource->setItemData(index, true);
				} else {
					comboBoxXmlSource->setItemIcon(index, m_iconNoCheckpoint);
					comboBoxXmlSource->setItemData(index, false);
				}
				index++;
			} else {
				xmlSources.removeAt(i);
				setEntities.removeAt(i);
				romEntities.removeAt(i);
				diskEntities.removeAt(i);
				checkpointList.removeAt(i);
				checkpoints.removeAt(i);
				if ( romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE )
					softwareCheckpointList.removeAt(i);
				i--;
			}
		}
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSources", xmlSources);
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetEntities", setEntities);
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/RomEntities", romEntities);
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/DiskEntities", diskEntities);
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceCheckpoints", checkpointList);
		if ( romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE )
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceListCheckpoints", softwareCheckpointList);
	} else {
		qmc2Config->remove(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSources");
		qmc2Config->remove(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetEntities");
		qmc2Config->remove(QMC2_FRONTEND_PREFIX + m_settingsKey + "/RomEntities");
		qmc2Config->remove(QMC2_FRONTEND_PREFIX + m_settingsKey + "/DiskEntities");
		qmc2Config->remove(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceCheckpoints");
		qmc2Config->remove(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceListCheckpoints");
	}
	comboBoxXmlSource->insertSeparator(index);
	index++;
	comboBoxXmlSource->insertItem(index, tr("Select XML file..."));
	comboBoxXmlSource->setItemIcon(index, QIcon(QString::fromUtf8(":/data/img/fileopen.png")));
	comboBoxXmlSource->setCurrentIndex(0);
	comboBoxXmlSource->blockSignals(false);
	lineEditSetEntity->setText(m_defaultSetEntity);
	lineEditRomEntity->setText(m_defaultRomEntity);
	lineEditDiskEntity->setText(m_defaultDiskEntity);
	rebuilderThread_statusUpdated(0, 0, 0);
	comboBoxXmlSource->setFocus();
}

CollectionRebuilder::~CollectionRebuilder()
{
	hideEvent(0);
	if ( missingDumpsViewer() )
		delete missingDumpsViewer();
	if ( rebuilderThread() )
		delete rebuilderThread();
}

void CollectionRebuilder::on_spinBoxMaxLogSize_valueChanged(int value)
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/MaxLogSize", value);
	plainTextEditLog->setMaximumBlockCount(value);
}

void CollectionRebuilder::log(const QString &message)
{
	if ( checkBoxEnableLog->isChecked() )
		plainTextEditLog->appendPlainText(QDateTime::currentDateTime().toString("hh:mm:ss.zzz") + ": " + message);
}

void CollectionRebuilder::scrollToEnd()
{
	plainTextEditLog->horizontalScrollBar()->setValue(plainTextEditLog->horizontalScrollBar()->minimum());
	plainTextEditLog->verticalScrollBar()->setValue(plainTextEditLog->verticalScrollBar()->maximum());
}

void CollectionRebuilder::adjustIconSizes()
{
	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFontMetrics fm(f);
	QSize iconSize = QSize(fm.height() - 2, fm.height() - 2);

	pushButtonStartStop->setIconSize(iconSize);
	pushButtonPauseResume->setIconSize(iconSize);
	comboBoxXmlSource->setIconSize(iconSize);
	toolButtonRemoveXmlSource->setIconSize(iconSize);
	toolButtonViewMissingList->setIconSize(iconSize);
	toolButtonExactMatch->setIconSize(iconSize);
	toolButtonClearFilterExpression->setIconSize(iconSize);
	toolButtonExactMatchSoftwareLists->setIconSize(iconSize);
	toolButtonClearFilterExpressionSoftwareLists->setIconSize(iconSize);

	QSize iconSizeMiddle = QSize(fm.height(), fm.height());
	toolButtonStateC->setIcon(QIcon(m_correctIconPixmap.scaled(iconSizeMiddle, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
	toolButtonStateM->setIcon(QIcon(m_mostlyCorrectIconPixmap.scaled(iconSizeMiddle, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
	toolButtonStateI->setIcon(QIcon(m_incorrectIconPixmap.scaled(iconSizeMiddle, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
	toolButtonStateN->setIcon(QIcon(m_notFoundIconPixmap.scaled(iconSizeMiddle, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
	toolButtonStateU->setIcon(QIcon(m_unknownIconPixmap.scaled(iconSizeMiddle, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
}

void CollectionRebuilder::on_pushButtonStartStop_clicked()
{
	pushButtonStartStop->setEnabled(false);
	pushButtonStartStop->update();
	pushButtonPauseResume->setEnabled(false);
	pushButtonPauseResume->update();
	qApp->processEvents();
	if ( rebuilderThread()->isActive )
		rebuilderThread()->stopRebuilding = true;
	else if ( rebuilderThread()->isWaiting ) {
		newMissingList().clear();
		if ( comboBoxXmlSource->itemData(comboBoxXmlSource->currentIndex()).toBool() ) {
			if ( !ignoreCheckpoint() ) {
				switch ( QMessageBox::question(this, tr("Confirm checkpoint restart"), tr("Restart from stored checkpoint?"), QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::No) ) {
					case QMessageBox::Yes: {
							qint64 cp = 0;
							int index = comboBoxXmlSource->currentIndex();
							if ( index == 0 )
								cp = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/Checkpoint", -1).toLongLong();
							else {
								index -= 1;
								QStringList checkpointList = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceCheckpoints", QStringList()).toStringList();
								QStringList softwareCheckpointList = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceListCheckpoints", QStringList()).toStringList();
								if ( index >= 0 && index < checkpointList.count() ) {
									cp = checkpointList[index].toLongLong();
									if ( romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE )
										rebuilderThread()->setListCheckpoint(softwareCheckpointList[index], index);
								} else {
									cp = -1;
									if ( romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE )
										rebuilderThread()->setListCheckpoint(QString(), index);
								}
							}
							setDefaultEmulator(comboBoxXmlSource->currentIndex() == 0);
							setIgnoreCheckpoint(false);
							rebuilderThread()->checkpointRestart(cp);
						}
						break;
					case QMessageBox::No:
						rebuilderThread()->setCheckpoint(-1, comboBoxXmlSource->currentIndex());
						if ( romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE )
							rebuilderThread()->setListCheckpoint(QString(), comboBoxXmlSource->currentIndex());
						setDefaultEmulator(comboBoxXmlSource->currentIndex() == 0);
						setIgnoreCheckpoint(false);
						break;
					case QMessageBox::Cancel:
					default:
						pushButtonStartStop->setEnabled(true);
						pushButtonPauseResume->setEnabled(true);
						return;
				}
			} else {
				rebuilderThread()->setCheckpoint(-1, comboBoxXmlSource->currentIndex());
				if ( romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE )
					rebuilderThread()->setListCheckpoint(QString(), comboBoxXmlSource->currentIndex());
				setDefaultEmulator(comboBoxXmlSource->currentIndex() == 0);
				setIgnoreCheckpoint(false);
			}
		} else {
			rebuilderThread()->setCheckpoint(-1, comboBoxXmlSource->currentIndex());
			if ( romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE )
				rebuilderThread()->setListCheckpoint(QString(), comboBoxXmlSource->currentIndex());
			setDefaultEmulator(comboBoxXmlSource->currentIndex() == 0);
			setIgnoreCheckpoint(false);
		}
		if ( missingDumpsViewer() ) {
			missingDumpsViewer()->setDefaultEmulator(defaultEmulator());
			// FIXME "non-default emulator"
			//missingDumpsViewer()->frameExport->setVisible(romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE ? false : true);
			missingDumpsViewer()->frameExport->setVisible(romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE ? false : defaultEmulator());
		}
		if ( romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE ) {
			if ( checkBoxFilterExpressionSoftwareLists->isChecked() && !lineEditFilterExpressionSoftwareLists->text().isEmpty() )
				rebuilderThread()->setFilterExpressionSoftware(lineEditFilterExpressionSoftwareLists->text(), comboBoxFilterSyntaxSoftwareLists->currentIndex(), comboBoxFilterTypeSoftwareLists->currentIndex(), toolButtonExactMatchSoftwareLists->isChecked());
			else
				rebuilderThread()->clearFilterExpressionSoftware();
		}
		if ( checkBoxFilterExpression->isChecked() && !lineEditFilterExpression->text().isEmpty() )
			rebuilderThread()->setFilterExpression(lineEditFilterExpression->text(), comboBoxFilterSyntax->currentIndex(), comboBoxFilterType->currentIndex(), toolButtonExactMatch->isChecked());
		else
			rebuilderThread()->clearFilterExpression();
		if ( checkBoxFilterStates->isChecked() && defaultEmulator() )
			rebuilderThread()->setStateFilter(true, toolButtonStateC->isChecked(), toolButtonStateM->isChecked(), toolButtonStateI->isChecked(), toolButtonStateN->isChecked(), toolButtonStateU->isChecked());
		else
			rebuilderThread()->clearStateFilter();
		rebuilderThread()->useHashCache = romAlyzer()->checkBoxCollectionRebuilderHashCache->isChecked();
		rebuilderThread()->dryRun = romAlyzer()->checkBoxCollectionRebuilderDryRun->isChecked();
		rebuilderThread()->waitCondition.wakeAll();
	}
}

void CollectionRebuilder::on_pushButtonPauseResume_clicked()
{
	pushButtonPauseResume->setEnabled(false);
	if ( rebuilderThread()->isPaused )
		QTimer::singleShot(0, rebuilderThread(), SLOT(resume()));
	else
		QTimer::singleShot(0, rebuilderThread(), SLOT(pause()));
}

void CollectionRebuilder::setStateFilterVisibility(bool visible)
{
	checkBoxFilterStates->setVisible(visible);
	toolButtonStateC->setVisible(visible);
	toolButtonStateM->setVisible(visible);
	toolButtonStateI->setVisible(visible);
	toolButtonStateN->setVisible(visible);
	toolButtonStateU->setVisible(visible);
}

void CollectionRebuilder::on_comboBoxXmlSource_currentIndexChanged(int index)
{
	static int lastIndex = -1;

	if ( index == 0 ) {
		if ( lastIndex >= 0 ) {
			QStringList setEntities(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetEntities", QStringList()).toStringList());
			QStringList romEntities(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/RomEntities", QStringList()).toStringList());
			QStringList diskEntities(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/DiskEntities", QStringList()).toStringList());
			if ( lastIndex < setEntities.count() ) {
				setEntities.replace(lastIndex, lineEditSetEntity->text());
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetEntities", setEntities);
			}
			if ( lastIndex < romEntities.count() ) {
				romEntities.replace(lastIndex, lineEditRomEntity->text());
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/RomEntities", romEntities);
			}
			if ( lastIndex < diskEntities.count() ) {
				diskEntities.replace(lastIndex, lineEditDiskEntity->text());
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/DiskEntities", diskEntities);
			}
		}
		lineEditSetEntity->setText(m_defaultSetEntity);
		lineEditRomEntity->setText(m_defaultRomEntity);
		lineEditDiskEntity->setText(m_defaultDiskEntity);
		frameEntities->setVisible(false);
		if ( romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SYSTEM )
			showStateFilter(); // FIXME: add software-state filteirng
		QTimer::singleShot(0, this, SLOT(scrollToEnd()));
		toolButtonRemoveXmlSource->setVisible(false);
		lastIndex = -1;
	} else if ( index == comboBoxXmlSource->count() - 1 ) {
		QString xmlSource = QFileDialog::getOpenFileName(this, tr("Choose source XML file"), QString(), tr("Data and XML files (*.dat *.xml)") + ";;" + tr("Data files (*.dat)") + ";;" + tr("XML files (*.xml)") + ";;" + tr("All files (*)"), 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
		if ( !xmlSource.isNull() ) {
			int foundAtIndex = comboBoxXmlSource->findText(xmlSource);
			if ( foundAtIndex < 0 ) {
				comboBoxXmlSource->blockSignals(true);
				QStringList xmlSources(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSources", QStringList()).toStringList());
				xmlSources << xmlSource;
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSources", xmlSources);
				QStringList setEntities(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetEntities", QStringList()).toStringList());
				setEntities << m_defaultSetEntity;
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetEntities", setEntities);
				lineEditSetEntity->setText(m_defaultSetEntity);
				QStringList romEntities(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/RomEntities", QStringList()).toStringList());
				romEntities << m_defaultRomEntity;
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/RomEntities", romEntities);
				lineEditRomEntity->setText(m_defaultRomEntity);
				QStringList diskEntities(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/DiskEntities", QStringList()).toStringList());
				diskEntities << m_defaultDiskEntity;
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/DiskEntities", diskEntities);
				lineEditDiskEntity->setText(m_defaultDiskEntity);
				QStringList checkpointList(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceCheckpoints", QStringList()).toStringList());
				checkpointList << "-1";
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceCheckpoints", checkpointList);
				if ( romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE ) {
					QStringList softwareCheckpointList = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceListCheckpoints", QStringList()).toStringList();
					softwareCheckpointList << QString();
					qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceListCheckpoints", softwareCheckpointList);
				}
				int insertIndex = comboBoxXmlSource->count() - 2;
				lastIndex = insertIndex - 1;
				comboBoxXmlSource->insertItem(insertIndex, xmlSource);
				comboBoxXmlSource->setItemIcon(insertIndex, m_iconNoCheckpoint);
				comboBoxXmlSource->setItemData(insertIndex, false);
				comboBoxXmlSource->setCurrentIndex(insertIndex);
				comboBoxXmlSource->blockSignals(false);
				frameEntities->setVisible(true);
				hideStateFilter();
				toolButtonRemoveXmlSource->setVisible(true);
				QTimer::singleShot(0, this, SLOT(scrollToEnd()));
			} else
				comboBoxXmlSource->setCurrentIndex(foundAtIndex);
		} else
			comboBoxXmlSource->setCurrentIndex(0);
		raise();
	} else {
		index -= 1;
		if ( index >= 0 ) {
			QStringList setEntities(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetEntities", QStringList()).toStringList());
			QStringList romEntities(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/RomEntities", QStringList()).toStringList());
			QStringList diskEntities(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/DiskEntities", QStringList()).toStringList());
			if ( lastIndex >= 0 ) {
				if ( lastIndex < setEntities.count() ) {
					setEntities.replace(lastIndex, lineEditSetEntity->text());
					qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetEntities", setEntities);
				}
				if ( lastIndex < romEntities.count() ) {
					romEntities.replace(lastIndex, lineEditRomEntity->text());
					qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/RomEntities", romEntities);
				}
				if ( lastIndex < diskEntities.count() ) {
					diskEntities.replace(lastIndex, lineEditDiskEntity->text());
					qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/DiskEntities", diskEntities);
				}
			}
			lastIndex = index;
			lineEditSetEntity->setText(setEntities[index]);
			lineEditRomEntity->setText(romEntities[index]);
			lineEditDiskEntity->setText(diskEntities[index]);
			frameEntities->setVisible(true);
			hideStateFilter();
			toolButtonRemoveXmlSource->setVisible(true);
			QTimer::singleShot(0, this, SLOT(scrollToEnd()));
		}
	}
}

void CollectionRebuilder::on_toolButtonRemoveXmlSource_clicked()
{
	int index = comboBoxXmlSource->currentIndex() - 1;
	comboBoxXmlSource->setCurrentIndex(0);
	QStringList xmlSources(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSources", QStringList()).toStringList());
	xmlSources.removeAt(index);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSources", xmlSources);
	QStringList setEntities(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetEntities", QStringList()).toStringList());
	setEntities.removeAt(index);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetEntities", setEntities);
	QStringList romEntities(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/RomEntities", QStringList()).toStringList());
	romEntities.removeAt(index);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/RomEntities", romEntities);
	QStringList diskEntities(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/DiskEntities", QStringList()).toStringList());
	diskEntities.removeAt(index);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/DiskEntities", diskEntities);
	QStringList checkpointList(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceCheckpoints", QStringList()).toStringList());
	checkpointList.removeAt(index);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceCheckpoints", checkpointList);
	if ( romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE ) {
		QStringList softwareCheckpointList = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceListCheckpoints", QStringList()).toStringList();
		softwareCheckpointList.removeAt(index);
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceListCheckpoints", softwareCheckpointList);
	}
	comboBoxXmlSource->blockSignals(true);
	comboBoxXmlSource->removeItem(index + 1);
	comboBoxXmlSource->blockSignals(false);
}

void CollectionRebuilder::on_toolButtonViewMissingList_clicked()
{
	if ( missingDumpsViewer() ) {
		if ( missingDumpsViewer()->isVisible() )
			missingDumpsViewer()->hide();
		else
			missingDumpsViewer()->show();
	} else {
		m_missingDumpsViewer = new MissingDumpsViewer(romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE ? "SoftwareMissingDumpsViewer" : "MissingDumpsViewer", 0);
		// FIXME "non-default emulator"
		//missingDumpsViewer()->frameExport->setVisible(romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE ? false : true);
		missingDumpsViewer()->setDefaultEmulator(defaultEmulator());
		missingDumpsViewer()->frameExport->setVisible(romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE ? false : defaultEmulator());
		if ( rebuilderThread()->isActive )
			missingDumpsViewer()->toolButtonExportToDataFile->setEnabled(false);
		else
			missingDumpsViewer()->toolButtonExportToDataFile->setEnabled(!newMissingList().isEmpty() || missingDumpsViewer()->treeWidget->topLevelItemCount() > 0);
		if ( !newMissingList().isEmpty() )
			QTimer::singleShot(0, this, SLOT(updateMissingList()));
		missingDumpsViewer()->show();
	}
}

void CollectionRebuilder::rebuilderThread_rebuildStarted()
{
	if ( missingDumpsViewer() ) {
		missingDumpsViewer()->treeWidget->clear();
		missingDumpsViewer()->toolButtonExportToDataFile->setEnabled(false);
	}
	pushButtonStartStop->setIcon(QIcon(QString::fromUtf8(":/data/img/halt.png")));
	if ( rebuilderThread()->dryRun ) {
		pushButtonStartStop->setText(tr("Stop dry run"));
		pushButtonStartStop->setToolTip(tr("Start / stop dry run"));
	} else {
		pushButtonStartStop->setText(tr("Stop rebuilding"));
		pushButtonStartStop->setToolTip(tr("Start / stop rebuilding"));
	}
	pushButtonPauseResume->setText(tr("Pause"));
	pushButtonPauseResume->show();
	pushButtonStartStop->setEnabled(true);
	pushButtonPauseResume->setEnabled(true);
	comboBoxXmlSource->setEnabled(false);
	comboBoxXmlSource->setItemIcon(comboBoxXmlSource->currentIndex(), m_iconCheckpoint);
	comboBoxXmlSource->setItemData(comboBoxXmlSource->currentIndex(), true);
	labelXmlSource->setEnabled(false);
	toolButtonRemoveXmlSource->setEnabled(false);
	comboBoxModeSwitch->setEnabled(false);
	checkBoxFilterExpression->setEnabled(false);
	checkBoxFilterExpressionSoftwareLists->setEnabled(false);
	comboBoxFilterSyntax->setEnabled(false);
	comboBoxFilterSyntaxSoftwareLists->setEnabled(false);
	comboBoxFilterType->setEnabled(false);
	comboBoxFilterTypeSoftwareLists->setEnabled(false);
	lineEditFilterExpression->setEnabled(false);
	lineEditFilterExpressionSoftwareLists->setEnabled(false);
	toolButtonExactMatch->setEnabled(false);
	toolButtonExactMatchSoftwareLists->setEnabled(false);
	toolButtonClearFilterExpression->setEnabled(false);
	toolButtonClearFilterExpressionSoftwareLists->setEnabled(false);
	checkBoxFilterStates->setEnabled(false);
	toolButtonStateC->setEnabled(false);
	toolButtonStateM->setEnabled(false);
	toolButtonStateI->setEnabled(false);
	toolButtonStateN->setEnabled(false);
	toolButtonStateU->setEnabled(false);
	frameEntities->setEnabled(false);
	romAlyzer()->groupBoxCheckSumDatabase->setEnabled(false);
	romAlyzer()->groupBoxSetRewriter->setEnabled(false);
	romAlyzer()->checkBoxCollectionRebuilderDryRun->setEnabled(false);
	m_animationSequence = 0;
	m_animationTimer.start(QMC2_ROMALYZER_REBUILD_ANIM_SPEED);
	if ( romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE && qmc2SoftwareList ) {
		qmc2SoftwareList->rebuildMenuAction->setEnabled(false);
		qmc2SoftwareList->actionRebuildSoftware->setEnabled(false);
		qmc2SoftwareList->actionRebuildSoftwareList->setEnabled(false);
		qmc2SoftwareList->actionRebuildSoftwareLists->setEnabled(false);
		qmc2SoftwareList->toolButtonRebuildSoftware->setEnabled(false);
	}
	qApp->processEvents();
}

void CollectionRebuilder::rebuilderThread_rebuildFinished()
{
	int index = comboBoxXmlSource->currentIndex();
	qint64 cp = rebuilderThread()->checkpoint();
	if ( index == 0 )
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/Checkpoint", cp);
	else {
		int xmlSourceIndex = index - 1;
		QStringList checkpointList(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceCheckpoints", QStringList()).toStringList());
		if ( xmlSourceIndex >= 0 && xmlSourceIndex < checkpointList.count() ) {
			checkpointList.replace(xmlSourceIndex, QString::number(cp));
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceCheckpoints", checkpointList);
			if ( romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE ) {
				QStringList softwareCheckpointList = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceListCheckpoints", QStringList()).toStringList();
				if ( xmlSourceIndex < softwareCheckpointList.count() ) {
					softwareCheckpointList.replace(xmlSourceIndex, rebuilderThread()->currentListName());
					qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceListCheckpoints", softwareCheckpointList);
				}
			}
		}
	}
	if ( index < 0 )
		return;
	if ( cp >= 0 ) {
		comboBoxXmlSource->setItemIcon(index, m_iconCheckpoint);
		comboBoxXmlSource->setItemData(index, true);
	} else {
		comboBoxXmlSource->setItemIcon(index, m_iconNoCheckpoint);
		comboBoxXmlSource->setItemData(index, false);
	}
	if ( missingDumpsViewer() ) {
		missingDumpsViewer()->toolButtonExportToDataFile->setEnabled(!newMissingList().isEmpty() || missingDumpsViewer()->treeWidget->topLevelItemCount() > 0);
		missingDumpsViewer()->setDefaultEmulator(defaultEmulator());
	}
	pushButtonStartStop->setIcon(QIcon(QString::fromUtf8(":/data/img/refresh.png")));
	if ( romAlyzer()->checkBoxCollectionRebuilderDryRun->isChecked() ) {
		pushButtonStartStop->setText(tr("Start dry run"));
		pushButtonStartStop->setToolTip(tr("Start / stop dry run"));
	} else {
		pushButtonStartStop->setText(tr("Start rebuilding"));
		pushButtonStartStop->setToolTip(tr("Start / stop rebuilding"));
	}
	pushButtonPauseResume->hide();
	pushButtonStartStop->setEnabled(true);
	pushButtonPauseResume->setEnabled(true);
	comboBoxXmlSource->setEnabled(true);
	labelXmlSource->setEnabled(true);
	toolButtonRemoveXmlSource->setEnabled(true);
	comboBoxModeSwitch->setEnabled(true);
	checkBoxFilterExpression->setEnabled(true);
	checkBoxFilterExpressionSoftwareLists->setEnabled(true);
	comboBoxFilterSyntax->setEnabled(checkBoxFilterExpression->isChecked());
	comboBoxFilterSyntaxSoftwareLists->setEnabled(checkBoxFilterExpressionSoftwareLists->isChecked());
	comboBoxFilterType->setEnabled(checkBoxFilterExpression->isChecked());
	comboBoxFilterTypeSoftwareLists->setEnabled(checkBoxFilterExpressionSoftwareLists->isChecked());
	lineEditFilterExpression->setEnabled(checkBoxFilterExpression->isChecked());
	lineEditFilterExpressionSoftwareLists->setEnabled(checkBoxFilterExpressionSoftwareLists->isChecked());
	toolButtonExactMatch->setEnabled(checkBoxFilterExpression->isChecked());
	toolButtonExactMatchSoftwareLists->setEnabled(checkBoxFilterExpressionSoftwareLists->isChecked());
	toolButtonClearFilterExpression->setEnabled(checkBoxFilterExpression->isChecked());
	toolButtonClearFilterExpressionSoftwareLists->setEnabled(checkBoxFilterExpressionSoftwareLists->isChecked());
	checkBoxFilterStates->setEnabled(true);
	toolButtonStateC->setEnabled(checkBoxFilterStates->isChecked());
	toolButtonStateM->setEnabled(checkBoxFilterStates->isChecked());
	toolButtonStateI->setEnabled(checkBoxFilterStates->isChecked());
	toolButtonStateN->setEnabled(checkBoxFilterStates->isChecked());
	toolButtonStateU->setEnabled(checkBoxFilterStates->isChecked());
	frameEntities->setEnabled(true);
	romAlyzer()->groupBoxCheckSumDatabase->setEnabled(true);
	romAlyzer()->groupBoxSetRewriter->setEnabled(true);
	romAlyzer()->checkBoxCollectionRebuilderDryRun->setEnabled(true);
	m_animationTimer.stop();
	romAlyzer()->tabWidgetAnalysis->setTabIcon(QMC2_ROMALYZER_PAGE_RCR, QIcon(QString::fromUtf8(":/data/img/rebuild.png")));
	if ( romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE && qmc2SoftwareList ) {
		qmc2SoftwareList->rebuildMenuAction->setEnabled(true);
		qmc2SoftwareList->actionRebuildSoftware->setEnabled(true);
		qmc2SoftwareList->actionRebuildSoftwareList->setEnabled(true);
		qmc2SoftwareList->actionRebuildSoftwareLists->setEnabled(true);
		qmc2SoftwareList->toolButtonRebuildSoftware->setEnabled(true);
	}
	qApp->processEvents();
	if ( !newMissingList().isEmpty() && missingDumpsViewer() )
		QTimer::singleShot(0, this, SLOT(updateMissingList()));
}

void CollectionRebuilder::rebuilderThread_rebuildPaused()
{
	pushButtonPauseResume->setText(tr("Resume"));
	pushButtonPauseResume->setEnabled(true);
}

void CollectionRebuilder::rebuilderThread_rebuildResumed()
{
	pushButtonPauseResume->setText(tr("Pause"));
	pushButtonPauseResume->setEnabled(true);
}

void CollectionRebuilder::rebuilderThread_progressTextChanged(const QString &text)
{
	progressBar->setFormat(text);
}

void CollectionRebuilder::rebuilderThread_progressRangeChanged(int min, int max)
{
	progressBar->setRange(min, max);
}

void CollectionRebuilder::rebuilderThread_progressChanged(int progress)
{
	progressBar->setValue(progress);
}

void CollectionRebuilder::rebuilderThread_statusUpdated(quint64 setsProcessed, quint64 missingDumps, quint64 missingDisks)
{
	QString statusString("<table border=\"0\" cellpadding=\"0\" cellspacing=\"4\" width=\"100%\"><tr>");
	statusString += "<td nowrap align=\"left\" width=\"16.16%\"><b>" + tr("Sets processed") + ":</b></td><td nowrap align=\"right\" width=\"16.16%\">" + QString::number(setsProcessed) + "</td>";
	statusString += "<td nowrap align=\"center\" width=\"1%\">|</td>";
	statusString += "<td nowrap align=\"left\" width=\"16.16%\"><b>" + tr("Missing ROMs") + ":</b></td><td nowrap align=\"right\" width=\"16.16%\">" + QString::number(missingDumps) + "</td>";
	statusString += "<td nowrap align=\"center\" width=\"1%\">|</td>";
	statusString += "<td nowrap align=\"left\" width=\"16.16%\"><b>" + tr("Missing disks") + ":</b></td><td nowrap align=\"right\" width=\"16.16%\">" + QString::number(missingDisks) + "</td>";
	statusString += "<td nowrap align=\"right\" width=\"1%\">|</td>";
	statusString += "</tr></table>";
	labelRebuildStatus->setText(statusString);
}

void CollectionRebuilder::rebuilderThread_newMissing(QString id, QString type, QString name, QString size, QString crc, QString sha1, QString reason)
{
	newMissingList() << id + "|" + type + "|" + name + "|" + size + "|" + crc + "|" + sha1 + "|" + reason;
}

void CollectionRebuilder::animationTimer_timeout()
{
	m_animationTimer.stop();
	QPixmap pixmap(QString::fromUtf8(":/data/img/rebuild.png"));
	QPixmap rotatedPixmap(pixmap.size());
	rotatedPixmap.fill(Qt::transparent);
	QPainter p(&rotatedPixmap); 
	QSize size = pixmap.size();
	p.translate(size.height()/2,size.height()/2);
	p.rotate(24 * ++m_animationSequence);
	if ( m_animationSequence > 14 )
		m_animationSequence = 0;
	p.translate(-size.height()/2,-size.height()/2);
	p.drawPixmap(0, 0, pixmap);
	p.end();
	romAlyzer()->tabWidgetAnalysis->setTabIcon(QMC2_ROMALYZER_PAGE_RCR, QIcon(rotatedPixmap));
	m_animationTimer.start(QMC2_ROMALYZER_REBUILD_ANIM_SPEED);
	if ( !newMissingList().isEmpty() && missingDumpsViewer() )
		QTimer::singleShot(0, this, SLOT(updateMissingList()));
}

void CollectionRebuilder::updateMissingList()
{
	QList<QTreeWidgetItem *> itemList;
	foreach (QString newMissing, newMissingList()) {
		QStringList missingWords(newMissing.split('|'));
		if ( missingWords.count() >= 7 ) {
			QTreeWidgetItem *newItem = new QTreeWidgetItem(0);
			newItem->setText(QMC2_MDV_COLUMN_ID, missingWords.at(0));
			newItem->setText(QMC2_MDV_COLUMN_TYPE, missingWords.at(1));
			newItem->setText(QMC2_MDV_COLUMN_NAME, missingWords.at(2));
			newItem->setText(QMC2_MDV_COLUMN_SIZE, missingWords.at(3));
			newItem->setText(QMC2_MDV_COLUMN_CRC, missingWords.at(4));
			newItem->setText(QMC2_MDV_COLUMN_SHA1, missingWords.at(5));
			newItem->setText(QMC2_MDV_COLUMN_REASON, missingWords.at(6));
			itemList << newItem;
		}
	}
	missingDumpsViewer()->treeWidget->insertTopLevelItems(0, itemList);
	newMissingList().clear();
	if ( rebuilderThread()->isActive )
		missingDumpsViewer()->toolButtonExportToDataFile->setEnabled(false);
	else
		missingDumpsViewer()->toolButtonExportToDataFile->setEnabled(missingDumpsViewer()->treeWidget->topLevelItemCount() > 0);
}

void CollectionRebuilder::updateModeSetup()
{
	if ( active() ) {
		if ( rebuilderThread()->dryRun ) {
			pushButtonStartStop->setText(tr("Stop dry run"));
			pushButtonStartStop->setToolTip(tr("Start / stop dry run"));
		} else {
			pushButtonStartStop->setText(tr("Stop rebuilding"));
			pushButtonStartStop->setToolTip(tr("Start / stop rebuilding"));
		}
	} else {
		if ( romAlyzer()->checkBoxCollectionRebuilderDryRun->isChecked() ){
			pushButtonStartStop->setText(tr("Start dry run"));
			pushButtonStartStop->setToolTip(tr("Start / stop dry run"));
			comboBoxModeSwitch->blockSignals(true);
			comboBoxModeSwitch->setCurrentIndex(1);
			comboBoxModeSwitch->blockSignals(false);
		} else {
			pushButtonStartStop->setText(tr("Start rebuilding"));
			pushButtonStartStop->setToolTip(tr("Start / stop rebuilding"));
			comboBoxModeSwitch->blockSignals(true);
			comboBoxModeSwitch->setCurrentIndex(0);
			comboBoxModeSwitch->blockSignals(false);
		}
	}
}

void CollectionRebuilder::on_comboBoxModeSwitch_currentIndexChanged(int index)
{
	if ( index == 0 ) {
		romAlyzer()->checkBoxCollectionRebuilderDryRun->setChecked(false);
		pushButtonStartStop->setText(tr("Start rebuilding"));
		pushButtonStartStop->setToolTip(tr("Start / stop rebuilding"));
	} else {
		romAlyzer()->checkBoxCollectionRebuilderDryRun->setChecked(true);
		pushButtonStartStop->setText(tr("Start dry run"));
		pushButtonStartStop->setToolTip(tr("Start / stop dry run"));
	}
}

void CollectionRebuilder::showEvent(QShowEvent *e)
{
	QTimer::singleShot(0, this, SLOT(updateModeSetup()));
}

void CollectionRebuilder::hideEvent(QHideEvent *e)
{
	switch ( romAlyzer()->mode() ) {
		case QMC2_ROMALYZER_MODE_SOFTWARE:
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/UseFilterExpressionSoftwareLists", checkBoxFilterExpressionSoftwareLists->isChecked());
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterSyntaxSoftwareLists", comboBoxFilterSyntaxSoftwareLists->currentIndex());
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterTypeSoftwareLists", comboBoxFilterTypeSoftwareLists->currentIndex());
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/ExactMatchSoftwareLists", toolButtonExactMatchSoftwareLists->isChecked());
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterExpressionSoftwareLists", lineEditFilterExpressionSoftwareLists->text());
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/UseFilterExpression", checkBoxFilterExpression->isChecked());
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterSyntax", comboBoxFilterSyntax->currentIndex());
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterType", comboBoxFilterType->currentIndex());
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/ExactMatch", toolButtonExactMatch->isChecked());
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterExpression", lineEditFilterExpression->text());
			break;
		case QMC2_ROMALYZER_MODE_SYSTEM:
		default:
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/UseFilterExpression", checkBoxFilterExpression->isChecked());
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterSyntax", comboBoxFilterSyntax->currentIndex());
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterType", comboBoxFilterType->currentIndex());
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/ExactMatch", toolButtonExactMatch->isChecked());
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterExpression", lineEditFilterExpression->text());
			break;
	}
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/UseStateFilter", checkBoxFilterStates->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/IncludeStateC", toolButtonStateC->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/IncludeStateM", toolButtonStateM->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/IncludeStateI", toolButtonStateI->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/IncludeStateN", toolButtonStateN->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/IncludeStateU", toolButtonStateU->isChecked());
	if ( missingDumpsViewer() )
		missingDumpsViewer()->close();
	if ( e )
		e->ignore();
}

CollectionRebuilderThread::CollectionRebuilderThread(QObject *parent)
	: QThread(parent)
{
	isActive = exitThread = isWaiting = isPaused = pauseRequested = stopRebuilding = doFilter = doFilterSoftware = isIncludeFilter = isIncludeFilterSoftware = doFilterState = false;
	includeStateC = includeStateM = includeStateI = includeStateN = includeStateU = true;
	exactMatch = exactMatchSoftware = useHashCache = dryRun = false;
	m_rebuilderDialog = (CollectionRebuilder *)parent;
	m_checkSumDb = 0;
	m_xmlIndex = m_xmlIndexCount = m_checkpoint = -1;
	m_hashCacheUpdateTime = 0;
	setListEntityStartPattern("<softwarelist name=\"");
	if ( m_replacementHash.isEmpty() ) {
		m_replacementHash.insert("&amp;", "&");
		m_replacementHash.insert("&lt;", "<");
		m_replacementHash.insert("&gt;", ">");
		m_replacementHash.insert("&quot;", "\"");
		m_replacementHash.insert("&apos;", "'");
	}
	if ( m_fileTypes.isEmpty() )
		m_fileTypes << "ZIP" << "7Z" << "CHD" << "FILE";
	reopenCheckSumDb();
	switch ( rebuilderDialog()->romAlyzer()->mode() ) {
		case QMC2_ROMALYZER_MODE_SOFTWARE:
			m_swlDb = new SoftwareListXmlDatabaseManager(this);
			m_xmlDb = 0;
			break;
		case QMC2_ROMALYZER_MODE_SYSTEM:
		default:
			m_xmlDb = new XmlDatabaseManager(this);
			m_swlDb = 0;
			break;
	}
	start();
}

CollectionRebuilderThread::~CollectionRebuilderThread()
{
	exitThread = true;
	waitCondition.wakeAll();
	wait();
	if ( checkSumDb() ) {
		checkSumDb()->disconnect(rebuilderDialog());
		delete checkSumDb();
	}
	if ( xmlDb() )
		delete xmlDb();
	if ( swlDb() )
		delete swlDb();
}

void CollectionRebuilderThread::reopenCheckSumDb()
{
	if ( checkSumDb() ) {
		checkSumDb()->disconnect(rebuilderDialog());
		delete checkSumDb();
	}
	m_checkSumDb = new CheckSumDatabaseManager(this, rebuilderDialog()->romAlyzer()->settingsKey());
	connect(checkSumDb(), SIGNAL(log(const QString &)), rebuilderDialog(), SLOT(log(const QString &)));
}

bool CollectionRebuilderThread::parseXml(QString xml, QString *id, QStringList *romNameList, QStringList *romSha1List, QStringList *romCrcList, QStringList *romSizeList, QStringList *diskNameList, QStringList *diskSha1List, QStringList *diskSizeList)
{
	static const QString statusString("status=\"");
	static const QString mergeString("merge=\"");
	static const QString sha1String("sha1=\"");
	static const QString crcString("crc=\"");
	static const QString sizeString("size=\"");
	static const QChar quoteChar('\"');

	if ( xml.isEmpty() )
		return false;

	int startIndex = -1;
	int endIndex = -1;
	QStringList xmlLines(xml.split('\n'));
	QString xmlLine(xmlLines.at(0));
	startIndex = xmlLine.indexOf(setEntityPattern());
	if ( startIndex >= 0 ) {
		startIndex += setEntityPattern().length();
		endIndex = xmlLine.indexOf(quoteChar, startIndex);
		if ( endIndex >= 0 ) {
			*id = xmlLine.mid(startIndex, endIndex - startIndex);
			QString mergeName, romName, status, romSha1, romCrc, romSize, diskName, diskSha1, diskSize;
			for (int i = 1; i < xmlLines.count(); i++) {
				xmlLine = xmlLines.at(i);
				bool romFound = false;
				startIndex = xmlLine.indexOf(romEntityPattern());
				if ( startIndex >= 0 ) {
					startIndex += romEntityPattern().length();
					endIndex = xmlLine.indexOf(quoteChar, startIndex);
					if ( endIndex >= 0 ) {
						romFound = true;
						romName = xmlLine.mid(startIndex, endIndex - startIndex);
						mergeName.clear();
						status.clear();
						startIndex = xmlLine.indexOf(statusString);
						if ( startIndex >= 0 ) {
							startIndex += statusString.length();
							endIndex = xmlLine.indexOf(quoteChar, startIndex);
							if ( endIndex >= 0 )
								status = xmlLine.mid(startIndex, endIndex - startIndex);
						}
						startIndex = xmlLine.indexOf(mergeString);
						if ( startIndex >= 0 ) {
							startIndex += mergeString.length();
							endIndex = xmlLine.indexOf(quoteChar, startIndex);
							if ( endIndex >= 0 )
								mergeName = xmlLine.mid(startIndex, endIndex - startIndex);
						}
						if ( status != "nodump" && (!merge() || mergeName.isEmpty()) ) {
							romSha1.clear(); romCrc.clear(); romSize.clear();
							startIndex = xmlLine.indexOf(sha1String);
							if ( startIndex >= 0 ) {
								startIndex += sha1String.length();
								endIndex = xmlLine.indexOf(quoteChar, startIndex);
								if ( endIndex >= 0 )
									romSha1 = xmlLine.mid(startIndex, endIndex - startIndex);
							}
							startIndex = xmlLine.indexOf(crcString);
							if ( startIndex >= 0 ) {
								startIndex += crcString.length();
								endIndex = xmlLine.indexOf(quoteChar, startIndex);
								if ( endIndex >= 0 )
									romCrc = xmlLine.mid(startIndex, endIndex - startIndex);
							}
							startIndex = xmlLine.indexOf(sizeString);
							if ( startIndex >= 0 ) {
								startIndex += sizeString.length();
								endIndex = xmlLine.indexOf(quoteChar, startIndex);
								if ( endIndex >= 0 )
									romSize = xmlLine.mid(startIndex, endIndex - startIndex);
							}
							if ( !romSha1.isEmpty() || !romCrc.isEmpty() ) {
								romNameList->append(toHumanReadable(romName));
								romSha1List->append(romSha1);
								romCrcList->append(romCrc);
								romSizeList->append(romSize);
							}
						}
					}
				}
				if ( romFound )
					continue;
				startIndex = xmlLine.indexOf(diskEntityPattern());
				if ( startIndex >= 0 ) {
					startIndex += diskEntityPattern().length();
					endIndex = xmlLine.indexOf(quoteChar, startIndex);
					if ( endIndex >= 0 ) {
						diskName = xmlLine.mid(startIndex, endIndex - startIndex);
						mergeName.clear();
						startIndex = xmlLine.indexOf(mergeString);
						if ( startIndex >= 0 ) {
							startIndex += mergeString.length();
							endIndex = xmlLine.indexOf(quoteChar, startIndex);
							if ( endIndex >= 0 )
								mergeName = xmlLine.mid(startIndex, endIndex - startIndex);
						}
						if ( !merge() || mergeName.isEmpty() ) {
							diskSha1.clear();
							startIndex = xmlLine.indexOf(sha1String);
							if ( startIndex >= 0 ) {
								startIndex += sha1String.length();
								endIndex = xmlLine.indexOf(quoteChar, startIndex);
								if ( endIndex >= 0 )
									diskSha1 = xmlLine.mid(startIndex, endIndex - startIndex);
							}
							diskSize.clear();
							startIndex = xmlLine.indexOf(sizeString);
							if ( startIndex >= 0 ) {
								startIndex += sizeString.length();
								endIndex = xmlLine.indexOf(quoteChar, startIndex);
								if ( endIndex >= 0 )
									diskSize = xmlLine.mid(startIndex, endIndex - startIndex);
							}
							if ( !diskSha1.isEmpty() ) {
								diskNameList->append(toHumanReadable(diskName));
								diskSha1List->append(diskSha1);
								diskSizeList->append(diskSize);
							}
						}
					}
				}
			}
			return true;
		} else
			return false;
	} else
		return false;
}

bool CollectionRebuilderThread::nextId(QString *id, QStringList *romNameList, QStringList *romSha1List, QStringList *romCrcList, QStringList *romSizeList, QStringList *diskNameList, QStringList *diskSha1List, QStringList *diskSizeList)
{
	id->clear();
	romNameList->clear();
	romSha1List->clear();
	romCrcList->clear();
	romSizeList->clear();
	diskNameList->clear();
	diskSha1List->clear();
	diskSizeList->clear();
	if ( m_xmlIndex < 0 || m_xmlIndexCount < 0 ) {
		if ( rebuilderDialog()->defaultEmulator() ) {
			m_xmlIndex = 0;
			switch ( rebuilderDialog()->romAlyzer()->mode() ) {
				case QMC2_ROMALYZER_MODE_SOFTWARE:
					swlDb()->initIdAtIndexCache();
					m_xmlIndexCount = swlDb()->idAtIndexCacheSize();
					break;
				case QMC2_ROMALYZER_MODE_SYSTEM:
				default:
					xmlDb()->initIdAtIndexCache();
					m_xmlIndexCount = xmlDb()->idAtIndexCacheSize();
					break;
			}
			emit progressRangeChanged(m_xmlIndex, m_xmlIndexCount);
			emit progressChanged(m_xmlIndex);
			setCheckpoint(m_xmlIndex, rebuilderDialog()->comboBoxXmlSource->currentIndex());
			return true;
		} else {
			m_xmlFile.setFileName(rebuilderDialog()->comboBoxXmlSource->currentText());
			if ( m_xmlFile.open(QIODevice::ReadOnly | QIODevice::Text) ) {
				m_xmlIndex = 0;
				m_xmlIndexCount = m_xmlFile.size() - 1;
				emit progressRangeChanged(m_xmlIndex, m_xmlIndexCount);
				emit progressChanged(m_xmlIndex);
				setCheckpoint(0, rebuilderDialog()->comboBoxXmlSource->currentIndex());
				return true;
			} else {
				emit log(tr("FATAL: can't open XML file '%1' for reading, please check permissions").arg(rebuilderDialog()->comboBoxXmlSource->currentText()));
				setCheckpoint(-1, rebuilderDialog()->comboBoxXmlSource->currentIndex());
				return false;
			}
		}
	} else {
		if ( m_xmlIndex > m_xmlIndexCount ) {
			emit progressChanged(m_xmlIndexCount);
			setCheckpoint(-1, rebuilderDialog()->comboBoxXmlSource->currentIndex());
			return false;
		}
		if ( rebuilderDialog()->defaultEmulator() ) {
			QString setKey;
			switch ( rebuilderDialog()->romAlyzer()->mode() ) {
				case QMC2_ROMALYZER_MODE_SOFTWARE:
					setKey = swlDb()->idAtIndex(m_xmlIndex);
					if ( !evaluateFilters(setKey) ) {
						m_xmlIndex++;
						emit progressChanged(m_xmlIndex);
						return true;
					}
					if ( parseXml(swlDb()->xml(setKey), id, romNameList, romSha1List, romCrcList, romSizeList, diskNameList, diskSha1List, diskSizeList) ) {
						*id = setKey;
						m_xmlIndex++;
						emit progressChanged(m_xmlIndex);
						return true;
					} else {
						emit log(tr("FATAL: XML parsing failed"));
						setCheckpoint(-1, rebuilderDialog()->comboBoxXmlSource->currentIndex());
						return false;
					}
				case QMC2_ROMALYZER_MODE_SYSTEM:
				default:
					setKey = xmlDb()->idAtIndex(m_xmlIndex);
					if ( !evaluateFilters(setKey) ) {
						m_xmlIndex++;
						emit progressChanged(m_xmlIndex);
						return true;
					}
					if ( parseXml(xmlDb()->xml(setKey), id, romNameList, romSha1List, romCrcList, romSizeList, diskNameList, diskSha1List, diskSizeList) ) {
						m_xmlIndex++;
						emit progressChanged(m_xmlIndex);
						return true;
					} else {
						emit log(tr("FATAL: XML parsing failed"));
						setCheckpoint(-1, rebuilderDialog()->comboBoxXmlSource->currentIndex());
						return false;
					}
			}
		} else {
			QByteArray line(m_xmlFile.readLine());
			while ( !m_xmlFile.atEnd() && line.indexOf(setEntityStartPattern()) < 0 && (rebuilderDialog()->romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE ? line.indexOf(listEntityStartPattern()) < 0 : true) && !exitThread )
				line = m_xmlFile.readLine();
			if ( m_xmlFile.atEnd() ) {
				emit progressChanged(m_xmlIndexCount);
				setCheckpoint(-1, rebuilderDialog()->comboBoxXmlSource->currentIndex());
				return false;
			} else if ( !exitThread ) {
				QString xmlString;
				if ( rebuilderDialog()->romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE ) {
					int startIndex = line.indexOf(listEntityStartPattern());
					if ( startIndex >= 0 ) {
						startIndex += listEntityStartPattern().length();
						setListCheckpoint(line.mid(startIndex, line.indexOf("\"", startIndex) - startIndex), rebuilderDialog()->comboBoxXmlSource->currentIndex());
						return true;
					}
				}
				QString setEntityEndPattern("</" + rebuilderDialog()->lineEditSetEntity->text() + ">");
				while ( !m_xmlFile.atEnd() && line.indexOf(setEntityEndPattern) < 0 && !exitThread ) {
					xmlString += line;
					line = m_xmlFile.readLine();
				}
				if ( !m_xmlFile.atEnd() && !exitThread ) {
					xmlString += line;
					if ( parseXml(xmlString, id, romNameList, romSha1List, romCrcList, romSizeList, diskNameList, diskSha1List, diskSizeList) ) {
						if ( rebuilderDialog()->romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE && !m_currentListName.isEmpty() )
							id->prepend(m_currentListName + ":");
						if ( !evaluateFilters(*id) )
							id->clear();
						m_xmlIndex = m_xmlFile.pos();
						emit progressChanged(m_xmlIndex);
						return true;
					} else {
						emit log(tr("FATAL: XML parsing failed"));
						setCheckpoint(-1, rebuilderDialog()->comboBoxXmlSource->currentIndex());
						return false;
					}
				} else {
					emit log(tr("FATAL: XML parsing failed"));
					setCheckpoint(-1, rebuilderDialog()->comboBoxXmlSource->currentIndex());
					return false;
				}
			} else {
				setCheckpoint(-1, rebuilderDialog()->comboBoxXmlSource->currentIndex());
				return false;
			}
		}
	}
}

void CollectionRebuilderThread::setCheckpoint(qint64 cp, int xmlSourceIndex)
{
	m_checkpoint = cp;
	if ( xmlSourceIndex == 0 )
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + rebuilderDialog()->settingsKey() + "/Checkpoint", checkpoint());
	else if ( xmlSourceIndex > 0 && xmlSourceIndex < rebuilderDialog()->comboBoxXmlSource->count() - 1 ) {
		xmlSourceIndex -= 1;
		QStringList checkpointList(qmc2Config->value(QMC2_FRONTEND_PREFIX + rebuilderDialog()->settingsKey() + "/XmlSourceCheckpoints", QStringList()).toStringList());
		checkpointList.replace(xmlSourceIndex, QString::number(checkpoint()));
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + rebuilderDialog()->settingsKey() + "/XmlSourceCheckpoints", checkpointList);
	}
}

void CollectionRebuilderThread::setListCheckpoint(QString list, int xmlSourceIndex)
{
	m_currentListName = list;
	if ( xmlSourceIndex > 0 && xmlSourceIndex < rebuilderDialog()->comboBoxXmlSource->count() - 1 ) {
		xmlSourceIndex -= 1;
		QStringList checkpointList(qmc2Config->value(QMC2_FRONTEND_PREFIX + rebuilderDialog()->settingsKey() + "/XmlSourceListCheckpoints", QStringList()).toStringList());
		checkpointList.replace(xmlSourceIndex, list);
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + rebuilderDialog()->settingsKey() + "/XmlSourceListCheckpoints", checkpointList);
	}
}

void CollectionRebuilderThread::checkpointRestart(qint64 cp)
{
	if ( cp < 0 ) {
		m_xmlIndex = m_xmlIndexCount = -1;
		return;
	}
	m_xmlIndex = cp;
	if ( rebuilderDialog()->defaultEmulator() ) {
		emit log(tr("restarting from checkpoint '%1'").arg(m_xmlIndex));
		switch ( rebuilderDialog()->romAlyzer()->mode() ) {
			case QMC2_ROMALYZER_MODE_SOFTWARE:
				swlDb()->initIdAtIndexCache();
				m_xmlIndexCount = swlDb()->idAtIndexCacheSize();
				break;
			case QMC2_ROMALYZER_MODE_SYSTEM:
			default:
				xmlDb()->initIdAtIndexCache();
				m_xmlIndexCount = xmlDb()->idAtIndexCacheSize();
				break;
		}
		emit progressRangeChanged(m_xmlIndex, m_xmlIndexCount);
		emit progressChanged(m_xmlIndex);
	} else {
		if ( m_xmlFile.isOpen() )
			m_xmlFile.close();
		m_xmlFile.setFileName(rebuilderDialog()->comboBoxXmlSource->currentText());
		if ( m_xmlFile.open(QIODevice::ReadOnly | QIODevice::Text) ) {
			emit log(tr("restarting from checkpoint '%1'").arg(m_xmlIndex));
			m_xmlIndexCount = m_xmlFile.size() - 1;
			m_xmlFile.seek(m_xmlIndex);
			emit progressRangeChanged(m_xmlIndex, m_xmlIndexCount);
			emit progressChanged(m_xmlIndex);
		} else {
			emit log(tr("FATAL: can't open XML file '%1' for reading, please check permissions").arg(rebuilderDialog()->comboBoxXmlSource->currentText()));
			m_xmlIndex = m_xmlIndexCount = -1;
		}
	}
	setCheckpoint(m_xmlIndex, rebuilderDialog()->comboBoxXmlSource->currentIndex());
}

bool CollectionRebuilderThread::rewriteSet(QString *setKey, QStringList *romNameList, QStringList *romSha1List, QStringList *romCrcList, QStringList *romSizeList, QStringList *diskNameList, QStringList *diskSha1List)
{
	QString set, baseDir = rebuilderDialog()->romAlyzer()->lineEditSetRewriterOutputPath->text();
	if ( rebuilderDialog()->romAlyzer()->mode() == QMC2_ROMALYZER_MODE_SOFTWARE ) {
		QStringList setKeyTokens(setKey->split(':', QString::SkipEmptyParts));
		if ( setKeyTokens.count() < 2 )
			return false;
		else {
			baseDir += "/" + setKeyTokens.at(0);
			set = setKeyTokens.at(1);
		}
	} else
		set = *setKey;
	bool rebuildOkay = true;
	if ( !romNameList->isEmpty() ) {
		switch ( rebuilderDialog()->romAlyzer()->comboBoxSetRewriterReproductionType->currentIndex() ) {
			case QMC2_ROMALYZER_RT_ZIP_BUILTIN:
				rebuildOkay = writeAllZipData(baseDir, set, romNameList, romSha1List, romCrcList, romSizeList, diskNameList, diskSha1List);
				break;
#if defined(QMC2_LIBARCHIVE_ENABLED)
			case QMC2_ROMALYZER_RT_ZIP_LIBARCHIVE:
				rebuildOkay = writeAllArchiveData(baseDir, set, romNameList, romSha1List, romCrcList, romSizeList, diskNameList, diskSha1List);
				break;
#endif
			case QMC2_ROMALYZER_RT_FOLDERS:
				rebuildOkay = writeAllFileData(baseDir, set, romNameList, romSha1List, romCrcList, romSizeList, diskNameList, diskSha1List);
				break;
			default:
				break;
		}
	}
	if ( rebuildOkay && !diskNameList->isEmpty() ) {
		switch ( rebuilderDialog()->romAlyzer()->comboBoxCollectionRebuilderCHDHandling->currentIndex() ) {
			case QMC2_COLLECTIONREBUILDER_CHD_HARDLINK:
				rebuildOkay = hardlinkChds(baseDir, set, diskNameList, diskSha1List);
				break;
			case QMC2_COLLECTIONREBUILDER_CHD_SYMLINK:
				rebuildOkay = symlinkChds(baseDir, set, diskNameList, diskSha1List);
				break;
			case QMC2_COLLECTIONREBUILDER_CHD_COPY:
				rebuildOkay = copyChds(baseDir, set, diskNameList, diskSha1List);
				break;
			case QMC2_COLLECTIONREBUILDER_CHD_MOVE:
				rebuildOkay = moveChds(baseDir, set, diskNameList, diskSha1List);
				break;
			default:
				break;
		}
	}
	return rebuildOkay;
}

bool CollectionRebuilderThread::writeAllFileData(QString baseDir, QString id, QStringList *romNameList, QStringList *romSha1List, QStringList *romCrcList, QStringList *romSizeList, QStringList * /*diskNameList*/, QStringList * /*diskSha1List*/)
{
	bool success = true;
	QDir d(QDir::cleanPath(baseDir + "/" + id));
	if ( !d.exists() )
		success = d.mkdir(QDir::cleanPath(baseDir + "/" + id));
	int reproducedDumps = 0;
	bool ignoreErrors = !rebuilderDialog()->romAlyzer()->checkBoxSetRewriterAbortOnError->isChecked();
	for (int i = 0; i < romNameList->count() && !exitThread && success; i++) {
		QString fileName(d.absoluteFilePath(romNameList->at(i)));
		if ( !createBackup(fileName) ) {
			emit log(tr("FATAL: backup creation failed"));
			success = false;
		}
		QFile f(fileName);
		QString errorReason(tr("file error"));
		if ( success && f.open(QIODevice::WriteOnly) ) {
			BigByteArray data;
			quint64 size = romSizeList->at(i).toULongLong();
			QString path, member, type;
			if ( checkSumDb()->getData(romSha1List->at(i), romCrcList->at(i), &size, &path, &member, &type) ) {
				switch ( m_fileTypes.indexOf(type) ) {
					case QMC2_COLLECTIONREBUILDER_FILETYPE_ZIP:
						success = readZipFileData(path, romCrcList->at(i), member, &data);
						break;
					case QMC2_COLLECTIONREBUILDER_FILETYPE_7Z:
						success = readSevenZipFileData(path, romCrcList->at(i), member, &data);
						break;
					case QMC2_COLLECTIONREBUILDER_FILETYPE_FILE:
						success = readFileData(path, &data);
						break;
					default:
						success = false;
						errorReason = tr("unknown file type '%1'").arg(type);
						break;
				}
				if ( success ) {
					emit log(tr("writing '%1' (size: %2)").arg(fileName).arg(ROMAlyzer::humanReadable(data.length())));
					quint64 bytesWritten = 0;
					while ( bytesWritten < (quint64)data.length() && !exitThread && success ) {
						quint64 bufferLength = QMC2_ZIP_BUFFER_SIZE;
						if ( bytesWritten + bufferLength > (quint64)data.length() )
							bufferLength = data.length() - bytesWritten;
						qint64 len = f.write(data.mid(bytesWritten, bufferLength));
						success = (len >= 0);
						if ( success ) {
							bytesWritten += len;
						} else
							log(tr("FATAL: failed writing '%1'").arg(fileName));
					}
				}
				f.close();
				reproducedDumps++;
			} else {
				f.close();
				f.remove();
			}
		} else {
			emit log(tr("FATAL: failed opening '%1' for writing"));
			success = false;
		}
		if ( !success ) {
			emit statusUpdated(setsProcessed, ++missingROMs, missingDisks);
			emit newMissing(id, tr("ROM"), romNameList->at(i), romSizeList->at(i), romCrcList->at(i), romSha1List->at(i), errorReason);
		}
		if ( ignoreErrors )
			success = true;
		else if ( !success ) {
			for (int j = i + 1; j < romNameList->count() && !exitThread; j++) {
				emit statusUpdated(setsProcessed, ++missingROMs, missingDisks);
				emit newMissing(id, tr("ROM"), romNameList->at(j), romSizeList->at(j), romCrcList->at(j), romSha1List->at(j), errorReason);
			}
		}
	}
	if ( reproducedDumps == 0 )
		d.rmdir(d.absolutePath());
	return success;
}

bool CollectionRebuilderThread::writeAllZipData(QString baseDir, QString id, QStringList *romNameList, QStringList *romSha1List, QStringList *romCrcList, QStringList *romSizeList, QStringList * /*diskNameList*/, QStringList * /*diskSha1List*/)
{
	QDir d(QDir::cleanPath(baseDir));
	if ( !d.exists() )
		if ( !d.mkdir(QDir::cleanPath(baseDir)) )
			return false;
	QString fileName(QDir::cleanPath(baseDir) + "/" + id + ".zip");
	if ( !createBackup(fileName) ) {
		emit log(tr("FATAL: backup creation failed"));
		return false;
	}
	QFile f(fileName);
	if ( f.exists() )
		if ( !f.remove() )
			return false;
	bool success = true;
	bool uniqueCRCs = rebuilderDialog()->romAlyzer()->checkBoxSetRewriterUniqueCRCs->isChecked();
	bool ignoreErrors = !rebuilderDialog()->romAlyzer()->checkBoxSetRewriterAbortOnError->isChecked();
	int zipLevel = rebuilderDialog()->romAlyzer()->spinBoxSetRewriterZipLevel->value();
	zipFile zip = zipOpen(fileName.toUtf8().constData(), APPEND_STATUS_CREATE);
	if ( zip ) {
		emit log(tr("creating new ZIP archive '%1'").arg(fileName));
		zip_fileinfo zipInfo;
		QDateTime cDT = QDateTime::currentDateTime();
		zipInfo.tmz_date.tm_sec = cDT.time().second();
		zipInfo.tmz_date.tm_min = cDT.time().minute();
		zipInfo.tmz_date.tm_hour = cDT.time().hour();
		zipInfo.tmz_date.tm_mday = cDT.date().day();
		zipInfo.tmz_date.tm_mon = cDT.date().month() - 1;
		zipInfo.tmz_date.tm_year = cDT.date().year();
		zipInfo.dosDate = zipInfo.internal_fa = zipInfo.external_fa = 0;
		QStringList storedCRCs;
		int reproducedDumps = 0;
		for (int i = 0; i < romNameList->count() && !exitThread && success; i++) {
			if ( uniqueCRCs && storedCRCs.contains(romCrcList->at(i)) ) {
				emit log(tr("skipping '%1'").arg(romNameList->at(i)) + " ("+ tr("a dump with CRC '%1' already exists").arg(romCrcList->at(i)) + ")");
				continue;
			}
			QString file(romNameList->at(i));
			BigByteArray data;
			quint64 size = romSizeList->at(i).toULongLong();
			QString path, member, type;
			QString errorReason(tr("file error"));
			if ( checkSumDb()->getData(romSha1List->at(i), romCrcList->at(i), &size, &path, &member, &type) ) {
				switch ( m_fileTypes.indexOf(type) ) {
					case QMC2_COLLECTIONREBUILDER_FILETYPE_ZIP:
						success = readZipFileData(path, romCrcList->at(i), member, &data);
						break;
					case QMC2_COLLECTIONREBUILDER_FILETYPE_7Z:
						success = readSevenZipFileData(path, romCrcList->at(i), member, &data);
						break;
					case QMC2_COLLECTIONREBUILDER_FILETYPE_FILE:
						success = readFileData(path, &data);
						break;
					default:
						success = false;
						errorReason = tr("unknown file type '%1'").arg(type);
						break;
				}
				if ( success && zipOpenNewFileInZip(zip, file.toUtf8().constData(), &zipInfo, file.toUtf8().constData(), file.length(), 0, 0, 0, Z_DEFLATED, zipLevel) == ZIP_OK ) {
					emit log(tr("writing '%1' to ZIP archive '%2' (uncompressed size: %3)").arg(file).arg(fileName).arg(ROMAlyzer::humanReadable(data.length())));
					quint64 bytesWritten = 0;
					while ( bytesWritten < (quint64)data.length() && !exitThread && success ) {
						quint64 bufferLength = QMC2_ZIP_BUFFER_SIZE;
						if ( bytesWritten + bufferLength > (quint64)data.length() )
							bufferLength = data.length() - bytesWritten;
						QByteArray writeBuffer(data.mid(bytesWritten, bufferLength));
						success = (zipWriteInFileInZip(zip, (const void *)writeBuffer.data(), bufferLength) == ZIP_OK);
						if ( success )
							bytesWritten += bufferLength;
						else {
							emit log(tr("FATAL: failed writing '%1' to ZIP archive '%2'").arg(file).arg(fileName));
							success = false;
						}
					}
					storedCRCs << romCrcList->at(i);
					zipCloseFileInZip(zip);
					if ( success )
						reproducedDumps++;
				}
			}
			if ( !success ) {
				emit statusUpdated(setsProcessed, ++missingROMs, missingDisks);
				emit newMissing(id, tr("ROM"), romNameList->at(i), romSizeList->at(i), romCrcList->at(i), romSha1List->at(i), errorReason);
			}
			if ( ignoreErrors )
				success = true;
			else if ( !success ) {
				for (int j = i + 1; j < romNameList->count() && !exitThread; j++) {
					emit statusUpdated(setsProcessed, ++missingROMs, missingDisks);
					emit newMissing(id, tr("ROM"), romNameList->at(j), romSizeList->at(j), romCrcList->at(j), romSha1List->at(j), errorReason);
				}
			}
		}
		if ( rebuilderDialog()->romAlyzer()->checkBoxSetRewriterAddZipComment->isChecked() )
			zipClose(zip, tr("Created by QMC2 v%1 (%2)").arg(XSTR(QMC2_VERSION)).arg(cDT.toString(Qt::SystemLocaleShortDate)).toUtf8().constData());
		else
			zipClose(zip, "");
		if ( reproducedDumps == 0 )
			f.remove();
		emit log(tr("done (creating new ZIP archive '%1')").arg(fileName));
	} else {
		emit log(tr("FATAL: failed creating ZIP archive '%1'").arg(fileName));
		success = false;
	}
	return success;
}

#if defined(QMC2_LIBARCHIVE_ENABLED)
bool CollectionRebuilderThread::writeAllArchiveData(QString baseDir, QString id, QStringList *romNameList, QStringList *romSha1List, QStringList *romCrcList, QStringList *romSizeList, QStringList * /*diskNameList*/, QStringList * /*diskSha1List*/)
{
	QDir d(QDir::cleanPath(baseDir));
	if ( !d.exists() )
		if ( !d.mkdir(QDir::cleanPath(baseDir)) )
			return false;
	QString fileName(QDir::cleanPath(baseDir) + "/" + id + ".zip");
	if ( !createBackup(fileName) ) {
		emit log(tr("FATAL: backup creation failed"));
		return false;
	}
	QFile f(fileName);
	if ( f.exists() )
		if ( !f.remove() )
			return false;
	bool success = true;
	bool uniqueCRCs = rebuilderDialog()->romAlyzer()->checkBoxSetRewriterUniqueCRCs->isChecked();
	bool ignoreErrors = !rebuilderDialog()->romAlyzer()->checkBoxSetRewriterAbortOnError->isChecked();
	ArchiveFile af(fileName, true, rebuilderDialog()->romAlyzer()->comboBoxSetRewriterLibArchiveDeflate->currentIndex() == 0);
	if ( af.open(QIODevice::WriteOnly) ) {
		emit log(tr("creating new ZIP archive '%1'").arg(fileName));
		QStringList storedCRCs;
		int reproducedDumps = 0;
		for (int i = 0; i < romNameList->count() && !exitThread && success; i++) {
			if ( uniqueCRCs && storedCRCs.contains(romCrcList->at(i)) ) {
				emit log(tr("skipping '%1'").arg(romNameList->at(i)) + " ("+ tr("a dump with CRC '%1' already exists").arg(romCrcList->at(i)) + ")");
				continue;
			}
			QString file(romNameList->at(i));
			BigByteArray data;
			quint64 size = romSizeList->at(i).toULongLong();
			QString path, member, type;
			QString errorReason(tr("file error"));
			if ( checkSumDb()->getData(romSha1List->at(i), romCrcList->at(i), &size, &path, &member, &type) ) {
				switch ( m_fileTypes.indexOf(type) ) {
					case QMC2_COLLECTIONREBUILDER_FILETYPE_ZIP:
						success = readZipFileData(path, romCrcList->at(i), member, &data);
						break;
					case QMC2_COLLECTIONREBUILDER_FILETYPE_7Z:
						success = readSevenZipFileData(path, romCrcList->at(i), member, &data);
						break;
					case QMC2_COLLECTIONREBUILDER_FILETYPE_FILE:
						success = readFileData(path, &data);
						break;
					default:
						success = false;
						errorReason = tr("unknown file type '%1'").arg(type);
						break;
				}
				if ( success && af.createEntry(file, data.size()) ) {
					emit log(tr("writing '%1' to ZIP archive '%2' (uncompressed size: %3)").arg(file).arg(fileName).arg(ROMAlyzer::humanReadable(data.length())));
					if ( !af.writeEntryDataBig(data) ) {
						emit log(tr("FATAL: failed writing '%1' to ZIP archive '%2'").arg(file).arg(fileName));
						success = false;
					}
					storedCRCs << romCrcList->at(i);
					af.closeEntry();
					if ( success )
						reproducedDumps++;
				}
			}
			if ( !success ) {
				emit statusUpdated(setsProcessed, ++missingROMs, missingDisks);
				emit newMissing(id, tr("ROM"), romNameList->at(i), romSizeList->at(i), romCrcList->at(i), romSha1List->at(i), errorReason);
			}
			if ( ignoreErrors )
				success = true;
			else if ( !success ) {
				for (int j = i + 1; j < romNameList->count() && !exitThread; j++) {
					emit statusUpdated(setsProcessed, ++missingROMs, missingDisks);
					emit newMissing(id, tr("ROM"), romNameList->at(j), romSizeList->at(j), romCrcList->at(j), romSha1List->at(j), errorReason);
				}
			}
		}
		af.close();
		if ( reproducedDumps == 0 )
			f.remove();
		emit log(tr("done (creating new ZIP archive '%1')").arg(fileName));
	} else {
		emit log(tr("FATAL: failed creating ZIP archive '%1'").arg(fileName));
		success = false;
	}
	return success;
}
#endif

bool CollectionRebuilderThread::readFileData(QString fileName, BigByteArray *data)
{
	QFile file(fileName);
	data->clear();
	if ( file.open(QIODevice::ReadOnly) ) {
  		char ioBuffer[QMC2_ROMALYZER_FILE_BUFFER_SIZE];
		int len = 0;
		emit log(tr("reading '%1' (size: %2)").arg(fileName).arg(ROMAlyzer::humanReadable(file.size())));
		while ( (len = file.read(ioBuffer, QMC2_FILE_BUFFER_SIZE)) > 0 && !exitThread )
			data->append(QByteArray((const char *)ioBuffer, len));
		file.close();
		return true;
	} else {
		emit log(tr("FATAL: failed reading '%1'").arg(fileName));
		return false;
	}
}

bool CollectionRebuilderThread::readSevenZipFileData(QString fileName, QString crc, QString member, BigByteArray *data)
{
	SevenZipFile sevenZipFile(fileName);
	if ( sevenZipFile.open() ) {
		int index = sevenZipFile.indexOfCrc(crc);
		if ( index >= 0 ) {
			if ( sevenZipFile.isCrcDuplicate(crc) ) {
				int nameIndex = sevenZipFile.indexOfName(member);
				if ( nameIndex >= 0 )
					index = nameIndex;
			}
			SevenZipMetaData metaData = sevenZipFile.entryList()[index];
			emit log(tr("reading '%1' from 7Z archive '%2' (uncompressed size: %3)").arg(metaData.name()).arg(fileName).arg(ROMAlyzer::humanReadable(metaData.size())));
			quint64 readLength = sevenZipFile.readBig(index, data); // can't be interrupted!
			if ( sevenZipFile.hasError() ) {
				emit log(tr("FATAL: failed reading '%1' from 7Z archive '%2'").arg(metaData.name()).arg(fileName));
				return false;
			}
			if ( readLength != metaData.size() ) {
				emit log(tr("FATAL: failed reading '%1' from 7Z archive '%2'").arg(metaData.name()).arg(fileName));
				return false;
			}
			return true;
		} else {
			emit log(tr("FATAL: failed reading from 7Z archive '%1'").arg(fileName));
			return false;
		}
	} else {
		emit log(tr("FATAL: failed reading from 7Z archive '%1'").arg(fileName));
		return false;
	}
}

bool CollectionRebuilderThread::readZipFileData(QString fileName, QString crc, QString member, BigByteArray *data)
{
	bool success = true;
	unzFile zipFile = unzOpen(fileName.toUtf8().constData());
	if ( zipFile ) {
  		char ioBuffer[QMC2_ZIP_BUFFER_SIZE];
		unz_file_info zipInfo;
		QMultiMap<uLong, QString> crcIdentMap;
		uLong ulCRC = crc.toULong(0, 16);
		do {
			if ( unzGetCurrentFileInfo(zipFile, &zipInfo, ioBuffer, QMC2_ROMALYZER_ZIP_BUFFER_SIZE, 0, 0, 0, 0) == UNZ_OK )
				crcIdentMap.insert(zipInfo.crc, QString((const char *)ioBuffer));
		} while ( unzGoToNextFile(zipFile) == UNZ_OK );
		unzGoToFirstFile(zipFile);
		if ( crcIdentMap.contains(ulCRC) ) {
			QString fn;
			QStringList names(crcIdentMap.values(ulCRC));
			if ( names.contains(member) )
				fn = member;
			else
				fn = names.at(0);
			if ( unzLocateFile(zipFile, fn.toUtf8().constData(), 0) == UNZ_OK ) {
				if ( unzOpenCurrentFile(zipFile) == UNZ_OK ) {
					if ( unzGetCurrentFileInfo(zipFile, &zipInfo, 0, 0, 0, 0, 0, 0) == UNZ_OK )
						emit log(tr("reading '%1' from ZIP archive '%2' (uncompressed size: %3)").arg(fn).arg(fileName).arg(ROMAlyzer::humanReadable(zipInfo.uncompressed_size)));
					else
						emit log(tr("reading '%1' from ZIP archive '%2' (uncompressed size: %3)").arg(fn).arg(fileName).arg(tr("unknown")));
					qint64 len;
					while ( (len = unzReadCurrentFile(zipFile, ioBuffer, QMC2_ROMALYZER_ZIP_BUFFER_SIZE)) > 0 && !exitThread )
						data->append(QByteArray((const char *)ioBuffer, len));
					unzCloseCurrentFile(zipFile);
				} else {
					emit log(tr("FATAL: failed reading '%1' from ZIP archive '%2'").arg(fn).arg(fileName));
					success = false;
				}
			} else {
				emit log(tr("FATAL: failed reading '%1' from ZIP archive '%2'").arg(fn).arg(fileName));
				success = false;
			}
		} else {
			emit log(tr("FATAL: CRC '%1' not found in ZIP archive '%2'").arg(crc).arg(fileName));
			success = false;
		}
		unzClose(zipFile);
	} else {
		emit log(tr("FATAL: failed reading from ZIP archive '%1'").arg(fileName));
		success = false;
	}
	return success;
}

bool CollectionRebuilderThread::hardlinkChds(QString baseDir, QString id, QStringList *diskNameList, QStringList *diskSha1List)
{
	QString targetPath(QDir::cleanPath(baseDir) + "/" + id);
	QDir d(targetPath);
	if ( !d.exists() )
		if ( !d.mkdir(QDir::cleanPath(targetPath)) )
			return false;
	bool success = true;
	QString errorReason = tr("file error");
	int reproducedDumps = 0;
	for (int i = 0; i < diskNameList->count() && !exitThread && success; i++) {
		QString fileName(QDir::cleanPath(targetPath) + "/" + diskNameList->at(i) + ".chd");
		quint64 size = 0;
		QString path, member, type;
		if ( checkSumDb()->getData(diskSha1List->at(i), QString(), &size, &path, &member, &type) ) {
			if ( m_fileTypes.indexOf(type) == QMC2_COLLECTIONREBUILDER_FILETYPE_CHD ) {
				if ( !createBackup(fileName) ) {
					emit log(tr("FATAL: backup creation failed"));
					return false;
				}
				if ( !sameFileSystem(path, fileName) ) {
					emit log(tr("WARNING: '%1' and '%2' are NOT on the same file system, hard-linking will not work").arg(path).arg(fileName) + " - " + tr("falling back to copy mode"));
					emit log(tr("copying CHD file '%1' to '%2'").arg(path).arg(fileName));
					QFile targetChd(fileName);
					if ( targetChd.exists() )
						targetChd.remove();
					QFile sourceChd(path);
					if ( sourceChd.open(QIODevice::ReadOnly) ) {
						if ( targetChd.open(QIODevice::WriteOnly) ) {
							char ioBuffer[QMC2_ROMALYZER_FILE_BUFFER_SIZE];
							int count = 0;
							int len = 0;
							while ( success && (len = sourceChd.read(ioBuffer, QMC2_ROMALYZER_FILE_BUFFER_SIZE)) > 0 ) {
								if ( count++ % QMC2_COPY_IO_RESPONSE == 0 )
									qApp->processEvents();
								if ( targetChd.write(ioBuffer, len) != len ) {
									emit log(tr("FATAL: I/O error while writing to '%1'").arg(fileName));
									success = false;
								}
							}
						} else
							success = false;
					} else
						success = false;
				} else {
					emit log(tr("hard-linking CHD file '%1' to '%2'").arg(path).arg(fileName));
					QFile f(fileName);
					if ( f.exists() )
						f.remove();
#if defined(QMC2_OS_WIN)
					success = CreateHardLink((LPCTSTR)fileName.utf16(), (LPCTSTR)path.utf16(), NULL);
#else
					success = ::link(path.toUtf8().constData(), fileName.toUtf8().constData()) == 0;
#endif
				}
				if ( success )
					reproducedDumps++;
				else
					errorReason = tr("failed hard-linking '%1' to '%2'").arg(path).arg(fileName);
			} else {
				success = false;
				errorReason = tr("invalid file type '%1'").arg(type);
				break;
			}
		}
	}
	if ( reproducedDumps == 0 )
		d.rmdir(d.absolutePath());
	return success;
}

bool CollectionRebuilderThread::symlinkChds(QString baseDir, QString id, QStringList *diskNameList, QStringList *diskSha1List)
{
	QString targetPath(QDir::cleanPath(baseDir) + "/" + id);
	QDir d(targetPath);
	if ( !d.exists() )
		if ( !d.mkdir(QDir::cleanPath(targetPath)) )
			return false;
	bool success = true;
	QString errorReason = tr("file error");
	int reproducedDumps = 0;
	for (int i = 0; i < diskNameList->count() && !exitThread && success; i++) {
		QString fileName(QDir::cleanPath(targetPath) + "/" + diskNameList->at(i) + ".chd");
		quint64 size = 0;
		QString path, member, type;
		if ( checkSumDb()->getData(diskSha1List->at(i), QString(), &size, &path, &member, &type) ) {
			if ( m_fileTypes.indexOf(type) == QMC2_COLLECTIONREBUILDER_FILETYPE_CHD ) {
				if ( !createBackup(fileName) ) {
					emit log(tr("FATAL: backup creation failed"));
					return false;
				}
				emit log(tr("sym-linking CHD file '%1' to '%2'").arg(path).arg(fileName));
				QFile f(fileName);
				if ( f.exists() )
					f.remove();
				QFile sourceChd(path);
				success = sourceChd.link(fileName);
				if ( success )
					reproducedDumps++;
				else
					errorReason = tr("failed sym-linking '%1' to '%2'").arg(path).arg(fileName);
			} else {
				success = false;
				errorReason = tr("invalid file type '%1'").arg(type);
				break;
			}
		}
	}
	if ( reproducedDumps == 0 )
		d.rmdir(d.absolutePath());
	return success;
}

bool CollectionRebuilderThread::copyChds(QString baseDir, QString id, QStringList *diskNameList, QStringList *diskSha1List)
{
	QString targetPath(QDir::cleanPath(baseDir) + "/" + id);
	QDir d(targetPath);
	if ( !d.exists() )
		if ( !d.mkdir(QDir::cleanPath(targetPath)) )
			return false;
	bool success = true;
	QString errorReason = tr("file error");
	int reproducedDumps = 0;
	for (int i = 0; i < diskNameList->count() && !exitThread && success; i++) {
		QString fileName(QDir::cleanPath(targetPath) + "/" + diskNameList->at(i) + ".chd");
		quint64 size = 0;
		QString path, member, type;
		if ( checkSumDb()->getData(diskSha1List->at(i), QString(), &size, &path, &member, &type) ) {
			if ( m_fileTypes.indexOf(type) == QMC2_COLLECTIONREBUILDER_FILETYPE_CHD ) {
				if ( !createBackup(fileName) ) {
					emit log(tr("FATAL: backup creation failed"));
					return false;
				}
				emit log(tr("copying CHD file '%1' to '%2'").arg(path).arg(fileName));
				QFile targetChd(fileName);
				if ( targetChd.exists() )
					targetChd.remove();
				QFile sourceChd(path);
				if ( sourceChd.open(QIODevice::ReadOnly) ) {
					if ( targetChd.open(QIODevice::WriteOnly) ) {
						char ioBuffer[QMC2_ROMALYZER_FILE_BUFFER_SIZE];
						int count = 0;
						int len = 0;
						while ( success && (len = sourceChd.read(ioBuffer, QMC2_ROMALYZER_FILE_BUFFER_SIZE)) > 0 ) {
							if ( count++ % QMC2_COPY_IO_RESPONSE == 0 )
								qApp->processEvents();
							if ( targetChd.write(ioBuffer, len) != len ) {
								emit log(tr("FATAL: I/O error while writing to '%1'").arg(fileName));
								success = false;
							}
						}
					} else
						success = false;
				} else
					success = false;
				if ( success )
					reproducedDumps++;
				else
					errorReason = tr("failed copying '%1' to '%2'").arg(path).arg(fileName);
			} else {
				success = false;
				errorReason = tr("invalid file type '%1'").arg(type);
				break;
			}
		}
	}
	if ( reproducedDumps == 0 )
		d.rmdir(d.absolutePath());
	return success;
}

bool CollectionRebuilderThread::moveChds(QString baseDir, QString id, QStringList *diskNameList, QStringList *diskSha1List)
{
	QString targetPath(QDir::cleanPath(baseDir) + "/" + id);
	QDir d(targetPath);
	if ( !d.exists() )
		if ( !d.mkdir(QDir::cleanPath(targetPath)) )
			return false;
	bool success = true;
	QString errorReason = tr("file error");
	int reproducedDumps = 0;
	for (int i = 0; i < diskNameList->count() && !exitThread && success; i++) {
		QString fileName(QDir::cleanPath(targetPath) + "/" + diskNameList->at(i) + ".chd");
		quint64 size = 0;
		QString path, member, type;
		if ( checkSumDb()->getData(diskSha1List->at(i), QString(), &size, &path, &member, &type) ) {
			if ( path == fileName )
				continue;
			if ( m_fileTypes.indexOf(type) == QMC2_COLLECTIONREBUILDER_FILETYPE_CHD ) {
				if ( !createBackup(fileName) ) {
					emit log(tr("FATAL: backup creation failed"));
					return false;
				}
				emit log(tr("moving CHD file '%1' to '%2'").arg(path).arg(fileName));
				QFile targetChd(fileName);
				if ( targetChd.exists() )
					targetChd.remove();
				QFile sourceChd(path);
				if ( sameFileSystem(path, fileName) )
					success = QFile::rename(path, fileName);
				else {
					if ( sourceChd.open(QIODevice::ReadOnly) ) {
						if ( targetChd.open(QIODevice::WriteOnly) ) {
							char ioBuffer[QMC2_ROMALYZER_FILE_BUFFER_SIZE];
							int count = 0;
							int len = 0;
							while ( success && (len = sourceChd.read(ioBuffer, QMC2_ROMALYZER_FILE_BUFFER_SIZE)) > 0 ) {
								if ( count++ % QMC2_COPY_IO_RESPONSE == 0 )
									qApp->processEvents();
								if ( targetChd.write(ioBuffer, len) != len ) {
									emit log(tr("FATAL: I/O error while writing to '%1'").arg(fileName));
									success = false;
								}
							}
						} else
							success = false;
					} else
						success = false;
				}
				if ( success ) {
					reproducedDumps++;
					sourceChd.remove();
				} else
					errorReason = tr("failed moving '%1' to '%2'").arg(path).arg(fileName);
			} else {
				success = false;
				errorReason = tr("invalid file type '%1'").arg(type);
				break;
			}
		}
	}
	if ( reproducedDumps == 0 )
		d.rmdir(d.absolutePath());
	return success;
}

bool CollectionRebuilderThread::sameFileSystem(QString path1, QString path2)
{
	// - path1 and path2 are expected to be fully-qualified file names
	// - the files may or may not exist, however, the folders they belong to MUST exist for this to work
	//   (otherwise stat() returns ENOENT) 
	struct stat stat1, stat2;
	stat(QFileInfo(path1).absoluteDir().absolutePath().toUtf8().constData(), &stat1);
	stat(QFileInfo(path2).absoluteDir().absolutePath().toUtf8().constData(), &stat2);
	return stat1.st_dev == stat2.st_dev;
}

bool CollectionRebuilderThread::createBackup(QString filePath)
{
	if ( !rebuilderDialog()->romAlyzer()->checkBoxCreateBackups->isChecked() || rebuilderDialog()->romAlyzer()->lineEditBackupFolder->text().isEmpty() )
		return true;
	QFile sourceFile(filePath);
	if ( !sourceFile.exists() )
		return true;
	QDir backupDir(rebuilderDialog()->romAlyzer()->lineEditBackupFolder->text());
	QFileInfo backupDirInfo(backupDir.absolutePath());
	if ( backupDirInfo.exists() ) {
		if ( backupDirInfo.isWritable() ) {
#if defined(QMC2_OS_WIN)
			QString filePathCopy = filePath;
			QString destinationPath = QDir::cleanPath(QString(backupDir.absolutePath() + "/" + filePathCopy.replace(":", "")));
#else
			QString destinationPath = QDir::cleanPath(backupDir.absolutePath() + "/" + filePath);
#endif
			QFileInfo destinationPathInfo(destinationPath);
			if ( !destinationPathInfo.dir().exists() ) {
				if ( !backupDir.mkpath(destinationPathInfo.dir().absolutePath()) ) {
					emit log(tr("backup") + ": " + tr("FATAL: target path '%1' cannot be created").arg(destinationPathInfo.dir().absolutePath()));
					return false;
				}
			}
			if ( !sourceFile.open(QIODevice::ReadOnly) ) {
				emit log(tr("backup") + ": " + tr("FATAL: source file '%1' cannot be opened for reading").arg(filePath));
				return false;
			}
			QFile destinationFile(destinationPath);
			if ( destinationFile.open(QIODevice::WriteOnly) ) {
				emit log(tr("backup") + ": " + tr("creating backup copy of '%1' as '%2'").arg(filePath).arg(destinationPath));
				char ioBuffer[QMC2_ROMALYZER_FILE_BUFFER_SIZE];
				int count = 0;
				int len = 0;
				bool success = true;
				while ( success && (len = sourceFile.read(ioBuffer, QMC2_ROMALYZER_FILE_BUFFER_SIZE)) > 0 ) {
					if ( count++ % QMC2_BACKUP_IO_RESPONSE == 0 )
						qApp->processEvents();
					if ( destinationFile.write(ioBuffer, len) != len ) {
						emit log(tr("backup") + ": " + tr("FATAL: I/O error while writing to '%1'").arg(destinationPath));
						success = false;
					}
				}
				sourceFile.close();
				destinationFile.close();
				if ( success ) {
					emit log(tr("backup") + ": " + tr("done (creating backup copy of '%1' as '%2')").arg(filePath).arg(destinationPath));
					return true;
				} else
					return false;
			} else {
				emit log(tr("backup") + ": " + tr("FATAL: destination file '%1' cannot be opened for writing").arg(destinationPath));
				sourceFile.close();
				return false;
			}
		} else {
			emit log(tr("backup") + ": " + tr("FATAL: backup folder '%1' isn't writable").arg(backupDir.absolutePath()));
			return false;
		}
	} else {
		emit log(tr("backup") + ": " + tr("FATAL: backup folder '%1' doesn't exist").arg(backupDir.absolutePath()));
		return false;
	}
}

void CollectionRebuilderThread::setFilterExpression(QString expression, int syntax, int type, bool exact)
{
	doFilter = !expression.isEmpty();
	exactMatch = exact;
	isIncludeFilter = (type == 0);
	QRegExp::PatternSyntax ps;
	switch ( syntax ) {
		case 1:
			ps = QRegExp::RegExp2;
			break;
		case 2:
			ps = QRegExp::Wildcard;
			break;
		case 3:
			ps = QRegExp::WildcardUnix;
			break;
		case 4:
			ps = QRegExp::FixedString;
			break;
		case 5:
			ps = QRegExp::W3CXmlSchema11;
			break;
		case 0:
		default:
			ps = QRegExp::RegExp;
			break;
	}
	filterRx = QRegExp(expression, Qt::CaseSensitive, ps);
	if ( doFilter && !filterRx.isValid() ) {
		emit log(tr("WARNING: invalid filter expression '%1' ignored").arg(expression));
		doFilter = false;
	}
}

void CollectionRebuilderThread::setFilterExpressionSoftware(QString expression, int syntax, int type, bool exact)
{
	doFilterSoftware = !expression.isEmpty();
	exactMatchSoftware = exact;
	isIncludeFilterSoftware = (type == 0);
	QRegExp::PatternSyntax ps;
	switch ( syntax ) {
		case 1:
			ps = QRegExp::RegExp2;
			break;
		case 2:
			ps = QRegExp::Wildcard;
			break;
		case 3:
			ps = QRegExp::WildcardUnix;
			break;
		case 4:
			ps = QRegExp::FixedString;
			break;
		case 5:
			ps = QRegExp::W3CXmlSchema11;
			break;
		case 0:
		default:
			ps = QRegExp::RegExp;
			break;
	}
	filterRxSoftware = QRegExp(expression, Qt::CaseSensitive, ps);
	if ( doFilterSoftware && !filterRxSoftware.isValid() ) {
		emit log(tr("WARNING: invalid filter expression '%1' ignored").arg(expression));
		doFilterSoftware = false;
	}
}

void CollectionRebuilderThread::pause()
{
	pauseRequested = true;
}

void CollectionRebuilderThread::resume()
{
	isPaused = false;
}

bool CollectionRebuilderThread::evaluateFilters(QString &setKey)
{
	static QString list, set;

	if ( setKey.isEmpty() )
		return false;

	switch ( rebuilderDialog()->romAlyzer()->mode() ) {
		case QMC2_ROMALYZER_MODE_SOFTWARE: {
				QStringList setKeyTokens(setKey.split(':', QString::SkipEmptyParts));
				if ( setKeyTokens.count() < 2 )
					return false;
				list = setKeyTokens.at(0);
				if ( doFilterSoftware ) {
					if ( isIncludeFilterSoftware ) {
						if ( exactMatchSoftware ) {
							if ( !filterRxSoftware.exactMatch(list) )
								return false;
						} else if ( filterRxSoftware.indexIn(list) < 0 )
							return false;
					} else {
						if ( exactMatchSoftware ) {
							if ( filterRxSoftware.exactMatch(list) )
								return false;
						} else if ( filterRxSoftware.indexIn(list) >= 0 )
							return false;
					}
				}
				set = setKeyTokens.at(1);
				break;
			}
		case QMC2_ROMALYZER_MODE_SYSTEM:
		default:
			set = setKey;
			if ( doFilterState ) {
				switch ( qmc2MachineList->romState(set) ) {
					case 'C':
						if ( !includeStateC )
							return false;
						break;
					case 'M':
						if ( !includeStateM )
							return false;
						break;
					case 'I':
						if ( !includeStateI )
							return false;
						break;
					case 'N':
						if ( !includeStateN )
							return false;
						break;
					case 'U':
					default:
						if ( !includeStateU )
							return false;
						break;
				}
			}
			break;
	}
	if ( doFilter ) {
		if ( isIncludeFilter ) {
			if ( exactMatch ) {
				if ( !filterRx.exactMatch(set) )
					return false;
			} else if ( filterRx.indexIn(set) < 0 )
				return false;
		} else {
			if ( exactMatch ) {
				if ( filterRx.exactMatch(set) )
					return false;
			} else if ( filterRx.indexIn(set) >= 0 )
				return false;
		}
	}
	return true;
}

bool CollectionRebuilderThread::checkSumExists(QString sha1, QString crc, quint64 size)
{
	if ( useHashCache ) {
		if ( sha1.isEmpty() ) {
			if ( m_hashCache.contains(QString("-%1-%2").arg(crc).arg(size)) )
				return true;
			else { // rare case so shouldn't hurt
				QStringList uniqueKeys(m_hashCache.uniqueKeys());
				return uniqueKeys.indexOf(QRegExp(QString(".*-%1-%2").arg(crc).arg(size))) >= 0;
			}
		} else {
			if ( m_hashCache.contains(QString("%1-%2-%3").arg(sha1).arg(crc).arg(size)) )
				return true;
			else
				return m_hashCache.contains(QString("-%1-%2").arg(crc).arg(size));
		}
	} else
		return checkSumDb()->exists(sha1, crc, size);
}

void CollectionRebuilderThread::updateHashCache()
{
	if ( checkSumDb()->scanTime() > m_hashCacheUpdateTime ) {
		qint64 row = checkSumDb()->nextRowId(true);
		emit progressTextChanged(tr("Preparing"));
		emit progressRangeChanged(0, checkSumDb()->checkSumRowCount() - 1);
		emit progressChanged(0);
		int count = 0;
		emit log(tr("updating hash cache"));
		m_hashCache.clear();
		while ( row > 0 && !exitThread && !stopRebuilding ) {
			emit progressChanged(count++);
			QString key;
			if ( !checkSumDb()->pathOfRow(row, &key, true).isEmpty() )
				m_hashCache.insert(key, true);
			row = checkSumDb()->nextRowId();
			if ( exitThread || stopRebuilding )
				break;
		}
		if ( exitThread || stopRebuilding ) {
			m_hashCacheUpdateTime = 0;
			m_hashCache.clear();
			emit log(tr("hash cache update interrupted"));
		} else {
			emit log(tr("hash cache updated") + " - " + tr("%n hash(es) loaded", "", m_hashCache.count()));
			m_hashCacheUpdateTime = QDateTime::currentDateTime().toTime_t();
			emit progressRangeChanged(m_xmlIndex, m_xmlIndexCount);
			emit progressChanged(m_xmlIndex);
		}
		emit progressChanged(0);
	}
}

void CollectionRebuilderThread::run()
{
	emit log(tr("rebuilder thread started"));
	while ( !exitThread ) {
		emit log(tr("waiting for work"));
		mutex.lock();
		isWaiting = true;
		isActive = isPaused = stopRebuilding = false;
		waitCondition.wait(&mutex);
		isActive = true;
		isWaiting = false;
		mutex.unlock();
		if ( !exitThread && !stopRebuilding ) {
			setsProcessed = missingROMs = missingDisks = 0;
			setSetEntityPattern("<" + rebuilderDialog()->lineEditSetEntity->text() + " name=\"");
			setRomEntityPattern("<" + rebuilderDialog()->lineEditRomEntity->text() + " name=\"");
			setDiskEntityPattern("<" + rebuilderDialog()->lineEditDiskEntity->text() + " name=\"");
			setSetEntityStartPattern("<" + rebuilderDialog()->lineEditSetEntity->text() + " name=\"");
			setMerge(!rebuilderDialog()->romAlyzer()->checkBoxSetRewriterSelfContainedSets->isChecked());
			if ( dryRun )
				emit log(tr("dry run started"));
			else
				emit log(tr("rebuilding started"));
			emit statusUpdated(0, 0, 0);
			emit rebuildStarted();
			QTime rebuildTimer, elapsedTime(0, 0, 0, 0);
			rebuildTimer.start();
			if ( useHashCache )
				updateHashCache();
			if ( dryRun )
				emit progressTextChanged(tr("Analyzing"));
			else
				emit progressTextChanged(tr("Rebuilding"));
			if ( checkpoint() < 0 )
				m_xmlIndex = m_xmlIndexCount = -1;
			QString setKey;
			QStringList romNameList, romSha1List, romCrcList, romSizeList, diskNameList, diskSha1List, diskSizeList;
			while ( !exitThread && !stopRebuilding && nextId(&setKey, &romNameList, &romSha1List, &romCrcList, &romSizeList, &diskNameList, &diskSha1List, &diskSizeList) ) {
				bool pauseMessageLogged = false;
				while ( (pauseRequested || isPaused) && !exitThread && !stopRebuilding ) {
					if ( !pauseMessageLogged ) {
						pauseMessageLogged = true;
						isPaused = true;
						pauseRequested = false;
						emit progressTextChanged(tr("Paused"));
						emit rebuildPaused();
						if ( dryRun )
							emit log(tr("dry run paused"));
						else
							emit log(tr("rebuilding paused"));
					}
					QTest::qWait(100);
				}
				if ( pauseMessageLogged && !exitThread && !stopRebuilding ) {
					isPaused = false;
					if ( dryRun )
						emit progressTextChanged(tr("Analyzing"));
					else
						emit progressTextChanged(tr("Rebuilding"));
					emit rebuildResumed();
					if ( dryRun )
						emit log(tr("dry run resumed"));
					else
						emit log(tr("rebuilding resumed"));
				}
				if ( setKey.isEmpty() )
					continue;
				if ( !exitThread && !stopRebuilding && (!romNameList.isEmpty() || !diskNameList.isEmpty()) ) {
					if ( !dryRun )
						emit log(tr("set rebuilding started for '%1'").arg(setKey));
					for (int i = 0; i < romNameList.count(); i++) {
						bool dbStatusGood = checkSumExists(romSha1List[i], romCrcList[i], romSizeList[i].toULongLong());
						if ( !dryRun )
							emit log(tr("required ROM") + ": " + tr("name = '%1', crc = '%2', sha1 = '%3', database status = '%4'").arg(romNameList[i]).arg(romCrcList[i]).arg(romSha1List[i]).arg(dbStatusGood ? tr("available") : tr("not available")));
						if ( !dbStatusGood ) {
							missingROMs++;
							emit newMissing(setKey, tr("ROM"), romNameList[i], romSizeList[i], romCrcList[i], romSha1List[i], tr("check-sum not available in database"));
						}
					}
					for (int i = 0; i < diskNameList.count(); i++) {
						bool dbStatusGood = checkSumExists(diskSha1List[i], useHashCache ? "0" : QString());
						if ( !dryRun )
							emit log(tr("required disk") + ": " + tr("name = '%1', sha1 = '%2', database status = '%3'").arg(diskNameList[i]).arg(diskSha1List[i]).arg(dbStatusGood ? tr("available") : tr("not available")));
						if ( !dbStatusGood ) {
							missingDisks++;
							emit newMissing(setKey, tr("DISK"), diskNameList[i], diskSizeList[i], QString(), diskSha1List[i], tr("check-sum not available in database"));
						}
					}
					emit statusUpdated(setsProcessed, missingROMs, missingDisks);
					if ( !dryRun ) {
						bool rewriteOkay = true;
						if ( !romNameList.isEmpty() || !diskNameList.isEmpty() )
							rewriteOkay = rewriteSet(&setKey, &romNameList, &romSha1List, &romCrcList, &romSizeList, &diskNameList, &diskSha1List);
						if ( rewriteOkay )
							emit log(tr("set rebuilding finished for '%1'").arg(setKey));
						else
							emit log(tr("set rebuilding failed for '%1'").arg(setKey));
					}
					emit statusUpdated(++setsProcessed, missingROMs, missingDisks);
					setCheckpoint(m_xmlIndex, rebuilderDialog()->comboBoxXmlSource->currentIndex());
					if ( !dryRun ) {
						QTest::qWait(1);
						yieldCurrentThread();
					}
				}
			}
			if ( rebuilderDialog()->defaultEmulator() ) {
				if ( xmlDb() )
					xmlDb()->clearIdAtIndexCache();
				if ( swlDb() )
					swlDb()->clearIdAtIndexCache();
			}
			elapsedTime = elapsedTime.addMSecs(rebuildTimer.elapsed());
			if ( dryRun )
				emit log(tr("dry run finished - total analysis time = %1, sets processed = %2, missing ROMs = %3, missing disks = %4").arg(elapsedTime.toString("hh:mm:ss.zzz")).arg(setsProcessed).arg(missingROMs).arg(missingDisks));
			else
				emit log(tr("rebuilding finished - total rebuild time = %1, sets processed = %2, missing ROMs = %3, missing disks = %4").arg(elapsedTime.toString("hh:mm:ss.zzz")).arg(setsProcessed).arg(missingROMs).arg(missingDisks));
			emit progressRangeChanged(0, 100);
			emit progressChanged(0);
			emit progressTextChanged(tr("Idle"));
			if ( m_xmlFile.isOpen() )
				m_xmlFile.close();
			emit rebuildFinished();
		}
	}
	emit log(tr("rebuilder thread ended"));
}
