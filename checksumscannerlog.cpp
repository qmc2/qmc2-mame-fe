#include <QApplication>
#include <QScrollBar>
#include <QString>
#include <QFont>

#include "checksumscannerlog.h"
#include "settings.h"
#include "macros.h"

extern Settings *qmc2Config;

CheckSumScannerLog::CheckSumScannerLog(QString settingsKey, QWidget *parent)
#if defined(QMC2_OS_WIN)
	: QDialog(parent, Qt::Dialog)
#else
	: QDialog(parent, Qt::Dialog | Qt::SubWindow)
#endif
{
	hide();
	m_progress = -1;
	m_settingsKey = settingsKey;
	setupUi(this);
	QFont logFont;
	logFont.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/LogFont").toString());
	plainTextEditLog->setFont(logFont);
	spinBoxMaxLogSize->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + m_settingsKey + "/MaxLogSize", 10000).toInt());
}

void CheckSumScannerLog::on_spinBoxMaxLogSize_valueChanged(int value)
{
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + m_settingsKey + "/MaxLogSize", value);
	plainTextEditLog->setMaximumBlockCount(value);
}

void CheckSumScannerLog::log(const QString &message)
{
	if ( checkBoxEnableLog->isChecked() ) {
		m_messageQueue << message;
		if ( m_logSyncMutex->tryLock(10) ) {
			for (int i = 0; i < m_messageQueue.count(); i++)
				plainTextEditLog->appendPlainText(m_messageQueue[i]);
			m_messageQueue.clear();
			m_logSyncMutex->unlock();
			plainTextEditLog->update();
		}
	}
}

void CheckSumScannerLog::flushMessageQueue()
{
	if ( checkBoxEnableLog->isChecked() ) {
		m_logSyncMutex->lock();
		for (int i = 0; i < m_messageQueue.count(); i++)
			plainTextEditLog->appendPlainText(m_messageQueue[i]);
		m_messageQueue.clear();
		m_logSyncMutex->unlock();
	}
}

void CheckSumScannerLog::scrollToEnd()
{
	plainTextEditLog->horizontalScrollBar()->setValue(plainTextEditLog->horizontalScrollBar()->minimum());
	plainTextEditLog->verticalScrollBar()->setValue(plainTextEditLog->verticalScrollBar()->maximum());
}

void CheckSumScannerLog::progressTextChanged(const QString &text)
{
	progressBar->setFormat(text);
}

void CheckSumScannerLog::progressRangeChanged(int min, int max)
{
	progressBar->setRange(min, max);
	m_progress = -1;
}

void CheckSumScannerLog::progressChanged(int progress)
{
	progressBar->setValue(progress);
	if ( progressBar->maximum() > progressBar->minimum() )
		m_progress = 100.0 * (qreal)progress / (qreal)(progressBar->maximum() - progressBar->minimum());
	else
		m_progress = -1;
}

void CheckSumScannerLog::showEvent(QShowEvent *e)
{
	restoreGeometry(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/" + m_settingsKey + "/Geometry", QByteArray()).toByteArray());
	emit windowOpened();
	if ( e )
		QWidget::showEvent(e);
}

void CheckSumScannerLog::hideEvent(QHideEvent *e)
{
	if ( isVisible() )
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/" + m_settingsKey + "/Geometry", saveGeometry());
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
