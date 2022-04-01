#include "settings.h"
#include "gradientstopactions.h"
#include "macros.h"

extern Settings *qmc2Config;

GradientStopActions::GradientStopActions(QTreeWidgetItem *item, QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);
	parentItem = item;
	adjustIconSizes();
}

GradientStopActions::~GradientStopActions()
{
}

void GradientStopActions::adjustIconSizes()
{
	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFontMetrics fm(f);
	QSize iconSize = QSize(fm.height() - 2, fm.height() - 2);

	toolButtonRemove->setIconSize(iconSize);
	toolButtonUp->setIconSize(iconSize);
	toolButtonDown->setIconSize(iconSize);
}

void GradientStopActions::on_toolButtonRemove_clicked()
{
	emit removeRequested(parentItem);
}

void GradientStopActions::on_toolButtonUp_clicked()
{
	emit upRequested(parentItem);
}

void GradientStopActions::on_toolButtonDown_clicked()
{
	emit downRequested(parentItem);
}
