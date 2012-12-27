#include "arcademodesetup.h"
#include "macros.h"

ArcadeModeSetup::ArcadeModeSetup(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);

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
	toolButtonSelectC->setIconSize(iconSize);
	toolButtonSelectM->setIconSize(iconSize);
	toolButtonSelectI->setIconSize(iconSize);
	toolButtonSelectN->setIconSize(iconSize);
	toolButtonSelectU->setIconSize(iconSize);
	comboBoxSortOrder->setIconSize(iconSize);

	adjustSize();
}
