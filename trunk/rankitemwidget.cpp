#include <QPainter>
#include <QPixmap>
#include <QHash>
#include <QApplication>
#include <QStyle>

#include "machinelistviewer.h"
#include "machinelistmodel.h"
#include "rankitemwidget.h"
#include "machinelist.h"
#include "settings.h"
#include "qmc2main.h"
#include "macros.h"

extern MachineList *qmc2MachineList;
extern Settings *qmc2Config;
extern QHash<QString, QTreeWidgetItem *> qmc2MachineListItemHash;
extern QHash<QString, QTreeWidgetItem *> qmc2HierarchyItemHash;
extern QHash<QString, QTreeWidgetItem *> qmc2CategoryItemHash;
extern QHash<QString, QTreeWidgetItem *> qmc2VersionItemHash;

QImage RankItemWidget::rankBackround;
QImage RankItemWidget::rankSingle;
QImage RankItemWidget::rankSingleFlat;
QLinearGradient RankItemWidget::rankGradient;
bool RankItemWidget::useFlatRankImage;
bool RankItemWidget::useColorRankImage;
bool RankItemWidget::ranksLocked = false;
QColor RankItemWidget::rankImageColor;

RankItemWidget::RankItemWidget(QTreeWidgetItem *item, QWidget *parent) :
	QWidget(parent),
	m_rank(0),
	m_item(item)
{
	setupUi(this);
	updateSize();
	if ( m_item ) {
		m_palette = m_item->treeWidget()->palette();
		QTimer::singleShot(0, this, SLOT(updateRankFromDb()));
	} else
		m_palette = qApp->palette();
}

QIcon RankItemWidget::gradientRankIcon()
{
	QPixmap pmRank(QPixmap::fromImage(rankSingle));
	QPainter pRank;
	pRank.begin(&pmRank);
	pRank.setCompositionMode(QPainter::CompositionMode_Overlay);
	pRank.fillRect(pmRank.rect(), rankImageColor);
	pRank.end();
	return QIcon(pmRank);
}

QIcon RankItemWidget::flatRankIcon()
{
	QPixmap pmRank(QPixmap::fromImage(rankSingleFlat));
	QPainter pRank;
	pRank.begin(&pmRank);
	pRank.setCompositionMode(QPainter::CompositionMode_Overlay);
	pRank.fillRect(pmRank.rect(), rankImageColor);
	pRank.end();
	return QIcon(pmRank);
}

QIcon RankItemWidget::colorRankIcon()
{
	QPixmap pmRank(QPixmap::fromImage(rankSingleFlat));
	QPainter pRank;
	pRank.begin(&pmRank);
	pRank.setCompositionMode(QPainter::CompositionMode_SourceIn);
	pRank.fillRect(pmRank.rect(), rankImageColor);
	pRank.end();
	return QIcon(pmRank);
}

void RankItemWidget::updateSize(QFontMetrics *fm)
{
	QSize newSize(size());
	if ( fm )
		newSize.scale(width(), QMC2_MAX(qApp->style()->pixelMetric(QStyle::PM_IndicatorHeight), fm->height()), Qt::KeepAspectRatio);
	else
		newSize.scale(width(), QMC2_MAX(qApp->style()->pixelMetric(QStyle::PM_IndicatorHeight), fontMetrics().height()), Qt::KeepAspectRatio);
	if ( !m_item )
		newSize += QSize(2, 2);
	setFixedSize(newSize);
}

