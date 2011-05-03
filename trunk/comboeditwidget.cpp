#include <QApplication>
#include "comboeditwidget.h"

#ifdef QMC2_DEBUG
#include "qmc2main.h"
#include "macros.h"
extern MainWindow *qmc2MainWindow;
#endif

ComboBoxEditWidget::ComboBoxEditWidget(QStringList choices, QString curText, QWidget *parent)
	: QWidget(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ComboBoxEditWidget::ComboBoxEditWidget(QStringList choices = ..., QString curText = %1, QWidget *parent = %2").arg(curText).arg((qulonglong) parent));
#endif

	setupUi(this);

	comboBoxValue->insertItems(0, choices);
}

ComboBoxEditWidget::~ComboBoxEditWidget()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ComboBoxEditWidget::~ComboBoxEditWidget()");
#endif

}
