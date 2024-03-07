#ifndef MACHINELIST_H
#define MACHINELIST_H

#include <QtGui>
#include <QIcon>
#include <QTreeWidgetItem>
#include <QHash>
#include <QList>

#include "xmldbmgr.h"
#include "userdatadbmgr.h"
#include "datinfodbmgr.h"
#include "iconcachedbmgr.h"
#include "machinelistdbmgr.h"
#include "macros.h"

class MachineList : public QObject
{
	Q_OBJECT

	public:
		int numTotalMachines;
		int numMachines;
		int numCorrectMachines;
		int numMostlyCorrectMachines;
		int numIncorrectMachines;
		int numNotFoundMachines;
		int numUnknownMachines;
		int numMatchedMachines;
		int numTaggedSets;
		int numVerifyRoms;
		int uncommittedXmlDbRows;
		bool verifyCurrentOnly;
		bool autoRomCheck;
		bool mergeCategories;
		bool dtdBufferReady;
		bool initialLoad;
		char oldRomState;
		QIcon qmc2UnknownImageIcon;
		QIcon qmc2UnknownBIOSImageIcon;
		QIcon qmc2UnknownDeviceImageIcon;
		QIcon qmc2CorrectImageIcon;
		QIcon qmc2CorrectBIOSImageIcon;
		QIcon qmc2CorrectDeviceImageIcon;
		QIcon qmc2MostlyCorrectImageIcon;
		QIcon qmc2MostlyCorrectBIOSImageIcon;
		QIcon qmc2MostlyCorrectDeviceImageIcon;
		QIcon qmc2IncorrectImageIcon;
		QIcon qmc2IncorrectBIOSImageIcon;
		QIcon qmc2IncorrectDeviceImageIcon;
		QIcon qmc2NotFoundImageIcon;
		QIcon qmc2NotFoundBIOSImageIcon;
		QIcon qmc2NotFoundDeviceImageIcon;
		QProcess *loadProc;
		QProcess *verifyProc;
		QTime loadTimer;
		QTime verifyTimer;
		QTime parseTimer;
		QTime miscTimer;
		QFile romStateCache;
		QFile machineListCache;
		QTextStream tsRomCache;
		QTextStream tsMachineListCache;
		QString emulatorType;
		QString emulatorVersion;
		QString verifyLastLine;
		QString xmlLineBuffer;
		QStringList emulatorIdentifiers;
		QStringList verifiedList;
		QHash<QString, QString> driverNameHash;
		QHash<QString, char> machineStatusHash;
		QHash<QString, QString *> categoryNames;
		QHash<QString, QString *> categoryHash;
		QHash<QString, QString *> versionNames;
		QHash<QString, QString *> versionHash;
		QHash<QString, bool> biosSets;
		QHash<QString, bool> deviceSets;
		QHash<QString, QStringList> hierarchyHash;
		QTreeWidgetItem *checkedItem;

		static QStringList romTypeNames;
		static QStringList phraseTranslatorList;
		static QHash<QString, QString> reverseTranslations;
		static QHash<QString, QString> machineStateTranslations;
		static bool creatingCatView;
		static bool creatingVerView;
		static QString trQuestionMark;
		static QString trWaitingForData;

		QString lookupDriverName(const QString &);
		QString romStatus(const QString &, bool translated = false);
		QString &status();
		char romState(const QString &systemName) { char state = machineStatusHash.value(systemName); return (state == 0 ? 'U' : state); }
		bool isBios(const QString &systemName) { return biosSets.contains(systemName); }
		bool isDevice(const QString &systemName) { return deviceSets.contains(systemName); }
		int rank(const QString &systemName) { return userDataDb()->rank(systemName); }
		QString comment(const QString systemName) { return userDataDb()->comment(systemName); }

		void reopenIconCacheDb();
		bool loadIcon(const QString &, QTreeWidgetItem *);
		void clearCategoryNames() { qDeleteAll(categoryNames); categoryNames.clear(); }
		void clearVersionNames() { qDeleteAll(versionNames); versionNames.clear(); }

