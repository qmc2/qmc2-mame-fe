#ifndef _ROMPATHCLEANER_H_
#define _ROMPATHCLEANER_H_

#include "ui_rompathcleaner.h"

#define QMC2_RPC_PATH_INDEX_ROMPATH		0
#define QMC2_RPC_PATH_INDEX_SEPARATOR		1
#define QMC2_RPC_PATH_INDEX_SELECT		2

#define QMC2_RPC_MODE_INDEX_DELETE		0
#define QMC2_RPC_MODE_INDEX_MOVE		1
#define QMC2_RPC_MODE_INDEX_DRYRUN		2

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
