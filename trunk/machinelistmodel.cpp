#include "machinelistmodel.h"

MachineListModelItem::MachineListModelItem(const QString &name, const QIcon &icon, const QString &parent, const QString &description, const QString &manufacturer, const QString &year, const QString &source_file, int players, const QString &category, const QString &version, int rank, char rom_status, char rom_types, char driver_status, bool is_device, bool is_bios, bool tagged, MachineListModelItem *parentItem) :
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

MachineListModelItem::MachineListModelItem(MachineListModelItem *parentItem) :
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

MachineListModelItem::~MachineListModelItem()
{
	qDeleteAll(childItems());
}

MachineListProxyModel::MachineListProxyModel(QObject *parent) :
	QSortFilterProxyModel(parent)
{
	// NOP
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
	m_rootItem(0)
{
	m_headers << tr("Tag") << tr("Icon") << tr("Name") << tr("Parent") << tr("Description") << tr("Manufacturer") << tr("Year") << tr("ROM Status") << tr("ROM Types") << tr("Driver Status") << tr("Source File") << tr("Players") << tr("Rank") << tr("Is BIOS?") << tr("Is Device?") << tr("Category") << tr("Version");
}

MachineListModel::~MachineListModel()
{
	delete m_rootItem;
}

void MachineListModel::setRootItem(MachineListModelItem *item)
{
	delete m_rootItem;
	m_rootItem = item;
#if QT_VERSION < 0x050000
	reset();
#endif
}
 
QModelIndex MachineListModel::index(int row, int column, const QModelIndex &parent) const
{
	if ( !m_rootItem )
		return QModelIndex();
	MachineListModelItem *parentItem = itemFromIndex(parent);
	return createIndex(row, column, parentItem->childItems().at(row));
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

/*
bool MachineListModel::canFetchMore(const QModelIndex &parent) const
{
	// FIXME
	return false;
}

void MachineListModel::fetchMore(const QModelIndex &parent) const
{
	// FIXME
}
*/

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

QModelIndex MachineListModel::parent(const QModelIndex &child) const
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

MachineListModelItem *MachineListModel::itemFromIndex(const QModelIndex &index) const
{
	if ( index.isValid() )
		return static_cast<MachineListModelItem *>(index.internalPointer());
        else
		return m_rootItem;
}
