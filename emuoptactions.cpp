#include <QApplication>
#include <QFontMetrics>
#include <QSettings>
#include <QMessageBox>

#include "emuoptactions.h"
#include "emuopt.h"
#include "qmc2main.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;
extern QSettings *qmc2Config;
extern EmulatorOptions *qmc2GlobalEmulatorOptions;
extern EmulatorOptions *qmc2EmulatorOptions;

EmulatorOptionActions::EmulatorOptionActions(QTreeWidgetItem *item, bool global, QString sysName, QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);

	optionName = item->text(0);
	for (int i = 0; i < item->childCount(); i++) {
		QTreeWidgetItem *subItem = item->child(i);
		if ( subItem->text(0) == tr("Default") )
			defaultValue = subItem->text(1);
		else if ( subItem->text(0) == tr("Type") )
			optionType = subItem->text(1);
	}

	if ( defaultValue == tr("<EMPTY>") )
		defaultValue.clear();

	myItem = item;
	isGlobal = global;
	systemName = sysName;

	if ( !isGlobal )
		toolButtonReset->setToolTip(tr("Reset to global value"));

	adjustIconSizes();
}

EmulatorOptionActions::~EmulatorOptionActions()
{
}

void EmulatorOptionActions::on_toolButtonReset_clicked()
{
	if ( qmc2Config->contains(QMC2_EMULATOR_PREFIX + "Configuration/Global/" + optionName) )
		globalValue = qmc2Config->value(QMC2_EMULATOR_PREFIX + "Configuration/Global/" + optionName, QString()).toString();
	else
		globalValue = "<UNSET>";

	if ( isGlobal )
		myItem->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, defaultValue);
	else {
		if ( globalValue == "<UNSET>" )
			myItem->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, defaultValue);
		else
			myItem->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, globalValue);
	}
}

void EmulatorOptionActions::on_toolButtonRevert_clicked()
{
	if ( qmc2Config->contains(QMC2_EMULATOR_PREFIX + "Configuration/Global/" + optionName) )
		globalValue = qmc2Config->value(QMC2_EMULATOR_PREFIX + "Configuration/Global/" + optionName, QString()).toString();
	else
		globalValue = "<UNSET>";

	if ( isGlobal ) {
		if ( qmc2Config->contains(QMC2_EMULATOR_PREFIX + "Configuration/Global/" + optionName) )
			storedValue = qmc2Config->value(QMC2_EMULATOR_PREFIX + "Configuration/Global/" + optionName, QString()).toString();
		else
			storedValue = "<UNSET>";
	} else {
		if ( qmc2Config->contains(QMC2_EMULATOR_PREFIX + "Configuration/" + systemName + "/" + optionName) )
			storedValue = qmc2Config->value(QMC2_EMULATOR_PREFIX + "Configuration/" + systemName + "/" + optionName, QString()).toString();
		else
			storedValue = "<UNSET>";
	}

	if ( isGlobal ) {
		if ( storedValue == "<UNSET>" )
			myItem->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, defaultValue);
		else
			myItem->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, storedValue);
		QTimer::singleShot(0, qmc2GlobalEmulatorOptions, SLOT(updateAllEmuOptActions()));
	} else {
		if ( storedValue == "<UNSET>" ) {
			if ( globalValue == "<UNSET>" )
				myItem->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, defaultValue);
			else
				myItem->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, globalValue);
		} else
			myItem->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, storedValue);
		QTimer::singleShot(0, qmc2EmulatorOptions, SLOT(updateAllEmuOptActions()));
	}
}

