#ifndef _COLORWIDGET_H_
#define _COLORWIDGET_H_

#include <QPalette>
#include "ui_colorwidget.h"

class ColorWidget : public QWidget, public Ui::ColorWidget
{
	Q_OBJECT

       	public:
		ColorWidget(QPalette::ColorGroup, QPalette::ColorRole, QColor, QBrush, QWidget *parent = 0);
		~ColorWidget();

		QPalette::ColorGroup colorGroup;
		QPalette::ColorRole colorRole;
		QBrush activeBrush;
		QColor activeColor;

	public slots:
		void adjustIconSizes();
		void on_toolButtonColor_clicked();
		void on_toolButtonBrush_clicked();

	protected:
		void showEvent(QShowEvent *);
};

#endif
