#include <QFileDialog>
#include <QInputDialog>

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

#if QT_VERSION < 0x050000
    ui->tableWidgetInputOutput->horizontalHeader()->setResizeMode(QHeaderView::Interactive);
#else
    ui->tableWidgetInputOutput->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
#endif
    ui->tableWidgetInputOutput->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    ui->tableWidgetInputOutput->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableWidgetInputOutput->horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(tableWidgetInputOutput_sectionClicked(int)));
    ui->tableWidgetInputOutput->setVisible(false);

    parentWidget()->setWindowIcon(QIcon(":/images/script.png"));
    ioVariableNames << "$INPUT1$" << "$INPUT2$" << "$OUTPUT1$" << "$OUTPUT2$";

    ioHeaderMenu = new QMenu(this);
    connect(ioHeaderMenu->addAction(QIcon(":/images/edit.png"), tr("Edit variable name")), SIGNAL(triggered(bool)), this, SLOT(changeVariableName()));
    connect(ui->tableWidgetInputOutput->horizontalHeader(), SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(tableWidgetInputOutput_horizontalHeader_customContextMenuRequested(const QPoint &)));

    groupSeqNum = projectSeqNum = commandSeqNum = lastWidgetWidth = 0;
    ioHeaderLogicalIndex = -1;
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
    QStringList sl = QFileDialog::getOpenFileNames(this, tr("Choose files") + " (" + ioVariableNames[logicalIndex] + ")",
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

void ScriptWidget::changeVariableName()
{
    if ( ioHeaderLogicalIndex >= 0 ) {
        QString varName = ioVariableNames[ioHeaderLogicalIndex];
        bool ok;
        varName = QInputDialog::getText(this, tr("Edit variable name"),
                                        tr("Enter a new <b>unique</b> variable name (variable names are case-sensitive, any $-characters<br>"
                                           "will be removed and all white-space characters will be replaced with underscores)") + ":",
                                        QLineEdit::Normal, varName.remove("$"), &ok);
        if ( ok ) {
            varName.replace(QRegExp("\\s"), "_").replace("$", "");
            varName.prepend("$"); varName.append("$");
            if ( varName != "$$" && !ioVariableNames.contains(varName) ) {
                ioVariableNames[ioHeaderLogicalIndex] = varName;
                ui->tableWidgetInputOutput->setHorizontalHeaderLabels(ioVariableNames);
            }
        }
    }
}

void ScriptWidget::tableWidgetInputOutput_horizontalHeader_customContextMenuRequested(const QPoint &pos)
{
    ioHeaderLogicalIndex = ui->tableWidgetInputOutput->horizontalHeader()->logicalIndexAt(pos);
    ioHeaderMenu->move(ui->tableWidgetInputOutput->horizontalHeader()->viewport()->mapToGlobal(pos));
    ioHeaderMenu->show();
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
