#include <QHash>
#include <QPixmap>
#include <QPainter>
#include <QHelpEvent>
#include <QMouseEvent>
#include <QFontMetrics>
#include <QApplication>
#include <QTreeWidgetItem>

#include "machinelistviewer.h"
#include "rankitemdelegate.h"
#include "rankitemwidget.h"
#include "machinelist.h"
#include "qmc2main.h"
#include "macros.h"

extern MachineList *qmc2MachineList;
extern QHash<QString, QTreeWidgetItem *> qmc2MachineListItemHash;
extern QHash<QString, QTreeWidgetItem *> qmc2HierarchyItemHash;
extern QHash<QString, QTreeWidgetItem *> qmc2CategoryItemHash;
extern QHash<QString, QTreeWidgetItem *> qmc2VersionItemHash;

RankItemDelegate::RankItemDelegate(QWidget *parent) :
	QStyledItemDelegate(parent),
	m_treeView((QTreeView *)parent)
{
	// NOP
}

void RankItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	int rank = index.data().toInt();
	if ( option.state & QStyle::State_Selected )
		painter->fillRect(option.rect, option.palette.highlight());
	QPixmap pm(RankItemWidget::rankBackround.size());
	pm.fill(Qt::transparent);
	QPainter p;
	p.begin(&pm);
	p.setBrush(RankItemWidget::rankGradient);
	QPixmap pmRank(RankItemWidget::useFlatRankImage || RankItemWidget::useColorRankImage ? QPixmap::fromImage(RankItemWidget::rankSingleFlat) : QPixmap::fromImage(RankItemWidget::rankSingle));
	QPainter pRank;
	pRank.begin(&pmRank);
	pRank.setCompositionMode(RankItemWidget::useColorRankImage ? QPainter::CompositionMode_SourceIn : QPainter::CompositionMode_Overlay);
	pRank.fillRect(pmRank.rect(), RankItemWidget::rankImageColor);
	pRank.end();
	for (int r = 0; r < 6; r++) {
		int x = r * RankItemWidget::rankSingle.width();
		if ( r < rank )
			p.drawPixmap(x, 0, pmRank);
		else
			p.fillRect(x, 0, RankItemWidget::rankSingle.width(), RankItemWidget::rankSingle.height(), option.palette.brush(QPalette::Base));
		p.drawRoundedRect(x + 2, 3, RankItemWidget::rankSingle.width() - 2, RankItemWidget::rankSingle.height() - 3, 5, 5, Qt::RelativeSize);
	}
	QPixmap pmBackground(QPixmap::fromImage(RankItemWidget::rankBackround));
	QPainter pBackground;
	pBackground.begin(&pmBackground);
	pBackground.setCompositionMode(QPainter::CompositionMode_SourceIn);
	pBackground.fillRect(pmBackground.rect(), option.palette.color(QPalette::Text));
	pBackground.end();
	p.drawPixmap(0, 0, pmBackground);
	p.end();
	painter->drawPixmap(option.rect.topLeft().x(), option.rect.topLeft().y() + 1, pm.scaled(sizeHint(option, index), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

QSize RankItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QFontMetrics fm(option.font);
	QSize s(RankItemWidget::rankBackround.size());
	s.scale(s.width(), QMC2_MAX(qApp->style()->pixelMetric(QStyle::PM_IndicatorHeight), fm.height() - 2), Qt::KeepAspectRatio);
	return s;
}

bool RankItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
	if ( RankItemWidget::ranksLocked )
		return false;
	bool doUpdateRank = false;
	switch ( event->type() ) {
		case QEvent::MouseButtonPress:
			doUpdateRank = ((QMouseEvent *)event)->button() == Qt::LeftButton;
			break;
		case QEvent::MouseMove:
			doUpdateRank = true;
			break;
		default:
			break;
	}
	if ( doUpdateRank ) {
		MachineListModelItem *item = ((MachineListModel *)model)->itemFromIndex(index);
		if ( item ) {
			QRect r(((MachineListModel *)model)->treeView()->visualRect(index));
			QPoint p(((QMouseEvent *)event)->pos());
			int rank = int(6.0f * (double)(p.x() - r.x()) / (double)sizeHint(option, index).width());
			item->setRank(rank);
			qmc2MachineList->userDataDb()->setRank(item->id(), rank);
			updateForeignItems(item);
		}
	}
	return false;
}

void RankItemDelegate::updateForeignItems(MachineListModelItem *item)
{
	foreach (MachineListViewer *v, MainWindow::machineListViewers) {
		if ( v != item->treeView()->parent() ) {
			MachineListModelItem *foreignItem = v->model()->itemHash().value(item->id());
			if ( foreignItem )
				foreignItem->setRank(item->rank());
		}
		v->treeView->viewport()->update();
	}
	QTreeWidgetItem *twi = qmc2MachineListItemHash.value(item->id());
	if ( twi ) {
		RankItemWidget *riw = (RankItemWidget *)twi->treeWidget()->itemWidget(twi, QMC2_MACHINELIST_COLUMN_RANK);
		if ( riw )
			riw->setRank(item->rank());
	}
	twi = qmc2HierarchyItemHash.value(item->id());
	if ( twi ) {
		RankItemWidget *riw = (RankItemWidget *)twi->treeWidget()->itemWidget(twi, QMC2_MACHINELIST_COLUMN_RANK);
		if ( riw )
			riw->setRank(item->rank());
	}
	twi = qmc2CategoryItemHash.value(item->id());
	if ( twi ) {
		RankItemWidget *riw = (RankItemWidget *)twi->treeWidget()->itemWidget(twi, QMC2_MACHINELIST_COLUMN_RANK);
		if ( riw )
			riw->setRank(item->rank());
	}
	twi = qmc2VersionItemHash.value(item->id());
	if ( twi ) {
		RankItemWidget *riw = (RankItemWidget *)twi->treeWidget()->itemWidget(twi, QMC2_MACHINELIST_COLUMN_RANK);
		if ( riw )
			riw->setRank(item->rank());
	}
}
