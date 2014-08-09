#include <QTest>
#include <QFont>
#include <QFontInfo>
#include <QDateTime>
#include <QDir>
#include <QFileDialog>

#include "collectionrebuilder.h"
#include "settings.h"
#include "options.h"
#include "romalyzer.h"
#include "macros.h"

extern Settings *qmc2Config;
extern ROMAlyzer *qmc2ROMAlyzer;
extern Options *qmc2Options;

CollectionRebuilder::CollectionRebuilder(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
	pushButtonPauseResume->setVisible(false);
	QFont logFont;
	logFont.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/LogFont").toString());
	plainTextEditLog->setFont(logFont);
	spinBoxMaxLogSize->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/MaxLogSize", 10000).toInt());
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

#if defined(QMC2_EMUTYPE_MESS)
	m_defaultSetEntity = "machine";
#else
	m_defaultSetEntity = "game";
#endif
	m_defaultRomEntity = "rom";
	m_defaultDiskEntity = "disk";

	frameEntities->setVisible(false);
	comboBoxXmlSource->blockSignals(true);
	comboBoxXmlSource->clear();
	comboBoxXmlSource->insertItem(0, tr("Current default emulator"));
	comboBoxXmlSource->insertSeparator(1);
	QStringList xmlSources = qmc2Config->value(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/XmlSources", QStringList()).toStringList();
	QStringList setEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/SetEntities", QStringList()).toStringList();
	QStringList romEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/RomEntities", QStringList()).toStringList();
	QStringList diskEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/DiskEntities", QStringList()).toStringList();
	int index = 2;
	if ( xmlSources.count() > 0 && setEntities.count() == xmlSources.count() && romEntities.count() == xmlSources.count() && diskEntities.count() == xmlSources.count() ) {
		for (int i = 0; i < xmlSources.count(); i++) {
			QString xmlSource = xmlSources[i];
			QFileInfo fi(xmlSource);
			if ( fi.exists() && fi.isReadable() ) {
				comboBoxXmlSource->insertItem(index, xmlSource);
				index++;
			} else {
				xmlSources.removeAt(i);
				setEntities.removeAt(i);
				romEntities.removeAt(i);
				diskEntities.removeAt(i);
				i--;
			}
		}
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/XmlSources", xmlSources);
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/SetEntities", setEntities);
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/RomEntities", romEntities);
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/DiskEntities", diskEntities);
	} else {
		qmc2Config->remove(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/XmlSources");
		qmc2Config->remove(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/SetEntities");
		qmc2Config->remove(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/RomEntities");
		qmc2Config->remove(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/DiskEntities");
	}
	if ( index > 2 ) {
		comboBoxXmlSource->insertSeparator(index);
		index++;
	}
	comboBoxXmlSource->insertItem(index, tr("Select XML file..."));
	comboBoxXmlSource->setItemIcon(index, QIcon(QString::fromUtf8(":/data/img/fileopen.png")));
	comboBoxXmlSource->blockSignals(false);
	comboBoxXmlSource->setCurrentIndex(0);
	comboBoxXmlSource->setFocus();
}

CollectionRebuilder::~CollectionRebuilder()
{
	if ( rebuilderThread() )
		delete rebuilderThread();
}

void CollectionRebuilder::on_spinBoxMaxLogSize_valueChanged(int value)
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/MaxLogSize", value);
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
	else if ( rebuilderThread()->isWaiting )
		rebuilderThread()->waitCondition.wakeAll();
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
			QStringList setEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/SetEntities", QStringList()).toStringList();
			QStringList romEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/RomEntities", QStringList()).toStringList();
			QStringList diskEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/DiskEntities", QStringList()).toStringList();
			if ( lastIndex < setEntities.count() ) {
				setEntities.replace(lastIndex, lineEditSetEntity->text());
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/SetEntities", setEntities);
			}
			if ( lastIndex < romEntities.count() ) {
				romEntities.replace(lastIndex, lineEditRomEntity->text());
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/RomEntities", romEntities);
			}
			if ( lastIndex < diskEntities.count() ) {
				diskEntities.replace(lastIndex, lineEditDiskEntity->text());
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/DiskEntities", diskEntities);
			}
		}
		lineEditSetEntity->setText(m_defaultSetEntity);
		lineEditRomEntity->setText(m_defaultRomEntity);
		lineEditDiskEntity->setText(m_defaultDiskEntity);
		frameEntities->setVisible(false);
		lastIndex = -1;
	} else if ( index == comboBoxXmlSource->count() - 1 ) {
		QString xmlSource = QFileDialog::getOpenFileName(this, tr("Choose source XML file"), QString(), tr("XML files (*.xml)") + ";;" + tr("All files (*)"), 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
		if ( !xmlSource.isNull() ) {
			int foundAtIndex = comboBoxXmlSource->findText(xmlSource);
			if ( foundAtIndex < 0 ) {
				comboBoxXmlSource->blockSignals(true);
				QStringList xmlSources = qmc2Config->value(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/XmlSources", QStringList()).toStringList();
				xmlSources << xmlSource;
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/XmlSources", xmlSources);
				QStringList setEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/SetEntities", QStringList()).toStringList();
				setEntities << m_defaultSetEntity;
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/SetEntities", setEntities);
				lineEditSetEntity->setText(m_defaultSetEntity);
				QStringList romEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/RomEntities", QStringList()).toStringList();
				romEntities << m_defaultRomEntity;
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/RomEntities", romEntities);
				lineEditRomEntity->setText(m_defaultRomEntity);
				QStringList diskEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/DiskEntities", QStringList()).toStringList();
				diskEntities << m_defaultDiskEntity;
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/DiskEntities", diskEntities);
				lineEditDiskEntity->setText(m_defaultDiskEntity);
				lastIndex = -1;
				int insertIndex = comboBoxXmlSource->count() - 2;
				comboBoxXmlSource->insertItem(insertIndex, xmlSource);
				comboBoxXmlSource->setCurrentIndex(insertIndex);
				comboBoxXmlSource->blockSignals(false);
				frameEntities->setVisible(true);
			} else
				comboBoxXmlSource->setCurrentIndex(foundAtIndex);
		} else
			comboBoxXmlSource->setCurrentIndex(0);
		raise();
	} else {
		index -= 2;
		QStringList setEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/SetEntities", QStringList()).toStringList();
		QStringList romEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/RomEntities", QStringList()).toStringList();
		QStringList diskEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/DiskEntities", QStringList()).toStringList();
		if ( lastIndex >= 0 ) {
			if ( lastIndex < setEntities.count() ) {
				setEntities.replace(lastIndex, lineEditSetEntity->text());
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/SetEntities", setEntities);
			}
			if ( lastIndex < romEntities.count() ) {
				romEntities.replace(lastIndex, lineEditRomEntity->text());
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/RomEntities", romEntities);
			}
			if ( lastIndex < diskEntities.count() ) {
				diskEntities.replace(lastIndex, lineEditDiskEntity->text());
				qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/DiskEntities", diskEntities);
			}
		}
		lastIndex = index;
		lineEditSetEntity->setText(setEntities[index]);
		lineEditRomEntity->setText(romEntities[index]);
		lineEditDiskEntity->setText(diskEntities[index]);
		frameEntities->setVisible(true);
	}

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
	qmc2ROMAlyzer->groupBoxCheckSumDatabase->setEnabled(false);
	qApp->processEvents();
}

void CollectionRebuilder::rebuilderThread_rebuildFinished()
{
	pushButtonStartStop->setIcon(QIcon(QString::fromUtf8(":/data/img/refresh.png")));
	pushButtonStartStop->setText(tr("Start rebuilding"));
	pushButtonPauseResume->hide();
	pushButtonStartStop->setEnabled(true);
	pushButtonPauseResume->setEnabled(true);
	comboBoxXmlSource->setEnabled(true);
	qmc2ROMAlyzer->groupBoxCheckSumDatabase->setEnabled(true);
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

void CollectionRebuilder::showEvent(QShowEvent *e)
{
	restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/CollectionRebuilder/Geometry", QByteArray()).toByteArray());
	if ( e )
		QDialog::showEvent(e);
}

void CollectionRebuilder::hideEvent(QHideEvent *e)
{
	if ( isVisible() )
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/CollectionRebuilder/Geometry", saveGeometry());
	if ( e )
		QDialog::hideEvent(e);
}

void CollectionRebuilder::closeEvent(QCloseEvent *e)
{
	hideEvent(0);
	QDialog::closeEvent(e);
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
	isActive = exitThread = isWaiting = isPaused = pauseRequested = stopRebuilding = false;
	m_rebuilderDialog = (CollectionRebuilder *)parent;
	m_checkSumDb = NULL;
	m_xmlIndex = m_xmlIndexCount = -1;
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
		checkSumDb()->disconnect(m_rebuilderDialog);
		delete checkSumDb();
	}
	if ( xmlDb() )
		delete xmlDb();
}

void CollectionRebuilderThread::reopenDatabase()
{
	if ( checkSumDb() ) {
		checkSumDb()->disconnect(m_rebuilderDialog);
		delete checkSumDb();
	}
	m_checkSumDb = new CheckSumDatabaseManager(this);
	connect(checkSumDb(), SIGNAL(log(const QString &)), m_rebuilderDialog, SLOT(log(const QString &)));
}

bool CollectionRebuilderThread::parseXml(QString xml, QString *id, QStringList *romNameList, QStringList *romSha1List, QStringList *romCrcList, QStringList *diskNameList, QStringList *diskSha1List)
{
	// FIXME
	return true;
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
			m_xmlIndex = 0;
			m_xmlIndexCount = xmlDb()->xmlRowCount();
			emit progressRangeChanged(m_xmlIndex, m_xmlIndexCount);
			emit progressChanged(m_xmlIndex);
			return true;
		} else {
			m_xmlFile.setFileName(rebuilderDialog()->comboBoxXmlSource->currentText());
			if ( m_xmlFile.open(QIODevice::ReadOnly | QIODevice::Text) ) {
				m_xmlIndex = 0;
				m_xmlIndexCount = m_xmlFile.size();
				emit progressRangeChanged(m_xmlIndex, m_xmlIndexCount);
				emit progressChanged(m_xmlIndex);
				return true;
			} else {
				emit log(tr("FATAL: can't open XML file '%1' for reading, please check permissions").arg(rebuilderDialog()->comboBoxXmlSource->currentText()));
				return false;
			}
		}
	} else {
		if ( m_xmlIndex > m_xmlIndexCount - 1 ) {
			emit progressChanged(m_xmlIndexCount);
			return false;
		}
		if ( rebuilderDialog()->comboBoxXmlSource->currentIndex() == 0 ) {
			if ( parseXml(xmlDb()->xml(m_xmlIndex), id, romNameList, romSha1List, romCrcList, diskNameList, diskSha1List) ) {
				m_xmlIndex++;
				emit progressChanged(m_xmlIndex);
				return true;
			} else {
				emit log(tr("FATAL: XML parsing failed"));
				return false;
			}
		} else {
			QString setEntityPattern("<" + rebuilderDialog()->lineEditSetEntity->text() + " name=\"");
			QByteArray line = m_xmlFile.readLine();
			while ( !m_xmlFile.atEnd() && line.indexOf(setEntityPattern) < 0 && !exitThread && !stopRebuilding )
				line = m_xmlFile.readLine();
			if ( m_xmlFile.atEnd() ) {
				emit progressChanged(m_xmlIndexCount);
				return false;
			} else {
				QString xmlString;
				// FIXME
				m_xmlIndex = m_xmlFile.pos();
				emit progressChanged(m_xmlIndex);
				return true;
			}
		}
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
			emit log(tr("rebuilding started"));
			emit rebuildStarted();
			emit progressTextChanged(tr("Rebuilding"));
			QTime rebuildTimer, elapsedTime(0, 0, 0, 0);
			rebuildTimer.start();
			m_xmlIndex = m_xmlIndexCount = -1;
			QString id;
			QStringList romNameList, romSha1List, romCrcList, diskNameList, diskSha1List;
			while ( !exitThread && !stopRebuilding && nextId(&id, &romNameList, &romSha1List, &romCrcList, &diskNameList, &diskSha1List) ) {
				if ( id.isEmpty() )
					continue;
				// FIXME
			}
			elapsedTime = elapsedTime.addMSecs(rebuildTimer.elapsed());
			emit log(tr("rebuilding finished, total rebuild time = %1").arg(elapsedTime.toString("hh:mm:ss.zzz")));
			emit rebuildFinished();
			emit progressTextChanged(tr("Idle"));
			emit progressChanged(0);
			if ( m_xmlFile.isOpen() )
				m_xmlFile.close();
		}
	}
	emit log(tr("rebuilder thread ended"));
}
