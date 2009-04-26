#include <QHeaderView>
#include "procmgr.h"
#include "qmc2main.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern bool qmc2GuiReady;
#if defined(QMC2_SDLMESS) || defined(QMC2_MESS)
extern QString qmc2MessMachineName;
#endif

ProcessManager::ProcessManager(QWidget *parent)
  : QObject(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ProcessManager::ProcessManager(QWidget *parent = 0x" + QString::number((ulong)parent, 16) + ")");
#endif

  procCount = 0;
#if QMC2_USE_PHONON_API
  musicWasPlaying = sentPlaySignal = FALSE;
#endif
}

ProcessManager::~ProcessManager()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ProcessManager::~ProcessManager()");
#endif

}

int ProcessManager::start(QString &command, QStringList &arguments, bool autoConnect)
{
#ifdef QMC2_DEBUG
  QString logMsg = "DEBUG: ProcessManager::start(QString &command = \"" + command + "\", QStringList &arguments = \"";
  int argCount;
  for (argCount = 0; argCount < arguments.count(); argCount++)
    logMsg += QString(argCount > 0 ? " " + arguments[argCount] : arguments[argCount]);
  logMsg += "\", bool autoConnect = " + QString(autoConnect ? "TRUE" : "FALSE") + ")";
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, logMsg);
#endif

  QProcess *proc = new QProcess(this);
  if ( autoConnect ) {
    lastCommand = command;
    int i;
    for (i = 0; i < arguments.count(); i++)
      lastCommand += " " + arguments[i];
#if defined(Q_WS_WIN)
    QString emuCommandLine = lastCommand;
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("starting emulator #%1, command = %2").arg(procCount).arg(emuCommandLine.replace('/', '\\')));
#else
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("starting emulator #%1, command = %2").arg(procCount).arg(lastCommand));
#endif
    connect(proc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(error(QProcess::ProcessError)));
    connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finished(int, QProcess::ExitStatus)));
    connect(proc, SIGNAL(readyReadStandardOutput()), this, SLOT(readyReadStandardOutput()));
    connect(proc, SIGNAL(readyReadStandardError()), this, SLOT(readyReadStandardError()));
    connect(proc, SIGNAL(started()), this, SLOT(started()));
    connect(proc, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(stateChanged(QProcess::ProcessState)));
  }
  procMap[proc] = procCount++;
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
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ProcessManager::terminate(QProcess *proc = 0x" + QString::number((qulonglong)proc, 16) + ")");
#endif

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("terminating emulator #%1, PID = %2").arg(procMap[proc]).arg((quint64)proc->pid()));
  proc->terminate();
}

void ProcessManager::terminate(ushort index)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ProcessManager::terminate(ushort index = " + QString::number(index) + ")");
#endif

  QProcess *proc = process(index);
  if ( proc )
    terminate(proc);
  else
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: ProcessManager::terminate(ushort index = %1): trying to terminate a null process").arg(index));
}

void ProcessManager::kill(QProcess *proc)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ProcessManager::kill(QProcess *proc = 0x" + QString::number((qulonglong)proc, 16) + ")");
#endif

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("killing emulator #%1, PID = %2").arg(procMap[proc]).arg((quint64)proc->pid()));
  proc->kill();
}

void ProcessManager::kill(ushort index)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ProcessManager::kill(ushort index = " + QString::number(index) + ")");
#endif

  QProcess *proc = process(index);
  if ( proc )
    kill(proc);
  else
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: ProcessManager::kill(ushort index = %1): trying to kill a null process").arg(index));
}

void ProcessManager::readyReadStandardOutput()
{
  QProcess *proc = (QProcess *)sender();

#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ProcessManager::readyReadStandardOutput(): proc = 0x" + QString::number((qulonglong)proc, 16));
#endif

  QString s = proc->readAllStandardOutput();
  QStringList sl = s.split("\n");
  int i;
  for (i = 0; i < sl.count(); i++) {
    s = sl[i].simplified();
    if ( !s.isEmpty() )
      qmc2MainWindow->log(QMC2_LOG_EMULATOR, tr("stdout[#%1]:").arg(procMap[proc]) + " " + s);
  }
}

void ProcessManager::readyReadStandardError()
{
  QProcess *proc = (QProcess *)sender();

#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ProcessManager::readyReadStandardError(): proc = 0x" + QString::number((qulonglong)proc, 16));
#endif

  QString s = proc->readAllStandardError();
  QStringList sl = s.split("\n");
  int i;
  for (i = 0; i < sl.count(); i++) {
    s = sl[i].simplified();
    if ( !s.isEmpty() )
      qmc2MainWindow->log(QMC2_LOG_EMULATOR, tr("stderr[#%1]:").arg(procMap[proc]) + " " + s);
  }
}

