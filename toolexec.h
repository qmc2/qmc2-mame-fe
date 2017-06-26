#ifndef TOOLEXEC_H
#define TOOLEXEC_H

#include <QProcess>
#include "ui_toolexec.h"

class ToolExecutor : public QDialog, public Ui::ToolExecutor
{
	Q_OBJECT

	public:
		QString toolCommand;
		QStringList toolArgs;
		QProcess *toolProc;
		int toolExitCode;
		QProcess::ExitStatus toolExitStatus;

		ToolExecutor(QWidget *, QString &, QStringList &, QString workDir = QString());

	public slots:
		void execute();
		void toolStarted();
		void toolFinished(int, QProcess::ExitStatus);
		void toolReadyReadStandardOutput();
		void toolReadyReadStandardError();
		void toolError(QProcess::ProcessError);
		void toolStateChanged(QProcess::ProcessState);

	protected:
		void closeEvent(QCloseEvent *);
};

#endif
