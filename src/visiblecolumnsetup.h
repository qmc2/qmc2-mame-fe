#ifndef VISIBLECOLUMNSETUP_H
#define VISIBLECOLUMNSETUP_H

#include "ui_visiblecolumnsetup.h"

class MachineListViewer;

class VisibleColumnSetup : public QDialog, public Ui::VisibleColumnSetup
{
	Q_OBJECT

       	public:
		VisibleColumnSetup(MachineListViewer *viewer, QWidget *parent = 0);

		MachineListViewer *viewer() { return m_viewer; }

	public slots:
		void init();
		void on_pushButtonOk_clicked();
		void on_pushButtonApply_clicked();

	protected:
		void showEvent(QShowEvent *e);
		void hideEvent(QHideEvent *e);

	private:
		MachineListViewer *m_viewer;
};

#endif
