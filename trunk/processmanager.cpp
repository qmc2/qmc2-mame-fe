#include <qglobal.h>
#include <QtGui>
#include <QFile>
#include <QSocketNotifier>
#include <QMap>

#include "settings.h"
#include "processmanager.h"
#include "qmc2main.h"
#include "embedder.h"
#include "youtubevideoplayer.h"
#include "macros.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern bool qmc2GuiReady;
extern bool qmc2StartEmbedded;
extern QString qmc2DriverName;
extern QMap<QWidget *, Qt::WindowStates> qmc2AutoMinimizedWidgets;
#if defined(QMC2_YOUTUBE_ENABLED)
extern YouTubeVideoPlayer *qmc2YouTubeWidget;
#endif
extern Settings *qmc2Config;
extern QFile *qmc2FifoFile;
extern QSocketNotifier *qmc2FifoNotifier;

ProcessManager::ProcessManager(QWidget *parent)
	: QObject(parent)
{
	procCount = 0;
#if QMC2_USE_PHONON_API
	musicWasPlaying = sentPlaySignal = false;
#endif
#if defined(QMC2_YOUTUBE_ENABLED)
	videoWasPlaying = true;
#endif
	launchForeignID = false;
}

int ProcessManager::start(QString &command, QStringList &arguments, bool autoConnect, QString workDir, QStringList softwareLists, QStringList softwareNames)
{
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/OneEmulatorOnly", false).toBool() ) {
		foreach (int index, procMap) {
			QProcess *proc = process(index);
			if ( !proc )
				continue;
			bool finished = false;
			proc->terminate();
			for (int i = 0; i < 1000 / QMC2_PROCESS_POLL_TIME && !finished; i++) {
				finished = proc->waitForFinished(QMC2_PROCESS_POLL_TIME);
				qApp->processEvents();
			}
			if ( !proc->waitForFinished(0) )
				proc->kill();
		}
		qApp->processEvents();
	}
	launchForeignID = qmc2MainWindow->launchForeignID;
	QProcess *proc = new QProcess(this);
	if ( !workDir.isEmpty() ) {
		QFileInfo fi(workDir);
		if ( fi.exists() ) {
			if ( fi.isDir () )
				proc->setWorkingDirectory(workDir);
			else
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: ProcessManager::start(): the specified working directory '%1' is not a directory -- ignored").arg(workDir));
		} else
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: ProcessManager::start(): the specified working directory '%1' does not exist -- ignored").arg(workDir));
	}
#if defined(QMC2_OS_UNIX)
	// we use a (session-)unique ID in the WM_CLASS property to identify the window later...
	QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
	env.insert("SDL_VIDEO_X11_WMCLASS", QString("QMC2-MAME-ID-%1").arg(procCount));
	proc->setProcessEnvironment(env);
#endif
	if ( autoConnect ) {
#if defined(QMC2_OS_WIN)
		bool snapnameActive = false;
#endif
		QStringList fullArgs = QStringList() << command;
		for (int i = 0; i < arguments.count(); i++) {
			QString arg = arguments[i];
			if ( !launchForeignID ) {
#if defined(QMC2_OS_WIN)
				if ( arg == "-snapname" )
					snapnameActive = true;
				if ( arg.contains(QRegExp("(\\s|\\\\|\\(|\\)|\\/|\\;)")) ) {
					arg = "\"" + arg + "\"";
					if ( snapnameActive ) {
						if ( arg.contains("/") )
							arg.replace("/", "$QMC2FWSL$");
						snapnameActive = false;
					}
				}
#else
				if ( arg.contains(QRegExp("(\\s|\\\\|\\(|\\)|\\;)")) )
					arg = "\"" + arg + "\"";
#endif
			}
			fullArgs << arg;
		}
		loggedCommandLine = fullArgs.join(" ");
#if defined(QMC2_OS_WIN)
		QString emuCommandLine = loggedCommandLine;
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("starting emulator #%1, command = %2").arg(procCount).arg(emuCommandLine.replace('/', '\\').replace("$QMC2FWSL$", "/")));
#else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("starting emulator #%1, command = %2").arg(procCount).arg(loggedCommandLine));
#endif
		connect(proc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(error(QProcess::ProcessError)));
		connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finished(int, QProcess::ExitStatus)));
		connect(proc, SIGNAL(readyReadStandardOutput()), this, SLOT(readyReadStandardOutput()));
		connect(proc, SIGNAL(readyReadStandardError()), this, SLOT(readyReadStandardError()));
		connect(proc, SIGNAL(started()), this, SLOT(started()));
		connect(proc, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(stateChanged(QProcess::ProcessState)));
	}
	procMap[proc] = procCount++;
	softwareListsMap[proc] = softwareLists;
	softwareNamesMap[proc] = softwareNames;
	proc->start(command, arguments);
	return procCount - 1;
}

