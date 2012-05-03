#include <QtGui>
#include <QFileDialog>

#include "projectwidget.h"
#include "ui_projectwidget.h"
#include "mainwindow.h"
#include "macros.h"
#include "settings.h"

extern Settings *globalConfig;
extern MainWindow *mW;

ProjectWidget::ProjectWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProjectWidget)
{
    ui->setupUi(this);

    ui->groupBoxProjectDetails->setTitle(ui->comboBoxProjectType->currentText());
    QList<int> sizesList;
    QSize sizes = globalConfig->projectWidgetSplitterSizes();
    sizesList << sizes.width() << sizes.height();
    ui->splitter->setSizes(sizesList);
}

ProjectWidget::~ProjectWidget()
{
    delete ui;
}

QSize ProjectWidget::splitterSizes()
{
    QList<int> sizesList = ui->splitter->sizes();
    QSize sizes(sizesList[0], sizesList[1]);
    return sizes;
}

void ProjectWidget::on_comboBoxProjectType_currentIndexChanged(int)
{
    ui->groupBoxProjectDetails->setTitle(ui->comboBoxProjectType->currentText());
}

void ProjectWidget::on_splitter_splitterMoved(int, int)
{
    globalConfig->setProjectWidgetSplitterSizes(splitterSizes());
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
        break;
    case QCHDMAN_PRJ_VERIFY:
        jobName = tr("Verify");
        args << "verify";
        if ( !ui->lineEditVerifyInputFile->text().isEmpty() )
            args << "--input" << ui->lineEditVerifyInputFile->text();
        if ( !ui->lineEditVerifyParentInputFile->text().isEmpty() )
            args << "--inputparent" << ui->lineEditVerifyParentInputFile->text();
        break;
    case QCHDMAN_PRJ_COPY:
        jobName = tr("Copy");
        break;
    case QCHDMAN_PRJ_CREATE_RAW:
        jobName = tr("CreateRaw");
        break;
    case QCHDMAN_PRJ_CREATE_HD:
        jobName = tr("CreateHD");
        break;
    case QCHDMAN_PRJ_CREATE_CD:
        jobName = tr("CreateCD");
        break;
    case QCHDMAN_PRJ_CREATE_LD:
        jobName = tr("CreateLD");
        break;
    case QCHDMAN_PRJ_EXTRACT_RAW:
        jobName = tr("ExtractRaw");
        break;
    case QCHDMAN_PRJ_EXTRACT_HD:
        jobName = tr("ExtractHD");
        break;
    case QCHDMAN_PRJ_EXTRACT_CD:
        jobName = tr("ExtractCD");
        break;
    case QCHDMAN_PRJ_EXTRACT_LD:
        jobName = tr("ExtractLD");
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
    log(tr("starting external process"));

    chdmanProc->start(globalConfig->preferencesChdmanBinary(), args);
}

void ProjectWidget::started()
{
    log(tr("process started; PID = %1").arg(chdmanProc->pid()));
}

void ProjectWidget::finished(int exitCode, QProcess::ExitStatus exitStatus)
{
    log(tr("process finished: exitCode = %1, exitStatus = %2").arg(exitCode).arg(QString(exitStatus == QProcess::NormalExit ? tr("normal") : tr("crashed"))));
}

void ProjectWidget::readyReadStandardOutput()
{
    QString s = chdmanProc->readAllStandardOutput();
    QStringList sl = s.split("\n");
    int i;
    for (i = 0; i < sl.count(); i++) {
        s = sl[i].simplified();
        if ( !s.isEmpty() )
            log(tr("stdout:") + " " + s);
    }
}

void ProjectWidget::readyReadStandardError()
{
    QString s = chdmanProc->readAllStandardError();
    QStringList sl = s.split("\n");
    int i;
    for (i = 0; i < sl.count(); i++) {
        s = sl[i].simplified();
        if ( !s.isEmpty() )
            log(tr("stderr:") + " " + s);
    }
}

void ProjectWidget::error(QProcess::ProcessError processError)
{
    QString errString;
    switch ( processError ) {
    case QProcess::FailedToStart:
        errString = tr("failed to start");
        break;
    case QProcess::Crashed:
        errString = tr("crashed");
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

    log(tr("process error: %1").arg(errString));
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
}
