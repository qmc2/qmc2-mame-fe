#include <QFont>

#include "checksumscannerlog.h"
#include "settings.h"
#include "macros.h"

extern Settings *qmc2Config;

CheckSumScannerLog::CheckSumScannerLog(QWidget *parent)
	: QWidget(parent, Qt::Tool)
{
	hide();
	setupUi(this);

	QFont logFont;
	logFont.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/LogFont").toString());
	plainTextEditLog->setFont(logFont);
}

void CheckSumScannerLog::on_spinBoxMaxLogSize_valueChanged(int value)
{
	plainTextEditLog->setMaximumBlockCount(value);
}

void CheckSumScannerLog::showEvent(QShowEvent *e)
{
	restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/CheckSumScannerLog/Geometry", QByteArray()).toByteArray());
	spinBoxMaxLogSize->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + "CheckSumScannerLog/MaxLogSize", 0).toInt());
	emit windowOpened();
	if ( e )
		QWidget::showEvent(e);
}

void CheckSumScannerLog::hideEvent(QHideEvent *e)
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/CheckSumScannerLog/Geometry", saveGeometry());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "CheckSumScannerLog/MaxLogSize", spinBoxMaxLogSize->value());
	emit windowClosed();
	if ( e )
		QWidget::hideEvent(e);
}

void CheckSumScannerLog::closeEvent(QCloseEvent *e)
{
	hideEvent(0);
	QWidget::closeEvent(e);
}

void CheckSumScannerLog::keyPressEvent(QKeyEvent *e)
{
	if ( e->key() == Qt::Key_Escape )
		close();
	else
		QWidget::keyPressEvent(e);
}
