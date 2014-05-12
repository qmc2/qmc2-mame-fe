#include <QtGui>

#include "toolbarcustomizer.h"
#include "qmc2main.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;

ToolBarCustomizer::ToolBarCustomizer(QWidget *parent)
	: QDialog(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ToolBarCustomizer::ToolBarCustomizer(QWidget *parent = %1)").arg((qulonglong) parent));
#endif

	setupUi(this);

	resetToDefault = false;
	firstRefresh = true;

	int consecutiveSeparators = 0;
	foreach (QAction *action, qmc2MainWindow->toolbar->actions()) {
		if ( action->isSeparator() ) {
			if ( consecutiveSeparators++ < 1 )
				defaultToolBarActions << "--";
		} else if ( action->isVisible() && !action->icon().isNull() ) {
			defaultToolBarActions << action->objectName();
			consecutiveSeparators = 0;
		}
	}
	defaultToolBarActions << qmc2MainWindow->widgetActionToolbarSearch->objectName();
	separatorAction = new QAction(this);
	separatorAction->setSeparator(true);
	separatorAction->setObjectName("--");

	activeActions.clear();
	QTimer::singleShot(0, this, SLOT(refreshAvailableActions()));
}

ToolBarCustomizer::~ToolBarCustomizer()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ToolBarCustomizer::~ToolBarCustomizer()");
#endif

}

void ToolBarCustomizer::refreshAvailableActions()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ToolBarCustomizer::refreshAvailableActions()");
#endif

	listWidgetAvailableActions->clear();
	availableToolBarActions.clear();
	availableActionsByName.clear();
	foreach (QAction *menuBarAction, qmc2MainWindow->menuBar()->actions()) {
		foreach (QAction *action, menuBarAction->menu()->actions()) {
			if ( action->isSeparator() || !action->isVisible() || action->icon().isNull() )
				continue;
			if ( action->menu() ) {
				foreach (QAction *subAction, action->menu()->actions()) {
					if ( subAction->isSeparator() || !subAction->isVisible() || subAction->icon().isNull() )
						continue;
					QListWidgetItem *item = new QListWidgetItem(listWidgetAvailableActions);
					item->setText(subAction->statusTip());
					item->setIcon(subAction->icon());
					availableToolBarActions[item] = subAction;
					availableActionsByName[subAction->objectName()] = subAction;
				}
			} else {
				QListWidgetItem *item = new QListWidgetItem(listWidgetAvailableActions);
				item->setText(action->statusTip());
				item->setIcon(action->icon());
				availableToolBarActions[item] = action;
				availableActionsByName[action->objectName()] = action;
			}
		}
	}
	QListWidgetItem *item = new QListWidgetItem(listWidgetAvailableActions);
	item->setText(tr("Tool-bar search box"));
	item->setIcon(QIcon(QString::fromUtf8(":/data/img/find.png")));
	availableToolBarActions[item] = qmc2MainWindow->widgetActionToolbarSearch;
	availableActionsByName[qmc2MainWindow->widgetActionToolbarSearch->objectName()] = qmc2MainWindow->widgetActionToolbarSearch;
	QTimer::singleShot(0, this, SLOT(refreshActiveActions()));
}

void ToolBarCustomizer::refreshActiveActions()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ToolBarCustomizer::refreshActiveActions()");
#endif

	listWidgetActiveActions->clear();
	activeToolBarActions.clear();
	if ( activeActions.isEmpty() && !resetToDefault ) {
		activeActions = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/ToolBarActions", QStringList()).toStringList();
		appliedActions = activeActions;
	}
	if ( activeActions.isEmpty() )
		activeActions = defaultToolBarActions;
	foreach (QString actionName, activeActions) {
		if ( actionName == "--" ) {
			QListWidgetItem *item = new QListWidgetItem(listWidgetActiveActions);
			item->setText(tr("-- Separator --"));
			activeToolBarActions[item] = separatorAction;
		} else if ( actionName == "widgetActionToolbarSearch" ) {
			QListWidgetItem *item = new QListWidgetItem(listWidgetActiveActions);
			item->setText(tr("Tool-bar search box"));
			item->setIcon(QIcon(QString::fromUtf8(":/data/img/find.png")));
			activeToolBarActions[item] = qmc2MainWindow->widgetActionToolbarSearch;
		} else if ( availableActionsByName.contains(actionName) ) {
			QListWidgetItem *item = new QListWidgetItem(listWidgetActiveActions);
			QAction *action = availableActionsByName[actionName];
			item->setText(action->statusTip());
			item->setIcon(action->icon());
			activeToolBarActions[item] = action;
		}
	}

	if ( firstRefresh ) {
		on_pushButtonApply_clicked();
		firstRefresh = false;
	}

	resetToDefault = false;
}

void ToolBarCustomizer::adjustIconSizes()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ToolBarCustomizer::adjustIconSizes()");
#endif

	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFontMetrics fm(f);
	QSize iconSize = QSize(fm.height() - 2, fm.height() - 2);

	pushButtonOk->setIconSize(iconSize);
	pushButtonApply->setIconSize(iconSize);
	pushButtonCancel->setIconSize(iconSize);
	pushButtonActivateActions->setIconSize(iconSize);
	pushButtonDeactivateActions->setIconSize(iconSize);
	pushButtonActionUp->setIconSize(iconSize);
	pushButtonActionDown->setIconSize(iconSize);
	pushButtonInsertSeparator->setIconSize(iconSize);
	pushButtonDefault->setIconSize(iconSize);
	listWidgetAvailableActions->setIconSize(iconSize);
	listWidgetActiveActions->setIconSize(iconSize);
}

