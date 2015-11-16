#ifndef _SOFTWARELIST_H_
#define _SOFTWARELIST_H_

#include <QProcess>
#include <QTime>
#include <QTimer>
#include <QFile>
#include <QList>
#include <QAction>
#include <QTextStream>
#include <QXmlDefaultHandler>
#include <QMenu>
#include <QMap>
#include <QWidgetAction>

#include "ui_softwarelist.h"
#include "unzip.h"
#include "sevenzipfile.h"
#include "swlistexport.h"
#include "imagewidget.h"
#include "softwarestatefilter.h"

class SoftwareListExporter;

class SoftwareItem : public QTreeWidgetItem
{
	public:
		SoftwareItem(QTreeWidget *parent) : QTreeWidgetItem(parent) { ; }
		SoftwareItem(QTreeWidgetItem *parent) : QTreeWidgetItem(parent) { ; }
		SoftwareItem(SoftwareItem *parent) : QTreeWidgetItem((QTreeWidgetItem *)parent) { ; }

	protected:
		bool operator<(const QTreeWidgetItem &other) const
		{
			if ( parent() != NULL )
				return false;
			if ( other.parent() != NULL )
				return false;
			int col = treeWidget()->sortColumn();
			return ( text(col) < other.text(col) );
		}
};

class SoftwareListXmlHandler : public QXmlDefaultHandler
{
	public:
		QTreeWidget *parentTreeWidget;
		SoftwareItem *softwareItem;
		QString softwareListName;
		QString softwareName;
		QString softwareSupported;
		QString softwareTitle;
		QString softwarePublisher;
		QString softwareYear;
		QString softwarePart;
		QString softwareInterface;
		QString softwareParentName;
		QString currentText;
		QStringList compatFilters;
		int elementCounter;
		bool newSoftwareStates;
		bool softwareListHidden;
		quint64 numTotal, numCorrect, numMostlyCorrect, numIncorrect, numNotFound, numUnknown;

		SoftwareListXmlHandler(QTreeWidget *, QStringList *hiddenLists, bool viewTree = false);
		~SoftwareListXmlHandler();
		
		bool startElement(const QString &, const QString &, const QString &, const QXmlAttributes &);
		bool endElement(const QString &, const QString &, const QString &);
		bool characters(const QString &);

		void loadSoftwareStates(QString);

		QList<QTreeWidgetItem *> &itemList() { return m_itemList; }
		QList<QTreeWidgetItem *> &hideList() { return m_hideList; }
		bool viewTree() { return m_viewTree; }
		void setViewTree(bool viewTree) { m_viewTree = viewTree; }
		void setHiddenLists(QStringList *hiddenLists) { m_hiddenLists = hiddenLists; }
		QStringList *hiddenLists() { return m_hiddenLists; }

	private:
		bool m_viewTree;
		QList<QTreeWidgetItem *> m_itemList;
		QList<QTreeWidgetItem *> m_hideList;
		QStringList *m_hiddenLists;
};

class SoftwareEntryXmlHandler : public QXmlDefaultHandler
{
	public:
		SoftwareItem *parentTreeWidgetItem;
		SoftwareItem *partItem;
		SoftwareItem *infoItem;
		SoftwareItem *requirementItem;
		SoftwareItem *compatItem;
		SoftwareItem *dataareaItem;
		SoftwareItem *romItem;
		bool softwareValid;
		bool success;
		QString softwareName;
		int elementCounter;
		int animSequenceCounter;
		QList<QTreeWidgetItem *> partItems;
		QList<QTreeWidgetItem *> infoItems;
		QList<QTreeWidgetItem *> requirementItems;
		QMap<QTreeWidgetItem *, QComboBox *> comboBoxes;

		SoftwareEntryXmlHandler(QTreeWidgetItem *, bool viewTree = false);
		~SoftwareEntryXmlHandler();
		
		bool startElement(const QString &, const QString &, const QString &, const QXmlAttributes &);
		bool endElement(const QString &, const QString &, const QString &);

		bool viewTree() { return m_viewTree; }
		void setViewTree(bool viewTree) { m_viewTree = viewTree; }

