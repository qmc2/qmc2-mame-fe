#if defined(QMC2_YOUTUBE_ENABLED)

#include <QtTest>
#include <QSettings>
#include <QClipboard>
#include <QInputDialog>
#include <QPixmapCache>
#include <QImageReader>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QMap>

#include "macros.h"
#include "qmc2main.h"
#include "youtubevideoplayer.h"
#include "videoitemwidget.h"

extern MainWindow *qmc2MainWindow;
extern QSettings *qmc2Config;
extern QMap <QString, YouTubeVideoInfo> qmc2YouTubeVideoInfoMap;
extern QMap<QString, QString> qmc2CustomShortcutMap;
extern bool qmc2YouTubeVideoInfoMapChanged;

YouTubeVideoPlayer::YouTubeVideoPlayer(QString sID, QString sName, QWidget *parent)
	: QWidget(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::YouTubeVideoPlayer(QString sID = %1, QString sName = %2, QWidget *parent = %3)").arg(sID).arg(sName).arg((qulonglong) parent));
#endif

	setupUi(this);

	mySetID = sID;
	mySetName = sName;

	videoPlayer->videoWidget()->setScaleMode(Phonon::VideoWidget::FitInView);
	videoPlayer->videoWidget()->setAspectRatio(Phonon::VideoWidget::AspectRatioAuto);

	videoOverlayWidget = new VideoOverlayWidget(videoPlayer->videoWidget());

	toolButtonPlayPause->setEnabled(false);
	toolButtonSearch->setEnabled(false);
	comboBoxPreferredFormat->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/PreferredFormat", YOUTUBE_FORMAT_MP4_1080P_INDEX).toInt());
	videoPlayer->audioOutput()->setVolume(qmc2Config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/AudioVolume", 0.5).toDouble());
	toolBox->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/PageIndex", YOUTUBE_SEARCH_VIDEO_PAGE).toInt());
	checkBoxPlayOMatic->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/PlayOMatic/Enabled", false).toBool());
	comboBoxMode->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/PlayOMatic/Mode", YOUTUBE_PLAYOMATIC_SEQUENTIAL).toInt());
	checkBoxRepeat->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/PlayOMatic/Repeat", true).toBool());
#if defined(QMC2_EMUTYPE_MAME)
	suggestorAppendString = qmc2Config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/SuggestorAppendString", "Arcade").toString();
#else
	suggestorAppendString = qmc2Config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/SuggestorAppendString", "").toString();
#endif
	spinBoxResultsPerRequest->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/SearchResultsPerRequest", 10).toInt());

	// serious hack to access the volume slider's tool button object
	privateMuteButton = volumeSlider->findChild<QToolButton *>();
	if ( privateMuteButton ) {
		privateMuteButton->setCheckable(true);
		privateMuteButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		privateMuteButton->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/AudioMuted", false).toBool());
		privateMuteButton->setToolTip(tr("Mute / unmute audio output"));
		videoPlayer->audioOutput()->setMuted(privateMuteButton->isChecked());
	}

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

	forcedExit = loadOnly = isMuted = pausedByHideEvent = viError = viFinished = vimgError = vimgFinished = false;
	videoInfoReply = videoImageReply = searchRequestReply = NULL;
	videoInfoManager = videoImageManager = searchRequestManager = NULL;
	currentFormat = bestAvailableFormat = YOUTUBE_FORMAT_UNKNOWN_INDEX;
	videoSeqNum = 0;

	videoPlayer->mediaObject()->setTickInterval(1000);
	volumeSlider->setAudioOutput(videoPlayer->audioOutput());
	seekSlider->setMediaObject(videoPlayer->mediaObject());
	seekSlider->setToolTip(tr("Video progress"));
	connect(videoPlayer->mediaObject(), SIGNAL(tick(qint64)), this, SLOT(videoTick(qint64)));
	connect(videoPlayer->mediaObject(), SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(videoStateChanged(Phonon::State, Phonon::State)));
	connect(videoPlayer->mediaObject(), SIGNAL(bufferStatus(int)), this, SLOT(videoBufferStatus(int)));
	connect(videoPlayer, SIGNAL(finished()), this, SLOT(videoFinished()));

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
	s = tr("Copy author URL");
	action = menuAttachedVideos->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/youtube.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(copyAuthorUrl()));
	avmActionCopyAuthorUrl = action;
	s = tr("Paste video URL");
	action = menuAttachedVideos->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/youtube.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(pasteYouTubeUrl()));
	avmActionPasteVideoUrl = action;
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
	connect(action, SIGNAL(triggered()), this, SLOT(goFullScreen()));
	menuVideoPlayer->addSeparator();
	s = tr("Copy video URL");
	action = menuVideoPlayer->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/youtube.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(copyCurrentYouTubeUrl()));
	s = tr("Copy author URL");
	action = menuVideoPlayer->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/youtube.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(copyCurrentAuthorUrl()));
	menuVideoPlayer->addSeparator();
	s = tr("Attach this video");
	action = menuVideoPlayer->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/movie.png")));
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
	if ( autoSuggestAction->isChecked() )
		on_toolButtonSuggest_clicked();

	imageDownloadManager = new QNetworkAccessManager(this);
	connect(imageDownloadManager, SIGNAL(finished(QNetworkReply *)), this, SLOT(imageDownloadFinished(QNetworkReply *)));

	lineEditSearchString->setPlaceholderText(tr("Enter search string"));

	QTimer::singleShot(0, this, SLOT(adjustIconSizes()));
	QTimer::singleShot(100, this, SLOT(init()));
}

