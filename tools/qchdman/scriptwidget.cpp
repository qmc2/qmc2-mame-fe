#include <QTimer>
#include <QTime>
#include <QPixmap>
#include <QPainter>
#include <QIcon>
#include <QFont>
#include <QFontMetrics>
#include <QScrollBar>
#include <QFileDialog>
#include <QStatusBar>

#include "ui_scriptwidget.h"
#include "scriptwidget.h"
#include "scripteditor.h"
#include "mainwindow.h"
#include "qchdmansettings.h"
#include "ecmascripthighlighter.h"
#include "macros.h"

extern QtChdmanGuiSettings *globalConfig;
extern MainWindow *mainWindow;
extern quint64 runningScripts;

ScriptWidget::ScriptWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScriptWidget)
{
    ui->setupUi(this);

    parentWidget()->setWindowIcon(QIcon(":/images/script.png"));

    // use our 'tweaked' line-numbering script-editor
    delete ui->plainTextEditScript;
    ui->plainTextEditScript = new ScriptEditor(this);
    ui->splitter->insertWidget(0, ui->plainTextEditScript);

    askFileName = isRunning = false;

    // log-limit spin-box
    spinBoxLimitScriptLog = new QSpinBox(this);
    spinBoxLimitScriptLog->setToolTip(tr("Maximum number of lines held in script log"));
    spinBoxLimitScriptLog->setPrefix(tr("Limit") + ": ");
    spinBoxLimitScriptLog->setSpecialValueText(tr("No limit"));
    spinBoxLimitScriptLog->setRange(0, 100000);
    spinBoxLimitScriptLog->setWrapping(true);
    spinBoxLimitScriptLog->setSingleStep(100);
    spinBoxLimitScriptLog->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    spinBoxLimitScriptLog->setKeyboardTracking(false);
    ui->tabWidget->setCornerWidget(spinBoxLimitScriptLog, Qt::BottomRightCorner);

    // newly set tab-stops
    setTabOrder(ui->toolButtonActions, ui->plainTextEditScript);
    setTabOrder(ui->plainTextEditScript, ui->plainTextEditLog);
    setTabOrder(ui->plainTextEditLog, ui->treeWidgetProjectMonitor);
    setTabOrder(ui->treeWidgetProjectMonitor, ui->tabWidget);
    setTabOrder(ui->tabWidget, spinBoxLimitScriptLog);
    setTabOrder(spinBoxLimitScriptLog, ui->toolButtonRun);

    // adjust fonts and restore settings
    adjustFonts();
    restoreSettings();

    // set script-log's maximumBlockCount when the spin-box value changes
    connect(spinBoxLimitScriptLog, SIGNAL(valueChanged(int)), this, SLOT(setLogLimit(int)));

    // script-engine
    scriptEngine = new ScriptEngine(this);

    // linear gradient from red over yellow to green (this assumes that sub-window icons are in 64x64)
    linearGradient = QLinearGradient(QPointF(0, 0), QPointF(63, 0));
    linearGradient.setColorAt(0.0, QColor(255, 0, 0, 192));
    linearGradient.setColorAt(0.5, QColor(220, 220, 0, 192));
    linearGradient.setColorAt(1.0, QColor(0, 255, 0, 192));

    // setup tools-menu
    menuActions = new QMenu(this);
    actionLoad = menuActions->addAction(tr("Load"), this, SLOT(load()));
    actionSave = menuActions->addAction(tr("Save"), this, SLOT(save()));
    actionSaveAs = menuActions->addAction(tr("Save as..."), this, SLOT(triggerSaveAs()));
    actionClone = menuActions->addAction(tr("Clone"), this, SLOT(clone()));
    actionCopyLogToClipboard = menuActions->addAction(tr("Copy log to clipboard"), this, SLOT(copyLogToClipboard()));
    menuActions->insertSeparator(actionSave);
    menuActions->insertSeparator(actionClone);
    menuActions->insertSeparator(actionCopyLogToClipboard);
    ui->toolButtonActions->setMenu(menuActions);

    // syntax-highlighter
    new ECMAScriptHighlighter(ui->plainTextEditScript->document());
}

ScriptWidget::~ScriptWidget()
{
    delete ui;
    delete scriptEngine;
}

void ScriptWidget::on_toolButtonRun_clicked()
{
    ui->plainTextEditLog->clear();
    ui->toolButtonRun->setEnabled(false);
    ui->toolButtonStop->setEnabled(true);
    scriptEngine->externalStop = false;
    ui->progressBar->setFormat(tr("Running"));
    runningScripts++;
    isRunning = true;
    scriptEngine->runScript(ui->plainTextEditScript->toPlainText());
    isRunning = false;
    runningScripts--;
    ui->toolButtonRun->setEnabled(true);
    ui->toolButtonStop->setEnabled(false);
    QTimer::singleShot(0, this, SLOT(resetProgressBar()));
}

