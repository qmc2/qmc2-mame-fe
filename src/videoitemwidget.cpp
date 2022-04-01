#if defined(QMC2_YOUTUBE_ENABLED)

#include <QRegExp>

#include "macros.h"
#include "videoitemwidget.h"
#include "youtubevideoplayer.h"

VideoItemWidget::VideoItemWidget(QString vID, QString vTitle, QString vAuthor, ImagePixmap *vImage, int vType, void *vPlayer, QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);
	textBrowserVideoTitle->setObjectName("QMC2_VIDEO_TITLE");
	setAutoFillBackground(true);
	myVideoPlayer = vPlayer;
	setType(vType);
	setID(vID);
	setAuthor(vAuthor);
	if ( vType == VIDEOITEM_TYPE_YOUTUBE_SEARCH )
		labelVideoImage->hide();
	else if ( vImage )
		setImage(vImage, true);
	else
		setImage(ImagePixmap(QPixmap(QString::fromUtf8(":/data/img/video_thumbnail.png"))), false);
	setTitle(vTitle);
}

bool VideoItemWidget::closingState()
{
	switch ( itemType ) {
		case VIDEOITEM_TYPE_YOUTUBE:
		default:
			if ( ((YouTubeVideoPlayer *)myVideoPlayer)->forcedExit )
				return true;
			break;
	}
	return false;
}

void VideoItemWidget::setType(int type)
{
	itemType = type;
	switch ( itemType ) {
		case VIDEOITEM_TYPE_YOUTUBE_SEARCH:
			videoUrlPattern = VIDEOITEM_YOUTUBE_URL_PATTERN;
			//authorUrlPattern = VIDEOITEM_YOUTUBE_AUTHOR_URL_PATTERN;
			authorUrlPattern.clear();
			if ( myVideoPlayer ) {
				textBrowserVideoTitle->disconnect((YouTubeVideoPlayer *)myVideoPlayer);
				connect(textBrowserVideoTitle, SIGNAL(customContextMenuRequested(const QPoint &)), (YouTubeVideoPlayer *)myVideoPlayer, SLOT(on_listWidgetSearchResults_customContextMenuRequested(const QPoint &))); 
			}
			break;
		case VIDEOITEM_TYPE_LOCAL_MOVIE:
		case VIDEOITEM_TYPE_VIDEO_SNAP:
			videoUrlPattern.clear();
			authorUrlPattern.clear();
			if ( myVideoPlayer ) {
				textBrowserVideoTitle->disconnect((YouTubeVideoPlayer *)myVideoPlayer);
				connect(textBrowserVideoTitle, SIGNAL(customContextMenuRequested(const QPoint &)), (YouTubeVideoPlayer *)myVideoPlayer, SLOT(on_listWidgetAttachedVideos_customContextMenuRequested(const QPoint &))); 
			}
			break;
		case VIDEOITEM_TYPE_YOUTUBE:
		default:
			videoUrlPattern = VIDEOITEM_YOUTUBE_URL_PATTERN;
			//authorUrlPattern = VIDEOITEM_YOUTUBE_AUTHOR_URL_PATTERN;
			authorUrlPattern.clear();
			if ( myVideoPlayer ) {
				textBrowserVideoTitle->disconnect((YouTubeVideoPlayer *)myVideoPlayer);
				connect(textBrowserVideoTitle, SIGNAL(customContextMenuRequested(const QPoint &)), (YouTubeVideoPlayer *)myVideoPlayer, SLOT(on_listWidgetAttachedVideos_customContextMenuRequested(const QPoint &))); 
			}
			break;
	}
}

