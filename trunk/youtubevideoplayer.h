#if defined(QMC2_YOUTUBE_ENABLED)

#ifndef _YOUTUBEVIDEOPLAYER_H_
#define _YOUTUBEVIDEOPLAYER_H_

#include <qglobal.h>
#include <QtGui>
#include <QtXml>
#include <QtNetwork>
#include <QMap>
#include <QDesktopWidget>
#include <QPainterPath>
#if QT_VERSION >= 0x050000
#include <QMediaPlayer>
#include <QVideoWidget>
#endif

#include "ui_youtubevideoplayer.h"
#include "videoitemwidget.h"
#if QT_VERSION < 0x050000
#include "qmc2_phonon.h"
#endif

// supported YouTube formats (see http://en.wikipedia.org/wiki/YouTube#Quality_and_codecs)
#define YOUTUBE_FORMAT_COUNT		7
#define YOUTUBE_FORMAT_FLV_240P		"5"
#define YOUTUBE_FORMAT_FLV_240P_INDEX	0
#define YOUTUBE_FORMAT_FLV_360P		"34"
#define YOUTUBE_FORMAT_FLV_360P_INDEX	1
#define YOUTUBE_FORMAT_MP4_360P		"18"
#define YOUTUBE_FORMAT_MP4_360P_INDEX	2
#define YOUTUBE_FORMAT_FLV_480P		"35"
#define YOUTUBE_FORMAT_FLV_480P_INDEX	3
#define YOUTUBE_FORMAT_MP4_720P		"22"
#define YOUTUBE_FORMAT_MP4_720P_INDEX	4
#define YOUTUBE_FORMAT_MP4_1080P	"37"
#define YOUTUBE_FORMAT_MP4_1080P_INDEX	5
#define YOUTUBE_FORMAT_MP4_3072P	"38"
#define YOUTUBE_FORMAT_MP4_3072P_INDEX	6
#define YOUTUBE_FORMAT_UNKNOWN_INDEX	-1

// tool-box pages
#define YOUTUBE_ATTACHED_VIDEOS_PAGE	0
#define YOUTUBE_VIDEO_PLAYER_PAGE	1
#define YOUTUBE_SEARCH_VIDEO_PAGE	2

// Play-O-Matic modes
#define YOUTUBE_PLAYOMATIC_SEQUENTIAL	0
#define YOUTUBE_PLAYOMATIC_RANDOM	1

// Play-O-Matic delay between videos
#define YOUTUBE_PLAYOMATIC_DELAY	1000

// timeout and reply polling (wait) time for video info requests in ms
#define YOUTUBE_VIDEOINFO_TIMEOUT	10000
#define YOUTUBE_VIDEOINFO_WAIT		10

class YouTubeVideoInfo
{
	public:
		QString author, title;

		YouTubeVideoInfo() { ; }
		YouTubeVideoInfo(QString t, QString a) { title = t; author = a; }
};

class VideoOverlayWidget : public QWidget
{
	Q_OBJECT

	public:
		VideoOverlayWidget(QWidget *parent = 0) : QWidget(parent, Qt::Tool | Qt::CustomizeWindowHint | Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint)
		{
			hide();
			setVideoWidget(parent);
			setAttribute(Qt::WA_TransparentForMouseEvents);
			setAttribute(Qt::WA_NoSystemBackground);
			setAttribute(Qt::WA_TranslucentBackground);
			setAttribute(Qt::WA_ShowWithoutActivating);
			connect(&clearMessageTimer, SIGNAL(timeout()), this, SLOT(clearMessage()));
		}

		void setVideoWidget(QWidget *widget) { mVideoWidget = widget; }
		QWidget *videoWidget() { return mVideoWidget; }

	public slots:
		void showMessage(QString message, int timeout = 2000)
		{
			if ( videoWidget()->isVisible() ) {
				QRect r;
				messageText = message;
				if ( videoWidget()->isFullScreen() )
					r = qApp->desktop()->rect();
				else
					r = videoWidget()->rect();
				QFont f(qApp->font());
				f.setWeight(QFont::Bold);
				QFontMetrics fm(f);
				QRect adjRect = fm.boundingRect(r, Qt::AlignCenter, "W");
				r = fm.boundingRect(r, Qt::AlignCenter | Qt::TextWordWrap, messageText);
				r.adjust(-adjRect.width()*2, -adjRect.height(), +adjRect.width()*2, +adjRect.height());
				int myHeight = r.height();
				if ( videoWidget()->isFullScreen() )
					r.setBottom(qApp->desktop()->rect().bottom());
				else
					r.setBottom(videoWidget()->rect().bottom());
				r.setTop(r.bottom() - myHeight);
				resize(r.size());
				move(videoWidget()->mapToGlobal(r.topLeft()));
				show();
				repaint();
				clearMessageTimer.start(timeout);
			} else
				hide();
		}
		void clearMessage()
		{
			clearMessageTimer.stop();
			messageText.clear();
			hide();
		}