void ScriptWidget::on_toolButtonStop_clicked()
{
    scriptEngine->externalStop = true;
    scriptEngine->stopScript();
    isRunning = false;
    ui->toolButtonRun->setEnabled(true);
    ui->toolButtonStop->setEnabled(false);
    QTimer::singleShot(0, this, SLOT(resetProgressBar()));
}

void ScriptWidget::on_progressBar_valueChanged(int value)
{
    qreal percent = (qreal)value / (qreal)(ui->progressBar->maximum() - ui->progressBar->minimum());
    QPixmap pm(QIcon(":/images/script.png").pixmap(64, 64));
    QPainter p(&pm);
    int w = int((qreal)pm.height() * percent) + 1;
    p.fillRect(0, 47, 64, 16, QColor(64, 64, 64, 64));
    p.fillRect(0, 47, w, 16, QBrush(linearGradient));
    p.end();
    QIcon icon;
    icon.addPixmap(pm);
    parentWidget()->setWindowIcon(icon);
}

void ScriptWidget::adjustFonts()
{
    QFont f;

    f.fromString(globalConfig->preferencesEditorFont());
    f.setPointSize(globalConfig->preferencesEditorFontSize());
    ui->plainTextEditScript->setFont(f);

    QFontMetrics fmEditorFont(f);
    ui->plainTextEditScript->setTabStopWidth(fmEditorFont.width(' ') * 8);

    f.fromString(globalConfig->preferencesLogFont());
    f.setPointSize(globalConfig->preferencesLogFontSize());
    ui->plainTextEditLog->setFont(f);

    f.fromString(globalConfig->preferencesAppFont());
    f.setPointSize(globalConfig->preferencesAppFontSize());

    QFontMetrics fmAppFont(f);
    for (int i = 0; i < ui->treeWidgetProjectMonitor->topLevelItemCount(); i++) {
        QTreeWidgetItem *item = ui->treeWidgetProjectMonitor->topLevelItem(i);
        QProgressBar *progressBar = (QProgressBar *)ui->treeWidgetProjectMonitor->itemWidget(item, QCHDMAN_PRJMON_PROGRESS);
        if ( progressBar )
            progressBar->setFixedHeight(fmAppFont.height() + 2);
    }
}

void ScriptWidget::log(QString message)
{
    message.prepend(QTime::currentTime().toString("hh:mm:ss.zzz") + ": ");
    ui->plainTextEditLog->appendPlainText(message);
}

