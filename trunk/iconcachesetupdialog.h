#ifndef ICONCACHESETUPDIALOG_H
#define ICONCACHESETUPDIALOG_H

#include "ui_iconcachesetupdialog.h"

class IconCacheSetupDialog : public QDialog, public Ui::IconCacheSetupDialog
{
	Q_OBJECT

       	public:
		IconCacheSetupDialog(QWidget *parent = 0);

	public slots:
		void adjustIconSizes();
		void on_toolButtonImportIcons_clicked();
		void on_toolButtonBrowseIconCacheDatabase_clicked();
		void on_pushButtonOk_clicked();

	protected:
		void showEvent(QShowEvent *);
};

#endif
