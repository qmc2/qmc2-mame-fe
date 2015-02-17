#ifndef _MISSINGDUMPSVIEWER_H_
#define _MISSINGDUMPSVIEWER_H_

#include <QTreeWidget>
#include "ui_missingdumpsviewer.h"

#define QMC2_MDV_COLUMN_ID		0
#define QMC2_MDV_COLUMN_TYPE		1
#define QMC2_MDV_COLUMN_NAME		2
#define QMC2_MDV_COLUMN_SIZE		3
#define QMC2_MDV_COLUMN_CRC		4
#define QMC2_MDV_COLUMN_SHA1		5
#define QMC2_MDV_COLUMN_REASON		6

class MissingDumpsViewer : public QDialog, public Ui::MissingDumpsViewer
{
	Q_OBJECT

       	public:
		MissingDumpsViewer(QWidget *parent = 0);

	public slots:

	protected:
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);
		void closeEvent(QCloseEvent *);
};

#endif
