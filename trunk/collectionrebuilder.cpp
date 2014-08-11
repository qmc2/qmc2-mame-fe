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

#include "collectionrebuilder.h"
#include "settings.h"
#include "options.h"
#include "romalyzer.h"
#include "unzip.h"
#include "zip.h"
#include "sevenzipfile.h"
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
	connect(rebuilderThread(), SIGNAL(statusUpdated(int, int, int)), this, SLOT(rebuilderThread_statusUpdated(int, int, int)));

#if defined(QMC2_EMUTYPE_MESS)
	m_defaultSetEntity = "machine";
#else
	m_defaultSetEntity = "game";
#endif
	m_defaultRomEntity = "rom";
	m_defaultDiskEntity = "disk";

	frameEntities->setVisible(false);
	toolButtonRemoveXmlSource->setVisible(false);
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
	toolButtonRemoveXmlSource->setIconSize(iconSize);
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
		QTimer::singleShot(0, this, SLOT(scrollToEnd()));
		toolButtonRemoveXmlSource->setVisible(false);
		lastIndex = -1;
	} else if ( index == comboBoxXmlSource->count() - 1 ) {
		QString xmlSource = QFileDialog::getOpenFileName(this, tr("Choose source XML file"), QString(), tr("Data files (*.dat)") + ";;" + tr("XML files (*.xml)") + ";;" + tr("All files (*)"), 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
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
				int insertIndex = comboBoxXmlSource->count() - 2;
				lastIndex = insertIndex;
				comboBoxXmlSource->insertItem(insertIndex, xmlSource);
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
		toolButtonRemoveXmlSource->setVisible(true);
		QTimer::singleShot(0, this, SLOT(scrollToEnd()));
	}

}

