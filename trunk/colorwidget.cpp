#include <QSettings>
#include "colorwidget.h"
#include "qmc2main.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;
extern QSettings *qmc2Config;

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

void ColorWidget::adjustIconSizes()
{
	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFontMetrics fm(f);
	QSize iconSize = QSize(fm.height() - 2, fm.height() - 2);

	toolButtonColor->setIconSize(iconSize);
	toolButtonBrush->setIconSize(iconSize);
}

void ColorWidget::on_toolButtonColor_clicked()
{
}

void ColorWidget::on_toolButtonBrush_clicked()
{
}

void ColorWidget::showEvent(QShowEvent *e)
{
	adjustIconSizes();
	QWidget::showEvent(e);
}
