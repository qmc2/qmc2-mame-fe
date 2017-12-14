#ifndef EMUOPTACTIONS_H
#define EMUOPTACTIONS_H

#include <QTreeWidgetItem>
#include "ui_emuoptactions.h"

class EmulatorOptionActions : public QWidget, public Ui::EmulatorOptionActions
{
	Q_OBJECT

       	public:
		EmulatorOptionActions(QTreeWidgetItem *, bool, const QString &, QWidget *parent = 0);

	public slots:
		void on_toolButtonReset_clicked();
		void on_toolButtonRevert_clicked();
		void on_toolButtonStore_clicked();
		void on_toolButtonEnforceDefault_toggled(bool);

		void enableResetAction() { toolButtonReset->setEnabled(true); }
		void disableResetAction() { toolButtonReset->setEnabled(false); }
		void enableRevertAction() { toolButtonRevert->setEnabled(true); }
		void disableRevertAction() { toolButtonRevert->setEnabled(false); }
		void enableStoreAction() { toolButtonStore->setEnabled(true); }
		void disableStoreAction() { toolButtonStore->setEnabled(false); }
		void enableEnforceDefaultAction() { toolButtonEnforceDefault->setEnabled(true); }
		void disableEnforceDefaultAction() { uncheckEnforceDefaultAction(); toolButtonEnforceDefault->setEnabled(false); }
		void checkEnforceDefaultAction() { toolButtonEnforceDefault->setChecked(true); }
		void uncheckEnforceDefaultAction() { toolButtonEnforceDefault->setChecked(false); }
		bool enforceDefaultIsChecked() { return toolButtonEnforceDefault->isEnabled() && toolButtonEnforceDefault->isChecked(); }

		void adjustIconSizes();

	private:
		QTreeWidgetItem *m_myItem;
		QString m_optionType;
		QString m_optionName;
		QString m_defaultValue;
		QString m_globalValue;
		QString m_storedValue;
		QString m_currentValue;
		QString m_systemName;
		bool m_isGlobal;
};

#endif
