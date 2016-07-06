#ifndef _MACHINELISTVIEWER_H_
#define _MACHINELISTVIEWER_H_

#include "machinelistmodel.h"
#include "ui_machinelistviewer.h"

class MachineListViewer : public QWidget, public Ui::MachineListViewer
{
	Q_OBJECT

       	public:
		explicit MachineListViewer(QWidget *parent = 0);
		~MachineListViewer();

		MachineListModel *model() { return m_model; }

	public slots:
		void init();

	private:
		MachineListModel *m_model;
};

#endif
