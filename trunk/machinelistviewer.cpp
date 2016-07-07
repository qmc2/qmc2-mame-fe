#include <QFont>
#include <QSize>
#include <QLineEdit>
#include <QTreeWidgetItem>
#include <QAbstractItemView>
#include <QFontMetrics>
#include <QFont>

#include "qmc2main.h"
#include "machinelistviewer.h"
#include "rankitemwidget.h"
#include "settings.h"
#include "macros.h"

extern Settings *qmc2Config;
extern QTreeWidgetItem *qmc2CurrentItem;
extern QAbstractItemView::ScrollHint qmc2CursorPositioningMode;
extern int qmc2UpdateDelay;

MachineListViewer::MachineListViewer(QWidget *parent) :
	QWidget(parent),
	m_model(0),
	m_ignoreSelectionChange(false)
{
	setupUi(this);
	comboBoxViewName->lineEdit()->setPlaceholderText(tr("Enter a unique name for this view"));
	m_rankUpdateTimer.setSingleShot(true);
	connect(&m_rankUpdateTimer, SIGNAL(timeout()), this, SLOT(treeViewUpdateRanks()));

	// FIXME: this is only valid for "flat" mode (we don't support "tree" mode yet)
	treeView->setRootIsDecorated(false);

	QTimer::singleShot(0, this, SLOT(init()));
}

MachineListViewer::~MachineListViewer()
{
	MainWindow::machineListViewers.removeAll(this);
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
			m_currentId = item->id();
			int row = model()->rootItem()->childItems().indexOf(item);
			if ( row >= 0 ) {
				QModelIndex idx(model()->index(row, 0, QModelIndex()));
				treeView->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
				treeView->scrollTo(idx, qmc2CursorPositioningMode);
				treeViewUpdateRanks();
			}
		}
	}
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

void MachineListViewer::currentChanged(const QModelIndex &current, const QModelIndex & /*previous*/)
{
	m_currentId = current.sibling(current.row(), MachineListModel::ID).data().toString();
	if ( !m_ignoreSelectionChange )
		emit selectionChanged(m_currentId);
	m_ignoreSelectionChange = false;
}

void MachineListViewer::mainSelectionChanged(const QString &id)
{
	m_currentId = id;
	MachineListModelItem *item = model()->itemHash().value(m_currentId);
	if ( item ) {
		int row = model()->rootItem()->childItems().indexOf(item);
		if ( row >= 0 ) {
			m_ignoreSelectionChange = true;
			QModelIndex idx(model()->index(row, 0, QModelIndex()));
			treeView->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
			treeView->scrollTo(idx, qmc2CursorPositioningMode);
		}
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
		} else
			treeView->setIndexWidget(idx, new RankItemWidget(model()->itemFromIndex(idx)));
		if ( index == endIndex )
			break;
		index = treeView->indexBelow(index);
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