void RankItemWidget::setRank(int rank)
{
	QPixmap pm(rankBackround.size());
	pm.fill(Qt::transparent);
	QPainter p;
	p.begin(&pm);
	p.setBrush(rankGradient);
	m_rank = rank;
	if ( m_item )
		m_item->setWhatsThis(QMC2_MACHINELIST_COLUMN_RANK, QString::number(m_rank));
	QPixmap pmRank(useFlatRankImage || useColorRankImage ? QPixmap::fromImage(rankSingleFlat) : QPixmap::fromImage(rankSingle));
	QPainter pRank;
	pRank.begin(&pmRank);
	pRank.setCompositionMode(useColorRankImage ? QPainter::CompositionMode_SourceIn : QPainter::CompositionMode_Overlay);
	pRank.fillRect(pmRank.rect(), rankImageColor);
	pRank.end();
	for (int r = 0; r < 6; r++) {
		int x = r * rankSingle.width();
		if ( r < m_rank )
			p.drawPixmap(x, 0, pmRank);
		else
			p.fillRect(x, 0, rankSingle.width(), rankSingle.height(), m_palette.brush(QPalette::Base));
		p.drawRoundedRect(x + 2, 3, rankSingle.width() - 2, rankSingle.height() - 3, 5, 5, Qt::RelativeSize);
	}
	QPixmap pmBackground(QPixmap::fromImage(rankBackround));
	QPainter pBackground;
	pBackground.begin(&pmBackground);
	pBackground.setCompositionMode(QPainter::CompositionMode_SourceIn);
	pBackground.fillRect(pmBackground.rect(), m_palette.color(QPalette::Text));
	pBackground.end();
	p.drawPixmap(0, 0, pmBackground);
	p.end();
	rankImage->setPixmap(pm.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
	emit rankChanged(m_rank);
}

void RankItemWidget::setRankComplete(int rank)
{
	if ( rank != m_rank ) {
		setRank(rank);
		if ( m_item ) {
			qmc2MachineList->userDataDb()->setRank(m_item->text(QMC2_MACHINELIST_COLUMN_NAME), m_rank);
			updateForeignItems();
		}
	}
}

void RankItemWidget::increaseRank()
{
	int newRank = m_rank + 1;
	if ( newRank < 6 )
		setRankComplete(newRank);
}

void RankItemWidget::decreaseRank()
{
	int newRank = m_rank - 1;
	if ( newRank >= 0 )
		setRankComplete(newRank);
}

void RankItemWidget::updateRankFromDb()
{
	if ( m_item )
		setRank(qmc2MachineList->userDataDb()->rank(m_item->text(QMC2_MACHINELIST_COLUMN_NAME)));
}

void RankItemWidget::updateRankFromMousePos(int mouseX)
{
	if ( m_item && RankItemWidget::ranksLocked )
		return;
	setRankComplete(int(0.5f + 6.0f * (double)mouseX / (double)(width())));
}

void RankItemWidget::mousePressEvent(QMouseEvent *e)
{
	if ( e->buttons() & Qt::LeftButton )
		if ( rect().contains(e->pos()) )
			updateRankFromMousePos(e->x());
	e->ignore();
}

void RankItemWidget::mouseMoveEvent(QMouseEvent *e)
{
	if ( e->buttons() & Qt::LeftButton )
		if ( rect().contains(e->pos()) )
			updateRankFromMousePos(e->x());
	e->ignore();
}

void RankItemWidget::updateForeignItems()
{
	RankItemWidget *foreignRiw;
	QString myId;
	if ( m_item )
		myId = m_item->text(QMC2_MACHINELIST_COLUMN_NAME);
	QTreeWidgetItem *item = qmc2MachineListItemHash.value(myId);
	if ( item && item != m_item ) {
		foreignRiw = (RankItemWidget *)item->treeWidget()->itemWidget(item, QMC2_MACHINELIST_COLUMN_RANK);
		if ( foreignRiw )
			foreignRiw->setRank(m_rank);
	}
	item = qmc2HierarchyItemHash.value(myId);
	if ( item && item != m_item ) {
		foreignRiw = (RankItemWidget *)item->treeWidget()->itemWidget(item, QMC2_MACHINELIST_COLUMN_RANK);
		if ( foreignRiw )
			foreignRiw->setRank(m_rank);
	}
	item = qmc2CategoryItemHash.value(myId);
	if ( item && item != m_item ) {
		foreignRiw = (RankItemWidget *)item->treeWidget()->itemWidget(item, QMC2_MACHINELIST_COLUMN_RANK);
		if ( foreignRiw )
			foreignRiw->setRank(m_rank);
	}
	item = qmc2VersionItemHash.value(myId);
	if ( item && item != m_item ) {
		foreignRiw = (RankItemWidget *)item->treeWidget()->itemWidget(item, QMC2_MACHINELIST_COLUMN_RANK);
		if ( foreignRiw )
			foreignRiw->setRank(m_rank);
	}
	foreach (MachineListViewer *v, MainWindow::machineListViewers) {
		MachineListModelItem *foreignItem = v->model()->itemHash().value(myId);
		if ( foreignItem ) {
			foreignItem->setRank(m_rank);
			v->treeView->viewport()->update();
		}
	}
}
