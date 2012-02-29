#ifndef _PROCMGR_H_
#define _PROCMGR_H_

#include <QProcess>
#include <QMap>
#include <QString>
#include <QStringList>
#include "macros.h"

class ProcessManager : public QObject
{
  Q_OBJECT

  public:
    QMap<QProcess *, ushort> procMap;
    ushort procCount;
    QString lastCommand;
    QString exitString;
    bool launchForeignID;
#if QMC2_USE_PHONON_API
    bool musicWasPlaying;
    bool sentPlaySignal;
#if defined(QMC2_YOUTUBE_ENABLED)
    bool videoWasPlaying;
#endif
#endif

    ProcessManager(QWidget *parent = 0);
    ~ProcessManager();

    int start(QString &, QStringList &, bool autoConnect = true, QString workDir = QString());
    QProcess *process(ushort);
    QString readStandardOutput(QProcess *);
    QString readStandardOutput(ushort);
    QString readStandardError(QProcess *);
    QString readStandardError(ushort);
    void terminate(QProcess *);
    void terminate(ushort);
    void kill(QProcess *);
    void kill(ushort);
    QString &exitCodeString(int, bool textOnly = false);
    Q_PID getPid(int);

  public slots:
    void started();
    void finished(int, QProcess::ExitStatus);
    void readyReadStandardOutput();
    void readyReadStandardError();
    void error(QProcess::ProcessError);
    void stateChanged(QProcess::ProcessState);
};

#endif
