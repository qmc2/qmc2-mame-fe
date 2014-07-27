#include <QFileDialog>

#include "settings.h"
#include "mawsqdlsetup.h"
#include "options.h"
#include "qmc2main.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;
extern Options *qmc2Options;

MawsQuickDownloadSetup::MawsQuickDownloadSetup(QWidget *parent)
	: QDialog(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MawsQuickDownloadSetup::MawsQuickDownloadSetup(QWidget *parent = %1)").arg((qulonglong) parent));
#endif

	setupUi(this);

	adjustIconSizes();
	adjustSize();

	// this just reads the current configuration, doesn't cancel the dialog
	on_pushButtonCancel_clicked();
}

MawsQuickDownloadSetup::~MawsQuickDownloadSetup()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MawsQuickDownloadSetup::~MawsQuickDownloadSetup()");
#endif

}

void MawsQuickDownloadSetup::adjustIconSizes()
{
	QFontMetrics fm(QApplication::font());
	QSize iconSize(fm.height() - 2, fm.height() - 2);
	pushButtonOk->setIconSize(iconSize);
	pushButtonCancel->setIconSize(iconSize);
	toolButtonBrowseIconDirectory->setIconSize(iconSize);
	toolButtonBrowseFlyerDirectory->setIconSize(iconSize);
	toolButtonBrowseCabinetDirectory->setIconSize(iconSize);
	toolButtonBrowseControllerDirectory->setIconSize(iconSize);
	toolButtonBrowseMarqueeDirectory->setIconSize(iconSize);
	toolButtonBrowsePCBDirectory->setIconSize(iconSize);
	toolButtonBrowsePreviewDirectory->setIconSize(iconSize);
	toolButtonBrowseTitleDirectory->setIconSize(iconSize);
}

void MawsQuickDownloadSetup::on_pushButtonOk_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MawsQuickDownloadSetup::on_pushButtonOk_clicked()");
#endif

	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "MAWS/AutoDownloadIcons", checkBoxAutoIcons->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "MAWS/IconDirectory", lineEditIconDirectory->text());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "MAWS/AutoDownloadFlyers", checkBoxAutoFlyers->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "MAWS/FlyerDirectory", lineEditFlyerDirectory->text());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "MAWS/AutoDownloadCabinets", checkBoxAutoCabinets->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "MAWS/CabinetDirectory", lineEditCabinetDirectory->text());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "MAWS/AutoDownloadControllers", checkBoxAutoControllers->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "MAWS/ControllerDirectory", lineEditControllerDirectory->text());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "MAWS/AutoDownloadMarquees", checkBoxAutoMarquees->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "MAWS/MarqueeDirectory", lineEditMarqueeDirectory->text());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "MAWS/AutoDownloadPCBs", checkBoxAutoPCBs->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "MAWS/PCBDirectory", lineEditPCBDirectory->text());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "MAWS/AutoDownloadPreviews", checkBoxAutoPreviews->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "MAWS/PreviewDirectory", lineEditPreviewDirectory->text());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "MAWS/PreferredPreviewCollection", comboBoxPreferredPreviewCollection->currentText());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "MAWS/AutoDownloadTitles", checkBoxAutoTitles->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "MAWS/TitleDirectory", lineEditTitleDirectory->text());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "MAWS/PreferredTitleCollection", comboBoxPreferredTitleCollection->currentText());
}

void MawsQuickDownloadSetup::on_pushButtonCancel_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MawsQuickDownloadSetup::on_pushButtonCancel_clicked()");
#endif

	checkBoxAutoIcons->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/AutoDownloadIcons", false).toBool());
	lineEditIconDirectory->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/IconDirectory", QMC2_DEFAULT_DATA_PATH + "/ico/").toString());
	checkBoxAutoFlyers->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/AutoDownloadFlyers", false).toBool());
	lineEditFlyerDirectory->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/FlyerDirectory", QMC2_DEFAULT_DATA_PATH + "/fly/").toString());
	checkBoxAutoCabinets->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/AutoDownloadCabinets", false).toBool());
	lineEditCabinetDirectory->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/CabinetDirectory", QMC2_DEFAULT_DATA_PATH + "/cab/").toString());
	checkBoxAutoControllers->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/AutoDownloadControllers", false).toBool());
	lineEditControllerDirectory->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/ControllerDirectory", QMC2_DEFAULT_DATA_PATH + "/ctl/").toString());
	checkBoxAutoMarquees->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/AutoDownloadMarquees", false).toBool());
	lineEditMarqueeDirectory->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/MarqueeDirectory", QMC2_DEFAULT_DATA_PATH + "/mrq/").toString());
	checkBoxAutoPCBs->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/AutoDownloadPCBs", false).toBool());
	lineEditPCBDirectory->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/PCBDirectory", QMC2_DEFAULT_DATA_PATH + "/pcb/").toString());
	checkBoxAutoPreviews->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/AutoDownloadPreviews", false).toBool());
	lineEditPreviewDirectory->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/PreviewDirectory", QMC2_DEFAULT_DATA_PATH + "/prv/").toString());
	int i = comboBoxPreferredPreviewCollection->findText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/PreferredPreviewCollection", tr("AntoPISA progettoSNAPS")).toString());
	if ( i >= 0 )
		comboBoxPreferredPreviewCollection->setCurrentIndex(i);
	checkBoxAutoTitles->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/AutoDownloadTitles", false).toBool());
	lineEditTitleDirectory->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/TitleDirectory", QMC2_DEFAULT_DATA_PATH + "/ttl/").toString());
	i = comboBoxPreferredTitleCollection->findText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "MAWS/PreferredTitleCollection", tr("AntoPISA progettoSNAPS")).toString());
	if ( i >= 0 )
		comboBoxPreferredTitleCollection->setCurrentIndex(i);
}

