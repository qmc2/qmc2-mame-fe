#include <QFileDialog>

#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"
#include "mainwindow.h"
#include "macros.h"
#include "settings.h"

extern Settings *globalConfig;
extern MainWindow *mW;

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::initialSetup()
{
    ui->tabWidget->setCurrentWidget(ui->tabPaths);
    ui->lineEditChdmanBinary->setFocus();
    exec();
}

void PreferencesDialog::on_pushButtonOk_clicked()
{
    // GUI

    // Paths
    globalConfig->setPreferencesChdmanBinary(ui->lineEditChdmanBinary->text());
}

void PreferencesDialog::on_toolButtonBrowseChdmanBinary_clicked()
{
    QString s = QFileDialog::getOpenFileName(this, tr("Choose CHDMAN binary"), ui->lineEditChdmanBinary->text(), tr("All files (*)"));
    if ( !s.isNull() )
        ui->lineEditChdmanBinary->setText(s);
}

void PreferencesDialog::showEvent(QShowEvent *e)
{
    // GUI

    // Paths
    ui->lineEditChdmanBinary->setText(globalConfig->preferencesChdmanBinary());

    QDialog::showEvent(e);
}
