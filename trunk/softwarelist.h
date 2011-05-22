#ifndef _SOFTWARELIST_H_
#define _SOFTWARELIST_H_

#include <QProcess>
#include <QTime>
#include <QFile>
#include <QTextStream>
#include <QXmlDefaultHandler>
#include "ui_softwarelist.h"
#include "unzip.h"

class SoftwareListXmlHandler : public QXmlDefaultHandler
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

		SoftwareListXmlHandler(QTreeWidget *);
		~SoftwareListXmlHandler();
		
		bool startElement(const QString &, const QString &, const QString &, const QXmlAttributes &);
		bool endElement(const QString &, const QString &, const QString &);
		bool characters(const QString &);
};

class SoftwareSnap : public QWidget
{
	Q_OBJECT

	public:
		QMenu *contextMenu;
		unzFile snapFile;

		SoftwareSnap(QWidget *parent = 0);

	public slots:
		void copyToClipboard();
		void saveAs();

	protected:
		void leaveEvent(QEvent *);
		void mousePressEvent(QMouseEvent *);
		void contextMenuEvent(QContextMenuEvent *);
		void keyPressEvent(QKeyEvent *);
		void paintEvent(QPaintEvent *);
};

class SoftwareList : public QWidget, public Ui::SoftwareList
{
	Q_OBJECT
	
	public:
		QProcess *loadProc;
		QTime loadTimer;
		bool validData;
		QFile fileSWLCache;
		QString systemName;
		QTextStream tsSWLCache;
		QStringList swlLines;
		QMenu *softwareListMenu;
		QAction *actionAddToFavorites;
		QAction *actionRemoveFromFavorites;

		SoftwareList(QString, QWidget *);
		~SoftwareList();

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
