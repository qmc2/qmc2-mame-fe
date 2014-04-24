#include <QPainter>
#include <QBrush>
#include <QPixmap>
#include <QMap>

#include "rankitemwidget.h"
#include "gamelist.h"
#include "macros.h"

extern Gamelist *qmc2Gamelist;
extern QMap<QString, QTreeWidgetItem *> qmc2GamelistItemMap;
extern QMap<QString, QTreeWidgetItem *> qmc2HierarchyItemMap;
extern QMap<QString, QTreeWidgetItem *> qmc2CategoryItemMap;
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
extern QMap<QString, QTreeWidgetItem *> qmc2VersionItemMap;
#endif

QImage RankItemWidget::rankBackround;
QImage RankItemWidget::rankSingle;
QLinearGradient RankItemWidget::rankGradient;

RankItemWidget::RankItemWidget(QTreeWidgetItem *item, QWidget *parent)
	: QWidget(parent)
{
	m_item = item;
	if ( rankBackround.isNull() ) {
		rankBackround = QImage(QString::fromUtf8(":/data/img/rank_bg.png"));
		rankSingle = QImage(QString::fromUtf8(":/data/img/rank.png"));
		rankGradient = QLinearGradient(0, 0, rankBackround.width() - 1, 0);
		rankGradient.setColorAt(0, QColor(255, 255, 255, 127));
		rankGradient.setColorAt(1, Qt::transparent);
	}
	setupUi(this);
	setMouseTracking(true);
	updateSize();
}

void RankItemWidget::updateSize(QFontMetrics *fm)
{
	QSize newSize = size();
	if ( fm )
		newSize.scale(width(), fm->height(), Qt::KeepAspectRatio);
	else
		newSize.scale(width(), fontMetrics().height(), Qt::KeepAspectRatio);
	setFixedSize(newSize);
	QTimer::singleShot(0, this, SLOT(updateRankFromDb()));
}

void RankItemWidget::setRank(int rank)
{
	QPixmap pm(rankBackround.size());
	pm.fill(Qt::transparent);
	QPainter p;
	p.begin(&pm);
	p.setBrush(rankGradient);
	m_rank = rank;
	m_item->setWhatsThis(QMC2_GAMELIST_COLUMN_RANK, QString::number(m_rank));
	for (int r = 0; r < m_rank; r++) {
		int x = r * rankSingle.width();
		p.drawImage(x, 0, rankSingle);
		p.drawRoundedRect(x + 2, 3, rankSingle.width() - 2, rankSingle.height() - 3, 5, 5, Qt::RelativeSize);
	}
	p.drawImage(0, 0, rankBackround);
	p.end();
	rankImage->setPixmap(pm.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void RankItemWidget::updateRankFromDb()
{
	setRank(qmc2Gamelist->userDataDb()->rank(m_item->text(QMC2_GAMELIST_COLUMN_NAME)));
}

void RankItemWidget::updateRankFromMousePos(int mouseX)
{
	setRank(int(0.5f + 6.0f * (double)mouseX / (double)(width())));
	qmc2Gamelist->userDataDb()->setRank(m_item->text(QMC2_GAMELIST_COLUMN_NAME), m_rank);
	updateForeignItems();
}

void RankItemWidget::mousePressEvent(QMouseEvent *e)
{
	if ( e->buttons() & Qt::LeftButton )
		if ( rect().contains(e->pos()) )
			updateRankFromMousePos(e->x());
}

void RankItemWidget::mouseMoveEvent(QMouseEvent *e)
{
	if ( e->buttons() & Qt::LeftButton )
		if ( rect().contains(e->pos()) )
			updateRankFromMousePos(e->x());
}

void RankItemWidget::updateForeignItems()
{
	RankItemWidget *foreignRiw;
	QString myId = m_item->text(QMC2_GAMELIST_COLUMN_NAME);
	QTreeWidgetItem *item = qmc2GamelistItemMap[myId];
	if ( item && item != m_item ) {
		foreignRiw = (RankItemWidget *)item->treeWidget()->itemWidget(item, QMC2_GAMELIST_COLUMN_RANK);
		if ( foreignRiw )
			foreignRiw->setRank(m_rank);
	}
	item = qmc2HierarchyItemMap[myId];
	if ( item && item != m_item ) {
		foreignRiw = (RankItemWidget *)item->treeWidget()->itemWidget(item, QMC2_GAMELIST_COLUMN_RANK);
		if ( foreignRiw )
			foreignRiw->setRank(m_rank);
	}
	item = qmc2CategoryItemMap[myId];
	if ( item && item != m_item ) {
		foreignRiw = (RankItemWidget *)item->treeWidget()->itemWidget(item, QMC2_GAMELIST_COLUMN_RANK);
		if ( foreignRiw )
			foreignRiw->setRank(m_rank);
	}
#if defined(QMC2_EMUTYPE_MAME) || defined(QMC2_EMUTYPE_UME)
	item = qmc2VersionItemMap[myId];
	if ( item && item != m_item ) {
		foreignRiw = (RankItemWidget *)item->treeWidget()->itemWidget(item, QMC2_GAMELIST_COLUMN_RANK);
		if ( foreignRiw )
			foreignRiw->setRank(m_rank);
	}
#endif
}
