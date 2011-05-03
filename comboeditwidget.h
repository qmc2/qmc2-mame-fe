#ifndef _COMBOEDITWIDGET_H_
#define _COMBOEDITWIDGET_H_

#include "ui_comboeditwidget.h"

class ComboBoxEditWidget : public QWidget, public Ui::ComboBoxEditWidget
{
	Q_OBJECT

	public:
		ComboBoxEditWidget(QStringList, QString, QWidget *parent = 0);
		~ComboBoxEditWidget();

	public slots:

	signals:
		void dataChanged(QWidget *);
};

#endif
