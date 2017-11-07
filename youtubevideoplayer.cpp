#if defined(QMC2_YOUTUBE_ENABLED)

#include <QtTest>
#include <QClipboard>
#include <QInputDialog>
#include <QImageReader>
#include <QFileDialog>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QMap>
#include <QHash>
#include <QCache>
#include <QApplication>

#include "settings.h"
#include "macros.h"
#include "qmc2main.h"
#include "options.h"
#include "machinelist.h"
#include "youtubevideoplayer.h"
#include "videoitemwidget.h"

extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;
extern Options *qmc2Options;
extern QHash <QString, YouTubeVideoInfo> qmc2YouTubeVideoInfoHash;
extern QHash<QString, QString> qmc2CustomShortcutHash;
extern QHash<QString, QTreeWidgetItem *> qmc2MachineListItemHash;
extern QHash<QString, QString> qmc2ParentHash;
extern bool qmc2YouTubeVideoInfoHashChanged;
extern bool qmc2ParentImageFallback;
extern QCache<QString, ImagePixmap> qmc2ImagePixmapCache;

YouTubeVideoPlayer::YouTubeVideoPlayer(QString sID, QString sName, QWidget *parent) :
	QWidget(parent)
{
	setupUi(this);

	mySetID = sID;
	mySetName = sName;

#if QT_VERSION < 0x050000
	mVideoPlayer = new Phonon::VideoPlayer(this);
	videoPlayer()->setContextMenuPolicy(Qt::CustomContextMenu);
	videoWidget()->setPalette(Qt::black);
	videoWidget()->setAutoFillBackground(true);
	videoWidget()->setScaleMode(Phonon::VideoWidget::FitInView);
	videoWidget()->setAspectRatio(Phonon::VideoWidget::AspectRatioAuto);
	gridLayoutVideoPlayer->removeWidget(widgetVideoPlayerPlaceholder);
	delete widgetVideoPlayerPlaceholder;
	gridLayoutVideoPlayer->addWidget(videoPlayer(), 0, 0, 1, gridLayoutVideoPlayer->columnCount());
	connect(mediaObject(), SIGNAL(seekableChanged(bool)), seekSlider, SLOT(setEnabled(bool)));
	connect(mediaObject(), SIGNAL(totalTimeChanged(qint64)), this, SLOT(videoPlayer_durationChanged(qint64)));
	connect(mediaObject(), SIGNAL(tick(qint64)), this, SLOT(videoTick(qint64)));
	connect(mediaObject(), SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(videoStateChanged(Phonon::State, Phonon::State)));
	connect(mediaObject(), SIGNAL(bufferStatus(int)), this, SLOT(videoBufferStatus(int)));
	connect(videoPlayer(), SIGNAL(finished()), this, SLOT(videoFinished()));
#else
	mFullscreenVideoWidget = 0;
	mVideoWidget = new QVideoWidget(this);
	mVideoPlayer = new QMediaPlayer(this, (QMediaPlayer::Flags)(QMediaPlayer::VideoSurface | QMediaPlayer::StreamPlayback));
	videoWidget()->setAspectRatioMode(Qt::KeepAspectRatio);
	videoWidget()->setContextMenuPolicy(Qt::CustomContextMenu);
	videoWidget()->setPalette(Qt::black);
	videoWidget()->setAutoFillBackground(true);
	videoPlayer()->setVideoOutput(videoWidget());
	gridLayoutVideoPlayer->removeWidget(widgetVideoPlayerPlaceholder);
	delete widgetVideoPlayerPlaceholder;
	gridLayoutVideoPlayer->addWidget(videoWidget(), 0, 0, 1, gridLayoutVideoPlayer->columnCount());
	connect(mediaObject(), SIGNAL(seekableChanged(bool)), seekSlider, SLOT(setEnabled(bool)));
	connect(mediaObject(), SIGNAL(durationChanged(qint64)), this, SLOT(videoPlayer_durationChanged(qint64)));
	connect(mediaObject(), SIGNAL(positionChanged(qint64)), this, SLOT(videoTick(qint64)));
	connect(mediaObject(), SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(videoStateChanged(QMediaPlayer::State)));
	connect(mediaObject(), SIGNAL(bufferStatusChanged(int)), this, SLOT(videoBufferStatus(int)));
#endif

	videoEventFilter = new VideoEventFilter(this, 0);
	videoWidget()->installEventFilter(videoEventFilter);

	videoOverlayWidget = new VideoOverlayWidget(videoWidget());

	toolButtonPlayPause->setEnabled(false);
	toolButtonSearch->setEnabled(false);
	comboBoxPreferredFormat->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/PreferredFormat", YOUTUBE_FORMAT_MP4_1080P_INDEX).toInt());
	toolBox->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/PageIndex", YOUTUBE_VIDEO_PLAYER_PAGE).toInt());
	checkBoxPlayOMatic->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/PlayOMatic/Enabled", false).toBool());
	comboBoxMode->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/PlayOMatic/Mode", YOUTUBE_PLAYOMATIC_SEQUENTIAL).toInt());
	checkBoxRepeat->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/PlayOMatic/Repeat", true).toBool());
	suggestorAppendString = qmc2Config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/SuggestorAppendString", "Arcade").toString();
	spinBoxResultsPerRequest->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/SearchResultsPerRequest", 10).toInt());

	toolButtonMute->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/AudioMuted", false).toBool());
	toolButtonMute->setToolTip(tr("Mute / unmute audio output"));
	volumeSlider->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/AudioVolume", 50).toInt());

	currentVideoID.clear();
	currentVideoAuthor.clear();
	currentVideoTitle.clear();

	youTubeFormats 
		<< YOUTUBE_FORMAT_FLV_240P
		<< YOUTUBE_FORMAT_FLV_360P
		<< YOUTUBE_FORMAT_MP4_360P
		<< YOUTUBE_FORMAT_FLV_480P
		<< YOUTUBE_FORMAT_MP4_720P
		<< YOUTUBE_FORMAT_MP4_1080P
		<< YOUTUBE_FORMAT_MP4_3072P;

	youTubeFormatNames
		<< tr("FLV 240P")
		<< tr("FLV 360P")
		<< tr("MP4 360P")
		<< tr("FLV 480P")
		<< tr("MP4 720P")
		<< tr("MP4 1080P")
		<< tr("MP4 3072P");

	forcedExit = loadOnly = isMuted = pausedByHideEvent = viError = viFinished = vimgError = vimgFinished = fullyLoaded = false;
	videoInfoReply = videoImageReply = searchRequestReply = 0;
	videoInfoManager = videoImageManager = searchRequestManager = 0;
	currentFormat = bestAvailableFormat = YOUTUBE_FORMAT_UNKNOWN_INDEX;
	videoSeqNum = 0;

#if QT_VERSION < 0x050000
	mediaObject()->setTickInterval(1000);
#else
	videoPlayer()->setNotifyInterval(1000);
#endif

	volumeSlider->setToolTip(tr("Volume level"));
	seekSlider->setToolTip(tr("Video progress"));
	labelSeekSlider->setText(tr("Remaining") + " --:--:--");

	progressBarBufferStatus->setValue(0);
	progressBarBufferStatus->setToolTip(tr("Current buffer fill level: %1%").arg(0));

	QString s;
	QAction *action;

	// context menus
	menuAttachedVideos = new QMenu(0);
	s = tr("Play this video");
	action = menuAttachedVideos->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/media_play.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(playAttachedVideo()));
	avmActionPlayVideo = action;
	menuAttachedVideos->addSeparator();
	s = tr("Copy video URL");
	action = menuAttachedVideos->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/youtube.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(copyYouTubeUrl()));
	avmActionCopyVideoUrl = action;
	s = tr("Copy video URL (no country filter)");
	action = menuAttachedVideos->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/youtube.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(copyYouTubeUrlAlt()));
	avmActionCopyVideoUrlAlt = action;
	s = tr("Copy author URL");
	action = menuAttachedVideos->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/youtube.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(copyAuthorUrl()));
	avmActionCopyAuthorUrl = action;
	// FIXME: begin
	avmActionCopyAuthorUrl->setVisible(false);
	// FIXME: end
	s = tr("Paste video URL");
	action = menuAttachedVideos->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/youtube.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(pasteYouTubeUrl()));
	avmActionPasteVideoUrl = action;
	menuAttachedVideos->addSeparator();
	s = tr("Local movie file...");
	action = menuAttachedVideos->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/fileopen.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(attachMovieFile()));
	menuAttachedVideos->addSeparator();
	s = tr("Remove selected videos");
	action = menuAttachedVideos->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/remove.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(removeSelectedVideos()));
	avmActionRemoveVideos = action;

	menuVideoPlayer = new QMenu(0);
	s = tr("Start / pause / resume video playback");
	action = menuVideoPlayer->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/media_stop.png")));
	videoMenuPlayPauseAction = action;
	videoMenuPlayPauseAction->setEnabled(false);
	connect(action, SIGNAL(triggered()), this, SLOT(on_toolButtonPlayPause_clicked()));
	menuVideoPlayer->addSeparator();
	s = tr("Full screen (return with toggle-key)");
	action = menuVideoPlayer->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/toggle_fullscreen.png")));
	videoMenuFullscreenAction = action;
	connect(action, SIGNAL(triggered()), this, SLOT(switchToFullScreen()));
	menuVideoPlayer->addSeparator();
	s = tr("Copy video URL");
	action = menuVideoPlayer->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/youtube.png")));
	videoMenuCopyVideoUrlAction = action;
	connect(action, SIGNAL(triggered()), this, SLOT(copyCurrentYouTubeUrl()));
	s = tr("Copy video URL (no country filter)");
	action = menuVideoPlayer->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/youtube.png")));
	videoMenuCopyVideoUrlAltAction = action;
	connect(action, SIGNAL(triggered()), this, SLOT(copyCurrentYouTubeUrlAlt()));
	s = tr("Copy author URL");
	action = menuVideoPlayer->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/youtube.png")));
	videoMenuCopyAuthorUrlAction = action;
	connect(action, SIGNAL(triggered()), this, SLOT(copyCurrentAuthorUrl()));
	// FIXME: begin
	videoMenuCopyAuthorUrlAction->setVisible(false);
	// FIXME: end
	s = tr("Paste video URL");
	action = menuVideoPlayer->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/youtube.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(playerPasteYouTubeUrl()));
	videoMenuPasteVideoUrlAction = action;
	menuVideoPlayer->addSeparator();
	s = tr("Local movie file...");
	action = menuVideoPlayer->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/fileopen.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(playerLocalMovieFile()));
	menuVideoPlayer->addSeparator();
	s = tr("Attach this video");
	action = menuVideoPlayer->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/movie.png")));
	videoMenuAttachVideoAction = action;
	connect(action, SIGNAL(triggered()), this, SLOT(attachCurrentVideo()));

	menuSearchResults = new QMenu(0);
	s = tr("Play this video");
	action = menuSearchResults->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/media_play.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(playSearchedVideo()));
	s = tr("Attach this video");
	action = menuSearchResults->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/movie.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(attachSearchedVideo()));
	menuSearchResults->addSeparator();
	s = tr("Copy video URL");
	action = menuSearchResults->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/youtube.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(copySearchYouTubeUrl()));
	s = tr("Copy video URL (no country filter)");
	action = menuSearchResults->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/youtube.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(copySearchYouTubeUrlAlt()));
	s = tr("Copy author URL");
	action = menuSearchResults->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/youtube.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(copySearchAuthorUrl()));

	menuSuggestButton = new QMenu(0);
	s = tr("Auto-suggest a search pattern?");
	action = menuSuggestButton->addAction(tr("Auto-suggest"));
	action->setToolTip(s); action->setStatusTip(s);
	action->setCheckable(true);
	action->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/AutoSuggest", true).toBool());
	autoSuggestAction = action;
	s = tr("Enter string to be appended");
	action = menuSuggestButton->addAction(tr("Append..."));
	action->setToolTip(s); action->setStatusTip(s);
	connect(action, SIGNAL(triggered()), this, SLOT(setSuggestorAppendString()));
	toolButtonSuggest->setMenu(menuSuggestButton);

	imageDownloadManager = new QNetworkAccessManager(this);
	connect(imageDownloadManager, SIGNAL(finished(QNetworkReply *)), this, SLOT(imageDownloadFinished(QNetworkReply *)));

	lineEditSearchString->setPlaceholderText(tr("Enter search string"));

	// FIXME: begin
	if ( toolBox->currentIndex() == YOUTUBE_SEARCH_VIDEO_PAGE )
		 toolBox->setCurrentIndex(YOUTUBE_VIDEO_PLAYER_PAGE);
	toolBox->removeItem(YOUTUBE_SEARCH_VIDEO_PAGE);
	delete pageSearchVideos;
	//if ( autoSuggestAction->isChecked() )
	//	QTimer::singleShot(0, this, SLOT(on_toolButtonSuggest_clicked()));
	// FIXME: end

	QTimer::singleShot(0, this, SLOT(adjustIconSizes()));
	QTimer::singleShot(250, this, SLOT(init()));
}

