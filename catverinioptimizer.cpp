#include <QFont>
#include <QChar>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QDateTime>
#include <QTreeWidgetItem>
#include <QApplication>

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

void CatverIniOptimizer::loadCatverIni()
{
	clearCategoryNames();
	m_categoryHash.clear();
	clearVersionNames();
	m_versionHash.clear();
	log(tr("loading catver.ini data from '%1'").arg(m_fileName));
	QFile catverIniFile(m_fileName);
	if ( catverIniFile.open(QIODevice::ReadOnly | QIODevice::Text) ) {
		QTextStream tsCatverIni(&catverIniFile);
		bool categoryDone = false, versionDone = false;
		int lineCounter = 0;
		char catVerSwitch = 0;
		QChar splitChar('='), dotChar('.'), zeroChar('0');
		QString catStr("[Category]"), verStr("[VerAdded]");
		while ( !tsCatverIni.atEnd() ) {
			QString catverLine(tsCatverIni.readLine());
			if ( catverLine.isEmpty() )
				continue;
			if ( !categoryDone && catVerSwitch != 1 ) {
				if ( catverLine.indexOf(catStr) >= 0 ) {
					categoryDone = true;
					catVerSwitch = 1;
				}
			}
			if ( !versionDone && catVerSwitch != 2 ) {
				if ( catverLine.indexOf(verStr) >= 0 ) {
					versionDone = true;
					catVerSwitch = 2;
				}
			}
			QStringList tokens(catverLine.split(splitChar, QString::SkipEmptyParts));
			if ( tokens.count() > 1 ) {
				QString token1(tokens.at(1).trimmed());
				switch ( catVerSwitch ) {
					case 1: // category
						if ( !m_categoryNames.contains(token1) )
							m_categoryNames.insert(token1, new QString(token1));
						m_categoryHash.insert(tokens.at(0).trimmed(), m_categoryNames.value(token1));
						break;
					case 2: // version
						if ( token1.startsWith(dotChar) )
							token1.prepend(zeroChar);
						if ( !m_versionNames.contains(token1) )
							m_versionNames.insert(token1, new QString(token1));
						m_versionHash.insert(tokens.at(0).trimmed(), m_versionNames.value(token1));
						break;
				}
			}
		}
		catverIniFile.close();
	} else
		log(tr("ERROR: can't open '%1' for reading -- no catver.ini data available").arg(m_fileName));
	log(tr("done (loading catver.ini data from '%1')").arg(m_fileName));
	log(tr("%1 category / %2 version records loaded").arg(m_categoryHash.count()).arg(m_versionHash.count()));
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
	progressBar->setRange(0, m_categoryHash.count() + m_versionHash.count());
	progressBar->setValue(0);
	int count = 0;
	// [Category]
	ts << "[Category]\n";
	QHashIterator<QString, QString *> catIter(m_categoryHash);
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
			continue;
		}
		QString parentName(qmc2ParentHash.value(machineName));
		if ( parentName.isEmpty() ) {
			ts << machineName << " = " << machineCategory << "\n";
			log(QString("[Category] ") + tr("kept parent set '%1' with category '%2'").arg(machineName).arg(machineCategory));
		} else {
			if ( !m_categoryHash.contains(parentName) && !replacedParentsHash.contains(parentName) ) {
				ts << parentName << " = " << machineCategory << "\n";
				replacedParentsHash.insert(parentName, true);
				log(QString("[Category] ") + tr("added parent set '%1' with category '%2' and removed clone set '%3'").arg(parentName).arg(machineCategory).arg(machineName));
			} else
				log(QString("[Category] ") + tr("removed clone set '%1' with category '%2'").arg(machineName).arg(machineCategory));
		}
	}
	// [VerAdded]
	ts << "\n[VerAdded]\n";
	QHashIterator<QString, QString *> verIter(m_versionHash);
	while ( verIter.hasNext() ) {
		verIter.next();
		if ( count % 10 == 0 )
			qApp->processEvents();
		progressBar->setValue(count++);
		QString machineName(verIter.key());
		QString machineVerAdded(*verIter.value());
		if ( machineName.isEmpty() || machineVerAdded.isEmpty() )
			continue;
		if ( !qmc2MachineListItemHash.contains(machineName) )
			log(QString("[VerAdded] ") + tr("removed invalid set '%1' with version '%2'").arg(machineName).arg(machineVerAdded));
		else {
			ts << machineName << " = " << machineVerAdded << "\n";
			log(QString("[VerAdded] ") + tr("kept %1 set '%2' with version '%3'").arg(qmc2ParentHash.value(machineName).isEmpty() ? tr("parent") : tr("clone")).arg(machineName).arg(machineVerAdded));
		}
	}
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
	loadCatverIni();
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

void CatverIniOptimizer::closeEvent(QCloseEvent *e)
{
	if ( pushButtonClose->isEnabled() )
		e->accept();
	else
		e->ignore();
}
