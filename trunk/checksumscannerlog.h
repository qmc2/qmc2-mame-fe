#ifndef _CHECKSUMSCANNERLOG_H_
#define _CHECKSUMSCANNERLOG_H_

#include <QStringList>

#include "ui_checksumscannerlog.h"

class CheckSumScannerLog : public QWidget, public Ui::CheckSumScannerLog
{
	Q_OBJECT

       	public:
		CheckSumScannerLog(QWidget *parent = 0);

	public slots:
		void on_spinBoxMaxLogSize_valueChanged(int);
		void log(const QString &);
		void clear() { plainTextEditLog->clear(); }
		void scrollToEnd();

	signals:
		void windowOpened();
		void windowClosed();

	protected:
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);
		void closeEvent(QCloseEvent *);
		void keyPressEvent(QKeyEvent *);
};

#endif
