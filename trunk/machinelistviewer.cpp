#include <QFont>
#include <QSize>
#include <QPoint>
#include <QMenu>
#include <QList>
#include <QCursor>
#include <QLineEdit>
#include <QFontMetrics>
#include <QApplication>
#include <QTreeWidgetItem>
#include <QAbstractItemView>

#include "qmc2main.h"
#include "machinelistviewer.h"
#include "machinelist.h"
#include "settings.h"
#include "demomode.h"
#include "macros.h"

extern Settings *qmc2Config;
extern QTreeWidgetItem *qmc2CurrentItem;
extern QAbstractItemView::ScrollHint qmc2CursorPositioningMode;
extern int qmc2UpdateDelay;
extern QMenu *qmc2MachineMenu;
extern MainWindow *qmc2MainWindow;
extern DemoModeDialog *qmc2DemoModeDialog;
extern MachineList *qmc2MachineList;
extern int qmc2DefaultLaunchMode;
extern bool qmc2StartEmbedded;
extern bool qmc2IgnoreItemActivation;
extern QList<QWidget *> qmc2ActiveViews;

MachineListViewer::MachineListViewer(QWidget *parent) :
	QWidget(parent),
	m_model(0),
	m_ignoreSelectionChange(false),
	m_filterConfigurationDialog(0)
{
	setupUi(this);
	comboBoxViewName->lineEdit()->setPlaceholderText(tr("Enter a unique name for this view"));
	m_rankUpdateTimer.setSingleShot(true);
	connect(&m_rankUpdateTimer, SIGNAL(timeout()), this, SLOT(treeViewUpdateRanks()));
	qmc2ActiveViews << treeView;

	// FIXME: this is only valid for "flat" mode (we don't support "tree" mode yet)
	treeView->setRootIsDecorated(false);

	QTimer::singleShot(0, this, SLOT(init()));
}

MachineListViewer::~MachineListViewer()
{
	MainWindow::machineListViewers.removeAll(this);
	qmc2ActiveViews.removeAll(treeView);
	treeView->setModel(0);
	delete model();
	delete filterConfigurationDialog();
}

void MachineListViewer::init()
{
	m_filterConfigurationDialog = new FilterConfigurationDialog(this, this);
	m_model = new MachineListModel(treeView, this);
	treeView->setModel(model());
	if ( qmc2CurrentItem ) {
		MachineListModelItem *item = model()->itemHash().value(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_NAME));
		if ( item ) {
			m_currentId = item->id();
			int row = item->row();
			if ( row >= 0 ) {
				QModelIndex idx(model()->index(row, 0, QModelIndex()));
				treeView->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
				treeView->scrollTo(idx, qmc2CursorPositioningMode);
				treeViewUpdateRanks();
			}
		}
	}
	lcdNumberRecordCount->display((int)model()->recordCount());
	connect(treeView->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(currentChanged(const QModelIndex &, const QModelIndex &)));
	connect((QObject *)treeView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(treeViewVerticalScrollChanged(int)));
	treeView->setFocus();
}

void MachineListViewer::adjustIconSizes()
{
	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFontMetrics fm(f);
	QSize iconSize(fm.height() - 2, fm.height() - 2);
	QSize iconSizeView(fm.height(), fm.height());

	toolButtonSaveView->setIconSize(iconSize);
	toolButtonCloneView->setIconSize(iconSize);
	toolButtonViewModeToggle->setIconSize(iconSize);
	toolButtonConfigureFilters->setIconSize(iconSize);
	toolButtonVisibleColumns->setIconSize(iconSize);
	toolButtonUpdateView->setIconSize(iconSize);
	treeView->setIconSize(iconSizeView);
}

void MachineListViewer::on_toolButtonToggleMenu_clicked()
{
	widgetMenu->setVisible(!widgetMenu->isVisible());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MachineListViewer/MenuActive", widgetMenu->isVisible());
}

void MachineListViewer::on_toolButtonConfigureFilters_clicked()
{
	filterConfigurationDialog()->show();
	if ( filterConfigurationDialog()->isMinimized() )
		filterConfigurationDialog()->showNormal();
	filterConfigurationDialog()->exec();
}

void MachineListViewer::on_toolButtonUpdateView_clicked()
{
	int currentSortColumn = treeView->header()->sortIndicatorSection();
	Qt::SortOrder currentSortOrder = treeView->header()->sortIndicatorOrder();
	model()->resetModel();
	qDeleteAll(m_rankItemWidgets);
	m_rankItemWidgets.clear();
	treeView->sortByColumn(currentSortColumn, currentSortOrder);
	if ( qmc2CurrentItem ) {
		MachineListModelItem *item = model()->itemHash().value(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_NAME));
		if ( item ) {
			m_currentId = item->id();
			int row = item->row();
			if ( row >= 0 ) {
				QModelIndex idx(model()->index(row, 0, QModelIndex()));
				treeView->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
				treeView->scrollTo(idx, qmc2CursorPositioningMode);
				treeViewUpdateRanks();
			}
		}
	}
	lcdNumberRecordCount->display((int)model()->recordCount());
}

void MachineListViewer::currentChanged(const QModelIndex &current, const QModelIndex & /*previous*/)
{
	m_currentId = current.sibling(current.row(), MachineListModel::ID).data().toString();
	if ( !m_ignoreSelectionChange )
		emit selectionChanged(m_currentId);
	m_ignoreSelectionChange = false;
}