	private:
		bool m_viewTree;
};

// 'floating' snapshot-viewer
class SoftwareSnap : public QWidget
{
	Q_OBJECT

	public:
		QString listName;
		QString entryName;
		QPoint position;
		QMap<QString, unzFile> snapFileMap;
		QMap<QString, SevenZipFile *> snapFileMap7z;
		SoftwareItem *myItem;
		QTimer snapForcedResetTimer;
		QMenu *contextMenu;
		bool ctxMenuRequested;
		QString myCacheKey;
		QAction *actionCopyPathToClipboard;
		int zoom;
		QList<int> activeFormats;

		SoftwareSnap(QWidget *parent = 0);
		~SoftwareSnap();

		QString primaryPathFor(QString, QString);
		void reloadActiveFormats();
		void enableWidgets(bool enable = true);

		bool useZip();
		bool useSevenZip();

	public slots:
		void loadImage(bool fromParent = false);
		bool replaceImage(QString, QString, QPixmap &);
		void resetSnapForced();
		void copyToClipboard();
		void copyPathToClipboard();
		void refresh();
		void sevenZipDataReady();
		void zoomIn();
		void zoomOut();
		void resetZoom();

	protected:
		void paintEvent(QPaintEvent *);
		void mousePressEvent(QMouseEvent *);
		void enterEvent(QEvent *);
		void leaveEvent(QEvent *);
		void contextMenuEvent(QContextMenuEvent *);

	private:
		bool m_async;
};

class SoftwareList : public QWidget, public Ui::SoftwareList
{
	Q_OBJECT
	
	public:
		bool snapForced;
		bool validData;
		bool autoSelectSearchItem;
		bool autoMounted;
		bool interruptLoad;
		bool isLoading;
		bool isReady;
		bool fullyLoaded;
		bool loadFinishedFlag;
		bool updatingMountDevices;
		bool negatedMatch;
		bool verifyReadingStdout;
		bool searchActive;
		bool stopSearch;
		int oldMax, oldMin;
		int uncommittedSwlDbRows;
		QAction *actionAddToFavorites;
		QAction *actionRemoveFromFavorites;
		QAction *actionSaveFavoritesToFile;
		QAction *actionClearSelection;
		QAction *actionNegateSearch;
		QAction *actionCheckSoftwareStates;
		QAction *actionAnalyzeSoftware;
		QAction *actionAnalyzeSoftwareList;
		QAction *actionAnalyzeSoftwareLists;
		QAction *actionRebuildSoftware;
		QAction *actionRebuildSoftwareList;
		QAction *actionRebuildSoftwareLists;
		QAction *analyzeMenuAction;
		QAction *rebuildMenuAction;
		QAction *viewFlatAction;
		QAction *viewTreeAction;
		QFile softwareStateFile;
		QList<QTreeWidgetItem *> softwareListItems, favoritesListItems, searchListItems;
		QMenu *softwareListMenu;
		QMenu *favoritesOptionsMenu;
		QMenu *menuKnownSoftwareHeader;
		QMenu *menuFavoriteSoftwareHeader;
		QMenu *menuSearchResultsHeader;
		QMenu *menuSnapnameAdjustment;
		QMenu *menuSoftwareStates;
		QMenu *menuSearchOptions;
		QMenu *knownSoftwareMenu;
		QMenu *toggleListMenu;
		QProcess *loadProc;
		QProcess *verifyProc;
		QString systemName;
		QString swlLastLine;
		QString softwareListName;
		QString oldFmt;
		QStringList successfulLookups;
		QStringList mountedSoftware;
		QStringList swlLines;
		QTextStream softwareStateStream;
		QTime loadTimer;
		QTimer snapTimer;
		QTimer searchTimer;
		QTimer detailUpdateTimer;
		QTreeWidgetItem *currentItem;
		QTreeWidgetItem *enteredItem;
		quint64 numSoftwareTotal, numSoftwareCorrect, numSoftwareIncorrect, numSoftwareMostlyCorrect, numSoftwareNotFound, numSoftwareUnknown, numSoftwareMatches;
		quint64 oldSoftwareCorrect, oldSoftwareIncorrect, oldSoftwareMostlyCorrect, oldSoftwareNotFound, oldSoftwareUnknown;
		SoftwareListExporter *exporter;
		SoftwareStateFilter *stateFilter;
		QWidgetAction *stateFilterAction;
		QToolButton *toolBoxButtonKnownSoftware;