void VideoItemWidget::setImage(const ImagePixmap &vImage, bool valid)
{
	if ( closingState() )
		return;
	videoImageValid = valid;
	videoImage = vImage;
	videoImage.imagePath = vImage.imagePath;
	labelVideoImage->setPixmap(videoImage.scaled(VIDEOITEM_IMAGE_WIDTH, VIDEOITEM_IMAGE_HEIGHT, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	labelVideoImage->setFixedSize(VIDEOITEM_IMAGE_WIDTH, VIDEOITEM_IMAGE_HEIGHT);
	textBrowserVideoTitle->setFixedHeight(VIDEOITEM_IMAGE_HEIGHT);
}

void VideoItemWidget::setImage(ImagePixmap *vImage, bool valid)
{
	if ( closingState() )
		return;
	videoImageValid = valid;
	videoImage = *vImage;
	videoImage.imagePath = vImage->imagePath;
	labelVideoImage->setPixmap(((QPixmap)videoImage).scaled(VIDEOITEM_IMAGE_WIDTH, VIDEOITEM_IMAGE_HEIGHT, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	labelVideoImage->setFixedSize(VIDEOITEM_IMAGE_WIDTH, VIDEOITEM_IMAGE_HEIGHT);
	textBrowserVideoTitle->setFixedHeight(VIDEOITEM_IMAGE_HEIGHT);
}

void VideoItemWidget::setID(QString vID)
{
	if ( closingState() )
		return;
	if ( itemType == VIDEOITEM_TYPE_LOCAL_MOVIE || itemType == VIDEOITEM_TYPE_VIDEO_SNAP )
		videoImageValid = true;
	videoID = vID;
	if ( !videoTitle.isEmpty() )
		setTitle(videoTitle);
}

void VideoItemWidget::setAuthor(QString vAuthor)
{
	if ( closingState() )
		return;
	videoAuthor = vAuthor;
	if ( !videoTitle.isEmpty() )
		setTitle(videoTitle);
}

void VideoItemWidget::setTitle(QString vTitle)
{
	if ( closingState() )
		return;
	videoTitle = vTitle;
	QString htmlText = "<html><body><table cellpadding=\"0\" border=\"0\" width=\"100%\" height=\"100%\">";
	if ( itemType == VIDEOITEM_TYPE_LOCAL_MOVIE || itemType == VIDEOITEM_TYPE_VIDEO_SNAP ) {
		QString vidCopy = videoID;
		vidCopy.remove(QRegExp("^\\#\\:"));
		htmlText += "<tr><td width=\"5%\" align=\"right\" valign=\"top\">" + tr("Path:") + "</td><td width=\"95%\" valign=\"top\"><b>" + vidCopy + "</b></td></tr>";
	} else {
		htmlText += "<tr><td width=\"5%\" align=\"right\" valign=\"top\">" + tr("Title:") + "</td><td width=\"95%\" valign=\"top\"><b>" + videoTitle + "</b></td></tr>";
		if ( !videoAuthor.isEmpty() ) {
			if ( !authorUrlPattern.isEmpty() ) {
				QString url = authorUrlPattern;
				url.replace("$USER_ID$", videoAuthor);
				htmlText += "<tr><td width=\"5%\" align=\"right\" valign=\"top\">" + tr("Author:") + "</td><td width=\"95%\" valign=\"top\">" + "<a href=\"" + url + "\" title=\"" + tr("Open author URL with the default browser") + "\">" + videoAuthor.replace("+", " ") + "</a></td></tr>";
			} else
				htmlText += "<tr><td width=\"5%\" align=\"right\" valign=\"top\">" + tr("Author:") + "</td><td width=\"95%\" valign=\"top\">" + videoAuthor.replace("+", " ") + "</td></tr>";
		}
		if ( !videoID.isEmpty() ) {
			if ( !videoUrlPattern.isEmpty() ) {
				QString url = videoUrlPattern;
				url.replace("$VIDEO_ID$", videoID);
				htmlText += "<tr><td width=\"5%\" align=\"right\" valign=\"top\">" + tr("Video:") + "</td><td width=\"95%\" valign=\"top\">" + "<a href=\"" + url + "\" title=\"" + tr("Open video URL with the default browser") + "\">" + videoID + "</a></td></tr>";
			} else
				htmlText += "<tr><td width=\"5%\" align=\"right\" valign=\"top\">" + tr("Video:") + "</td><td width=\"95%\" valign=\"top\">" + videoID + "</td></tr>";
		}
	}
	htmlText += "</table></body></html>";
	textBrowserVideoTitle->setHtml(htmlText);
}

#endif
