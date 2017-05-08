#include <QApplication>
#include <QFileDialog>

#include "qmc2main.h"
#include "options.h"
#include "settings.h"
#include "rompathcleaner.h"

extern MainWindow *qmc2MainWindow;
extern Options *qmc2Options;
extern Settings *qmc2Config;

RomPathCleaner::RomPathCleaner(QWidget *parent) :
	QWidget(parent)
{
	setupUi(this);
	comboBoxCheckedPath->insertSeparator(QMC2_RPC_PATH_INDEX_SEPARATOR);
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
	if ( index == QMC2_RPC_PATH_INDEX_SELECT ) {
		QString path(QFileDialog::getExistingDirectory(this, tr("Select path to be checked"), QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | (qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog)));
		if ( !path.isEmpty() ) {
			while ( comboBoxCheckedPath->count() > QMC2_RPC_PATH_INDEX_CUSTOMPATH )
				comboBoxCheckedPath->removeItem(comboBoxCheckedPath->count() - 1);
			comboBoxCheckedPath->insertItem(QMC2_RPC_PATH_INDEX_CUSTOMPATH, path);
			comboBoxCheckedPath->setCurrentIndex(QMC2_RPC_PATH_INDEX_CUSTOMPATH);
		} else
			comboBoxCheckedPath->setCurrentIndex(QMC2_RPC_PATH_INDEX_ROMPATH);
	}
}
