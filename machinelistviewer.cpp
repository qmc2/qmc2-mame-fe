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
	delete proxyModel();
	delete model();
}

void MachineListViewer::init()
{
	m_model = new MachineListModel;
	m_proxyModel = new MachineListProxyModel;
	proxyModel()->setSourceModel(model());
	treeView->setModel(proxyModel());
	//treeView->setModel(model());
}
