#ifndef _YOUTUBEVIDEOPLAYER_H_
#define _YOUTUBEVIDEOPLAYER_H_

#include "ui_youtubevideoplayer.h"

class YouTubeVideoPlayer : public QWidget, public Ui::YouTubeVideoPlayer
{
	Q_OBJECT

	public:
		YouTubeVideoPlayer(QWidget *parent = 0);
		~YouTubeVideoPlayer();
};

#endif
