#if defined(QMC2_YOUTUBE_ENABLED)

#include <QtTest>
#include <QSettings>
#include <QClipboard>
#include <QInputDialog>

#include "macros.h"
#include "qmc2main.h"
#include "youtubevideoplayer.h"
#include "videoitemwidget.h"

#define QMC2_DEBUG

extern MainWindow *qmc2MainWindow;
extern QNetworkAccessManager *qmc2NetworkAccessManager;
extern QSettings *qmc2Config;

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

	// serious hack to access the volume slider's tool button object
	privateMuteButton = volumeSlider->findChild<QToolButton *>();
	if ( privateMuteButton ) {
		privateMuteButton->setCheckable(true);
		privateMuteButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		privateMuteButton->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/AudioMuted", false).toBool());
		privateMuteButton->setToolTip(tr("Mute / unmute audio output"));
		videoPlayer->audioOutput()->setMuted(privateMuteButton->isChecked());
	}

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

	loadOnly = isMuted = pausedByHideEvent = viError = viFinished = false;
	videoInfoReply = NULL;
	videoInfoManager = NULL;
	currentFormat = bestAvailableFormat = YOUTUBE_FORMAT_UNKNOWN_INDEX;

	videoPlayer->mediaObject()->setTickInterval(1000);
	volumeSlider->setAudioOutput(videoPlayer->audioOutput());
	seekSlider->setMediaObject(videoPlayer->mediaObject());
	seekSlider->setToolTip(tr("Video progress"));
	connect(videoPlayer->mediaObject(), SIGNAL(tick(qint64)), this, SLOT(videoTick(qint64)));
	connect(videoPlayer->mediaObject(), SIGNAL(stateChanged(Phonon::State, Phonon::State)), this, SLOT(videoStateChanged(Phonon::State, Phonon::State)));
	connect(videoPlayer->mediaObject(), SIGNAL(bufferStatus(int)), this, SLOT(videoBufferStatus(int)));
	connect(videoPlayer, SIGNAL(finished()), this, SLOT(videoFinished()));

	adjustIconSizes();

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
	menuAttachedVideos->addSeparator();
	s = tr("Copy YouTube URL (standard)");
	action = menuAttachedVideos->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/youtube.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(copyYouTubeUrl()));
	s = tr("Copy alternate YouTube URL (no country filter)");
	action = menuAttachedVideos->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/youtube.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(copyAlternateYouTubeUrl()));
	menuAttachedVideos->addSeparator();
	s = tr("Remove selected videos");
	action = menuAttachedVideos->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/remove.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(removeSelectedVideos()));

	menuVideoPlayer = new QMenu(0);
	s = tr("Start / pause / resume video playback");
	action = menuVideoPlayer->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/media_stop.png")));
	videoMenuPlayPauseAction = action;
	videoMenuPlayPauseAction->setEnabled(false);
	connect(action, SIGNAL(triggered()), this, SLOT(on_toolButtonPlayPause_clicked()));
	menuVideoPlayer->addSeparator();
	s = tr("Copy YouTube URL (standard)");
	action = menuVideoPlayer->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/youtube.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(copyCurrentYouTubeUrl()));
	s = tr("Copy alternate YouTube URL (no country filter)");
	action = menuVideoPlayer->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/youtube.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(copyCurrentAlternateYouTubeUrl()));

	menuSearchResults = NULL;

	menuSuggestButton = new QMenu(0);
	s = tr("Auto-suggest");
	action = menuSuggestButton->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setCheckable(true);
	action->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "YouTubeWidget/AutoSuggest", true).toBool());
	autoSuggestAction = action;
	s = tr("Append...");
	action = menuSuggestButton->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	connect(action, SIGNAL(triggered()), this, SLOT(setSuggestorAppendString()));
	toolButtonSuggest->setMenu(menuSuggestButton);
	if ( autoSuggestAction->isChecked() )
		on_toolButtonSuggest_clicked();

	QTimer::singleShot(0, this, SLOT(init()));
}

YouTubeVideoPlayer::~YouTubeVideoPlayer()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::~YouTubeVideoPlayer()");
#endif

  	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "YouTubeWidget/PreferredFormat", comboBoxPreferredFormat->currentIndex());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "YouTubeWidget/AudioVolume", videoPlayer->audioOutput()->volume());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "YouTubeWidget/PageIndex", toolBox->currentIndex());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "YouTubeWidget/PlayOMatic/Enabled", checkBoxPlayOMatic->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "YouTubeWidget/PlayOMatic/Mode", comboBoxMode->currentIndex());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "YouTubeWidget/PlayOMatic/Repeat", checkBoxRepeat->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "YouTubeWidget/AutoSuggest", autoSuggestAction->isChecked());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "YouTubeWidget/SuggestorAppendString", suggestorAppendString);

	// serious hack to access the volume slider's tool button object
	privateMuteButton = volumeSlider->findChild<QToolButton *>();
	if ( privateMuteButton )
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "YouTubeWidget/AudioMuted", privateMuteButton->isChecked());

	// clean up
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
	if ( videoPlayer ) {
		disconnect(videoPlayer);
		delete videoPlayer;
	}
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

	currentVideoID.clear();
	videoPlayer->play(Phonon::MediaSource(QString::fromUtf8(":/data/img/ghost_video.png")));
	videoPlayer->stop();
}

