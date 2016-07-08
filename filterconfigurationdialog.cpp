#include <QSize>
#include <QFont>
#include <QFontMetrics>

#include "filterconfigurationdialog.h"
#include "machinelistviewer.h"
#include "settings.h"
#include "macros.h"

extern Settings *qmc2Config;

FilterConfigurationDialog::FilterConfigurationDialog(MachineListViewer *viewer, QWidget *parent) :
	QDialog(parent),
	m_viewer(viewer)
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

void FilterConfigurationDialog::on_pushButtonOk_clicked()
{
	on_pushButtonApply_clicked();
}

void FilterConfigurationDialog::on_pushButtonApply_clicked()
{
	// FIXME
	viewer()->toolButtonUpdateView->animateClick();
}

void FilterConfigurationDialog::on_pushButtonCancel_clicked()
{
	// FIXME
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
