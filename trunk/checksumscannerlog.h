#ifndef _CHECKSUMSCANNERLOG_H_
#define _CHECKSUMSCANNERLOG_H_

#include <QString>
#include <QMutex>

#include "ui_checksumscannerlog.h"

class CheckSumScannerLog : public QDialog, public Ui::CheckSumScannerLog
{
	Q_OBJECT

       	public:
		CheckSumScannerLog(QString settingsKey, QWidget *parent = 0);

		void setLogSyncMutex(QMutex *mtx) { m_logSyncMutex = mtx; }
		qreal progress() { return m_progress; }

	public slots:
		void on_spinBoxMaxLogSize_valueChanged(int);
		void log(const QString &);
		void clear() { plainTextEditLog->clear(); }
		void scrollToEnd();
		void progressTextChanged(const QString &);
		void progressRangeChanged(int, int);
		void progressChanged(int);

	signals:
		void windowOpened();
		void windowClosed();

	protected:
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);
		void closeEvent(QCloseEvent *);
		void keyPressEvent(QKeyEvent *);

	private:
		qreal m_progress;
		QString m_settingsKey;
		QMutex *m_logSyncMutex;
};

#endif
