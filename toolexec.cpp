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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ToolExecutor::ToolExecutor(QWidget *parent = %1)").arg((qulonglong)parent));
#endif

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

ToolExecutor::~ToolExecutor()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ToolExecutor::~ToolExecutor()");
#endif

}

void ToolExecutor::execute()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ToolExecutor::execute()");
#endif

	pushButtonOk->setEnabled(false);
	toolProc->start(toolCommand, toolArgs);
	textBrowserToolOutput->append(tr("### tool started, output below ###"));
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/CopyToolOutput").toBool() )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("tool control: ") + tr("### tool started, output below ###"));
}

void ToolExecutor::toolStarted()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ToolExecutor::toolStarted()");
#endif

}

void ToolExecutor::toolFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ToolExecutor::toolFinished(int exitCode = %1, QProcess::ExitStatus exitStatus = %2)").arg(exitCode).arg(exitStatus));
#endif

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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ToolExecutor::toolReadyReadStandardOutput()");
#endif

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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ToolExecutor::toolReadyReadStandardError()");
#endif

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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ToolExecutor::toolError(QProcess::ProcessError processError = %1)").arg(processError));
#endif

	textBrowserToolOutput->append(tr("### tool error, process error = %1 ###").arg(processError));
	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/CopyToolOutput").toBool() )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("tool control: ") + tr("### tool error, process error = %1 ###").arg(processError));
	pushButtonOk->setEnabled(true);

	if ( qmc2Config->value(QMC2_FRONTEND_PREFIX + "Tools/CloseToolDialog", false).toBool() )
		QTimer::singleShot(0, this, SLOT(accept()));
}

void ToolExecutor::toolStateChanged(QProcess::ProcessState processState)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ToolExecutor::toolStateChanged(QProcess::ProcessState processState = %1)").arg(processState));
#endif

}

void ToolExecutor::closeEvent(QCloseEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ToolExecutor::closeEvent(QCloseEvent *e = %1)").arg((qulonglong)e));
#endif

	if ( toolProc->state() != QProcess::NotRunning ) {
		toolProc->terminate();
		toolProc->waitForFinished(QMC2_TOOL_KILL_WAIT);
		if ( toolProc->state() != QProcess::NotRunning )
			toolProc->kill();
	}

	if ( e )
		e->accept();
}
