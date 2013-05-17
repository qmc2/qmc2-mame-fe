#ifndef _ADDITIONALARTWORKSETUP_H_
#define _ADDITIONALARTWORKSETUP_H_

#include "ui_additionalartworksetup.h"

class AdditionalArtworkSetup : public QDialog, public Ui::AdditionalArtworkSetup
{
	Q_OBJECT

       	public:
		AdditionalArtworkSetup(QWidget *parent = 0);
		~AdditionalArtworkSetup();

	public slots:
		void adjustIconSizes();
		void on_pushButtonOk_clicked();
		void on_pushButtonCancel_clicked();
		void on_pushButtonRestore_clicked();
		void on_toolButtonAdd_clicked();
		void on_toolButtonRemove_clicked();

	protected:
		void showEvent(QShowEvent *);
		void resizeEvent(QResizeEvent *);
		void hideEvent(QHideEvent *);
};

#endif
