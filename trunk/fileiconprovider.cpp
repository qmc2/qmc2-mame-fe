#include <QFileInfo>
#include <QPixmap>

#include "macros.h"
#include "fileiconprovider.h"

#if defined(QMC2_OS_WIN)
#include <windows.h>
#endif

QCache<QString, QIcon> FileIconProvider::iconCache;
QFileIconProvider FileIconProvider::iconProvider;

void FileIconProvider::setCacheSize(int size)
{
	iconCache.setMaxCost(size);
}

QIcon FileIconProvider::fileIcon(const QString &fileName)
{
#if defined(QMC2_OS_WIN)
	QFileInfo fileInfo(fileName);
	if ( fileInfo.suffix().isEmpty() || fileInfo.suffix() == "exe" && fileInfo.exists() )
		return iconProvider.icon(fileInfo);
	QIcon *cachedIcon = iconCache.object(fileInfo.suffix());
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
				iconCache.insert(fileInfo.suffix(), new QIcon(icon));
			}
			DestroyIcon(shFileInfo.hIcon);
		}
	} else
		icon = *cachedIcon;
	return icon;
#else
	return iconProvider.icon(QFileInfo(fileName));
#endif
}

QIcon FileIconProvider::defaultFileIcon()
{
	return iconProvider.icon(QFileIconProvider::File);
}

QIcon FileIconProvider::folderIcon()
{
	return iconProvider.icon(QFileIconProvider::Folder);
}
