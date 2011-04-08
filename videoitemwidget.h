#ifndef _VIDEOITEMWIDGET_H_
#define _VIDEOITEMWIDGET_H_

#include <Qt>
#include "ui_videoitemwidget.h"

class VideoItemWidget : public QWidget, public Ui::VideoItemWidget
{
	Q_OBJECT

	public:
		QString videoDescription;
		QImage videoImage;

		VideoItemWidget(QImage &, QString &, QWidget *parent = 0);
		~VideoItemWidget();
};

#endif
