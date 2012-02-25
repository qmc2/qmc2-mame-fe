#ifndef _CUSTOMIDSETUP_H_
#define _CUSTOMIDSETUP_H_

#include <QtGui>
#include "ui_customidsetup.h"

class CustomIDSetup : public QDialog, public Ui::CustomIDSetup
{
	Q_OBJECT

       	public:
		CustomIDSetup(QString, QWidget *parent = 0);
		~CustomIDSetup();

	public slots:
};

#endif
