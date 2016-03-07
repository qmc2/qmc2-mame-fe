#include <QFileInfo>
#include <QPixmap>

#include "macros.h"
#include "fileiconprovider.h"

#if defined(QMC2_OS_WIN)
#include <windows.h>
#endif

QCache<QString, QIcon> FileIconProvider::m_iconCache;
QFileIconProvider FileIconProvider::m_iconProvider;

void FileIconProvider::setCacheSize(int size)
{
	m_iconCache.setMaxCost(size);
}

QIcon FileIconProvider::fileIcon(const QString &fileName)
{
#if defined(QMC2_OS_WIN)
	QFileInfo fileInfo(fileName);
	if ( fileInfo.suffix().isEmpty() || fileInfo.suffix() == "exe" && fileInfo.exists() )
		return m_iconProvider.icon(fileInfo);
	QIcon *cachedIcon = m_iconCache.object(fileInfo.suffix());
	QIcon icon;
	if ( !cachedIcon ) {
		static HRESULT comInit = CoInitialize(0);
		Q_UNUSED(comInit);
		SHFILEINFO shFileInfo;
		unsigned long val = SHGetFileInfo((const wchar_t *)("dummy." + fileInfo.suffix()).utf16(), 0, &shFileInfo, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_USEFILEATTRIBUTES);
		if ( val && shFileInfo.hIcon ) {
			QPixmap pixmap = QPixmap::fromWinHICON(shFileInfo.hIcon);
			if ( !pixmap.isNull() ) {
				icon = QIcon(pixmap);
				m_iconCache.insert(fileInfo.suffix(), new QIcon(icon));
			}
			DestroyIcon(shFileInfo.hIcon);
		}
	} else
		icon = *cachedIcon;
	return icon;
#else
	return m_iconProvider.icon(QFileInfo(fileName));
#endif
}

QIcon FileIconProvider::defaultFileIcon()
{
	return m_iconProvider.icon(QFileIconProvider::File);
}

QIcon FileIconProvider::folderIcon()
{
	return m_iconProvider.icon(QFileIconProvider::Folder);
}
