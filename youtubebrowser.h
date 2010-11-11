#ifndef _YOUTUBEBROWSER_H_
#define _YOUTUBEBROWSER_H_

#include "ui_youtubebrowser.h"

class YouTubeBrowser : public QWidget, public Ui::YouTubeBrowser
{
	Q_OBJECT

	public:
		YouTubeBrowser(QWidget *parent = 0);
		~YouTubeBrowser();
};

#endif
