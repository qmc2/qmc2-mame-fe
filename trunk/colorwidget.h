#ifndef _COLORWIDGET_H_
#define _COLORWIDGET_H_

#include <QPalette>
#include <QString>
#include "ui_colorwidget.h"

class ColorWidget : public QWidget, public Ui::ColorWidget
{
	Q_OBJECT

       	public:
		ColorWidget(QString, QString, QPalette::ColorGroup, QPalette::ColorRole, QColor, QBrush, QWidget *parent = 0);
		~ColorWidget();

		QPalette::ColorGroup colorGroup;
		QPalette::ColorRole colorRole;
		QBrush activeBrush;
		QColor activeColor;
		QString colorName;
		QString groupName;

	public slots:
		void adjustIconSizes();
		void on_toolButtonColor_clicked();
		void on_toolButtonBrush_clicked();

	protected:
		void showEvent(QShowEvent *);

	signals:
		void colorChanged(QPalette::ColorGroup, QPalette::ColorRole, QColor);
		void brushChanged(QPalette::ColorGroup, QPalette::ColorRole, QBrush);
};

#endif
