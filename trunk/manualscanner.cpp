#include <QScrollBar>
#include <QDateTime>

#include "manualscanner.h"
#include "settings.h"
#include "macros.h"

extern Settings *qmc2Config;

ManualScanner::ManualScanner(int mode, QWidget *parent) :
	QDialog(parent),
	m_mode(mode)
{
	setupUi(this);
	switch ( m_mode ) {
		case QMC2_MANUALSCANNER_MODE_SYSTEMS:
			setWindowTitle(tr("System manual scanner"));
			m_settingsKey = "SystemManualScanner";
			break;
		case QMC2_MANUALSCANNER_MODE_SOFTWARE:
			setWindowTitle(tr("Software manual scanner"));
			m_settingsKey = "SoftwareManualScanner";
			break;
	}
}

ManualScanner::~ManualScanner()
{
	// NOP
}

void ManualScanner::on_pushButtonScanNow_clicked()
{
	pushButtonClose->setEnabled(false);
	pushButtonScanNow->setEnabled(false);
	plainTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	plainTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	plainTextEdit->clear();
	log(tr("scanner started"));
	scan();
	log(tr("scanner ended"));
	plainTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	plainTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	pushButtonScanNow->setEnabled(true);
	pushButtonClose->setEnabled(true);
}

void ManualScanner::log(const QString &message)
{
	plainTextEdit->appendPlainText(QDateTime::currentDateTime().toString("hh:mm:ss.zzz") + ": " + message);
	plainTextEdit->horizontalScrollBar()->setValue(plainTextEdit->horizontalScrollBar()->minimum());
	plainTextEdit->verticalScrollBar()->setValue(plainTextEdit->verticalScrollBar()->maximum());
}

void ManualScanner::showEvent(QShowEvent *e)
{
	restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + QString("Layout/%1/Geometry").arg(m_settingsKey), QByteArray()).toByteArray());
	QDialog::showEvent(e);
}

void ManualScanner::hideEvent(QHideEvent *e)
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + QString("Layout/%1/Geometry").arg(m_settingsKey), saveGeometry());
	QDialog::hideEvent(e);
}

void ManualScanner::closeEvent(QCloseEvent *e)
{
	if ( pushButtonClose->isEnabled() )
		e->accept();
	else
		e->ignore();
}

void ManualScanner::scan()
{
	// FIXME
}
