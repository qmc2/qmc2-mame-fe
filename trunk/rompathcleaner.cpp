#include <QApplication>
#include <QFileDialog>
#include <QDateTime>
#include <QFontMetrics>
#include <QFont>

#include "qmc2main.h"
#include "options.h"
#include "settings.h"
#include "rompathcleaner.h"

extern MainWindow *qmc2MainWindow;
extern Options *qmc2Options;
extern Settings *qmc2Config;

RomPathCleaner::RomPathCleaner(const QString &settingsKey, QWidget *parent) :
	QWidget(parent),
	m_romPathCleanerThread(0),
	m_settingsKey(settingsKey)
{
	setupUi(this);
	comboBoxCheckedPath->insertSeparator(QMC2_RPC_PATH_INDEX_SEPARATOR);

	QFont logFont;
	logFont.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/LogFont").toString());
	plainTextEditLog->setFont(logFont);
	spinBoxMaxLogSize->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/MaxLogSize", 10000).toInt());

	m_romPathCleanerThread = new RomPathCleanerThread(this);
	connect(romPathCleanerThread(), SIGNAL(log(const QString &)), this, SLOT(log(const QString &)));
}

RomPathCleaner::~RomPathCleaner()
{
	if ( romPathCleanerThread() )
		delete romPathCleanerThread();
}

void RomPathCleaner::adjustIconSizes()
{
	QFont f(qApp->font());
	QFontMetrics fm(f);
	QSize iconSize(fm.height() - 2, fm.height() - 2);
	pushButtonStartStop->setIconSize(iconSize);
	comboBoxCheckedPath->setIconSize(iconSize);
}

void RomPathCleaner::log(const QString &message)
{
	if ( checkBoxEnableLog->isChecked() )
		plainTextEditLog->appendPlainText(QDateTime::currentDateTime().toString("hh:mm:ss.zzz") + ": " + message);
}

void RomPathCleaner::on_pushButtonStartStop_clicked()
{
	// FIXME
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("FIXME: RomPathCleaner::on_pushButtonStartStop_clicked(): not implemented yet!"));
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

RomPathCleanerThread::RomPathCleanerThread(QObject *parent) :
	QThread(parent),
	m_exit(false),
	m_active(false),
	m_waiting(false),
	m_paused(false)
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
	while ( !m_exit ) {
		emit log(tr("waiting for work"));
		m_mutex.lock();
		m_waiting = true;
		m_active = m_paused = false;
		m_waitCondition.wait(&m_mutex);
		m_active = true;
		m_waiting = false;
		m_mutex.unlock();
		// FIXME
	}
	emit log(tr("cleaner thread ended"));
}