YouTubeVideoPlayer::~YouTubeVideoPlayer()
{
	// clean up
	if ( videoOverlayWidget ) {
		disconnect(videoOverlayWidget);
		delete videoOverlayWidget;
	}
	if ( videoPlayer() ) {
		disconnect(videoPlayer());
		delete videoPlayer();
	}
#if QT_VERSION >= 0x050000
	if ( mVideoWidget ) {
		disconnect(mVideoWidget);
		delete mVideoWidget;
	}
	if ( mFullscreenVideoWidget ) {
		disconnect(mFullscreenVideoWidget);
		delete mFullscreenVideoWidget;
	}
#endif
	if ( videoEventFilter )
		delete videoEventFilter;
	if ( menuAttachedVideos ) {
		disconnect(menuAttachedVideos);
		delete menuAttachedVideos;
	}
	if ( menuSearchResults ) {
		disconnect(menuSearchResults);
		delete menuSearchResults;
	}
	if ( menuVideoPlayer ) {
		disconnect(menuVideoPlayer);
		delete menuVideoPlayer;
	}
	if ( menuSuggestButton ) {
		disconnect(menuSuggestButton);
		delete menuSuggestButton;
	}
	if ( videoInfoReply ) {
		disconnect(videoInfoReply);
		delete videoInfoReply;
	}
	if ( videoInfoManager ) {
		disconnect(videoInfoManager);
		delete videoInfoManager;
	}
	if ( videoImageReply ) {
		disconnect(videoImageReply);
		delete videoImageReply;
	}
	if ( videoImageManager ) {
		disconnect(videoImageManager);
		delete videoImageManager;
	}
	if ( searchRequestReply ) {
		disconnect(searchRequestReply);
		delete searchRequestReply;
	}
	if ( searchRequestManager ) {
		disconnect(searchRequestManager);
		delete searchRequestManager;
	}
	if ( imageDownloadManager ) {
		disconnect(imageDownloadManager);
		delete imageDownloadManager;
	}
}

void YouTubeVideoPlayer::saveSettings()
{
  	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "YouTubeWidget/PreferredFormat", comboBoxPreferredFormat->currentIndex());
#if QT_VERSION < 0x050000
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "YouTubeWidget/AudioVolume", int(audioOutput()->volume() * 100));
#else
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "YouTubeWidget/AudioVolume", audioOutput()->volume());
#endif
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "YouTubeWidget/PageIndex", toolBox->currentIndex());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "YouTubeWidget/PlayOMatic/Enabled", checkBoxPlayOMatic->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "YouTubeWidget/PlayOMatic/Mode", comboBoxMode->currentIndex());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "YouTubeWidget/PlayOMatic/Repeat", checkBoxRepeat->isChecked());
	// FIXME: begin
	//qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "YouTubeWidget/AutoSuggest", autoSuggestAction->isChecked());
	//qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "YouTubeWidget/SuggestorAppendString", suggestorAppendString);
	//qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "YouTubeWidget/SearchResultsPerRequest", spinBoxResultsPerRequest->value());
	// FIXME: end

	if ( fullyLoaded ) {
		QStringList attachedVideos;
		for (int i = 0; i < listWidgetAttachedVideos->count(); i++) {
			VideoItemWidget *viw = (VideoItemWidget *)listWidgetAttachedVideos->itemWidget(listWidgetAttachedVideos->item(i));
			if ( viw->itemType != VIDEOITEM_TYPE_VIDEO_SNAP )
				attachedVideos << viw->videoID;
		}
		if ( attachedVideos.isEmpty() )
			qmc2Config->remove(QString(QMC2_FRONTEND_PREFIX + "YouTubeVideos/%1").arg(mySetID));
		else
			qmc2Config->setValue(QString(QMC2_FRONTEND_PREFIX + "YouTubeVideos/%1").arg(mySetID), attachedVideos);
	}

	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "YouTubeWidget/AudioMuted", toolButtonMute->isChecked());
}

void YouTubeVideoPlayer::setSuggestorAppendString()
{
	bool ok;
	QString appendString = QInputDialog::getText(this,
						tr("Appended string"),
						tr("Enter the string to be appended when suggesting a pattern:") +
						"\n(" + tr("Valid placeholder macros:") + " $MANUFACTURER$, $YEAR$)",
						QLineEdit::Normal,
						suggestorAppendString,
						&ok);
	if ( ok )
		suggestorAppendString = appendString;
}

void YouTubeVideoPlayer::loadNullVideo()
{
	if ( forcedExit )
		return;

	currentVideoID.clear();
	currentVideoAuthor.clear();
	currentVideoTitle.clear();

	if ( isPlaying() || isPaused() )
		stop();
}

void YouTubeVideoPlayer::playNextVideo()
{
	toolBox->setCurrentIndex(YOUTUBE_VIDEO_PLAYER_PAGE);
	QList<QListWidgetItem *> il = listWidgetAttachedVideos->findItems("*", Qt::MatchWildcard);
	if ( il.count() > 0 ) {
		switch ( comboBoxMode->currentIndex() ) {
			case YOUTUBE_PLAYOMATIC_RANDOM: {
					int index = qrand() % il.count();
					VideoItemWidget *viw = (VideoItemWidget *)listWidgetAttachedVideos->itemWidget(il[index]);
					bool localFile = (viw->itemType == VIDEOITEM_TYPE_LOCAL_MOVIE || viw->itemType == VIDEOITEM_TYPE_VIDEO_SNAP);
					QString vidCopy = viw->videoID;
					if ( localFile )
						vidCopy.remove(QRegExp("^\\#\\:"));
					if ( checkBoxRepeat->isChecked() ) {
						if ( localFile )
							playMovieFile(vidCopy);
						else
							playVideo(viw->videoID);
					} else if ( !playedVideos.contains(viw->videoID) ) {
						if ( localFile )
							playMovieFile(vidCopy);
						else
							playVideo(viw->videoID);
					} else if ( playedVideos.count() < il.count() ) {
						do {
							index = qrand() % il.count();
							viw = (VideoItemWidget *)listWidgetAttachedVideos->itemWidget(il[index]);
						} while ( playedVideos.contains(viw->videoID) );
						if ( localFile )
							playMovieFile(vidCopy);
						else
							playVideo(viw->videoID);
					} else
						stop();
				}
				break;
			case YOUTUBE_PLAYOMATIC_SEQUENTIAL:
			default: {
					 if ( videoSeqNum > il.count() - 1 || videoSeqNum < 0 )
						 videoSeqNum = 0;
					 VideoItemWidget *viw = (VideoItemWidget *)listWidgetAttachedVideos->itemWidget(il[videoSeqNum]);
					 bool localFile = (viw->itemType == VIDEOITEM_TYPE_LOCAL_MOVIE || viw->itemType == VIDEOITEM_TYPE_VIDEO_SNAP);
					 QString vidCopy = viw->videoID;
					 if ( localFile )
						 vidCopy.remove(QRegExp("^\\#\\:"));
					 if ( checkBoxRepeat->isChecked() ) {
						 if ( localFile )
							 playMovieFile(vidCopy);
						 else
							 playVideo(viw->videoID);
					 } else if ( !playedVideos.contains(viw->videoID) ) {
						 if ( localFile )
							 playMovieFile(vidCopy);
						 else
							 playVideo(viw->videoID);
					 } else
						 stop();
					 videoSeqNum++;
				}
				break;
		}	
	}
}

void YouTubeVideoPlayer::playAttachedVideo()
{
	QListWidgetItem *item = listWidgetAttachedVideos->currentItem();
	if ( item ) {
		on_listWidgetAttachedVideos_itemActivated(item);
		videoSeqNum = listWidgetAttachedVideos->row(item);
	}
}

void YouTubeVideoPlayer::playSearchedVideo()
{
	QListWidgetItem *item = listWidgetSearchResults->currentItem();
	if ( item )
		on_listWidgetSearchResults_itemActivated(item);
}

void YouTubeVideoPlayer::copyYouTubeUrl()
{
	QListWidgetItem *item = listWidgetAttachedVideos->currentItem();
	if ( item ) {
		VideoItemWidget *viw = (VideoItemWidget *)listWidgetAttachedVideos->itemWidget(item);
		if ( viw ) {
			if ( !viw->videoID.isEmpty() ) {
				if ( !viw->videoUrlPattern.isEmpty() ) {
					QString url = VIDEOITEM_YOUTUBE_URL_PATTERN;
					url.replace("$VIDEO_ID$", viw->videoID);
					qApp->clipboard()->setText(url);
				}
			}
		}
	}
}