YouTubeVideoPlayer::~YouTubeVideoPlayer()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::~YouTubeVideoPlayer()");
#endif

	// clean up
	if ( videoOverlayWidget ) {
		disconnect(videoOverlayWidget);
		delete videoOverlayWidget;
	}
	if ( videoPlayer ) {
		disconnect(videoPlayer);
		delete videoPlayer;
	}
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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::saveSettings()");
#endif

  	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "YouTubeWidget/PreferredFormat", comboBoxPreferredFormat->currentIndex());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "YouTubeWidget/AudioVolume", videoPlayer->audioOutput()->volume());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "YouTubeWidget/PageIndex", toolBox->currentIndex());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "YouTubeWidget/PlayOMatic/Enabled", checkBoxPlayOMatic->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "YouTubeWidget/PlayOMatic/Mode", comboBoxMode->currentIndex());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "YouTubeWidget/PlayOMatic/Repeat", checkBoxRepeat->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "YouTubeWidget/AutoSuggest", autoSuggestAction->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "YouTubeWidget/SuggestorAppendString", suggestorAppendString);
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "YouTubeWidget/SearchResultsPerRequest", spinBoxResultsPerRequest->value());

	QList<QListWidgetItem *> il = listWidgetAttachedVideos->findItems("*", Qt::MatchWildcard);
	QStringList attachedVideos;
	foreach (QListWidgetItem *item, il) {
		VideoItemWidget *viw = (VideoItemWidget *)listWidgetAttachedVideos->itemWidget(item);
		attachedVideos << viw->videoID;
	}
	if ( attachedVideos.isEmpty() )
		qmc2Config->remove(QString(QMC2_FRONTEND_PREFIX + "YouTubeVideos/%1").arg(mySetID));
	else
		qmc2Config->setValue(QString(QMC2_FRONTEND_PREFIX + "YouTubeVideos/%1").arg(mySetID), attachedVideos);

	// serious hack to access the volume slider's tool button object
	privateMuteButton = volumeSlider->findChild<QToolButton *>();
	if ( privateMuteButton )
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "YouTubeWidget/AudioMuted", privateMuteButton->isChecked());
}

void YouTubeVideoPlayer::setSuggestorAppendString()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::setSuggestorAppendString()");
#endif

	bool ok;
	QString appendString = QInputDialog::getText(this,
						tr("Appended string"),
						tr("Enter the string to be appended when suggesting a pattern:"),
						QLineEdit::Normal,
						suggestorAppendString,
						&ok);
	if ( ok )
		suggestorAppendString = appendString;
}

void YouTubeVideoPlayer::loadNullVideo()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::loadNullVideo()");
#endif

	if ( forcedExit )
		return;

	currentVideoID.clear();
	currentVideoAuthor.clear();
	currentVideoTitle.clear();
	if ( videoPlayer->isPlaying() || videoPlayer->isPaused() )
		videoPlayer->stop();
}

void YouTubeVideoPlayer::playNextVideo()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::playNextVideo()");
#endif

	QList<QListWidgetItem *> il = listWidgetAttachedVideos->findItems("*", Qt::MatchWildcard);
	if ( il.count() > 0 ) {
		switch ( comboBoxMode->currentIndex() ) {
			case YOUTUBE_PLAYOMATIC_RANDOM: {
					int index = qrand() % il.count();
					VideoItemWidget *viw = (VideoItemWidget *)listWidgetAttachedVideos->itemWidget(il[index]);
					if ( checkBoxRepeat->isChecked() )
						playVideo(viw->videoID);
					else if ( !playedVideos.contains(viw->videoID) )
						playVideo(viw->videoID);
					else if ( playedVideos.count() < il.count() ) {
						do {
							index = qrand() % il.count();
							viw = (VideoItemWidget *)listWidgetAttachedVideos->itemWidget(il[index]);
						} while ( playedVideos.contains(viw->videoID) );
						playVideo(viw->videoID);
					} else
						videoPlayer->stop();
				}
				break;
			case YOUTUBE_PLAYOMATIC_SEQUENTIAL:
			default: {
					 if ( videoSeqNum > il.count() - 1 || videoSeqNum < 0 )
						 videoSeqNum = 0;
					 VideoItemWidget *viw = (VideoItemWidget *)listWidgetAttachedVideos->itemWidget(il[videoSeqNum]);
					 if ( checkBoxRepeat->isChecked() )
						 playVideo(viw->videoID);
					 else if ( !playedVideos.contains(viw->videoID) )
						 playVideo(viw->videoID);
					 else
						 videoPlayer->stop();
					 videoSeqNum++;
				}
				break;
		}	
	}
}

void YouTubeVideoPlayer::playAttachedVideo()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::playAttachedVideo()");
#endif

	QListWidgetItem *item = listWidgetAttachedVideos->currentItem();
	if ( item ) {
		on_listWidgetAttachedVideos_itemActivated(item);
		videoSeqNum = listWidgetAttachedVideos->row(item);
	}
}

void YouTubeVideoPlayer::playSearchedVideo()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::playSearchedVideo()");
#endif

	QListWidgetItem *item = listWidgetSearchResults->currentItem();
	if ( item )
		on_listWidgetSearchResults_itemActivated(item);
}

void YouTubeVideoPlayer::copyYouTubeUrl()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::copyYouTubeUrl()");
#endif

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

void YouTubeVideoPlayer::pasteYouTubeUrl()
{
	QString videoID = qApp->clipboard()->text();
	videoID.replace(QRegExp("^http\\:\\/\\/.*youtube\\.com\\/watch\\?v\\=(.*)$"), "\\1").replace(QRegExp("\\&.*$"), "");

#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::pasteYouTubeUrl(): videoID = '%1'").arg(videoID));
#endif

	if ( videoID.isEmpty() )
		return;

	QStringList videoInfoList;
	getVideoStreamUrl(videoID, &videoInfoList, true);
	if ( videoInfoList.count() > 2 ) {
		attachVideo(videoID, videoInfoList[0], videoInfoList[1]);
		QTimer::singleShot(10, this, SLOT(updateAttachedVideoInfoImages()));
	}
}

void YouTubeVideoPlayer::copyAuthorUrl()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::copyAuthorUrl()");
#endif

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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::copySearchYouTubeUrl()");
#endif

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

void YouTubeVideoPlayer::copySearchAuthorUrl()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::copySearchAuthorUrl()");
#endif

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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::copyCurrentYouTubeUrl()");
#endif

	if ( !currentVideoID.isEmpty() ) {
		QString url = VIDEOITEM_YOUTUBE_URL_PATTERN;
		url.replace("$VIDEO_ID$", currentVideoID);
		qApp->clipboard()->setText(url);
	}
}

void YouTubeVideoPlayer::copyCurrentAuthorUrl()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::copyCurrentAuthorUrl()");
#endif

	if ( !currentVideoAuthor.isEmpty() ) {
		QString url = VIDEOITEM_YOUTUBE_AUTHOR_URL_PATTERN;
		url.replace("$USER_ID$", currentVideoAuthor);
		qApp->clipboard()->setText(url);
	}
}

void YouTubeVideoPlayer::removeSelectedVideos()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::removeSelectedVideos()");
#endif

	QList<QListWidgetItem *> il = listWidgetAttachedVideos->selectedItems();
	foreach (QListWidgetItem *item, il) {
		VideoItemWidget *viw = (VideoItemWidget *)listWidgetAttachedVideos->itemWidget(item);
		viwMap.remove(viw->videoID);
		QListWidgetItem *i = listWidgetAttachedVideos->takeItem(listWidgetAttachedVideos->row(item));
		delete i;
	}
}

