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
#include <QScrollBar>

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

// static members
QStringList MachineListViewer::m_savedViews;
QStringList MachineListViewer::m_attachedViews;
bool MachineListViewer::m_savedViewsLoaded = false;

MachineListViewer::MachineListViewer(QWidget *parent) :
	QWidget(parent),
	m_model(0),
	m_ignoreSelectionChange(false),
	m_filterConfigurationDialog(0),
	m_visibleColumnSetup(0),
	m_toolsMenu(0),
	m_saveViewAction(0),
	m_removeViewAction(0),
	m_attachViewAction(0),
	m_detachViewAction(0),
	m_cloneViewAction(0)
{
	setAttribute(Qt::WA_DeleteOnClose);
	setupUi(this);
	comboBoxViewName->lineEdit()->setPlaceholderText(tr("Enter a unique name for this view or select an existing one"));
	comboBoxViewName->lineEdit()->completer()->setCaseSensitivity(Qt::CaseSensitive);
	m_rankUpdateTimer.setSingleShot(true);
	connect(&m_rankUpdateTimer, SIGNAL(timeout()), this, SLOT(treeViewUpdateRanks()));
	m_selectionUpdateTimer.setSingleShot(true);
	connect(&m_selectionUpdateTimer, SIGNAL(timeout()), this, SLOT(currentChangedDelayed()));
	qmc2ActiveViews << treeView;

	// FIXME: this is only valid for "flat" mode (we don't support "tree" mode yet)
	treeView->setRootIsDecorated(false);

	m_toolsMenu = new QMenu(this);
	m_toolsMenu->setTearOffEnabled(true);
	m_saveViewAction = m_toolsMenu->addAction(QIcon(QString::fromUtf8(":/data/img/filesaveas_and_apply.png")), tr("Save"));
	m_saveViewAction->setToolTip(tr("Save view"));
	m_saveViewAction->setStatusTip(tr("Save view"));
	m_saveViewAction->setEnabled(false);
	connect(m_saveViewAction, SIGNAL(triggered(bool)), this, SLOT(saveViewAction_triggered(bool)));
	m_removeViewAction = m_toolsMenu->addAction(QIcon(QString::fromUtf8(":/data/img/broom.png")), tr("Remove"));
	m_removeViewAction->setToolTip(tr("Remove view"));
	m_removeViewAction->setStatusTip(tr("Remove view"));
	m_removeViewAction->setEnabled(false);
	connect(m_removeViewAction, SIGNAL(triggered(bool)), this, SLOT(removeViewAction_triggered(bool)));
	m_toolsMenu->addSeparator();
	m_attachViewAction = m_toolsMenu->addAction(QIcon(QString::fromUtf8(":/data/img/plus.png")), tr("Attach"));
	m_attachViewAction->setToolTip(tr("Attach view"));
	m_attachViewAction->setStatusTip(tr("Attach view"));
	m_attachViewAction->setEnabled(false);
	connect(m_attachViewAction, SIGNAL(triggered(bool)), this, SLOT(attachViewAction_triggered(bool)));
	m_detachViewAction = m_toolsMenu->addAction(QIcon(QString::fromUtf8(":/data/img/minus.png")), tr("Detach"));
	m_detachViewAction->setToolTip(tr("Detach view"));
	m_detachViewAction->setStatusTip(tr("Detach view"));
	m_detachViewAction->setEnabled(false);
	connect(m_detachViewAction, SIGNAL(triggered(bool)), this, SLOT(detachViewAction_triggered(bool)));
	m_toolsMenu->addSeparator();
	m_cloneViewAction = m_toolsMenu->addAction(QIcon(QString::fromUtf8(":/data/img/editcopy.png")), tr("Clone"));
	m_cloneViewAction->setToolTip(tr("Clone view"));
	m_cloneViewAction->setStatusTip(tr("Clone view"));
	connect(m_cloneViewAction, SIGNAL(triggered(bool)), this, SLOT(cloneViewAction_triggered(bool)));
	toolButtonToolsMenu->setMenu(m_toolsMenu);

	if ( !m_savedViewsLoaded ) {
		qmc2Config->beginGroup(QMC2_VIEWS_PREFIX);
		foreach (QString v, qmc2Config->childGroups()) {
			savedViews() << v;
			if ( qmc2Config->value(v + "/Attached", false).toBool() )
				attachedViews() << v;
		}
		qmc2Config->endGroup();
		m_savedViewsLoaded = true;
	}

	connect(comboBoxViewName->lineEdit(), SIGNAL(textChanged(const QString &)), this, SLOT(lineEdit_textChanged(const QString &)));
	connect(headerView(), SIGNAL(sectionMoved(int, int, int)), this, SLOT(treeViewSectionMoved(int, int, int)));

	QTimer::singleShot(0, this, SLOT(init()));
}