void MawsQuickDownloadSetup::on_toolButtonBrowseIconDirectory_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MawsQuickDownloadSetup::on_toolButtonBrowseIconDirectory_clicked()");
#endif

	QString s = QFileDialog::getExistingDirectory(this, tr("Choose icon directory"), lineEditIconDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() ) {
		if ( !s.endsWith("/") )
			s += "/";
		lineEditIconDirectory->setText(s);
	}
}

void MawsQuickDownloadSetup::on_toolButtonBrowseFlyerDirectory_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MawsQuickDownloadSetup::on_toolButtonBrowseFlyerDirectory_clicked()");
#endif

	QString s = QFileDialog::getExistingDirectory(this, tr("Choose flyer directory"), lineEditFlyerDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() ) {
		if ( !s.endsWith("/") )
			s += "/";
		lineEditFlyerDirectory->setText(s);
	}
}

void MawsQuickDownloadSetup::on_toolButtonBrowseCabinetDirectory_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MawsQuickDownloadSetup::on_toolButtonBrowseCabinetDirectory_clicked()");
#endif

	QString s = QFileDialog::getExistingDirectory(this, tr("Choose cabinet directory"), lineEditCabinetDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() ) {
		if ( !s.endsWith("/") )
			s += "/";
		lineEditCabinetDirectory->setText(s);
	}
}

void MawsQuickDownloadSetup::on_toolButtonBrowseControllerDirectory_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MawsQuickDownloadSetup::on_toolButtonBrowseControllerDirectory_clicked()");
#endif

	QString s = QFileDialog::getExistingDirectory(this, tr("Choose controller directory"), lineEditControllerDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() ) {
		if ( !s.endsWith("/") )
			s += "/";
		lineEditControllerDirectory->setText(s);
	}
}

void MawsQuickDownloadSetup::on_toolButtonBrowseMarqueeDirectory_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MawsQuickDownloadSetup::on_toolButtonBrowseMarqueeDirectory_clicked()");
#endif

	QString s = QFileDialog::getExistingDirectory(this, tr("Choose marquee directory"), lineEditMarqueeDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() ) {
		if ( !s.endsWith("/") )
			s += "/";
		lineEditMarqueeDirectory->setText(s);
	}
}

void MawsQuickDownloadSetup::on_toolButtonBrowsePCBDirectory_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MawsQuickDownloadSetup::on_toolButtonBrowsePCBDirectory_clicked()");
#endif

	QString s = QFileDialog::getExistingDirectory(this, tr("Choose PCB directory"), lineEditPCBDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() ) {
		if ( !s.endsWith("/") )
			s += "/";
		lineEditPCBDirectory->setText(s);
	}
}

void MawsQuickDownloadSetup::on_toolButtonBrowsePreviewDirectory_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MawsQuickDownloadSetup::on_toolButtonBrowsePreviewDirectory_clicked()");
#endif

	QString s = QFileDialog::getExistingDirectory(this, tr("Choose preview directory"), lineEditPreviewDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() ) {
		if ( !s.endsWith("/") )
			s += "/";
		lineEditPreviewDirectory->setText(s);
	}
}

void MawsQuickDownloadSetup::on_toolButtonBrowseTitleDirectory_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MawsQuickDownloadSetup::on_toolButtonBrowseTitleDirectory_clicked()");
#endif

	QString s = QFileDialog::getExistingDirectory(this, tr("Choose title directory"), lineEditTitleDirectory->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !s.isNull() ) {
		if ( !s.endsWith("/") )
			s += "/";
		lineEditTitleDirectory->setText(s);
	}
}
