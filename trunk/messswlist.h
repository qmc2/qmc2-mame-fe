#ifndef _MESSSWLIST_H_
#define _MESSSWLIST_H_

#include <QProcess>
#include <QXmlDefaultHandler>
#include "ui_messswlist.h"

/*
class MESSSoftwareListXmlHandler : public QXmlDefaultHandler
{
	public:
		QTreeWidget *parentTreeWidget;

		MESSSoftwareListXmlHandler(QTreeWidget *);
		~MESSSoftwareListXmlHandler();
		
		bool startElement(const QString &, const QString &, const QString &, const QXmlAttributes &);
		bool endElement(const QString &, const QString &, const QString &);
		bool characters(const QString &);
};
*/

class MESSSoftwareList : public QWidget, public Ui::MESSSoftwareList
{
	Q_OBJECT
	
	public:
		QProcess *loadProc;
		QString messMachineName;

		MESSSoftwareList(QString, QWidget *);
		~MESSSoftwareList();

		QString &getXmlData(QString);

	public slots:
		bool load();
		bool save();

		// callback functions

		// process management
		void loadStarted();
		void loadFinished(int, QProcess::ExitStatus);
		void loadReadyReadStandardOutput();
		void loadReadyReadStandardError();
		void loadError(QProcess::ProcessError);
		void loadStateChanged(QProcess::ProcessState);

	protected:
		void closeEvent(QCloseEvent *);
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);
};

#endif