void YouTubeVideoPlayer::playAttachedVideo()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::playAttachedVideo()");
#endif

	QListWidgetItem *item = listWidgetAttachedVideos->currentItem();
	if ( item )
		on_listWidgetAttachedVideos_itemActivated(item);
}

void YouTubeVideoPlayer::copyYouTubeUrl()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::copyYouTubeUrl()");
#endif

	QListWidgetItem *item = listWidgetAttachedVideos->currentItem();
	if ( item ) {
		VideoItemWidget *itemWidget = (VideoItemWidget *)listWidgetAttachedVideos->itemWidget(item);
		if ( itemWidget ) {
			if ( !itemWidget->videoID.isEmpty() ) {
				if ( !itemWidget->videoUrlPattern.isEmpty() ) {
					QString url = VIDEOITEM_YOUTUBE_URL_PATTERN;
					url.replace("$VIDEO_ID$", itemWidget->videoID);
					qApp->clipboard()->setText(url);
				}
			}
		}
	}
}

void YouTubeVideoPlayer::copyAlternateYouTubeUrl()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::copyAlternateYouTubeUrl()");
#endif

	QListWidgetItem *item = listWidgetAttachedVideos->currentItem();
	if ( item ) {
		VideoItemWidget *itemWidget = (VideoItemWidget *)listWidgetAttachedVideos->itemWidget(item);
		if ( itemWidget ) {
			if ( !itemWidget->videoID.isEmpty() ) {
				if ( !itemWidget->videoUrlPattern.isEmpty() ) {
					QString url = VIDEOITEM_YOUTUBE_URL_PATTERN_NO_COUNTRY_FILTER;
					url.replace("$VIDEO_ID$", itemWidget->videoID);
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

void YouTubeVideoPlayer::copyCurrentAlternateYouTubeUrl()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::copyCurrentYouTubeUrl()");
#endif

	if ( !currentVideoID.isEmpty() ) {
		QString url = VIDEOITEM_YOUTUBE_URL_PATTERN_NO_COUNTRY_FILTER;
		url.replace("$VIDEO_ID$", currentVideoID);
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
		QListWidgetItem *i = listWidgetAttachedVideos->takeItem(listWidgetAttachedVideos->row(item));
		delete i;
	}
}

void YouTubeVideoPlayer::init()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::init()");
#endif

	VideoItemWidget *videoItemWidget;
	QListWidgetItem *listWidgetItem;
	QSize size(VIDEOITEM_IMAGE_WIDTH, VIDEOITEM_IMAGE_HEIGHT + 4);

	listWidgetItem = new QListWidgetItem(listWidgetAttachedVideos);
	listWidgetItem->setSizeHint(size);
	videoItemWidget = new VideoItemWidget("bFjX1uUhB1A", "<p><b>Joe Satriani - The Mystical Potato Head Groove Thing (G3 LIVE in Denver 2003)</b></p>", VIDEOITEM_TYPE_YOUTUBE, this);
	listWidgetAttachedVideos->setItemWidget(listWidgetItem, videoItemWidget);

	listWidgetItem = new QListWidgetItem(listWidgetAttachedVideos);
	listWidgetItem->setSizeHint(size);
	videoItemWidget = new VideoItemWidget("bcwBowBFFzc", "<p><b>Frogger (Arcade) Demo</b></p>", VIDEOITEM_TYPE_YOUTUBE, this);
	listWidgetAttachedVideos->setItemWidget(listWidgetItem, videoItemWidget);

	listWidgetItem = new QListWidgetItem(listWidgetAttachedVideos);
	listWidgetItem->setSizeHint(size);
	videoItemWidget = new VideoItemWidget("xkTasNER4vI", "<p><b>Arcade Longplay [119] Bionic Commando</b></p>", VIDEOITEM_TYPE_YOUTUBE, this);
	listWidgetAttachedVideos->setItemWidget(listWidgetItem, videoItemWidget);

	listWidgetItem = new QListWidgetItem(listWidgetAttachedVideos);
	listWidgetItem->setSizeHint(size);
	videoItemWidget = new VideoItemWidget("vK9rfCpjOQc", "<p><b>Orianthi</b></p>", VIDEOITEM_TYPE_YOUTUBE, this);
	listWidgetAttachedVideos->setItemWidget(listWidgetItem, videoItemWidget);

	listWidgetItem = new QListWidgetItem(listWidgetAttachedVideos);
	listWidgetItem->setSizeHint(size);
	videoItemWidget = new VideoItemWidget("gO-OwcBCa8Y", "<p><b>Orianthi HD</b></p>", VIDEOITEM_TYPE_YOUTUBE, this);
	listWidgetAttachedVideos->setItemWidget(listWidgetItem, videoItemWidget);

	listWidgetItem = new QListWidgetItem(listWidgetAttachedVideos);
	listWidgetItem->setSizeHint(size);
	videoItemWidget = new VideoItemWidget("XCv5aPqlDd4", "<p><b>world.avi</b></p>", VIDEOITEM_TYPE_YOUTUBE, this);
	listWidgetAttachedVideos->setItemWidget(listWidgetItem, videoItemWidget);

	listWidgetItem = new QListWidgetItem(listWidgetAttachedVideos);
	listWidgetItem->setSizeHint(size);
	videoItemWidget = new VideoItemWidget("PZxfFxYY7JM", "<p><b>Golden Globe</b></p>", VIDEOITEM_TYPE_YOUTUBE, this);
	listWidgetAttachedVideos->setItemWidget(listWidgetItem, videoItemWidget);

	listWidgetItem = new QListWidgetItem(listWidgetAttachedVideos);
	listWidgetItem->setSizeHint(size);
	videoItemWidget = new VideoItemWidget("X6Qvpz5d18g", "<p><b>Earth 3D.wmv</b></p>", VIDEOITEM_TYPE_YOUTUBE, this);
	listWidgetAttachedVideos->setItemWidget(listWidgetItem, videoItemWidget);

	listWidgetItem = new QListWidgetItem(listWidgetAttachedVideos);
	listWidgetItem->setSizeHint(size);
	videoItemWidget = new VideoItemWidget("xdg5wBd9r_U", "<p><b>Realistic 3d Earth using only AE</b></p>", VIDEOITEM_TYPE_YOUTUBE, this);
	listWidgetAttachedVideos->setItemWidget(listWidgetItem, videoItemWidget);

	listWidgetItem = new QListWidgetItem(listWidgetAttachedVideos);
	listWidgetItem->setSizeHint(size);
	videoItemWidget = new VideoItemWidget("SyhO0Ypukfc", "<p><b>Universo 3D</b></p>", VIDEOITEM_TYPE_YOUTUBE, this);
	listWidgetAttachedVideos->setItemWidget(listWidgetItem, videoItemWidget);

	listWidgetItem = new QListWidgetItem(listWidgetAttachedVideos);
	listWidgetItem->setSizeHint(size);
	videoItemWidget = new VideoItemWidget("9XyR8buBfNk", "<p><b>RayStorm - first stage</b></p>", VIDEOITEM_TYPE_YOUTUBE, this);
	listWidgetAttachedVideos->setItemWidget(listWidgetItem, videoItemWidget);

	listWidgetItem = new QListWidgetItem(listWidgetAttachedVideos);
	listWidgetItem->setSizeHint(size);
	videoItemWidget = new VideoItemWidget("3qD453R7usI", "<p><b>The Block Kuzushi</b></p>", VIDEOITEM_TYPE_YOUTUBE, this);
	listWidgetAttachedVideos->setItemWidget(listWidgetItem, videoItemWidget);

	listWidgetAttachedVideos->updateGeometry();

	QTimer::singleShot(100, this, SLOT(loadNullVideo()));

	/*
	if ( checkBoxPlayOMatic->isChecked() )
		playVideo(video);
	else
		loadVideo(video);
		*/

	/*
	if ( listWidgetAttachedVideos->count() < 1 )
		QTimer::singleShot(100, this, SLOT(loadNullVideo()));
	*/
}

void YouTubeVideoPlayer::adjustIconSizes()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::adjustIconSizes()"));
#endif

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
		case Phonon::BufferingState:
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
			// serious hack to access the seekSlider's slider object
			privateSeekSlider = seekSlider->findChild<QSlider *>();
			if ( privateSeekSlider )
				privateSeekSlider->setValue(0);
			progressBarBufferStatus->setValue(0);
			progressBarBufferStatus->setToolTip(tr("Current buffer fill level: %1%").arg(0));
			break;
		case Phonon::PlayingState:
			toolButtonPlayPause->setIcon(QIcon(QString::fromUtf8(":/data/img/media_play.png")));
			toolButtonPlayPause->setEnabled(true);
			videoMenuPlayPauseAction->setIcon(QIcon(QString::fromUtf8(":/data/img/media_play.png")));
			videoMenuPlayPauseAction->setEnabled(true);
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
			break;
		case Phonon::ErrorState:
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("video player: playback error: %1").arg(videoPlayer->mediaObject()->errorString()));
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
	}
}

