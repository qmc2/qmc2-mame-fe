#ifndef ROMSTATEFILTER_H
#define ROMSTATEFILTER_H

#include <QAction>

#include "ui_romstatefilter.h"

class RomStateFilter : public QWidget, public Ui::RomStateFilter
{
	Q_OBJECT

       	public:
		RomStateFilter(QWidget *parent = 0);
		~RomStateFilter();

		QAction *actionShowCorrect;
		QAction *actionShowMostlyCorrect;
		QAction *actionShowIncorrect;
		QAction *actionShowNotFound;
		QAction *actionShowUnknown;

	public slots:
		void adjustIconSizes();
		void on_toolButtonCorrect_toggled(bool);
		void on_toolButtonMostlyCorrect_toggled(bool);
		void on_toolButtonIncorrect_toggled(bool);
		void on_toolButtonNotFound_toggled(bool);
		void on_toolButtonUnknown_toggled(bool);

	protected:
		void showEvent(QShowEvent *);
		void enterEvent(QEvent *);
		void leaveEvent(QEvent *);
};

#endif
