#include <QApplication>
#include <QLineEdit>
#include "comboeditwidget.h"
#include "macros.h"
#include "qmc2main.h"

extern MainWindow *qmc2MainWindow;

ComboBoxEditWidget::ComboBoxEditWidget(QStringList choices, QString curText, QWidget *parent)
	: QWidget(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ComboBoxEditWidget::ComboBoxEditWidget(QStringList choices = ..., QString curText = %1, QWidget *parent = %2)").arg(curText).arg((qulonglong) parent));
#endif

	setupUi(this);

	comboBoxValue->insertItems(0, choices);
	connect(comboBoxValue, SIGNAL(editTextChanged(const QString &)), this, SLOT(lineEditValue_textChanged(const QString &)));
}

ComboBoxEditWidget::~ComboBoxEditWidget()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ComboBoxEditWidget::~ComboBoxEditWidget()");
#endif

}

void ComboBoxEditWidget::on_comboBoxValue_activated(int index)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ComboBoxEditWidget::on_comboBoxValue_activated(int index = %1)").arg(index));
#endif

	QString userData = comboBoxValue->itemData(index, Qt::UserRole).toString();
	if ( !userData.isEmpty() )
		comboBoxValue->lineEdit()->setText(userData);
}
