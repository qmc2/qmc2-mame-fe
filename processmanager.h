#ifndef _PROCESSMANAGER_H_
#define _PROCESSMANAGER_H_

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
		QMap<QProcess *, QStringList> softwareListsMap;
		QMap<QProcess *, QStringList> softwareNamesMap;
		ushort procCount;
		QString loggedCommandLine;
		QString exitString;
		bool launchForeignID;
#if QMC2_USE_PHONON_API
		bool musicWasPlaying;
		bool sentPlaySignal;
#endif
#if defined(QMC2_YOUTUBE_ENABLED)
		bool videoWasPlaying;
#endif

		ProcessManager(QWidget *parent = 0);

		int start(QString &, QStringList &, bool autoConnect = true, QString workDir = QString(), QStringList softwareLists = QStringList(), QStringList softwareNames = QStringList());
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

		static QString errorText(QProcess::ProcessError);

	public slots:
		void started();
		void finished(int, QProcess::ExitStatus);
		void readyReadStandardOutput();
		void readyReadStandardError();
		void error(QProcess::ProcessError);
		void stateChanged(QProcess::ProcessState);
};

#endif
