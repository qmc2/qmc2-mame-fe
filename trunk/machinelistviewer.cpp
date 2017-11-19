#include <QFont>
#include <QSize>
#include <QPoint>
#include <QMenu>
#include <QList>
#include <QCursor>
#include <QCursor>
#include <QAction>
#include <QLineEdit>
#include <QScrollBar>
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

// static members
QStringList MachineListViewer::m_savedViews;
QStringList MachineListViewer::m_attachedViews;
bool MachineListViewer::m_savedViewsLoaded = false;
int MachineListViewer::m_viewSelectSeparatorIndex = -1;

MachineListViewer::MachineListViewer(QWidget *parent) :
	QWidget(parent),
	m_model(0),
	m_ignoreSelectionChange(false),
	m_initCalled(false),
	m_filterConfigurationDialog(0),
	m_visibleColumnSetup(0),
	m_toolsMenu(0),
	m_saveViewAction(0),
	m_removeViewAction(0),
	m_attachViewAction(0),
	m_detachViewAction(0),
	m_cloneViewAction(0),
	m_rankItemDelegate(0)
{
	setAttribute(Qt::WA_DeleteOnClose);
	setupUi(this);
	comboBoxViewName->lineEdit()->setPlaceholderText(tr("Enter a unique name for this view or select an existing one"));
	comboBoxViewName->lineEdit()->completer()->setCaseSensitivity(Qt::CaseSensitive);
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

	if ( !savedViewsLoaded() )
		loadSavedViews();

	connect(comboBoxViewName->lineEdit(), SIGNAL(textChanged(const QString &)), this, SLOT(lineEdit_textChanged(const QString &)));
	connect(headerView(), SIGNAL(sectionMoved(int, int, int)), this, SLOT(treeViewSectionMoved(int, int, int)));

	m_rankItemDelegate = new RankItemDelegate(treeView);
	treeView->setItemDelegateForColumn(MachineListModel::RANK, m_rankItemDelegate);

	if ( !parent )
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
	delete rankItemDelegate();
}

void MachineListViewer::loadSavedViews()
{
	qmc2Config->beginGroup(QMC2_VIEWS_PREFIX);
	foreach (QString v, qmc2Config->childGroups()) {
		savedViews() << v;
		if ( qmc2Config->value(v + "/Attached", false).toBool() )
			attachedViews() << v;
	}
	qmc2Config->endGroup();
	savedViews().sort();
	attachedViews().sort();

	qmc2MainWindow->comboBoxViewSelect->blockSignals(true);
	for (int index = qmc2MainWindow->comboBoxViewSelect->count() - 1; index >= viewSelectSeparatorIndex(); index--)
		qmc2MainWindow->comboBoxViewSelect->removeItem(index);
	if ( !attachedViews().isEmpty() ) {
		qmc2MainWindow->comboBoxViewSelect->insertSeparator(viewSelectSeparatorIndex());
		qmc2MainWindow->comboBoxViewSelect->setItemData(viewSelectSeparatorIndex(), -1);
		for (int index = 0; index < attachedViews().count(); index++) {
			int insertIndex = viewSelectSeparatorIndex() + index + 1;
			qmc2MainWindow->comboBoxViewSelect->insertItem(insertIndex, attachedViews().at(index));
			qmc2MainWindow->comboBoxViewSelect->setItemIcon(insertIndex, QIcon(QString::fromUtf8(":/data/img/filtered_view.png")));
		}
	}
	qmc2MainWindow->comboBoxViewSelect->blockSignals(false);

	QList<QAction *> actions(qmc2MainWindow->menuView->actions());
	for (int index = actions.count() - 1; index >= viewSelectSeparatorIndex(); index--) {
		QAction *a = actions.at(index);
		qmc2MainWindow->menuView->removeAction(a);
		a->disconnect();
		delete a;
	}
	if ( !attachedViews().isEmpty() ) {
		qmc2MainWindow->menuView->addSeparator();
		for (int index = 0; index < attachedViews().count(); index++) {
			QAction *a = qmc2MainWindow->menuView->addAction(QIcon(QString::fromUtf8(":/data/img/filtered_view.png")), attachedViews().at(index));
			a->setToolTip(tr("Show attached view '%1'").arg(attachedViews().at(index)));
			a->setStatusTip(a->toolTip());
			connect(a, SIGNAL(triggered(bool)), qmc2MainWindow, SLOT(attachedViewAction_triggered(bool)));
		}
	}

	setSavedViewsLoaded(true);
}

