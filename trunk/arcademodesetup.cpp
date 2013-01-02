#include "arcademodesetup.h"
#include "macros.h"

ArcadeModeSetup::ArcadeModeSetup(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);

	// FIXME: keyboard- & joystick-mappings are not supported yet
#if !defined(QMC2_WIP_ENABLED) 
	tabWidget->removeTab(tabKeys);
	tabWidget->removeTab(tabJoystick);
#endif

	adjustIconSizes();
}

ArcadeModeSetup::~ArcadeModeSetup()
{
}

void ArcadeModeSetup::adjustIconSizes()
{
	QFontMetrics fm(qApp->font());
	QSize iconSize(fm.height() - 2, fm.height() - 2);

	toolButtonBrowseExecutableFile->setIconSize(iconSize);
	toolButtonBrowseWorkingDirectory->setIconSize(iconSize);
	toolButtonBrowseConfigurationPath->setIconSize(iconSize);
	toolButtonBrowseFilteredListFile->setIconSize(iconSize);
	pushButtonExport->setIconSize(iconSize);
	toolButtonSelectC->setIconSize(iconSize);
	toolButtonSelectM->setIconSize(iconSize);
	toolButtonSelectI->setIconSize(iconSize);
	toolButtonSelectN->setIconSize(iconSize);
	toolButtonSelectU->setIconSize(iconSize);
	comboBoxSortOrder->setIconSize(iconSize);
	toolButtonClearNameFilter->setIconSize(iconSize);

	adjustSize();
}

void ArcadeModeSetup::on_checkBoxUseFilteredList_toggled(bool enable)
{
	lineEditFilteredListFile->setEnabled(enable);
	toolButtonBrowseFilteredListFile->setEnabled(enable);
	pushButtonExport->setEnabled(enable);
	progressBarFilter->setEnabled(enable);
	toolButtonSelectC->setEnabled(enable);
	toolButtonSelectM->setEnabled(enable);
	toolButtonSelectI->setEnabled(enable);
	toolButtonSelectN->setEnabled(enable);
	toolButtonSelectU->setEnabled(enable);
	comboBoxSortCriteria->setEnabled(enable);
	comboBoxSortOrder->setEnabled(enable);
	comboBoxDriverStatus->setEnabled(enable);
	lineEditNameFilter->setEnabled(enable);
	toolButtonClearNameFilter->setEnabled(enable);
	listWidgetCategoryFilter->setEnabled(enable);
}
