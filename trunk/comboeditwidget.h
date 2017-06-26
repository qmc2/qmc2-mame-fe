#ifndef COMBOEDITWIDGET_H
#define COMBOEDITWIDGET_H

#include "ui_comboeditwidget.h"

class ComboBoxEditWidget : public QWidget, public Ui::ComboBoxEditWidget
{
	Q_OBJECT

	public:
		ComboBoxEditWidget(QStringList, QString, QWidget *parent = 0);

	public slots:
    		void lineEditValue_textChanged(const QString &) { emit dataChanged(this); }
		void on_comboBoxValue_activated(int);

	signals:
		void dataChanged(QWidget *);
};

#endif
