#ifndef SAMPLECHECKER_H
#define SAMPLECHECKER_H

#include <QDialog>
#include <QTime>
#include <QMap>
#include <QStringList>
#include "ui_samplechecker.h"

class SampleChecker : public QDialog, public Ui::SampleChecker
{
	Q_OBJECT

	public:
		QMap<QString, QString> sampleMap;
		QTime verifyTimer;
		QStringList sampleSets;

		SampleChecker(QWidget *parent = 0);

	public slots:
		void adjustIconSizes();
		void restoreLayout();
		void recursiveFileList(const QString &, QStringList &);
		void verify();
		void verifyObsolete();
		void on_pushButtonSamplesCheck_clicked();
		void on_toolButtonSamplesRemoveObsolete_clicked();

	protected:
		void closeEvent(QCloseEvent *);
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);
};

#endif
