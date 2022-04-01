#include <QApplication>
#include <QLineEdit>
#include "comboeditwidget.h"
#include "macros.h"
#include "qmc2main.h"

extern MainWindow *qmc2MainWindow;

ComboBoxEditWidget::ComboBoxEditWidget(QStringList choices, QString curText, QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);

	comboBoxValue->insertItems(0, choices);
	connect(comboBoxValue, SIGNAL(editTextChanged(const QString &)), this, SLOT(lineEditValue_textChanged(const QString &)));
}

void ComboBoxEditWidget::on_comboBoxValue_activated(int index)
{
	QString userData = comboBoxValue->itemData(index, Qt::UserRole).toString();
	if ( !userData.isEmpty() )
		comboBoxValue->lineEdit()->setText(userData);
}