		static bool isInitialLoad;
		static bool swlSupported;
		static QString swStatesLastLine;

		SoftwareList(QString, QWidget *);
		~SoftwareList();

		void getXmlData();
		QString &getSoftwareListXmlData(QString);
		QString &getXmlDataWithEnabledSlots(QStringList);
		QString &lookupMountDevice(QString, QString, QStringList *mountList = NULL);
		QStringList &arguments(QStringList *softwareLists = NULL, QStringList *softwareNames = NULL);
		QString softwareStatus(QString, QString, bool translated = false);

		bool viewTree() { return m_viewTree; }
		void setViewTree(bool viewTree) { m_viewTree = viewTree; }

		void addMenuSectionHeader(QMenu *menu, QString text);

	signals:
		void loadFinished(bool);

	public slots:
		bool load();
		bool save();
		void updateMountDevices();
		void checkSoftwareStates();
		QString status(SoftwareListXmlHandler *handler = NULL);
		void updateStats(SoftwareListXmlHandler *handler = NULL);
		void loadTree();

		// auto-connected callback functions
		void on_toolButtonReload_clicked(bool);
		void on_toolButtonExport_clicked(bool);
		void on_toolButtonAddToFavorites_clicked(bool);
		void on_toolButtonToggleSoftwareInfo_clicked(bool);
		void on_toolButtonCompatFilterToggle_clicked(bool);
		void on_toolButtonToggleSnapnameAdjustment_clicked(bool);
		void on_toolButtonRemoveFromFavorites_clicked(bool);
		void on_toolButtonPlay_clicked(bool);
		void on_toolButtonPlayEmbedded_clicked(bool);
		void on_treeWidgetKnownSoftware_itemSelectionChanged();
		void on_treeWidgetKnownSoftware_customContextMenuRequested(const QPoint &);
		void on_treeWidgetKnownSoftware_itemEntered(QTreeWidgetItem *, int);
		void on_treeWidgetKnownSoftware_itemActivated(QTreeWidgetItem *, int);
		void on_treeWidgetKnownSoftware_itemDoubleClicked(QTreeWidgetItem *item, int column);
		void on_treeWidgetKnownSoftware_itemExpanded(QTreeWidgetItem *);
		void on_treeWidgetKnownSoftware_itemClicked(QTreeWidgetItem *, int) { on_treeWidgetKnownSoftware_itemSelectionChanged(); }
		void on_treeWidgetKnownSoftwareTree_itemSelectionChanged();
		void on_treeWidgetKnownSoftwareTree_customContextMenuRequested(const QPoint &);
		void on_treeWidgetKnownSoftwareTree_itemEntered(QTreeWidgetItem *, int);
		void on_treeWidgetKnownSoftwareTree_itemActivated(QTreeWidgetItem *, int);
		void on_treeWidgetKnownSoftwareTree_itemDoubleClicked(QTreeWidgetItem *item, int column);
		void on_treeWidgetKnownSoftwareTree_itemExpanded(QTreeWidgetItem *);
		void on_treeWidgetKnownSoftwareTree_itemClicked(QTreeWidgetItem *, int) { on_treeWidgetKnownSoftwareTree_itemSelectionChanged(); }
		void on_treeWidgetFavoriteSoftware_itemSelectionChanged();
		void on_treeWidgetFavoriteSoftware_customContextMenuRequested(const QPoint &);
		void on_treeWidgetFavoriteSoftware_itemEntered(QTreeWidgetItem *, int);
		void on_treeWidgetFavoriteSoftware_itemActivated(QTreeWidgetItem *, int);
		void on_treeWidgetFavoriteSoftware_itemDoubleClicked(QTreeWidgetItem *item, int column);
		void on_treeWidgetFavoriteSoftware_itemExpanded(QTreeWidgetItem *);
		void on_treeWidgetFavoriteSoftware_itemClicked(QTreeWidgetItem *, int) { on_treeWidgetFavoriteSoftware_itemSelectionChanged(); }
		void on_treeWidgetSearchResults_itemSelectionChanged();
		void on_treeWidgetSearchResults_customContextMenuRequested(const QPoint &);
		void on_treeWidgetSearchResults_itemEntered(QTreeWidgetItem *, int);
		void on_treeWidgetSearchResults_itemActivated(QTreeWidgetItem *, int);
		void on_treeWidgetSearchResults_itemDoubleClicked(QTreeWidgetItem *item, int column);
		void on_treeWidgetSearchResults_itemExpanded(QTreeWidgetItem *);
		void on_treeWidgetSearchResults_itemClicked(QTreeWidgetItem *, int) { on_treeWidgetSearchResults_itemSelectionChanged(); }
		void on_comboBoxSearch_editTextChanged(const QString &);
		void on_comboBoxSearch_activated(const QString &);
		void on_comboBoxDeviceConfiguration_currentIndexChanged(int);
		void on_toolBoxSoftwareList_currentChanged(int);
		void on_toolButtonSoftwareStates_toggled(bool);
		void on_stackedWidgetKnownSoftware_currentChanged(int);

