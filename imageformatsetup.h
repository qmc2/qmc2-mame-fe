#ifndef _IMAGEFORMATSETUP_H_
#define _IMAGEFORMATSETUP_H_

#include "ui_imageformatsetup.h"

class ImageFormatSetup : public QDialog, public Ui::ImageFormatSetup
{
	Q_OBJECT

       	public:
		ImageFormatSetup(QWidget *parent = 0);
		~ImageFormatSetup();

	public slots:
		void adjustIconSizes();
		void on_pushButtonOk_clicked();
		void on_pushButtonCancel_clicked();
		void on_pushButtonRestore_clicked();
		void on_comboBoxImageType_currentIndexChanged(int);

	protected:
		void showEvent(QShowEvent *);
		void resizeEvent(QResizeEvent *);
		void hideEvent(QHideEvent *);
};

#endif
