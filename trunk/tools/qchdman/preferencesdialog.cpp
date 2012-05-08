#include <QtGui>
#include <QFileDialog>
#include <QMessageBox>

#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"
#include "mainwindow.h"
#include "macros.h"
#include "settings.h"

extern Settings *globalConfig;
extern MainWindow *mainWindow;

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);
    ui->comboBoxStyle->addItems(QStyleFactory::keys());
    restoreSettings();
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

void PreferencesDialog::applySettings()
{
    // GUI
    globalConfig->setPreferencesGuiStyle(ui->comboBoxStyle->currentText());
    qApp->setStyle(globalConfig->preferencesGuiStyle());
    globalConfig->setPreferencesAppFont(ui->fontComboBoxAppFont->currentFont().toString());
    globalConfig->setPreferencesAppFontSize(ui->spinBoxAppFontSize->value());
    QFont f;
    f.fromString(globalConfig->preferencesAppFont());
    f.setPointSize(globalConfig->preferencesAppFontSize());
    qApp->setFont(f);
    globalConfig->setPreferencesLogFont(ui->fontComboBoxLogFont->currentFont().toString());
    globalConfig->setPreferencesLogFontSize(ui->spinBoxLogFontSize->value());
    globalConfig->setPreferencesShowHelpTexts(ui->checkBoxShowProjectHelp->isChecked());
    globalConfig->setPreferencesMaximizeWindows(ui->checkBoxMaximizeWindows->isChecked());

    // Paths
    globalConfig->setPreferencesChdmanBinary(ui->lineEditChdmanBinary->text());

    mainWindow->applySettings();
}

void PreferencesDialog::on_pushButtonOk_clicked()
{
    applySettings();
}

void PreferencesDialog::on_pushButtonApply_clicked()
{
    applySettings();
    adjustSize();
}

void PreferencesDialog::on_toolButtonBrowseChdmanBinary_clicked()
{
    QString s = QFileDialog::getOpenFileName(this, tr("Choose CHDMAN binary"), ui->lineEditChdmanBinary->text(), tr("All files (*)"));
    if ( !s.isNull() )
        ui->lineEditChdmanBinary->setText(s);
}

void PreferencesDialog::restoreSettings()
{
    // GUI
    int i = ui->comboBoxStyle->findText(globalConfig->preferencesGuiStyle());
    if ( i >= 0 )
        ui->comboBoxStyle->setCurrentIndex(i);
    QFont f;
    if ( !f.fromString(globalConfig->preferencesAppFont()) )
        f = qApp->font();
    ui->fontComboBoxAppFont->setCurrentFont(f);
    ui->spinBoxAppFontSize->setValue(globalConfig->preferencesAppFontSize());
    if ( !f.fromString(globalConfig->preferencesLogFont()) )
        f = qApp->font();
    ui->fontComboBoxLogFont->setCurrentFont(f);
    ui->spinBoxLogFontSize->setValue(globalConfig->preferencesLogFontSize());
    ui->checkBoxShowProjectHelp->setChecked(globalConfig->preferencesShowHelpTexts());
    ui->checkBoxMaximizeWindows->setChecked(globalConfig->preferencesMaximizeWindows());

    // Paths
    ui->lineEditChdmanBinary->setText(globalConfig->preferencesChdmanBinary());
}

void PreferencesDialog::showEvent(QShowEvent *e)
{
    adjustSize();
    QDialog::showEvent(e);
}
