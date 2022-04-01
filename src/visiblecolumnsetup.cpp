#include <QTimer>
#include <QList>
#include <QListWidgetItem>

#include "visiblecolumnsetup.h"
#include "machinelistviewer.h"
#include "settings.h"
#include "macros.h"

extern Settings *qmc2Config;

VisibleColumnSetup::VisibleColumnSetup(MachineListViewer *viewer, QWidget *parent) :
	QDialog(parent),
	m_viewer(viewer)
{
	setupUi(this);
}

void VisibleColumnSetup::init()
{
	listWidget->setUpdatesEnabled(false);
	listWidget->clear();
	for (int c = 0; c < viewer()->headerView()->count(); c++) {
		QListWidgetItem *item = new QListWidgetItem;
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsUserCheckable);
		item->setText(viewer()->headers().at(viewer()->headerView()->logicalIndex(c)));
		item->setCheckState(viewer()->headerView()->isSectionHidden(viewer()->headerView()->logicalIndex(c)) ? Qt::Unchecked : Qt::Checked);
		listWidget->addItem(item);
	}
	listWidget->setUpdatesEnabled(true);
}

void VisibleColumnSetup::on_pushButtonOk_clicked()
{
	on_pushButtonApply_clicked();
}

void VisibleColumnSetup::on_pushButtonApply_clicked()
{
	viewer()->headerView()->blockSignals(true);
	for (int to = 0; to < listWidget->count(); to++) {
		int from = viewer()->headerView()->visualIndex(viewer()->headers().indexOf(listWidget->item(to)->text()));
		if ( from != to )
			viewer()->headerView()->moveSection(from, to);
		viewer()->treeView->setColumnHidden(viewer()->headerView()->logicalIndex(to), listWidget->item(to)->checkState() == Qt::Unchecked);
	}
	viewer()->headerView()->blockSignals(false);
	viewer()->treeView->viewport()->setUpdatesEnabled(false);
	QSize s(viewer()->treeView->viewport()->size());
	viewer()->treeView->viewport()->resize(0, 0);
	viewer()->treeView->viewport()->resize(s);
	viewer()->treeView->viewport()->setUpdatesEnabled(true);
	viewer()->treeView->viewport()->update();
	QTimer::singleShot(QMC2_RANK_UPDATE_DELAY, viewer(), SLOT(treeViewUpdateRanks()));
}

void VisibleColumnSetup::showEvent(QShowEvent *e)
{
	restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/VisibleColumnSetup/Geometry", QByteArray()).toByteArray());
	QTimer::singleShot(0, this, SLOT(init()));
	if ( e )
		QDialog::showEvent(e);
}

void VisibleColumnSetup::hideEvent(QHideEvent *e)
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/VisibleColumnSetup/Geometry", saveGeometry());
	if ( e )
		QDialog::hideEvent(e);
}
