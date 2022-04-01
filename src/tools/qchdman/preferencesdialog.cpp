#include <QtGui>
#include <QFileDialog>
#include <QMessageBox>
#include <QStyleFactory>

#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"
#include "mainwindow.h"
#include "macros.h"
#include "qchdmansettings.h"

extern QtChdmanGuiSettings *globalConfig;
extern MainWindow *mainWindow;

PreferencesDialog::PreferencesDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::PreferencesDialog)
{
	ui->setupUi(this);
	setModal(true);
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
	QString lang = ui->comboBoxLanguage->currentText();
	globalConfig->setPreferencesLanguage(lang.mid(lang.indexOf("(") + 1, 2));
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
	globalConfig->setPreferencesEditorFont(ui->fontComboBoxEditorFont->currentFont().toString());
	globalConfig->setPreferencesEditorFontSize(ui->spinBoxEditorFontSize->value());
	globalConfig->setPreferencesShowHelpTexts(ui->checkBoxShowProjectHelp->isChecked());
	globalConfig->setPreferencesMaximizeWindows(ui->checkBoxMaximizeWindows->isChecked());
	globalConfig->setPreferencesNativeFileDialogs(ui->checkBoxNativeFileDialogs->isChecked());
	globalConfig->setPreferencesLogChannelNames(ui->checkBoxLogChannelNames->isChecked());

	// Paths
	globalConfig->setPreferencesChdmanBinary(ui->lineEditChdmanBinary->text());
	QFileInfo chdmanInfo(ui->lineEditChdmanBinary->text());
	if ( chdmanInfo.exists() && chdmanInfo.isExecutable() )
		ui->labelChdmanWarningPixmap->hide();
	else
		ui->labelChdmanWarningPixmap->show();
	globalConfig->setPreferencesPreferredCHDInputPath(ui->lineEditPreferredCHDInputPath->text());
	globalConfig->setPreferencesPreferredInputPath(ui->lineEditPreferredInputPath->text());
	globalConfig->setPreferencesPreferredCHDOutputPath(ui->lineEditPreferredCHDOutputPath->text());
	globalConfig->setPreferencesPreferredOutputPath(ui->lineEditPreferredOutputPath->text());

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

void PreferencesDialog::on_pushButtonCancel_clicked()
{
	restoreSettings();
}

void PreferencesDialog::on_toolButtonBrowseChdmanBinary_clicked()
{
	QString s = QFileDialog::getOpenFileName(this, tr("Choose CHDMAN binary"), ui->lineEditChdmanBinary->text(), tr("All files (*)"), 0, globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditChdmanBinary->setText(s);
}

void PreferencesDialog::on_toolButtonBrowsePreferredCHDInputPath_clicked()
{
	QString s = QFileDialog::getExistingDirectory(this, tr("Choose preferred CHD input path"), ui->lineEditPreferredCHDInputPath->text(), globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditPreferredCHDInputPath->setText(s);
}

void PreferencesDialog::on_toolButtonBrowsePreferredInputPath_clicked()
{
	QString s = QFileDialog::getExistingDirectory(this, tr("Choose preferred non-CHD input path"), ui->lineEditPreferredInputPath->text(), globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditPreferredInputPath->setText(s);
}

void PreferencesDialog::on_toolButtonBrowsePreferredCHDOutputPath_clicked()
{
	QString s = QFileDialog::getExistingDirectory(this, tr("Choose preferred CHD output path"), ui->lineEditPreferredCHDOutputPath->text(), globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditPreferredCHDOutputPath->setText(s);
}

void PreferencesDialog::on_toolButtonBrowsePreferredOutputPath_clicked()
{
	QString s = QFileDialog::getExistingDirectory(this, tr("Choose preferred non-CHD output path"), ui->lineEditPreferredOutputPath->text(), globalConfig->preferencesNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() )
		ui->lineEditPreferredOutputPath->setText(s);
}

void PreferencesDialog::restoreSettings()
{
	// GUI
	QString lang = globalConfig->preferencesLanguage();
	int i = ui->comboBoxLanguage->findText("(" + lang + ")", Qt::MatchEndsWith);
	if ( i >= 0 )
		ui->comboBoxLanguage->setCurrentIndex(i);
	else
		ui->comboBoxLanguage->setCurrentIndex(ui->comboBoxLanguage->findText("(us)", Qt::MatchEndsWith));

	i = ui->comboBoxStyle->findText(globalConfig->preferencesGuiStyle());
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
	if ( !f.fromString(globalConfig->preferencesEditorFont()) )
		f = qApp->font();
	ui->fontComboBoxEditorFont->setCurrentFont(f);
	ui->spinBoxEditorFontSize->setValue(globalConfig->preferencesEditorFontSize());
	ui->checkBoxShowProjectHelp->setChecked(globalConfig->preferencesShowHelpTexts());
	ui->checkBoxMaximizeWindows->setChecked(globalConfig->preferencesMaximizeWindows());
	ui->checkBoxNativeFileDialogs->setChecked(globalConfig->preferencesNativeFileDialogs());
	ui->checkBoxLogChannelNames->setChecked(globalConfig->preferencesLogChannelNames());

	// Paths
	ui->lineEditChdmanBinary->setText(globalConfig->preferencesChdmanBinary());
	QFileInfo chdmanInfo(ui->lineEditChdmanBinary->text());
	if ( chdmanInfo.exists() && chdmanInfo.isExecutable() )
		ui->labelChdmanWarningPixmap->hide();
	else
		ui->labelChdmanWarningPixmap->show();
	ui->lineEditPreferredCHDInputPath->setText(globalConfig->preferencesPreferredCHDInputPath());
	ui->lineEditPreferredInputPath->setText(globalConfig->preferencesPreferredInputPath());
	ui->lineEditPreferredCHDOutputPath->setText(globalConfig->preferencesPreferredCHDOutputPath());
	ui->lineEditPreferredOutputPath->setText(globalConfig->preferencesPreferredOutputPath());
}

void PreferencesDialog::showEvent(QShowEvent *e)
{
	adjustSize();
	QDialog::showEvent(e);
}
