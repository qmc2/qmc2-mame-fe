#include <QPainter>
#include <QBrush>
#include <QPixmap>

#include "rankitemwidget.h"
#include "gamelist.h"
#include "macros.h"

extern Gamelist *qmc2Gamelist;

QImage RankItemWidget::rankBackround;
QImage RankItemWidget::rankSingle;
QLinearGradient RankItemWidget::rankGradient;

RankItemWidget::RankItemWidget(QString id, QWidget *parent)
	: QWidget(parent)
{
	m_id = id;
	if ( rankBackround.isNull() ) {
		rankBackround = QImage(QString::fromUtf8(":/data/img/rank_bg.png"));
		rankSingle = QImage(QString::fromUtf8(":/data/img/rank.png"));
		rankGradient = QLinearGradient(0, 0, rankBackround.width() - 1, 0);
		rankGradient.setColorAt(0, QColor(255, 255, 255, 128));
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

void RankItemWidget::updateRankFromDb()
{
	QPixmap pm(rankBackround.size());
	pm.fill(Qt::transparent);
	QPainter p;
	p.begin(&pm);
	p.setBrush(rankGradient);
	int selectedRank = qmc2Gamelist->userDataDb()->rank(m_id);
	for (int r = 0; r < selectedRank; r++) {
		int x = r * rankSingle.width();
		p.drawImage(x, 0, rankSingle);
		p.drawRoundedRect(x + 2, 3, rankSingle.width() - 2, rankSingle.height() - 3, 5, 5, Qt::RelativeSize);
	}
	p.drawImage(0, 0, rankBackround);
	p.end();
	rankImage->setPixmap(pm.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void RankItemWidget::updateRank(int mouseX)
{
	QPixmap pm(rankBackround.size());
	pm.fill(Qt::transparent);
	QPainter p;
	p.begin(&pm);
	p.setBrush(rankGradient);
	int selectedRank = int(0.5f + 6.0f * (double)mouseX / (double)(width()));
	for (int r = 0; r < selectedRank; r++) {
		int x = r * rankSingle.width();
		p.drawImage(x, 0, rankSingle);
		p.drawRoundedRect(x + 2, 3, rankSingle.width() - 2, rankSingle.height() - 3, 5, 5, Qt::RelativeSize);
	}
	p.drawImage(0, 0, rankBackround);
	p.end();
	rankImage->setPixmap(pm.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
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
