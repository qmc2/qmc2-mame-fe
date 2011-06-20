#ifndef _FLOATEDITWIDGET_H_
#define _FLOATEDITWIDGET_H_

#include "ui_floateditwidget.h"

class FloatEditWidget : public QWidget, public Ui::FloatEditWidget
{
	Q_OBJECT

	public:
		int numValues;

		FloatEditWidget(int nValues = 3, QWidget *parent = 0);
		~FloatEditWidget();

	public slots:
		void on_soubleSpinBox0_valueChanged(double) { emit dataChanged(this); }
		void on_soubleSpinBox1_valueChanged(double) { emit dataChanged(this); }
		void on_soubleSpinBox2_valueChanged(double) { emit dataChanged(this); }

	signals:
		void dataChanged(QWidget *);
};

#endif
