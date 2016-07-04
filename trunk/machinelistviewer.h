#ifndef _MACHINELISTVIEWER_H_
#define _MACHINELISTVIEWER_H_

#include "ui_machinelistviewer.h"

class MachineListViewer : public QWidget, public Ui::MachineListViewer
{
	Q_OBJECT

       	public:
		explicit MachineListViewer(QWidget *parent = 0);
		~MachineListViewer();

	public slots:

	private:
};

#endif