	protected:
		void paintEvent(QPaintEvent *)
		{
			if ( !messageText.isEmpty() && videoWidget()->isVisible() ) {
				QPainter p;
				p.begin(this);
				QRect r = rect();
				QFont f(qApp->font());
				f.setWeight(QFont::Bold);
				p.setFont(f);
				QFontMetrics fm(f);
				QPainterPath pp;
				pp.addRoundedRect(r, 5, 5);
				p.fillPath(pp, QBrush(QColor(0, 0, 0, 128), Qt::SolidPattern));
				p.setPen(QPen(QColor(255, 255, 255, 255)));
				p.drawText(r, Qt::AlignCenter | Qt::TextWordWrap, messageText);
				p.end();
			} else
				hide();
		}

	private:
		QString messageText;
		QTimer clearMessageTimer;
		QWidget *mVideoWidget;
};

class YouTubeVideoPlayer;

class VideoEventFilter : public QObject
{
	Q_OBJECT

	public:
		VideoEventFilter(YouTubeVideoPlayer *player, QObject *parent = 0) : QObject(parent) { mPlayer = player; }

	protected:
		bool eventFilter(QObject *, QEvent *);

	private:
		YouTubeVideoPlayer *mPlayer;
};

class YouTubeVideoPlayer : public QWidget, public Ui::YouTubeVideoPlayer
{
	Q_OBJECT

	public:
		YouTubeVideoPlayer(QString, QString, QWidget *parent = 0);
		~YouTubeVideoPlayer();

		qint64 videoSeqNum;
		QString currentVideoID;
		QString currentVideoAuthor;
		QString currentVideoTitle;
		QString mySetID;
		QString mySetName;
		QString suggestorAppendString;
		QString savedSearchString;
		QStringList youTubeFormats;
		QStringList youTubeFormatNames;
		QNetworkReply *videoInfoReply, *videoImageReply, *searchRequestReply;
		QNetworkAccessManager *videoInfoManager, *videoImageManager, *searchRequestManager, *imageDownloadManager;
		QNetworkRequest videoInfoRequest, videoImageRequest, searchRequest;
		QString videoInfoBuffer, videoImageBuffer, searchRequestBuffer;
		QAction *videoMenuPlayPauseAction;
		QAction *videoMenuFullscreenAction;
		QAction *videoMenuCopyVideoUrlAction;
		QAction *videoMenuCopyVideoUrlAltAction;
		QAction *videoMenuCopyAuthorUrlAction;
		QAction *videoMenuPasteVideoUrlAction;
		QAction *videoMenuAttachVideoAction;
		QAction *autoSuggestAction;
		QAction *avmActionPlayVideo;
		QAction *avmActionCopyVideoUrl;
		QAction *avmActionCopyVideoUrlAlt;
		QAction *avmActionCopyAuthorUrl;
		QAction *avmActionPasteVideoUrl;
		QAction *avmActionRemoveVideos;
		QStringList playedVideos;
		QMap<QString, VideoItemWidget *> viwMap;
		bool viFinished, vimgFinished;
		bool viError, vimgError;
		bool loadOnly;
		bool isMuted;
		bool pausedByHideEvent;
		bool forcedExit;
		bool fullyLoaded;
		QList<int> availableFormats;
		int bestAvailableFormat;
		int currentFormat;
		QMenu *menuAttachedVideos;
		QMenu *menuSearchResults;
		QMenu *menuVideoPlayer;
		QMenu *menuSuggestButton;
		VideoOverlayWidget *videoOverlayWidget;
		VideoEventFilter *videoEventFilter;

		QUrl getVideoStreamUrl(QString, QStringList *videoInfoStringList = NULL, bool videoInfoOnly = false);
		QString indexToFormat(int);

#if QT_VERSION < 0x050000
		bool isPlaying() { return videoPlayer()->isPlaying(); }
		bool isPaused() { return videoPlayer()->isPaused(); }
		bool hasVideo() { return mediaObject()->hasVideo(); }
		Phonon::VideoPlayer *videoPlayer() { return mVideoPlayer; }
		Phonon::VideoWidget *videoWidget() { return mVideoPlayer->videoWidget(); }
		Phonon::AudioOutput *audioOutput() { return mVideoPlayer->audioOutput(); }
		Phonon::MediaObject *mediaObject() { return mVideoPlayer->mediaObject(); }
		qint64 remainingTime() { return mediaObject()->remainingTime(); }
#else
		bool isPlaying() { return videoPlayer()->state() == QMediaPlayer::PlayingState; }
		bool isPaused() { return videoPlayer()->state() == QMediaPlayer::PausedState; }
		bool hasVideo() { return videoPlayer()->isVideoAvailable(); }
		QMediaPlayer *videoPlayer() { return mVideoPlayer; }
		QVideoWidget *videoWidget() { if ( mFullscreenVideoWidget ) return mFullscreenVideoWidget; else return mVideoWidget; }
		QMediaPlayer *audioOutput() { return mVideoPlayer; }
		QMediaPlayer *mediaObject() { return mVideoPlayer; }
		qint64 remainingTime() { return videoPlayer()->duration() - videoPlayer()->position(); }
#endif
		void showMessage(QString message, int timeout = 2000) { videoOverlayWidget->showMessage(message, timeout); }
		void clearMessage() { videoOverlayWidget->clearMessage(); }

