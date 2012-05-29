#include <QtGui>

#include "toolbarcustomizer.h"
#include "qmc2main.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;
extern QSettings *qmc2Config;

#define QMC2_DEBUG

ToolBarCustomizer::ToolBarCustomizer(QWidget *parent)
	: QDialog(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ToolBarCustomizer::ToolBarCustomizer(QWidget *parent = %1)").arg((qulonglong) parent));
#endif

	setupUi(this);
	
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
				}
			} else {
				QListWidgetItem *item = new QListWidgetItem(listWidgetAvailableActions);
				item->setText(action->statusTip());
				item->setIcon(action->icon());
				availableToolBarActions[item] = action;
			}
		}
	}
	QListWidgetItem *item = new QListWidgetItem(listWidgetAvailableActions);
	item->setText(tr("Tool-bar search box"));
	item->setIcon(QIcon(QString::fromUtf8(":/data/img/hint.png")));
	availableToolBarActions[item] = qmc2MainWindow->widgetActionToolbarSearch;
}

ToolBarCustomizer::~ToolBarCustomizer()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ToolBarCustomizer::~ToolBarCustomizer()");
#endif

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

}

void ToolBarCustomizer::on_pushButtonApply_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ToolBarCustomizer::on_pushButtonApply_clicked()");
#endif

}

void ToolBarCustomizer::on_pushButtonCancel_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ToolBarCustomizer::on_pushButtonCancel_clicked()");
#endif

}
