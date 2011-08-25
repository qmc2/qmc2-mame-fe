#ifndef _FILESYSTEMMODEL_H_
#define _FILESYSTEMMODEL_H_

#include <QDir>
#include <QDirIterator>
#include <QDateTime>
#include <QFileIconProvider>
#include <QAbstractItemModel>
#include <QApplication>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QTimer>
#include <QTest>

#define QMC2_DIRENTRY_THRESHOLD		100

class DirectoryScannerThread : public QThread
{
	Q_OBJECT

	public:
		QMutex waitMutex;
		QWaitCondition waitCondition;
		bool isScanning;
		bool stopScanning;
		bool quitFlag;
		bool isReady;
		QString dirPath;
		QStringList dirEntries;
		QStringList nameFilters;

		DirectoryScannerThread(QString path, QObject *parent = 0) : QThread(parent)
		{
			isReady = isScanning = stopScanning = quitFlag = false;
			dirPath = path;
			start();
		}

		~DirectoryScannerThread()
		{
			stopScanning = quitFlag = true;
			quit();
		}

	protected:
		void run()
		{
			while ( !quitFlag ) {
#if defined(QMC2_DEBUG)
				printf("DirectoryScannerThread: waiting\n");
#endif
				waitMutex.lock();
				isReady = true;
				waitCondition.wait(&waitMutex);
				waitMutex.unlock();
#if defined(QMC2_DEBUG)
				printf("DirectoryScannerThread: starting scan of %s\n", (const char *)dirPath.toAscii());
#endif
				if ( !stopScanning && !quitFlag ) {
					waitMutex.lock();
					isScanning = true;
					stopScanning = false;
					dirEntries.clear();
					QDirIterator dirIterator(dirPath, nameFilters, QDir::Files);
					while ( dirIterator.hasNext() && !stopScanning && !quitFlag ) {
						dirIterator.next();
						dirEntries << dirIterator.fileName();
						if ( dirEntries.count() >= QMC2_DIRENTRY_THRESHOLD ) {
							emit entriesAvailable(dirEntries);
#if defined(QMC2_DEBUG)
							foreach (QString entry, dirEntries)
								printf("DirectoryScannerThread: %s\n", (const char *)entry.toAscii());
#endif
							dirEntries.clear();
							QThread::yieldCurrentThread();
							QTest::qSleep(1);
						}
					}
					if ( !stopScanning && !quitFlag ) {
						if ( dirEntries.count() > 0 ) {
							emit entriesAvailable(dirEntries);
#if defined(QMC2_DEBUG)
							foreach (QString entry, dirEntries)
								printf("DirectoryScannerThread: %s\n", (const char *)entry.toAscii());
#endif
						}
						emit finished();
					}
					isScanning = false;
					waitMutex.unlock();
				}
#if defined(QMC2_DEBUG)
				printf("DirectoryScannerThread: finished scan of %s\n", (const char *)dirPath.toAscii());
#endif
			}
#if defined(QMC2_DEBUG)
			printf("DirectoryScannerThread: ended\n");
#endif
		}

	signals:
		void entriesAvailable(const QStringList &);
		void finished();
};

class FileSystemItem : public QObject
{
	Q_OBJECT

	public:
		QList<FileSystemItem*> mChildren;

