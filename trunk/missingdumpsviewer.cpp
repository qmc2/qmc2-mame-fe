#include "missingdumpsviewer.h"
#include "settings.h"
#include "macros.h"

extern Settings *qmc2Config;

MissingDumpsViewer::MissingDumpsViewer(QWidget *parent)
#if defined(QMC2_OS_WIN)
	: QDialog(parent, Qt::Dialog)
#else
	: QDialog(parent, Qt::Dialog | Qt::SubWindow)
#endif
{
	setVisible(false);
	setupUi(this);
}

void MissingDumpsViewer::showEvent(QShowEvent *e)
{
	restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MissingDumpsViewer/Geometry", QByteArray()).toByteArray());
	treeWidget->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/MissingDumpsViewer/HeaderState", QByteArray()).toByteArray());
	if ( e )
		QDialog::showEvent(e);
}

void MissingDumpsViewer::hideEvent(QHideEvent *e)
{
	closeEvent(0);
	if ( e )
		QDialog::hideEvent(e);
}

void MissingDumpsViewer::closeEvent(QCloseEvent *e)
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MissingDumpsViewer/Geometry", saveGeometry());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/MissingDumpsViewer/HeaderState", treeWidget->header()->saveState());
	if ( e )
		QDialog::closeEvent(e);
}
