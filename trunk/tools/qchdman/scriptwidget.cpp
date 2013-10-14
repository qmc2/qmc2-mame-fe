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
    message.prepend(QTime::currentTime().toString("hh:mm:ss.zzz") + ": " + tr("Script") + ": ");
    ui->plainTextEditLog->appendPlainText(message);
    ui->plainTextEditLog->verticalScrollBar()->setValue(ui->plainTextEditLog->verticalScrollBar()->maximum());
}
