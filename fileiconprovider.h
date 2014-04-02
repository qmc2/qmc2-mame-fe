#ifndef _FILEICONPROVIDER_H_
#define _FILEICONPROVIDER_H_

#include <QIcon>
#include <QCache>
#include <QFileIconProvider>

#define QMC2_FILEICONPROVIDER_CACHE_SIZE	1000

class FileIconProvider
{
	public:
		FileIconProvider() {}

		static QIcon fileIcon(const QString &fileName);
		static QIcon defaultFileIcon();
		static QIcon folderIcon();
		static void setCacheSize(int size);
		static QFileIconProvider *iconProvider() { return &m_iconProvider; }

	private:
		static QCache<QString, QIcon> m_iconCache;
		static QFileIconProvider m_iconProvider;
};

#endif
