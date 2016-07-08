#ifndef _FILTERCONFIGURATIONDIALOG_H_
#define _FILTERCONFIGURATIONDIALOG_H_

#include "ui_filterconfigurationdialog.h"

class FilterConfigurationDialog : public QDialog, public Ui::FilterConfigurationDialog
{
	Q_OBJECT

       	public:
		FilterConfigurationDialog(QWidget *parent = 0);

	public slots:
		void adjustIconSizes();

	protected:
		void showEvent(QShowEvent *e);
		void hideEvent(QHideEvent *e);

	private:
};

#endif
