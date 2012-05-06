#include <QtGui>
#include <QFileDialog>
#if defined(Q_OS_WIN)
#include <windows.h>
#endif

#include "projectwindow.h"
#include "projectwidget.h"
#include "ui_projectwidget.h"
#include "mainwindow.h"
#include "macros.h"
#include "settings.h"

extern Settings *globalConfig;
extern MainWindow *mW;
extern quint64 runningProjects;

ProjectWidget::ProjectWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProjectWidget)
{
    ui->setupUi(this);

    chdmanProc = NULL;
    terminatedOnDemand = false;

    menuActions = new QMenu(this);
    actionCopyStdoutToClipboard = menuActions->addAction(tr("Copy stdout to clipboard"), this, SLOT(copyStdoutToClipboard()));
    actionCopyStderrToClipboard = menuActions->addAction(tr("Copy stderr to clipboard"), this, SLOT(copyStderrToClipboard()));
    ui->toolButtonActions->setMenu(menuActions);

    QFont f;
    f.fromString(globalConfig->preferencesLogFont());
    f.setPointSize(globalConfig->preferencesLogFontSize());
    ui->plainTextEditProjectLog->setFont(f);
}

ProjectWidget::~ProjectWidget()
{
    delete ui;
}

void ProjectWidget::on_comboBoxProjectType_currentIndexChanged(int index)
{
    // FIXME: copy data where applicable
    switch ( index ) {
    case QCHDMAN_PRJ_INFO:
        break;
    case QCHDMAN_PRJ_VERIFY:
        break;
    case QCHDMAN_PRJ_COPY:
        break;
    case QCHDMAN_PRJ_CREATE_RAW:
        break;
    case QCHDMAN_PRJ_CREATE_HD:
        break;
    case QCHDMAN_PRJ_CREATE_CD:
        break;
    case QCHDMAN_PRJ_CREATE_LD:
        break;
    case QCHDMAN_PRJ_EXTRACT_RAW:
        break;
    case QCHDMAN_PRJ_EXTRACT_HD:
        break;
    case QCHDMAN_PRJ_EXTRACT_CD:
        break;
    case QCHDMAN_PRJ_EXTRACT_LD:
        break;
    }
}

void ProjectWidget::on_toolButtonRun_clicked()
{
    QStringList args;

    switch ( ui->comboBoxProjectType->currentIndex() ) {
    case QCHDMAN_PRJ_INFO:
        jobName = tr("Info");
        args << "info";
        if ( !ui->lineEditInfoInputFile->text().isEmpty() )
            args << "--input" << ui->lineEditInfoInputFile->text();
        if ( ui->checkBoxInfoVerbose->isChecked() )
            args << "--verbose";
        ui->progressBar->setRange(0, 100);
        break;
    case QCHDMAN_PRJ_VERIFY:
        jobName = tr("Verify");
        args << "verify";
        if ( !ui->lineEditVerifyInputFile->text().isEmpty() )
            args << "--input" << ui->lineEditVerifyInputFile->text();
        if ( !ui->lineEditVerifyParentInputFile->text().isEmpty() )
            args << "--inputparent" << ui->lineEditVerifyParentInputFile->text();
        ui->progressBar->setRange(0, 100);
        break;
    case QCHDMAN_PRJ_COPY:
        jobName = tr("Copy");
        args << "copy";
        // FIXME
        break;
    case QCHDMAN_PRJ_CREATE_RAW:
        jobName = tr("CreateRaw");
        args << "createraw";
        // FIXME
        break;
    case QCHDMAN_PRJ_CREATE_HD:
        jobName = tr("CreateHD");
        args << "createhd";
        // FIXME
        break;
    case QCHDMAN_PRJ_CREATE_CD:
        jobName = tr("CreateCD");
        args << "createcd";
        // FIXME
        break;
    case QCHDMAN_PRJ_CREATE_LD:
        jobName = tr("CreateLD");
        args << "createld";
        // FIXME
        break;
    case QCHDMAN_PRJ_EXTRACT_RAW:
        jobName = tr("ExtractRaw");
        args << "extractraw";
        // FIXME
        break;
    case QCHDMAN_PRJ_EXTRACT_HD:
        jobName = tr("ExtractHD");
        args << "extracthd";
        // FIXME
        break;
    case QCHDMAN_PRJ_EXTRACT_CD:
        jobName = tr("ExtractCD");
        args << "extractcd";
        // FIXME
        break;
    case QCHDMAN_PRJ_EXTRACT_LD:
        jobName = tr("ExtractLD");
        args << "extractld";
        // FIXME
        break;
    }

    chdmanProc = new QProcess(this);

    connect(chdmanProc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(error(QProcess::ProcessError)));
    connect(chdmanProc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(finished(int, QProcess::ExitStatus)));
    connect(chdmanProc, SIGNAL(readyReadStandardOutput()), this, SLOT(readyReadStandardOutput()));
    connect(chdmanProc, SIGNAL(readyReadStandardError()), this, SLOT(readyReadStandardError()));
    connect(chdmanProc, SIGNAL(started()), this, SLOT(started()));
    connect(chdmanProc, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(stateChanged(QProcess::ProcessState)));

    ui->plainTextEditProjectLog->clear();
    log(tr("starting process"));

    stdoutOutput.clear();
    stderrOutput.clear();

    terminatedOnDemand = false;
    chdmanProc->start(globalConfig->preferencesChdmanBinary(), args);
    ui->toolButtonRun->setEnabled(false);
    ui->progressBar->setFormat(tr("Starting"));
    ui->progressBar->setValue(0);
}

