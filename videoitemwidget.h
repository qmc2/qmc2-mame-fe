#if defined(QMC2_YOUTUBE_ENABLED)

#ifndef _VIDEOITEMWIDGET_H_
#define _VIDEOITEMWIDGET_H_

#include <Qt>
#include "ui_videoitemwidget.h"
#include "imagewidget.h"

#define VIDEOITEM_IMAGE_WIDTH					100
#define VIDEOITEM_IMAGE_HEIGHT					75

#define VIDEOITEM_TYPE_UNKNOWN					-1
#define VIDEOITEM_TYPE_YOUTUBE					0
#define VIDEOITEM_TYPE_YOUTUBE_SEARCH				1
#define VIDEOITEM_TYPE_LOCAL_MOVIE				2
#define VIDEOITEM_TYPE_VIDEO_SNAP				3

#define VIDEOITEM_YOUTUBE_URL_PATTERN				"http://www.youtube.com/watch?v=$VIDEO_ID$"
#define VIDEOITEM_YOUTUBE_URL_PATTERN_NO_COUNTRY_FILTER		"http://www.youtube.com/v/$VIDEO_ID$"
#define VIDEOITEM_YOUTUBE_AUTHOR_URL_PATTERN			"http://www.youtube.com/user/$USER_ID$"

class VideoItemWidget : public QWidget, public Ui::VideoItemWidget
{
	Q_OBJECT

	public:
		ImagePixmap videoImage;
		QString videoID;
		QString videoAuthor;
		QString videoTitle;
		QString videoUrlPattern;
		QString authorUrlPattern;
		bool videoImageValid;
		void *myVideoPlayer;
		int itemType;

		VideoItemWidget(QString, QString, QString, ImagePixmap *vImage = 0, int vType = VIDEOITEM_TYPE_YOUTUBE, void *vPlayer = 0, QWidget *parent = 0);

		bool closingState();

	public slots:
		void setImage(const ImagePixmap &, bool valid = true);
		void setImage(ImagePixmap *, bool valid = true);
		void setID(QString);
		void setTitle(QString);
		void setAuthor(QString);
		void setType(int);
};

#endif

#endif
