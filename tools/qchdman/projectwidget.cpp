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
extern MainWindow *mainWindow;
extern quint64 runningProjects;

ProjectWidget::ProjectWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProjectWidget)
{
    ui->setupUi(this);

    chdmanProc = NULL;
    terminatedOnDemand = false;
    askFileName = false;

    compressionTypes["avhu"] = tr("avhu (A/V Huffman)");
    compressionTypes["cdfl"] = tr("cdfl (CD FLAC)");
    compressionTypes["cdlz"] = tr("cdlz (CD LZMA)");
    compressionTypes["cdzl"] = tr("cdzl (CD Deflate)");
    compressionTypes["flac"] = tr("flac (FLAC)");
    compressionTypes["huff"] = tr("huff (Huffman)");
    compressionTypes["lzma"] = tr("lzma (LZMA)");
    compressionTypes["zlib"] = tr("zlib (Deflate)");

    copyCompressors.clear();

    ui->comboBoxCopyCompression->blockSignals(true);
    ui->comboBoxCopyCompression->insertItem(0, tr("default"));
    ui->comboBoxCopyCompression->setItemIcon(0, QIcon(":/images/compression.png"));
    ui->comboBoxCopyCompression->insertSeparator(1);
    ui->comboBoxCopyCompression->insertItem(2, tr("default"));
    ui->comboBoxCopyCompression->setItemIcon(2, QIcon(":/images/default.png"));
    ui->comboBoxCopyCompression->insertItem(3, tr("none"));
    ui->comboBoxCopyCompression->setItemIcon(3, QIcon(":/images/none.png"));
    ui->comboBoxCopyCompression->insertSeparator(4);
    int i = 5;
    foreach (QString cmp, compressionTypes) {
        ui->comboBoxCopyCompression->insertItem(i, cmp);
        ui->comboBoxCopyCompression->setItemIcon(i, QIcon(":/images/inactive.png"));
        ui->comboBoxCopyCompression->setItemData(i, QCHDMAN_ITEM_INACTIVE, Qt::WhatsThisRole);
        i++;
    }
    ui->comboBoxCopyCompression->blockSignals(false);

    menuActions = new QMenu(this);
    actionCopyStdoutToClipboard = menuActions->addAction(tr("Copy stdout to clipboard"), this, SLOT(copyStdoutToClipboard()));
    actionCopyStderrToClipboard = menuActions->addAction(tr("Copy stderr to clipboard"), this, SLOT(copyStderrToClipboard()));
    actionCopyCommandToClipboard = menuActions->addAction(tr("Copy command to clipboard"), this, SLOT(copyCommandToClipboard()));
    menuActions->insertSeparator(actionCopyCommandToClipboard);
    ui->toolButtonActions->setMenu(menuActions);

    QFont f;
    f.fromString(globalConfig->preferencesLogFont());
    f.setPointSize(globalConfig->preferencesLogFontSize());
    ui->plainTextEditProjectLog->setFont(f);

    QTimer::singleShot(0, this, SLOT(init()));
}

ProjectWidget::~ProjectWidget()
{
    delete ui;
}

