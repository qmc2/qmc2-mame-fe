#ifndef _MAWSQDLSETUP_H_
#define _MAWSQDLSETUP_H_

#include "ui_mawsqdlsetup.h"

class MawsQuickDownloadSetup : public QDialog, public Ui::MawsQuickDownloadSetup
{
	Q_OBJECT

	public:
		MawsQuickDownloadSetup(QWidget *parent = 0);
		~MawsQuickDownloadSetup();

	public slots:
		void on_pushButtonOk_clicked();
		void on_pushButtonCancel_clicked();
		void on_toolButtonBrowseIconDirectory_clicked();
		void on_toolButtonBrowseFlyerDirectory_clicked();
		void on_toolButtonBrowseCabinetDirectory_clicked();
		void on_toolButtonBrowseControllerDirectory_clicked();
		void on_toolButtonBrowseMarqueeDirectory_clicked();
		void on_toolButtonBrowsePCBDirectory_clicked();
		void on_toolButtonBrowsePreviewDirectory_clicked();
		void on_toolButtonBrowseTitleDirectory_clicked();
		void adjustIconSizes();
};

#endif
