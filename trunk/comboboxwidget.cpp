#include "comboboxwidget.h"

ComboBoxWidget::ComboBoxWidget(QStringList choices, QString title, QString comboBoxToolTip, QString buttonText, QString buttonToolTip, QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);
	setLabel(title);
	setComboBox(choices, comboBoxToolTip);
	setPushButton(buttonText, buttonToolTip);
}

void ComboBoxWidget::setLabel(QString text)
{
	if ( text.isEmpty() )
		label->hide();
	else {
		label->setText(text);
		label->show();
	}
}

void ComboBoxWidget::setPushButton(QString text, QString toolTip)
{
	if ( text.isEmpty() )
		pushButton->hide();
	else {
		pushButton->setText(text);
		pushButton->setToolTip(toolTip);
		pushButton->show();
	}
}

void ComboBoxWidget::setComboBox(QStringList list, QString toolTip)
{
	comboBox->clear();
	if ( list.isEmpty() )
		comboBox->hide();
	else {
		comboBox->insertItems(0, list);
		comboBox->setToolTip(toolTip);
		comboBox->show();
	}
}
