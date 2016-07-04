#include <QTimer>
#include <QHash>

#include "machinelistmodel.h"
#include "machinelist.h"

extern MachineList *qmc2MachineList;
extern QHash<QString, QIcon> qmc2IconHash;

#define mlDb	qmc2MachineList->machineListDb()

MachineListModelItem::MachineListModelItem(const QString &id, const QIcon &icon, const QString &parent, const QString &description, const QString &manufacturer, const QString &year, const QString &source_file, int players, const QString &category, const QString &version, int rank, char rom_status, bool has_roms, bool has_chds, const QString &driver_status, bool is_device, bool is_bios, bool tagged, MachineListModelItem *parentItem) :
	m_parentItem(parentItem)
{
	setId(id);
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
	setHasRoms(has_roms);
	setHasChds(has_chds);
	setDriverStatus(driver_status);
	setIsDevice(is_device);
	setIsBios(is_bios);
	setTag(tagged);
}

MachineListModelItem::MachineListModelItem(MachineListModelItem *parentItem) :
	m_parentItem(parentItem)
{
	setRank(0);
	setRomStatus('U');
	setDriverStatus(QObject::tr("unknown"));
	setIsDevice(false);
	setIsBios(false);
	setHasRoms(false);
	setHasChds(false);
	setTag(false);
}

MachineListModelItem::~MachineListModelItem()
{
	qDeleteAll(childItems());
}

int MachineListModelItem::row() const
{
	if ( m_parentItem )
		return m_parentItem->childItems().indexOf(const_cast<MachineListModelItem *>(this));
        return 0;
}

MachineListProxyModel::MachineListProxyModel(QObject *parent) :
	QSortFilterProxyModel(parent)
{
	setDynamicSortFilter(true);
}

bool MachineListProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	// FIXME
	return true;
}

bool MachineListProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
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

MachineListModel::MachineListModel(QObject *parent) :
	QAbstractItemModel(parent),
	m_rootItem(0),
	m_query(0),
	m_recordCount(0)
{
	m_query = new QSqlQuery(mlDb->db());
	m_headers << tr("Tag") << tr("Icon") << tr("Name") << tr("Parent") << tr("Description") << tr("Manufacturer") << tr("Year") << tr("ROM Status") << tr("Has ROMs?") << tr("Has CHDs?") << tr("Driver Status") << tr("Source File") << tr("Players") << tr("Rank") << tr("Is BIOS?") << tr("Is Device?") << tr("Category") << tr("Version");
	setRootItem(new MachineListModelItem);
	QTimer::singleShot(0, this, SLOT(startQuery()));
}

MachineListModel::~MachineListModel()
{
	m_query->clear();
	delete m_rootItem;
	delete m_query;
}

void MachineListModel::setRootItem(MachineListModelItem *item)
{
	if ( m_rootItem )
		delete m_rootItem;
	m_rootItem = item;
	reset();
}
 
void MachineListModel::startQuery()
{
	mlDb->queryRecords(m_query);
	fetchMore(QModelIndex());
}

QModelIndex MachineListModel::index(int row, int column, const QModelIndex &parent) const
{
	if ( parent.isValid() && parent.column() != int(ID) )
		return QModelIndex();
	MachineListModelItem *childItem = itemFromIndex(parent)->childItems().at(row);
	if ( childItem )
		return createIndex(row, column, childItem);
	else
		return QModelIndex();
}

QVariant MachineListModel::headerData(int section, Qt::Orientation orientation, int role) const
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

bool MachineListModel::canFetchMore(const QModelIndex &parent) const
{
	return m_recordCount < qmc2MachineList->numMachines;
}

