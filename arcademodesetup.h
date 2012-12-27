#ifndef _ARCADEMODESETUP_H_
#define _ARCADEMODESETUP_H_

#include "ui_arcademodesetup.h"

class ArcadeModeSetup : public QDialog, public Ui::ArcadeModeSetup
{
	Q_OBJECT

       	public:
		ArcadeModeSetup(QWidget *parent = 0);
		~ArcadeModeSetup();

	public slots:
		void adjustIconSizes();
};

#endif
