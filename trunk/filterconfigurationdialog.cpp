#include <QSize>
#include <QFont>
#include <QFontMetrics>

#include "filterconfigurationdialog.h"
#include "settings.h"
#include "macros.h"

extern Settings *qmc2Config;

FilterConfigurationDialog::FilterConfigurationDialog(QWidget *parent) :
	QDialog(parent)
{
	setVisible(false);
	setupUi(this);
}

void FilterConfigurationDialog::adjustIconSizes()
{
	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFontMetrics fm(f);
	QSize iconSize(fm.height() - 2, fm.height() - 2);
	toolButtonClearFilterExpression->setIconSize(iconSize);
}

void FilterConfigurationDialog::showEvent(QShowEvent *e)
{
	adjustIconSizes();
	restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/FilterConfigurationDialog/Geometry", QByteArray()).toByteArray());
	if ( e )
		QDialog::showEvent(e);
}

void FilterConfigurationDialog::hideEvent(QHideEvent *e)
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/FilterConfigurationDialog/Geometry", saveGeometry());
	if ( e )
		QDialog::hideEvent(e);
}
