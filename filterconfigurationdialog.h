#ifndef _FILTERCONFIGURATIONDIALOG_H_
#define _FILTERCONFIGURATIONDIALOG_H_

#include "ui_filterconfigurationdialog.h"

class MachineListViewer;

#define QMC2_FCDLG_COLUMN_ACTION	0
#define QMC2_FCDLG_COLUMN_NAME		1

class FilterConfigurationDialog : public QDialog, public Ui::FilterConfigurationDialog
{
	Q_OBJECT

       	public:
		FilterConfigurationDialog(MachineListViewer *viewer, QWidget *parent = 0);

		MachineListViewer *viewer() { return m_viewer; }

	public slots:
		void init();
		void adjustIconSizes();
		void on_pushButtonOk_clicked();
		void on_pushButtonApply_clicked();
		void on_pushButtonCancel_clicked();

	protected:
		void showEvent(QShowEvent *e);
		void hideEvent(QHideEvent *e);

	private:
		MachineListViewer *m_viewer;
};

#endif
