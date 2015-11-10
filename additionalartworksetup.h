#ifndef _ADDITIONALARTWORKSETUP_H_
#define _ADDITIONALARTWORKSETUP_H_

#include "ui_additionalartworksetup.h"

#define QMC2_ADDITIONALARTWORK_COLUMN_SELECT			0
#define QMC2_ADDITIONALARTWORK_COLUMN_NAME			1
#define QMC2_ADDITIONALARTWORK_COLUMN_ICON			2
#define QMC2_ADDITIONALARTWORK_COLUMN_TARGET			3
#define QMC2_ADDITIONALARTWORK_COLUMN_TYPE			4
#define QMC2_ADDITIONALARTWORK_COLUMN_FOLDER_OR_ARCHIVE		5

#define QMC2_ADDITIONALARTWORK_INDEX_SYSTEM			0
#define QMC2_ADDITIONALARTWORK_INDEX_SOFTWARE			1

#define QMC2_ADDITIONALARTWORK_INDEX_FOLDER			0
#define QMC2_ADDITIONALARTWORK_INDEX_ARCHIVE			1

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
		void selectionFlagsChanged(bool checked = false);
		void chooseIcon();
		void load();
		void save();

	protected:
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);
};

#endif