void YouTubeVideoPlayer::copyYouTubeUrlAlt()
{
	QListWidgetItem *item = listWidgetAttachedVideos->currentItem();
	if ( item ) {
		VideoItemWidget *viw = (VideoItemWidget *)listWidgetAttachedVideos->itemWidget(item);
		if ( viw ) {
			if ( !viw->videoID.isEmpty() ) {
				if ( !viw->videoUrlPattern.isEmpty() ) {
					QString url = VIDEOITEM_YOUTUBE_URL_PATTERN_NO_COUNTRY_FILTER;
					url.replace("$VIDEO_ID$", viw->videoID);
					qApp->clipboard()->setText(url);
				}
			}
		}
	}
}

void YouTubeVideoPlayer::pasteYouTubeUrl()
{
	QString clipboardText(qApp->clipboard()->text()), videoID;
	clipboardText.replace("https:", "http:");
	if ( clipboardText.indexOf(QRegExp("^http\\:\\/\\/.*youtube\\.com\\/watch\\?.*v\\=.*$")) == 0 )
		videoID = clipboardText.replace(QRegExp("^http\\:\\/\\/.*youtube\\.com\\/watch\\?.*v\\=(.*)$"), "\\1").replace(QRegExp("\\&.*$"), "");
	else if ( clipboardText.indexOf(QRegExp("^http\\:\\/\\/.*youtu\\.be\\/.*$")) == 0 )
		videoID = clipboardText.replace(QRegExp("^http\\:\\/\\/.*youtu\\.be\\/(.*)$"), "\\1").replace(QRegExp("\\&.*$"), "");
	if ( videoID.isEmpty() )
		return;
	QStringList videoInfoList;
	getVideoStreamUrl(videoID, &videoInfoList, true);
	if ( videoInfoList.count() > 2 ) {
		attachVideo(videoID, videoInfoList[0], videoInfoList[1]);
		QTimer::singleShot(100, this, SLOT(updateAttachedVideoInfoImages()));
	}
}

void YouTubeVideoPlayer::playerPasteYouTubeUrl()
{
	QString clipboardText(qApp->clipboard()->text()), videoID;
	clipboardText.replace("https:", "http:");
	if ( clipboardText.indexOf(QRegExp("^http\\:\\/\\/.*youtube\\.com\\/watch\\?.*v\\=.*$")) == 0 )
		videoID = clipboardText.replace(QRegExp("^http\\:\\/\\/.*youtube\\.com\\/watch\\?.*v\\=(.*)$"), "\\1").replace(QRegExp("\\&.*$"), "");
	else if ( clipboardText.indexOf(QRegExp("^http\\:\\/\\/.*youtu\\.be\\/.*$")) == 0 )
		videoID = clipboardText.replace(QRegExp("^http\\:\\/\\/.*youtu\\.be\\/(.*)$"), "\\1").replace(QRegExp("\\&.*$"), "");
	if ( videoID.isEmpty() )
		return;
	playVideo(videoID);
}

void YouTubeVideoPlayer::playerLocalMovieFile()
{
	QString filter = tr("All files (*)");
	QString path = QFileDialog::getOpenFileName(this, tr("Choose movie file"), QString(), filter, 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !path.isNull() )
		playMovieFile(path);
}

void YouTubeVideoPlayer::attachMovieFile()
{
	QString filter = tr("All files (*)");
	QString path = QFileDialog::getOpenFileName(this, tr("Choose movie file"), QString(), filter, 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);
	if ( !path.isNull() )
		attachVideo("#:" + path, QString(), QString());
}

void YouTubeVideoPlayer::playMovieFile(QString &filePath)
{
	QFileInfo fi(filePath);
	if ( fi.exists() ) {
		currentVideoID = "#:" + filePath;
		loadOnly = false;
#if QT_VERSION < 0x050000
		videoPlayer()->play(Phonon::MediaSource(filePath));
#else
		videoPlayer()->setMedia(QUrl::fromLocalFile(filePath));
		play();
#endif
		comboBoxPreferredFormat->setEnabled(false);
		if ( !playedVideos.contains(currentVideoID) )
			playedVideos << currentVideoID;
	}
}

void YouTubeVideoPlayer::copyAuthorUrl()
{
	QListWidgetItem *item = listWidgetAttachedVideos->currentItem();
	if ( item ) {
		VideoItemWidget *viw = (VideoItemWidget *)listWidgetAttachedVideos->itemWidget(item);
		if ( viw ) {
			if ( !viw->videoAuthor.isEmpty() ) {
				if ( !viw->authorUrlPattern.isEmpty() ) {
					QString url = VIDEOITEM_YOUTUBE_AUTHOR_URL_PATTERN;
					url.replace("$USER_ID$", viw->videoAuthor);
					qApp->clipboard()->setText(url);
				}
			}
		}
	}
}

void YouTubeVideoPlayer::copySearchYouTubeUrl()
{
	QListWidgetItem *item = listWidgetSearchResults->currentItem();
	if ( item ) {
		VideoItemWidget *viw = (VideoItemWidget *)listWidgetSearchResults->itemWidget(item);
		if ( viw ) {
			if ( !viw->videoID.isEmpty() ) {
				if ( !viw->videoUrlPattern.isEmpty() ) {
					QString url = VIDEOITEM_YOUTUBE_URL_PATTERN;
					url.replace("$VIDEO_ID$", viw->videoID);
					qApp->clipboard()->setText(url);
				}
			}
		}
	}
}

void YouTubeVideoPlayer::copySearchYouTubeUrlAlt()
{
	QListWidgetItem *item = listWidgetSearchResults->currentItem();
	if ( item ) {
		VideoItemWidget *viw = (VideoItemWidget *)listWidgetSearchResults->itemWidget(item);
		if ( viw ) {
			if ( !viw->videoID.isEmpty() ) {
				if ( !viw->videoUrlPattern.isEmpty() ) {
					QString url = VIDEOITEM_YOUTUBE_URL_PATTERN_NO_COUNTRY_FILTER;
					url.replace("$VIDEO_ID$", viw->videoID);
					qApp->clipboard()->setText(url);
				}
			}
		}
	}
}

void YouTubeVideoPlayer::copySearchAuthorUrl()
{
	QListWidgetItem *item = listWidgetSearchResults->currentItem();
	if ( item ) {
		VideoItemWidget *viw = (VideoItemWidget *)listWidgetSearchResults->itemWidget(item);
		if ( viw ) {
			if ( !viw->videoAuthor.isEmpty() ) {
				if ( !viw->authorUrlPattern.isEmpty() ) {
					QString url = VIDEOITEM_YOUTUBE_AUTHOR_URL_PATTERN;
					url.replace("$USER_ID$", viw->videoAuthor);
					qApp->clipboard()->setText(url);
				}
			}
		}
	}
}

void YouTubeVideoPlayer::copyCurrentYouTubeUrl()
{
	if ( !currentVideoID.isEmpty() ) {
		QString url = VIDEOITEM_YOUTUBE_URL_PATTERN;
		url.replace("$VIDEO_ID$", currentVideoID);
		qApp->clipboard()->setText(url);
	}
}

void YouTubeVideoPlayer::copyCurrentYouTubeUrlAlt()
{
	if ( !currentVideoID.isEmpty() ) {
		QString url = VIDEOITEM_YOUTUBE_URL_PATTERN_NO_COUNTRY_FILTER;
		url.replace("$VIDEO_ID$", currentVideoID);
		qApp->clipboard()->setText(url);
	}
}

void YouTubeVideoPlayer::copyCurrentAuthorUrl()
{
	if ( !currentVideoAuthor.isEmpty() ) {
		QString url = VIDEOITEM_YOUTUBE_AUTHOR_URL_PATTERN;
		url.replace("$USER_ID$", currentVideoAuthor);
		qApp->clipboard()->setText(url);
	}
}

void YouTubeVideoPlayer::removeSelectedVideos()
{
	QList<QListWidgetItem *> il = listWidgetAttachedVideos->selectedItems();
	foreach (QListWidgetItem *item, il) {
		VideoItemWidget *viw = (VideoItemWidget *)listWidgetAttachedVideos->itemWidget(item);
		if ( viw->itemType != VIDEOITEM_TYPE_VIDEO_SNAP ) {
			viwMap.remove(viw->videoID);
			QListWidgetItem *i = listWidgetAttachedVideos->takeItem(listWidgetAttachedVideos->row(item));
			delete i;
		}
	}
}

void YouTubeVideoPlayer::switchToFullScreen()
{
	if ( videoWidget()->isFullScreen() ) {
#if QT_VERSION < 0x050000
		videoWidget()->setFullScreen(false);
#else
		switchToWindowed();
#endif
		menuVideoPlayer->hide();
		return;
	}
#if QT_VERSION < 0x050000
	if ( !videoWidget()->isFullScreen() ) {
		videoWidget()->setFullScreen(true);
		QString keySeq = qmc2CustomShortcutHash["F11"];
		if ( !keySeq.isEmpty() )
			showMessage(tr("Full-screen mode -- press %1 to return to windowed mode").arg(keySeq), 4000);
		else
			showMessage(tr("Full-screen mode -- press toggle-key to return to windowed mode"), 4000);
	}
#else
	if ( !mFullscreenVideoWidget ) {
		mFullscreenVideoWidget = new QVideoWidget(0);
		mFullscreenVideoWidget->setFullScreen(true);
		mFullscreenVideoWidget->setAspectRatioMode(Qt::KeepAspectRatio);
		mFullscreenVideoWidget->setContextMenuPolicy(Qt::CustomContextMenu);
		mFullscreenVideoWidget->setPalette(Qt::black);
		mFullscreenVideoWidget->setAutoFillBackground(true);
		mFullscreenVideoWidget->show();
		qApp->processEvents();
		videoPlayer()->setVideoOutput(videoWidget());
		videoOverlayWidget->setVideoWidget(videoWidget());
		QString keySeq = qmc2CustomShortcutHash["F11"];
		if ( !keySeq.isEmpty() )
			showMessage(tr("Full-screen mode -- press %1 to return to windowed mode").arg(keySeq), 4000);
		else
			showMessage(tr("Full-screen mode -- press toggle-key to return to windowed mode"), 4000);
	}
#endif
}

void YouTubeVideoPlayer::switchToWindowed()
{
#if QT_VERSION >= 0x050000
	if ( mFullscreenVideoWidget ) {
		mFullscreenVideoWidget->hide();
		videoPlayer()->setVideoOutput(mVideoWidget);
		delete mFullscreenVideoWidget;
		mFullscreenVideoWidget = 0;
		videoOverlayWidget->setVideoWidget(videoWidget());
	}
#endif
}

void YouTubeVideoPlayer::attachVideoById(QString id)
{
	QStringList videoInfoList;
	getVideoStreamUrl(id, &videoInfoList, true);
	if ( videoInfoList.count() > 2 )
		attachVideo(id, videoInfoList[0], videoInfoList[1]);
}

