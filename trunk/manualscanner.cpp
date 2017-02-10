#include <QScrollBar>
#include <QDateTime>
#include <QTreeWidgetItem>
#include <QFileInfo>
#include <QDir>
#include <QHash>
#include <QHashIterator>
#if defined(QMC2_OS_WIN)
#include <windows.h>
#endif

#include "manualscanner.h"
#include "machinelist.h"
#include "settings.h"
#include "macros.h"

extern Settings *qmc2Config;
extern MachineList *qmc2MachineList;
extern QHash<QString, QTreeWidgetItem *> qmc2MachineListItemHash;

#define userDataDb	qmc2MachineList->userDataDb()

ManualScanner::ManualScanner(int mode, QWidget *parent) :
	QDialog(parent),
	m_mode(mode)
{
	setupUi(this);
	switch ( m_mode ) {
		case QMC2_MANUALSCANNER_MODE_SYSTEMS:
			setWindowTitle(tr("System manual scanner"));
			m_settingsKey = "SystemManualScanner";
			break;
		case QMC2_MANUALSCANNER_MODE_SOFTWARE:
			setWindowTitle(tr("Software manual scanner"));
			m_settingsKey = "SoftwareManualScanner";
			break;
	}
	log(tr("click 'scan now' to start"));
}

ManualScanner::~ManualScanner()
{
	// NOP
}

void ManualScanner::on_pushButtonScanNow_clicked()
{
	pushButtonClose->setEnabled(false);
	pushButtonScanNow->setEnabled(false);
	plainTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	plainTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	plainTextEdit->clear();
	log(tr("scanner started"));
	scan();
	log(tr("scanner ended"));
	plainTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	plainTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	pushButtonScanNow->setEnabled(true);
	pushButtonClose->setEnabled(true);
}

void ManualScanner::log(const QString &message)
{
	plainTextEdit->appendPlainText(QDateTime::currentDateTime().toString("hh:mm:ss.zzz") + ": " + message);
	plainTextEdit->horizontalScrollBar()->setValue(plainTextEdit->horizontalScrollBar()->minimum());
	plainTextEdit->verticalScrollBar()->setValue(plainTextEdit->verticalScrollBar()->maximum());
}

void ManualScanner::showEvent(QShowEvent *e)
{
	restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + QString("Layout/%1/Geometry").arg(m_settingsKey), QByteArray()).toByteArray());
	QDialog::showEvent(e);
}

void ManualScanner::hideEvent(QHideEvent *e)
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + QString("Layout/%1/Geometry").arg(m_settingsKey), saveGeometry());
	QDialog::hideEvent(e);
}

void ManualScanner::closeEvent(QCloseEvent *e)
{
	if ( pushButtonClose->isEnabled() )
		e->accept();
	else
		e->ignore();
}