QProcess *ProcessManager::process(ushort index)
{
	QList<QProcess *> vl = procMap.keys(index);
	if ( vl.count() > 0 )
		return vl.at(0);
	else
		return NULL;
}

void ProcessManager::terminate(QProcess *proc)
{
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("terminating emulator #%1, PID = %2").arg(procMap[proc]).arg((quint64)proc->pid()));
	proc->terminate();
}

void ProcessManager::terminate(ushort index)
{
	QProcess *proc = process(index);
	if ( proc ) {
#if defined(QMC2_OS_WIN)
		Embedder *embedder = NULL;
		for (int j = 0; j < qmc2MainWindow->tabWidgetEmbeddedEmulators->count() && embedder == NULL; j++) {
			if ( qmc2MainWindow->tabWidgetEmbeddedEmulators->tabText(j).startsWith(QString("#%1 - ").arg(index)) )
				embedder = (Embedder *)qmc2MainWindow->tabWidgetEmbeddedEmulators->widget(j);
		}
		if ( embedder )
			embedder->release();
#endif
		terminate(proc);
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: ProcessManager::terminate(ushort index = %1): trying to terminate a null process").arg(index));
}

void ProcessManager::kill(QProcess *proc)
{
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("killing emulator #%1, PID = %2").arg(procMap[proc]).arg((quint64)proc->pid()));
	proc->kill();
}

void ProcessManager::kill(ushort index)
{
	QProcess *proc = process(index);
	if ( proc )
		kill(proc);
	else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: ProcessManager::kill(ushort index = %1): trying to kill a null process").arg(index));
}

void ProcessManager::readyReadStandardOutput()
{
	QProcess *proc = (QProcess *)sender();
	QString s = proc->readAllStandardOutput();
#if defined(QMC2_OS_WIN)
	s.replace("\r\n", "\n"); // convert WinDOS's "0x0D 0x0A" to just "0x0A" 
#endif
	QStringList sl = s.split("\n");
	int i;
	for (i = 0; i < sl.count(); i++) {
		s = sl[i];
		if ( !s.isEmpty() )
			qmc2MainWindow->log(QMC2_LOG_EMULATOR, tr("stdout[#%1]:").arg(procMap[proc]) + " " + s);
	}
}

void ProcessManager::readyReadStandardError()
{
	QProcess *proc = (QProcess *)sender();
	QString s = proc->readAllStandardError();
#if defined(QMC2_OS_WIN)
	s.replace("\r\n", "\n"); // convert WinDOS's "0x0D 0x0A" to just "0x0A" 
#endif
	QStringList sl = s.split("\n");
	int i;
	for (i = 0; i < sl.count(); i++) {
		s = sl[i];
		if ( !s.isEmpty() )
			qmc2MainWindow->log(QMC2_LOG_EMULATOR, tr("stderr[#%1]:").arg(procMap[proc]) + " " + s);
	}
}

void ProcessManager::finished(int exitCode, QProcess::ExitStatus exitStatus)
{
	QProcess *proc = (QProcess *)sender();
	QList<QTreeWidgetItem *> il = qmc2MainWindow->treeWidgetEmulators->findItems(QString::number(procMap[proc]), Qt::MatchStartsWith);
	if ( il.count() > 0 ) {
		QTreeWidgetItem *item = qmc2MainWindow->treeWidgetEmulators->takeTopLevelItem(qmc2MainWindow->treeWidgetEmulators->indexOfTopLevelItem(il[0]));
		if ( item ) {
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
			Embedder *embedder = NULL;
			for (int j = 0; j < qmc2MainWindow->tabWidgetEmbeddedEmulators->count() && embedder == NULL; j++) {
				if ( qmc2MainWindow->tabWidgetEmbeddedEmulators->tabText(j).startsWith(QString("#%1 - ").arg(item->text(QMC2_EMUCONTROL_COLUMN_NUMBER))) )
					embedder = (Embedder *)qmc2MainWindow->tabWidgetEmbeddedEmulators->widget(j);
			}
			if ( embedder )
				QTimer::singleShot(0, embedder, SLOT(clientClosed()));
#endif
			delete item;
		} else
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: ProcessManager::finished(...): trying to remove a null item"));
	}

	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("emulator #%1 finished, exit code = %2, exit status = %3, remaining emulators = %4").arg(procMap[proc]).arg(exitCodeString(exitCode)).arg(QString(exitStatus == QProcess::NormalExit ? tr("normal") : tr("crashed"))).arg(procMap.count() - 1));
	procMap.remove(proc);
	softwareListsMap.remove(proc);
	softwareNamesMap.remove(proc);