void YouTubeVideoPlayer::attachVideo(QString id, QString title, QString author, int itemType)
{
	if ( id.startsWith("#:") ) {
		if ( itemType == VIDEOITEM_TYPE_YOUTUBE )
			itemType = VIDEOITEM_TYPE_LOCAL_MOVIE;
		if ( viwMap.keys().contains(id) ) {
			id.remove(QRegExp("^\\#\\:"));
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("video player: the local movie file '%1' is already attached, ignored").arg(id));
			return;
		}
	} else if ( viwMap.keys().contains(id) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("video player: a video with the ID '%1' is already attached, ignored").arg(id));
		return;
	}

	ImagePixmap *imagePixmap = qmc2ImagePixmapCache.object("yt_" + id);
	bool pixmapFound = (imagePixmap != 0);
	if ( !pixmapFound ) {
		QDir youTubeCacheDir(qmc2Config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/CacheDirectory").toString());
		if ( youTubeCacheDir.exists() ) {
			QString imageFile = id + ".png";
			if ( youTubeCacheDir.exists(imageFile) ) {
				QPixmap pm;
				pixmapFound = pm.load(youTubeCacheDir.filePath(imageFile), "PNG");
				if ( pixmapFound ) {
					imagePixmap = new ImagePixmap(pm);
					imagePixmap->imagePath = youTubeCacheDir.filePath(imageFile);
					qmc2ImagePixmapCache.insert("yt_" + id, imagePixmap, pm.toImage().byteCount());
				}
			}
		}
	}

	QListWidgetItem *listWidgetItem = new QListWidgetItem(listWidgetAttachedVideos);
	listWidgetItem->setSizeHint(QSize(VIDEOITEM_IMAGE_WIDTH, VIDEOITEM_IMAGE_HEIGHT + 4));
	VideoItemWidget *videoItemWidget;
	if ( pixmapFound )
		videoItemWidget = new VideoItemWidget(id, title, author, imagePixmap, itemType, this, listWidgetAttachedVideos);
	else
		videoItemWidget = new VideoItemWidget(id, title, author, 0, itemType, this, listWidgetAttachedVideos);
	listWidgetAttachedVideos->setItemWidget(listWidgetItem, videoItemWidget);
	viwMap[id] = videoItemWidget;
	qmc2YouTubeVideoInfoHash[id] = YouTubeVideoInfo(title, author);
	qmc2YouTubeVideoInfoHashChanged = true;
}

void YouTubeVideoPlayer::attachCurrentVideo()
{
	if ( currentVideoID.startsWith("#:") ) {
		attachVideo(currentVideoID, QString(), QString());
	} else if ( !currentVideoID.isEmpty() && !currentVideoTitle.isEmpty() && !currentVideoAuthor.isEmpty() ) {
		attachVideo(currentVideoID, currentVideoTitle, currentVideoAuthor);
		QTimer::singleShot(100, this, SLOT(updateAttachedVideoInfoImages()));
	}
}

void YouTubeVideoPlayer::attachSearchedVideo()
{
	QListWidgetItem *item = listWidgetSearchResults->currentItem();
	if ( item ) {
		VideoItemWidget *viw = (VideoItemWidget *)listWidgetSearchResults->itemWidget(item);
		if ( viw ) {
			attachVideo(viw->videoID, viw->videoTitle, viw->videoAuthor);
			QTimer::singleShot(100, this, SLOT(updateAttachedVideoInfoImages()));
		}
	}
}

void YouTubeVideoPlayer::init()
{
	if ( forcedExit )
		return;

#if QT_VERSION < 0x050000
	videoWidget()->resize(videoPlayer()->size());
#endif

	int videoSnapCounter = 0;
	foreach (QString videoSnapFolder, qmc2Config->value("MAME/FilesAndDirectories/VideoSnapFolder", QMC2_DEFAULT_DATA_PATH + "/vdo/").toString().split(";", QString::SkipEmptyParts)) {
		foreach (QString formatExtension, qmc2MainWindow->videoSnapAllowedFormatExtensions) {
			QFileInfo fi(QDir::cleanPath(videoSnapFolder + "/" + mySetID + formatExtension));
			if ( fi.exists() && fi.isReadable() ) {
				QString vid = fi.absoluteFilePath();
				vid.prepend("#:");
				attachVideo(vid, QString(), QString(), VIDEOITEM_TYPE_VIDEO_SNAP);
				videoSnapCounter++;
			}
		}
		if ( videoSnapCounter == 0 ) { // parent fallback
			if ( qmc2ParentImageFallback && qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/VideoFallback", 0).toInt() == 0 ) {
				QString parentId = qmc2ParentHash.value(mySetID);
				if ( !parentId.isEmpty() ) {
					foreach (QString formatExtension, qmc2MainWindow->videoSnapAllowedFormatExtensions) {
						QFileInfo fi(QDir::cleanPath(videoSnapFolder + "/" + parentId + formatExtension));
						if ( fi.exists() && fi.isReadable() ) {
							QString vid = fi.absoluteFilePath();
							vid.prepend("#:");
							attachVideo(vid, QString(), QString(), VIDEOITEM_TYPE_VIDEO_SNAP);
							videoSnapCounter++;
						}
					}
				}
			}
		}
	}

	if ( forcedExit )
		return;

	QStringList attachedVideos(qmc2Config->value(QString(QMC2_FRONTEND_PREFIX + "YouTubeVideos/%1").arg(mySetID), QStringList()).toStringList());
	listWidgetAttachedVideos->setUpdatesEnabled(false);
	foreach(QString vid, attachedVideos) {
		if ( vid.startsWith("#:" ) )
			attachVideo(vid, QString(), QString());
		else {
			if ( qmc2YouTubeVideoInfoHash.contains(vid) ) {
				YouTubeVideoInfo vi = qmc2YouTubeVideoInfoHash.value(vid);
				attachVideo(vid, vi.title, vi.author);
			} else
				attachVideoById(vid); // this is more expensive
		}
	}

	if ( forcedExit )
		return;

	if ( checkBoxPlayOMatic->isChecked() ) {
		if ( listWidgetAttachedVideos->count() > 0 )
			QTimer::singleShot(YOUTUBE_PLAYOMATIC_DELAY, this, SLOT(playNextVideo()));
		else
			QTimer::singleShot(0, this, SLOT(loadNullVideo()));
	} else
		QTimer::singleShot(0, this, SLOT(loadNullVideo()));

	fullyLoaded = true;
	QTimer::singleShot(100, this, SLOT(updateAttachedVideoInfoImages()));
}

