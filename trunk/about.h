#ifndef ABOUT_H
#define ABOUT_H

#include "ui_about.h"
#include "macros.h"

class About : public QDialog, public Ui::About
{
	Q_OBJECT

	public:
		QPoint widgetPos;
		QSize widgetSize;
		bool widgetPosValid;
		bool ignoreResizeAndMove;
#if defined(QMC2_OS_MAC)
		QString macVersion;
#elif defined(QMC2_OS_WIN)
		QString winVersion;
#endif

		About(QWidget *parent = 0);

	protected:
		void showEvent(QShowEvent *);
		void moveEvent(QMoveEvent *);
		void resizeEvent(QResizeEvent *);
};

#endif