void ScriptWidget::load(const QString &fileName, QString *buffer)
{
    QString scriptName = fileName;

    if ( buffer == NULL && scriptName.isEmpty() ) {
        scriptName = QFileDialog::getOpenFileName(this, tr("Choose script file"), QString(), tr("Script files (*.scr)")  + ";;" + tr("All files (*)"), 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
        if ( scriptName.isNull() )
            return;
    }

    QFile loadFile(scriptName);
    QTextStream ts;
    QByteArray ba(buffer != NULL ? buffer->toUtf8().constData() : "");
    QBuffer baBuffer(&ba);

    if ( buffer != NULL ) {
        baBuffer.open(QBuffer::ReadOnly);
        ts.setDevice(&baBuffer);
    } else if ( loadFile.open(QIODevice::ReadOnly | QIODevice::Text) )
        ts.setDevice(&loadFile);

    if ( ts.device() ) {
        bool foundECMAScriptStart = false;
        while ( !ts.atEnd() && !foundECMAScriptStart )
            foundECMAScriptStart = ts.readLine().trimmed().startsWith("ECMAScript [");
        if ( foundECMAScriptStart ) {
            bool foundECMAScriptEnd = false;
            QString ecmaScript;
            QString lineSep;
            while ( !ts.atEnd() && !foundECMAScriptEnd ) {
                QString line = ts.readLine();
                if ( line.trimmed() == "]" )
                    foundECMAScriptEnd = true;
                else
                    ecmaScript += lineSep + line;
                lineSep = "\n";
            }
            ui->plainTextEditScript->setPlainText(ecmaScript);
        }

        if ( buffer != NULL ) {
            baBuffer.close();
        } else
            loadFile.close();

        if ( !scriptName.isEmpty() ) {
            parentWidget()->setWindowTitle(scriptName);
            mainWindow->addRecentScript(scriptName);
        }
    } else if ( !scriptName.isEmpty() )
        mainWindow->statusBar()->showMessage(tr("Failed loading script '%1'").arg(scriptName), QCHDMAN_STATUS_MSGTIME);
}

void ScriptWidget::save(QString *buffer)
{
    QString scriptName = ((ProjectWindow *)parentWidget())->projectName;

    if ( scriptName.startsWith(tr("Noname-%1").arg("")) )
        askFileName = true;

    saveAs(scriptName, buffer);

    askFileName = false;
}

void ScriptWidget::saveAs(const QString &fileName, QString *buffer)
{
    QString scriptName = fileName;

    if ( buffer == NULL && (scriptName.isEmpty() || askFileName) ) {
        scriptName = ((ProjectWindow *)parentWidget())->projectName;
        if ( scriptName.startsWith(tr("Noname-%1").arg("")) || scriptName.isEmpty() || askFileName ) {
            QString s = QFileDialog::getSaveFileName(this, tr("Choose script file"), scriptName, tr("Script files (*.scr)") + ";;" + tr("All files (*)"), 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
            if ( s.isNull() )
                return;
            else
                scriptName = s;
        }
    }

    QFile saveFile(scriptName);
    QTextStream ts;
    QByteArray ba;
    QBuffer baBuffer(&ba);

    if ( buffer != NULL ) {
        baBuffer.open(QBuffer::WriteOnly);
        ts.setDevice(&baBuffer);
    } else if ( saveFile.open(QIODevice::WriteOnly | QIODevice::Text) )
        ts.setDevice(&saveFile);

    if ( ts.device() ) {
        ts << "# " << tr("Qt CHDMAN GUI script file -- please do not edit manually") << "\n";
        ts << "ApplicationVersion = " << QCHDMAN_APP_VERSION << "\n";
        ts << "ScriptFormatVersion = " << QCHDMAN_SCR_FMT_VERSION << "\n";
        ts << "ECMAScript [" << "\n";
        ts << ui->plainTextEditScript->toPlainText() << "\n";
        ts << "]\n";

        if ( buffer != NULL ) {
            baBuffer.close();
            *buffer = QString(ba);
        } else
            saveFile.close();

        ((ProjectWindow *)parentWidget())->projectName = scriptName;
        parentWidget()->setWindowTitle(scriptName);
        mainWindow->addRecentScript(scriptName);
        if ( !scriptName.isEmpty() )
        mainWindow->statusBar()->showMessage(tr("Script '%1' saved").arg(scriptName), QCHDMAN_STATUS_MSGTIME);
    } else if ( !scriptName.isEmpty() )
        mainWindow->statusBar()->showMessage(tr("Failed saving script '%1'").arg(scriptName), QCHDMAN_STATUS_MSGTIME);
}

QString ScriptWidget::toString()
{
    QString buffer;
    save(&buffer);
    return buffer;
}

void ScriptWidget::fromString(QString buffer)
{
    load(QString(), &buffer);
}

void ScriptWidget::triggerSaveAs()
{
    askFileName = true;
    saveAs();
    askFileName = false;
}

void ScriptWidget::resetProgressBar()
{
    ui->progressBar->setFormat(tr("Idle"));
    ui->progressBar->setRange(0, 100);
    ui->progressBar->setValue(0);
    parentWidget()->setWindowIcon(QIcon(":/images/script.png"));
}

void ScriptWidget::clone()
{
    ProjectWindow *projectWindow = mainWindow->createProjectWindow(QCHDMAN_MDI_SCRIPT);
    projectWindow->scriptWidget->fromString(toString());
}

void ScriptWidget::copyLogToClipboard()
{
    qApp->clipboard()->setText(ui->plainTextEditLog->toPlainText());
}

void ScriptWidget::saveSettings()
{
    globalConfig->setScriptWidgetLogLimit(spinBoxLimitScriptLog->value());
    globalConfig->setScriptWidgetTabIndex(ui->tabWidget->currentIndex());
    globalConfig->setScriptWidgetSplitterState(ui->splitter->saveState());
    globalConfig->setScriptWidgetProjectMonitorHeaderState(ui->treeWidgetProjectMonitor->header()->saveState());
}

void ScriptWidget::restoreSettings()
{
    spinBoxLimitScriptLog->setValue(globalConfig->scriptWidgetLogLimit());
    setLogLimit(globalConfig->scriptWidgetLogLimit());
    ui->tabWidget->setCurrentIndex(globalConfig->scriptWidgetTabIndex());
    on_tabWidget_currentChanged(globalConfig->scriptWidgetTabIndex());
    QByteArray splitterState = globalConfig->scriptWidgetSplitterState();
    if ( !splitterState.isNull() )
        ui->splitter->restoreState(globalConfig->scriptWidgetSplitterState());
    else
        ui->splitter->setSizes(QList<int>() << 700 << 300);
    ui->treeWidgetProjectMonitor->header()->restoreState(globalConfig->scriptWidgetProjectMonitorHeaderState());
}

void ScriptWidget::setLogLimit(int limit)
{
    if ( limit < 100 ) {
        spinBoxLimitScriptLog->blockSignals(true);
        spinBoxLimitScriptLog->setValue(0);
        spinBoxLimitScriptLog->blockSignals(false);
        ui->plainTextEditLog->setMaximumBlockCount(0);
    } else
        ui->plainTextEditLog->setMaximumBlockCount(limit);
}

void ScriptWidget::on_tabWidget_currentChanged(int index)
{
    if ( index == QCHDMAN_SCRIPT_LOG_INDEX ) {
        spinBoxLimitScriptLog->show();
        ui->plainTextEditLog->verticalScrollBar()->setValue(ui->plainTextEditLog->verticalScrollBar()->maximum());
    } else
        spinBoxLimitScriptLog->hide();
}
