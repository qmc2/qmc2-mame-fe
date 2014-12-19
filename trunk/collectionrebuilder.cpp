#include <QApplication>
#include <QTest>
#include <QFont>
#include <QFontInfo>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMap>
#include <QDateTime>
#include <QMessageBox>
#include <QScrollBar>

#include "collectionrebuilder.h"
#include "settings.h"
#include "options.h"
#include "unzip.h"
#include "zip.h"
#include "sevenzipfile.h"
#include "macros.h"

extern Settings *qmc2Config;
extern Options *qmc2Options;

CollectionRebuilder::CollectionRebuilder(ROMAlyzer *myROMAlyzer, QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
	m_romAlyzer = myROMAlyzer;
	switch ( romAlyzer()->mode() ) {
		case QMC2_ROMALYZER_MODE_SOFTWARE:
			setWindowTitle(tr("Software Collection Rebuilder"));
			m_settingsKey = "SoftwareCollectionRebuilder";
			m_defaultSetEntity = "software";
			m_defaultRomEntity = "rom";
			m_defaultDiskEntity = "disk";
			break;
		case QMC2_ROMALYZER_MODE_SYSTEM:
		default:
			setWindowTitle(tr("ROM Collection Rebuilder"));
			m_settingsKey = "CollectionRebuilder";
#if defined(QMC2_EMUTYPE_MESS)
			m_defaultSetEntity = "machine";
#else
			m_defaultSetEntity = "game";
#endif
			m_defaultRomEntity = "rom";
			m_defaultDiskEntity = "disk";
			break;
	}
	pushButtonPauseResume->setVisible(false);
	QFont logFont;
	logFont.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/LogFont").toString());
	plainTextEditLog->setFont(logFont);
	spinBoxMaxLogSize->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/MaxLogSize", 10000).toInt());
	comboBoxFilterSyntax->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterSyntax", 0).toInt());
	comboBoxFilterType->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterType", 0).toInt());
	checkBoxFilterExpression->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/UseFilterExpression", false).toBool());
	lineEditFilterExpression->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterExpression", QString()).toString());
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
	connect(rebuilderThread(), SIGNAL(statusUpdated(int, int, int)), this, SLOT(rebuilderThread_statusUpdated(int, int, int)));

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
	QList<qint64> checkpoints;
	foreach (QString cp, checkpointList)
		checkpoints << cp.toLongLong();
	int index = 1;
	if ( xmlSources.count() > 0 && setEntities.count() == xmlSources.count() && romEntities.count() == xmlSources.count() && diskEntities.count() == xmlSources.count() && checkpointList.count() == xmlSources.count() ) {
		for (int i = 0; i < xmlSources.count(); i++) {
			QString xmlSource = xmlSources[i];
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
				i--;
			}
		}
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSources", xmlSources);
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetEntities", setEntities);
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/RomEntities", romEntities);
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/DiskEntities", diskEntities);
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceCheckpoints", checkpointList);
	} else {
		qmc2Config->remove(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSources");
		qmc2Config->remove(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetEntities");
		qmc2Config->remove(QMC2_FRONTEND_PREFIX + m_settingsKey + "/RomEntities");
		qmc2Config->remove(QMC2_FRONTEND_PREFIX + m_settingsKey + "/DiskEntities");
		qmc2Config->remove(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceCheckpoints");
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
	toolButtonClearFilterExpression->setIconSize(iconSize);
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
		if ( comboBoxXmlSource->itemData(comboBoxXmlSource->currentIndex()).toBool() ) {
			if ( QMessageBox::question(this, tr("Confirm checkpoint restart"), tr("Restart from stored checkpoint?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes ) {
				qint64 cp = 0;
				int index = comboBoxXmlSource->currentIndex();
				if ( index == 0 )
					cp = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/Checkpoint", -1).toLongLong();
				else {
					index -= 1;
					QStringList checkpointList = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceCheckpoints", QStringList()).toStringList();
					if ( index >= 0 && index < checkpointList.count() )
						cp = checkpointList[index].toLongLong();
					else
						cp = -1;
				}
				if ( cp >= 0 )
					rebuilderThread()->checkpointRestart(cp);
				else
					rebuilderThread()->checkpointRestart(-1);
			} else
				rebuilderThread()->setCheckpoint(-1, comboBoxXmlSource->currentIndex());
		} else
			rebuilderThread()->setCheckpoint(-1, comboBoxXmlSource->currentIndex());
		if ( checkBoxFilterExpression->isChecked() && !lineEditFilterExpression->text().isEmpty() )
			rebuilderThread()->setFilterExpression(lineEditFilterExpression->text(), comboBoxFilterSyntax->currentIndex(), comboBoxFilterType->currentIndex());
		else
			rebuilderThread()->clearFilterExpression();
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

void CollectionRebuilder::on_comboBoxXmlSource_currentIndexChanged(int index)
{
	static int lastIndex = -1;

	if ( index == 0 ) {
		if ( lastIndex >= 0 ) {
			QStringList setEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetEntities", QStringList()).toStringList();
			QStringList romEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/RomEntities", QStringList()).toStringList();
			QStringList diskEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/DiskEntities", QStringList()).toStringList();
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
		QTimer::singleShot(0, this, SLOT(scrollToEnd()));
		toolButtonRemoveXmlSource->setVisible(false);
		lastIndex = -1;
	} else if ( index == comboBoxXmlSource->count() - 1 ) {
		QString xmlSource = QFileDialog::getOpenFileName(this, tr("Choose source XML file"), QString(), tr("Data and XML files (*.dat *.xml)") + ";;" + tr("Data files (*.dat)") + ";;" + tr("XML files (*.xml)") + ";;" + tr("All files (*)"), 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
		if ( !xmlSource.isNull() ) {
			int foundAtIndex = comboBoxXmlSource->findText(xmlSource);
			if ( foundAtIndex < 0 ) {
				comboBoxXmlSource->blockSignals(true);
				QStringList xmlSources = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSources", QStringList()).toStringList();
				xmlSources << xmlSource;
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSources", xmlSources);
				QStringList setEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetEntities", QStringList()).toStringList();
				setEntities << m_defaultSetEntity;
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetEntities", setEntities);
				lineEditSetEntity->setText(m_defaultSetEntity);
				QStringList romEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/RomEntities", QStringList()).toStringList();
				romEntities << m_defaultRomEntity;
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/RomEntities", romEntities);
				lineEditRomEntity->setText(m_defaultRomEntity);
				QStringList diskEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/DiskEntities", QStringList()).toStringList();
				diskEntities << m_defaultDiskEntity;
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/DiskEntities", diskEntities);
				lineEditDiskEntity->setText(m_defaultDiskEntity);
				QStringList checkpointList = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceCheckpoints", QStringList()).toStringList();
				checkpointList << "-1";
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceCheckpoints", checkpointList);
				int insertIndex = comboBoxXmlSource->count() - 2;
				lastIndex = insertIndex - 1;
				comboBoxXmlSource->insertItem(insertIndex, xmlSource);
				comboBoxXmlSource->setItemIcon(insertIndex, m_iconNoCheckpoint);
				comboBoxXmlSource->setItemData(insertIndex, false);
				comboBoxXmlSource->setCurrentIndex(insertIndex);
				comboBoxXmlSource->blockSignals(false);
				frameEntities->setVisible(true);
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
			QStringList setEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetEntities", QStringList()).toStringList();
			QStringList romEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/RomEntities", QStringList()).toStringList();
			QStringList diskEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/DiskEntities", QStringList()).toStringList();
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
			toolButtonRemoveXmlSource->setVisible(true);
			QTimer::singleShot(0, this, SLOT(scrollToEnd()));
		}
	}
}

void CollectionRebuilder::on_toolButtonRemoveXmlSource_clicked()
{
	int index = comboBoxXmlSource->currentIndex() - 1;
	comboBoxXmlSource->setCurrentIndex(0);
	QStringList xmlSources = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSources", QStringList()).toStringList();
	xmlSources.removeAt(index);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSources", xmlSources);
	QStringList setEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetEntities", QStringList()).toStringList();
	setEntities.removeAt(index);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SetEntities", setEntities);
	QStringList romEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/RomEntities", QStringList()).toStringList();
	romEntities.removeAt(index);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/RomEntities", romEntities);
	QStringList diskEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/DiskEntities", QStringList()).toStringList();
	diskEntities.removeAt(index);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/DiskEntities", diskEntities);
	QStringList checkpointList = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceCheckpoints", QStringList()).toStringList();
	checkpointList.removeAt(index);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceCheckpoints", checkpointList);
	comboBoxXmlSource->blockSignals(true);
	comboBoxXmlSource->removeItem(index + 1);
	comboBoxXmlSource->blockSignals(false);
}

void CollectionRebuilder::rebuilderThread_rebuildStarted()
{
	pushButtonStartStop->setIcon(QIcon(QString::fromUtf8(":/data/img/halt.png")));
	pushButtonStartStop->setText(tr("Stop rebuilding"));
	pushButtonPauseResume->setText(tr("Pause"));
	pushButtonPauseResume->show();
	pushButtonStartStop->setEnabled(true);
	pushButtonPauseResume->setEnabled(true);
	comboBoxXmlSource->setEnabled(false);
	comboBoxXmlSource->setItemIcon(comboBoxXmlSource->currentIndex(), m_iconCheckpoint);
	comboBoxXmlSource->setItemData(comboBoxXmlSource->currentIndex(), true);
	labelXmlSource->setEnabled(false);
	toolButtonRemoveXmlSource->setEnabled(false);
	checkBoxFilterExpression->setEnabled(false);
	comboBoxFilterSyntax->setEnabled(false);
	comboBoxFilterType->setEnabled(false);
	lineEditFilterExpression->setEnabled(false);
	toolButtonClearFilterExpression->setEnabled(false);
	frameEntities->setEnabled(false);
	romAlyzer()->groupBoxCheckSumDatabase->setEnabled(false);
	switch ( romAlyzer()->mode() ) {
		case QMC2_ROMALYZER_MODE_SOFTWARE:
			romAlyzer()->pushButtonRomCollectionRebuilder->setText(tr("Rebuilding software collection..."));
			break;
		case QMC2_ROMALYZER_MODE_SYSTEM:
		default:
			romAlyzer()->pushButtonRomCollectionRebuilder->setText(tr("Rebuilding ROM collection..."));
			break;
	}
	m_animationSequence = 0;
	m_animationTimer.start(QMC2_ROMALYZER_REBUILD_ANIM_SPEED);
	qApp->processEvents();
}

void CollectionRebuilder::rebuilderThread_rebuildFinished()
{
	int index = comboBoxXmlSource->currentIndex();
	qint64 cp = rebuilderThread()->checkpoint();
	if ( index == 0 )
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/Checkpoint", cp);
	else {
		index -= 1;
		QStringList checkpointList = qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceCheckpoints", QStringList()).toStringList();
		if ( index >= 0 && index < checkpointList.count() ) {
			checkpointList.replace(index, QString::number(cp));
			qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/XmlSourceCheckpoints", checkpointList);
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
	pushButtonStartStop->setIcon(QIcon(QString::fromUtf8(":/data/img/refresh.png")));
	pushButtonStartStop->setText(tr("Start rebuilding"));
	pushButtonPauseResume->hide();
	pushButtonStartStop->setEnabled(true);
	pushButtonPauseResume->setEnabled(true);
	comboBoxXmlSource->setEnabled(true);
	labelXmlSource->setEnabled(true);
	toolButtonRemoveXmlSource->setEnabled(true);
	checkBoxFilterExpression->setEnabled(true);
	comboBoxFilterSyntax->setEnabled(checkBoxFilterExpression->isChecked());
	comboBoxFilterType->setEnabled(checkBoxFilterExpression->isChecked());
	lineEditFilterExpression->setEnabled(checkBoxFilterExpression->isChecked());
	toolButtonClearFilterExpression->setEnabled(checkBoxFilterExpression->isChecked());
	frameEntities->setEnabled(true);
	romAlyzer()->groupBoxCheckSumDatabase->setEnabled(true);
	m_animationTimer.stop();
	romAlyzer()->pushButtonRomCollectionRebuilder->setIcon(QIcon(QString::fromUtf8(":/data/img/rebuild.png")));
	switch ( romAlyzer()->mode() ) {
		case QMC2_ROMALYZER_MODE_SOFTWARE:
			romAlyzer()->pushButtonRomCollectionRebuilder->setText(tr("Rebuild software collection..."));
			break;
		case QMC2_ROMALYZER_MODE_SYSTEM:
		default:
			romAlyzer()->pushButtonRomCollectionRebuilder->setText(tr("Rebuild ROM collection..."));
			break;
	}
	qApp->processEvents();
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

void CollectionRebuilder::rebuilderThread_statusUpdated(int setsProcessed, int missingDumps, int missingDisks)
{
	QString statusString = "<center><table border=\"0\" cellpadding=\"0\" cellspacing=\"4\"><tr>";
	statusString += "<td nowrap width=\"16.5%\" align=\"left\"><b>" + tr("Sets processed") + ":</b></td><td nowrap width=\"16.5%\" align=\"right\">" + QString::number(setsProcessed) + "</td>";
	statusString += "<td nowrap align=\"center\">|</td>";
	statusString += "<td nowrap width=\"16.5%\" align=\"left\"><b>" + tr("Missing ROMs") + ":</b></td><td nowrap width=\"16.5%\" align=\"right\">" + QString::number(missingDumps) + "</td>";
	statusString += "<td nowrap align=\"center\">|</td>";
	statusString += "<td nowrap width=\"16.5%\" align=\"left\"><b>" + tr("Missing disks") + ":</b></td><td nowrap width=\"16.5%\" align=\"right\">" + QString::number(missingDisks) + "</td>";
	statusString += "</tr></table></center>";
	labelRebuildStatus->setText(statusString);
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
	romAlyzer()->pushButtonRomCollectionRebuilder->setIcon(QIcon(rotatedPixmap));
	m_animationTimer.start(QMC2_ROMALYZER_REBUILD_ANIM_SPEED);
}

void CollectionRebuilder::showEvent(QShowEvent *e)
{
	restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + QString("Layout/%1/Geometry").arg(m_settingsKey), QByteArray()).toByteArray());
	if ( e )
		QDialog::showEvent(e);
}

void CollectionRebuilder::hideEvent(QHideEvent *e)
{
	if ( isVisible() )
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + QString("Layout/%1/Geometry").arg(m_settingsKey), saveGeometry());
	if ( e )
		QDialog::hideEvent(e);
}

void CollectionRebuilder::closeEvent(QCloseEvent *e)
{
	hideEvent(0);
	QDialog::closeEvent(e);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/UseFilterExpression", checkBoxFilterExpression->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterSyntax", comboBoxFilterSyntax->currentIndex());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterType", comboBoxFilterType->currentIndex());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/FilterExpression", lineEditFilterExpression->text());
}

void CollectionRebuilder::keyPressEvent(QKeyEvent *e)
{
	if ( e->key() == Qt::Key_Escape )
		close();
	else
		QDialog::keyPressEvent(e);
}

CollectionRebuilderThread::CollectionRebuilderThread(QObject *parent)
	: QThread(parent)
{
	isActive = exitThread = isWaiting = isPaused = pauseRequested = stopRebuilding = doFilter = isIncludeFilter = false;
	m_rebuilderDialog = (CollectionRebuilder *)parent;
	m_checkSumDb = NULL;
	m_xmlIndex = m_xmlIndexCount = m_checkpoint = -1;
	reopenDatabase();
	m_xmlDb = new XmlDatabaseManager(this);
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
}

void CollectionRebuilderThread::reopenDatabase()
{
	if ( checkSumDb() ) {
		checkSumDb()->disconnect(rebuilderDialog());
		delete checkSumDb();
	}
	m_checkSumDb = new CheckSumDatabaseManager(this, rebuilderDialog()->romAlyzer()->settingsKey());
	connect(checkSumDb(), SIGNAL(log(const QString &)), rebuilderDialog(), SLOT(log(const QString &)));
}

bool CollectionRebuilderThread::parseXml(QString xml, QString *id, QStringList *romNameList, QStringList *romSha1List, QStringList *romCrcList, QStringList *diskNameList, QStringList *diskSha1List)
{
	if ( xml.isEmpty() )
		return false;

	QString setEntityPattern("<" + rebuilderDialog()->lineEditSetEntity->text() + " name=\"");
	QString romEntityPattern("<" + rebuilderDialog()->lineEditRomEntity->text() + " name=\"");
	QString diskEntityPattern("<" + rebuilderDialog()->lineEditDiskEntity->text() + " name=\"");
	bool merge = !rebuilderDialog()->romAlyzer()->checkBoxSetRewriterSelfContainedSets->isChecked();
	int startIndex = -1;
	int endIndex = -1;
	QStringList xmlLines = xml.split("\n");
	QString xmlLine = xmlLines[0];
	startIndex = xmlLine.indexOf(setEntityPattern);
	if ( startIndex >= 0 ) {
		startIndex += setEntityPattern.length();
		endIndex = xmlLine.indexOf("\"", startIndex);
		if ( endIndex >= 0 ) {
			*id = xmlLine.mid(startIndex, endIndex - startIndex);
			for (int i = 1; i < xmlLines.count(); i++) {
				xmlLine = xmlLines[i];
				bool romFound = false;
				startIndex = xmlLine.indexOf(romEntityPattern);
				if ( startIndex >= 0 ) {
					startIndex += romEntityPattern.length();
					endIndex = xmlLine.indexOf("\"", startIndex);
					if ( endIndex >= 0 ) {
						romFound = true;
						QString romName = xmlLine.mid(startIndex, endIndex - startIndex);
						QString mergeName;
						QString status;
						startIndex = xmlLine.indexOf("status=\"");
						if ( startIndex >= 0 ) {
							startIndex += 8;
							endIndex = xmlLine.indexOf("\"", startIndex);
							if ( endIndex >= 0 )
								status = xmlLine.mid(startIndex, endIndex - startIndex);
						}
						startIndex = xmlLine.indexOf("merge=\"");
						if ( startIndex >= 0 ) {
							startIndex += 7;
							endIndex = xmlLine.indexOf("\"", startIndex);
							if ( endIndex >= 0 )
								mergeName = xmlLine.mid(startIndex, endIndex - startIndex);
						}
						if ( status != "nodump" && (!merge || mergeName.isEmpty()) ) {
							QString romSha1, romCrc;
							startIndex = xmlLine.indexOf("sha1=\"");
							if ( startIndex >= 0 ) {
								startIndex += 6;
								endIndex = xmlLine.indexOf("\"", startIndex);
								if ( endIndex >= 0 )
									romSha1 = xmlLine.mid(startIndex, endIndex - startIndex);
							}
							startIndex = xmlLine.indexOf("crc=\"");
							if ( startIndex >= 0 ) {
								startIndex += 5;
								endIndex = xmlLine.indexOf("\"", startIndex);
								if ( endIndex >= 0 )
									romCrc = xmlLine.mid(startIndex, endIndex - startIndex);
							}
							if ( !romSha1.isEmpty() || !romCrc.isEmpty() ) {
								*romNameList << romName.replace("&amp;", "&").replace("&lt;", "<").replace("&gt;", ">").replace("&quot;", "\"").replace("&apos;", "'");
								*romSha1List << romSha1;
								*romCrcList << romCrc;
							}
						}
					}
				}
				if ( romFound )
					continue;
				startIndex = xmlLine.indexOf(diskEntityPattern);
				if ( startIndex >= 0 ) {
					startIndex += diskEntityPattern.length();
					endIndex = xmlLine.indexOf("\"", startIndex);
					if ( endIndex >= 0 ) {
						QString diskName = xmlLine.mid(startIndex, endIndex - startIndex);
						QString mergeName;
						startIndex = xmlLine.indexOf("merge=\"");
						if ( startIndex >= 0 ) {
							startIndex += 7;
							endIndex = xmlLine.indexOf("\"", startIndex);
							if ( endIndex >= 0 )
								mergeName = xmlLine.mid(startIndex, endIndex - startIndex);
						}
						if ( !merge || mergeName.isEmpty() ) {
							QString diskSha1;
							startIndex = xmlLine.indexOf("sha1=\"");
							if ( startIndex >= 0 ) {
								startIndex += 6;
								endIndex = xmlLine.indexOf("\"", startIndex);
								if ( endIndex >= 0 )
									diskSha1 = xmlLine.mid(startIndex, endIndex - startIndex);
							}
							if ( !diskSha1.isEmpty() ) {
								*diskNameList << diskName.replace("&amp;", "&").replace("&lt;", "<").replace("&gt;", ">").replace("&quot;", "\"").replace("&apos;", "'");;
								*diskSha1List << diskSha1;
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

bool CollectionRebuilderThread::nextId(QString *id, QStringList *romNameList, QStringList *romSha1List, QStringList *romCrcList, QStringList *diskNameList, QStringList *diskSha1List)
{
	id->clear();
	romNameList->clear();
	romSha1List->clear();
	romCrcList->clear();
	diskNameList->clear();
	diskSha1List->clear();
	if ( m_xmlIndex < 0 || m_xmlIndexCount < 0 ) {
		if ( rebuilderDialog()->comboBoxXmlSource->currentIndex() == 0 ) {
			m_xmlIndex = 1;
			m_xmlIndexCount = xmlDb()->xmlRowCount();
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
		if ( rebuilderDialog()->comboBoxXmlSource->currentIndex() == 0 ) {
			if ( parseXml(xmlDb()->xml(m_xmlIndex), id, romNameList, romSha1List, romCrcList, diskNameList, diskSha1List) ) {
				setCheckpoint(m_xmlIndex, rebuilderDialog()->comboBoxXmlSource->currentIndex());
				m_xmlIndex++;
				emit progressChanged(m_xmlIndex);
				return true;
			} else {
				emit log(tr("FATAL: XML parsing failed"));
				setCheckpoint(-1, rebuilderDialog()->comboBoxXmlSource->currentIndex());
				return false;
			}
		} else {
			QString setEntityStartPattern("<" + rebuilderDialog()->lineEditSetEntity->text() + " name=\"");
			QByteArray line = m_xmlFile.readLine();
			while ( !m_xmlFile.atEnd() && line.indexOf(setEntityStartPattern) < 0 && !exitThread )
				line = m_xmlFile.readLine();
			if ( m_xmlFile.atEnd() ) {
				emit progressChanged(m_xmlIndexCount);
				setCheckpoint(-1, rebuilderDialog()->comboBoxXmlSource->currentIndex());
				return false;
			} else if ( !exitThread ) {
				QString xmlString;
				QString setEntityEndPattern("</" + rebuilderDialog()->lineEditSetEntity->text() + ">");
				while ( !m_xmlFile.atEnd() && line.indexOf(setEntityEndPattern) < 0 && !exitThread ) {
					xmlString += line;
					line = m_xmlFile.readLine();
				}
				if ( !m_xmlFile.atEnd() && !exitThread ) {
					xmlString += line;
					if ( parseXml(xmlString, id, romNameList, romSha1List, romCrcList, diskNameList, diskSha1List) ) {
						setCheckpoint(m_xmlIndex, rebuilderDialog()->comboBoxXmlSource->currentIndex());
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
		QStringList checkpointList = qmc2Config->value(QMC2_FRONTEND_PREFIX + rebuilderDialog()->settingsKey() + "/XmlSourceCheckpoints", QStringList()).toStringList();
		checkpointList.replace(xmlSourceIndex, QString::number(checkpoint()));
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + rebuilderDialog()->settingsKey() + "/XmlSourceCheckpoints", checkpointList);
	}
}

void CollectionRebuilderThread::checkpointRestart(qint64 cp)
{
	if ( cp < 0 ) {
		m_xmlIndex = m_xmlIndexCount = -1;
		return;
	}
	m_xmlIndex = cp;
	if ( rebuilderDialog()->comboBoxXmlSource->currentIndex() == 0 ) {
		emit log(tr("restarting from checkpoint '%1'").arg(m_xmlIndex));
		m_xmlIndexCount = xmlDb()->xmlRowCount();
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

bool CollectionRebuilderThread::rewriteSet(QString *id, QStringList *romNameList, QStringList *romSha1List, QStringList *romCrcList, QStringList *diskNameList, QStringList *diskSha1List)
{
	QString baseDir = rebuilderDialog()->romAlyzer()->lineEditSetRewriterOutputPath->text();
	if ( rebuilderDialog()->romAlyzer()->radioButtonSetRewriterZipArchives->isChecked() )
		return writeAllZipData(baseDir, *id, romNameList, romSha1List, romCrcList, diskNameList, diskSha1List);
	else
		return writeAllFileData(baseDir, *id, romNameList, romSha1List, romCrcList, diskNameList, diskSha1List);
}

bool CollectionRebuilderThread::writeAllFileData(QString baseDir, QString id, QStringList *romNameList, QStringList *romSha1List, QStringList *romCrcList, QStringList *diskNameList, QStringList *diskSha1List)
{
	// FIXME: no support for disks
	bool success = true;
	QDir d(QDir::cleanPath(baseDir + "/" + id));
	if ( !d.exists() )
		success = d.mkdir(QDir::cleanPath(baseDir + "/" + id));
	int reproducedDumps = 0;
	for (int i = 0; i < romNameList->count() && !exitThread && success; i++) {
		QString fileName = d.absoluteFilePath(romNameList->at(i));
		if ( !createBackup(fileName) ) {
			emit log(tr("FATAL: backup creation failed"));
			success = false;
		}
		QFile f(fileName);
		if ( f.open(QIODevice::WriteOnly) ) {
			QByteArray data;
			quint64 size;
			QString path, member, type;
			if ( checkSumDb()->getData(romSha1List->at(i), romCrcList->at(i), &size, &path, &member, &type) ) {
				if ( type == "ZIP" )
					success = readZipFileData(path, romCrcList->at(i), &data);
				else if ( type == "7Z" )
					success = readSevenZipFileData(path, romCrcList->at(i), &data);
				else if ( type == "FILE" )
					success = readFileData(path, &data);
				else
					success = false;
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
	}
	if ( reproducedDumps == 0 )
		d.rmdir(d.absolutePath());
	return success;
}

bool CollectionRebuilderThread::writeAllZipData(QString baseDir, QString id, QStringList *romNameList, QStringList *romSha1List, QStringList *romCrcList, QStringList *diskNameList, QStringList *diskSha1List)
{
	// FIXME: no support for disks
	QDir d(QDir::cleanPath(baseDir));
	if ( !d.exists() )
		if ( !d.mkdir(QDir::cleanPath(baseDir)) )
			return false;
	QString fileName = QDir::cleanPath(baseDir) + "/" + id + ".zip";
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
	int zipLevel = rebuilderDialog()->romAlyzer()->spinBoxSetRewriterZipLevel->value();
	zipFile zip = zipOpen(fileName.toLocal8Bit().constData(), APPEND_STATUS_CREATE);
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
			QString file = romNameList->at(i);
			QByteArray data;
			quint64 size;
			QString path, member, type;
			if ( checkSumDb()->getData(romSha1List->at(i), romCrcList->at(i), &size, &path, &member, &type) ) {
				if ( type == "ZIP" )
					success = readZipFileData(path, romCrcList->at(i), &data);
				else if ( type == "7Z" )
					success = readSevenZipFileData(path, romCrcList->at(i), &data);
				else if ( type == "FILE" )
					success = readFileData(path, &data);
				else
					success = false;
				if ( success && zipOpenNewFileInZip(zip, file.toLocal8Bit().constData(), &zipInfo, (const void *)file.toLocal8Bit().constData(), file.length(), 0, 0, 0, Z_DEFLATED, zipLevel) == ZIP_OK ) {
					emit log(tr("writing '%1' to ZIP archive '%2' (uncompressed size: %3)").arg(file).arg(fileName).arg(ROMAlyzer::humanReadable(data.length())));
					quint64 bytesWritten = 0;
					while ( bytesWritten < (quint64)data.length() && !exitThread && success ) {
						quint64 bufferLength = QMC2_ZIP_BUFFER_SIZE;
						if ( bytesWritten + bufferLength > (quint64)data.length() )
							bufferLength = data.length() - bytesWritten;
						QByteArray writeBuffer = data.mid(bytesWritten, bufferLength);
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
		}
		if ( rebuilderDialog()->romAlyzer()->checkBoxAddZipComment->isChecked() )
			zipClose(zip, tr("Created by QMC2 v%1 (%2)").arg(XSTR(QMC2_VERSION)).arg(cDT.toString(Qt::SystemLocaleShortDate)).toLocal8Bit().constData());
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

bool CollectionRebuilderThread::readFileData(QString fileName, QByteArray *data)
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

bool CollectionRebuilderThread::readSevenZipFileData(QString fileName, QString crc, QByteArray *data)
{
	SevenZipFile sevenZipFile(fileName);
	if ( sevenZipFile.open() ) {
		int index = sevenZipFile.indexOfCrc(crc);
		if ( index >= 0 ) {
			SevenZipMetaData metaData = sevenZipFile.itemList()[index];
			emit log(tr("reading '%1' from 7Z archive '%2' (uncompressed size: %3)").arg(metaData.name()).arg(fileName).arg(ROMAlyzer::humanReadable(metaData.size())));
			quint64 readLength = sevenZipFile.read(index, data); // can't be interrupted!
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

bool CollectionRebuilderThread::readZipFileData(QString fileName, QString crc, QByteArray *data)
{
	bool success = true;
	unzFile zipFile = unzOpen(fileName.toLocal8Bit().constData());
	if ( zipFile ) {
  		char ioBuffer[QMC2_ZIP_BUFFER_SIZE];
		unz_file_info zipInfo;
		QMap<uLong, QString> crcIdentMap;
		uLong ulCRC = crc.toULong(0, 16);
		do {
			if ( unzGetCurrentFileInfo(zipFile, &zipInfo, ioBuffer, QMC2_ROMALYZER_ZIP_BUFFER_SIZE, 0, 0, 0, 0) == UNZ_OK )
				crcIdentMap[zipInfo.crc] = QString((const char *)ioBuffer);
		} while ( unzGoToNextFile(zipFile) == UNZ_OK && !crcIdentMap.contains(ulCRC) );
		unzGoToFirstFile(zipFile);
		if ( crcIdentMap.contains(ulCRC) ) {
			QString fn = crcIdentMap[ulCRC];
			if ( unzLocateFile(zipFile, fn.toLocal8Bit().constData(), 2) == UNZ_OK ) {
				if ( unzOpenCurrentFile(zipFile) == UNZ_OK ) {
					emit log(tr("reading '%1' from ZIP archive '%2' (uncompressed size: %3)").arg(fn).arg(fileName).arg(ROMAlyzer::humanReadable(zipInfo.uncompressed_size)));
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

void CollectionRebuilderThread::setFilterExpression(QString expression, int syntax, int type)
{
	doFilter = !expression.isEmpty();
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

void CollectionRebuilderThread::pause()
{
	pauseRequested = true;
}

void CollectionRebuilderThread::resume()
{
	isPaused = false;
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
			quint64 setsProcessed = 0, missingDumps = 0, missingDisks = 0;
			emit log(tr("rebuilding started"));
			emit statusUpdated(setsProcessed, missingDumps, missingDisks);
			emit rebuildStarted();
			emit progressTextChanged(tr("Rebuilding"));
			QTime rebuildTimer, elapsedTime(0, 0, 0, 0);
			rebuildTimer.start();
			if ( checkpoint() < 0 )
				m_xmlIndex = m_xmlIndexCount = -1;
			QString id;
			QStringList romNameList, romSha1List, romCrcList, diskNameList, diskSha1List;
			while ( !exitThread && !stopRebuilding && nextId(&id, &romNameList, &romSha1List, &romCrcList, &diskNameList, &diskSha1List) ) {
				bool pauseMessageLogged = false;
				while ( (pauseRequested || isPaused) && !exitThread && !stopRebuilding ) {
					if ( !pauseMessageLogged ) {
						pauseMessageLogged = true;
						isPaused = true;
						pauseRequested = false;
						emit progressTextChanged(tr("Paused"));
						emit rebuildPaused();
						emit log(tr("rebuilding paused"));
					}
					QTest::qWait(100);
				}
				if ( pauseMessageLogged && !exitThread && !stopRebuilding ) {
					isPaused = false;
					emit progressTextChanged(tr("Rebuilding"));
					emit rebuildResumed();
					emit log(tr("rebuilding resumed"));
				}
				QTest::qWait(0);
				if ( id.isEmpty() )
					continue;
				if ( doFilter ) {
					if ( isIncludeFilter ) {
						if ( filterRx.indexIn(id) < 0 ) {
							emit log(tr("rebuilding of '%1' skipped due to filter").arg(id));
							continue;
						}
					} else {
						if ( filterRx.indexIn(id) >= 0 ) {
							emit log(tr("rebuilding of '%1' skipped due to filter").arg(id));
							continue;
						}
					}
				}
				if ( !exitThread && !stopRebuilding && (!romNameList.isEmpty() || !diskNameList.isEmpty()) ) {
					emit log(tr("set rebuilding started for '%1'").arg(id));
					for (int i = 0; i < romNameList.count(); i++) {
						bool dbStatusGood = checkSumDb()->exists(romSha1List[i], romCrcList[i]);
						emit log(tr("required ROM") + ": " + tr("name = '%1', crc = '%2', sha1 = '%3', database status = '%4'").arg(romNameList[i]).arg(romCrcList[i]).arg(romSha1List[i]).arg(dbStatusGood ? tr("available") : tr("not available")));
						if ( !dbStatusGood )
							missingDumps++;
					}
					for (int i = 0; i < diskNameList.count(); i++) {
						bool dbStatusGood = checkSumDb()->exists(diskSha1List[i], QString());
						emit log(tr("required disk") + ": " + tr("name = '%1', sha1 = '%2', database status = '%3'").arg(diskNameList[i]).arg(diskSha1List[i]).arg(dbStatusGood ? tr("available") : tr("not available")));
						if ( !dbStatusGood )
							missingDisks++;
					}
					bool rewriteOkay = true;
					if ( !romNameList.isEmpty() )
						rewriteOkay = rewriteSet(&id, &romNameList, &romSha1List, &romCrcList, &diskNameList, &diskSha1List);
					if ( rewriteOkay )
						emit log(tr("set rebuilding finished for '%1'").arg(id));
					else
						emit log(tr("set rebuilding failed for '%1'").arg(id));
					emit statusUpdated(++setsProcessed, missingDumps, missingDisks);
				}
			}
			elapsedTime = elapsedTime.addMSecs(rebuildTimer.elapsed());
			emit log(tr("rebuilding finished - total rebuild time = %1, sets processed = %2, missing ROMs = %3, missing disks = %4").arg(elapsedTime.toString("hh:mm:ss.zzz")).arg(setsProcessed).arg(missingDumps).arg(missingDisks));
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
