#ifndef EMUOPTACTIONS_H
#define EMUOPTACTIONS_H

#include <QTreeWidgetItem>
#include "ui_emuoptactions.h"

class EmulatorOptionActions : public QWidget, public Ui::EmulatorOptionActions
{
	Q_OBJECT

       	public:
		QTreeWidgetItem *myItem;
		QString optionType;
		QString optionName;
		QString defaultValue;
		QString globalValue;
		QString storedValue;
		QString currentValue;
		bool isGlobal;
		QString systemName;

		EmulatorOptionActions(QTreeWidgetItem *, bool, QString &, QWidget *parent = 0);
		~EmulatorOptionActions();

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
		void disableEnforceDefaultAction() { toolButtonEnforceDefault->setEnabled(false); }
		void checkEnforceDefaultAction() { toolButtonEnforceDefault->setChecked(true); }
		void uncheckEnforceDefaultAction() { toolButtonEnforceDefault->setChecked(false); }

		void adjustIconSizes();
};

#endif
