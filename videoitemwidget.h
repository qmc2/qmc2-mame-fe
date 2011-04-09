#ifndef _VIDEOITEMWIDGET_H_
#define _VIDEOITEMWIDGET_H_

#include <Qt>
#include "ui_videoitemwidget.h"

#define VIDEOITEM_IMAGE_WIDTH		100
#define VIDEOITEM_IMAGE_HEIGHT		75

#define VIDEOITEM_TYPE_UNKNOWN		-1
#define VIDEOITEM_TYPE_YOUTUBE		0

#define VIDEOITEM_YOUTUBE_URL_PATTERN	"http://www.youtube.com/v/$VIDEO_ID$"
//#define VIDEOITEM_YOUTUBE_URL_PATTERN	"http://www.youtube.com/watch?v=$VIDEO_ID$"

class VideoItemWidget : public QWidget, public Ui::VideoItemWidget
{
	Q_OBJECT

	public:
		QImage videoImage;
		QString videoID;
		QString videoDescription;
		QString videoUrlPattern;
		int itemType;

		VideoItemWidget(QString, QString, QImage &vImage, int type = VIDEOITEM_TYPE_YOUTUBE, QWidget *parent = 0);
		VideoItemWidget(QString, QString, int type = VIDEOITEM_TYPE_YOUTUBE, QWidget *parent = 0);
		~VideoItemWidget();

	public slots:
		void setImage(QImage &);
		void setVideoID(QString);
		void setVideoDescription(QString);
		void setType(int);
};

#endif
