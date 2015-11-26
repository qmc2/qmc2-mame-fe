#ifndef _COMBOBOXWIDGET_H_
#define _COMBOBOXWIDGET_H_

#include <QStringList>
#include <QSize>
#include <QFontMetrics>

#include "ui_comboboxwidget.h"

class ComboBoxWidget : public QWidget, public Ui::ComboBoxWidget
{
	Q_OBJECT

	public:
		ComboBoxWidget(QStringList, QString, QString, QString, QString, QWidget *parent = 0);
		void setLabel(QString);
		void setPushButton(QString, QString);
		void setComboBox(QStringList, QString);
		void clearAll() { setLabel(QString()); setPushButton(QString(), QString()); setComboBox(QStringList(), QString()); }
};

#endif