void YouTubeVideoPlayer::adjustIconSizes()
{
	if ( forcedExit )
		return;

	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFontMetrics fm(f);
	QSize iconSize = QSize(fm.height() - 2, fm.height() - 2);
	comboBoxPreferredFormat->setIconSize(iconSize);
	toolButtonPlayPause->setIconSize(iconSize);
	toolButtonMute->setIconSize(iconSize);
	toolBox->setItemIcon(YOUTUBE_ATTACHED_VIDEOS_PAGE, QIcon(QPixmap(QString::fromUtf8(":/data/img/movie.png")).scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
	toolBox->setItemIcon(YOUTUBE_VIDEO_PLAYER_PAGE, QIcon(QPixmap(QString::fromUtf8(":/data/img/youtube.png")).scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
	// FIXME: begin
	//toolButtonSuggest->setIconSize(iconSize);
	//toolButtonSearch->setIconSize(iconSize);
	//toolBox->setItemIcon(YOUTUBE_SEARCH_VIDEO_PAGE, QIcon(QPixmap(QString::fromUtf8(":/data/img/pacman.png")).scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
	// FIXME: end
	progressBarBufferStatus->setFixedWidth(progressBarBufferStatus->sizeHint().width() / 2);
}

void YouTubeVideoPlayer::videoFinished()
{
	seekSlider->setValue(0);
	labelSeekSlider->setText(tr("Remaining") + " --:--:--");
	toolButtonPlayPause->setIcon(QIcon(QString::fromUtf8(":/data/img/media_stop.png")));
	videoMenuPlayPauseAction->setIcon(QIcon(QString::fromUtf8(":/data/img/media_stop.png")));
	progressBarBufferStatus->setValue(0);
	progressBarBufferStatus->setToolTip(tr("Current buffer fill level: %1%").arg(0));
	if ( checkBoxPlayOMatic->isChecked() )
		QTimer::singleShot(YOUTUBE_PLAYOMATIC_DELAY, this, SLOT(playNextVideo()));
}

void YouTubeVideoPlayer::videoTick(qint64 time)
{
	static qint64 oldTime = 0;
	if ( time == oldTime )
		return;
	oldTime = time;
	QTime hrTime(0, 0, 0, 0);
	hrTime = hrTime.addMSecs(remainingTime());
	labelSeekSlider->setText(tr("Remaining") + " " + hrTime.toString("hh:mm:ss"));
	seekSlider->blockSignals(true);
	seekSlider->setValue((int)time);
	seekSlider->blockSignals(false);
}

void YouTubeVideoPlayer::videoBufferStatus(int percentFilled)
{
	progressBarBufferStatus->setValue(percentFilled);
	progressBarBufferStatus->setToolTip(tr("Current buffer fill level: %1%").arg(percentFilled));
	showMessage(tr("Buffering: %1%").arg(percentFilled));
}

#if QT_VERSION < 0x050000
void YouTubeVideoPlayer::videoStateChanged(Phonon::State newState, Phonon::State oldState)
{
	QTime hrTime(0, 0, 0, 0);

	switch ( newState ) {
		case Phonon::LoadingState:
			seekSlider->setValue(0);
			progressBarBufferStatus->setValue(0);
			progressBarBufferStatus->setToolTip(tr("Current buffer fill level: %1%").arg(0));
		case Phonon::BufferingState:
			showMessage(tr("Loading"));
			toolButtonPlayPause->setIcon(QIcon(QString::fromUtf8(":/data/img/media_stop.png")));
			toolButtonPlayPause->setEnabled(false);
			videoMenuPlayPauseAction->setIcon(QIcon(QString::fromUtf8(":/data/img/media_stop.png")));
			videoMenuPlayPauseAction->setEnabled(false);
			if ( remainingTime() > 0 ) {
				hrTime = hrTime.addMSecs(remainingTime());
				labelSeekSlider->setText(tr("Remaining") + " " + hrTime.toString("hh:mm:ss"));
			} else
				labelSeekSlider->setText(tr("Remaining") + " --:--:--");
			break;
		case Phonon::PlayingState:
			toolButtonPlayPause->setIcon(QIcon(QString::fromUtf8(":/data/img/media_play.png")));
			toolButtonPlayPause->setEnabled(true);
			videoMenuPlayPauseAction->setIcon(QIcon(QString::fromUtf8(":/data/img/media_play.png")));
			videoMenuPlayPauseAction->setEnabled(true);
			showMessage(tr("Playing"));
			break;
		case Phonon::PausedState:
			if ( loadOnly ) {
				loadOnly = false;
				audioOutput()->setMuted(isMuted);
			}
			toolButtonPlayPause->setIcon(QIcon(QString::fromUtf8(":/data/img/media_pause.png")));
			toolButtonPlayPause->setEnabled(true);
			videoMenuPlayPauseAction->setIcon(QIcon(QString::fromUtf8(":/data/img/media_pause.png")));
			videoMenuPlayPauseAction->setEnabled(true);
			showMessage(tr("Paused"));
			break;
		case Phonon::ErrorState:
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("video player: playback error: %1").arg(mediaObject()->errorString()));
			showMessage(tr("Video playback error: %1").arg(mediaObject()->errorString()), 4000);
			if ( loadOnly ) {
				loadOnly = false;
				audioOutput()->setMuted(isMuted);
			}
			toolButtonPlayPause->setIcon(QIcon(QString::fromUtf8(":/data/img/media_stop.png")));
			toolButtonPlayPause->setEnabled(!currentVideoID.isEmpty());
			videoMenuPlayPauseAction->setIcon(QIcon(QString::fromUtf8(":/data/img/media_stop.png")));
			videoMenuPlayPauseAction->setEnabled(!currentVideoID.isEmpty());
			labelSeekSlider->setText(tr("Remaining") + " --:--:--");
			seekSlider->setValue(0);
			progressBarBufferStatus->setValue(0);
			progressBarBufferStatus->setToolTip(tr("Current buffer fill level: %1%").arg(0));
			break;
		case Phonon::StoppedState:
		default:
			if ( loadOnly ) {
				loadOnly = false;
				audioOutput()->setMuted(isMuted);
			}
			comboBoxPreferredFormat->setEnabled(true);
			toolButtonPlayPause->setIcon(QIcon(QString::fromUtf8(":/data/img/media_stop.png")));
			toolButtonPlayPause->setEnabled(!currentVideoID.isEmpty());
			videoMenuPlayPauseAction->setIcon(QIcon(QString::fromUtf8(":/data/img/media_stop.png")));
			videoMenuPlayPauseAction->setEnabled(!currentVideoID.isEmpty());
			labelSeekSlider->setText(tr("Remaining") + " --:--:--");
			seekSlider->setValue(0);
			progressBarBufferStatus->setValue(0);
			progressBarBufferStatus->setToolTip(tr("Current buffer fill level: %1%").arg(0));
			break;
	}
}
#else
void YouTubeVideoPlayer::videoStateChanged(QMediaPlayer::State state)
{
	QTime hrTime(0, 0, 0, 0);

	switch ( state ) {
		case QMediaPlayer::PlayingState:
			toolButtonPlayPause->setIcon(QIcon(QString::fromUtf8(":/data/img/media_play.png")));
			toolButtonPlayPause->setEnabled(true);
			videoMenuPlayPauseAction->setIcon(QIcon(QString::fromUtf8(":/data/img/media_play.png")));
			videoMenuPlayPauseAction->setEnabled(true);
			showMessage(tr("Playing"));
			break;
		case QMediaPlayer::PausedState:
			if ( loadOnly ) {
				loadOnly = false;
				audioOutput()->setMuted(isMuted);
			}
			toolButtonPlayPause->setIcon(QIcon(QString::fromUtf8(":/data/img/media_pause.png")));
			toolButtonPlayPause->setEnabled(true);
			videoMenuPlayPauseAction->setIcon(QIcon(QString::fromUtf8(":/data/img/media_pause.png")));
			videoMenuPlayPauseAction->setEnabled(true);
			showMessage(tr("Paused"));
			break;
		case QMediaPlayer::StoppedState:
		default:
			if ( loadOnly ) {
				loadOnly = false;
				audioOutput()->setMuted(isMuted);
			}
			comboBoxPreferredFormat->setEnabled(true);
			toolButtonPlayPause->setIcon(QIcon(QString::fromUtf8(":/data/img/media_stop.png")));
			toolButtonPlayPause->setEnabled(!currentVideoID.isEmpty());
			videoMenuPlayPauseAction->setIcon(QIcon(QString::fromUtf8(":/data/img/media_stop.png")));
			videoMenuPlayPauseAction->setEnabled(!currentVideoID.isEmpty());
			labelSeekSlider->setText(tr("Remaining") + " --:--:--");
			seekSlider->setValue(0);
			progressBarBufferStatus->setValue(0);
			progressBarBufferStatus->setToolTip(tr("Current buffer fill level: %1%").arg(0));
			QTimer::singleShot(0, this, SLOT(videoFinished()));
			break;
	}
}
#endif

void YouTubeVideoPlayer::loadVideo(QString &videoID)
{
	currentVideoID = videoID;
	QUrl url = getVideoStreamUrl(videoID);
	isMuted = audioOutput()->isMuted();
	if ( url.isValid() ) {
		loadOnly = true;
		if ( !isMuted )
			audioOutput()->setMuted(true);
#if QT_VERSION < 0x050000
		videoPlayer()->load(Phonon::MediaSource(QUrl::fromEncoded(url.toString().toLatin1().constData())));
#else
		videoPlayer()->setMedia(url);
#endif
		pause();
		if ( !playedVideos.contains(videoID) )
			playedVideos << videoID;
	}
}

void YouTubeVideoPlayer::playVideo(QString &videoID)
{
	currentVideoID = videoID;
	QUrl url = getVideoStreamUrl(videoID);
	if ( url.isValid() ) {
		loadOnly = false;
#if QT_VERSION < 0x050000
		videoPlayer()->play(Phonon::MediaSource(QUrl::fromEncoded(url.toString().toLatin1().constData())));
#else
		videoPlayer()->setMedia(url);
		play();
#endif
		comboBoxPreferredFormat->setEnabled(true);
		if ( !playedVideos.contains(videoID) )
			playedVideos << videoID;
	}
}

QUrl YouTubeVideoPlayer::getVideoStreamUrl(QString videoID, QStringList *videoInfoStringList, bool videoInfoOnly)
{
	static QUrl videoUrl;

	showMessage(tr("Fetching info for video ID '%1'").arg(videoID));

	availableFormats.clear();
	currentFormat = bestAvailableFormat = YOUTUBE_FORMAT_UNKNOWN_INDEX;

	if ( videoInfoReply ) {
		disconnect(videoInfoReply);
		delete videoInfoReply;
		videoInfoReply = 0;
	}
	videoInfoBuffer.clear();
	viError = viFinished = false;
	videoInfoRequest.setUrl(QString("http://www.youtube.com/get_video_info?&video_id=%1&key=%2").arg(videoID).arg(QMC2_GOOGLE_DEV_KEY));
	videoInfoRequest.setRawHeader("User-Agent", "QMC2 - MAME Catalog / Launcher II");
	videoInfoRequest.setRawHeader("X-GData-Key", QString("key=%1").arg(QMC2_GOOGLE_DEV_KEY).toLatin1());
	if ( videoInfoManager ) {
		disconnect(videoInfoManager);
		delete videoInfoManager;
		videoInfoManager = 0;
	}

	if ( forcedExit )
		return QUrl();

	videoInfoManager = new QNetworkAccessManager(this);
	videoInfoReply = videoInfoManager->get(videoInfoRequest);
	connect(videoInfoReply, SIGNAL(readyRead()), this, SLOT(videoInfoReadyRead()));
	connect(videoInfoReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(videoInfoError(QNetworkReply::NetworkError)));
	connect(videoInfoReply, SIGNAL(finished()), this, SLOT(videoInfoFinished()));

	QTime timer(0, 0, 0, 0);
	bool timeoutOccurred = false;
	timer.start();
	while ( !viFinished && !viError && !timeoutOccurred ) {
		timeoutOccurred = ( timer.elapsed() >= YOUTUBE_VIDEOINFO_TIMEOUT );
		if ( !timeoutOccurred ) {
			qApp->processEvents();
			QTest::qWait(YOUTUBE_VIDEOINFO_WAIT);
		}
		if ( forcedExit )
			return QUrl();
	}

	if ( viFinished && !viError && !timeoutOccurred ) {
		QStringList videoInfoList(videoInfoBuffer.split('&', QString::SkipEmptyParts));
#ifdef QMC2_DEBUG
		printf("\nFull info for video ID '%s':\n>>>>\n", (const char *)videoID.toLatin1());
#endif
		QString status, errorcode, errortext, author, authorUrl, thumbnail_url, title;
		QUrl debugUrl;
		foreach (QString vInfo, videoInfoList) {
#ifdef QMC2_DEBUG
			 printf("%s\n", (const char *)vInfo.toLatin1());
#endif
			 if ( vInfo.startsWith("status=") ) {
				 vInfo.replace(QRegExp("^status="), "");
				 status = vInfo;
			 } else if ( vInfo.startsWith("errorcode=") ) {
				 vInfo.replace(QRegExp("^errorcode="), "");
				 errorcode = vInfo;
			 } else if ( vInfo.startsWith("reason=") ) {
				 vInfo.replace(QRegExp("^reason="), "").replace("+", " ");
				 errortext = QUrl::fromPercentEncoding(vInfo.toLatin1());
			 } else if ( vInfo.startsWith("author=") ) {
				 vInfo.replace(QRegExp("^author="), "");
				 debugUrl = QUrl::fromEncoded(vInfo.toLatin1());
				 author = debugUrl.toString();
				 authorUrl = VIDEOITEM_YOUTUBE_AUTHOR_URL_PATTERN;
				 authorUrl.replace("$USER_ID$", author);
			 } else if ( vInfo.startsWith("thumbnail_url=") ) {
				 vInfo.replace(QRegExp("^thumbnail_url="), "");
#if QT_VERSION < 0x050000
				 debugUrl = QUrl::fromEncoded(vInfo.toLatin1());
				 thumbnail_url = debugUrl.toString();
#else
				 thumbnail_url = vInfo.replace("http%3A%2F%2F", "http://").replace("%2F", "/");
#endif
			 } else if ( vInfo.startsWith("title") ) {
				 vInfo.replace(QRegExp("^title="), "");
				 debugUrl = QUrl::fromEncoded(vInfo.toLatin1());
				 title = debugUrl.toString();
				 title.replace("+", " ");
			 }
		}

#ifdef QMC2_DEBUG
		printf(">>>>\n\nSelected (decoded) info for video ID '%s':\n>>>>\n", videoID.toLatin1().constData());
		printf("status        = '%s'\n", status.toLatin1().constData());
		printf("errorcode     = '%s'\n", errorcode.toLatin1().constData());
		printf("errortext     = '%s'\n", errortext.toLatin1().constData());
		printf("title         = '%s'\n", title.toLatin1().constData());
		printf("author        = '%s'\n", author.toLatin1().constData());
		printf("author_url    = '%s'\n", authorUrl.toLatin1().constData());
		printf("thumbnail_url = '%s'\n>>>>\n", thumbnail_url.toLatin1().constData());
		printf("\nAvailable formats / stream URLs for video ID '%s':\n>>>>\n", videoID.toLatin1().constData());
#endif

		if ( status != "ok" ) {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("video player: video info error: ID = '%1', status = '%2', errorCode = '%3', errorText = '%4'").arg(videoID).arg(status).arg(errorcode).arg(errortext));
			showMessage(tr("Video info error: %1").arg(errortext), 4000);
			return QString();
		}

		if ( videoInfoStringList ) {
			videoInfoStringList->clear();
			videoInfoStringList->append(title);
			videoInfoStringList->append(author);
			videoInfoStringList->append(authorUrl);
			videoInfoStringList->append(thumbnail_url);
		}

		if ( videoInfoOnly )
			return QString();

		currentVideoAuthor = author;
		currentVideoTitle = title;

		QMap<QString, QUrl> formatToUrlMap;
		foreach (QString videoInfo, videoInfoList) {
			if ( forcedExit )
				break;
			if ( videoInfo.startsWith("url_encoded_fmt_stream_map=") ) {
				QStringList fmtUrlMap(videoInfo.replace(QRegExp("^url_encoded_fmt_stream_map="), "").split("%2C", QString::SkipEmptyParts));
				foreach (QString fmtUrl, fmtUrlMap) {
					if ( forcedExit )
						break;
					QString sig, itag, url;
					foreach (QString urlPart, QUrl::fromEncoded(fmtUrl.toLatin1()).toString().split('&', QString::SkipEmptyParts)) {
						if ( urlPart.startsWith("url=") ) {
							url = urlPart.split("=")[1];
						} else {
							foreach (QString subPart, urlPart.split(',', QString::SkipEmptyParts)) {
								if ( subPart.startsWith("sig=") )
									sig = subPart.split('=')[1];
								if ( subPart.startsWith("itag=") )
									itag = subPart.split('=')[1];
							}
						}
					}
					if ( !itag.isEmpty() ) {
						QUrl decodedUrl;
#if QT_VERSION < 0x050000
						if ( !sig.isEmpty() )
							url += "&signature=" + sig;
						decodedUrl = QUrl::fromEncoded(url.toLatin1());
#else
						if ( !sig.isEmpty() )
							url += "%2526signature%253D" + sig;
						url.replace("http%253A%252F%252F", "http://").replace("%252F", "/").replace("%253F", "?").replace("%2526", "&").replace("%253D", "=").replace("%2525", "%25").replace("%25", "%");
						decodedUrl = QUrl(url, QUrl::StrictMode);
#endif
#ifdef QMC2_DEBUG
						printf("decodedUrl[%s] = %s\n", itag.toLatin1().constData(), decodedUrl.toString().toLatin1().constData());
#endif
						formatToUrlMap[itag] = decodedUrl;
					}
				}
			}
		}

		if ( forcedExit )
			return QString();

		int maxChars = comboBoxPreferredFormat->minimumContentsLength();
		for (int i = 0; i < comboBoxPreferredFormat->count(); i++) {
			if ( formatToUrlMap.contains(indexToFormat(i)) ) {
				comboBoxPreferredFormat->setItemIcon(i, QIcon(QString::fromUtf8(":/data/img/trafficlight_green.png")));
				availableFormats << i;
				if ( bestAvailableFormat < i )
					bestAvailableFormat = i;
			} else {
				comboBoxPreferredFormat->setItemIcon(i, QIcon(QString::fromUtf8(":/data/img/trafficlight_off.png")));
			}
			comboBoxPreferredFormat->setItemText(i, youTubeFormatNames[i]);
			maxChars = qMax(youTubeFormatNames[i].length(), maxChars);
		}
		for (int i = comboBoxPreferredFormat->currentIndex(); i >= 0; i--) {
			if ( formatToUrlMap.contains(indexToFormat(i)) ) {
				videoUrl = formatToUrlMap[indexToFormat(i)];
#ifdef QMC2_DEBUG
				QString fmtStr = indexToFormat(i);
				printf(">>>>\n\nSelected format / stream URL for video ID '%s':\n>>>>\n[%s]\t%s\n>>>>\n", (const char *)videoID.toLatin1(), (const char *)fmtStr.toLatin1(), (const char *)videoUrl.toString().toLatin1());
#endif
				currentFormat = i;
				comboBoxPreferredFormat->setItemText(i, "[" + youTubeFormatNames[i] + "]");
				maxChars = qMax(comboBoxPreferredFormat->itemText(i).length(), maxChars);
				break;
			}
		}
		comboBoxPreferredFormat->setMinimumContentsLength(maxChars);
		return videoUrl;
	} else if ( viError ) {
		return QString();
	} else if ( timeoutOccurred ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("video player: video info error: timeout occurred"));
		showMessage(tr("video info error: timeout occurred"), 4000);
		return QString();
	}

	showMessage(tr("video info error: unknown reason"), 4000);
	return QUrl();
}

QString YouTubeVideoPlayer::indexToFormat(int index)
{
	switch ( index ) {
		case YOUTUBE_FORMAT_MP4_3072P_INDEX: return YOUTUBE_FORMAT_MP4_3072P;
		case YOUTUBE_FORMAT_MP4_1080P_INDEX: return YOUTUBE_FORMAT_MP4_1080P;
		case YOUTUBE_FORMAT_MP4_720P_INDEX: return YOUTUBE_FORMAT_MP4_720P;
		case YOUTUBE_FORMAT_MP4_360P_INDEX: return YOUTUBE_FORMAT_MP4_360P;
		case YOUTUBE_FORMAT_FLV_480P_INDEX: return YOUTUBE_FORMAT_FLV_480P;
		case YOUTUBE_FORMAT_FLV_360P_INDEX: return YOUTUBE_FORMAT_FLV_360P;
		case YOUTUBE_FORMAT_FLV_240P_INDEX: default: return YOUTUBE_FORMAT_FLV_240P;
	}
}

void YouTubeVideoPlayer::videoInfoReadyRead()
{
	videoInfoBuffer += videoInfoReply->readAll();
}

void YouTubeVideoPlayer::videoInfoError(QNetworkReply::NetworkError error)
{
	viError = true;
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("video player: video info error: %1").arg(videoInfoReply->errorString()));
	showMessage(tr("Video info error: %1").arg(videoInfoReply->errorString()), 4000);
}

