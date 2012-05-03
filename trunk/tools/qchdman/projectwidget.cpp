#include <QtGui>

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

void ProjectWidget::on_toolButtonRun_clicked()
{
    log(tr("run clicked"));
}

void ProjectWidget::on_splitter_splitterMoved(int, int)
{
    globalConfig->setProjectWidgetSplitterSizes(splitterSizes());
}

void ProjectWidget::log(QString message)
{
    message.prepend(QTime::currentTime().toString("hh:mm:ss.zzz") + ": ");
    ui->plainTextEditProjectLog->appendPlainText(message);
}