void YouTubeVideoPlayer::goFullScreen()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::goFullScreen()");
#endif

	if ( !videoPlayer->videoWidget()->isFullScreen() ) {
		videoPlayer->videoWidget()->setFullScreen(true);
		QString keySeq = qmc2CustomShortcutMap["F11"];
		if ( !keySeq.isEmpty() )
			videoOverlayWidget->showMessage(tr("Full-screen mode -- press %1 to return to windowed mode").arg(keySeq), 4000);
		else
			videoOverlayWidget->showMessage(tr("Full-screen mode -- press toggle-key to return to windowed mode"), 4000);
	} else
		videoPlayer->videoWidget()->setFullScreen(false);
}

void YouTubeVideoPlayer::attachVideoById(QString id)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::attachVideoById(QString id = %1)").arg(id));
#endif

	QStringList videoInfoList;
	getVideoStreamUrl(id, &videoInfoList, true);
	if ( videoInfoList.count() > 2 )
		attachVideo(id, videoInfoList[0], videoInfoList[1]);
}

void YouTubeVideoPlayer::attachVideo(QString id, QString title, QString author)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::attachVideo(QString id = '%1', QString title = '%2', QString author = '%3')").arg(id).arg(title).arg(author));
#endif

	if ( viwMap.keys().contains(id) ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("video player: a video with the ID '%1' is already attached, ignored").arg(id));
		return;
	}

	QSize size(VIDEOITEM_IMAGE_WIDTH, VIDEOITEM_IMAGE_HEIGHT + 4);

	bool pixmapFound = false;
	QPixmap imagePixmap;
	pixmapFound = QPixmapCache::find("yt_" + id, &imagePixmap);
	if ( !pixmapFound ) {
		QDir youTubeCacheDir(qmc2Config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/CacheDirectory").toString());
		if ( youTubeCacheDir.exists() ) {
			QString imageFile = id + ".png";
			if ( youTubeCacheDir.exists(imageFile) ) {
				pixmapFound = imagePixmap.load(youTubeCacheDir.filePath(imageFile), "PNG");
				if ( pixmapFound )
					QPixmapCache::insert("yt_" + id, imagePixmap);
			}
		}
	}

	QListWidgetItem *listWidgetItem = new QListWidgetItem(listWidgetAttachedVideos);
	listWidgetItem->setSizeHint(size);
	VideoItemWidget *videoItemWidget;
	if ( pixmapFound )
		videoItemWidget = new VideoItemWidget(id, title, author, imagePixmap, VIDEOITEM_TYPE_YOUTUBE, this, this);
	else
		videoItemWidget = new VideoItemWidget(id, title, author, VIDEOITEM_TYPE_YOUTUBE, this, this);
	listWidgetAttachedVideos->setItemWidget(listWidgetItem, videoItemWidget);
	viwMap[id] = videoItemWidget;
	qmc2YouTubeVideoInfoMap[id] = YouTubeVideoInfo(title, author);
	qmc2YouTubeVideoInfoMapChanged = true;
}

void YouTubeVideoPlayer::attachCurrentVideo()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::attachCurrentVideo()");
#endif

	if ( !currentVideoID.isEmpty() && !currentVideoTitle.isEmpty() && !currentVideoAuthor.isEmpty() ) {
		attachVideo(currentVideoID, currentVideoTitle, currentVideoAuthor);
		QTimer::singleShot(10, this, SLOT(updateAttachedVideoInfoImages()));
	}
}

void YouTubeVideoPlayer::attachSearchedVideo()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::attachSearchedVideo()");
#endif

	QListWidgetItem *item = listWidgetSearchResults->currentItem();
	if ( item ) {
		VideoItemWidget *viw = (VideoItemWidget *)listWidgetSearchResults->itemWidget(item);
		if ( viw ) {
			attachVideo(viw->videoID, viw->videoTitle, viw->videoAuthor);
			QTimer::singleShot(10, this, SLOT(updateAttachedVideoInfoImages()));
		}
	}
}

void YouTubeVideoPlayer::init()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::init()");
#endif

	if ( forcedExit )
		return;

	QStringList attachedVideos = qmc2Config->value(QString(QMC2_FRONTEND_PREFIX + "YouTubeVideos/%1").arg(mySetID), QStringList()).toStringList();
	foreach(QString vid, attachedVideos ) {
		if ( qmc2YouTubeVideoInfoMap.contains(vid) ) {
			YouTubeVideoInfo vi = qmc2YouTubeVideoInfoMap[vid];
			attachVideo(vid, vi.title, vi.author);
		} else
			attachVideoById(vid); // this is more expensive
	}

	if ( forcedExit )
		return;

	if ( checkBoxPlayOMatic->isChecked() ) {
		QList<QListWidgetItem *> il = listWidgetAttachedVideos->findItems("*", Qt::MatchWildcard);
		if ( il.count() > 0 ) {
			QTimer::singleShot(YOUTUBE_PLAYOMATIC_DELAY, this, SLOT(playNextVideo()));
			toolBox->setCurrentIndex(YOUTUBE_VIDEO_PLAYER_PAGE);
		} else
			QTimer::singleShot(0, this, SLOT(loadNullVideo()));
	} else
		QTimer::singleShot(0, this, SLOT(loadNullVideo()));

	QTimer::singleShot(100, this, SLOT(updateAttachedVideoInfoImages()));
}

