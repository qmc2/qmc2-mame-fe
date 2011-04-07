#if defined(QMC2_YOUTUBE_ENABLED)

#ifndef _YOUTUBEVIDEOPLAYER_H_
#define _YOUTUBEVIDEOPLAYER_H_

#include <QtNetwork>
#include "ui_youtubevideoplayer.h"

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

// tool-box pages
#define YOUTUBE_ATTACHED_VIDEOS_PAGE	0
#define YOUTUBE_VIDEO_PLAYER_PAGE	1
#define YOUTUBE_SEARCH_VIDEO_PAGE	2

// Play-O-Matic modes
#define YOUTUBE_PLAYOMATIC_SEQUENTIAL	0
#define YOUTUBE_PLAYOMATIC_RANDOM	1

class YouTubeVideoPlayer : public QWidget, public Ui::YouTubeVideoPlayer
{
	Q_OBJECT

	public:
		YouTubeVideoPlayer(QWidget *parent = 0);
		~YouTubeVideoPlayer();

		QString currentVideoID;
		QStringList youTubeFormats;
		QNetworkReply *videoInfoReply;
		QNetworkAccessManager *videoInfoManager;
		QNetworkRequest videoInfoRequest;
		QString videoInfoBuffer;
		bool viFinished;
		bool viError;
		QSlider *privateSeekSlider;
		QToolButton *privateMuteButton;

		QUrl getVideoStreamUrl(QString);
		QString indexToFormat(int);

	public slots:
		void init();
		void adjustIconSizes();

		void playVideo(QString &);
		void loadVideo(QString &);
		void videoTick(qint64);
		void videoFinished();
		void videoStateChanged(Phonon::State, Phonon::State);

		void videoInfoReadyRead();
		void videoInfoError(QNetworkReply::NetworkError);
		void videoInfoFinished();

		void on_toolButtonPlayPause_clicked();
		void on_comboBoxPreferredFormat_currentIndexChanged(int);
		void on_toolBox_currentChanged(int);

	protected:
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);
};

#endif

#endif
