#include "floateditwidget.h"

#ifdef QMC2_DEBUG
#include "qmc2main.h"
#include "macros.h"
extern MainWindow *qmc2MainWindow;
#endif

FloatEditWidget::FloatEditWidget(int nValues, QWidget *parent)
	: QWidget(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: FloatEditWidget::FloatEditWidget(int nValues = %1, QWidget *parent = %2)").arg(nValues).arg((qulonglong) parent));
#endif

	setupUi(this);

	numValues = nValues;

	if ( numValues < 3 )
		doubleSpinBox2->hide();

	if ( numValues < 2 )
		doubleSpinBox1->hide();
}

FloatEditWidget::~FloatEditWidget()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: FloatEditWidget::~FloatEditWidget()");
#endif

}
