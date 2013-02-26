#include "macros.h"
#include "scriptwidget.h"
#include "ui_scriptwidget.h"

ScriptWidget::ScriptWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScriptWidget)
{
    ui->setupUi(this);

    QList<int> splitterSizes;
    splitterSizes << 500 << 500;
    ui->vSplitter->setSizes(splitterSizes);
    splitterSizes.clear();
    splitterSizes << 250 << 750;
    ui->hSplitter->setSizes(splitterSizes);

    ui->tableWidgetInputOutput->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
    ui->tableWidgetInputOutput->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    ui->tableWidgetInputOutput->setVisible(false);

    parentWidget()->setWindowIcon(QIcon(":/images/script.png"));

    groupSeqNum = projectSeqNum = commandSeqNum = 0;
    inputOutputTableShownInitially = true;
}

ScriptWidget::~ScriptWidget()
{
    delete ui;
}

void ScriptWidget::on_toolButtonInputOutput_toggled(bool enable)
{
    ui->tableWidgetInputOutput->setVisible(enable);
    if ( inputOutputTableShownInitially ) {
        ui->tableWidgetInputOutput->horizontalHeader()->resizeSections(QHeaderView::Stretch);
        inputOutputTableShownInitially = false;
    }
}

void ScriptWidget::on_toolButtonRun_clicked()
{
}

void ScriptWidget::on_toolButtonStop_clicked()
{
}

void ScriptWidget::on_toolButtonAddGroup_clicked()
{
}

void ScriptWidget::on_toolButtonRemoveGroup_clicked()
{
}

void ScriptWidget::on_toolButtonAddProject_clicked()
{
}

void ScriptWidget::on_toolButtonRemoveProject_clicked()
{
}

void ScriptWidget::on_toolButtonAddCommand_clicked()
{
}

void ScriptWidget::on_toolButtonRemoveCommand_clicked()
{
}

void ScriptWidget::doCleanUp()
{
}

void ScriptWidget::resizeEvent(QResizeEvent *e)
{
    static int lastWidth = 0;
    if ( lastWidth != e->size().width() ) {
        ui->tableWidgetInputOutput->horizontalHeader()->resizeSections(QHeaderView::Stretch);
        lastWidth = e->size().width();
    }
}