void MachineListViewer::romStatusChanged(const QString &id, char status)
{
	MachineListModelItem *item = model()->itemHash().value(id);
	if ( item ) {
		item->setRomStatus(status);
		model()->updateData(model()->index(item->row(), MachineListModel::ROM_STATUS, QModelIndex()));
	}
}

void MachineListViewer::mainSelectionChanged(const QString &id)
{
	bool wasEqual = m_currentId == id;
	m_currentId = id;
	MachineListModelItem *item = model()->itemHash().value(m_currentId);
	if ( item ) {
		int row = item->row();
		if ( row >= 0 ) {
			m_ignoreSelectionChange = !wasEqual;
			QModelIndex idx(model()->index(row, 0, QModelIndex()));
			treeView->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
			treeView->scrollTo(idx, qmc2CursorPositioningMode);
		}
	}
}

void MachineListViewer::mainTagChanged(const QString &id, bool tagged)
{
	MachineListModelItem *item = model()->itemHash().value(id);
	if ( item ) {
		item->setTagged(tagged);
		model()->updateData(model()->index(item->row(), MachineListModel::TAG, QModelIndex()));
	}
}

void MachineListViewer::treeViewVerticalScrollChanged(int)
{
	m_rankUpdateTimer.start(qmc2UpdateDelay + QMC2_RANK_UPDATE_DELAY);
}

void MachineListViewer::treeViewUpdateRanks()
{
	QModelIndex index = treeView->indexAt(treeView->viewport()->rect().topLeft());
	QModelIndex endIndex = treeView->indexAt(treeView->viewport()->rect().bottomLeft());
	QFontMetrics fm(treeView->fontMetrics());
	while ( index.isValid() ) {
		QModelIndex idx(index.sibling(index.row(), MachineListModel::RANK));
		RankItemWidget *riw = (RankItemWidget *)treeView->indexWidget(idx);
		if ( riw ) {
			riw->updateSize(&fm);
			if ( riw->rank() > 0 )
				QTimer::singleShot(0, riw, SLOT(updateRankImage()));
		} else {
			RankItemWidget *riw = new RankItemWidget(model()->itemFromIndex(idx));
			m_rankItemWidgets << riw;
			treeView->setIndexWidget(idx, riw);
		}
		if ( index == endIndex )
			break;
		index = treeView->indexBelow(index);
	}
}

void MachineListViewer::on_treeView_customContextMenuRequested(const QPoint &p)
{
	QModelIndex idx(treeView->indexAt(p));
	if ( !idx.isValid() )
		return;
	qmc2MachineMenu->move(qmc2MainWindow->adjustedWidgetPosition(treeView->viewport()->mapToGlobal(p), qmc2MachineMenu));
	qmc2MachineMenu->show();
}

void MachineListViewer::on_treeView_activated(const QModelIndex &)
{
	if ( qmc2DemoModeDialog )
		if ( qmc2DemoModeDialog->demoModeRunning )
			return;
	qmc2StartEmbedded = false;
	if ( !qmc2IgnoreItemActivation ) {
		switch ( qmc2DefaultLaunchMode ) {
#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)
			case QMC2_LAUNCH_MODE_EMBEDDED:
				qmc2MainWindow->on_actionPlayEmbedded_triggered();
				break;
#endif
			case QMC2_LAUNCH_MODE_INDEPENDENT:
			default:
				qmc2MainWindow->on_actionPlay_triggered();
				break;
		}
	}
	qmc2IgnoreItemActivation = false;
}

void MachineListViewer::on_treeView_entered(const QModelIndex &index)
{
	if ( MachineListModel::Column(index.column()) == MachineListModel::TAG ) {
		if ( qApp->mouseButtons() == Qt::LeftButton && qApp->activeWindow() ) {
			if ( treeView->indexAt(treeView->viewport()->mapFromGlobal(QCursor::pos())) == index ) {
				MachineListModelItem *item = model()->itemFromIndex(index);
				if ( item ) {
					bool wasTagged = item->tagged();
					item->setTagged(!wasTagged);
					model()->updateData(model()->index(item->row(), MachineListModel::TAG, QModelIndex()));
					emit tagChanged(item->id(), item->tagged());
					if ( wasTagged )
						qmc2MachineList->numTaggedSets--;
					else
						qmc2MachineList->numTaggedSets++;
					qmc2MainWindow->labelMachineListStatus->setText(qmc2MachineList->status());
				}
			}
		}
	}
}

void MachineListViewer::on_treeView_clicked(const QModelIndex &index)
{
	if ( MachineListModel::Column(index.column()) == MachineListModel::TAG ) {
		MachineListModelItem *item = model()->itemFromIndex(index);
		if ( item ) {
			bool wasTagged = item->tagged();
			item->setTagged(!wasTagged);
			model()->updateData(model()->index(item->row(), MachineListModel::TAG, QModelIndex()));
			emit tagChanged(item->id(), item->tagged());
			if ( wasTagged )
				qmc2MachineList->numTaggedSets--;
			else
				qmc2MachineList->numTaggedSets++;
			qmc2MainWindow->labelMachineListStatus->setText(qmc2MachineList->status());
		}
	}
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
	if ( e )
		QWidget::hideEvent(e);
}

void MachineListViewer::resizeEvent(QResizeEvent *e)
{
	m_rankUpdateTimer.start(qmc2UpdateDelay + QMC2_RANK_UPDATE_DELAY);
	if ( e )
		QWidget::resizeEvent(e);
}
