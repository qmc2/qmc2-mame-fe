#include <QApplication>

#include "qmc2main.h"
#include "rompathcleaner.h"

extern MainWindow *qmc2MainWindow;

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
	comboBoxCheckedPath->setIconSize(iconSize);
}

void RomPathCleaner::on_pushButtonStartStop_clicked()
{
	// FIXME
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("FIXME: RomPathCleaner::on_pushButtonStartStop_clicked(): not implemented yet!"));
}

void RomPathCleaner::on_comboBoxCheckedPath_activated(int index)
{
	// FIXME
	if ( index == QMC2_RPC_INDEX_SELECT_PATH )
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("FIXME: RomPathCleaner::on_comboBoxCheckedPath_activated(): not implemented yet!"));
}
