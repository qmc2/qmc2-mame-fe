#include <QApplication>
#include <QFileDialog>

#include "iconcachesetupdialog.h"
#include "settings.h"
#include "options.h"
#include "qmc2main.h"
#include "machinelist.h"
#include "macros.h"

extern Settings *qmc2Config;
extern MachineList *qmc2MachineList;
extern MainWindow *qmc2MainWindow;
extern Options *qmc2Options;

IconCacheSetupDialog::IconCacheSetupDialog(QWidget *parent) :
	QDialog(parent)
{
	setupUi(this);
	checkBoxEnableIconCacheDatabase->setChecked(qmc2Config->value(QMC2_EMULATOR_PREFIX + "IconCacheDatabase/Enabled", true).toBool());
	lineEditIconCacheDatabase->setText(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/IconCacheDatabase", QString(Options::configPath() + "/%1-icon-cache.db").arg(QMC2_EMU_NAME.toLower())).toString());
}

void IconCacheSetupDialog::adjustIconSizes()
{
	QFont f(qApp->font());
	QFontMetrics fm(f);
	QSize iconSize(fm.height() - 2, fm.height() - 2);
	toolButtonImportIcons->setIconSize(iconSize);
	toolButtonBrowseIconCacheDatabase->setIconSize(iconSize);
}

void IconCacheSetupDialog::on_toolButtonImportIcons_clicked()
{
	toolButtonImportIcons->setEnabled(false);
	qApp->processEvents();
	on_pushButtonOk_clicked();
	qmc2Config->remove(QMC2_EMULATOR_PREFIX + "IconCacheDatabase/ImportDates");
	qmc2MainWindow->on_actionClearIconCache_triggered();
	qmc2MachineList->loadIcon(QString(), 0);
	toolButtonImportIcons->setEnabled(true);
}

void IconCacheSetupDialog::on_toolButtonBrowseIconCacheDatabase_clicked()
{
	QString s(QFileDialog::getSaveFileName(this, tr("Choose icon cache database file"), lineEditIconCacheDatabase->text(), tr("All files (*)"), 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog));
	if ( !s.isNull() )
		lineEditIconCacheDatabase->setText(s);
}

void IconCacheSetupDialog::on_pushButtonOk_clicked()
{
	qmc2Config->setValue(QMC2_EMULATOR_PREFIX + "IconCacheDatabase/Enabled", checkBoxEnableIconCacheDatabase->isChecked());
	QString oldDbFile(qmc2Config->value(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/IconCacheDatabase", QString(Options::configPath() + "/%1-icon-cache.db").arg(QMC2_EMU_NAME.toLower())).toString());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "FilesAndDirectories/IconCacheDatabase", lineEditIconCacheDatabase->text());
	if ( oldDbFile != lineEditIconCacheDatabase->text() )
		qmc2MachineList->reopenIconCacheDb();
}

void IconCacheSetupDialog::showEvent(QShowEvent *e)
{
	adjustIconSizes();
	adjustSize();
	QDialog::showEvent(e);
}