void ProjectWidget::on_comboBoxProjectType_currentIndexChanged(int index)
{
    int widgetHeight = 0;

    if ( index == -1 )
        index = ui->comboBoxProjectType->currentIndex();

    // FIXME: copy data where applicable

    switch ( index ) {
    case QCHDMAN_PRJ_INFO:
        widgetHeight = ui->frameInfo->height() + 4 * ui->gridLayoutScrollArea->contentsMargins().bottom();
        if ( globalConfig->preferencesShowHelpTexts() )
            widgetHeight += ui->labelInfoHelp->height() + ui->gridLayoutScrollArea->contentsMargins().bottom();
        break;
    case QCHDMAN_PRJ_VERIFY:
        widgetHeight = ui->frameVerify->height() + 4 * ui->gridLayoutScrollArea->contentsMargins().bottom();
        if ( globalConfig->preferencesShowHelpTexts() )
            widgetHeight += ui->labelVerifyHelp->height() + ui->gridLayoutScrollArea->contentsMargins().bottom();
        break;
    case QCHDMAN_PRJ_COPY:
        widgetHeight = ui->frameCopy->height() + 4 * ui->gridLayoutScrollArea->contentsMargins().bottom();
        if ( globalConfig->preferencesShowHelpTexts() )
            widgetHeight += ui->labelCopyHelp->height() + ui->gridLayoutScrollArea->contentsMargins().bottom();
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

    // adjust splitter
    int splitterHeight = ui->splitter->height();
    double percent = (double)widgetHeight / (double)splitterHeight;
    int newHeight = int((double)splitterHeight * percent);
    QList<int> newSizes;
    newSizes << newHeight << splitterHeight - newHeight;
    ui->splitter->setSizes(newSizes);
}

void ProjectWidget::on_toolButtonRun_clicked(bool refreshArgsOnly)
{    
    arguments.clear();

    switch ( ui->comboBoxProjectType->currentIndex() ) {
    case QCHDMAN_PRJ_INFO:
        jobName = tr("Info");
        arguments << "info";
        if ( !ui->lineEditInfoInputFile->text().isEmpty() )
            arguments << "--input" << ui->lineEditInfoInputFile->text();
        if ( ui->checkBoxInfoVerbose->isChecked() )
            arguments << "--verbose";
        break;
    case QCHDMAN_PRJ_VERIFY:
        jobName = tr("Verify");
        arguments << "verify";
        if ( !ui->lineEditVerifyInputFile->text().isEmpty() )
            arguments << "--input" << ui->lineEditVerifyInputFile->text();
        if ( !ui->lineEditVerifyParentInputFile->text().isEmpty() )
            arguments << "--inputparent" << ui->lineEditVerifyParentInputFile->text();
        break;
    case QCHDMAN_PRJ_COPY:
        jobName = tr("Copy");
        arguments << "copy";
        if ( !ui->lineEditCopyInputFile->text().isEmpty() )
            arguments << "--input" << ui->lineEditCopyInputFile->text();
        if ( !ui->lineEditCopyParentInputFile->text().isEmpty() )
            arguments << "--parentinput" << ui->lineEditCopyParentInputFile->text();
        if ( !ui->lineEditCopyOutputFile->text().isEmpty() )
            arguments << "--output" << ui->lineEditCopyOutputFile->text();
        if ( !ui->lineEditCopyParentOutputFile->text().isEmpty() )
            arguments << "--parentoutput" << ui->lineEditCopyParentOutputFile->text();
        if ( ui->checkBoxCopyForce->isChecked() )
            arguments << "--force";
        if ( ui->spinBoxCopyInputStartByte->value() >= 0 )
            arguments << "--inputstartbyte" << QString::number(ui->spinBoxCopyInputStartByte->value());
        if ( ui->spinBoxCopyInputStartHunk->value() >= 0 )
            arguments << "--inputstarthunk" << QString::number(ui->spinBoxCopyInputStartHunk->value());
        if ( ui->spinBoxCopyInputBytes->value() >= 0 )
            arguments << "--inputbytes" << QString::number(ui->spinBoxCopyInputBytes->value());
        if ( ui->spinBoxCopyInputHunks->value() >= 0 )
            arguments << "--inputhunks" << QString::number(ui->spinBoxCopyInputHunks->value());
        if ( ui->spinBoxCopyHunkSize->value() >= 0 )
            arguments << "--hunksize" << QString::number(ui->spinBoxCopyHunkSize->value());
        if ( !copyCompressors.isEmpty() )
            arguments << "--compression" << copyCompressors.join(",");
        else if ( ui->comboBoxCopyCompression->currentText() != tr("default") )
            arguments << "--compression" << "none";
        if ( ui->spinBoxCopyProcessors->value() >= 1 )
            arguments << "--numprocessors" << QString::number(ui->spinBoxCopyProcessors->value());
        break;
    case QCHDMAN_PRJ_CREATE_RAW:
        jobName = tr("CreateRaw");
        arguments << "createraw";
        // FIXME
        break;
    case QCHDMAN_PRJ_CREATE_HD:
        jobName = tr("CreateHD");
        arguments << "createhd";
        // FIXME
        break;
    case QCHDMAN_PRJ_CREATE_CD:
        jobName = tr("CreateCD");
        arguments << "createcd";
        // FIXME
        break;
    case QCHDMAN_PRJ_CREATE_LD:
        jobName = tr("CreateLD");
        arguments << "createld";
        // FIXME
        break;
    case QCHDMAN_PRJ_EXTRACT_RAW:
        jobName = tr("ExtractRaw");
        arguments << "extractraw";
        // FIXME
        break;
    case QCHDMAN_PRJ_EXTRACT_HD:
        jobName = tr("ExtractHD");
        arguments << "extracthd";
        // FIXME
        break;
    case QCHDMAN_PRJ_EXTRACT_CD:
        jobName = tr("ExtractCD");
        arguments << "extractcd";
        // FIXME
        break;
    case QCHDMAN_PRJ_EXTRACT_LD:
        jobName = tr("ExtractLD");
        arguments << "extractld";
        // FIXME
        break;
    }

    if ( refreshArgsOnly )
        return;

    ui->progressBar->setRange(0, 100);

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
    chdmanProc->start(globalConfig->preferencesChdmanBinary(), arguments);
    ui->toolButtonRun->setEnabled(false);
    ui->comboBoxProjectType->setEnabled(false);
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
    ui->comboBoxProjectType->setEnabled(true);
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
            case QCHDMAN_PRJ_COPY:
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
    ui->comboBoxProjectType->setEnabled(true);
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

void ProjectWidget::on_toolButtonBrowseCopyInputFile_clicked()
{
    QString s = QFileDialog::getOpenFileName(this, tr("Choose CHD input file"), ui->lineEditCopyInputFile->text(), tr("CHD files (*.chd);;All files (*)"));
    if ( !s.isNull() )
        ui->lineEditCopyInputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseCopyOutputFile_clicked()
{
    QString s = QFileDialog::getOpenFileName(this, tr("Choose CHD output file"), ui->lineEditCopyOutputFile->text(), tr("CHD files (*.chd);;All files (*)"));
    if ( !s.isNull() )
        ui->lineEditCopyOutputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseCopyParentInputFile_clicked()
{
    QString s = QFileDialog::getOpenFileName(this, tr("Choose parent CHD input file"), ui->lineEditCopyParentInputFile->text(), tr("CHD files (*.chd);;All files (*)"));
    if ( !s.isNull() )
        ui->lineEditCopyParentInputFile->setText(s);
}

void ProjectWidget::on_toolButtonBrowseCopyParentOutputFile_clicked()
{
    QString s = QFileDialog::getOpenFileName(this, tr("Choose parent CHD output file"), ui->lineEditCopyParentOutputFile->text(), tr("CHD files (*.chd);;All files (*)"));
    if ( !s.isNull() )
        ui->lineEditCopyParentOutputFile->setText(s);
}

void ProjectWidget::on_comboBoxCopyCompression_currentIndexChanged(int index)
{
    bool isDefault = false;

    if ( index == 2 ) { // default
        isDefault = true;
        copyCompressors.clear();
        for (int i = 5; i < ui->comboBoxCopyCompression->count(); i++) {
            ui->comboBoxCopyCompression->setItemIcon(i, QIcon(":/images/inactive.png"));
            ui->comboBoxCopyCompression->setItemData(i, QCHDMAN_ITEM_INACTIVE, Qt::WhatsThisRole);
        }
    } else if ( index == 3 ) { // none
        copyCompressors.clear();
        for (int i = 5; i < ui->comboBoxCopyCompression->count(); i++) {
            ui->comboBoxCopyCompression->setItemIcon(i, QIcon(":/images/inactive.png"));
            ui->comboBoxCopyCompression->setItemData(i, QCHDMAN_ITEM_INACTIVE, Qt::WhatsThisRole);
        }
    } else if ( index > 4 ) { // toggles
        copyCompressors.clear();
        if ( ui->comboBoxCopyCompression->itemData(index, Qt::WhatsThisRole).toString() == QCHDMAN_ITEM_ACTIVE ) {
            ui->comboBoxCopyCompression->setItemIcon(index, QIcon(":/images/inactive.png"));
            ui->comboBoxCopyCompression->setItemData(index, QCHDMAN_ITEM_INACTIVE, Qt::WhatsThisRole);
        } else {
            ui->comboBoxCopyCompression->setItemIcon(index, QIcon(":/images/active.png"));
            ui->comboBoxCopyCompression->setItemData(index, QCHDMAN_ITEM_ACTIVE, Qt::WhatsThisRole);
        }
        for (int i = 5; i < ui->comboBoxCopyCompression->count(); i++)
            if ( ui->comboBoxCopyCompression->itemData(i, Qt::WhatsThisRole).toString() == QCHDMAN_ITEM_ACTIVE )
                copyCompressors << ui->comboBoxCopyCompression->itemText(i).split(" ", QString::SkipEmptyParts)[0];
    }

    if ( isDefault )
        ui->comboBoxCopyCompression->setItemText(0, tr("default"));
    else if ( copyCompressors.isEmpty() )
        ui->comboBoxCopyCompression->setItemText(0, tr("none"));
    else
        ui->comboBoxCopyCompression->setItemText(0, copyCompressors.join(","));

    ui->comboBoxCopyCompression->blockSignals(true);
    ui->comboBoxCopyCompression->setCurrentIndex(0);
    ui->comboBoxCopyCompression->blockSignals(false);
}

void ProjectWidget::init()
{
    on_comboBoxProjectType_currentIndexChanged(QCHDMAN_PRJ_INFO);
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

void ProjectWidget::copyCommandToClipboard()
{
    on_toolButtonRun_clicked(true);
    QString command = globalConfig->preferencesChdmanBinary();
    foreach (QString arg, arguments) {
        if ( arg.contains(QRegExp("\\s")) )
            command += " \"" + arg + "\"";
        else
            command += " " + arg;
    }
    qApp->clipboard()->setText(command);
}

void ProjectWidget::load(const QString &filename)
{
}

void ProjectWidget::save()
{
    QString projectName = ((ProjectWindow *)parentWidget())->projectName;

    if ( projectName.startsWith(tr("Noname-%1").arg("")) )
        askFileName = true;

    saveAs(projectName);

    askFileName = false;
}

void ProjectWidget::saveAs(const QString &fileName)
{
    QString projectName = fileName;

    if ( fileName.isEmpty() || askFileName ) {
        projectName = ((ProjectWindow *)parentWidget())->projectName;
        if ( projectName.startsWith(tr("Noname-%1").arg("")) || projectName.isEmpty() || askFileName ) {
            QString s = QFileDialog::getSaveFileName(this, tr("Choose file name"), projectName, tr("All files (*)"));
            if ( s.isEmpty() )
                return;
            else
                projectName = s;
        }
    }

    QFile saveFile(projectName);
    if ( saveFile.open(QIODevice::WriteOnly | QIODevice::Text) ) {
        QTextStream ts(&saveFile);
        int projectType = ui->comboBoxProjectType->currentIndex();
        ts << "# " << tr("Qt CHDMAN project file -- please do not edit manually") << "\n";
        ts << "ApplicationVersion = " << QCHDMAN_APP_VERSION << "\n";
        ts << "ProjectFormatVersion = " << QCHDMAN_PRJ_FMT_VERSION << "\n";
        ts << "ProjectType = " << mainWindow->projectTypes[projectType] << "\n";
        switch ( projectType ) {
        case QCHDMAN_PRJ_INFO:
            ts << "InfoInputFile = " << ui->lineEditInfoInputFile->text() << "\n";
            ts << "InfoVerbose = " << ui->checkBoxInfoVerbose->isChecked() << "\n";
            break;
        case QCHDMAN_PRJ_VERIFY:
            ts << "VerifyInputFile = " << ui->lineEditVerifyInputFile->text() << "\n";
            ts << "VerifyParentInputFile = " << ui->lineEditVerifyParentInputFile->text() << "\n";
            break;
        case QCHDMAN_PRJ_COPY:
            ts << "CopyInputFile = " << ui->lineEditCopyInputFile->text() << "\n";
            ts << "CopyOutputFile = " << ui->lineEditCopyOutputFile->text() << "\n";
            ts << "CopyParentInputFile = " << ui->lineEditCopyParentInputFile->text() << "\n";
            ts << "CopyParentOutputFile = " << ui->lineEditCopyParentOutputFile->text() << "\n";
            ts << "CopyForce = " << ui->checkBoxCopyForce->isChecked() << "\n";
            ts << "CopyProcessors = " << ui->spinBoxCopyProcessors->value() << "\n";
            ts << "CopyInputStartByte = " << ui->spinBoxCopyInputStartByte->value() << "\n";
            ts << "CopyInputStartHunk = " << ui->spinBoxCopyInputStartHunk->value() << "\n";
            ts << "CopyInputBytes = " << ui->spinBoxCopyInputBytes->value() << "\n";
            ts << "CopyInputHunks = " << ui->spinBoxCopyInputHunks->value() << "\n";
            ts << "CopyHunkSize = " << ui->spinBoxCopyHunkSize->value() << "\n";
            ts << "CopyCompression = ";
            if ( !copyCompressors.isEmpty() )
                ts << copyCompressors.join(",");
            else if ( ui->comboBoxCopyCompression->currentText() == tr("default") )
                ts << "default";
            else
                ts << "none";
            ts << "\n";
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
        saveFile.close();
        ((ProjectWindow *)parentWidget())->projectName = projectName;
        parentWidget()->setWindowTitle(projectName);
    } else {
        // FIXME
    }
}