		QString sortCriteriaName(int);
		XmlDatabaseManager *xmlDb() { return m_xmlDb; }
		UserDataDatabaseManager *userDataDb() { return m_userDataDb; }
		DatInfoDatabaseManager *datInfoDb() { return m_datInfoDb; }
		MachineListDatabaseManager *machineListDb() { return m_machineListDb; }
		IconCacheDatabaseManager *iconCacheDb() { return m_iconCacheDb; }

		MachineList(QObject *parent = 0);
		~MachineList();

	public slots:
		void load();
		void verify(bool currentOnly = false);
		void loadCatverIni();
		void createVersionView();
		void loadCategoryIni();
		void createCategoryView();
		void loadFavorites();
		void saveFavorites();
		void loadPlayHistory();
		void savePlayHistory();

		// process management
		void loadStarted();
		void loadFinished(int, QProcess::ExitStatus);
		void loadReadyReadStandardOutput();
		void verifyStarted();
		void verifyFinished(int, QProcess::ExitStatus);
		void verifyReadyReadStandardOutput();

		// internal methods
		QString value(QXmlStreamReader *, QString, bool translate = false);
		void parse();
		void parseMachineDetail(QTreeWidgetItem *);
		void insertAttributeItems(QTreeWidgetItem *, QXmlStreamReader *, QStringList, QStringList, bool translate = false);
		void insertAttributeItems(QList<QTreeWidgetItem *> *itemList, QXmlStreamReader *element, QStringList attributes, QStringList descriptions, bool translate = false);
		void enableWidgets(bool enable = true);
		void disableWidgets() { enableWidgets(false); }
		void filter(bool initial = false);
		void filterOne(QTreeWidgetItem *item, char romState);

	signals:
		void widgetsEnabled(bool);

	private:
		XmlDatabaseManager *m_xmlDb;
		UserDataDatabaseManager *m_userDataDb;
		DatInfoDatabaseManager *m_datInfoDb;
		MachineListDatabaseManager *m_machineListDb;
		IconCacheDatabaseManager *m_iconCacheDb;
		QString m_trL, m_trC, m_trM, m_trI, m_trN, m_trU, m_trS, m_trT;
		QString m_statusString, m_statusTemplate;
		bool m_showC, m_showM, m_showI, m_showN, m_showU;
		bool m_showDeviceSets, m_showBiosSets;
		bool m_doFilter;
};

class MachineListItem : public QTreeWidgetItem
{
	public:
		static Qt::ItemFlags defaultItemFlags;

		MachineListItem(QTreeWidget *parentTreeWidget = 0) : QTreeWidgetItem(parentTreeWidget, QTreeWidgetItem::UserType) {}
		MachineListItem(QTreeWidgetItem *parentItem) : QTreeWidgetItem(parentItem, QTreeWidgetItem::UserType) {}

		virtual bool operator<(const QTreeWidgetItem &) const;

		QString id() { return text(QMC2_MACHINELIST_COLUMN_NAME); }
		QString name() { return id(); }
		QString description() { return text(QMC2_MACHINELIST_COLUMN_MACHINE); }
		QString manufacturer() { return text(QMC2_MACHINELIST_COLUMN_MANU); }
		QString year() { return text(QMC2_MACHINELIST_COLUMN_YEAR); }
		QString romTypes() { return text(QMC2_MACHINELIST_COLUMN_RTYPES); }
		QString driverStatus() { return text(QMC2_MACHINELIST_COLUMN_DRVSTAT); }
		QString sourceFile() { return text(QMC2_MACHINELIST_COLUMN_SRCFILE); }
		QString category() { return text(QMC2_MACHINELIST_COLUMN_CATEGORY); }
		QString version() { return text(QMC2_MACHINELIST_COLUMN_VERSION); }
		QIcon machineIcon() { return icon(QMC2_MACHINELIST_COLUMN_MACHINE); }
		int players() { bool ok; int p = text(QMC2_MACHINELIST_COLUMN_PLAYERS).toInt(&ok); return ok ? p : -1; }
		int rank() { return whatsThis(QMC2_MACHINELIST_COLUMN_RANK).toInt(); }
		bool isParent() { return parentId().isEmpty(); }
		bool isClone() { return !isParent(); }
		bool tagged() { return checkState(QMC2_MACHINELIST_COLUMN_TAG) == Qt::Checked; }
		bool isBios();
		bool isDevice();
		char romStatus();
		QString parentId();
};

#endif
