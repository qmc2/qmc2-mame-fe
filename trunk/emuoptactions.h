#ifndef _EMUOPTACTIONS_H_
#define _EMUOPTACTIONS_H_

#include "ui_emuoptactions.h"

class EmulatorOptionActions : public QWidget, public Ui::EmulatorOptionActions
{
	Q_OBJECT

       	public:
		EmulatorOptionActions(QWidget *parent = 0);
		~EmulatorOptionActions();

	public slots:
		void on_toolButtonReset_clicked();
		void on_toolButtonRevert_clicked();
		void on_toolButtonStore_clicked();

		void adjustIconSizes();
};

#endif