		// process management
		void loadStarted();
		void loadFinished(int, QProcess::ExitStatus);
		void loadReadyReadStandardOutput();
		void loadReadyReadStandardError();
		void loadError(QProcess::ProcessError);
		void verifyStarted();
		void verifyFinished(int, QProcess::ExitStatus);
		void verifyReadyReadStandardOutput();
		void verifyReadyReadStandardError();
		void verifyError(QProcess::ProcessError);
 
		// other
		void treeWidgetKnownSoftware_headerSectionClicked(int);
		void treeWidgetKnownSoftwareTree_headerSectionClicked(int);
		void treeWidgetFavoriteSoftware_headerSectionClicked(int);
		void treeWidgetSearchResults_headerSectionClicked(int);
		void addToFavorites() { on_toolButtonAddToFavorites_clicked(false); }
		void removeFromFavorites() { on_toolButtonRemoveFromFavorites_clicked(false); }
		void analyzeSoftware();
		void analyzeSoftwareList();
		void analyzeSoftwareLists();
		void playActivated() { on_toolButtonPlay_clicked(false); }
		void playEmbeddedActivated() { on_toolButtonPlayEmbedded_clicked(false); }
		void cancelSoftwareSnap();
		void comboBoxSearch_editTextChanged_delayed();
		void checkMountDeviceSelection();
		void loadFavoritesFromFile();
		void saveFavoritesToFile();
		void checkSoftwareSnap();
		void updateDetail();
		void adjustSnapnamePattern();
		void clearSoftwareSelection();
		void negateSearchTriggered(bool);
		void rebuildSoftware();
		void rebuildSoftwareList();
		void rebuildSoftwareLists();
		void updateRebuildSoftwareMenuVisibility();
		void analyzeSoftwareMenu_aboutToShow();
		void rebuildSoftwareMenu_aboutToShow();
		void actionViewFlat_triggered();
		void actionViewTree_triggered();
		void toggleSoftwareList();
		void showOnlyThisSoftwareList();
		void showAllExceptThisSoftwareList();

		// callbacks for software-list header context menu requests
		void treeWidgetKnownSoftwareHeader_customContextMenuRequested(const QPoint &);
		void treeWidgetKnownSoftwareTreeHeader_customContextMenuRequested(const QPoint &);
		void actionKnownSoftwareHeader_triggered();
		void treeWidgetFavoriteSoftwareHeader_customContextMenuRequested(const QPoint &);
		void actionFavoriteSoftwareHeader_triggered();
		void treeWidgetSearchResultsHeader_customContextMenuRequested(const QPoint &);
		void actionSearchResultsHeader_triggered();

	protected:
		void closeEvent(QCloseEvent *);
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);
		void mouseMoveEvent(QMouseEvent *);
		void leaveEvent(QEvent *);
		void resizeEvent(QResizeEvent *);

	private:
		bool m_viewTree;
};

#endif