void YouTubeVideoPlayer::adjustIconSizes()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::adjustIconSizes()"));
#endif

	if ( forcedExit )
		return;

	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFontMetrics fm(f);
	QSize iconSize = QSize(fm.height() - 2, fm.height() - 2);
	comboBoxPreferredFormat->setIconSize(iconSize);
	toolButtonPlayPause->setIconSize(iconSize);
	seekSlider->setIconSize(iconSize);
	volumeSlider->setIconSize(iconSize);
	// serious hack to access the volume slider's tool button object
	privateMuteButton = volumeSlider->findChild<QToolButton *>();
	if ( privateMuteButton ) {
		privateMuteButton->setIconSize(iconSize);
		privateMuteButton->setFixedHeight(toolButtonPlayPause->height());
	}
	toolButtonSuggest->setIconSize(iconSize);
	toolButtonSearch->setIconSize(iconSize);
	toolBox->setItemIcon(YOUTUBE_ATTACHED_VIDEOS_PAGE, QIcon(QPixmap(QString::fromUtf8(":/data/img/movie.png")).scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
	toolBox->setItemIcon(YOUTUBE_VIDEO_PLAYER_PAGE, QIcon(QPixmap(QString::fromUtf8(":/data/img/youtube.png")).scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
	toolBox->setItemIcon(YOUTUBE_SEARCH_VIDEO_PAGE, QIcon(QPixmap(QString::fromUtf8(":/data/img/pacman.png")).scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
	progressBarBufferStatus->setFixedWidth(progressBarBufferStatus->sizeHint().width() / 2);
}

void YouTubeVideoPlayer::videoFinished()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::videoFinished()");
#endif

	// serious hack to access the seekSlider's slider object
	privateSeekSlider = seekSlider->findChild<QSlider *>();
	if ( privateSeekSlider )
		privateSeekSlider->setValue(0);

	labelPlayingTime->setText("--:--:--");
	toolButtonPlayPause->setIcon(QIcon(QString::fromUtf8(":/data/img/media_stop.png")));
	videoMenuPlayPauseAction->setIcon(QIcon(QString::fromUtf8(":/data/img/media_stop.png")));
	progressBarBufferStatus->setValue(0);
	progressBarBufferStatus->setToolTip(tr("Current buffer fill level: %1%").arg(0));

	if ( checkBoxPlayOMatic->isChecked() )
		QTimer::singleShot(YOUTUBE_PLAYOMATIC_DELAY, this, SLOT(playNextVideo()));
}

void YouTubeVideoPlayer::videoTick(qint64 time)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::videoTick(quint64 time = %1)").arg(time));
#endif

	QTime hrTime;
	hrTime = hrTime.addMSecs(videoPlayer->mediaObject()->remainingTime());
	labelPlayingTime->setText(hrTime.toString("hh:mm:ss"));
}

void YouTubeVideoPlayer::videoBufferStatus(int percentFilled)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::videoBufferStatus(int percentFilled = %1)").arg(percentFilled));
#endif

	progressBarBufferStatus->setValue(percentFilled);
	progressBarBufferStatus->setToolTip(tr("Current buffer fill level: %1%").arg(percentFilled));
	videoOverlayWidget->showMessage(tr("Buffering: %1%").arg(percentFilled));
}

void YouTubeVideoPlayer::videoStateChanged(Phonon::State newState, Phonon::State oldState)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::videoStateChanged(Phonon::State newState = %1, Phonon::State oldState = %2)").arg(newState).arg(oldState));
#endif

	QTime hrTime;
	qint64 remainingTime;

	switch ( newState ) {
		case Phonon::LoadingState:
			// serious hack to access the seekSlider's slider object
			privateSeekSlider = seekSlider->findChild<QSlider *>();
			if ( privateSeekSlider )
				privateSeekSlider->setValue(0);
			progressBarBufferStatus->setValue(0);
			progressBarBufferStatus->setToolTip(tr("Current buffer fill level: %1%").arg(0));
		case Phonon::BufferingState:
			videoOverlayWidget->showMessage(tr("Loading"));
			toolButtonPlayPause->setIcon(QIcon(QString::fromUtf8(":/data/img/media_stop.png")));
			toolButtonPlayPause->setEnabled(false);
			videoMenuPlayPauseAction->setIcon(QIcon(QString::fromUtf8(":/data/img/media_stop.png")));
			videoMenuPlayPauseAction->setEnabled(false);
			remainingTime = videoPlayer->mediaObject()->remainingTime();
			if ( remainingTime > 0 ) {
				hrTime = hrTime.addMSecs(remainingTime);
				labelPlayingTime->setText(hrTime.toString("hh:mm:ss"));
			} else
				labelPlayingTime->setText("--:--:--");
			break;
		case Phonon::PlayingState:
			toolButtonPlayPause->setIcon(QIcon(QString::fromUtf8(":/data/img/media_play.png")));
			toolButtonPlayPause->setEnabled(true);
			videoMenuPlayPauseAction->setIcon(QIcon(QString::fromUtf8(":/data/img/media_play.png")));
			videoMenuPlayPauseAction->setEnabled(true);
			videoOverlayWidget->showMessage(tr("Playing"));
			break;
		case Phonon::PausedState:
			if ( loadOnly ) {
				loadOnly = false;
				videoPlayer->audioOutput()->setMuted(isMuted);
			}
			toolButtonPlayPause->setIcon(QIcon(QString::fromUtf8(":/data/img/media_pause.png")));
			toolButtonPlayPause->setEnabled(true);
			videoMenuPlayPauseAction->setIcon(QIcon(QString::fromUtf8(":/data/img/media_pause.png")));
			videoMenuPlayPauseAction->setEnabled(true);
			videoOverlayWidget->showMessage(tr("Paused"));
			break;
		case Phonon::ErrorState:
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("video player: playback error: %1").arg(videoPlayer->mediaObject()->errorString()));
			videoOverlayWidget->showMessage(tr("Video playback error: %1").arg(videoPlayer->mediaObject()->errorString()), 4000);
			if ( loadOnly ) {
				loadOnly = false;
				videoPlayer->audioOutput()->setMuted(isMuted);
			}
			toolButtonPlayPause->setIcon(QIcon(QString::fromUtf8(":/data/img/media_stop.png")));
			toolButtonPlayPause->setEnabled(!currentVideoID.isEmpty());
			videoMenuPlayPauseAction->setIcon(QIcon(QString::fromUtf8(":/data/img/media_stop.png")));
			videoMenuPlayPauseAction->setEnabled(!currentVideoID.isEmpty());
			labelPlayingTime->setText("--:--:--");
			// serious hack to access the seekSlider's slider object
			privateSeekSlider = seekSlider->findChild<QSlider *>();
			if ( privateSeekSlider )
				privateSeekSlider->setValue(0);
			progressBarBufferStatus->setValue(0);
			progressBarBufferStatus->setToolTip(tr("Current buffer fill level: %1%").arg(0));
			break;
		case Phonon::StoppedState:
		default:
			if ( loadOnly ) {
				loadOnly = false;
				videoPlayer->audioOutput()->setMuted(isMuted);
			}
			toolButtonPlayPause->setIcon(QIcon(QString::fromUtf8(":/data/img/media_stop.png")));
			toolButtonPlayPause->setEnabled(!currentVideoID.isEmpty());
			videoMenuPlayPauseAction->setIcon(QIcon(QString::fromUtf8(":/data/img/media_stop.png")));
			videoMenuPlayPauseAction->setEnabled(!currentVideoID.isEmpty());
			labelPlayingTime->setText("--:--:--");
			// serious hack to access the seekSlider's slider object
			privateSeekSlider = seekSlider->findChild<QSlider *>();
			if ( privateSeekSlider )
				privateSeekSlider->setValue(0);
			progressBarBufferStatus->setValue(0);
			progressBarBufferStatus->setToolTip(tr("Current buffer fill level: %1%").arg(0));
			break;
	}
}