void YouTubeVideoPlayer::videoInfoFinished()
{
	viFinished = true;
}

void YouTubeVideoPlayer::on_toolButtonPlayPause_clicked()
{
	if ( isPlaying() ) {
		pausedByHideEvent = false;
		pause();
	} else if ( isPaused() )
		play();
	else if ( hasVideo() )
		play();
	else if ( !currentVideoID.isEmpty() ) {
		if ( currentVideoID.startsWith("#:") ) {
			QString vidCopy = currentVideoID;;
			vidCopy.remove(QRegExp("^\\#\\:"));
			playMovieFile(vidCopy);
		} else
			playVideo(currentVideoID);
	}
}

void YouTubeVideoPlayer::on_toolButtonMute_toggled(bool mute)
{
	audioOutput()->setMuted(mute);

	if ( mute )
		toolButtonMute->setIcon(QIcon(QString::fromUtf8(":/data/img/no_sound.png")));
	else
		toolButtonMute->setIcon(QIcon(QString::fromUtf8(":/data/img/sound.png")));
}

void YouTubeVideoPlayer::on_comboBoxPreferredFormat_activated(int index)
{
	if ( !currentVideoID.isEmpty() ) {
		if ( availableFormats.contains(index) ) {
			if ( isPlaying() ) {
				if ( currentFormat != index )
					playVideo(currentVideoID);
			} else if ( index == currentFormat ) {
				play();
			} else {
				playVideo(currentVideoID);
			}
		} else if ( currentFormat < bestAvailableFormat && index > currentFormat ) {
			playVideo(currentVideoID);
		} else if ( index >= currentFormat ) {
			play();
		}
	}
}

void YouTubeVideoPlayer::on_toolBox_currentChanged(int page)
{
	switch ( page ) {
		case YOUTUBE_VIDEO_PLAYER_PAGE:
			break;
		case YOUTUBE_ATTACHED_VIDEOS_PAGE:
		case YOUTUBE_SEARCH_VIDEO_PAGE:
		default:
			clearMessage();
			break;
	}
}

void YouTubeVideoPlayer::showEvent(QShowEvent *e)
{
	if ( isPaused() && pausedByHideEvent )
		play();
	pausedByHideEvent = false;
}

void YouTubeVideoPlayer::hideEvent(QHideEvent *e)
{
	if ( isPlaying() ) {
		pausedByHideEvent = true;
		pause();
	}
}

void YouTubeVideoPlayer::on_listWidgetAttachedVideos_itemActivated(QListWidgetItem *item)
{
	VideoItemWidget *viw = (VideoItemWidget *)listWidgetAttachedVideos->itemWidget(item);
	if ( viw ) {
		toolBox->setCurrentIndex(YOUTUBE_VIDEO_PLAYER_PAGE);
		if ( viw->itemType == VIDEOITEM_TYPE_LOCAL_MOVIE || viw->itemType == VIDEOITEM_TYPE_VIDEO_SNAP ) {
			if ( currentVideoID == viw->videoID ) {
				if ( !isPlaying() )
					play();
			} else {
				QString vidCopy = viw->videoID;
				vidCopy.remove(QRegExp("^\\#\\:"));
				playMovieFile(vidCopy);
			}
		} else {
			if ( currentVideoID == viw->videoID && !isPlaying() )
				play();
			else if ( currentVideoID != viw->videoID || !isPlaying() )
				playVideo(viw->videoID);
		}
	}
}

void YouTubeVideoPlayer::on_listWidgetAttachedVideos_itemSelectionChanged()
{
	for (int i = 0; i < listWidgetAttachedVideos->count(); i++) {
		QListWidgetItem *item = listWidgetAttachedVideos->item(i);
		VideoItemWidget *viw = (VideoItemWidget *)listWidgetAttachedVideos->itemWidget(item);
		if ( viw ) {
			if ( item->isSelected() ) {
				viw->setBackgroundRole(QPalette::Highlight);
				viw->setForegroundRole(QPalette::HighlightedText);
			} else {
				viw->setBackgroundRole(QPalette::Window);
				viw->setForegroundRole(QPalette::WindowText);
			}
		}
	}
}

void YouTubeVideoPlayer::on_listWidgetSearchResults_itemActivated(QListWidgetItem *item)
{
	VideoItemWidget *viw = (VideoItemWidget *)listWidgetSearchResults->itemWidget(item);
	if ( viw ) {
		toolBox->setCurrentIndex(YOUTUBE_VIDEO_PLAYER_PAGE);
		if ( currentVideoID == viw->videoID && !isPlaying() )
			play();
		else if ( currentVideoID != viw->videoID || !isPlaying() )
			playVideo(viw->videoID);
	}
}

