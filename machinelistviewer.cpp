#include <QTimer>

#include "machinelistviewer.h"
#include "macros.h"

MachineListViewer::MachineListViewer(QWidget *parent) :
	QWidget(parent)
{
	setupUi(this);
	QTimer::singleShot(0, this, SLOT(init()));

	// FIXME: this is only valid for "flat" mode (we don't support "tree" mode yet)
	treeView->setRootIsDecorated(false);
}

MachineListViewer::~MachineListViewer()
{
	treeView->setModel(0);
	delete model();
}

void MachineListViewer::init()
{
	m_model = new MachineListModel(treeView, this);
	treeView->setModel(model());
}
