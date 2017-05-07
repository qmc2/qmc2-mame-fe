#ifndef _ROMPATHCLEANER_H_
#define _ROMPATHCLEANER_H_

#include "ui_rompathcleaner.h"

#define QMC2_RPC_INDEX_ROM_PATH		0
#define QMC2_RPC_INDEX_SELECT_PATH	1

class RomPathCleaner : public QWidget, public Ui::RomPathCleaner
{
	Q_OBJECT

       	public:
		RomPathCleaner(QWidget *parent = 0);

	public slots:
		void adjustIconSizes();
		void on_pushButtonStartStop_clicked();
		void on_comboBoxCheckedPath_activated(int);

	signals:

	private:
};

#endif
