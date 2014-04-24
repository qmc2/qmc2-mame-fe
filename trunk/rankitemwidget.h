#ifndef _RANKITEMWIDGET_H_
#define _RANKITEMWIDGET_H_

#include <QFontMetrics>
#include <QMouseEvent>
#include <QImage>
#include <QLinearGradient>
#include <QTreeWidgetItem>

#include "ui_rankitemwidget.h"

class RankItemWidget : public QWidget, public Ui::RankItemWidget
{
	Q_OBJECT

       	public:
		RankItemWidget(QTreeWidgetItem *item, QWidget *parent = 0);

		bool checkSize(QFontMetrics *fm) { return rankImage->pixmap()->height() == fm->height(); }

		static QImage rankBackround;
		static QImage rankSingle;
		static QLinearGradient rankGradient;

		void setRank(int rank);
		int rank() { return m_rank; }

	public slots:
		void updateSize(QFontMetrics *fm = 0);
		void updateRankFromDb();
		void updateRankFromMousePos(int mouseX = 0);

	protected:
		void mousePressEvent(QMouseEvent *e);
		void mouseMoveEvent(QMouseEvent *e);

	private:
		void updateForeignItems();
		int m_rank;
		QTreeWidgetItem *m_item;
};

#endif
