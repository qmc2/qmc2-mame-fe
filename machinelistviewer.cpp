#include <QTimer>
#include <QFont>
#include <QSize>
#include <QLineEdit>
#include <QTreeWidgetItem>
#include <QAbstractItemView>

#include "machinelistviewer.h"
#include "settings.h"
#include "macros.h"

extern Settings *qmc2Config;
extern QTreeWidgetItem *qmc2CurrentItem;
extern QAbstractItemView::ScrollHint qmc2CursorPositioningMode;

MachineListViewer::MachineListViewer(QWidget *parent) :
	QWidget(parent)
{
	setupUi(this);
	comboBoxViewName->lineEdit()->setPlaceholderText(tr("Enter a unique name for this view"));

	// FIXME: this is only valid for "flat" mode (we don't support "tree" mode yet)
	treeView->setRootIsDecorated(false);

	QTimer::singleShot(0, this, SLOT(init()));
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
	if ( qmc2CurrentItem ) {
		MachineListModelItem *item = model()->itemHash().value(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_NAME));
		if ( item ) {
			int row = model()->rootItem()->childItems().indexOf(item);
			if ( row >= 0 ) {
				QModelIndex idx(model()->index(row, 0, QModelIndex()));
				treeView->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
				treeView->scrollTo(idx, qmc2CursorPositioningMode);
			}
		}
	}
	treeView->setFocus();
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

void MachineListViewer::on_toolButtonToggleMenu_clicked()
{
	widgetMenu->setVisible(!widgetMenu->isVisible());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MachineListViewer/MenuActive", widgetMenu->isVisible());
}

void MachineListViewer::showEvent(QShowEvent *e)
{
	adjustIconSizes();
	restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MachineListViewer/Geometry", QByteArray()).toByteArray());
	treeView->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MachineListViewer/HeaderState").toByteArray());
	widgetMenu->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MachineListViewer/MenuActive", true).toBool());
	if ( e )
		QWidget::showEvent(e);
}

void MachineListViewer::hideEvent(QHideEvent *e)
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MachineListViewer/Geometry", saveGeometry());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MachineListViewer/HeaderState", treeView->header()->saveState());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MachineListViewer/MenuActive", widgetMenu->isVisible());
}
