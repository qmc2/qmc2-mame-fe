#include <QApplication>

#include "rompathcleaner.h"

RomPathCleaner::RomPathCleaner(QWidget *parent) :
	QWidget(parent)
{
	setupUi(this);
}

void RomPathCleaner::adjustIconSizes()
{
	QFont f(qApp->font());
	QFontMetrics fm(f);
	QSize iconSize(fm.height() - 2, fm.height() - 2);
	pushButtonStartStop->setIconSize(iconSize);
}