#if QMC2_USE_PHONON_API
	if ( procMap.count() == 0 && musicWasPlaying ) {
		sentPlaySignal = true;
		QTimer::singleShot(QMC2_AUDIOPLAYER_RESUME_DELAY, qmc2MainWindow, SLOT(on_actionAudioPlayTrack_triggered()));
	}
#endif

#if defined(QMC2_YOUTUBE_ENABLED)
	if ( procMap.count() == 0 && videoWasPlaying )
		if ( qmc2YouTubeWidget )
			if ( qmc2YouTubeWidget->isVisible() )
				QTimer::singleShot(QMC2_VIDEOPLAYER_RESUME_DELAY, qmc2YouTubeWidget, SLOT(play()));
#endif

	if ( !qmc2AutoMinimizedWidgets.isEmpty() )
		qmc2MainWindow->setWindowState(qmc2AutoMinimizedWidgets[qmc2MainWindow]);

#if defined(QMC2_SDLMAME)
	if ( qmc2FifoFile && qmc2FifoFile->isOpen() ) {
		if ( qmc2FifoNotifier ) {
			qmc2FifoNotifier->setEnabled(false);
			qmc2FifoNotifier->disconnect();
			delete qmc2FifoNotifier;
			qmc2FifoNotifier = NULL;
		}
		if ( qmc2FifoFile->isOpen() )
			qmc2FifoFile->close();
		delete qmc2FifoFile;
		qmc2FifoFile = NULL;
		qmc2MainWindow->createFifo(false);
	}
#endif
}

void ProcessManager::started()
{
	QProcess *proc = (QProcess *)sender();
	QTreeWidgetItem *procItem = new QTreeWidgetItem(qmc2MainWindow->treeWidgetEmulators);
	procItem->setText(QMC2_EMUCONTROL_COLUMN_NUMBER, QString::number(procMap[proc]));
	procItem->setText(QMC2_EMUCONTROL_COLUMN_PID, QString::number((quint64)(proc->pid())));
	procItem->setIcon(QMC2_EMUCONTROL_COLUMN_LED0, QIcon(QString::fromUtf8(":/data/img/led_off.png")));
	procItem->setIcon(QMC2_EMUCONTROL_COLUMN_LED1, QIcon(QString::fromUtf8(":/data/img/led_off.png")));
	procItem->setIcon(QMC2_EMUCONTROL_COLUMN_LED2, QIcon(QString::fromUtf8(":/data/img/led_off.png")));
	if ( launchForeignID ) {
		if ( !qmc2MainWindow->foreignID.isEmpty() ) 
			procItem->setText(QMC2_EMUCONTROL_COLUMN_GAME, qmc2MainWindow->foreignID.split(" ", QString::SkipEmptyParts)[0]);
	} else
		procItem->setText(QMC2_EMUCONTROL_COLUMN_GAME, qmc2DriverName);
#if defined(QMC2_OS_WIN)
	QString emuCommandLine = loggedCommandLine;
	procItem->setText(QMC2_EMUCONTROL_COLUMN_COMMAND, emuCommandLine.replace('/', '\\'));
#else
	procItem->setText(QMC2_EMUCONTROL_COLUMN_COMMAND, loggedCommandLine);
#endif
	// expand command column if it's still at the rightmost position
	if ( qmc2MainWindow->treeWidgetEmulators->header()->visualIndex(QMC2_EMUCONTROL_COLUMN_COMMAND) == QMC2_EMUCONTROL_COLUMN_COMMAND ) 
		qmc2MainWindow->treeWidgetEmulators->resizeColumnToContents(QMC2_EMUCONTROL_COLUMN_COMMAND);

	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("emulator #%1 started, PID = %2, running emulators = %3").arg(procMap[proc]).arg((quint64)proc->pid()).arg(procMap.count()));

