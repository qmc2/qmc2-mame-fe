#if defined(QMC2_YOUTUBE_ENABLED)

#ifndef _YOUTUBEVIDEOPLAYER_H_
#define _YOUTUBEVIDEOPLAYER_H_

#include <QtNetwork>
#include "ui_youtubevideoplayer.h"
#include "videoitemwidget.h"

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

// timeout and reply polling (wait) time for video info requests in ms
#define YOUTUBE_VIDEOINFO_TIMEOUT	10000
#define YOUTUBE_VIDEOINFO_WAIT		10

class YouTubeVideoPlayer : public QWidget, public Ui::YouTubeVideoPlayer
{
	Q_OBJECT

	public:
		YouTubeVideoPlayer(QString, QString, QWidget *parent = 0);
		~YouTubeVideoPlayer();

		QString currentVideoID;
		QString currentAuthor;
		QString mySetID;
		QString mySetName;
		QString suggestorAppendString;
		QStringList youTubeFormats;
		QStringList youTubeFormatNames;
		QNetworkReply *videoInfoReply, *videoImageReply;
		QNetworkAccessManager *videoInfoManager, *videoImageManager, *imageDownloadManager;
		QNetworkRequest videoInfoRequest, videoImageRequest;
		QString videoInfoBuffer, videoImageBuffer;
		QAction *videoMenuPlayPauseAction;
		QAction *autoSuggestAction;
		QStringList playedVideos;
		QMap<QString, VideoItemWidget *> viwMap;
		bool viFinished, vimgFinished;
		bool viError, vimgError;
		bool loadOnly;
		bool isMuted;
		bool pausedByHideEvent;
		bool forcedExit;
		QSlider *privateSeekSlider;
		QToolButton *privateMuteButton;
		QList<int> availableFormats;
		int bestAvailableFormat;
		int currentFormat;
		QMenu *menuAttachedVideos;
		QMenu *menuSearchResults;
		QMenu *menuVideoPlayer;
		QMenu *menuSuggestButton;

		QUrl getVideoStreamUrl(QString, QStringList *videoInfoStringList = NULL, bool videoInfoOnly = false);
		QString indexToFormat(int);

	public slots:
		void init();
		void adjustIconSizes();
		void saveSettings();

		void playVideo(QString &);
		void loadVideo(QString &);
		void loadNullVideo();
		void playNextVideo();
		void videoTick(qint64);
		void videoFinished();
		void videoStateChanged(Phonon::State, Phonon::State);
		void videoBufferStatus(int);

		void videoInfoReadyRead();
		void videoInfoError(QNetworkReply::NetworkError);
		void videoInfoFinished();
		void videoImageReadyRead();
		void videoImageError(QNetworkReply::NetworkError);
		void videoImageFinished();

		void on_toolButtonPlayPause_clicked();
		void on_comboBoxPreferredFormat_activated(int);
		void on_toolBox_currentChanged(int);
		void on_listWidgetAttachedVideos_itemActivated(QListWidgetItem *);
		void on_listWidgetAttachedVideos_customContextMenuRequested(const QPoint &);
		void on_listWidgetSearchResults_customContextMenuRequested(const QPoint &);
		void on_videoPlayer_customContextMenuRequested(const QPoint &);
		void on_lineEditSearchString_textChanged(const QString &);
		void on_toolButtonSuggest_clicked();
		void on_toolButtonSearch_clicked();

		void playAttachedVideo();
		void copyYouTubeUrl();
		void copyAlternateYouTubeUrl();
		void copyAuthorUrl();
		void copyCurrentYouTubeUrl();
		void copyCurrentAlternateYouTubeUrl();
		void copyCurrentAuthorUrl();
		void removeSelectedVideos();
		void setSuggestorAppendString();
		void updateAttachedVideoInfoImages();
		void imageDownloadFinished(QNetworkReply *);
		void attachVideo(QString, QString, QString);

	protected:
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);
};

#endif

#endif
