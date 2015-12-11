#include "settings.h"
#include "toolexec.h"
#include "qmc2main.h"
#include "macros.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;

ToolExecutor::ToolExecutor(QWidget *parent, QString &command, QStringList &args, QString workDir)
	: QDialog(parent)
{
	setupUi(this);
	setAttribute(Qt::WA_ShowWithoutActivating);

	toolExitCode = -1;
	toolExitStatus = QProcess::CrashExit;

	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFont logFont = f;
	if ( !qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/LogFont").toString().isEmpty() )
		logFont.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/LogFont").toString());
	textBrowserToolOutput->setFont(logFont);

	toolCommand = command;
	toolArgs = args;
	toolProc = new QProcess(this);
	if ( !workDir.isEmpty() )
		toolProc->setWorkingDirectory(workDir);
	connect(toolProc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(toolError(QProcess::ProcessError)));
	connect(toolProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(toolFinished(int, QProcess::ExitStatus)));
	connect(toolProc, SIGNAL(readyReadStandardOutput()), this, SLOT(toolReadyReadStandardOutput()));
	connect(toolProc, SIGNAL(readyReadStandardError()), this, SLOT(toolReadyReadStandardError()));
	connect(toolProc, SIGNAL(started()), this, SLOT(toolStarted()));
	connect(toolProc, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(toolStateChanged(QProcess::ProcessState)));
	QString commandString = toolCommand, s;
	foreach (s, toolArgs)
		commandString += " " + s;
	lineEditCommand->setText(commandString);
	QTimer::singleShot(0, this, SLOT(execute()));
}

void ToolExecutor::execute()
{
	pushButtonOk->setEnabled(false);
	toolProc->start(toolCommand, toolArgs);
	textBrowserToolOutput->append(tr("### tool started, output below ###"));
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/CopyToolOutput").toBool() )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("tool control: ") + tr("### tool started, output below ###"));
}

void ToolExecutor::toolStarted()
{
    // NOP
}

void ToolExecutor::toolFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	toolExitCode = exitCode;
	toolExitStatus = exitStatus;
	textBrowserToolOutput->append(tr("### tool finished, exit code = %1, exit status = %2 ###").arg(exitCode).arg(exitStatus));
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/CopyToolOutput").toBool() )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("tool control: ") + tr("### tool finished, exit code = %1, exit status = %2 ###").arg(exitCode).arg(exitStatus));
	pushButtonOk->setEnabled(true);

	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/CloseToolDialog", false).toBool() )
		QTimer::singleShot(0, this, SLOT(accept()));
}

void ToolExecutor::toolReadyReadStandardOutput()
{
	QString s = toolProc->readAllStandardOutput();
	QStringList sl = s.split("\n");
	foreach (s, sl)
		if ( !s.isEmpty() ) {
			textBrowserToolOutput->append(s);
			if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/CopyToolOutput").toBool() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("tool output: ") + tr("stdout: %1").arg(s));
		}
}

void ToolExecutor::toolReadyReadStandardError()
{
	QString s = toolProc->readAllStandardError();
	QStringList sl = s.split("\n");
	foreach (s, sl)
		if ( !s.isEmpty() ) {
			textBrowserToolOutput->append(s);
			if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/CopyToolOutput").toBool() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("tool output: ") + tr("stderr: %1").arg(s));
		}
}

void ToolExecutor::toolError(QProcess::ProcessError processError)
{
	textBrowserToolOutput->append(tr("### tool error, process error = %1 ###").arg(processError));
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/CopyToolOutput").toBool() )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("tool control: ") + tr("### tool error, process error = %1 ###").arg(processError));
	pushButtonOk->setEnabled(true);

	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/CloseToolDialog", false).toBool() )
		QTimer::singleShot(0, this, SLOT(accept()));
}

void ToolExecutor::toolStateChanged(QProcess::ProcessState processState)
{
    // NOP
}

void ToolExecutor::closeEvent(QCloseEvent *e)
{
	if ( toolProc->state() != QProcess::NotRunning ) {
		toolProc->terminate();
		toolProc->waitForFinished(QMC2_TOOL_KILL_WAIT);
		if ( toolProc->state() != QProcess::NotRunning )
			toolProc->kill();
	}

	if ( e )
		e->accept();
}
