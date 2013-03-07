#include "colorwidget.h"
#include "qmc2main.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;

ColorWidget::ColorWidget(QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);

#if !defined(QMC2_WIP_ENABLED)
	toolButtonBrush->setVisible(false);
#endif
}

ColorWidget::~ColorWidget()
{
}

void ColorWidget::on_toolButtonColor_clicked()
{
}

void ColorWidget::on_toolButtonBrush_clicked()
{
}
