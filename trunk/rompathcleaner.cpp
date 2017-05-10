#include <QApplication>
#include <QFileDialog>
#include <QDateTime>
#include <QFontMetrics>
#include <QFont>
#include <QTime>
#include <QTest>
#include <QDir>
#include <QFileInfo>
#if defined(QMC2_OS_WIN)
#include <windows.h>
#endif

#include "qmc2main.h"
#include "options.h"
#include "settings.h"
#include "rompathcleaner.h"

extern MainWindow *qmc2MainWindow;
extern Options *qmc2Options;
extern Settings *qmc2Config;

RomPathCleaner::RomPathCleaner(const QString &settingsKey, QWidget *parent) :
	QWidget(parent),
	m_cleanerThread(0),
	m_settingsKey(settingsKey)
{
	setupUi(this);
	comboBoxCheckedPath->insertSeparator(QMC2_RPC_PATH_INDEX_SEPARATOR);
	pushButtonPauseResume->setVisible(false);

	QFont logFont;
	logFont.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/LogFont").toString());
	plainTextEditLog->setFont(logFont);
	comboBoxModeSwitch->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/Mode", QMC2_RPC_MODE_INDEX_DRYRUN).toInt());
	spinBoxMaxLogSize->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/MaxLogSize", 10000).toInt());
	checkBoxEnableLog->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/EnableLog", true).toBool());

	m_cleanerThread = new RomPathCleanerThread(this);
	connect(cleanerThread(), SIGNAL(log(const QString &)), this, SLOT(log(const QString &)));
	connect(cleanerThread(), SIGNAL(checkStarted()), this, SLOT(cleanerThread_checkStarted()));
	connect(cleanerThread(), SIGNAL(checkFinished()), this, SLOT(cleanerThread_checkFinished()));
	connect(cleanerThread(), SIGNAL(checkPaused()), this, SLOT(cleanerThread_checkPaused()));
	connect(cleanerThread(), SIGNAL(checkResumed()), this, SLOT(cleanerThread_checkResumed()));
	connect(cleanerThread(), SIGNAL(progressTextChanged(const QString &)), this, SLOT(cleanerThread_progressTextChanged(const QString &)));
	connect(cleanerThread(), SIGNAL(progressRangeChanged(int, int)), this, SLOT(cleanerThread_progressRangeChanged(int, int)));
	connect(cleanerThread(), SIGNAL(progressChanged(int)), this, SLOT(cleanerThread_progressChanged(int)));
	connect(cleanerThread(), SIGNAL(statusUpdated(quint64, quint64, quint64, quint64, quint64)), this, SLOT(cleanerThread_statusUpdated(quint64, quint64, quint64, quint64, quint64)));
}

RomPathCleaner::~RomPathCleaner()
{
	hideEvent(0);
	if ( cleanerThread() )
		delete cleanerThread();
}

void RomPathCleaner::adjustIconSizes()
{
	QFont f(qApp->font());
	QFontMetrics fm(f);
	QSize iconSize(fm.height() - 2, fm.height() - 2);
	comboBoxCheckedPath->setIconSize(iconSize);
	pushButtonStartStop->setIconSize(iconSize);
	pushButtonPauseResume->setIconSize(iconSize);
}

void RomPathCleaner::log(const QString &message)
{
	if ( checkBoxEnableLog->isChecked() )
		plainTextEditLog->appendPlainText(QDateTime::currentDateTime().toString("hh:mm:ss.zzz") + ": " + message);
}

void RomPathCleaner::cleanerThread_checkStarted()
{
	pushButtonStartStop->setIcon(QIcon(QString::fromUtf8(":/data/img/halt.png")));
	pushButtonStartStop->setText(tr("Stop check"));
	pushButtonPauseResume->setText(tr("Pause"));
	pushButtonPauseResume->show();
	pushButtonStartStop->setEnabled(true);
	pushButtonPauseResume->setEnabled(true);
	labelCheckedPath->setEnabled(false);
	comboBoxCheckedPath->setEnabled(false);
	labelModeSwitch->setEnabled(false);
	comboBoxModeSwitch->setEnabled(false);
}

void RomPathCleaner::cleanerThread_checkFinished()
{
	pushButtonStartStop->setIcon(QIcon(QString::fromUtf8(":/data/img/refresh.png")));
	pushButtonStartStop->setText(tr("Start check"));
	pushButtonPauseResume->hide();
	pushButtonStartStop->setEnabled(true);
	pushButtonPauseResume->setEnabled(true);
	labelCheckedPath->setEnabled(true);
	comboBoxCheckedPath->setEnabled(true);
	labelModeSwitch->setEnabled(true);
	comboBoxModeSwitch->setEnabled(true);
}

void RomPathCleaner::cleanerThread_checkPaused()
{
	pushButtonPauseResume->setText(tr("Resume"));
	pushButtonPauseResume->setEnabled(true);
}

void RomPathCleaner::cleanerThread_checkResumed()
{
	pushButtonPauseResume->setText(tr("Pause"));
	pushButtonPauseResume->setEnabled(true);
}

