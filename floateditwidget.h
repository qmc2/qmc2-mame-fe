#ifndef _FLOATEDITWIDGET_H_
#define _FLOATEDITWIDGET_H_

#include "ui_floateditwidget.h"

class FloatEditWidget : public QWidget, public Ui::FloatEditWidget
{
	Q_OBJECT

	public:
		int numValues;

		FloatEditWidget(int nValues = 3, QString sep = ",", QWidget *parent = 0);

	public slots:
		void on_doubleSpinBox0_valueChanged(double) { emit dataChanged(this); }
		void on_doubleSpinBox1_valueChanged(double) { emit dataChanged(this); }
		void on_doubleSpinBox2_valueChanged(double) { emit dataChanged(this); }

	signals:
		void dataChanged(QWidget *);
};

#endif
