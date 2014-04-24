#ifndef _RANKITEMWIDGET_H_
#define _RANKITEMWIDGET_H_

#include <QFontMetrics>
#include <QMouseEvent>
#include <QImage>

#include "ui_rankitemwidget.h"

class RankItemWidget : public QWidget, public Ui::RankItemWidget
{
	Q_OBJECT

       	public:
		RankItemWidget(QString id, QWidget *parent = 0);

		bool checkSize(QFontMetrics *fm) { return rankImage->pixmap()->height() == fm->height(); }

		static QImage rank_bg;
		static QImage rank;

	public slots:
		void updateSize(QFontMetrics *fm = 0);
		void updateRankFromDb();
		void updateRank(int mouseX = 0);

	protected:
		void mousePressEvent(QMouseEvent *e);
		void mouseMoveEvent(QMouseEvent *e);

	private:
		QString m_id;
};

#endif
