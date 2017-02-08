#ifndef _MANUALSCANNER_H_
#define _MANUALSCANNER_H_

#include <QString>

#include "ui_manualscanner.h"

#define QMC2_MANUALSCANNER_MODE_SYSTEMS		0
#define QMC2_MANUALSCANNER_MODE_SOFTWARE	1

class ManualScanner : public QDialog, public Ui::ManualScanner
{
	Q_OBJECT

       	public:
		explicit ManualScanner(int mode, QWidget *parent = 0);
		~ManualScanner();

	public slots:
		void on_pushButtonScanNow_clicked();
		void log(const QString &);

	protected:
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);
		void closeEvent(QCloseEvent *);

	private:
		void scan();

		int m_mode;
		QString m_settingsKey;
};

#endif
