#include "ui_scriptwidget.h"
#include "macros.h"
#include "scriptwidget.h"
#include "mainwindow.h"
#include "macros.h"
#include "settings.h"

extern Settings *globalConfig;
extern MainWindow *mainWindow;

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
    connect(ui->tableWidgetInputOutput->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(tableWidgetInputOutput_sectionClicked(int)));

    parentWidget()->setWindowIcon(QIcon(":/images/script.png"));

    groupSeqNum = projectSeqNum = commandSeqNum = lastWidgetWidth = 0;
    resizePending = true;
}

ScriptWidget::~ScriptWidget()
{
    delete ui;
}

void ScriptWidget::on_toolButtonInputOutput_toggled(bool enable)
{
    ui->tableWidgetInputOutput->setVisible(enable);
    if ( resizePending ) {
        ui->tableWidgetInputOutput->horizontalHeader()->resizeSections(QHeaderView::Stretch);
        resizePending = false;
    }
}

void ScriptWidget::on_toolButtonRun_clicked()
{
    // FIXME
}

void ScriptWidget::on_toolButtonStop_clicked()
{
    // FIXME
}

void ScriptWidget::on_toolButtonAddGroup_clicked()
{
    // FIXME
}

void ScriptWidget::on_toolButtonRemoveGroup_clicked()
{
    // FIXME
}

void ScriptWidget::on_toolButtonAddProject_clicked()
{
    // FIXME
}

void ScriptWidget::on_toolButtonRemoveProject_clicked()
{
    // FIXME
}

void ScriptWidget::on_toolButtonAddCommand_clicked()
{
    // FIXME
}

void ScriptWidget::on_toolButtonRemoveCommand_clicked()
{
    // FIXME
}

void ScriptWidget::tableWidgetInputOutput_sectionClicked(int logicalIndex)
{
    QStringList sl = QFileDialog::getOpenFileNames(this, tr("Choose files") + QString(logicalIndex < 2 ? " ($INPUT%1$)" : " ($OUTPUT%1$)").arg(logicalIndex < 2 ? logicalIndex + 1 : logicalIndex - 1),
                                                   logicalIndex < 2 ? mainWindow->preferredInputFolder : mainWindow->preferredOutputFolder, tr("All files (*)"), 0,
                                                   globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
    if ( !sl.isEmpty() ) {
        // FIXME
        QCHDMAN_PRINT_STRLST(sl);
    }
}

void ScriptWidget::doCleanUp()
{
    // FIXME
}

void ScriptWidget::doPendingResize()
{
    if ( ui->tableWidgetInputOutput->isVisible() && resizePending ) {
        ui->tableWidgetInputOutput->horizontalHeader()->resizeSections(QHeaderView::Stretch);
        resizePending = false;
    }
}

void ScriptWidget::resizeEvent(QResizeEvent *e)
{
    bool doResize = lastWidgetWidth != e->size().width();

    if ( doResize && globalConfig->mainWindowViewMode() == QCHDMAN_VIEWMODE_TABBED )
        doResize &= mainWindow->mdiArea()->activeSubWindow() == parentWidget();

    if ( doResize && ui->tableWidgetInputOutput->isVisible() ) {
        ui->tableWidgetInputOutput->horizontalHeader()->resizeSections(QHeaderView::Stretch);
    } else if ( doResize && !ui->tableWidgetInputOutput->isVisible() )
        resizePending = true;

    if ( resizePending && globalConfig->mainWindowViewMode() == QCHDMAN_VIEWMODE_WINDOWED )
        QTimer::singleShot(0, this, SLOT(doPendingResize()));

    lastWidgetWidth = e->size().width();
}