void YouTubeVideoPlayer::loadVideo(QString &videoID)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::loadVideo(QString videoID = %1)").arg(videoID));
#endif

	currentVideoID = videoID;
	QUrl url = getVideoStreamUrl(videoID);
	isMuted = videoPlayer->audioOutput()->isMuted();
	if ( url.isValid() ) {
		loadOnly = true;
		if ( !isMuted )
			videoPlayer->audioOutput()->setMuted(true);
		videoPlayer->load(Phonon::MediaSource(QUrl::fromEncoded((const char *)url.toString().toLatin1())));
		videoPlayer->pause();
		if ( !playedVideos.contains(videoID) ) playedVideos << videoID;
	}
}

void YouTubeVideoPlayer::playVideo(QString &videoID)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::playVideo(QString videoID = %1)").arg(videoID));
#endif

	currentVideoID = videoID;
	QUrl url = getVideoStreamUrl(videoID);
	if ( url.isValid() ) {
		loadOnly = false;
		videoPlayer->play(Phonon::MediaSource(QUrl::fromEncoded((const char *)url.toString().toLatin1())));
		if ( !playedVideos.contains(videoID) ) playedVideos << videoID;
	}
}

QUrl YouTubeVideoPlayer::getVideoStreamUrl(QString videoID, QStringList *videoInfoStringList, bool videoInfoOnly)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::getVideoStreamUrl(QString videoID = %1, QStringList *videoInfoStringList = %2, bool videoInfoOnly = %3)").arg(videoID).arg((qulonglong)videoInfoStringList).arg(videoInfoOnly));
#endif

	static QUrl videoUrl;

	videoOverlayWidget->showMessage(tr("Fetching info for video ID '%1'").arg(videoID));

	availableFormats.clear();
	currentFormat = bestAvailableFormat = YOUTUBE_FORMAT_UNKNOWN_INDEX;

	if ( videoInfoReply ) {
		disconnect(videoInfoReply);
		delete videoInfoReply;
		videoInfoReply = NULL;
	}
	videoInfoBuffer.clear();
	viError = viFinished = false;
	videoInfoRequest.setUrl(QString("http://www.youtube.com/get_video_info?&video_id=%1").arg(videoID));
	//videoInfoRequest.setRawHeader("User-Agent", "QMC2's YouTube Player");
	if ( videoInfoManager ) {
		disconnect(videoInfoManager);
		delete videoInfoManager;
		videoInfoManager = NULL;
	}
	videoInfoManager = new QNetworkAccessManager(this);
	videoInfoReply = videoInfoManager->get(videoInfoRequest);
	connect(videoInfoReply, SIGNAL(readyRead()), this, SLOT(videoInfoReadyRead()));
	connect(videoInfoReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(videoInfoError(QNetworkReply::NetworkError)));
	connect(videoInfoReply, SIGNAL(finished()), this, SLOT(videoInfoFinished()));

	QTime timer;
	bool timeoutOccurred = false;
	timer.start();
	while ( !viFinished && !viError && !timeoutOccurred ) {
		timeoutOccurred = ( timer.elapsed() >= YOUTUBE_VIDEOINFO_TIMEOUT );
		if ( !timeoutOccurred ) {
			qApp->processEvents();
			QTest::qWait(YOUTUBE_VIDEOINFO_WAIT);
		}
	}

	if ( viFinished && !viError ) {
		QStringList videoInfoList = videoInfoBuffer.split("&");
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
				 debugUrl = QUrl::fromEncoded(vInfo.toAscii());
				 author = debugUrl.toString();
				 authorUrl = VIDEOITEM_YOUTUBE_AUTHOR_URL_PATTERN;
				 authorUrl.replace("$USER_ID$", author);
			 } else if ( vInfo.startsWith("thumbnail_url=") ) {
				 vInfo.replace(QRegExp("^thumbnail_url="), "");
				 debugUrl = QUrl::fromEncoded(vInfo.toAscii());
				 thumbnail_url = debugUrl.toString();
			 } else if ( vInfo.startsWith("title") ) {
				 vInfo.replace(QRegExp("^title="), "");
				 debugUrl = QUrl::fromEncoded(vInfo.toAscii());
				 title = debugUrl.toString();
				 title.replace("+", " ");
			 }
		}

#ifdef QMC2_DEBUG
		printf(">>>>\n\nSelected (decoded) info for video ID '%s':\n>>>>\n", (const char *)videoID.toLatin1());
		printf("status        = '%s'\n", (const char *)status.toLatin1());
		printf("errorcode     = '%s'\n", (const char *)errorcode.toLatin1());
		printf("errortext     = '%s'\n", (const char *)errortext.toLatin1());
		printf("title         = '%s'\n", (const char *)title.toLatin1());
		printf("author        = '%s'\n", (const char *)author.toLatin1());
		printf("author_url    = '%s'\n", (const char *)authorUrl.toLatin1());
		printf("thumbnail_url = '%s'\n>>>>\n", (const char *)thumbnail_url.toLatin1());
		printf("\nAvailable formats / stream URLs for video ID '%s':\n>>>>\n", (const char *)videoID.toLatin1());
