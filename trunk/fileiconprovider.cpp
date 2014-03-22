#include <QFileInfo>
#include <QPixmap>

#include "macros.h"
#include "fileiconprovider.h"

#if defined(QMC2_OS_WIN)
#include <windows.h>
#endif

FileIconProvider *FileIconProvider::self = 0;

FileIconProvider::FileIconProvider() {
	iconCache.setMaxCost(QMC2_FILEICONPROVIDER_CACHE_SIZE);
}

FileIconProvider *FileIconProvider::instance()
{
	if( !self )
		self = new FileIconProvider();
	return self;
}

QIcon FileIconProvider::fileIcon(const QString &filename)
{
	QFileInfo fileInfo(filename);
#if defined(QMC2_OS_WIN)
	QIcon *iconPtr = instance()->iconCache.object(fileInfo.suffix());
	QIcon icon;
	if ( !iconPtr ) {
		if ( fileInfo.suffix().isEmpty() || fileInfo.suffix() == "exe" && fileInfo.exists() ) {
			icon = instance()->iconProvider.icon(fileInfo);
			instance()->iconCache.insert(fileInfo.suffix(), new QIcon(icon));
		} else {
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
		}
	} else
		icon = *iconPtr;
	return icon;
#else
	return instance()->iconProvider.icon(fileInfo);
#endif
}

QIcon FileIconProvider::folderIcon()
{
	return instance()->iconProvider.icon(QFileIconProvider::Folder);
}

QIcon FileIconProvider::defaultFileIcon()
{
	return instance()->iconProvider.icon(QFileIconProvider::File);
}
