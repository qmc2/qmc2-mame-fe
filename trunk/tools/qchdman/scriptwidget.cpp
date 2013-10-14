#include <QTime>

#include "ui_scriptwidget.h"
#include "scriptwidget.h"
#include "mainwindow.h"
#include "settings.h"
#include "macros.h"

extern Settings *globalConfig;
extern MainWindow *mainWindow;

ScriptWidget::ScriptWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScriptWidget)
{
    ui->setupUi(this);

    parentWidget()->setWindowIcon(QIcon(":/images/script.png"));

    QFont f;

    f.fromString(globalConfig->preferencesEditorFont());
    f.setPointSize(globalConfig->preferencesEditorFontSize());
    ui->textEditScript->setFont(f);

    f.fromString(globalConfig->preferencesLogFont());
    f.setPointSize(globalConfig->preferencesLogFontSize());
    ui->plainTextEditLog->setFont(f);

    QList<int> sizes;
    sizes << 700 << 300;
    ui->splitter->setSizes(sizes);
    scriptEngine = new ScriptEngine(this);

    askFileName = false;
}

ScriptWidget::~ScriptWidget()
{
    delete ui;
    delete scriptEngine;
}

void ScriptWidget::on_toolButtonRun_clicked()
{
    ui->plainTextEditLog->clear();
    scriptEngine->runScript(ui->textEditScript->toPlainText());
}

void ScriptWidget::on_toolButtonStop_clicked()
{
    scriptEngine->stopScript();
}

void ScriptWidget::log(QString message)
{
    message.prepend(QTime::currentTime().toString("hh:mm:ss.zzz") + ": ");
    ui->plainTextEditLog->appendPlainText(message);
    ui->plainTextEditLog->verticalScrollBar()->setValue(ui->plainTextEditLog->verticalScrollBar()->maximum());
}

void ScriptWidget::load(const QString &fileName, QString *buffer)
{
    QString scriptName = fileName;

    if ( buffer == NULL && scriptName.isEmpty() ) {
        scriptName = QFileDialog::getOpenFileName(this, tr("Choose script file"), QString(), tr("All files (*)") + ";;" + tr("Script files (*.scr)"), 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
        if ( scriptName.isNull() )
            return;
    }

    QFile loadFile(scriptName);
    QTextStream ts;
    QByteArray ba;
    QBuffer baBuffer(&ba);

    if ( buffer != NULL ) {
        baBuffer.open(QBuffer::ReadWrite);
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
            ui->textEditScript->setPlainText(ecmaScript);
        }
        mainWindow->addRecentScript(scriptName);
    } else
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
            QString s = QFileDialog::getSaveFileName(this, tr("Choose script file"), scriptName, tr("All files (*)") + ";;" + tr("Script files (*.scr)"), 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
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
        baBuffer.open(QBuffer::ReadWrite);
        ts.setDevice(&baBuffer);
    } else if ( saveFile.open(QIODevice::WriteOnly | QIODevice::Text) )
        ts.setDevice(&saveFile);

    if ( ts.device() ) {
        ts << "# " << tr("Qt CHDMAN GUI script file -- please do not edit manually") << "\n";
        ts << "ApplicationVersion = " << QCHDMAN_APP_VERSION << "\n";
        ts << "ScriptFormatVersion = " << QCHDMAN_SCR_FMT_VERSION << "\n";
        ts << "ECMAScript [" << "\n";
        ts << ui->textEditScript->toPlainText() << "\n";
        ts << "]\n";

        if ( buffer != NULL )
            *buffer = QString(ba);
        else
            saveFile.close();

        ((ProjectWindow *)parentWidget())->projectName = scriptName;
        parentWidget()->setWindowTitle(scriptName);
        mainWindow->addRecentScript(scriptName);
        mainWindow->statusBar()->showMessage(tr("Script '%1' saved").arg(scriptName), QCHDMAN_STATUS_MSGTIME);
    } else
        mainWindow->statusBar()->showMessage(tr("Failed saving script '%1'").arg(scriptName), QCHDMAN_STATUS_MSGTIME);
}

QString ScriptWidget::toString()
{
    QString buffer;
    save(&buffer);
    return buffer;
}

void ScriptWidget::fromString(QString)
{
    // FIXME
}

void ScriptWidget::triggerSaveAs()
{
    askFileName = true;
    saveAs();
    askFileName = false;
}

void ScriptWidget::closeEvent(QCloseEvent *)
{
    scriptEngine->externalStop = true;
}
