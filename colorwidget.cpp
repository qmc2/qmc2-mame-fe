#include <QSettings>
#include "colorwidget.h"
#include "qmc2main.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;
extern QSettings *qmc2Config;

ColorWidget::ColorWidget(QPalette::ColorGroup group, QPalette::ColorRole role, QColor color, QBrush brush, QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);

	colorGroup = group;
	colorRole = role;
	activeColor = color;
	activeBrush = brush;

#if !defined(QMC2_WIP_ENABLED)
	toolButtonBrush->setVisible(false);
#endif

	frameBrush->setAutoFillBackground(true);
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
	QPalette pal = frameBrush->palette();
	pal.setColor(frameBrush->backgroundRole(), activeColor);
	pal.setBrush(frameBrush->backgroundRole(), activeBrush);
	frameBrush->setPalette(pal);
	adjustIconSizes();
	QWidget::showEvent(e);
}