#if QMC2_USE_PHONON_API
	if ( qmc2MainWindow->phononAudioPlayer->state() == Phonon::PlayingState && procMap.count() == 1 ) {
		musicWasPlaying = true;
		if ( qmc2MainWindow->checkBoxAudioPause->isChecked() )
			QTimer::singleShot(0, qmc2MainWindow, SLOT(on_actionAudioPauseTrack_triggered()));
	} else if ( procMap.count() == 1 )
		musicWasPlaying = false;
#endif

#if defined(QMC2_YOUTUBE_ENABLED)
	if ( qmc2YouTubeWidget ) {
		videoWasPlaying = qmc2YouTubeWidget->isPlaying();
		if ( videoWasPlaying )
			qmc2YouTubeWidget->pause();
	} else
		videoWasPlaying = false;
#endif

#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
	if ( qmc2StartEmbedded ) {
		qmc2MainWindow->treeWidgetEmulators->clearSelection();
		procItem->setSelected(true);
		QTimer::singleShot(QMC2_EMBED_DELAY, qmc2MainWindow, SLOT(action_embedEmulator_triggered()));
		qmc2StartEmbedded = false;
	}
#endif
}

QString ProcessManager::errorText(QProcess::ProcessError processError)
{
	switch ( processError ) {
		case QProcess::FailedToStart:
			return tr("failed to start");
		case QProcess::Crashed:
			return tr("crashed");
		case QProcess::WriteError:
			return tr("write error");
		case QProcess::ReadError:
			return tr("read error");
		case QProcess::Timedout:
			return tr("timeout");
		default:
			return tr("unknown error");
	}
}

void ProcessManager::error(QProcess::ProcessError processError)
{
	QProcess *proc = (QProcess *)sender();
	switch ( processError ) {
		case QProcess::FailedToStart:
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: failed to start emulator #%1").arg(procMap[proc]));
			procMap.remove(proc);
			softwareListsMap.remove(proc);
			softwareNamesMap.remove(proc);
			break;
		case QProcess::Crashed:
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: emulator #%1 crashed").arg(procMap[proc]));
			break;
		case QProcess::WriteError:
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to write to emulator #%1").arg(procMap[proc]));
			break;
		case QProcess::ReadError:
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: failed to read from emulator #%1").arg(procMap[proc]));
			break;
		default:
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: unhandled error for emulator #%1, error code = %2").arg(procMap[proc]).arg(processError));
			break;
	}
}

void ProcessManager::stateChanged(QProcess::ProcessState processState)
{
    // NOP
}

QString &ProcessManager::exitCodeString(int exitCode, bool textOnly)
{
	QString exitCodeText;
	switch ( exitCode ) {
		case QMC2_MAME_ERROR_NONE: exitCodeText = tr("no error"); break;
		case QMC2_MAME_ERROR_FAILED_VALIDITY: exitCodeText = tr("failed validity checks"); break;
		case QMC2_MAME_ERROR_MISSING_FILES: exitCodeText = tr("missing files"); break;
		case QMC2_MAME_ERROR_FATALERROR: exitCodeText = tr("fatal error"); break;
		case QMC2_MAME_ERROR_DEVICE: exitCodeText = tr("device initialization error"); break; // MESS-specific
		case QMC2_MAME_ERROR_NO_SUCH_MACHINE: exitCodeText = tr("machine doesn't exist"); break;
		case QMC2_MAME_ERROR_INVALID_CONFIG: exitCodeText = tr("invalid configuration"); break;
		case QMC2_MAME_ERROR_IDENT_NONROMS: exitCodeText = tr("identified all non-ROM files"); break;
		case QMC2_MAME_ERROR_IDENT_PARTIAL: exitCodeText = tr("identified some files but not all"); break;
		case QMC2_MAME_ERROR_IDENT_NONE: exitCodeText = tr("identified no files"); break;
		case QMC2_MAME_ERROR_UNKNOWN: default: exitCodeText = tr("unknown error"); break;
	}

	if ( textOnly )
		exitString = exitCodeText;
	else
		exitString = QString("%1 (%2)").arg(exitCode).arg(exitCodeText);

	return exitString;
}

Q_PID ProcessManager::getPid(int id)
{
	QProcess *proc = process(id);
	if ( proc )
		return proc->pid();
	else
		return QProcess().pid();
}
