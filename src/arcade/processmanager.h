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

	QString fileErrorToString(QFile::FileError);
	QString processErrorToString(QProcess::ProcessError);
	QString processStateToString(QProcess::ProcessState);
	int highestProcessID() { return mCurrentProcessId; }
	int runningProcesses() { return mProcessMap.count(); }

signals:
	void emulatorStarted(int);
	void emulatorFinished(int);

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