void MachineListViewer::init()
{
	if ( m_initCalled )
		return;
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
			}
		}
	}
	lcdNumberRecordCount->display((int)model()->recordCount());
	connect(treeView->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(currentChanged(const QModelIndex &, const QModelIndex &)));
	connect(treeView->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(treeViewVerticalScrollChanged(int)));
	treeView->setFocus();
	m_initCalled = true;
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
	model()->resetModel();
	updateCurrentView();
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
	saveView();
}

void MachineListViewer::settingsRemoveView()
{
	if ( name().isEmpty() )
		return;
	qmc2Config->remove(QMC2_VIEWS_PREFIX + name());
}

void MachineListViewer::loadView(const QString &viewName)
{
	if ( viewName.isEmpty() )
		return;
	treeView->header()->restoreState(qmc2Config->value(QMC2_VIEWS_PREFIX + viewName + "/HeaderState", QByteArray()).toByteArray());
	// FIXME
	if ( !m_initCalled ) {
		init();
		comboBoxViewName->lineEdit()->setText(viewName);
	} else {
		comboBoxViewName->lineEdit()->setText(viewName);
		treeView->setFocus();
		on_toolButtonUpdateView_clicked();
	}
}

void MachineListViewer::saveView()
{
	if ( name().isEmpty() )
		return;
	qmc2Config->setValue(QMC2_VIEWS_PREFIX + name() + "/HeaderState", treeView->header()->saveState());
	// FIXME
}

void MachineListViewer::mainTagChanged(const QString &id, bool tagged)
{
	MachineListModelItem *item = model()->itemHash().value(id);
	if ( item ) {
		item->setTagged(tagged);
		model()->updateData(model()->index(item->row(), MachineListModel::TAG, QModelIndex()));
	}
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
	saveView();
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
	qmc2MainWindow->comboBoxViewSelect->blockSignals(true);
	QString curText(qmc2MainWindow->comboBoxViewSelect->currentText());
	for (int index = qmc2MainWindow->comboBoxViewSelect->count() - 1; index >= viewSelectSeparatorIndex(); index--)
		qmc2MainWindow->comboBoxViewSelect->removeItem(index);
	qmc2MainWindow->comboBoxViewSelect->insertSeparator(viewSelectSeparatorIndex());
	qmc2MainWindow->comboBoxViewSelect->setItemData(viewSelectSeparatorIndex(), -1);
	for (int index = 0; index < attachedViews().count(); index++) {
		int insertIndex = viewSelectSeparatorIndex() + index + 1;
		qmc2MainWindow->comboBoxViewSelect->insertItem(insertIndex, attachedViews().at(index));
		qmc2MainWindow->comboBoxViewSelect->setItemIcon(insertIndex, QIcon(QString::fromUtf8(":/data/img/filtered_view.png")));
	}
	int index = qmc2MainWindow->comboBoxViewSelect->findText(curText);
	if ( index > viewSelectSeparatorIndex() )
		qmc2MainWindow->comboBoxViewSelect->setCurrentIndex(index);
	qmc2MainWindow->comboBoxViewSelect->blockSignals(false);
	QList<QAction *> actions(qmc2MainWindow->menuView->actions());
	for (int index = actions.count() - 1; index >= viewSelectSeparatorIndex(); index--) {
		QAction *a = actions.at(index);
		qmc2MainWindow->menuView->removeAction(a);
		a->disconnect();
		delete a;
	}
	qmc2MainWindow->menuView->addSeparator();
	for (int index = 0; index < attachedViews().count(); index++) {
		QAction *a = qmc2MainWindow->menuView->addAction(QIcon(QString::fromUtf8(":/data/img/filtered_view.png")), attachedViews().at(index));
		a->setToolTip(tr("Show attached view '%1'").arg(attachedViews().at(index)));
		a->setStatusTip(a->toolTip());
		connect(a, SIGNAL(triggered(bool)), qmc2MainWindow, SLOT(attachedViewAction_triggered(bool)));
	}
}

