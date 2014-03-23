#include <QFileInfo>
#include <QPixmap>

#include "macros.h"
#include "fileiconprovider.h"

#if defined(QMC2_OS_WIN)
#include <windows.h>
#endif

FileIconProvider *FileIconProvider::self = 0;
QCache<QString, QIcon> FileIconProvider::iconCache;
QFileIconProvider FileIconProvider::iconProvider;

FileIconProvider::FileIconProvider()
{
	if ( iconCache.maxCost() != QMC2_FILEICONPROVIDER_CACHE_SIZE )
		iconCache.setMaxCost(QMC2_FILEICONPROVIDER_CACHE_SIZE);
}

FileIconProvider *FileIconProvider::instance()
{
	if( !self )
		self = new FileIconProvider();
	return self;
}

QIcon FileIconProvider::fileIcon(const QString &fileName)
{
#if defined(QMC2_OS_WIN)
	QFileInfo fileInfo(fileName);
	if ( fileInfo.suffix().isEmpty() || fileInfo.suffix() == "exe" && fileInfo.exists() )
		return instance()->iconProvider.icon(fileInfo);
	QIcon *cachedIcon = instance()->iconCache.object(fileInfo.suffix());
	QIcon icon;
	if ( !cachedIcon ) {
		static HRESULT comInit = CoInitialize(NULL);
		Q_UNUSED(comInit);
		SHFILEINFO shFileInfo;
		unsigned long val = SHGetFileInfo((const wchar_t *)("dummy." + fileInfo.suffix()).utf16(), 0, &shFileInfo, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_USEFILEATTRIBUTES);
		if ( val && shFileInfo.hIcon ) {
			QPixmap pixmap = QPixmap::fromWinHICON(shFileInfo.hIcon);
			if ( !pixmap.isNull() ) {
				icon = QIcon(pixmap);
				instance()->iconCache.insert(fileInfo.suffix(), new QIcon(icon));
			}
			DestroyIcon(shFileInfo.hIcon);
		}
	} else
		icon = *cachedIcon;
	return icon;
#else
	return instance()->iconProvider.icon(QFileInfo(fileName));
#endif
}

QIcon FileIconProvider::defaultFileIcon()
{
	return instance()->iconProvider.icon(QFileIconProvider::File);
}

QIcon FileIconProvider::folderIcon()
{
	return instance()->iconProvider.icon(QFileIconProvider::Folder);
}
