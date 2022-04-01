#include <QApplication>
#include <QFontMetrics>
#include <QMessageBox>

#include "settings.h"
#include "emuoptactions.h"
#include "emuopt.h"
#include "qmc2main.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;
extern EmulatorOptions *qmc2GlobalEmulatorOptions;
extern EmulatorOptions *qmc2EmulatorOptions;

EmulatorOptionActions::EmulatorOptionActions(QTreeWidgetItem *item, bool global, const QString &sysName, QWidget *parent) :
	QWidget(parent),
	m_myItem(item),
	m_systemName(sysName),
	m_isGlobal(global)
{
	setupUi(this);

	m_optionName = item->text(0);
	for (int i = 0; i < item->childCount(); i++) {
		QTreeWidgetItem *subItem = item->child(i);
		if ( subItem->text(0) == tr("Default") )
			m_defaultValue = subItem->text(1);
		else if ( subItem->text(0) == tr("Type") )
			m_optionType = subItem->text(1);
	}

	if ( m_defaultValue == tr("<EMPTY>") )
		m_defaultValue.clear();

	if ( !m_isGlobal )
		toolButtonReset->setToolTip(tr("Reset to global value"));

	adjustIconSizes();
}

void EmulatorOptionActions::on_toolButtonReset_clicked()
{
	if ( qmc2Config->contains(QMC2_EMULATOR_PREFIX + "Configuration/Global/" + m_optionName) )
		m_globalValue = qmc2Config->value(QMC2_EMULATOR_PREFIX + "Configuration/Global/" + m_optionName, QString()).toString();
	else
		m_globalValue = "<UNSET>";

	if ( m_isGlobal )
		m_myItem->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, m_defaultValue);
	else {
		if ( m_globalValue == "<UNSET>" )
			m_myItem->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, m_defaultValue);
		else
			m_myItem->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, m_globalValue);
	}
}

void EmulatorOptionActions::on_toolButtonRevert_clicked()
{
	if ( qmc2Config->contains(QMC2_EMULATOR_PREFIX + "Configuration/Global/" + m_optionName) )
		m_globalValue = qmc2Config->value(QMC2_EMULATOR_PREFIX + "Configuration/Global/" + m_optionName, QString()).toString();
	else
		m_globalValue = "<UNSET>";

	if ( m_isGlobal ) {
		if ( qmc2Config->contains(QMC2_EMULATOR_PREFIX + "Configuration/Global/" + m_optionName) )
			m_storedValue = qmc2Config->value(QMC2_EMULATOR_PREFIX + "Configuration/Global/" + m_optionName, QString()).toString();
		else
			m_storedValue = "<UNSET>";
	} else {
		if ( qmc2Config->contains(QMC2_EMULATOR_PREFIX + "Configuration/" + m_systemName + "/" + m_optionName) )
			m_storedValue = qmc2Config->value(QMC2_EMULATOR_PREFIX + "Configuration/" + m_systemName + "/" + m_optionName, QString()).toString();
		else
			m_storedValue = "<UNSET>";
	}

	if ( m_isGlobal ) {
		if ( m_storedValue == "<UNSET>" )
			m_myItem->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, m_defaultValue);
		else
			m_myItem->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, m_storedValue);
		QTimer::singleShot(0, qmc2GlobalEmulatorOptions, SLOT(updateAllEmuOptActions()));
	} else {
		if ( m_storedValue == "<UNSET>" ) {
			if ( m_globalValue == "<UNSET>" )
				m_myItem->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, m_defaultValue);
			else
				m_myItem->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, m_globalValue);
		} else
			m_myItem->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, m_storedValue);
		QTimer::singleShot(0, qmc2EmulatorOptions, SLOT(updateAllEmuOptActions()));
	}
}

