#ifndef _IMAGECHECKER_H_
#define _IMAGECHECKER_H_

#include "ui_imagechecker.h"

#include "preview.h"
#include "flyer.h"
#include "cabinet.h"
#include "controller.h"
#include "marquee.h"
#include "title.h"
#include "pcb.h"
#include "gamelist.h"

class ImageChecker : public QDialog, public Ui::ImageChecker
{
	Q_OBJECT

	public:
		ImageChecker(QWidget *parent = 0);
		~ImageChecker();

	public slots:
		void on_listWidgetFound_itemSelectionChanged();
		void on_listWidgetMissing_itemSelectionChanged();
		void selectItem(QString);
		void adjustIconSizes();

	protected:
		void closeEvent(QCloseEvent *);
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);
};

#endif
