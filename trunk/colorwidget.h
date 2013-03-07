#ifndef _COLORWIDGET_H_
#define _COLORWIDGET_H_

#include "ui_colorwidget.h"

class ColorWidget : public QWidget, public Ui::ColorWidget
{
	Q_OBJECT

       	public:
		ColorWidget(QWidget *parent = 0);
		~ColorWidget();

	public slots:
		void on_toolButtonColor_clicked();
		void on_toolButtonBrush_clicked();
};

#endif
