#ifndef _SOFTWARESTATEFILTER_H_
#define _SOFTWARESTATEFILTER_H_

#include "ui_softwarestatefilter.h"

class SoftwareStateFilter : public QWidget, public Ui::SoftwareStateFilter
{
	Q_OBJECT

       	public:
		SoftwareStateFilter(QWidget *parent = 0);
		~SoftwareStateFilter();

	public slots:
		void adjustIconSizes();
		void on_checkBoxStateFilter_stateChanged(int);
		void on_toolButtonCorrect_toggled(bool);
		void on_toolButtonMostlyCorrect_toggled(bool);
		void on_toolButtonIncorrect_toggled(bool);
		void on_toolButtonNotFound_toggled(bool);
		void on_toolButtonUnknown_toggled(bool);

	protected:
		void showEvent(QShowEvent *);
};

#endif