void EmulatorOptionActions::on_toolButtonStore_clicked()
{
	if ( qmc2Config->contains(QMC2_EMULATOR_PREFIX + "Configuration/Global/" + m_optionName) )
		m_globalValue = qmc2Config->value(QMC2_EMULATOR_PREFIX + "Configuration/Global/" + m_optionName, QString()).toString();
	else
		m_globalValue = "<UNSET>";

	m_currentValue = m_myItem->data(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole).toString();

	if ( m_isGlobal ) {
		if ( qmc2EmulatorOptions ) {
			QList<QTreeWidgetItem *> il;
			switch ( QMessageBox::question(this, tr("Confirm"), 
						       tr("An open machine-specific emulator configuration has been detected.\nUse local value for option '%1', overwrite with global value or don't apply?").arg(m_optionName),
						       tr("&Local"), tr("&Overwrite"), tr("Do&n't apply"), 0, 2) ) {
				case 0:
					qmc2GlobalEmulatorOptions->save(m_optionName);
					qmc2GlobalEmulatorOptions->load(false, m_optionName);
					break;

				case 1:
					m_systemName = qmc2EmulatorOptions->settingsGroup.split("/").last();
					if ( m_currentValue == m_defaultValue )
						m_globalValue = "<UNSET>";
					else
						m_globalValue = m_currentValue;
					qmc2GlobalEmulatorOptions->save(m_optionName);
					qmc2GlobalEmulatorOptions->load(false, m_optionName);
					qmc2Config->remove(QMC2_EMULATOR_PREFIX + "Configuration/" + m_systemName + "/" + m_optionName);
					il = qmc2EmulatorOptions->findItems(m_optionName, Qt::MatchRecursive | Qt::MatchExactly, QMC2_EMUOPT_COLUMN_OPTION);
					if ( !il.isEmpty() ) {
						if ( m_globalValue == "<UNSET>" )
							il[0]->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, m_defaultValue);
						else
							il[0]->setData(QMC2_EMUOPT_COLUMN_VALUE, Qt::EditRole, m_globalValue);
					}
					break;

				case 2:
				default:
					break;
			}
			QTimer::singleShot(0, qmc2GlobalEmulatorOptions, SLOT(updateAllEmuOptActions()));
			QTimer::singleShot(0, qmc2EmulatorOptions, SLOT(updateAllEmuOptActions()));
		} else {
			qmc2GlobalEmulatorOptions->save(m_optionName);
			qmc2GlobalEmulatorOptions->load(false, m_optionName);
			QTimer::singleShot(0, qmc2GlobalEmulatorOptions, SLOT(updateAllEmuOptActions()));
		}
	} else {
		if ( m_globalValue == "<UNSET>" ) {
			if ( m_currentValue == m_defaultValue )
				qmc2Config->remove(QMC2_EMULATOR_PREFIX + "Configuration/" + m_systemName + "/" + m_optionName);
			else
				qmc2Config->setValue(QMC2_EMULATOR_PREFIX + "Configuration/" + m_systemName + "/" + m_optionName, m_currentValue);
		} else {
			if ( m_currentValue == m_globalValue )
				qmc2Config->remove(QMC2_EMULATOR_PREFIX + "Configuration/" + m_systemName + "/" + m_optionName);
			else
				qmc2Config->setValue(QMC2_EMULATOR_PREFIX + "Configuration/" + m_systemName + "/" + m_optionName, m_currentValue);
		}
		QTimer::singleShot(0, qmc2EmulatorOptions, SLOT(updateAllEmuOptActions()));
	}
}

void EmulatorOptionActions::on_toolButtonEnforceDefault_toggled(bool checked)
{
	if ( m_isGlobal ) {
		if ( checked )
			qmc2Config->setValue(QMC2_EMULATOR_PREFIX + "Configuration/Global/EnforceDefault/" + m_optionName, true);
		else
			qmc2Config->remove(QMC2_EMULATOR_PREFIX + "Configuration/Global/EnforceDefault/" + m_optionName);
	} else {
		QString localGroup(QMC2_EMULATOR_PREFIX + "Configuration/" + m_systemName);
		bool leaveGroup = qmc2Config->group() == localGroup;
		if ( leaveGroup )
			qmc2Config->endGroup();
		if ( checked )
			qmc2Config->setValue(localGroup + "/EnforceDefault/" + m_optionName, true);
		else
			qmc2Config->remove(localGroup + "/EnforceDefault/" + m_optionName);
		if ( leaveGroup )
			qmc2Config->beginGroup(localGroup);
	}
}

void EmulatorOptionActions::adjustIconSizes()
{
	QFontMetrics fm(QApplication::font());
	QSize iconSize(fm.height() - 2, fm.height() - 2);
	toolButtonReset->setIconSize(iconSize);
	toolButtonRevert->setIconSize(iconSize);
	toolButtonStore->setIconSize(iconSize);
	toolButtonEnforceDefault->setIconSize(iconSize);
}