void RomPathCleaner::cleanerThread_progressTextChanged(const QString &text)
{
	progressBar->setFormat(text);
}

void RomPathCleaner::cleanerThread_progressRangeChanged(int min, int max)
{
	progressBar->setRange(min, max);
}

void RomPathCleaner::cleanerThread_progressChanged(int progress)
{
	progressBar->setValue(progress);
}

void RomPathCleaner::cleanerThread_statusUpdated(quint64 filesProcessed, quint64 renamedFiles, quint64 obsoleteROMs, quint64 obsoleteDisks, quint64 invalidFiles)
{
	QString statusString("<table border=\"0\" cellpadding=\"0\" cellspacing=\"4\" width=\"100%\"><tr>");
	statusString += "<td nowrap align=\"left\"><b>" + tr("Files processed") + ":</b></td><td nowrap align=\"right\">" + QString::number(filesProcessed) + "</td>";
	statusString += "<td nowrap align=\"center\" width=\"1%\">|</td>";
	statusString += "<td nowrap align=\"left\"><b>" + tr("Renamed files") + ":</b></td><td nowrap align=\"right\">" + QString::number(renamedFiles) + "</td>";
	statusString += "<td nowrap align=\"center\" width=\"1%\">|</td>";
	statusString += "<td nowrap align=\"left\"><b>" + tr("Obsolete ROMs / disks") + ":</b></td><td nowrap align=\"right\">" + QString::number(obsoleteROMs) + " / " + QString::number(obsoleteDisks) + "</td>";
	statusString += "<td nowrap align=\"right\" width=\"1%\">|</td>";
	statusString += "<td nowrap align=\"left\"><b>" + tr("Invalid files") + ":</b></td><td nowrap align=\"right\">" + QString::number(invalidFiles) + "</td>";
	statusString += "</tr></table>";
	labelStatus->setText(statusString);
}

void RomPathCleaner::on_comboBoxCheckedPath_activated(int index)
{
	if ( index == QMC2_RPC_PATH_INDEX_SELECT ) {
		QString path(QFileDialog::getExistingDirectory(this, tr("Select path to be checked"), QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog)));
		if ( !path.isEmpty() ) {
			while ( comboBoxCheckedPath->count() > QMC2_RPC_PATH_INDEX_CUSTOMPATH )
				comboBoxCheckedPath->removeItem(comboBoxCheckedPath->count() - 1);
			comboBoxCheckedPath->insertItem(QMC2_RPC_PATH_INDEX_CUSTOMPATH, path);
			comboBoxCheckedPath->setCurrentIndex(QMC2_RPC_PATH_INDEX_CUSTOMPATH);
		} else
			comboBoxCheckedPath->setCurrentIndex(QMC2_RPC_PATH_INDEX_ROMPATH);
	}
}

void RomPathCleaner::on_pushButtonStartStop_clicked()
{
	pushButtonStartStop->setEnabled(false);
	pushButtonStartStop->update();
	pushButtonPauseResume->setEnabled(false);
	pushButtonPauseResume->update();
	qApp->processEvents();
	if ( cleanerThread()->active() )
		cleanerThread()->requestStop();
	else {
		switch ( comboBoxCheckedPath->currentIndex() ) {
			case QMC2_RPC_PATH_INDEX_CUSTOMPATH:
				cleanerThread()->setCheckedPaths(QStringList() << comboBoxCheckedPath->currentText());
				break;
			default:
			case QMC2_RPC_PATH_INDEX_ROMPATH:
				if ( qmc2Config->contains(QMC2_EMULATOR_PREFIX + "Configuration/Global/rompath") )
					cleanerThread()->setCheckedPaths(qmc2Config->value(QMC2_EMULATOR_PREFIX + "Configuration/Global/rompath").toString().split(';', QString::SkipEmptyParts));
				else
					cleanerThread()->setCheckedPaths(QStringList() << "roms");
				break;
		}
		plainTextEditLog->clear();
		cleanerThread()->waitCondition().wakeAll();
	}
}

void RomPathCleaner::on_pushButtonPauseResume_clicked()
{
	pushButtonPauseResume->setEnabled(false);
	if ( cleanerThread()->paused() )
		QTimer::singleShot(0, cleanerThread(), SLOT(resume()));
	else
		QTimer::singleShot(0, cleanerThread(), SLOT(pause()));
}

void RomPathCleaner::on_spinBoxMaxLogSize_valueChanged(int value)
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/MaxLogSize", value);
	plainTextEditLog->setMaximumBlockCount(value);
}

void RomPathCleaner::hideEvent(QHideEvent *e)
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/Mode", comboBoxModeSwitch->currentIndex());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/EnableLog", checkBoxEnableLog->isChecked());
}

RomPathCleanerThread::RomPathCleanerThread(QObject *parent) :
	QThread(parent),
	m_exit(false),
	m_stop(false),
	m_active(false),
	m_waiting(false),
	m_paused(false),
	m_filesProcessed(0),
	m_renamedFiles(0),
	m_obsoleteROMs(0),
	m_obsoleteDisks(0),
	m_invalidFiles(0)
{
	start();
}