#endif

		if ( status != "ok" ) {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("video player: video info error: ID = '%1', status = '%2', errorCode = '%3', errorText = '%4'").arg(videoID).arg(status).arg(errorcode).arg(errortext));
			videoOverlayWidget->showMessage(tr("Video info error: %1").arg(errortext), 4000);
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

		QMap <QString, QUrl> formatToUrlMap;
		foreach (QString videoInfo, videoInfoList) {
			if ( videoInfo.startsWith("url_encoded_fmt_stream_map=") ) {
				QStringList fmtUrlMap = videoInfo.replace(QRegExp("^url_encoded_fmt_stream_map="), "").split(QRegExp("url%3D"), QString::SkipEmptyParts);
				foreach (QString fmtUrl, fmtUrlMap) {
					QString encodedUrlString = QUrl::fromEncoded(fmtUrl.toLatin1()).toString().remove(QRegExp("\\,$")).remove(QRegExp("\\&fallback_host\\=.*$"));
					QUrl decodedUrl;
				    	decodedUrl.setEncodedUrl(encodedUrlString.toLatin1());
					QString urlString = decodedUrl.toString();
					QString itagValue;
					int start = urlString.indexOf("&itag=");
					if ( start > 0 ) {
						start += 6;
						itagValue = urlString.mid(start, urlString.indexOf("&", start) - start);
					}
#ifdef QMC2_DEBUG
					printf("decodedUrl[itag = %s] = %s\n", (const char *)itagValue.toLatin1(), (const char *)decodedUrl.toString().toLatin1());
#endif
					if ( !itagValue.isEmpty() )
						formatToUrlMap[itagValue] = decodedUrl;
				}
			}
		}

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
		}
		for (int i = comboBoxPreferredFormat->currentIndex(); i >= 0; i--) {
			if ( formatToUrlMap.contains(indexToFormat(i)) ) {
				videoUrl = formatToUrlMap[indexToFormat(i)];
#ifdef QMC2_DEBUG
				QString fmtStr = indexToFormat(i);
				printf(">>>>\n\nSelected format / stream URL for video ID '%s':\n>>>>\n%s\t%s\n>>>>\n", (const char *)videoID.toLatin1(), (const char *)fmtStr.toLatin1(), (const char *)videoUrl.toString().toLatin1());
#endif
				currentFormat = i;
				comboBoxPreferredFormat->setItemText(i, "[" + youTubeFormatNames[i] + "]");
				break;
			}
		}
		return videoUrl;
	} else if ( viError ) {
		return QString();
	} else if ( timeoutOccurred ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("video player: video info error: timeout occurred"));
		videoOverlayWidget->showMessage(tr("Video info error: timeout occurred"), 4000);
		return QString();
	}

	return QString();
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
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::videoInfoReadyRead()");
#endif

	videoInfoBuffer += videoInfoReply->readAll();
}

void YouTubeVideoPlayer::videoInfoError(QNetworkReply::NetworkError error)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::videoInfoError(QNetworkReply::NetworkError error = %1)").arg(error));
#endif

	viError = true;
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("video player: video info error: %1").arg(videoInfoReply->errorString()));
	videoOverlayWidget->showMessage(tr("Video info error: %1").arg(videoInfoReply->errorString()), 4000);
}

void YouTubeVideoPlayer::videoInfoFinished()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::videoInfoFinished()");
#endif

	viFinished = true;
}

void YouTubeVideoPlayer::on_toolButtonPlayPause_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::on_toolButtonPlayPause_clicked()");
#endif

	if ( videoPlayer->isPlaying() ) {
		pausedByHideEvent = false;
		videoPlayer->pause();
	} else if ( videoPlayer->isPaused() )
		videoPlayer->play();
	else if ( videoPlayer->mediaObject()->hasVideo() )
		videoPlayer->play();
	else if ( !currentVideoID.isEmpty() )
		playVideo(currentVideoID);
}

void YouTubeVideoPlayer::on_comboBoxPreferredFormat_activated(int index)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::on_comboBoxPreferredFormat_activated(int index = %1)").arg(index));
#endif

	if ( !currentVideoID.isEmpty() ) {
		if ( availableFormats.contains(index) ) {
			if ( videoPlayer->isPlaying() ) {
				if ( currentFormat != index )
					playVideo(currentVideoID);
			} else if ( index == currentFormat ) {
				videoPlayer->play();
			} else {
				playVideo(currentVideoID);
			}
		} else if ( currentFormat < bestAvailableFormat && index > currentFormat ) {
			playVideo(currentVideoID);
		} else if ( index >= currentFormat ) {
			videoPlayer->play();
		}
	}
}

void YouTubeVideoPlayer::on_toolBox_currentChanged(int page)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::on_toolBox_currentChanged(int page = %1)").arg(page));
#endif

	switch ( page ) {
		case YOUTUBE_VIDEO_PLAYER_PAGE:
			break;
		case YOUTUBE_ATTACHED_VIDEOS_PAGE:
		case YOUTUBE_SEARCH_VIDEO_PAGE:
		default:
			videoOverlayWidget->clearMessage();
			break;
			break;
	}
}

void YouTubeVideoPlayer::showEvent(QShowEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::showEvent(QShowEvent *e = %1)").arg((qulonglong)e));
#endif

	if ( videoPlayer->isPaused() && pausedByHideEvent )
		videoPlayer->play();
	pausedByHideEvent = false;
}

void YouTubeVideoPlayer::hideEvent(QHideEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::hideEvent(QHideEvent *e = %1)").arg((qulonglong)e));
#endif

	if ( videoPlayer->isPlaying() ) {
		pausedByHideEvent = true;
		videoPlayer->pause();
	}
}

void YouTubeVideoPlayer::on_listWidgetAttachedVideos_itemActivated(QListWidgetItem *item)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::on_listWidgetAttachedVideos_itemActivated(QListWidgetItem *item = %1)").arg((qulonglong)item));
#endif

	VideoItemWidget *viw = (VideoItemWidget *)listWidgetAttachedVideos->itemWidget(item);
	if ( viw ) {
		toolBox->setCurrentIndex(YOUTUBE_VIDEO_PLAYER_PAGE);
		if ( currentVideoID == viw->videoID && !videoPlayer->isPlaying() )
			videoPlayer->play();
		else if ( currentVideoID != viw->videoID || !videoPlayer->isPlaying() )
			playVideo(viw->videoID);
	}
}

void YouTubeVideoPlayer::on_listWidgetSearchResults_itemActivated(QListWidgetItem *item)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::on_listWidgetSearchResults_itemActivated(QListWidgetItem *item = %1)").arg((qulonglong)item));
#endif

	VideoItemWidget *viw = (VideoItemWidget *)listWidgetSearchResults->itemWidget(item);
	if ( viw ) {
		toolBox->setCurrentIndex(YOUTUBE_VIDEO_PLAYER_PAGE);
		if ( currentVideoID == viw->videoID && !videoPlayer->isPlaying() )
			videoPlayer->play();
		else if ( currentVideoID != viw->videoID || !videoPlayer->isPlaying() )
			playVideo(viw->videoID);
	}
}