void MachineListModel::fetchMore(const QModelIndex &parent)
{
	//if ( !m_rootItem )
	//	return;
	MachineListModelItem *parentItem = itemFromIndex(parent);
	if ( !parentItem )
		return;
	int itemsToFetch = QMC2_MIN(100, qmc2MachineList->numMachines - m_recordCount);
	QMC2_PRINT_INT(itemsToFetch);
	QString id, description, manufacturer, year, cloneof, drvstat, srcfile;
	bool is_bios, is_device, has_roms, has_chds;
	int players, i = 0;
	beginInsertRows(QModelIndex(), m_recordCount, m_recordCount + itemsToFetch - 1);
	while ( i < itemsToFetch && mlDb->nextRecord(m_query, &id, &description, &manufacturer, &year, &cloneof, &is_bios, &is_device, &has_roms, &has_chds, &players, &drvstat, &srcfile) ) {
		parentItem->childItems().append(new MachineListModelItem(id,
									 qmc2IconHash.value(id),
									 cloneof,
									 description,
									 manufacturer,
									 year,
									 srcfile,
									 players,
									 QString(), // FIXME: category
									 QString(), // FIXME: version
									 0,         // FIXME: rank
									 qmc2MachineList->romState(id),
									 has_roms,
									 has_chds,
									 drvstat,
									 is_device,
									 is_bios,
									 false,
									 m_rootItem));
		i++;
	}
	m_recordCount += itemsToFetch;
        endInsertRows();
	QMC2_PRINT_INT(m_recordCount);
}

Qt::ItemFlags MachineListModel::flags(const QModelIndex &index) const
{
	return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled;
}

int MachineListModel::columnCount(const QModelIndex &) const
{
	return int(LAST_COLUMN);
}

int MachineListModel::rowCount(const QModelIndex &parent) const
{
        MachineListModelItem *parentItem = itemFromIndex(parent);
	if ( !parentItem )
		return 0;
	return parentItem->childItems().count();
}

QVariant MachineListModel::data(const QModelIndex &index, int role) const
{
	if ( !index.isValid() )
		return QVariant();
	MachineListModelItem *item = itemFromIndex(index);
	if ( !item )
		return QVariant();
	switch ( role ) {
		case Qt::TextAlignmentRole:
			return Qt::AlignLeft;
		case Qt::DisplayRole:
			switch ( Column(index.column()) ) {
				/*
				case TAG:
					return item->tagged();
				*/
				case ID:
					QMC2_PRINT_STR(item->id());
					return item->id();
				case PARENT:
					return item->parent();
				case DESCRIPTION:
					return item->description();
				case MANUFACTURER:
					return item->manufacturer();
				case YEAR:
					return item->year();
				case ROM_STATUS:
					return item->romStatus();
				case HAS_ROMS:
					return item->hasRoms() ? "true" : "false";
				case HAS_CHDS:
					return item->hasChds() ? "true" : "false";
				case DRIVER_STATUS:
					return item->driverStatus();
				case SOURCE_FILE:
					return item->sourceFile();
				case PLAYERS:
					return item->players();
				case RANK:
					return QString::number(item->rank());
				case IS_BIOS:
					return item->isBios() ? "true" : "false";
				case IS_DEVICE:
					return item->isDevice() ? "true" : "false";
				case CATEGORY:
					return item->category();
				case VERSION:
					return item->version();
				default:
					return QVariant();
			}
			break;
		case Qt::DecorationRole:
			switch ( Column(index.column()) ) {
				// FIXME
				case ICON:
					return item->icon();
				default:
					return QVariant();
			}
			break;
		default:
			return QVariant();
	}
	return QVariant();
}

QModelIndex MachineListModel::parent(const QModelIndex &index) const
{
	if ( !index.isValid() )
		return QModelIndex();
	MachineListModelItem *parentItem = itemFromIndex(index);
	if ( !parentItem || parentItem == m_rootItem )
		return QModelIndex();
	return createIndex(parentItem->row(), 0, parentItem);
}

MachineListModelItem *MachineListModel::itemFromIndex(const QModelIndex &index) const
{
	if ( index.isValid() ) {
		MachineListModelItem *item = static_cast<MachineListModelItem *>(index.internalPointer());
		if ( item )
			return item;
	}
	return m_rootItem;
}
