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

void ProjectWidget::on_toolButtonRun_clicked()
{
    log(tr("run clicked"));
}

void ProjectWidget::on_splitter_splitterMoved(int, int)
{
    globalConfig->setProjectWidgetSplitterSizes(splitterSizes());
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
    message.prepend(QTime::currentTime().toString("hh:mm:ss.zzz") + ": ");
    ui->plainTextEditProjectLog->appendPlainText(message);
}
