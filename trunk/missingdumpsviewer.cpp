#include "missingdumpsviewer.h"
#include "settings.h"
#include "macros.h"

extern Settings *qmc2Config;

MissingDumpsViewer::MissingDumpsViewer(QString settingsKey, QWidget *parent)
#if defined(QMC2_OS_WIN)
	: QDialog(parent, Qt::Dialog)
#else
	: QDialog(parent, Qt::Dialog | Qt::SubWindow)
#endif
{
	m_settingsKey = settingsKey;
	setVisible(false);
	setupUi(this);
}

void MissingDumpsViewer::on_toolButtonExportToDataFile_clicked()
{
	// FIXME
}

void MissingDumpsViewer::showEvent(QShowEvent *e)
{
	restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/" + m_settingsKey + "/Geometry", QByteArray()).toByteArray());
	treeWidget->header()->restoreState(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/" + m_settingsKey + "/HeaderState", QByteArray()).toByteArray());
	checkBoxSelectedDumpsOnly->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SelectedDumpsOnly", false).toBool());
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
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/" + m_settingsKey + "/Geometry", saveGeometry());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/" + m_settingsKey + "/HeaderState", treeWidget->header()->saveState());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/SelectedDumpsOnly", checkBoxSelectedDumpsOnly->isChecked());
	if ( e )
		QDialog::closeEvent(e);
}