void ManualScanner::scan()
{
	QStringList pathList;
	switch ( m_mode ) {
		case QMC2_MANUALSCANNER_MODE_SYSTEMS:
			pathList = qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SystemManualFolder").toString().split(';', QString::SkipEmptyParts);
			break;
		case QMC2_MANUALSCANNER_MODE_SOFTWARE:
			pathList = qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareManualFolder").toString().split(';', QString::SkipEmptyParts);
			break;
	}
	QHash<QString, QStringList> pdfManualHash;
	foreach (QString path, pathList) {
		path = QDir::cleanPath(path);
		QStringList fileList;
		log(tr("reading file list for path '%1'").arg(path));
		qApp->processEvents();
		recursiveFileList(path, &fileList);
		log(tr("scanning %n file(s)", "", fileList.count()));
		quint64 numIgnored = 0, numInaccessible = 0, numStoredInDb = 0, fileCount = 0;
		foreach (QString file, fileList) {
			QFileInfo fi(file);
			if ( fi.suffix().toLower() == "pdf" ) {
				if ( fi.isReadable() ) {
					if ( m_mode == QMC2_MANUALSCANNER_MODE_SYSTEMS ) {
						QString baseName(fi.baseName().toLower());
						if ( qmc2MachineListItemHash.contains(baseName) ) {
							log(tr("adding '%1' as a manual for '%2'").arg(file).arg(baseName));
							numStoredInDb++;
							pdfManualHash[baseName] << file;
						} else {
							log(tr("ignoring '%1' because its name doesn't match any machine").arg(file));
							numIgnored++;
						}
					} else {
						// FIXME: software-manuals
					}
				} else {
					log(tr("can't read '%1' - please check access permissions").arg(file));
					numInaccessible++;
				}
			} else {
				log(tr("ignoring '%1' because it doesn't appear to be a PDF document").arg(file));
				numIgnored++;
			}
			if ( fileCount++ % QMC2_MANUALSCANNER_SCAN_RESPONSE == 0 )
				qApp->processEvents();
		}
		log(tr("scan statistics for path '%1'").arg(path) + ": " + tr("%n file(s) scanned", "", fileList.count()) + ", " + tr("%n manual(s) stored in database", "", numStoredInDb) + ", " + tr("%n file(s) ignored", "", numIgnored) + ", " + tr("%n file(s) inaccessible", "", numInaccessible));
	}
	log(tr("updating the database"));
	switch ( m_mode ) {
		case QMC2_MANUALSCANNER_MODE_SYSTEMS:
			userDataDb->recreateSystemManualTable();
			break;
		case QMC2_MANUALSCANNER_MODE_SOFTWARE:
			userDataDb->recreateSoftwareManualTable();
			break;
	}
	userDataDb->beginTransaction();
	QHashIterator<QString, QStringList> pdfManualHashIterator(pdfManualHash);
	quint64 fileCount = 0;
	while ( pdfManualHashIterator.hasNext() ) {
		pdfManualHashIterator.next();
		switch ( m_mode ) {
			case QMC2_MANUALSCANNER_MODE_SYSTEMS:
				userDataDb->setSystemManualPath(pdfManualHashIterator.key(), pdfManualHashIterator.value().join(";"));
				break;
			case QMC2_MANUALSCANNER_MODE_SOFTWARE:
				// FIXME
				break;
		}
		if ( fileCount++ % QMC2_MANUALSCANNER_DB_COMMIT == 0 ) {
			userDataDb->commitTransaction();
			qApp->processEvents();
			userDataDb->beginTransaction();
		}
	}
	userDataDb->commitTransaction();
	log(tr("database update finished"));
}

void ManualScanner::recursiveFileList(const QString &sDir, QStringList *fileNames)
{
#if defined(QMC2_OS_WIN)
	WIN32_FIND_DATA ffd;
	QString dirName(QDir::toNativeSeparators(QDir::cleanPath(sDir + "/*")));
#ifdef UNICODE
	HANDLE hFind = FindFirstFile((TCHAR *)dirName.utf16(), &ffd);
#else
	HANDLE hFind = FindFirstFile((TCHAR *)dirName.toUtf8().constData(), &ffd);
#endif
	if ( hFind != INVALID_HANDLE_VALUE ) {
		do {
#ifdef UNICODE
			QString fName(QString::fromUtf16((ushort*)ffd.cFileName));
#else
			QString fName(QString::fromLocal8Bit(ffd.cFileName));
#endif
			if ( ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
				if ( fName != ".." && fName != "." )
					recursiveFileList(sDir + "/" + fName, fileNames);
			} else {
				fileNames->append(sDir + "/" + fName);
				if ( fileNames->count() % QMC2_MANUALSCANNER_SCAN_RESPONSE == 0 )
					qApp->processEvents();
			}
		} while ( FindNextFile(hFind, &ffd) != 0 );
	}
#else
	QDir dir(sDir);
	foreach (QFileInfo info, dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::Hidden | QDir::System)) {
		QString path(info.filePath());
		if ( info.isDir() ) {
			// directory recursion
			if ( info.fileName() != ".." && info.fileName() != "." )
				recursiveFileList(path, fileNames);
		} else {
			fileNames->append(path);
			if ( fileNames->count() % QMC2_MANUALSCANNER_SCAN_RESPONSE == 0 )
				qApp->processEvents();
		}
	}
#endif
}
