#include <QTimer>

#include "machinelistviewer.h"
#include "macros.h"

MachineListViewer::MachineListViewer(QWidget *parent) :
	QWidget(parent)
{
	setupUi(this);
	QTimer::singleShot(0, this, SLOT(init()));
}

MachineListViewer::~MachineListViewer()
{
	treeView->setModel(0);
	delete model();
}

void MachineListViewer::init()
{
	m_model = new MachineListModel;
	treeView->setModel(model());
}