void CollectionRebuilder::on_toolButtonRemoveXmlSource_clicked()
{
	int index = comboBoxXmlSource->currentIndex() - 2;
	comboBoxXmlSource->blockSignals(true);
	comboBoxXmlSource->removeItem(comboBoxXmlSource->currentIndex());
	comboBoxXmlSource->blockSignals(false);
	comboBoxXmlSource->setCurrentIndex(0);
	QStringList xmlSources = qmc2Config->value(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/XmlSources", QStringList()).toStringList();
	xmlSources.removeAt(index);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/XmlSources", xmlSources);
	QStringList setEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/SetEntities", QStringList()).toStringList();
	setEntities.removeAt(index);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/SetEntities", setEntities);
	QStringList romEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/RomEntities", QStringList()).toStringList();
	romEntities.removeAt(index);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/RomEntities", romEntities);
	QStringList diskEntities = qmc2Config->value(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/DiskEntities", QStringList()).toStringList();
	diskEntities.removeAt(index);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "CollectionRebuilder/DiskEntities", diskEntities);
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
	labelXmlSource->setEnabled(false);
	toolButtonRemoveXmlSource->setEnabled(false);
	frameEntities->setEnabled(false);
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
	labelXmlSource->setEnabled(true);
	toolButtonRemoveXmlSource->setEnabled(true);
	frameEntities->setEnabled(true);
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
	if ( xml.isEmpty() )
		return false;

	QString setEntityPattern("<" + rebuilderDialog()->lineEditSetEntity->text() + " name=\"");
	QString romEntityPattern("<" + rebuilderDialog()->lineEditRomEntity->text() + " name=\"");
	QString diskEntityPattern("<" + rebuilderDialog()->lineEditDiskEntity->text() + " name=\"");
	bool merge = !qmc2ROMAlyzer->checkBoxSetRewriterSelfContainedSets->isChecked();
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
						startIndex = xmlLine.indexOf("merge=\"");
						if ( startIndex >= 0 ) {
							startIndex += 7;
							endIndex = xmlLine.indexOf("\"", startIndex);
							if ( endIndex >= 0 )
								mergeName = xmlLine.mid(startIndex, endIndex - startIndex);
						}
						if ( !merge || mergeName.isEmpty() ) {
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
							if ( !romSha1.isEmpty() && !romCrc.isEmpty() ) {
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
								*diskNameList << diskName;
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
			return true;
		} else {
			m_xmlFile.setFileName(rebuilderDialog()->comboBoxXmlSource->currentText());
			if ( m_xmlFile.open(QIODevice::ReadOnly | QIODevice::Text) ) {
				m_xmlIndex = 0;
				m_xmlIndexCount = m_xmlFile.size() - 1;
				emit progressRangeChanged(m_xmlIndex, m_xmlIndexCount);
				emit progressChanged(m_xmlIndex);
				return true;
			} else {
				emit log(tr("FATAL: can't open XML file '%1' for reading, please check permissions").arg(rebuilderDialog()->comboBoxXmlSource->currentText()));
				return false;
			}
		}
	} else {
		if ( m_xmlIndex > m_xmlIndexCount ) {
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
			QString setEntityStartPattern("<" + rebuilderDialog()->lineEditSetEntity->text() + " name=\"");
			QByteArray line = m_xmlFile.readLine();
			while ( !m_xmlFile.atEnd() && line.indexOf(setEntityStartPattern) < 0 && !exitThread && !stopRebuilding )
				line = m_xmlFile.readLine();
			if ( m_xmlFile.atEnd() ) {
				emit progressChanged(m_xmlIndexCount);
				return false;
			} else if ( !exitThread && !stopRebuilding ) {
				QString xmlString;
				QString setEntityEndPattern("</" + rebuilderDialog()->lineEditSetEntity->text() + ">");
				while ( !m_xmlFile.atEnd() && line.indexOf(setEntityEndPattern) < 0 && !exitThread && !stopRebuilding ) {
					xmlString += line;
					line = m_xmlFile.readLine();
				}
				if ( !m_xmlFile.atEnd() && !exitThread && !stopRebuilding ) {
					xmlString += line;
					if ( parseXml(xmlString, id, romNameList, romSha1List, romCrcList, diskNameList, diskSha1List) ) {
						m_xmlIndex = m_xmlFile.pos();
						emit progressChanged(m_xmlIndex);
						return true;
					} else {
						emit log(tr("FATAL: XML parsing failed"));
						return false;
					}
				} else {
					emit log(tr("FATAL: XML parsing failed"));
					return false;
				}
			} else
				return false;
		}
	}
}

bool CollectionRebuilderThread::rewriteSet(QString *id, QStringList *romNameList, QStringList *romSha1List, QStringList *romCrcList, QStringList *diskNameList, QStringList *diskSha1List)
{
	QString baseDir = qmc2ROMAlyzer->lineEditSetRewriterOutputPath->text();
	if ( qmc2ROMAlyzer->radioButtonSetRewriterZipArchives->isChecked() )
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
	for (int i = 0; i < romNameList->count() && !exitThread && !stopRebuilding && success; i++) {
		QString fileName = d.absoluteFilePath(romNameList->at(i));
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
					while ( bytesWritten < (quint64)data.length() && !exitThread && !stopRebuilding && success ) {
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
			}
			f.close();
		} else {
			emit log(tr("FATAL: failed opening '%1' for writing"));
			success = false;
		}
	}
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
	QFile f(fileName);
	if ( f.exists() )
		if ( !f.remove() )
			return false;
	bool success = true;
	bool uniqueCRCs = qmc2ROMAlyzer->checkBoxSetRewriterUniqueCRCs->isChecked();
	int zipLevel = qmc2ROMAlyzer->spinBoxSetRewriterZipLevel->value();
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
		for (int i = 0; i < romNameList->count() && !exitThread && !stopRebuilding && success; i++) {
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
					while ( bytesWritten < (quint64)data.length() && !exitThread && !stopRebuilding && success ) {
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
				}
			}
		}
		zipClose(zip, tr("Created by QMC2 v%1 (%2)").arg(XSTR(QMC2_VERSION)).arg(cDT.toString(Qt::SystemLocaleShortDate)).toLocal8Bit().constData());
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
		while ( (len = file.read(ioBuffer, QMC2_FILE_BUFFER_SIZE)) > 0 && !exitThread && !stopRebuilding )
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
					while ( (len = unzReadCurrentFile(zipFile, ioBuffer, QMC2_ROMALYZER_ZIP_BUFFER_SIZE)) > 0 && !exitThread && !stopRebuilding )
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
				if ( !romNameList.isEmpty() || !diskNameList.isEmpty() ) {
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
					if ( rewriteSet(&id, &romNameList, &romSha1List, &romCrcList, &diskNameList, &diskSha1List) )
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
