#ifndef MOVIERECORDERSETUP_H
#define MOVIERECORDERSETUP_H

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