void YouTubeVideoPlayer::on_listWidgetAttachedVideos_customContextMenuRequested(const QPoint &p)
{
	QString clipboardText = qApp->clipboard()->text();
	clipboardText.replace("https:", "http:");
	if ( clipboardText.indexOf(QRegExp("^http\\:\\/\\/.*youtube\\.com\\/watch\\?.*v\\=.*$")) == 0 || clipboardText.indexOf(QRegExp("^http\\:\\/\\/.*youtu\\.be\\/.*$")) == 0 )
		avmActionPasteVideoUrl->setEnabled(true);
	else
		avmActionPasteVideoUrl->setEnabled(false);

	QWidget *w = listWidgetAttachedVideos->viewport();
	if ( sender() )
		if ( sender()->objectName() == "QMC2_VIDEO_TITLE" )
			w = (QWidget *)sender();
	if ( w && menuAttachedVideos ) {
		VideoItemWidget *viw = (VideoItemWidget *)listWidgetAttachedVideos->itemWidget(listWidgetAttachedVideos->itemAt(p));
		if ( w->objectName() == "QMC2_VIDEO_TITLE" )
			viw = (VideoItemWidget *)w->parentWidget();
		if ( viw ) {
			avmActionRemoveVideos->setEnabled(true);
			avmActionPlayVideo->setEnabled(true);
			if ( viw->itemType == VIDEOITEM_TYPE_YOUTUBE ) {
				avmActionCopyVideoUrl->setEnabled(true);
				avmActionCopyVideoUrlAlt->setEnabled(true);
				avmActionCopyAuthorUrl->setEnabled(true);
			} else {
				avmActionCopyVideoUrl->setEnabled(false);
				avmActionCopyVideoUrlAlt->setEnabled(false);
				avmActionCopyAuthorUrl->setEnabled(false);
			}
		} else {
			avmActionPlayVideo->setEnabled(false);
			avmActionCopyVideoUrl->setEnabled(false);
			avmActionCopyVideoUrlAlt->setEnabled(false);
			avmActionCopyAuthorUrl->setEnabled(false);
			avmActionRemoveVideos->setEnabled(false);
		}
		menuAttachedVideos->move(qmc2MainWindow->adjustedWidgetPosition(w->mapToGlobal(p), menuAttachedVideos));
		menuAttachedVideos->show();
	}
}

void YouTubeVideoPlayer::on_listWidgetSearchResults_customContextMenuRequested(const QPoint &p)
{
	QWidget *w = listWidgetSearchResults->viewport();
	if ( sender() )
		if ( sender()->objectName() == "QMC2_VIDEO_TITLE" )
			w = (QWidget *)sender();
	if ( w && menuSearchResults ) {
		if ( listWidgetSearchResults->itemAt(p) ) {
			menuSearchResults->move(qmc2MainWindow->adjustedWidgetPosition(w->mapToGlobal(p), menuSearchResults));
			menuSearchResults->show();
		}
	}
}

void YouTubeVideoPlayer::videoPlayer_customContextMenuRequested(const QPoint &p)
{
	if ( menuVideoPlayer ) {
		QString clipboardText = qApp->clipboard()->text();
		clipboardText.replace("https:", "http:");
		if ( clipboardText.indexOf(QRegExp("^http\\:\\/\\/.*youtube\\.com\\/watch\\?.*v\\=.*$")) == 0 || clipboardText.indexOf(QRegExp("^http\\:\\/\\/.*youtu\\.be\\/.*$")) == 0 )
			videoMenuPasteVideoUrlAction->setEnabled(true);
		else
			videoMenuPasteVideoUrlAction->setEnabled(false);

		if ( currentVideoID.isEmpty() ) {
			videoMenuPlayPauseAction->setEnabled(false);
			videoMenuCopyVideoUrlAction->setEnabled(false);
			videoMenuCopyVideoUrlAltAction->setEnabled(false);
			videoMenuCopyAuthorUrlAction->setEnabled(false);
			videoMenuAttachVideoAction->setEnabled(false);
		} else {
			videoMenuPlayPauseAction->setEnabled(true);
			videoMenuAttachVideoAction->setEnabled(true);
			if ( currentVideoID.startsWith("#:") ) {
				videoMenuCopyVideoUrlAction->setEnabled(false);
				videoMenuCopyVideoUrlAltAction->setEnabled(false);
				videoMenuCopyAuthorUrlAction->setEnabled(false);
			} else {
				videoMenuCopyVideoUrlAction->setEnabled(true);
				videoMenuCopyVideoUrlAltAction->setEnabled(true);
				videoMenuCopyAuthorUrlAction->setEnabled(true);
			}
		}
		QString keySeq = qmc2CustomShortcutHash["F11"];
#if QT_VERSION < 0x050000
		if ( videoWidget()->isFullScreen() ) {
			QString text = tr("Return to windowed mode");
			if ( !keySeq.isEmpty() )
				text += QString(" (%1)").arg(keySeq);
			videoMenuFullscreenAction->setText(text);
			menuVideoPlayer->move(p);
		} else {
			if ( !keySeq.isEmpty() )
				videoMenuFullscreenAction->setText(tr("Full screen (press %1 to return)").arg(keySeq));
			else
				videoMenuFullscreenAction->setText(tr("Full screen (return with toggle-key)"));
			menuVideoPlayer->move(qmc2MainWindow->adjustedWidgetPosition(videoPlayer()->mapToGlobal(p), menuVideoPlayer));
		}
#else
		if ( mFullscreenVideoWidget ) {
			QString text = tr("Return to windowed mode");
			if ( !keySeq.isEmpty() )
				text += QString(" (%1)").arg(keySeq);
			videoMenuFullscreenAction->setText(text);
			menuVideoPlayer->move(p);
		} else {
			if ( !keySeq.isEmpty() )
				videoMenuFullscreenAction->setText(tr("Full screen (press %1 to return)").arg(keySeq));
			else
				videoMenuFullscreenAction->setText(tr("Full screen (return with toggle-key)"));
			menuVideoPlayer->move(qmc2MainWindow->adjustedWidgetPosition(videoWidget()->mapToGlobal(p), menuVideoPlayer));
		}
#endif
		menuVideoPlayer->show();
	}
}

void YouTubeVideoPlayer::videoPlayer_durationChanged(qint64 duration)
{
	seekSlider->setMaximum((int)duration);
}

void YouTubeVideoPlayer::on_lineEditSearchString_textChanged(const QString &text)
{
	toolButtonSearch->setEnabled(!text.isEmpty());
	spinBoxStartIndex->setValue(1);
}

void YouTubeVideoPlayer::on_toolButtonSuggest_clicked()
{
	QString suggestedSearchPattern = mySetName;
	suggestedSearchPattern = suggestedSearchPattern.replace(QRegExp("\\(.*\\)"), "").replace("\\", " ").replace("/", " ").simplified();
	if ( !suggestorAppendString.isEmpty() )
		suggestedSearchPattern.append(" " + suggestorAppendString);
	QTreeWidgetItem *item = qmc2MachineListItemHash.value(mySetID);
	if ( item )
		suggestedSearchPattern.replace("$MANUFACTURER$", item->text(QMC2_MACHINELIST_COLUMN_MANU)).replace("$YEAR$", item->text(QMC2_MACHINELIST_COLUMN_YEAR));
	lineEditSearchString->setText(suggestedSearchPattern);
}

void YouTubeVideoPlayer::on_toolButtonSearch_clicked()
{
	lineEditSearchString->setEnabled(false);
	spinBoxStartIndex->setEnabled(false);
	spinBoxResultsPerRequest->setEnabled(false);
	toolButtonSuggest->setEnabled(false);
	toolButtonSearch->setEnabled(false);
	listWidgetSearchResults->setEnabled(false);
	listWidgetSearchResults->setUpdatesEnabled(false);

	searchRequestBuffer.clear();
	listWidgetSearchResults->clear();
	savedSearchString = lineEditSearchString->text();
	QString queryString = savedSearchString.simplified();
	// retrieve an XML feed from http://gdata.youtube.com/feeds/api/videos?max-results=<max-results>&start-index=<start-index>&q=<query-string>&key=<developer-key>
	searchRequest.setUrl(QString("http://gdata.youtube.com/feeds/api/videos?max-results=%1&start-index=%2&q=%3&key=%4").arg(spinBoxResultsPerRequest->value()).arg(spinBoxStartIndex->value()).arg(queryString).arg(QMC2_GOOGLE_DEV_KEY));
	searchRequest.setRawHeader("User-Agent", "QMC2 - MAME Catalog / Launcher II");
	searchRequest.setRawHeader("X-GData-Key", QString("key=%1").arg(QMC2_GOOGLE_DEV_KEY).toLatin1());
	if ( searchRequestManager ) {
		disconnect(searchRequestManager);
		delete searchRequestManager;
		searchRequestManager = 0;
	}
	searchRequestManager = new QNetworkAccessManager(this);
	searchRequestReply = searchRequestManager->get(searchRequest);
	connect(searchRequestReply, SIGNAL(readyRead()), this, SLOT(searchRequestReadyRead()));
	connect(searchRequestReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(searchRequestError(QNetworkReply::NetworkError)));
	connect(searchRequestReply, SIGNAL(finished()), this, SLOT(searchRequestFinished()));
}

void YouTubeVideoPlayer::on_volumeSlider_valueChanged(int volume)
{
#if QT_VERSION < 0x050000
	audioOutput()->setVolume((double)volume / 100.0);
#else
	audioOutput()->setVolume(volume);
#endif
	labelVolumeSlider->setText(tr("Volume") + " " + QString::number(volume) + "%");
}

void YouTubeVideoPlayer::on_seekSlider_valueChanged(int position)
{
#if QT_VERSION < 0x050000
	mediaObject()->seek(position);
#else
	videoPlayer()->setPosition(position);
#endif
}

