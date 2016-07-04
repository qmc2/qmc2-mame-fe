#ifndef _MACHINELISTMODEL_H_
#define _MACHINELISTMODEL_H_

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QList>
#include <QString>
#include <QVariant>
#include <QIcon>

//#include "macros.h"

#define QMC2_MLM_ROM_TYPE_NONE			0
#define QMC2_MLM_ROM_TYPE_ROM			1
#define QMC2_MLM_ROM_TYPE_CHD			2
#define QMC2_MLM_ROM_TYPE_BOTH			3

#define QMC2_MLM_DRIVER_STATUS_NONE		0
#define QMC2_MLM_DRIVER_STATUS_GOOD		1
#define QMC2_MLM_DRIVER_STATUS_IMPERFECT	2
#define QMC2_MLM_DRIVER_STATUS_PRELIMINARY	3

class MachineListModelItem
{
	public:
		MachineListModelItem(const QString &name, const QIcon &icon, const QString &parent, const QString &description, const QString &manufacturer, const QString &year, const QString &source_file, const QString &players, const QString &category, const QString &version, int rank, char rom_status, char rom_types, char driver_status, bool is_device, bool is_bios, bool tagged, MachineListModelItem *parentItem = 0) :
			m_parentItem(parentItem)
		{
			setName(name);
			setParent(parent);
			setDescription(description);
			setManufacturer(manufacturer);
			setYear(year);
			setSourceFile(source_file);
			setPlayers(players);
			setCategory(category);
			setVersion(version);
			setIcon(icon);
			setRank(rank);
			setRomStatus(rom_status);
			setRomTypes(rom_types);
			setDriverStatus(driver_status);
			setIsDevice(is_device);
			setIsBios(is_bios);
			setTag(tagged);
		}

		MachineListModelItem(MachineListModelItem *parentItem = 0) :
			m_parentItem(parentItem)
		{
			setRank(0);
			setRomStatus('U');
			setRomTypes(QMC2_MLM_ROM_TYPE_NONE);
			setDriverStatus(QMC2_MLM_DRIVER_STATUS_NONE);
			setIsDevice(false);
			setIsBios(false);
			setTag(false);
		}

		~MachineListModelItem()
		{
			qDeleteAll(childItems());
		}

		void setName(const QString &name) { m_name = name; }
		QString &name() { return m_name; }
		void setParent(const QString &parent) { m_parent = parent; }
		QString &parent() { return m_parent; }
		void setDescription(const QString &description) { m_description = description; }
		QString &description() { return m_description; }
		void setManufacturer(const QString &manufacturer) { m_manufacturer = manufacturer; }
		QString &manufacturer() { return m_manufacturer; }
		void setYear(const QString &year) { m_year = year; }
		QString &year() { return m_year; }
		void setSourceFile(const QString &source_file) { m_source_file = source_file; }
		QString &sourceFile() { return m_source_file; }
		void setPlayers(const QString &players) { m_players = players; }
		QString &players() { return m_players; }
		void setCategory(const QString &category) { m_category = category; }
		QString &category() { return m_category; }
		void setVersion(const QString &version) { m_version = version; }
		QString &version() { return m_version; }
		void setIcon(const QIcon &icon) { m_icon = icon; }
		QIcon &icon() { return m_icon; }
		void setRank(int rank) { m_rank = rank; }
		int rank() { return m_rank; }
		void setRomStatus(char rom_status) { m_rom_status = rom_status; }
		char romStatus() { return m_rom_status; }
		void setRomTypes(char rom_types) { m_rom_types = rom_types; }
		char romTypes() { return m_rom_types; }
		void setDriverStatus(char driver_status) { m_driver_status = driver_status; }
		char driverStatus() { return m_driver_status; }
		void setIsDevice(bool is_device) { m_is_device = is_device; }
		bool isDevice() { return m_is_device; }
		void setIsBios(bool is_bios) { m_is_bios = is_bios; }
		bool isBios() { return m_is_bios; }
		void setTag(bool tagged) { m_tagged = tagged; }
		bool tagged() { return m_tagged; }

		void setParentItem(MachineListModelItem *item) { m_parentItem = item; }
		MachineListModelItem *parentItem() { return m_parentItem; }
		QList<MachineListModelItem *> &childItems() { return m_childItems; }

	private:
		QString m_name;
		QString m_parent;
		QString m_description;
		QString m_manufacturer;
		QString m_year;
		QString m_source_file;
		QString m_players;
		QString m_category;
		QString m_version;
		QIcon m_icon;
		int m_rank;
		char m_rom_status;
		char m_rom_types;
		char m_driver_status;
		bool m_is_device;
		bool m_is_bios;
		bool m_tagged;
		QList<MachineListModelItem *> m_childItems;
		MachineListModelItem *m_parentItem;
};

class MachineListProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT

	public:
		MachineListProxyModel(QObject *parent = 0) :
			QSortFilterProxyModel(parent)
		{
			// NOP
		}

	protected:
		bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
		{
			// FIXME
			return true;
		}
		bool lessThan(const QModelIndex &left, const QModelIndex &right) const
		{
			QVariant leftData(sourceModel()->data(left));
			QVariant rightData(sourceModel()->data(right));
			switch ( leftData.type() ) {
				case QVariant::String:
					return QString::localeAwareCompare(leftData.toString(), rightData.toString()) < 0;
				case QVariant::Int:
					return leftData.toInt() < rightData.toInt();
				case QVariant::Char:
					return leftData.toChar() < rightData.toChar();
				case QVariant::Bool:
					return leftData.toBool() < rightData.toBool();
				default:
					return false;
			}
		}

	private:
};

class MachineListModel : public QAbstractItemModel
{
	Q_OBJECT

	public:
		enum Column {TAG, ICON, NAME, PARENT, DESCRIPTION, MANUFACTURER, YEAR, ROM_STATUS, ROM_TYPES, DRIVER_STATUS, SOURCE_FILE, PLAYERS, RANK, IS_BIOS, IS_DEVICE, CATEGORY, VERSION, LAST_COLUMN};

		MachineListModel(QObject *parent) :
			QAbstractItemModel(parent),
			m_rootItem(0)
		{
			m_headers << tr("Tag") << tr("Icon") << tr("Name") << tr("Parent") << tr("Description") << tr("Manufacturer") << tr("Year") << tr("ROM Status") << tr("ROM Types") << tr("Driver Status") << tr("Source File") << tr("Players") << tr("Rank") << tr("Is BIOS?") << tr("Is Device?") << tr("Category") << tr("Version");
		}

		~MachineListModel()
		{
			delete m_rootItem;
		}

		void setRootItem(MachineListModelItem *item)
		{
			delete m_rootItem;
			m_rootItem = item;
#if QT_VERSION < 0x050000
			reset();
#endif
		}
 
		virtual QModelIndex index(int row, int column, const QModelIndex &parent) const
		{
			if ( !m_rootItem )
				return QModelIndex();
			MachineListModelItem *parentItem = itemFromIndex(parent);
			return createIndex(row, column, parentItem->childItems().at(row));
		}

		virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const
		{
			if ( orientation == Qt::Horizontal ) {
				switch ( role ) {
					case Qt::DisplayRole:
						return m_headers.at(section);
					case Qt::TextAlignmentRole:
						return Qt::AlignLeft;
				}
			}
			return QVariant();
		}

		/*
		virtual bool canFetchMore(const QModelIndex &parent) const
		{
			// FIXME
			return false;
		}

		virtual void fetchMore(const QModelIndex &parent)
		{
			// FIXME
		}
		*/

		virtual Qt::ItemFlags flags(const QModelIndex &index) const
		{
			return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled;
		}

		virtual int columnCount(const QModelIndex &) const
		{
			return int(LAST_COLUMN);
		}

		virtual int rowCount(const QModelIndex &parent = QModelIndex()) const
		{
			MachineListModelItem *parentItem = itemFromIndex(parent);
			if ( !parentItem )
				return 0;
			return parentItem->childItems().count();
		}

		virtual QVariant data(const QModelIndex &index, int role) const
		{
			if ( !index.isValid() )
				return QVariant();
			MachineListModelItem *item = itemFromIndex(index);
			if ( !item )
				return QVariant();
			QVariant data;
			switch ( role ) {
				case Qt::TextAlignmentRole:
					return Qt::AlignLeft;
				case Qt::DisplayRole:
					switch ( Column(index.column()) ) {
						case TAG:
							return item->tagged();
						case ICON:
							return item->icon();
						// FIXME
						case NAME:
						default:
							return item->name();
					}
					break;
				case Qt::DecorationRole:
					switch ( Column(index.column()) ) {
						// FIXME
						default:
							break;
					}
					break;
				default:
					return QVariant();
			}
			return data;
		}

		virtual QModelIndex parent(const QModelIndex &child) const
		{
			MachineListModelItem *item = itemFromIndex(child);
			if ( !item )
				return QModelIndex();
			MachineListModelItem *parentItem = item->parentItem();
			if ( !parentItem )
				return QModelIndex();
			MachineListModelItem *grandParentItem = parentItem->parentItem();
			if ( !grandParentItem )
				return QModelIndex();
			int row = grandParentItem->childItems().indexOf(parentItem);
			return createIndex(row, child.column(), parentItem);
		}

	private:
		QStringList m_headers;
		MachineListModelItem *m_rootItem;

		MachineListModelItem *itemFromIndex(const QModelIndex &index) const
		{
			if ( index.isValid() )
				return static_cast<MachineListModelItem *>(index.internalPointer());
		        else
				return m_rootItem;
		}
};

#endif