void ProjectWidget::on_toolButtonStop_clicked()
{
    log(tr("terminating process"));
    terminatedOnDemand = true;
    chdmanProc->terminate();
}

void ProjectWidget::started()
{
    runningProjects++;
#if defined(Q_OS_WIN)
    log(tr("process started: PID = %1").arg(chdmanProc->pid()->dwProcessId));
#else
    log(tr("process started: PID = %1").arg(chdmanProc->pid()));
#endif
    ui->toolButtonStop->setEnabled(true);
    ui->progressBar->setFormat(tr("Running"));
}

void ProjectWidget::finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    runningProjects--;
    QString statusString = tr("normal");
    if ( !terminatedOnDemand && exitStatus == QProcess::CrashExit )
        statusString = tr("crashed");
    log(tr("process finished: exitCode = %1, exitStatus = %2").arg(exitCode).arg(statusString));
    ui->toolButtonRun->setEnabled(true);
    ui->toolButtonStop->setEnabled(false);
    ui->progressBar->setFormat(tr("Idle"));
    ui->progressBar->setValue(0);
}

void ProjectWidget::readyReadStandardOutput()
{
    QString s = chdmanProc->readAllStandardOutput();
    stdoutOutput += s;
    QStringList sl = s.split("\n");
    int i;
    for (i = 0; i < sl.count(); i++) {
        s = sl[i].simplified();
        if ( !s.isEmpty() )
            log(tr("stdout") + ": " + s);
    }
}

void ProjectWidget::readyReadStandardError()
{
    QString s = chdmanProc->readAllStandardError();
    stderrOutput += s;
    QStringList sl = s.split("\n");
    int percent = 0;
    for (int i = 0; i < sl.count(); i++) {
        s = sl[i].simplified();
        if ( !s.isEmpty() ) {
            log(tr("stderr") + ": " + s);
            switch ( ui->comboBoxProjectType->currentIndex() ) {
            case QCHDMAN_PRJ_VERIFY:
                if ( s.contains(QRegExp(", \\d+\\.\\d+\\%\\ complete\\.\\.\\.")) ) {
                    QRegExp rx(", (\\d+)\\.(\\d+)\\%\\ complete\\.\\.\\.");
                    int pos = rx.indexIn(s);
                    if ( pos > -1 ) {
                        int decimal = rx.cap(2).toInt();
                        percent = rx.cap(1).toInt() + (decimal >= 5 ? 1 : 0);
                    }
                } else if ( s.contains("Compression complete ... final ratio =") )
                    percent = 100;
                break;
            }
        }
    }
    ui->progressBar->setValue(percent);
}

void ProjectWidget::error(QProcess::ProcessError processError)
{
    QString errString;
    bool doLog = true;

    switch ( processError ) {
    case QProcess::FailedToStart:
        errString = tr("failed to start");
        break;
    case QProcess::Crashed:
        errString = tr("crashed");
        if ( terminatedOnDemand )
            doLog = false;
        else
        break;
    case QProcess::Timedout:
        errString = tr("timed out");
        break;
    case QProcess::WriteError:
        errString = tr("write error");
        break;
    case QProcess::ReadError:
        errString = tr("read error");
        break;
    case QProcess::UnknownError:
    default:
        errString = tr("unknown");
        break;
    }

    if ( doLog )
        log(tr("process error: %1").arg(errString));

    ui->toolButtonRun->setEnabled(true);
    ui->toolButtonStop->setEnabled(false);
    ui->progressBar->setFormat(tr("Idle"));
    ui->progressBar->setValue(0);
}

void ProjectWidget::stateChanged(QProcess::ProcessState)
{
    // NOP
}

void ProjectWidget::on_toolButtonBrowseInfoInputFile_clicked()
{
    QString s = QFileDialog::getOpenFileName(this, tr("Choose CHD input file"), ui->lineEditInfoInputFile->text(), tr("CHD files (*.chd);;All files (*)"));
    if ( !s.isNull() )
        ui->lineEditInfoInputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseVerifyInputFile_clicked()
{
    QString s = QFileDialog::getOpenFileName(this, tr("Choose CHD input file"), ui->lineEditVerifyInputFile->text(), tr("CHD files (*.chd);;All files (*)"));
    if ( !s.isNull() )
        ui->lineEditVerifyInputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseVerifyParentInputFile_clicked()
{
    QString s = QFileDialog::getOpenFileName(this, tr("Choose parent CHD input file"), ui->lineEditVerifyParentInputFile->text(), tr("CHD files (*.chd);;All files (*)"));
    if ( !s.isNull() )
        ui->lineEditVerifyParentInputFile->setText(s);
}

void ProjectWidget::log(QString message)
{
    message.prepend(QTime::currentTime().toString("hh:mm:ss.zzz") + ": " + jobName + ": ");
    ui->plainTextEditProjectLog->appendPlainText(message);
    ui->plainTextEditProjectLog->verticalScrollBar()->setValue(ui->plainTextEditProjectLog->verticalScrollBar()->maximum());
}

void ProjectWidget::setLogFont(QFont f)
{
    ui->plainTextEditProjectLog->setFont(f);
}

void ProjectWidget::copyStdoutToClipboard()
{
    qApp->clipboard()->setText(stdoutOutput);
}

void ProjectWidget::copyStderrToClipboard()
{
    qApp->clipboard()->setText(stderrOutput);
}