void ToolBarCustomizer::showEvent(QShowEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ToolBarCustomizer::showEvent(QShowEvent *e = %1)").arg((qulonglong)e));
#endif

	adjustIconSizes();
	QDialog::showEvent(e);
}

void ToolBarCustomizer::on_pushButtonOk_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ToolBarCustomizer::on_pushButtonOk_clicked()");
#endif

	on_pushButtonApply_clicked();
	accept();
}

void ToolBarCustomizer::on_pushButtonApply_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ToolBarCustomizer::on_pushButtonApply_clicked()");
#endif

	appliedActions.clear();
	QToolBar *tb = qmc2MainWindow->toolbar;
	tb->clear();
	for (int i = 0; i < listWidgetActiveActions->count(); i++) {
		QAction *action = activeToolBarActions[listWidgetActiveActions->item(i)];
		appliedActions << action->objectName();
		if ( action->objectName() == "--" )
			tb->addSeparator();
		else
			tb->addAction(action);
	}
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MainWidget/ToolBarActions", appliedActions);
}

void ToolBarCustomizer::on_pushButtonCancel_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ToolBarCustomizer::on_pushButtonCancel_clicked()");
#endif

	activeActions = appliedActions;
	refreshAvailableActions();
	reject();
}

void ToolBarCustomizer::on_pushButtonDefault_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ToolBarCustomizer::on_pushButtonDefault_clicked()");
#endif

	activeActions.clear();
	resetToDefault = true;
	refreshAvailableActions();
}

void ToolBarCustomizer::on_pushButtonActivateActions_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ToolBarCustomizer::on_pushButtonActivateActions_clicked()");
#endif

	foreach (QListWidgetItem *item, listWidgetAvailableActions->selectedItems()) {
		QAction *action = availableToolBarActions[item];
		if ( !activeToolBarActions.values().contains(action) ) {
			QListWidgetItem *activeItem = new QListWidgetItem(listWidgetActiveActions);
			activeItem->setText(item->text());
			activeItem->setIcon(item->icon());
			activeToolBarActions[activeItem] = action;
		}
	}
}

void ToolBarCustomizer::on_pushButtonDeactivateActions_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ToolBarCustomizer::on_pushButtonDeactivateActions_clicked()");
#endif

	foreach (QListWidgetItem *item, listWidgetActiveActions->selectedItems()) {
		QListWidgetItem *deactivateItem = listWidgetActiveActions->takeItem(listWidgetActiveActions->row(item));
		activeToolBarActions.remove(deactivateItem);
		delete deactivateItem;
	}
}

void ToolBarCustomizer::on_pushButtonActionUp_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ToolBarCustomizer::on_pushButtonActionUp_clicked()");
#endif

	foreach (QListWidgetItem *item, listWidgetActiveActions->selectedItems()) {
		if ( item ) {
			int row = listWidgetActiveActions->row(item);
			if ( row > 0 ) {
				QListWidgetItem *takenItem = listWidgetActiveActions->takeItem(row);
				if ( takenItem ) {
					listWidgetActiveActions->insertItem(row - 1, takenItem);
					listWidgetActiveActions->setCurrentItem(takenItem);
				}
			}
			listWidgetActiveActions->scrollToItem(item);
		}
	}
}

void ToolBarCustomizer::on_pushButtonActionDown_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ToolBarCustomizer::on_pushButtonActionDown_clicked()");
#endif

	foreach (QListWidgetItem *item, listWidgetActiveActions->selectedItems()) {
		if ( item ) {
			int row = listWidgetActiveActions->row(item);
			if ( row < listWidgetActiveActions->count() - 1 ) {
				QListWidgetItem *takenItem = listWidgetActiveActions->takeItem(row);
				if ( takenItem ) {
					listWidgetActiveActions->insertItem(row + 1, takenItem);
					listWidgetActiveActions->setCurrentItem(takenItem);
				}
			}
			listWidgetActiveActions->scrollToItem(item);
		}
	}
}

void ToolBarCustomizer::on_pushButtonInsertSeparator_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ToolBarCustomizer::on_pushButtonInsertSeparator_clicked()");
#endif

	QListWidgetItem *item = new QListWidgetItem(tr("-- Separator --"));
	if ( listWidgetActiveActions->selectedItems().isEmpty() )
		listWidgetActiveActions->insertItem(0, item);
	else
		listWidgetActiveActions->insertItem(listWidgetActiveActions->currentRow() + 1, item);
	listWidgetActiveActions->reset();
	listWidgetActiveActions->setCurrentItem(item);
	listWidgetActiveActions->scrollToItem(item);
	activeToolBarActions[item] = separatorAction;
}

void ToolBarCustomizer::on_listWidgetAvailableActions_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ToolBarCustomizer::on_listWidgetAvailableActions_itemSelectionChanged()");
#endif

	pushButtonActivateActions->setEnabled(listWidgetAvailableActions->selectedItems().count() > 0);
}
 
void ToolBarCustomizer::on_listWidgetActiveActions_itemSelectionChanged()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ToolBarCustomizer::on_listWidgetActiveActions_itemSelectionChanged()");
#endif

	if ( listWidgetActiveActions->selectedItems().count() > 0 ) {
		pushButtonDeactivateActions->setEnabled(true);
		if ( listWidgetActiveActions->selectedItems().count() == 1 ) {
			pushButtonActionUp->setEnabled(true);
			pushButtonActionDown->setEnabled(true);
		} else {
			pushButtonActionUp->setEnabled(false);
			pushButtonActionDown->setEnabled(false);
		}
	} else {
		pushButtonDeactivateActions->setEnabled(false);
		pushButtonActionUp->setEnabled(false);
		pushButtonActionDown->setEnabled(false);
	}
}

