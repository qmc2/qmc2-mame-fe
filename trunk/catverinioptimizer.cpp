#include <QFont>
#include <QChar>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QDateTime>
#include <QTreeWidgetItem>
#include <QApplication>
#include <QFileInfo>

#include "catverinioptimizer.h"
#include "settings.h"
#include "options.h"
#include "machinelist.h"
#include "macros.h"

extern Settings *qmc2Config;
extern Options *qmc2Options;
extern MachineList *qmc2MachineList;
extern QHash<QString, QString> qmc2ParentHash;
extern QHash<QString, QTreeWidgetItem *> qmc2MachineListItemHash;

CatverIniOptimizer::CatverIniOptimizer(QString fileName, QWidget *parent) :
	QDialog(parent)
{
	setupUi(this);
	m_fileName = fileName;
	QFont logFont;
	logFont.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/LogFont").toString());
	plainTextEdit->setFont(logFont);
	if ( m_fileName.isEmpty() ) {
		log(tr("ERROR: file name is empty") + " - " + tr("no catver.ini data available"));
		pushButtonOptimize->setEnabled(false);
	} else { 
		QFileInfo fi(m_fileName);
		if ( fi.exists() ) {
			if ( fi.isReadable() )
				log(tr("click 'optimize' to start"));
			else {
				log(tr("ERROR: '%1' isn't readable").arg(m_fileName) + " - " + tr("no catver.ini data available"));
				pushButtonOptimize->setEnabled(false);
			}
		} else {
			log(tr("ERROR: '%1' doesn't exist").arg(m_fileName) + " - " + tr("no catver.ini data available"));
			pushButtonOptimize->setEnabled(false);
		}
	}
}

CatverIniOptimizer::~CatverIniOptimizer()
{
	clearCategoryNames();
	clearVersionNames();
}

void CatverIniOptimizer::clearCategoryNames()
{
	foreach (QString *category, m_categoryNames)
		if ( category )
			delete category;
	m_categoryNames.clear();
}

void CatverIniOptimizer::clearVersionNames()
{
	foreach (QString *version, m_versionNames)
		if ( version )
			delete version;
	m_versionNames.clear();
}

bool CatverIniOptimizer::loadCatverIni()
{
	clearCategoryNames();
	m_categoryMap.clear();
	clearVersionNames();
	m_versionMap.clear();
	log(tr("loading catver.ini data from '%1'").arg(m_fileName));
	QFile catverIniFile(m_fileName);
	if ( catverIniFile.open(QIODevice::ReadOnly | QIODevice::Text) ) {
		QTextStream tsCatverIni(&catverIniFile);
		int lineCounter = 0, catVerSwitch = 0;
		QChar splitChar('='), dotChar('.'), zeroChar('0');
		QString catStr("[Category]"), verStr("[VerAdded]");
		while ( !tsCatverIni.atEnd() ) {
			QString catverLine(tsCatverIni.readLine());
			if ( catverLine.isEmpty() )
				continue;
			QStringList tokens(catverLine.split(splitChar, QString::SkipEmptyParts));
			if ( tokens.count() > 1 ) {
				QString token1(tokens.at(1).trimmed());
				switch ( catVerSwitch ) {
					case 1: // category
						if ( !m_categoryNames.contains(token1) )
							m_categoryNames.insert(token1, new QString(token1));
						m_categoryMap.insert(tokens.at(0).trimmed(), m_categoryNames.value(token1));
						break;
					case 2: // version
						if ( token1.startsWith(dotChar) )
							token1.prepend(zeroChar);
						if ( !m_versionNames.contains(token1) )
							m_versionNames.insert(token1, new QString(token1));
						m_versionMap.insert(tokens.at(0).trimmed(), m_versionNames.value(token1));
						break;
				}
			} else {
				if ( catVerSwitch != 1 ) {
					if ( catverLine.indexOf(catStr) >= 0 )
						catVerSwitch = 1;
				} else if ( catVerSwitch != 2 ) {
					if ( catverLine.indexOf(verStr) >= 0 )
						catVerSwitch = 2;
				}
			}
		}
		catverIniFile.close();
	} else {
		log(tr("ERROR: can't open '%1' for reading").arg(m_fileName) + " - " + tr("no catver.ini data available"));
		return false;
	}
	log(tr("done (loading catver.ini data from '%1')").arg(m_fileName));
	log(tr("%1 category / %2 version records loaded").arg(m_categoryMap.count()).arg(m_versionMap.count()));
	return true;
}

