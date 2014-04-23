#include <QPainter>
#include <QPixmap>

#include "rankitemwidget.h"
#include "gamelist.h"
#include "macros.h"

extern Gamelist *qmc2Gamelist;

RankItemWidget::RankItemWidget(QString id, QWidget *parent)
	: QWidget(parent)
{
	m_id = id;
	setupUi(this);
	setMouseTracking(true);
	updateSize();
	QTimer::singleShot(0, this, SLOT(updateRankFromDb()));
}

void RankItemWidget::updateSize(QFontMetrics *fm)
{
	QSize newSize = size();
	if ( fm )
		newSize.scale(width(), fm->height(), Qt::KeepAspectRatio);
	else
		newSize.scale(width(), fontMetrics().height(), Qt::KeepAspectRatio);
	setFixedSize(newSize);
}

void RankItemWidget::updateRankFromDb()
{
	QPixmap rank_bg(QString::fromUtf8(":/data/img/rank_bg.png"));
	QPixmap rank(QString::fromUtf8(":/data/img/rank.png"));
	QPainter p;
	p.begin(&rank_bg);
	int selectedRank = qmc2Gamelist->userDataDb()->rank(m_id);
	for (int r = 0; r < selectedRank; r++) {
		int x = r * rank.width();
		p.drawPixmap(x, 0, rank);
	}
	p.end();
	rankImage->setPixmap(rank_bg);
}

void RankItemWidget::updateRank(int mouseX)
{
	QPixmap rank_bg(QString::fromUtf8(":/data/img/rank_bg.png"));
	QPixmap rank(QString::fromUtf8(":/data/img/rank.png"));
	QPainter p;
	p.begin(&rank_bg);
	int selectedRank = int(0.5f + 6.0f * (double)mouseX / (double)(width()));
	for (int r = 0; r < selectedRank; r++) {
		int x = r * rank.width();
		p.drawPixmap(x, 0, rank);
	}
	p.end();
	rankImage->setPixmap(rank_bg);
	qmc2Gamelist->userDataDb()->setRank(m_id, selectedRank);
}

void RankItemWidget::mousePressEvent(QMouseEvent *e)
{
	if ( e->buttons() & Qt::LeftButton )
		if ( rect().contains(e->pos()) )
			updateRank(e->x());
}

void RankItemWidget::mouseMoveEvent(QMouseEvent *e)
{
	if ( e->buttons() & Qt::LeftButton )
		if ( rect().contains(e->pos()) )
			updateRank(e->x());
}