MachineListViewer::~MachineListViewer()
{
	MainWindow::machineListViewers.removeAll(this);
	qmc2ActiveViews.removeAll(treeView);
	treeView->setModel(0);
	delete model();
	delete filterConfigurationDialog();
	delete visibleColumnSetup();
}

void MachineListViewer::init()
{
	comboBoxViewName->insertItems(0, savedViews());
	comboBoxViewName->setCurrentIndex(-1);
	m_filterConfigurationDialog = new FilterConfigurationDialog(this, this);
	m_visibleColumnSetup = new VisibleColumnSetup(this);
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
				m_rankUpdateTimer.start(QMC2_RANK_UPDATE_DELAY);
			}
		}
	}
	lcdNumberRecordCount->display((int)model()->recordCount());
	connect(treeView->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(currentChanged(const QModelIndex &, const QModelIndex &)));
	connect(treeView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(treeViewVerticalScrollChanged(int)));
	treeView->setFocus();
}

void MachineListViewer::adjustIconSizes()
{
	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFontMetrics fm(f);
	QSize iconSize(fm.height() - 2, fm.height() - 2);
	QSize iconSizeView(fm.height(), fm.height());
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
	filterConfigurationDialog()->raise();
	filterConfigurationDialog()->exec();
}

void MachineListViewer::on_toolButtonVisibleColumns_clicked()
{
	visibleColumnSetup()->show();
	if ( visibleColumnSetup()->isMinimized() )
		visibleColumnSetup()->showNormal();
	visibleColumnSetup()->raise();
	visibleColumnSetup()->exec();
}

void MachineListViewer::on_toolButtonUpdateView_clicked()
{
	int currentSortColumn = treeView->header()->sortIndicatorSection();
	Qt::SortOrder currentSortOrder = treeView->header()->sortIndicatorOrder();
	model()->resetModel();
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
				m_rankUpdateTimer.start(QMC2_RANK_UPDATE_DELAY);
			}
		}
	}
	lcdNumberRecordCount->display((int)model()->recordCount());
}

void MachineListViewer::currentChanged(const QModelIndex &current, const QModelIndex & /*previous*/)
{
	m_currentId = current.sibling(current.row(), MachineListModel::ID).data().toString();
	m_selectionUpdateTimer.start(qmc2UpdateDelay);
}

