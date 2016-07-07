#include <QTimer>
#include <QFont>
#include <QSize>

#include "machinelistviewer.h"
#include "settings.h"
#include "macros.h"

extern Settings *qmc2Config;

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

void MachineListViewer::adjustIconSizes()
{
	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFontMetrics fm(f);
	QSize iconSize(fm.height() - 2, fm.height() - 2);

	toolButtonSaveView->setIconSize(iconSize);
	toolButtonCloneView->setIconSize(iconSize);
	toolButtonViewModeToggle->setIconSize(iconSize);
	toolButtonConfigureView->setIconSize(iconSize);
	toolButtonRefreshView->setIconSize(iconSize);
	treeView->setIconSize(iconSize);
}

void MachineListViewer::showEvent(QShowEvent *e)
{
	adjustIconSizes();
	restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MachineListViewer/Geometry", QByteArray()).toByteArray());
	treeView->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MachineListViewer/HeaderState").toByteArray());
	if ( e )
		QWidget::showEvent(e);
}

void MachineListViewer::hideEvent(QHideEvent *e)
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MachineListViewer/Geometry", saveGeometry());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MachineListViewer/HeaderState", treeView->header()->saveState());
}
