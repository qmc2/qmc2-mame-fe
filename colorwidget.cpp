#include <QColorDialog>

#include "settings.h"
#include "colorwidget.h"
#include "brusheditor.h"
#include "qmc2main.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;

ColorWidget::ColorWidget(QString gname, QString cname, QPalette::ColorGroup group, QPalette::ColorRole role, QColor color, QBrush brush, QWidget *parent, bool showBrushButton, bool simpleTxt)
	: QWidget(parent)
{
	setupUi(this);

	colorGroup = group;
	colorRole = role;
	activeColor = color;
	activeBrush = brush;
	colorName = cname;
	groupName = gname;
	simpleText = simpleTxt;

#if !defined(QMC2_WIP_ENABLED)
	toolButtonBrush->setVisible(false);
#else
	toolButtonBrush->setVisible(showBrushButton);
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
	QColor color = QColorDialog::getColor(activeColor, this, simpleText ? tr("Choose color") : tr("Choose color for %1 / %2").arg(colorName).arg(groupName), QColorDialog::DontUseNativeDialog | QColorDialog::ShowAlphaChannel);
	if ( color.isValid() ) {
		activeColor = color;
		QPalette pal = frameBrush->palette();
		pal.setColor(frameBrush->backgroundRole(), activeColor);
		frameBrush->setPalette(pal);
		frameBrush->update();
		emit colorChanged(colorGroup, colorRole, activeColor);
	}
}

void ColorWidget::on_toolButtonBrush_clicked()
{
	// FIXME
	BrushEditor brushEditor(this);
	brushEditor.setWindowTitle(simpleText ? tr("Edit brush") : tr("Edit brush for %1 / %2").arg(colorName).arg(groupName));
	brushEditor.exec();
}

void ColorWidget::showEvent(QShowEvent *e)
{
	QPalette pal = frameBrush->palette();
	pal.setColor(frameBrush->backgroundRole(), activeColor);
	// pal.setBrush(frameBrush->backgroundRole(), activeBrush);
	frameBrush->setPalette(pal);
	adjustIconSizes();

	if ( e )
		QWidget::showEvent(e);
}
