#include <QApplication>
#include <QScrollBar>
#include <QString>
#include <QFont>
#include <QTime>

#include "checksumscannerlog.h"
#include "settings.h"
#include "macros.h"

extern Settings *qmc2Config;

CheckSumScannerLog::CheckSumScannerLog(QWidget *parent)
	: QWidget(parent)
{
	hide();
	setupUi(this);
	QFont logFont;
	logFont.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/LogFont").toString());
	plainTextEditLog->setFont(logFont);
	spinBoxMaxLogSize->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + "CheckSumScannerLog/MaxLogSize", 10000).toInt());
}

void CheckSumScannerLog::on_spinBoxMaxLogSize_valueChanged(int value)
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "CheckSumScannerLog/MaxLogSize", value);
	plainTextEditLog->setMaximumBlockCount(value);
}

void CheckSumScannerLog::log(const QString &message)
{
	plainTextEditLog->appendPlainText(QTime::currentTime().toString("hh:mm:ss.zzz") + ": " + message);
}

void CheckSumScannerLog::scrollToEnd()
{
	plainTextEditLog->horizontalScrollBar()->setValue(plainTextEditLog->horizontalScrollBar()->minimum());
	plainTextEditLog->verticalScrollBar()->setValue(plainTextEditLog->verticalScrollBar()->maximum());
}

void CheckSumScannerLog::showEvent(QShowEvent *e)
{
	restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/CheckSumScannerLog/Geometry", QByteArray()).toByteArray());
	emit windowOpened();
	if ( e )
		QWidget::showEvent(e);
}

void CheckSumScannerLog::hideEvent(QHideEvent *e)
{
	if ( isVisible() )
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/CheckSumScannerLog/Geometry", saveGeometry());
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
