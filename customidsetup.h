#ifndef _CUSTOMIDSETUP_H_
#define _CUSTOMIDSETUP_H_

#include <QtGui>
#include "ui_customidsetup.h"

class CustomIDSetup : public QDialog, public Ui::CustomIDSetup
{
	Q_OBJECT

       	public:
		QString foreignEmulator;
		QMenu *copyIDsMenu;

		CustomIDSetup(QString, QWidget *parent = 0);
		~CustomIDSetup();

	public slots:
		// auto-connected slots
		void on_toolButtonAddID_clicked();
		void on_toolButtonRemoveID_clicked();
		void on_toolButtonSort_toggled(bool);
		void on_tableWidgetCustomIDs_itemSelectionChanged();
		void on_tableWidgetCustomIDs_currentItemChanged(QTableWidgetItem *, QTableWidgetItem *);

		// menu actions
		void action_copyIDsMenuItem_triggered();

		// other
		void load();
		void save();
		void adjustFontAndIconSizes();
		void setupCopyIDsMenu();
		void chooseIdIconClicked();
		void actionDefaultIdIconTriggered();
		void actionNoIdIconTriggered();
};

#endif