	public slots:
		void play() { videoPlayer()->play(); }
		void pause() { videoPlayer()->pause(); }
		void stop() { videoPlayer()->stop(); }

		void init();
		void adjustIconSizes();
		void saveSettings();

		void playMovieFile(QString &);
		void attachMovieFile();
		void playVideo(QString &);
		void loadVideo(QString &);
		void loadNullVideo();
		void playNextVideo();
		void videoTick(qint64);
		void videoFinished();
#if QT_VERSION < 0x050000
		void videoStateChanged(Phonon::State, Phonon::State);
#else
		void videoStateChanged(QMediaPlayer::State);
#endif
		void videoBufferStatus(int);

		void videoInfoReadyRead();
		void videoInfoError(QNetworkReply::NetworkError);
		void videoInfoFinished();
		void videoImageReadyRead();
		void videoImageError(QNetworkReply::NetworkError);
		void videoImageFinished();
		void searchRequestReadyRead();
		void searchRequestError(QNetworkReply::NetworkError);
		void searchRequestFinished();

		void on_toolButtonPlayPause_clicked();
		void on_toolButtonMute_toggled(bool);
		void on_comboBoxPreferredFormat_activated(int);
		void on_toolBox_currentChanged(int);
		void on_listWidgetAttachedVideos_itemActivated(QListWidgetItem *);
		void on_listWidgetAttachedVideos_itemSelectionChanged();
		void on_listWidgetSearchResults_itemActivated(QListWidgetItem *);
		void on_listWidgetAttachedVideos_customContextMenuRequested(const QPoint &);
		void on_listWidgetSearchResults_customContextMenuRequested(const QPoint &);
		void on_lineEditSearchString_textChanged(const QString &);
		void on_toolButtonSuggest_clicked();
		void on_toolButtonSearch_clicked();
		void on_volumeSlider_valueChanged(int);
		void on_seekSlider_valueChanged(int);

		void playAttachedVideo();
		void playSearchedVideo();
		void attachSearchedVideo();
		void attachCurrentVideo();
		void copyYouTubeUrl();
		void copyYouTubeUrlAlt();
		void pasteYouTubeUrl();
		void playerPasteYouTubeUrl();
		void playerLocalMovieFile();
		void copySearchYouTubeUrl();
		void copySearchYouTubeUrlAlt();
		void copyAuthorUrl();
		void copySearchAuthorUrl();
		void copyCurrentYouTubeUrl();
		void copyCurrentYouTubeUrlAlt();
		void copyCurrentAuthorUrl();
		void removeSelectedVideos();
		void setSuggestorAppendString();
		void updateAttachedVideoInfoImages();
		void imageDownloadFinished(QNetworkReply *);
		void attachVideo(QString, QString, QString, int itemType = VIDEOITEM_TYPE_YOUTUBE);
		void attachVideoById(QString);
		void switchToFullScreen();
		void switchToWindowed();

		void videoPlayer_customContextMenuRequested(const QPoint &);
		void videoPlayer_durationChanged(qint64);

	protected:
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);

	private:
#if QT_VERSION < 0x050000
		Phonon::VideoPlayer *mVideoPlayer;
#else
		QMediaPlayer *mVideoPlayer;
		QVideoWidget *mVideoWidget;
		QVideoWidget *mFullscreenVideoWidget;
#endif
};

class YouTubeXmlHandler : public QXmlDefaultHandler
{
	public:
		QListWidget *listWidget;
		QString currentText;
		QString id, title, author;
		bool isEntry;

		YouTubeXmlHandler(QListWidget *, YouTubeVideoPlayer *);

		bool startElement(const QString &, const QString &, const QString &, const QXmlAttributes &);
		bool endElement(const QString &, const QString &, const QString &);
		bool characters(const QString &);
		bool fatalError(const QXmlParseException &);

	private:
		YouTubeVideoPlayer *mVideoPlayer;
};

#endif

#endif