		FileSystemItem(QString path, FileSystemItem* parent = 0)
		{
			mParent = parent;

			if ( parent ) {
				parent->addChild(this);
				mFileInfo = QFileInfo(path);
				mFileName = mFileInfo.fileName();
				mAbsDirPath = parent->absoluteDirPath();
				mAbsFilePath = mAbsDirPath + QString("/") + path;
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
				mAbsDirPath = path;
				qDeleteAll(mChildren);
				mChildren.clear();
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
		QFileInfo mFileInfo;
		QString mAbsFilePath;
		QString mAbsDirPath;
		QString mFileName;
};

class FileSystemModel : public QAbstractItemModel
{
	Q_OBJECT

	public:
		DirectoryScannerThread *dirScanner;

		enum Column {NAME, /* SIZE, TYPE, DATE, */ LASTCOLUMN};

		FileSystemModel(QObject *parent) : QAbstractItemModel(parent), mIconFactory(new QFileIconProvider())
		{
			mHeaders << tr("Name"); // << tr("Size") << tr("Type") << tr("Date modified");
			mRootItem = new FileSystemItem("", 0);
			mCurrentPath = "";
			mFileCount = mStaleCount = 0;
			dirScanner = new DirectoryScannerThread(mRootItem->absoluteDirPath());
			connect(dirScanner, SIGNAL(entriesAvailable(const QStringList &)), this, SLOT(scannerEntriesAvailable(const QStringList &)));
		}

		~FileSystemModel()
		{
			if ( dirScanner ) {
				dirScanner->stopScanning = true;
				dirScanner->quitFlag = true;
				dirScanner->waitCondition.wakeAll();
				dirScanner->deleteLater();
				dirScanner = NULL;
			}
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
						return Qt::AlignLeft;
						//return int(SIZE) == section ? Qt::AlignRight : Qt::AlignLeft;
				}
			}

			return QVariant();
		}

		virtual bool canFetchMore(const QModelIndex &) const
		{
			return (mStaleCount > 0);
		}

		virtual void fetchMore(const QModelIndex &)
		{
			int itemsToFetch = qMin(QMC2_DIRENTRY_THRESHOLD, mStaleCount);
			beginInsertRows(QModelIndex(), mFileCount, mFileCount + itemsToFetch - 1);
			mFileCount += itemsToFetch;
			mStaleCount -= itemsToFetch;
			endInsertRows();
#if defined(QMC2_DEBUG)
			printf("mFileCount = %d, mStaleCount = %d\n", mFileCount, mStaleCount);
#endif
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

		int rowCount(const QModelIndex &) const
		{
			return mFileCount;
		}

		int rowCount() const
		{
			return mFileCount;
		}

		QVariant data(const QModelIndex &index, int role) const
		{
			if ( !index.isValid() )
				return QVariant();

			/*
			if( int(SIZE) == index.column() && Qt::TextAlignmentRole == role )
				return Qt::AlignRight;
			*/

			if ( role != Qt::DisplayRole && role != Qt::DecorationRole )
				return QVariant();

			FileSystemItem *item = getItem(index);

			if ( !item )
				return QVariant();

			if ( !mRootItem->mChildren.contains(item) )
				return QVariant();

			if ( role == Qt::DecorationRole /* && index.column() == int(NAME) */ )
				return mIconFactory->icon(item->fileInfo());

			QVariant data = item->fileName();
			/*
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
					QVariant();
					break;
			}
			*/

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

		QModelIndex setCurrentPath(const QString &path, bool scan = true)
		{
			mCurrentPath = path;
			if ( dirScanner->isScanning )
				dirScanner->stopScanning = true;
			mRootItem->deleteLater();
			mRootItem = new FileSystemItem(path, 0);
			mFileCount = mStaleCount = 0;
			if ( scan )
				populateItems();
			return rootIndex();
		}

		void setNameFilters(const QStringList &filters)
		{
			mNameFilters = filters;
		}

		QModelIndex refresh()
		{
			return setCurrentPath(mCurrentPath);
		}

	public slots:
		void scannerEntriesAvailable(const QStringList &entryList)
		{
			foreach (QString entry, entryList)
				new FileSystemItem(entry, mRootItem);
			mStaleCount += entryList.count();
			fetchMore(QModelIndex());
#if defined(QMC2_DEBUG)
			printf("mFileCount = %d, mStaleCount = %d\n", mFileCount, mStaleCount);
#endif
		}

	private slots:
		void populateItems()
		{
			if ( dirScanner ) {
				if ( !dirScanner->isReady ) QTimer::singleShot(0, this, SLOT(populateItems()));
				dirScanner->dirPath = mRootItem->absoluteDirPath();
				dirScanner->nameFilters = mNameFilters;
				dirScanner->stopScanning = false;
				dirScanner->waitCondition.wakeAll();
			}
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
		int mFileCount;
		int mStaleCount;
};

#endif
