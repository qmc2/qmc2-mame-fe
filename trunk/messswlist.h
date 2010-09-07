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
		QString softwarePart;
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
		QMenu *softwareListMenu;
		QAction *actionAddToFavorites;
		QAction *actionRemoveFromFavorites;

		MESSSoftwareList(QString, QWidget *);
		~MESSSoftwareList();

		QString &getSoftwareListXmlData(QString);
		QString &getXmlData(QString);
		QStringList &arguments();

	public slots:
		bool load();
		bool save();

		// callback functions
		void on_toolButtonReload_clicked(bool);
		void on_toolButtonAddToFavorites_clicked(bool);
		void on_toolButtonRemoveFromFavorites_clicked(bool);
		void on_toolButtonPlay_clicked(bool);
		void on_toolButtonPlayEmbedded_clicked(bool);
		void on_treeWidgetKnownSoftware_itemSelectionChanged();
		void on_treeWidgetFavoriteSoftware_itemSelectionChanged();
		void on_treeWidgetSearchResults_itemSelectionChanged();
		void on_treeWidgetKnownSoftware_customContextMenuRequested(const QPoint &);
		void on_treeWidgetFavoriteSoftware_customContextMenuRequested(const QPoint &);
		void on_treeWidgetSearchResults_customContextMenuRequested(const QPoint &);

		// process management
		void loadStarted();
		void loadFinished(int, QProcess::ExitStatus);
		void loadReadyReadStandardOutput();
		void loadReadyReadStandardError();
		void loadError(QProcess::ProcessError);
		void loadStateChanged(QProcess::ProcessState);
 
		void treeWidgetKnownSoftware_headerSectionClicked(int);
		void treeWidgetFavoriteSoftware_headerSectionClicked(int);
		void treeWidgetSearchResults_headerSectionClicked(int);
		void addToFavorites() { on_toolButtonAddToFavorites_clicked(false); }
		void removeFromFavorites() { on_toolButtonRemoveFromFavorites_clicked(false); }
		void playActivated() { on_toolButtonPlay_clicked(false); }
		void playEmbeddedActivated() { on_toolButtonPlayEmbedded_clicked(false); }

	protected:
		void closeEvent(QCloseEvent *);
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);
};

#endif