void ProcessManager::finished(int exitCode, QProcess::ExitStatus exitStatus)
{
  QProcess *proc = (QProcess *)sender();

#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ProcessManager::finished(int exitCode = " + QString::number(exitCode) + ", QProcess::ExitStatus exitStatus = "+ QString::number(exitStatus) + "): proc = 0x" + QString::number((qulonglong)proc, 16));
#endif

  QList<QTreeWidgetItem *> il = qmc2MainWindow->treeWidgetEmulators->findItems(QString::number(procMap[proc]), Qt::MatchStartsWith);
  if ( il.count() > 0 ) {
    QTreeWidgetItem *item = qmc2MainWindow->treeWidgetEmulators->takeTopLevelItem(qmc2MainWindow->treeWidgetEmulators->indexOfTopLevelItem(il[0]));
    if ( item )
      delete item;
    else
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: ProcessManager::finished(...): trying to remove a null item"));
  }

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("emulator #%1 finished, exit code = %2, exit status = %3, remaining emulators = %4").arg(procMap[proc]).arg(exitCode).arg(QString(exitStatus == QProcess::NormalExit ? tr("normal") : tr("crashed"))).arg(procMap.count() - 1));
  procMap.remove(proc);

#if QMC2_USE_PHONON_API
  if ( procMap.count() == 0 && musicWasPlaying ) {
    sentPlaySignal = TRUE;
    QTimer::singleShot(2000, qmc2MainWindow, SLOT(on_actionAudioPlayTrack_triggered()));
  }
#endif
}

void ProcessManager::started()
{
  QProcess *proc = (QProcess *)sender();

#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ProcessManager::started(): proc = 0x" + QString::number((qulonglong)proc, 16));
#endif

  QTreeWidgetItem *procItem = new QTreeWidgetItem(qmc2MainWindow->treeWidgetEmulators);
  procItem->setText(QMC2_EMUCONTROL_COLUMN_NUMBER, QString::number(procMap[proc]));
  procItem->setText(QMC2_EMUCONTROL_COLUMN_PID, QString::number((quint64)(proc->pid())));
  procItem->setIcon(QMC2_EMUCONTROL_COLUMN_LED0, QIcon(QString::fromUtf8(":/data/img/led_off.png")));
  procItem->setIcon(QMC2_EMUCONTROL_COLUMN_LED1, QIcon(QString::fromUtf8(":/data/img/led_off.png")));
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  procItem->setText(QMC2_EMUCONTROL_COLUMN_GAME, lastCommand.split(" ").last());
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  procItem->setText(QMC2_EMUCONTROL_COLUMN_GAME, qmc2MessMachineName);
#endif
#if defined(Q_WS_WIN)
  QString emuCommandLine = lastCommand;
  procItem->setText(QMC2_EMUCONTROL_COLUMN_COMMAND, emuCommandLine.replace('/', '\\'));
#else
  procItem->setText(QMC2_EMUCONTROL_COLUMN_COMMAND, lastCommand);
#endif
  // expand command column if it's still at the rightmost position
  if ( qmc2MainWindow->treeWidgetEmulators->header()->visualIndex(QMC2_EMUCONTROL_COLUMN_COMMAND) == QMC2_EMUCONTROL_COLUMN_COMMAND ) 
    qmc2MainWindow->treeWidgetEmulators->resizeColumnToContents(QMC2_EMUCONTROL_COLUMN_COMMAND);

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("emulator #%1 started, PID = %2, running emulators = %3").arg(procMap[proc]).arg((quint64)proc->pid()).arg(procMap.count()));

#if QMC2_USE_PHONON_API
  if ( qmc2MainWindow->phononAudioPlayer->state() == Phonon::PlayingState && procMap.count() == 1 ) {
    musicWasPlaying = TRUE;
    if ( qmc2MainWindow->checkBoxAudioPause->isChecked() )
      QTimer::singleShot(0, qmc2MainWindow, SLOT(on_actionAudioPauseTrack_triggered()));
  } else if ( procMap.count() == 1 )
    musicWasPlaying = FALSE;
#endif
}

void ProcessManager::error(QProcess::ProcessError processError)
{
  QProcess *proc = (QProcess *)sender();

#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ProcessManager::error(QProcess::ProcessError processError = " + QString::number(processError) + "): proc = 0x" + QString::number((qulonglong)proc, 16));
#endif
}

void ProcessManager::stateChanged(QProcess::ProcessState processState)
{
  QProcess *proc = (QProcess *)sender();

#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ProcessManager::stateChanged(QProcess::ProcessState processState = " + QString::number(processState) + "): proc = 0x" + QString::number((qulonglong)proc, 16));
#endif
}