void YouTubeVideoPlayer::on_listWidgetAttachedVideos_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::on_listWidgetAttachedVideos_customContextMenuRequested(const QPoint &p = [%1, %2])").arg(p.x()).arg(p.y()));
#endif

	QString clipboardText = qApp->clipboard()->text();
	if ( clipboardText.indexOf(QRegExp("^http\\:\\/\\/.*youtube\\.com\\/watch\\?v\\=.*$")) == 0 )
		avmActionPasteVideoUrl->setEnabled(true);
	else
		avmActionPasteVideoUrl->setEnabled(false);

	QWidget *w = listWidgetAttachedVideos->viewport();
	if ( sender() )
		if ( sender()->objectName() == "QMC2_VIDEO_TITLE" )
			w = (QWidget *)sender();
	if ( w && menuAttachedVideos ) {
		if ( listWidgetAttachedVideos->itemAt(p) ) {
			avmActionPlayVideo->setEnabled(true);
			avmActionCopyVideoUrl->setEnabled(true);
			avmActionCopyAuthorUrl->setEnabled(true);
			avmActionRemoveVideos->setEnabled(true);
			menuAttachedVideos->move(qmc2MainWindow->adjustedWidgetPosition(w->mapToGlobal(p), menuAttachedVideos));
			menuAttachedVideos->show();
		} else {
			if ( avmActionPasteVideoUrl->isEnabled() ) {
				avmActionPlayVideo->setEnabled(false);
				avmActionCopyVideoUrl->setEnabled(false);
				avmActionCopyAuthorUrl->setEnabled(false);
				avmActionRemoveVideos->setEnabled(false);
				menuAttachedVideos->move(qmc2MainWindow->adjustedWidgetPosition(w->mapToGlobal(p), menuAttachedVideos));
				menuAttachedVideos->show();
			}
		}
	}
}

void YouTubeVideoPlayer::on_listWidgetSearchResults_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::on_listWidgetSearchResults_customContextMenuRequested(const QPoint &p = [%1, %2])").arg(p.x()).arg(p.y()));
#endif

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

void YouTubeVideoPlayer::on_videoPlayer_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::on_videoPlayer_customContextMenuRequested(const QPoint &p = [%1, %2])").arg(p.x()).arg(p.y()));
#endif

	if ( currentVideoID.isEmpty() )
		return;

	if ( menuVideoPlayer ) {
		QString keySeq = qmc2CustomShortcutMap["F11"];
		if ( !keySeq.isEmpty() )
			videoMenuFullscreenAction->setText(tr("Full screen (press %1 to return)").arg(keySeq));
		else
			videoMenuFullscreenAction->setText(tr("Full screen (return with toggle-key)"));
		menuVideoPlayer->move(qmc2MainWindow->adjustedWidgetPosition(videoPlayer->mapToGlobal(p), menuVideoPlayer));
		menuVideoPlayer->show();
	}
}

void YouTubeVideoPlayer::on_lineEditSearchString_textChanged(const QString &text)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::on_lineEditSearchString_textChanged(const QString &text = %1)").arg(text));
#endif

	toolButtonSearch->setEnabled(!text.isEmpty());
	spinBoxStartIndex->setValue(1);
}

void YouTubeVideoPlayer::on_toolButtonSuggest_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::on_toolButtonSuggest_clicked()");
#endif

	QString suggestedSearchPattern = mySetName;
	suggestedSearchPattern = suggestedSearchPattern.replace(QRegExp("\\(.*\\)"), "").replace("\\", " ").replace("/", " ").simplified();
	if ( !suggestorAppendString.isEmpty() )
		suggestedSearchPattern.append(" " + suggestorAppendString);
	lineEditSearchString->setText(suggestedSearchPattern);
}

void YouTubeVideoPlayer::on_toolButtonSearch_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::on_toolButtonSearch_clicked()");
#endif

	lineEditSearchString->setEnabled(false);
	spinBoxStartIndex->setEnabled(false);
	spinBoxResultsPerRequest->setEnabled(false);
	toolButtonSuggest->setEnabled(false);
	toolButtonSearch->setEnabled(false);
	listWidgetSearchResults->setEnabled(false);

	searchRequestBuffer.clear();
	listWidgetSearchResults->clear();
	savedSearchString = lineEditSearchString->text();
	QString queryString = savedSearchString.simplified();
	// retrieve an XML feed from http://gdata.youtube.com/feeds/api/videos?max-results=<max-results>&start-index=<start-index>&q=<query-string>
	searchRequest.setUrl(QString("http://gdata.youtube.com/feeds/api/videos?max-results=%1&start-index=%2&q=%3").arg(spinBoxResultsPerRequest->value()).arg(spinBoxStartIndex->value()).arg(queryString));
	if ( searchRequestManager ) {
		disconnect(searchRequestManager);
		delete searchRequestManager;
		searchRequestManager = NULL;
	}
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::on_toolButtonSearch_clicked(): search request URL: %1").arg(searchRequest.url().toString()));
#endif
	searchRequestManager = new QNetworkAccessManager(this);
	searchRequestReply = searchRequestManager->get(searchRequest);
	connect(searchRequestReply, SIGNAL(readyRead()), this, SLOT(searchRequestReadyRead()));
	connect(searchRequestReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(searchRequestError(QNetworkReply::NetworkError)));
	connect(searchRequestReply, SIGNAL(finished()), this, SLOT(searchRequestFinished()));
}