QUrl YouTubeVideoPlayer::getVideoStreamUrl(QString videoID)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::getVideoStreamUrl(QString videoID = %1)").arg(videoID));
#endif

	static QUrl videoUrl;

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
	videoInfoRequest.setRawHeader("User-Agent", "QMC2's YouTube Player");
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
		if ( !timeoutOccurred )
			QTest::qWait(YOUTUBE_VIDEOINFO_WAIT);
	}

	if ( viFinished && !viError ) {
		QStringList videoInfoList = videoInfoBuffer.split("&");
#ifdef QMC2_DEBUG
		printf("\nAvailable formats / URLs for video ID '%s':\n", (const char *)videoID.toLatin1());
#endif
		QMap <QString, QUrl> formatToUrlMap;
		foreach (QString videoInfo, videoInfoList) {
			if ( videoInfo.startsWith("fmt_url_map=") ) {
				QStringList fmtUrlMap = videoInfo.replace("fmt_url_map=", "").split("%2C");
				foreach (QString fmtUrl, fmtUrlMap) {
					QStringList formatAndUrl = fmtUrl.split("%7C");
					if ( formatAndUrl.count() > 1 ) {
						QUrl url = QUrl::fromEncoded(formatAndUrl[1].toAscii());
						QString urlStr = url.toString();
						url.setEncodedUrl(urlStr.toAscii());
#ifdef QMC2_DEBUG
						printf("%s\t%s\n", (const char *)formatAndUrl[0].toLatin1(), (const char *)url.toString().toLatin1());
#endif
						formatToUrlMap[formatAndUrl[0]] = url;
					}
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
				printf("\nSelected format / URL for video ID '%s':\n%s\t%s\n", (const char *)videoID.toLatin1(), (const char *)fmtStr.toLatin1(), (const char *)videoUrl.toString().toLatin1());
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
		case YOUTUBE_ATTACHED_VIDEOS_PAGE:
			break;
		case YOUTUBE_VIDEO_PLAYER_PAGE:
			break;
		case YOUTUBE_SEARCH_VIDEO_PAGE:
			break;
		default:
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

	VideoItemWidget *itemWidget = (VideoItemWidget *)listWidgetAttachedVideos->itemWidget(item);
	if ( itemWidget ) {
		toolBox->setCurrentIndex(YOUTUBE_VIDEO_PLAYER_PAGE);
		if ( currentVideoID == itemWidget->videoID && !videoPlayer->isPlaying() )
			videoPlayer->play();
		else if ( currentVideoID != itemWidget->videoID || !videoPlayer->isPlaying() )
			playVideo(itemWidget->videoID);
	}
}

void YouTubeVideoPlayer::on_listWidgetAttachedVideos_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::on_listWidgetAttachedVideos_customContextMenuRequested(const QPoint &p = [%1, %2])").arg(p.x()).arg(p.y()));
#endif

	QWidget *w = listWidgetAttachedVideos->viewport();
	if ( sender() )
		if ( sender()->objectName() == "QMC2_VIDEO_DESCRIPTION" )
			w = (QWidget *)sender();
	if ( w && menuAttachedVideos ) {
		menuAttachedVideos->move(qmc2MainWindow->adjustedWidgetPosition(w->mapToGlobal(p), menuAttachedVideos));
		menuAttachedVideos->show();
	}
}

void YouTubeVideoPlayer::on_listWidgetSearchResults_customContextMenuRequested(const QPoint &p)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeVideoPlayer::on_listWidgetSearchResults_customContextMenuRequested(const QPoint &p = [%1, %2])").arg(p.x()).arg(p.y()));
#endif

	QWidget *w = listWidgetSearchResults->viewport();
	if ( sender() )
		if ( sender()->objectName() == "QMC2_VIDEO_DESCRIPTION" )
			w = (QWidget *)sender();
	if ( w && menuSearchResults ) {
		menuSearchResults->move(qmc2MainWindow->adjustedWidgetPosition(w->mapToGlobal(p), menuSearchResults));
		menuSearchResults->show();
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
}

void YouTubeVideoPlayer::on_toolButtonSuggest_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::on_toolButtonSuggest_clicked()");
#endif

	QString suggestedSearchPattern = mySetName;
	suggestedSearchPattern = suggestedSearchPattern.replace(QRegExp("\\(.*\\)"), "").simplified();
	if ( !suggestorAppendString.isEmpty() )
		suggestedSearchPattern.append(" " + suggestorAppendString);
	lineEditSearchString->setText(suggestedSearchPattern);
}

void YouTubeVideoPlayer::on_toolButtonSearch_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeVideoPlayer::on_toolButtonSearch_clicked()");
#endif

}
#endif