void YouTubeVideoPlayer::updateAttachedVideoInfoImages()
{
	for (int i = 0; !forcedExit && i < listWidgetAttachedVideos->count(); i++) {
		QListWidgetItem *item = listWidgetAttachedVideos->item(i);
		VideoItemWidget *viw = (VideoItemWidget *)listWidgetAttachedVideos->itemWidget(item);
		if ( !viw )
			continue;
		if ( viw->videoImageValid )
			continue;
		if ( viw->videoID.startsWith("#:") )
			continue;

		ImagePixmap *imagePixmap = qmc2ImagePixmapCache.object("yt_" + viw->videoID);
		bool pixmapFound = (imagePixmap != 0);
		if ( !pixmapFound ) {
			QDir youTubeCacheDir(qmc2Config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/CacheDirectory").toString());
			if ( youTubeCacheDir.exists() ) {
				QString imageFile = viw->videoID + ".png";
				if ( youTubeCacheDir.exists(imageFile) ) {
					QPixmap pm;
					pixmapFound = pm.load(youTubeCacheDir.filePath(imageFile), "PNG");
					if ( pixmapFound ) {
						ImagePixmap ipm = pm;
						ipm.imagePath = youTubeCacheDir.filePath(imageFile);
						qmc2ImagePixmapCache.insert("yt_" + viw->videoID, new ImagePixmap(ipm), pm.toImage().byteCount());
						viw->setImage(&ipm);
						continue;
					}
				}
			}
		} else {
			viw->setImage(imagePixmap);
			continue;
		}
		
		if ( videoImageReply ) {
			disconnect(videoImageReply);
			delete videoImageReply;
			videoImageReply = 0;
		}
		videoImageBuffer.clear();
		vimgError = vimgFinished = false;
		videoImageRequest.setUrl(QString("http://www.youtube.com/get_video_info?&video_id=%1&key=%2").arg(viw->videoID).arg(QMC2_GOOGLE_DEV_KEY));
		videoImageRequest.setRawHeader("User-Agent", "QMC2 - MAME Catalog / Launcher II");
		videoImageRequest.setRawHeader("X-GData-Key", QString("key=%1").arg(QMC2_GOOGLE_DEV_KEY).toLatin1());
		if ( videoImageManager ) {
			disconnect(videoImageManager);
			delete videoImageManager;
			videoImageManager = 0;
		}
		videoImageManager = new QNetworkAccessManager(this);
		videoImageReply = videoImageManager->get(videoImageRequest);
		connect(videoImageReply, SIGNAL(readyRead()), this, SLOT(videoImageReadyRead()));
		connect(videoImageReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(videoImageError(QNetworkReply::NetworkError)));
		connect(videoImageReply, SIGNAL(finished()), this, SLOT(videoImageFinished()));
		QTime timer(0, 0, 0, 0);
		bool timeoutOccurred = false;
		timer.start();
		while ( !forcedExit && !vimgFinished && !vimgError && !timeoutOccurred ) {
			timeoutOccurred = ( timer.elapsed() >= YOUTUBE_VIDEOINFO_TIMEOUT );
			if ( !timeoutOccurred ) {
				qApp->processEvents();
				QTest::qWait(YOUTUBE_VIDEOINFO_WAIT);
			}
		}
		if ( !forcedExit && vimgFinished && !vimgError ) {
			QStringList videoInfoList = videoImageBuffer.split("&", QString::SkipEmptyParts);
			QString thumbnail_url;
			foreach (QString vInfo, videoInfoList) {
				if ( forcedExit )
					break;
				if ( vInfo.startsWith("thumbnail_url=") ) {
					vInfo.replace(QRegExp("^thumbnail_url="), "");
#if QT_VERSION < 0x050000
					thumbnail_url = QUrl::fromEncoded(vInfo.toLatin1()).toString();
#else
					thumbnail_url = vInfo.replace("http%3A%2F%2F", "http://").replace("%2F", "/");
#endif
					break;
				}
			}
			if ( !forcedExit ) {
				if ( !thumbnail_url.isEmpty() ) {
#ifdef QMC2_DEBUG
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::updateAttachedVideoInfoImages(): downloading thumbnail for video ID '%1' from '%2'").arg(viw->videoID).arg(thumbnail_url));
#endif
					imageDownloadManager->get(QNetworkRequest(QUrl(thumbnail_url)));
				}
#ifdef QMC2_DEBUG
			       	else if ( !viw->videoID.startsWith("#:") )
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::updateAttachedVideoInfoImages(): thumbnail URL for video ID '%1' not found!").arg(viw->videoID));
#endif
			}
		}
	}

	if ( !forcedExit )
		listWidgetAttachedVideos->setUpdatesEnabled(true);
}

void YouTubeVideoPlayer::videoImageReadyRead()
{
	videoImageBuffer += videoImageReply->readAll();
}

void YouTubeVideoPlayer::videoImageError(QNetworkReply::NetworkError error)
{
	vimgError = true;
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("video player: video image info error: %1").arg(videoInfoReply->errorString()));
	showMessage(tr("Video info error: %1").arg(videoInfoReply->errorString()), 4000);
}

void YouTubeVideoPlayer::videoImageFinished()
{
	vimgFinished = true;
}

void YouTubeVideoPlayer::searchRequestReadyRead()
{
	QString part = searchRequestReply->readAll();
#ifdef QMC2_DEBUG
	printf("%s", (const char *)part.toLatin1());
#endif
	searchRequestBuffer += part;
}

void YouTubeVideoPlayer::searchRequestError(QNetworkReply::NetworkError error)
{
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("video player: search request error: %1").arg(searchRequestReply->errorString()));
	lineEditSearchString->setEnabled(true);
	spinBoxStartIndex->setEnabled(true);
	spinBoxResultsPerRequest->setEnabled(true);
	toolButtonSuggest->setEnabled(false);
	toolButtonSearch->setEnabled(true);
	listWidgetSearchResults->setEnabled(true);
	listWidgetSearchResults->setUpdatesEnabled(true);
}

void YouTubeVideoPlayer::searchRequestFinished()
{
	QXmlInputSource xmlInputSource;
	xmlInputSource.setData(searchRequestBuffer);
	YouTubeXmlHandler xmlHandler(listWidgetSearchResults, this);
	QXmlSimpleReader xmlReader;
	xmlReader.setContentHandler(&xmlHandler);
	xmlReader.setErrorHandler(&xmlHandler);
#ifdef QMC2_DEBUG
	printf("\n");
#endif
	if ( xmlReader.parse(xmlInputSource) ) {
		if ( savedSearchString == lineEditSearchString->text() && !savedSearchString.isEmpty() )
			spinBoxStartIndex->setValue(spinBoxStartIndex->value() + listWidgetSearchResults->count());
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("video player: search error: can't parse XML data"));

	lineEditSearchString->setEnabled(true);
	spinBoxStartIndex->setEnabled(true);
	spinBoxResultsPerRequest->setEnabled(true);
	toolButtonSuggest->setEnabled(true);
	toolButtonSearch->setEnabled(true);
	listWidgetSearchResults->setEnabled(true);
	listWidgetSearchResults->setUpdatesEnabled(true);
}

void YouTubeVideoPlayer::imageDownloadFinished(QNetworkReply *reply)
{
	QString urlString = reply->url().toString();
	if ( forcedExit ) {
		reply->deleteLater();
		return;
	}
	// example URL: 'http://i3.ytimg.com/vi/bFjX1uUhB1A/default.jpg'
	QString videoID;
	QRegExp rx("http\\:\\/\\/.*\\/vi\\/(.*)\\/.*");
	int pos = rx.indexIn(urlString);
	if ( pos > -1 ) {
		videoID = rx.cap(1);
#ifdef QMC2_DEBUG
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::imageDownloadFinished(QNetworkReply *reply = %1): videoID = '%2'").arg((qulonglong)reply).arg(videoID));
#endif
	} else {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("video player: can't determine the video ID from the reply URL '%1' -- please inform developers").arg(urlString));
		reply->deleteLater();
		return;
	}

	if ( !viwMap.contains(videoID) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("video player: can't associate the returned image for video ID '%1' -- please inform developers").arg(urlString));
		reply->deleteLater();
		return;
	}

	VideoItemWidget *viw = viwMap[videoID];

	if ( reply->error() == QNetworkReply::NoError ) {
		QImageReader imageReader(reply);
		ImagePixmap pm = QPixmap::fromImageReader(&imageReader);
		if ( !pm.isNull() ) {
			qmc2ImagePixmapCache.insert("yt_" + videoID, new ImagePixmap(pm), pm.toImage().byteCount());
			viw->setImage(pm, true);
			QDir youTubeCacheDir(qmc2Config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/CacheDirectory").toString());
			if ( youTubeCacheDir.exists() ) {
				QString imagePath = youTubeCacheDir.canonicalPath() + "/" + videoID + ".png";
				if ( !pm.save(imagePath, "PNG") )
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("video player: can't save the image for video ID '%1' to the YouTube cache directory '%2' -- please check permissions").arg(videoID).arg(youTubeCacheDir.canonicalPath()));
			} else
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("video player: can't save the image for video ID '%1', the YouTube cache directory '%2' does not exist -- please correct").arg(videoID).arg(youTubeCacheDir.canonicalPath()));
		} else
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("video player: image download failed for video ID '%1', retrieved image is not valid").arg(videoID));
	} else
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("video player: image download failed for video ID '%1', error text = '%2'").arg(videoID).arg(reply->errorString()));

	reply->deleteLater();
}

YouTubeXmlHandler::YouTubeXmlHandler(QListWidget *lw, YouTubeVideoPlayer *vp)
{
	listWidget = lw;
	mVideoPlayer = vp;
	isEntry = false;
}

bool YouTubeXmlHandler::startElement(const QString &/*namespaceURI*/, const QString &/*localName*/, const QString &qName, const QXmlAttributes &/*attributes*/)
{
	if ( qName == "entry" )
		isEntry = true;
	else if (qName == "id" )
		currentText.clear();
	else if ( qName == "title" )
		currentText.clear();
	else if ( qName == "author" )
		currentText.clear();
	return true;
}

bool YouTubeXmlHandler::endElement(const QString &/*namespaceURI*/, const QString &/*localName*/, const QString &qName)
{
	if ( !isEntry )
		return true;

	if ( qName == "entry" ) {
#ifdef QMC2_DEBUG
		printf("end of entry\n");
#endif
		QListWidgetItem *listWidgetItem = new QListWidgetItem(listWidget);
		listWidget->setItemWidget(listWidgetItem, new VideoItemWidget(id, title, author, 0, VIDEOITEM_TYPE_YOUTUBE_SEARCH, mVideoPlayer, listWidget));
		listWidgetItem->setSizeHint(QSize(VIDEOITEM_IMAGE_WIDTH, VIDEOITEM_IMAGE_HEIGHT + 4));
		isEntry = false;
	} else if ( qName == "id" ) {
		id = currentText.remove(0, currentText.lastIndexOf("/") + 1);
#ifdef QMC2_DEBUG
		printf("    id     = '%s'\n", id.toUtf8().constData());
#endif
	} else if ( qName == "title" ) {
		title = currentText;
#ifdef QMC2_DEBUG
		printf("    title  = '%s'\n", title.toUtf8().constData());
#endif
	} else if ( qName == "author" ) {
		author = currentText.left(currentText.indexOf("http://"));
#ifdef QMC2_DEBUG
		printf("    author = '%s'\n", author.toUtf8().constData());
#endif
	}

	return true;
}

bool YouTubeXmlHandler::characters(const QString &chars)
{
	currentText += QString::fromUtf8(chars.toUtf8());

	return true;
}

bool YouTubeXmlHandler::fatalError(const QXmlParseException &exception)
{
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QObject::tr("video player: XML error: fatal error on line %1, column %2: %3").arg(exception.lineNumber()).arg(exception.columnNumber()).arg(exception.message()));

	return false;
}

bool VideoEventFilter::eventFilter(QObject *object, QEvent *event)
{
	if ( event->type() == QEvent::MouseButtonPress ) {
		QMouseEvent *e = static_cast<QMouseEvent *>(event);
		switch ( e->button() ) {
			case Qt::LeftButton:
				if ( mPlayer->isPlaying() )
					mPlayer->pause();
				else
					mPlayer->play();
				break;
			case Qt::RightButton:
				mPlayer->videoPlayer_customContextMenuRequested(e->pos());
				break;
			default:
				break;
		}
	}
	return QObject::eventFilter(object, event);
}

#endif
