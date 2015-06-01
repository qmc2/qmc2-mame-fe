#ifndef _MOVIERECORDERSETUP_H_
#define _MOVIERECORDERSETUP_H_

#include "ui_movierecordersetup.h"

class MovieRecorderSetup : public QDialog, public Ui::MovieRecorderSetup
{
	Q_OBJECT

       	public:
		MovieRecorderSetup(QWidget *parent = 0);

	public slots:

	signals:
};

#endif
