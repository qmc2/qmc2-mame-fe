#ifndef _FILEICONPROVIDER_H_
#define _FILEICONPROVIDER_H_

#include <QIcon>
#include <QCache>
#include <QFileIconProvider>

#define QMC2_FILEICONPROVIDER_CACHE_SIZE	1000

class FileIconProvider
{
	public:
		static FileIconProvider *instance();
		static QIcon fileIcon(const QString &fileName);
		static QIcon defaultFileIcon();
		static QIcon folderIcon();

		FileIconProvider();

	private:
		static FileIconProvider *self;
		static QCache<QString, QIcon> iconCache;
		static QFileIconProvider iconProvider;
};

#endif