RomPathCleanerThread::~RomPathCleanerThread()
{
	requestExit();
	m_waitCondition.wakeAll();
	wait();
}

void RomPathCleanerThread::run()
{
	emit log(tr("cleaner thread started"));
	while ( !m_exit && !m_stop ) {
		emit log(tr("waiting for work"));
		m_mutex.lock();
		m_waiting = true;
		m_active = m_paused = false;
		m_checkedPaths.clear();
		m_waitCondition.wait(&m_mutex);
		m_active = true;
		m_waiting = m_stop = false;
		m_mutex.unlock();
		if ( !m_exit && !m_stop ) {
			m_filesProcessed = m_renamedFiles = m_obsoleteROMs = m_obsoleteDisks = m_invalidFiles = 0;
			emit statusUpdated(m_filesProcessed, m_renamedFiles, m_obsoleteROMs, m_obsoleteDisks, m_invalidFiles);
			emit log(tr("check started"));
			emit checkStarted();
			QTime checkTimer, elapsedTime(0, 0, 0, 0);
			checkTimer.start();
			int pathCount = 1;
			foreach (QString path, m_checkedPaths) {
				emit progressTextChanged(tr("Cleaning up path %1 / %2").arg(pathCount++).arg(m_checkedPaths.count()));
				emit log(tr("checking path '%1'").arg(path));
				QStringList fileList;
				emit log(tr("reading directory tree"));
				recursiveFileList(path, &fileList);
				emit log(tr("path contains %n file(s)", "", fileList.count()));
				emit progressRangeChanged(0, fileList.count());
				emit progressChanged(0);
				int index = 0;
				while ( !m_exit && !m_stop && index < fileList.count()) {
					if ( m_paused ) {
						emit log(tr("check paused"));
						emit checkPaused();
						while ( m_paused && !m_stop && !m_exit )
							QTest::qWait(100);
						if ( !m_paused ) {
							emit log(tr("check resumed"));
							emit checkResumed();
						}
					}
					if ( !m_paused && !m_stop && !m_exit ) {
						QString filePath(fileList.at(index));
						// FIXME
					}
					if ( m_filesProcessed % QMC2_RPC_STATUS_UPDATE == 0 ) {
						emit statusUpdated(m_filesProcessed, m_renamedFiles, m_obsoleteROMs, m_obsoleteDisks, m_invalidFiles);
						emit progressChanged(index);
						QTest::qWait(1);
					}
					m_filesProcessed++;
					index++;
				}
				emit log(tr("done (checking path '%1')").arg(path));
			}
			emit statusUpdated(m_filesProcessed, m_renamedFiles, m_obsoleteROMs, m_obsoleteDisks, m_invalidFiles);
			elapsedTime = elapsedTime.addMSecs(checkTimer.elapsed());
			emit log(tr("check finished") + " - " + tr("total check time = %1, files processed = %2, renamed files = %3, obsolete ROMs = %4, obsolete disks = %5, invalid files = %6").arg(elapsedTime.toString("hh:mm:ss.zzz")).arg(m_filesProcessed).arg(m_renamedFiles).arg(m_obsoleteROMs).arg(m_obsoleteDisks).arg(m_invalidFiles));
			emit checkFinished();
		}
		emit progressRangeChanged(0, 100);
		emit progressChanged(0);
		emit progressTextChanged(tr("Idle"));
		m_stop = false;
	}
	emit log(tr("cleaner thread ended"));
}

void RomPathCleanerThread::recursiveFileList(const QString &sDir, QStringList *fileNames)
{
	if ( m_exit || m_stop )
		return;
#if defined(QMC2_OS_WIN)
	WIN32_FIND_DATA ffd;
	QString dirName(QDir::toNativeSeparators(QDir::cleanPath(sDir + "/*")));
#ifdef UNICODE
	HANDLE hFind = FindFirstFile((TCHAR *)dirName.utf16(), &ffd);
#else
	HANDLE hFind = FindFirstFile((TCHAR *)dirName.toUtf8().constData(), &ffd);
#endif
	if ( !exitThread && !stopScan && hFind != INVALID_HANDLE_VALUE ) {
		do {
#ifdef UNICODE
			QString fName(QString::fromUtf16((ushort*)ffd.cFileName));
#else
			QString fName(QString::fromLocal8Bit(ffd.cFileName));
#endif
			if ( ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
				if ( fName != ".." && fName != "." )
					recursiveFileList(sDir + "/" + fName, fileNames);
			} else
				fileNames->append(sDir + "/" + fName);
		} while ( !m_exit && !m_stop && FindNextFile(hFind, &ffd) != 0 );
	}
#else
	QDir dir(sDir);
	foreach (QFileInfo info, dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::Hidden | QDir::System)) {
		if ( m_exit || m_stop )
			break;
		QString path(info.filePath());
		if ( info.isDir() ) {
			// directory recursion
			if ( info.fileName() != ".." && info.fileName() != "." )
				recursiveFileList(path, fileNames);
		} else
			fileNames->append(path);
	}
#endif
}
