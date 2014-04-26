#ifndef _RANKITEMWIDGET_H_
#define _RANKITEMWIDGET_H_

#include <QFontMetrics>
#include <QMouseEvent>
#include <QImage>
#include <QPixmap>
#include <QColor>
#include <QIcon>
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
		static QImage rankSingleFlat;
		static QLinearGradient rankGradient;
		static bool useFlatRankImage;
		static bool useColorRankImage;
		static QColor rankImageColor;

		static QIcon gradientRankIcon();
		static QIcon flatRankIcon();
		static QIcon colorRankIcon();
		static QIcon currentRankIcon() { return useFlatRankImage ? (useColorRankImage ? colorRankIcon() : flatRankIcon()) : gradientRankIcon(); }

		void setRank(int rank);
		int rank() { return m_rank; }

		void setRankComplete(int rank);

	public slots:
		void increaseRank();
		void decreaseRank();
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
