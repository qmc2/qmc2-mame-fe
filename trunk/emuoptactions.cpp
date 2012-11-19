#include <QApplication>
#include <QFontMetrics>

#include "emuoptactions.h"
#include "qmc2main.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;

EmulatorOptionActions::EmulatorOptionActions(QTreeWidgetItem *item, QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);

	// FIXME
	toolButtonReset->setEnabled(false);
	toolButtonRevert->setEnabled(false);
	toolButtonStore->setEnabled(false);

	myItem = item;
	adjustIconSizes();
}

EmulatorOptionActions::~EmulatorOptionActions()
{
}

void EmulatorOptionActions::on_toolButtonReset_clicked()
{
}

void EmulatorOptionActions::on_toolButtonRevert_clicked()
{

}

void EmulatorOptionActions::on_toolButtonStore_clicked()
{
}

void EmulatorOptionActions::adjustIconSizes()
{
	QFontMetrics fm(QApplication::font());
	QSize iconSize(fm.height() - 2, fm.height() - 2);
	toolButtonReset->setIconSize(iconSize);
	toolButtonRevert->setIconSize(iconSize);
	toolButtonStore->setIconSize(iconSize);
}
