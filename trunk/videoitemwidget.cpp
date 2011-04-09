#include "macros.h"
#include "videoitemwidget.h"

#define QMC2_DEBUG

#ifdef QMC2_DEBUG
#include "qmc2main.h"
extern MainWindow *qmc2MainWindow;
#endif

VideoItemWidget::VideoItemWidget(QString vID, QString vDescription, QImage &vImage, int type, QWidget *parent)
  : QWidget(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: VideoItemWidget::VideoItemWidget(QString vID = %1, QString vDescription = ..., QImage &vImage = ..., nt type = %2, QWidget *parent = %3)").arg(vID).arg(type).arg((qulonglong) parent));
#endif

	setupUi(this);

	setType(type);
	setVideoID(vID);
	setImage(vImage);
	setVideoDescription(vDescription);
}

VideoItemWidget::VideoItemWidget(QString vID, QString vDescription, int type, QWidget *parent)
  : QWidget(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: VideoItemWidget::VideoItemWidget(QString vID = %1, QString vDescription = ..., int type = %2, QWidget *parent = %3)").arg(vID).arg(type).arg((qulonglong) parent));
#endif

	setupUi(this);

	setType(type);
	setVideoID(vID);
	QImage ghostImage = QImage(QString::fromUtf8(":/data/img/ghost_video.png"));
	setImage(ghostImage);
	setVideoDescription(vDescription);
}

VideoItemWidget::~VideoItemWidget()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: VideoItemWidget::~VideoItemWidget()");
#endif

}

void VideoItemWidget::setType(int type)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: VideoItemWidget::setType(int type = %1)").arg(type));
#endif

	itemType = type;
	switch ( itemType ) {
		case VIDEOITEM_TYPE_YOUTUBE:
		default:
			videoUrlPattern = VIDEOITEM_YOUTUBE_URL_PATTERN;
			break;
	}
}

void VideoItemWidget::setImage(QImage &vImage)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: VideoItemWidget::setImage(QImage &vImage = ...)");
#endif

	videoImage = vImage;
	labelVideoImage->setFixedSize(VIDEOITEM_IMAGE_WIDTH, VIDEOITEM_IMAGE_HEIGHT);
	labelVideoImage->setPixmap(QPixmap::fromImage(videoImage).scaled(VIDEOITEM_IMAGE_WIDTH, VIDEOITEM_IMAGE_HEIGHT, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void VideoItemWidget::setVideoID(QString vID)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: VideoItemWidget::setVideoID(QString vID = %1)").arg(vID));
#endif

	videoID = vID;
	if ( !videoDescription.isEmpty() )
		setVideoDescription(videoDescription);
}

void VideoItemWidget::setVideoDescription(QString vDescription)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: VideoItemWidget::setVideoDescription(QString vDescription = %1)").arg(vDescription));
#endif

	videoDescription = vDescription;
	QString htmlText = "<html><body>" + videoDescription;
	if ( !videoID.isEmpty() ) {
		if ( !videoUrlPattern.isEmpty() ) {
			QString url = videoUrlPattern;
			url.replace("$VIDEO_ID$", videoID);
			htmlText += "<p>" + tr("Video ID: %1").arg("<a href=\"" + url + "\" title=\"" + tr("Open video URL with the default browser") + "\">" + videoID + "</a>") + "</p>";
		} else
			htmlText += "<p>" + tr("Video ID: %1").arg(videoID) + "</p>";
	}
	htmlText += "</body></html>";
	textBrowserVideoDescription->setHtml(htmlText);
}