void MachineListViewer::detachViewAction_triggered(bool)
{
	attachedViews().removeAll(name());
	foreach (MachineListViewer *v, MainWindow::machineListViewers)
		v->lineEdit_textChanged(v->name());
	qmc2Config->setValue(QMC2_VIEWS_PREFIX + name() + "/Attached", false);
	qmc2MainWindow->comboBoxViewSelect->blockSignals(true);
	for (int index = qmc2MainWindow->comboBoxViewSelect->count() - 1; index >= viewSelectSeparatorIndex(); index--)
		qmc2MainWindow->comboBoxViewSelect->removeItem(index);
	if ( !attachedViews().isEmpty() ) {
		qmc2MainWindow->comboBoxViewSelect->insertSeparator(viewSelectSeparatorIndex());
		qmc2MainWindow->comboBoxViewSelect->setItemData(viewSelectSeparatorIndex(), -1);
		for (int index = 0; index < attachedViews().count(); index++) {
			int insertIndex = viewSelectSeparatorIndex() + index + 1;
			qmc2MainWindow->comboBoxViewSelect->insertItem(insertIndex, attachedViews().at(index));
			qmc2MainWindow->comboBoxViewSelect->setItemIcon(insertIndex, QIcon(QString::fromUtf8(":/data/img/filtered_view.png")));
		}
	}
	qmc2MainWindow->comboBoxViewSelect->blockSignals(false);
	QList<QAction *> actions(qmc2MainWindow->menuView->actions());
	for (int index = actions.count() - 1; index >= viewSelectSeparatorIndex(); index--) {
		QAction *a = actions.at(index);
		qmc2MainWindow->menuView->removeAction(a);
		a->disconnect();
		delete a;
	}
	if ( !attachedViews().isEmpty() ) {
		qmc2MainWindow->menuView->addSeparator();
		for (int index = 0; index < attachedViews().count(); index++) {
			QAction *a = qmc2MainWindow->menuView->addAction(QIcon(QString::fromUtf8(":/data/img/filtered_view.png")), attachedViews().at(index));
			a->setToolTip(tr("Show attached view '%1'").arg(attachedViews().at(index)));
			a->setStatusTip(a->toolTip());
			connect(a, SIGNAL(triggered(bool)), qmc2MainWindow, SLOT(attachedViewAction_triggered(bool)));
		}
	}
	if ( qmc2MainWindow->attachedViewer() ) {
		if ( qmc2MainWindow->attachedViewer()->name() == name() )
			qmc2MainWindow->attachedViewer()->saveView();
		if ( !attachedViews().isEmpty() )
			qmc2MainWindow->comboBoxViewSelect->setCurrentIndex(viewSelectSeparatorIndex() + 1);
		else
			qmc2MainWindow->comboBoxViewSelect->setCurrentIndex(QMC2_VIEWMACHINELIST_INDEX);
	}
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

void MachineListViewer::updateCurrentView()
{
	int currentSortColumn = treeView->header()->sortIndicatorSection();
	Qt::SortOrder currentSortOrder = treeView->header()->sortIndicatorOrder();
	treeView->sortByColumn(currentSortColumn, currentSortOrder);
	m_ignoreSelectionChange = true;
	if ( qmc2CurrentItem ) {
		MachineListModelItem *item = model()->itemHash().value(qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_NAME));
		if ( item ) {
			m_currentId = item->id();
			int row = item->row();
			if ( row >= 0 ) {
				QModelIndex idx(model()->index(row, 0, QModelIndex()));
				treeView->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
				treeView->scrollTo(idx, qmc2CursorPositioningMode);
			}
		}
	}
	lcdNumberRecordCount->display((int)model()->recordCount());
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

void MachineListViewer::on_comboBoxViewName_activated(const QString &viewName)
{
	loadView(viewName);
}

void MachineListViewer::showEvent(QShowEvent *e)
{
	adjustIconSizes();
	if ( this != qmc2MainWindow->attachedViewer() ) {
		widgetMenu->setVisible(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MachineListViewer/MenuActive", true).toBool());
		restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MachineListViewer/Geometry", QByteArray()).toByteArray());
		treeView->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MachineListViewer/HeaderState", QByteArray()).toByteArray());
	} else {
		widgetMenu->setVisible(false);
		toolButtonToggleMenu->setVisible(false);
	}
	if ( e )
		QWidget::showEvent(e);
}

void MachineListViewer::hideEvent(QHideEvent *e)
{
	if ( this != qmc2MainWindow->attachedViewer() ) {
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MachineListViewer/MenuActive", widgetMenu->isVisible());
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MachineListViewer/Geometry", saveGeometry());
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MachineListViewer/HeaderState", treeView->header()->saveState());
	}
	if ( e )
		QWidget::hideEvent(e);
}