void CatverIniOptimizer::optimize()
{
	QFile catverIniFile(m_fileName);
	if ( !catverIniFile.open(QIODevice::WriteOnly | QIODevice::Text) ) {
		log(tr("ERROR: can't open '%1' for writing").arg(m_fileName));
		return;
	}
	QTextStream ts(&catverIniFile);
	progressBar->setFormat(tr("Optimizing"));
	progressBar->setRange(0, m_categoryMap.count() + m_versionMap.count());
	progressBar->setValue(0);
	int count = 0, categoryChanges = 0, verAddedChanges = 0;
	// [Category]
	ts << "[Category]\n";
	QMapIterator<QString, QString *> catIter(m_categoryMap);
	QHash<QString, bool> replacedParentsHash;
	while ( catIter.hasNext() ) {
		catIter.next();
		if ( count % 10 == 0 )
			qApp->processEvents();
		progressBar->setValue(count++);
		QString machineName(catIter.key());
		QString machineCategory(*catIter.value());
		if ( machineName.isEmpty() || machineCategory.isEmpty() )
			continue;
		if ( !qmc2MachineListItemHash.contains(machineName) ) {
			log(QString("[Category] ") + tr("removed invalid set '%1' with category '%2'").arg(machineName).arg(machineCategory));
			categoryChanges++;
			continue;
		}
		QString parentName(qmc2ParentHash.value(machineName));
		if ( qmc2MachineList->isDevice(machineName) ) {
			log(QString("[Category] ") + tr("removed device set '%1' with category '%2'").arg(machineName).arg(machineCategory));
			continue;
		}
		if ( qmc2MachineList->isBios(machineName) ) {
			log(QString("[Category] ") + tr("removed BIOS set '%1' with category '%2'").arg(machineName).arg(machineCategory));
			continue;
		}
		if ( parentName.isEmpty() ) {
			ts << machineName << " = " << machineCategory << "\n";
			log(QString("[Category] ") + tr("kept parent set '%1' with category '%2'").arg(machineName).arg(machineCategory));
		} else {
			if ( !m_categoryMap.contains(parentName) && !replacedParentsHash.contains(parentName) ) {
				ts << parentName << " = " << machineCategory << "\n";
				replacedParentsHash.insert(parentName, true);
				log(QString("[Category] ") + tr("added parent set '%1' with category '%2' and removed clone set '%3'").arg(parentName).arg(machineCategory).arg(machineName));
				categoryChanges++;
			} else {
				log(QString("[Category] ") + tr("removed clone set '%1' with category '%2'").arg(machineName).arg(machineCategory));
				categoryChanges++;
			}
		}
	}
	// [VerAdded]
	ts << "\n[VerAdded]\n";
	QMapIterator<QString, QString *> verIter(m_versionMap);
	while ( verIter.hasNext() ) {
		verIter.next();
		if ( count % 10 == 0 )
			qApp->processEvents();
		progressBar->setValue(count++);
		QString machineName(verIter.key());
		QString machineVerAdded(*verIter.value());
		if ( machineName.isEmpty() || machineVerAdded.isEmpty() )
			continue;
		if ( !qmc2MachineListItemHash.contains(machineName) ) {
			log(QString("[VerAdded] ") + tr("removed invalid set '%1' with version '%2'").arg(machineName).arg(machineVerAdded));
			verAddedChanges++;
		} else {
			ts << machineName << " = " << machineVerAdded << "\n";
			log(QString("[VerAdded] ") + tr("kept %1 set '%2' with version '%3'").arg(qmc2ParentHash.value(machineName).isEmpty() ? tr("parent") : tr("clone")).arg(machineName).arg(machineVerAdded));
		}
	}
	log(tr("changes to categories / versions: %1 / %2").arg(categoryChanges).arg(verAddedChanges));
	catverIniFile.close();
	progressBar->setValue(0);
	progressBar->setFormat(tr("Idle"));
}

void CatverIniOptimizer::on_pushButtonOptimize_clicked()
{
	pushButtonClose->setEnabled(false);
	pushButtonOptimize->setEnabled(false);
	plainTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	plainTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	plainTextEdit->clear();
	log(tr("optimizer started"));
	if ( loadCatverIni() )
		optimize();
	log(tr("optimizer ended"));
	plainTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	plainTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	pushButtonOptimize->setEnabled(true);
	pushButtonClose->setEnabled(true);
}

void CatverIniOptimizer::log(const QString &message)
{
	plainTextEdit->appendPlainText(QDateTime::currentDateTime().toString("hh:mm:ss.zzz") + ": " + message);
	plainTextEdit->horizontalScrollBar()->setValue(plainTextEdit->horizontalScrollBar()->minimum());
	plainTextEdit->verticalScrollBar()->setValue(plainTextEdit->verticalScrollBar()->maximum());
}

void CatverIniOptimizer::showEvent(QShowEvent *e)
{
	restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/CatverIniOptimizer/Geometry", QByteArray()).toByteArray());
	QDialog::showEvent(e);
}

void CatverIniOptimizer::hideEvent(QHideEvent *e)
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/CatverIniOptimizer/Geometry", saveGeometry());
	QDialog::hideEvent(e);
}

void CatverIniOptimizer::closeEvent(QCloseEvent *e)
{
	if ( pushButtonClose->isEnabled() )
		e->accept();
	else
		e->ignore();
}
