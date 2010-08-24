#ifndef _MESSSWLIST_H_
#define _MESSSWLIST_H_

#include <QProcess>
#include <QTime>
#include <QFile>
#include <QTextStream>
#include <QXmlDefaultHandler>
#include "ui_messswlist.h"

class MESSSoftwareListXmlHandler : public QXmlDefaultHandler
{
	public:
		QTreeWidget *parentTreeWidget;
		QTreeWidgetItem *softwareItem;
		QString softwareListName;
		QString softwareName;
		QString softwareTitle;
		QString softwarePublisher;
		QString softwareYear;
		QString currentText;

		MESSSoftwareListXmlHandler(QTreeWidget *);
		~MESSSoftwareListXmlHandler();
		
		bool startElement(const QString &, const QString &, const QString &, const QXmlAttributes &);
		bool endElement(const QString &, const QString &, const QString &);
		bool characters(const QString &);
};

class MESSSoftwareList : public QWidget, public Ui::MESSSoftwareList
{
	Q_OBJECT
	
	public:
		QProcess *loadProc;
		QTime loadTimer;
		bool validData;
		QFile fileSWLCache;
		QString messMachineName;
		QTextStream tsSWLCache;
		QStringList messSwlLines;

		MESSSoftwareList(QString, QWidget *);
		~MESSSoftwareList();

		QString &getSoftwareListXmlData(QString);
		QString &getXmlData(QString);

	public slots:
		bool load();
		bool save();

		// callback functions
		void on_toolButtonReload_clicked(bool);
		void on_toolButtonAddToFavorites_clicked(bool);
		void on_toolButtonRemoveFromFavorites_clicked(bool);
		void on_toolButtonPlay_clicked(bool);
		void on_toolButtonPlayEmbedded_clicked(bool);

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
