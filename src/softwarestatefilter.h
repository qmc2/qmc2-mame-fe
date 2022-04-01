#ifndef SOFTWARESTATEFILTER_H
#define SOFTWARESTATEFILTER_H

#include "ui_softwarestatefilter.h"

class SoftwareStateFilter : public QWidget, public Ui::SoftwareStateFilter
{
	Q_OBJECT

       	public:
		SoftwareStateFilter(QWidget *parent = 0);
		~SoftwareStateFilter();

		void filter();

		bool isReady;

	public slots:
		void adjustIconSizes();
		void on_checkBoxStateFilter_toggled(bool);
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