void YouTubeVideoPlayer::updateAttachedVideoInfoImages()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::updateAttachedVideoInfoImages()");
#endif

	foreach (QListWidgetItem *item, listWidgetAttachedVideos->findItems("*", Qt::MatchWildcard)) {
		if ( forcedExit )
			break;
		VideoItemWidget *viw = (VideoItemWidget *)listWidgetAttachedVideos->itemWidget(item);
		if ( !viw )
			continue;
		if ( viw->videoImageValid )
			continue;

		bool pixmapFound = false;
		QPixmap imagePixmap;
		pixmapFound = QPixmapCache::find("yt_" + viw->videoID, &imagePixmap);
		if ( !pixmapFound ) {
			QDir youTubeCacheDir(qmc2Config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/CacheDirectory").toString());
			if ( youTubeCacheDir.exists() ) {
				QString imageFile = viw->videoID + ".png";
				if ( youTubeCacheDir.exists(imageFile) ) {
					pixmapFound = imagePixmap.load(youTubeCacheDir.filePath(imageFile), "PNG");
					if ( pixmapFound )
						QPixmapCache::insert("yt_" + viw->videoID, imagePixmap);
				}
			}
		} else {
			viw->setImage(imagePixmap);
			continue;
		}
		
		if ( videoImageReply ) {
			disconnect(videoImageReply);
			delete videoImageReply;
			videoImageReply = NULL;
		}
		videoImageBuffer.clear();
		vimgError = vimgFinished = false;
		videoImageRequest.setUrl(QString("http://www.youtube.com/get_video_info?&video_id=%1").arg(viw->videoID));
		//videoImageRequest.setRawHeader("User-Agent", "QMC2's YouTube Player");
		if ( videoImageManager ) {
			disconnect(videoImageManager);
			delete videoImageManager;
			videoImageManager = NULL;
		}
		videoImageManager = new QNetworkAccessManager(this);
		videoImageReply = videoImageManager->get(videoImageRequest);
		connect(videoImageReply, SIGNAL(readyRead()), this, SLOT(videoImageReadyRead()));
		connect(videoImageReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(videoImageError(QNetworkReply::NetworkError)));
		connect(videoImageReply, SIGNAL(finished()), this, SLOT(videoImageFinished()));
		QTime timer;
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
			QStringList videoInfoList = videoImageBuffer.split("&");
			QString thumbnail_url;
			foreach (QString vInfo, videoInfoList) {
				if ( forcedExit )
					break;
				if ( vInfo.startsWith("thumbnail_url=") ) {
					vInfo.replace(QRegExp("^thumbnail_url="), "");
					thumbnail_url = QUrl::fromEncoded(vInfo.toAscii()).toString();
					break;
				}
			}
			if ( !forcedExit ) {
				if ( !thumbnail_url.isEmpty() ) {
#ifdef QMC2_DEBUG
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::updateAttachedVideoInfoImages(): downloading thumbnail for video ID '%1' from '%2'").arg(viw->videoID).arg(thumbnail_url));
#endif
					QNetworkReply *reply = imageDownloadManager->get(QNetworkRequest(QUrl(thumbnail_url)));
				} else {
#ifdef QMC2_DEBUG
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::updateAttachedVideoInfoImages(): thumbnail URL for video ID '%1' not found!").arg(viw->videoID));
#endif
				}
			}
		}
	}
}

void YouTubeVideoPlayer::videoImageReadyRead()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::videoImageReadyRead()");
#endif

	videoImageBuffer += videoImageReply->readAll();
}

void YouTubeVideoPlayer::videoImageError(QNetworkReply::NetworkError error)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::videoImageError(QNetworkReply::NetworkError error = %1)").arg(error));
#endif

	vimgError = true;
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("video player: video image info error: %1").arg(videoInfoReply->errorString()));
	videoOverlayWidget->showMessage(tr("Video info error: %1").arg(videoInfoReply->errorString()), 4000);
}

void YouTubeVideoPlayer::videoImageFinished()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::videoImageFinished()");
#endif

	vimgFinished = true;
}

void YouTubeVideoPlayer::searchRequestReadyRead()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::searchRequestReadyRead()");
#endif

	QString part = searchRequestReply->readAll();
#ifdef QMC2_DEBUG
	printf("%s", (const char *)part.toLatin1());
#endif
	searchRequestBuffer += part;
}

void YouTubeVideoPlayer::searchRequestError(QNetworkReply::NetworkError error)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::searchRequestError(QNetworkReply::NetworkError error = %1)").arg(error));
#endif

	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("video player: search request error: %1").arg(searchRequestReply->errorString()));
	lineEditSearchString->setEnabled(true);
	spinBoxStartIndex->setEnabled(true);
	spinBoxResultsPerRequest->setEnabled(true);
	toolButtonSuggest->setEnabled(false);
	toolButtonSearch->setEnabled(true);
	listWidgetSearchResults->setEnabled(true);
}

void YouTubeVideoPlayer::searchRequestFinished()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::searchRequestFinished()");
#endif

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
}

void YouTubeVideoPlayer::imageDownloadFinished(QNetworkReply *reply)
{
	QString urlString = reply->url().toString();

#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::imageDownloadFinished(QNetworkReply *reply = %1): URL = '%2'").arg((qulonglong)reply).arg(urlString));
#endif

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
		QPixmap pm = QPixmap::fromImageReader(&imageReader);
		if ( !pm.isNull() ) {
			QPixmapCache::insert("yt_" + videoID, pm);
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
	videoPlayer = vp;
	isEntry = false;
}

bool YouTubeXmlHandler::startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &attributes)
{
	if ( qName == "entry" ) {
#ifdef QMC2_DEBUG
		printf("start of entry\n");
#endif
		isEntry = true;
	} else if (qName == "id" ) {
		currentText.clear();
	} else if ( qName == "title" ) {
		currentText.clear();
	} else if ( qName == "author" ) {
		currentText.clear();
	}

	return true;
}

bool YouTubeXmlHandler::endElement(const QString &namespaceURI, const QString &localName, const QString &qName)
{
	if ( !isEntry )
		return true;

	if ( qName == "entry" ) {
#ifdef QMC2_DEBUG
		printf("end of entry\n");
#endif
		QSize size(VIDEOITEM_IMAGE_WIDTH, VIDEOITEM_IMAGE_HEIGHT + 4);
		QListWidgetItem *listWidgetItem = new QListWidgetItem(listWidget);
		listWidgetItem->setSizeHint(size);
		VideoItemWidget *videoItemWidget = new VideoItemWidget(id, title, author, VIDEOITEM_TYPE_YOUTUBE_SEARCH, videoPlayer, videoPlayer);
		listWidget->setItemWidget(listWidgetItem, videoItemWidget);
		isEntry = false;
	} else if (qName == "id" ) {
		id = currentText.remove(0, currentText.lastIndexOf("/") + 1);
#ifdef QMC2_DEBUG
		printf("    id     = '%s'\n", (const char *)id.toLatin1());
#endif
	} else if ( qName == "title" ) {
		title = currentText;
#ifdef QMC2_DEBUG
		printf("    title  = '%s'\n", (const char *)title.toLatin1());
#endif
	} else if ( qName == "author" ) {
		author = currentText.left(currentText.indexOf("http://"));
#ifdef QMC2_DEBUG
		printf("    author = '%s'\n", (const char *)author.toLatin1());
#endif
	}

	return true;
}

bool YouTubeXmlHandler::characters(const QString &chars)
{
	currentText += QString::fromUtf8(chars.toAscii());

	return true;
}

bool YouTubeXmlHandler::fatalError(const QXmlParseException &exception)
{
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QObject::tr("video player: XML error: fatal error on line %1, column %2: %3").arg(exception.lineNumber()).arg(exception.columnNumber()).arg(exception.message()));

	return false;
}
#endif
