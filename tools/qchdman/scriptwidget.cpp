#include "scriptwidget.h"
#include "ui_scriptwidget.h"

ScriptWidget::ScriptWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScriptWidget)
{
    ui->setupUi(this);

    QList<int> splitterSizes;
    splitterSizes << 500 << 500;
    ui->splitter->setSizes(splitterSizes);

    parentWidget()->setWindowIcon(QIcon(":/images/script.png"));
}

ScriptWidget::~ScriptWidget()
{
    delete ui;
}

void ScriptWidget::doCleanUp()
{
}
