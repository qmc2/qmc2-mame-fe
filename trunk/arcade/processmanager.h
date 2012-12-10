#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <QObject>
#include <QMap>
#include <QProcess>
#include "emulatoroption.h"

class ProcessManager : public QObject
{
    Q_OBJECT
public:
    explicit ProcessManager(QObject *parent = 0);
    virtual ~ProcessManager();

    int startEmulator(QString);
    void createTemplateList();

signals:
    void emulatorStarted(QProcess *);
    void emulatorFinished(QProcess *);

public slots:
    void error(QProcess::ProcessError);
    void finished(int, QProcess::ExitStatus);
    void readyReadStandardOutput();
    void readyReadStandardError();
    void started();
    void stateChanged(QProcess::ProcessState);

private:
    int mCurrentProcessId;
    QMap<int, QProcess *> mProcessMap;
    QList<EmulatorOption> mTemplateList;
};

#endif // PROCESSMANAGER_H
