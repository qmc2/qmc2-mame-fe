#ifndef _FILESYSTEMMODEL_H_
#define _FILESYSTEMMODEL_H_

#include <QDir>
#include <QDateTime>
#include <QFileIconProvider>
#include <QAbstractItemModel>

class FileSystemItem : public QObject
{
	Q_OBJECT

	public:
		FileSystemItem(QString path, FileSystemItem* parent = 0)
		{
			mParent = parent;

			if ( parent ) {
				parent->addChild(this);
				mFileInfo = QFileInfo(path);
				mFileName = mFileInfo.fileName();
				mAbsDirPath = parent->absoluteDirPath();
				mAbsFilePath = mAbsDirPath + QString("/") + path;
				mFileInfo = QFileInfo(mAbsFilePath);
			} else {
				mAbsDirPath = path;
				mFileInfo = QFileInfo();
			}
		}

		~FileSystemItem()
		{
			qDeleteAll(mChildren);
		}

		FileSystemItem* childAt(int position)
		{
			return mChildren.value(position, 0);
		}

		void setRootPath(const QString &path)
		{
			if ( mParent == 0 ) {
				qDeleteAll(mChildren);
				mAbsDirPath = path;
			}
		}

		int childCount() const
		{
			return mChildren.count();
		}

		int childNumber() const
		{
			if ( mParent )
				return mParent->mChildren.indexOf(const_cast<FileSystemItem*>(this));
			else
				return 0;
		}

		FileSystemItem* parent()
		{
			return mParent;
		}

		QString absoluteFilePath() const
		{
			return mAbsFilePath;
		}

		QString absoluteDirPath() const
		{
			return mAbsDirPath;
		}

		QString fileName() const
		{
			return mFileName;
		}

		QFileInfo fileInfo() const
		{
			return mFileInfo;
		}

		void addChild(FileSystemItem *child)
		{
			if( !mChildren.contains(child) )
				mChildren.append(child);
		}

		FileSystemItem* matchPath(QString path)
		{
			foreach (FileSystemItem* child, mChildren)
				if ( child->absoluteFilePath() == path )
					return child;
			return 0;
		}

	private:
		FileSystemItem* mParent;
		QList<FileSystemItem*> mChildren;
		QFileInfo mFileInfo;
		QString mAbsFilePath;
		QString mAbsDirPath;
		QString mFileName;
};

class FileSystemModel : public QAbstractItemModel
{
	Q_OBJECT

	public:
		enum Column {NAME, SIZE, TYPE, DATE, LASTCOLUMN};

		FileSystemModel(QObject* parent) : QAbstractItemModel(parent), mIconFactory(new QFileIconProvider())
		{
			mHeaders << tr("Name") << tr("Size") << tr("Type") << tr("Date modified");
			mRootItem = new FileSystemItem("", 0);
			mCurrentPath = "";
		}

		~FileSystemModel()
		{
			delete mRootItem;
			delete mIconFactory;
		}

		QVariant headerData(int section, Qt::Orientation orientation, int role) const
		{
			if ( orientation == Qt::Horizontal ) {
				switch ( role ) {
					case Qt::DisplayRole:
						return mHeaders.at(section);
					case Qt::TextAlignmentRole:
						return int(SIZE) == section ? Qt::AlignRight : Qt::AlignLeft;
				}
			}

			return QVariant();
		}

		Qt::ItemFlags flags(const QModelIndex &index) const
		{
			if ( !index.isValid() )
				return 0;

			return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
		}

		int columnCount(const QModelIndex &) const
		{
			return LASTCOLUMN;
		}

		int rowCount(const QModelIndex &parent) const
		{
			return mRootItem->childCount();
		}

		QVariant data(const QModelIndex &index, int role) const
		{
			if ( !index.isValid() )
				return QVariant();

			if( int(SIZE) == index.column() && Qt::TextAlignmentRole == role )
				return Qt::AlignRight;

			if ( role != Qt::DisplayRole && role != Qt::DecorationRole )
				return QVariant();

			FileSystemItem *item = getItem(index);

			if ( !item )
				return QVariant();

			if ( role == Qt::DecorationRole && index.column() == int(NAME) )
				return mIconFactory->icon(item->fileInfo());

			QVariant data;
			Column col = Column(index.column());

			switch ( col ) {
				case NAME:
					data = item->fileName();
					break;
				case SIZE:
					data = item->fileInfo().size();
					break;
				case TYPE:
					data = mIconFactory->type(item->fileInfo());
					break;
				case DATE:
					data = item->fileInfo().lastModified().toString(Qt::LocalDate);
					break;
				default:
					data = "";
					break;
			}

			return data;
		}

		QModelIndex rootIndex()
		{
			return createIndex(mRootItem->childNumber(), 0, mRootItem);
		}

		QModelIndex index(int row, int column, const QModelIndex &parent) const
		{
			if ( parent.column() != int(NAME) )
				return QModelIndex();

			FileSystemItem *parentItem = getItem(parent);

			if ( parentItem ) {
				FileSystemItem *childItem = parentItem->childAt(row);
				if ( childItem )
					return createIndex(row, column, childItem);
			}

			return QModelIndex();
		}

		QModelIndex index(const QString& path, int column = NAME) const
		{
			if ( !path.isEmpty() ) {
				FileSystemItem *item = mRootItem->matchPath(path);
				if ( item )
					return createIndex(item->childNumber(), column, item);
			}

			return QModelIndex();
		}

		QModelIndex parent(const QModelIndex &index) const
		{
			return createIndex(mRootItem->childNumber(), NAME, mRootItem);
		}

		QString absolutePath(const QModelIndex &index)
		{
			FileSystemItem *item = static_cast<FileSystemItem*>(index.internalPointer());

			if ( item )
				return item->absoluteFilePath();
			else
				return QString();
		}

		QString currentPath() const
		{
			return mCurrentPath;
		}

		QModelIndex setCurrentPath(const QString &path)
		{
			mRootItem->deleteLater();
			mRootItem = new FileSystemItem(path, 0);
			mCurrentPath = path;
			populateItem(mRootItem);
			return index(path);
		}

		void setNameFilters(const QStringList &filters)
		{
			mNameFilters = filters;
		}

		QModelIndex refresh()
		{
			return setCurrentPath(mCurrentPath);
		}

	private:
		void populateItem(FileSystemItem *item)
		{
			QDir dir(item->absoluteDirPath());
			dir.setNameFilters(mNameFilters);
			QFileInfoList all = dir.entryInfoList(QDir::Files);
			foreach (QFileInfo one, all)
				new FileSystemItem(one.fileName(), item);
		}

		FileSystemItem *getItem(const QModelIndex &index) const
		{
			if ( index.isValid() ) {
				FileSystemItem *item = static_cast<FileSystemItem*>(index.internalPointer());
				if ( item ) return item;
			}
			return mRootItem;
		}

	private:
		FileSystemItem *mRootItem;
		QString mCurrentPath;
		QStringList mHeaders;
		QStringList mNameFilters;
		QFileIconProvider *mIconFactory;
};

#endif