void MachineListViewer::currentChangedDelayed()
{
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

void MachineListViewer::settingsSaveView()
{
	if ( name().isEmpty() )
		return;
	qmc2Config->setValue(QMC2_VIEWS_PREFIX + name() + "/Attached", false);
	// FIXME
}

void MachineListViewer::settingsRemoveView()
{
	if ( name().isEmpty() )
		return;
	qmc2Config->remove(QMC2_VIEWS_PREFIX + name());
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
	m_rankUpdateTimer.start(QMC2_RANK_UPDATE_DELAY);
}

void MachineListViewer::treeViewUpdateRanks()
{
	if ( headerView()->isSectionHidden(MachineListModel::RANK) )
		return;
	QFontMetrics fm(treeView->fontMetrics());
	QModelIndex index(treeView->indexAt(treeView->viewport()->rect().topLeft()));
	index = index.sibling(index.row(), MachineListModel::RANK);
	QModelIndex endIndex(treeView->indexAt(treeView->viewport()->rect().bottomLeft()));
	endIndex = endIndex.sibling(endIndex.row(), MachineListModel::RANK);
	treeView->setUpdatesEnabled(false);
	while ( index.isValid() ) {
		treeView->setIndexWidget(index, new RankItemWidget(model()->itemFromIndex(index), treeView));
		if ( index == endIndex )
			break;
		index = treeView->indexBelow(index);
		index = index.sibling(index.row(), MachineListModel::RANK);
	}
	treeView->setUpdatesEnabled(true);
}

void MachineListViewer::treeViewSectionMoved(int /* logicalIndex */, int visualFrom, int visualTo)
{
	if ( visibleColumnSetup() )
		visibleColumnSetup()->listWidget->insertItem(visualTo, visibleColumnSetup()->listWidget->takeItem(visualFrom));
}

void MachineListViewer::saveViewAction_triggered(bool)
{
	if ( !savedViews().contains(name()) ) {
		savedViews().append(name());
		settingsSaveView();
		savedViews().sort();
		foreach (MachineListViewer *v, MainWindow::machineListViewers) {
			v->comboBoxViewName->blockSignals(true);
			v->comboBoxViewName->lineEdit()->blockSignals(true);
			v->comboBoxViewName->clear();
			v->comboBoxViewName->insertItems(0, savedViews());
			v->comboBoxViewName->lineEdit()->blockSignals(false);
			v->comboBoxViewName->blockSignals(false);
		}
	}
	foreach (MachineListViewer *v, MainWindow::machineListViewers) {
		v->comboBoxViewName->blockSignals(true);
		v->comboBoxViewName->lineEdit()->blockSignals(true);
		v->comboBoxViewName->lineEdit()->setText(v->name());
		v->lineEdit_textChanged(v->name());
		int index = v->comboBoxViewName->findText(v->name());
		if ( index >= 0 )
			v->comboBoxViewName->setCurrentIndex(index);
		v->comboBoxViewName->lineEdit()->blockSignals(false);
		v->comboBoxViewName->blockSignals(false);
	}
}

void MachineListViewer::removeViewAction_triggered(bool)
{
	savedViews().removeAll(name());
	settingsRemoveView();
	foreach (MachineListViewer *v, MainWindow::machineListViewers) {
		v->comboBoxViewName->blockSignals(true);
		v->comboBoxViewName->lineEdit()->blockSignals(true);
		v->comboBoxViewName->clear();
		v->comboBoxViewName->insertItems(0, savedViews());
		v->comboBoxViewName->lineEdit()->setText(v->name());
		v->lineEdit_textChanged(v->name());
		int index = v->comboBoxViewName->findText(v->name());
		if ( index >= 0 )
			v->comboBoxViewName->setCurrentIndex(index);
		v->comboBoxViewName->lineEdit()->blockSignals(false);
		v->comboBoxViewName->blockSignals(false);
	}
}

void MachineListViewer::attachViewAction_triggered(bool)
{
	attachedViews().append(name());
	attachedViews().sort();
	foreach (MachineListViewer *v, MainWindow::machineListViewers)
		v->lineEdit_textChanged(v->name());
	qmc2Config->setValue(QMC2_VIEWS_PREFIX + name() + "/Attached", true);
	// FIXME
}

void MachineListViewer::detachViewAction_triggered(bool)
{
	attachedViews().removeAll(name());
	foreach (MachineListViewer *v, MainWindow::machineListViewers)
		v->lineEdit_textChanged(v->name());
	qmc2Config->setValue(QMC2_VIEWS_PREFIX + name() + "/Attached", false);
	// FIXME
}

void MachineListViewer::cloneViewAction_triggered(bool)
{
	QMC2_PRINT_TXT(FIXME: MachineListViewer::cloneViewAction_triggered(bool));
}

void MachineListViewer::lineEdit_textChanged(const QString &text)
{
	m_name = text;
	if ( text.isEmpty() ) {
		m_saveViewAction->setEnabled(false);
		m_removeViewAction->setEnabled(false);
		m_attachViewAction->setEnabled(false);
		m_detachViewAction->setEnabled(false);
		setWindowTitle("MachineListViewer");
		return;
	}
	setWindowTitle("MachineListViewer :: " + text);
	if ( attachedViews().contains(text) ) {
		m_saveViewAction->setEnabled(false);
		m_removeViewAction->setEnabled(false);
		m_attachViewAction->setEnabled(false);
		m_detachViewAction->setEnabled(true);
	} else {
		m_saveViewAction->setEnabled(true);
		m_removeViewAction->setEnabled(savedViews().contains(text));
		m_attachViewAction->setEnabled(savedViews().contains(text));
		m_detachViewAction->setEnabled(false);
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
	widgetMenu->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MachineListViewer/MenuActive", true).toBool());
	restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MachineListViewer/Geometry", QByteArray()).toByteArray());
	treeView->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MachineListViewer/HeaderState", QByteArray()).toByteArray());
	if ( e )
		QWidget::showEvent(e);
}

void MachineListViewer::hideEvent(QHideEvent *e)
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MachineListViewer/MenuActive", widgetMenu->isVisible());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MachineListViewer/Geometry", saveGeometry());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MachineListViewer/HeaderState", treeView->header()->saveState());
	if ( e )
		QWidget::hideEvent(e);
}

void MachineListViewer::resizeEvent(QResizeEvent *e)
{
	m_rankUpdateTimer.start(QMC2_RANK_UPDATE_DELAY);
	if ( e )
		QWidget::resizeEvent(e);
}
