#include "floateditwidget.h"

#ifdef QMC2_DEBUG
#include "qmc2main.h"
#include "macros.h"
extern MainWindow *qmc2MainWindow;
#endif

FloatEditWidget::FloatEditWidget(int nValues, QString sep, QWidget *parent)
	: QWidget(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: FloatEditWidget::FloatEditWidget(int nValues = %1, QString sep = %2, QWidget *parent = %3)").arg(nValues).arg(sep).arg((qulonglong) parent));
#endif

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

FloatEditWidget::~FloatEditWidget()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: FloatEditWidget::~FloatEditWidget()");
#endif

}
