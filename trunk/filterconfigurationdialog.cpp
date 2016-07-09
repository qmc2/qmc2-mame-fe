#include <QSize>
#include <QFont>
#include <QTimer>
#include <QFontMetrics>

#include "filterconfigurationdialog.h"
#include "machinelistviewer.h"
#include "settings.h"
#include "macros.h"

extern Settings *qmc2Config;

FilterConfigurationDialog::FilterConfigurationDialog(MachineListViewer *viewer, QWidget *parent) :
	QDialog(parent),
	m_viewer(viewer)
{
	setVisible(false);
	setupUi(this);
	QTimer::singleShot(0, this, SLOT(init()));
}

void FilterConfigurationDialog::adjustIconSizes()
{
	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFontMetrics fm(f);
	QSize iconSize(fm.height() - 2, fm.height() - 2);
	QSize iconSizeTreeWidget(fm.height(), fm.height());
	toolButtonClearFilterExpression->setIconSize(iconSize);
	treeWidget->setIconSize(iconSizeTreeWidget);
}

void FilterConfigurationDialog::init()
{
	QList<int> pages(viewer()->pages());
	QStringList headers(viewer()->headers());
	for (int i = 0; i < headers.count(); i++) {
		QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget);
		item->setText(QMC2_FCDLG_COLUMN_NAME, headers.at(i));
		QToolButton *tb = new QToolButton;
		m_buttonToPageHash.insert(tb, pages.at(i));
		m_buttonToItemHash.insert(tb, item);
		tb->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
		tb->setIcon(QIcon(QString::fromUtf8(":/data/img/plus.png")));
		tb->setText(tr("Add filter"));
		tb->setFixedSize(tb->sizeHint());
		tb->setToolTip(tr("Add filter"));
		connect(tb, SIGNAL(clicked()), this, SLOT(addFilterClicked()));
		treeWidget->setItemWidget(item, QMC2_FCDLG_COLUMN_ACTION, tb);
	}
	treeWidget->resizeColumnToContents(QMC2_FCDLG_COLUMN_ACTION);
	treeWidget->sortByColumn(QMC2_FCDLG_COLUMN_NAME, Qt::AscendingOrder);
}

void FilterConfigurationDialog::addFilterClicked()
{
	QToolButton *tb = (QToolButton *)sender();
	int pageIndex = buttonToPage(tb);
	QTreeWidgetItem *parentItem = buttonToItem(tb);
	QTreeWidgetItem *item = new QTreeWidgetItem(parentItem);
	item->setText(QMC2_FCDLG_COLUMN_NAME, tr("Inactive filter"));
	item->setTextAlignment(QMC2_FCDLG_COLUMN_NAME, Qt::AlignRight | Qt::AlignVCenter);
	QToolButton *tbRemove = new QToolButton;
	m_removeButtonToItemHash.insert(tbRemove, item);
	tbRemove->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	tbRemove->setIcon(QIcon(QString::fromUtf8(":/data/img/minus.png")));
	tbRemove->setText(tr("Remove filter"));
	tbRemove->setFixedSize(tbRemove->sizeHint());
	tbRemove->setToolTip(tr("Remove filter"));
	connect(tbRemove, SIGNAL(clicked()), this, SLOT(removeFilterClicked()));
	treeWidget->setItemWidget(item, QMC2_FCDLG_COLUMN_ACTION, tbRemove);
	parentItem->setExpanded(true);
}

void FilterConfigurationDialog::removeFilterClicked()
{
	QToolButton *tb = (QToolButton *)sender();
	QTreeWidgetItem *item = m_removeButtonToItemHash.value(tb);
	item->parent()->removeChild(item);
	delete item;
	m_removeButtonToItemHash.remove(tb);
}

void FilterConfigurationDialog::on_pushButtonOk_clicked()
{
	on_pushButtonApply_clicked();
}

void FilterConfigurationDialog::on_pushButtonApply_clicked()
{
	// FIXME
	viewer()->toolButtonUpdateView->animateClick();
}

void FilterConfigurationDialog::on_pushButtonCancel_clicked()
{
	// FIXME
}

void FilterConfigurationDialog::showEvent(QShowEvent *e)
{
	adjustIconSizes();
	restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/FilterConfigurationDialog/Geometry", QByteArray()).toByteArray());
	if ( e )
		QDialog::showEvent(e);
}

void FilterConfigurationDialog::hideEvent(QHideEvent *e)
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/FilterConfigurationDialog/Geometry", saveGeometry());
	if ( e )
		QDialog::hideEvent(e);
}
