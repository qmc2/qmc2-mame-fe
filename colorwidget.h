#ifndef _COLORWIDGET_H_
#define _COLORWIDGET_H_

#include <QPalette>
#include <QSize>
#include <QString>
#include <QChar>
#include <QFontMetrics>
#include "ui_colorwidget.h"

class ColorWidget : public QWidget, public Ui::ColorWidget
{
	Q_OBJECT

       	public:
		ColorWidget(QString, QString, QPalette::ColorGroup, QPalette::ColorRole, QColor, QBrush, QWidget *parent = 0, bool showBrushButton = true, bool simpleTxt = false);

		QPalette::ColorGroup colorGroup;
		QPalette::ColorRole colorRole;
		QBrush activeBrush;
		QColor activeColor;
		QString colorName;
		QString groupName;
		bool simpleText;

		virtual QSize sizeHint() const
		{
			QFontMetrics fm(font());
			return QSize(50, fm.height() + 6);
		}

		QString argbValue()
		{
			int r, g, b, a;
			activeColor.getRgb(&r, &g, &b, &a);
			return QString("%1%2%3%4").arg(a, 2, 16, QChar('0')).arg(r, 2, 16, QChar('0')).arg(g, 2, 16, QChar('0')).arg(b, 2, 16, QChar('0'));
		}

		void setArgb(QString argb)
		{
			int a = argb.mid(0, 2).toInt(0, 16);
			int r = argb.mid(2, 2).toInt(0, 16);
			int g = argb.mid(4, 2).toInt(0, 16);
			int b = argb.mid(6, 2).toInt(0, 16);
			activeColor.setRgb(r, g, b, a);
		}

	public slots:
		void adjustIconSizes();
		void updateColor();
		void on_toolButtonColor_clicked();
		void on_toolButtonBrush_clicked();

	protected:
		void showEvent(QShowEvent *);

	signals:
		void colorChanged(QPalette::ColorGroup, QPalette::ColorRole, QColor);
		void brushChanged(QPalette::ColorGroup, QPalette::ColorRole, QBrush);
		void dataChanged();
};

#endif
