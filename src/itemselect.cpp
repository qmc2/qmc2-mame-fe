#include "itemselect.h"
#include "macros.h"

ItemSelector::ItemSelector(QWidget *parent, QStringList &items)
	: QDialog(parent)
{
	setupUi(this);
	listWidgetItems->addItems(items);
}
