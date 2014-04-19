#ifndef _RANKITEMWIDGET_H_
#define _RANKITEMWIDGET_H_

#include "ui_rankitemwidget.h"

class RankItemWidget : public QWidget, public Ui::RankItemWidget
{
	Q_OBJECT

       	public:
		RankItemWidget(QWidget *parent = 0);
		~RankItemWidget();

	public slots:

	signals:
};

#endif
