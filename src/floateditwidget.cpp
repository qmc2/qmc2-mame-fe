#include "floateditwidget.h"

FloatEditWidget::FloatEditWidget(int nValues, QString sep, QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);

	numValues = nValues;

	if ( numValues < 3 ) {
		doubleSpinBox2->hide();
		labelSeparator1->hide();
	} else
		labelSeparator1->setText(sep);

	if ( numValues < 2 ) {
		doubleSpinBox1->hide();
		labelSeparator0->hide();
	} else
		labelSeparator0->setText(sep);
}