void EmulatorOptionActions::on_toolButtonStore_clicked()
{
	if ( qmc2Config->contains(QMC2_EMULATOR_PREFIX + "Configuration/Global/" + optionName) )
		globalValue = qmc2Config->value(QMC2_EMULATOR_PREFIX + "Configuration/Global/" + optionName, QString()).toString();
	else
		globalValue = "<UNSET>";

	currentValue = myItem->data(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole).toString();
	QList<QTreeWidgetItem *> il;

	if ( isGlobal ) {
		if ( qmc2EmulatorOptions ) {
			switch ( QMessageBox::question(this, tr("Confirm"), 
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
						       tr("An open game-specific emulator configuration has been detected.\nUse local value for option '%1', overwrite with global value or don't apply?").arg(optionName),
#elif defined(QMC2_EMUTYPE_MESS)
						       tr("An open machine-specific emulator configuration has been detected.\nUse local value for option '%1', overwrite with global value or don't apply?").arg(optionName),
#endif
						       tr("&Local"), tr("&Overwrite"), tr("Do&n't apply"), 0, 2) ) {
				case 0:
					if ( currentValue == defaultValue )
						qmc2Config->remove(QMC2_EMULATOR_PREFIX + "Configuration/Global/" + optionName);
					else
						qmc2Config->setValue(QMC2_EMULATOR_PREFIX + "Configuration/Global/" + optionName, currentValue);
					break;

				case 1:
					systemName = qmc2EmulatorOptions->settingsGroup.split("/").last();
					if ( currentValue == defaultValue ) {
						qmc2Config->remove(QMC2_EMULATOR_PREFIX + "Configuration/Global/" + optionName);
						globalValue = "<UNSET>";
					} else {
						qmc2Config->setValue(QMC2_EMULATOR_PREFIX + "Configuration/Global/" + optionName, currentValue);
						globalValue = currentValue;
					}
					qmc2Config->remove(QMC2_EMULATOR_PREFIX + "Configuration/" + systemName + "/" + optionName);
					il = qmc2EmulatorOptions->findItems(optionName, Qt::MatchRecursive | Qt::MatchExactly, QMC2_EMUOPT_COLUMN_OPTION);
					if ( !il.isEmpty() ) {
						if ( globalValue == "<UNSET>" )
							il[0]->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, defaultValue);
						else
							il[0]->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, globalValue);
					}
					break;

				case 2:
				default:
					break;
			}
			QTimer::singleShot(0, qmc2GlobalEmulatorOptions, SLOT(updateAllEmuOptActions()));
			QTimer::singleShot(0, qmc2EmulatorOptions, SLOT(updateAllEmuOptActions()));
		} else {
			if ( currentValue == defaultValue )
				qmc2Config->remove(QMC2_EMULATOR_PREFIX + "Configuration/Global/" + optionName);
			else
				qmc2Config->setValue(QMC2_EMULATOR_PREFIX + "Configuration/Global/" + optionName, currentValue);
			QTimer::singleShot(0, qmc2GlobalEmulatorOptions, SLOT(updateAllEmuOptActions()));
		}
	} else {
		if ( globalValue == "<UNSET>" ) {
			if ( currentValue == defaultValue )
				qmc2Config->remove(QMC2_EMULATOR_PREFIX + "Configuration/" + systemName + "/" + optionName);
			else
				qmc2Config->setValue(QMC2_EMULATOR_PREFIX + "Configuration/" + systemName + "/" + optionName, currentValue);
		} else {
			if ( currentValue == globalValue )
				qmc2Config->remove(QMC2_EMULATOR_PREFIX + "Configuration/" + systemName + "/" + optionName);
			else
				qmc2Config->setValue(QMC2_EMULATOR_PREFIX + "Configuration/" + systemName + "/" + optionName, currentValue);
		}
		QTimer::singleShot(0, qmc2EmulatorOptions, SLOT(updateAllEmuOptActions()));
	}
}

void EmulatorOptionActions::adjustIconSizes()
{
	QFontMetrics fm(QApplication::font());
	QSize iconSize(fm.height() - 2, fm.height() - 2);
	toolButtonReset->setIconSize(iconSize);
	toolButtonRevert->setIconSize(iconSize);
	toolButtonStore->setIconSize(iconSize);
}
