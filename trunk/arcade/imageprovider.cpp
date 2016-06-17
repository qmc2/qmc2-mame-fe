#include <qglobal.h>

#if QT_VERSION < 0x050000
#include <QApplication>
#else
#include <QGuiApplication>
#endif
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>

#include "imageprovider.h"
#include "arcadesettings.h"
#include "tweakedqmlappviewer.h"
#include "macros.h"

extern ArcadeSettings *globalConfig;

#if QT_VERSION < 0x050000
ImageProvider::ImageProvider(QDeclarativeImageProvider::ImageType type, QObject *parent)
	: QObject(parent), QDeclarativeImageProvider(type)
	#else
ImageProvider::ImageProvider(QQuickImageProvider::ImageType type, QObject *parent)
	: QObject(parent), QQuickImageProvider(type)
	#endif
{
	mImageTypes << "prv" << "fly" << "cab" << "ctl" << "mrq" << "ttl" << "pcb" << "sws" << "ico";
	mCustomImageTypes << globalConfig->customSystemArtworkNames() << globalConfig->customSoftwareArtworkNames();
	mImageCache.setMaxCost(QMC2_ARCADE_IMGCACHE_SIZE);
	mPixmapCache.setMaxCost(QMC2_ARCADE_IMGCACHE_SIZE);
	foreach (QString imageType, QStringList() << mImageTypes << mCustomImageTypes) {
		foreach (QString imagePath, imageTypeToFile(imageType).split(";", QString::SkipEmptyParts)) {
			if ( isZippedImageType(imageType) ) {
				mFileMapZip.insert(imagePath, unzOpen(imagePath.toUtf8().constData()));
				if ( !mFileMapZip.value(imagePath) ) {
					QMC2_ARCADE_LOG_STR(tr("WARNING: Can't open %1 ZIP file '%2'").arg(imageTypeToLongName(imageType)).arg(imagePath));
				} else
					mFileTypeMap.insert(imagePath, imageType);
				emit imageDataUpdated(imageType);
			} else if ( isSevenZippedImageType(imageType) ) {
				SevenZipFile *imageFile = new SevenZipFile(imagePath);
				if ( imageFile->open() ) {
					connect(imageFile, SIGNAL(dataReady()), this, SLOT(sevenZipDataReady()));
					mFileMap7z.insert(imagePath, imageFile);
					mFileTypeMap.insert(imagePath, imageType);
					mAsyncMap.insert(imagePath, false);
				} else {
					QMC2_ARCADE_LOG_STR(tr("WARNING: Can't open %1 7z file '%2'").arg(imageTypeToLongName(imageType)).arg(imagePath));
					delete imageFile;
				}
			}
#if defined(QMC2_ARCADE_LIBARCHIVE_ENABLED)
			else if ( isArchivedImageType(imageType) ) {
				ArchiveFile *imageFile = new ArchiveFile(imagePath);
				if ( imageFile->open() ) {
					mArchiveMap.insert(imagePath, imageFile);
					mFileTypeMap.insert(imagePath, imageType);
				} else {
					QMC2_ARCADE_LOG_STR(tr("WARNING: Can't open %1 archive file '%2'").arg(imageTypeToLongName(imageType)).arg(imagePath));
					delete imageFile;
				}
			}
#endif
			else {
				emit imageDataUpdated(imageType);
				mFileTypeMap.insert(imagePath, imageType);
			}
		}
		QStringList activeFormats;
		if ( mCustomImageTypes.contains(imageType) ) {
			activeFormats = globalConfig->customArtworkFormats(imageType);
			mCustomCachePrefixes.insert(imageType, QUuid::createUuid().toString());
			imageType = mCustomCachePrefixes[imageType];
		} else
			activeFormats = globalConfig->activeImageFormats(imageType);
		if ( activeFormats.isEmpty() )
			mActiveFormatsMap[imageType] << QMC2_ARCADE_IMAGE_FORMAT_INDEX_PNG;
		else for (int i = 0; i < activeFormats.count(); i++)
			mActiveFormatsMap[imageType] << activeFormats[i].toInt();
	}
	// we support all formats for icons in this (hard-coded) order
	mActiveFormatsMap["ico"] << QMC2_ARCADE_IMAGE_FORMAT_INDEX_ICO << QMC2_ARCADE_IMAGE_FORMAT_INDEX_PNG << QMC2_ARCADE_IMAGE_FORMAT_INDEX_BMP << QMC2_ARCADE_IMAGE_FORMAT_INDEX_GIF << QMC2_ARCADE_IMAGE_FORMAT_INDEX_JPG
				 << QMC2_ARCADE_IMAGE_FORMAT_INDEX_PBM << QMC2_ARCADE_IMAGE_FORMAT_INDEX_PGM << QMC2_ARCADE_IMAGE_FORMAT_INDEX_PPM << QMC2_ARCADE_IMAGE_FORMAT_INDEX_TIFF << QMC2_ARCADE_IMAGE_FORMAT_INDEX_XBM
				 << QMC2_ARCADE_IMAGE_FORMAT_INDEX_XPM << QMC2_ARCADE_IMAGE_FORMAT_INDEX_SVG << QMC2_ARCADE_IMAGE_FORMAT_INDEX_TGA;
	mFormatExtensions << "png" << "bmp" << "gif" << "jpg, jpeg" << "pbm" << "pgm" << "ppm" << "tif, tiff" << "xbm" << "xpm" << "svg" << "tga" << "ico";
	mFormatNames << "PNG" << "BMP" << "GIF" << "JPG" << "PBM" << "PGM" << "PPM" << "TIFF" << "XBM" << "XPM" << "SVG" << "TGA" << "ICO";
}

ImageProvider::~ImageProvider()
{
	foreach (unzFile zipFile, mFileMapZip)
		unzClose(zipFile);
	foreach (SevenZipFile *sevenZipFile, mFileMap7z) {
		sevenZipFile->close();
		delete sevenZipFile;
	}
}

QImage ImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
	QImage image, result;
	QString cacheKey, cachePrefix;

	if ( !id.isEmpty() ) {
		cacheKey = loadImage(id, CacheClassImage);
		cachePrefix = id.split("/", QString::SkipEmptyParts).at(0);
	}

	if ( !cacheKey.isEmpty() ) {
		if ( isAsync(cachePrefix) ) {
			if ( cachePrefix == "ico" ) {
				image = QImage(QSize(1, 1), QImage::Format_ARGB32);
				image.fill(Qt::transparent);
			} else {
				image.load(QLatin1String(":/images/ghost.png"));
				QPainter p;
				QString message = tr("Decompressing archive, please wait...");
				p.begin(&image);
				p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);
				QFont f(qApp->font());
				f.setWeight(QFont::Bold);
				f.setPointSize(f.pointSize() * 2);
				QFontMetrics fm(f);
				int adjustment = fm.height() / 2;
				p.setFont(f);
				QRect outerRect = p.boundingRect(image.rect(), Qt::AlignCenter | Qt::TextWordWrap, message).adjusted(-adjustment, -adjustment, adjustment, adjustment);
				QPainterPath pp;
				pp.addRoundedRect(outerRect, 5, 5);
				p.fillPath(pp, QBrush(QColor(0, 0, 0, 128), Qt::SolidPattern));
				p.setPen(QColor(255, 255, 0, 255));
				p.drawText(image.rect(), Qt::AlignCenter | Qt::TextWordWrap, message);
				p.end();
			}
		} else if ( mImageCache.contains(cacheKey) ) {
			image = *mImageCache.object(cacheKey);
		} else {
			cacheKey = loadImage(id, CacheClassImage);
			if ( !cacheKey.isEmpty() )
				image = *mImageCache.object(cacheKey);
			else if ( cachePrefix == "ico" ) {
				image = QImage(QSize(1, 1), QImage::Format_ARGB32);
				image.fill(Qt::transparent);
			} else
				image.load(QLatin1String(":/images/ghost.png"));
		}
	} else {
		if ( cachePrefix == "ico" ) {
			image = QImage(QSize(1, 1), QImage::Format_ARGB32);
			image.fill(Qt::transparent);
		} else
			image.load(QLatin1String(":/images/ghost.png"));
	}

	if ( requestedSize.isValid() )
		result = image.scaled(requestedSize, Qt::KeepAspectRatio);
	else
		result = image;

	if ( size )
		*size = result.size();

	return result;
}

QPixmap ImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
	QPixmap image, result;
	QString cacheKey, cachePrefix;

	if ( !id.isEmpty() ) {
		cacheKey = loadImage(id, CacheClassPixmap);
		cachePrefix = id.split("/", QString::SkipEmptyParts).at(0);
	}

	if ( !cacheKey.isEmpty() ) {
		if ( isAsync(cachePrefix) ) {
			if ( cachePrefix == "ico" ) {
				image = QPixmap(QSize(1, 1));
				image.fill(Qt::transparent);
			} else {
				image.load(QLatin1String(":/images/ghost.png"));
				QPainter p;
				QString message = tr("Decompressing archive, please wait...");
				p.begin(&image);
				p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);
				QFont f(qApp->font());
				f.setWeight(QFont::Bold);
				f.setPointSize(f.pointSize() * 2);
				QFontMetrics fm(f);
				int adjustment = fm.height() / 2;
				p.setFont(f);
				QRect outerRect = p.boundingRect(image.rect(), Qt::AlignCenter | Qt::TextWordWrap, message).adjusted(-adjustment, -adjustment, adjustment, adjustment);
				QPainterPath pp;
				pp.addRoundedRect(outerRect, 5, 5);
				p.fillPath(pp, QBrush(QColor(0, 0, 0, 128), Qt::SolidPattern));
				p.setPen(QColor(255, 255, 0, 255));
				p.drawText(image.rect(), Qt::AlignCenter | Qt::TextWordWrap, message);
				p.end();
			}
		} else if ( mPixmapCache.contains(cacheKey) ) {
			image = *mPixmapCache.object(cacheKey);
		} else {
			cacheKey = loadImage(id, CacheClassPixmap);
			if ( !cacheKey.isEmpty() )
				image = *mPixmapCache.object(cacheKey);
			else if ( cachePrefix == "ico" ) {
				image = QPixmap(QSize(1, 1));
				image.fill(Qt::transparent);
			} else
				image.load(QLatin1String(":/images/ghost.png"));
		}
	} else {
		if ( cachePrefix == "ico" ) {
			image = QPixmap(QSize(1, 1));
			image.fill(Qt::transparent);
		} else
			image.load(QLatin1String(":/images/ghost.png"));
	}

	if ( requestedSize.isValid() )
		result = image.scaled(requestedSize, Qt::KeepAspectRatio);
	else
		result = image;

	if ( size )
		*size = result.size();

	return result;
}

void ImageProvider::sevenZipDataReady()
{
	SevenZipFile *sevenZipFile = (SevenZipFile *)sender();
	if ( sevenZipFile ) {
		QString cachePrefix = sevenZipFile->userData().split("/", QString::SkipEmptyParts)[0];
		mAsyncMap.insert(sevenZipFile->fileName(), false);
		emit imageDataUpdated(cachePrefix);
	}
}

QString ImageProvider::loadImage(const QString &id)
{
	if ( id.isEmpty() )
		return QString();
	QString cacheKey(loadImage(id, CacheClassImage));
	if ( !cacheKey.isEmpty() )
		return cacheKey;
	cacheKey = loadImage(id, CacheClassPixmap);
	if ( !cacheKey.isEmpty() )
		return cacheKey;
	return QString();
}

QString ImageProvider::loadImage(const QString &id, const enum CacheClass cacheClass)
{
	if ( id.isEmpty() )
		return QString();
	QString validCacheKey, machineId, parentId, imageType;
	QStringList idWords(id.split("/", QString::SkipEmptyParts));
	if ( idWords.count() < 2 ) {
		if ( idWords.count() > 0 ) {
			if ( idWords.at(0).compare("ghost") == 0 ) // allows using "image://qmc2/ghost" to retrieve the Ghostbusters image
				return QString();
			else
				QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: ImageProvider::loadImage(): invalid image ID '%1' requested").arg(id));
		} else
			QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: ImageProvider::loadImage(): invalid image ID '%1' requested").arg(id));
	} else {
		imageType = idWords.at(0);
		machineId = idWords.at(1);
		if ( idWords.count() > 2 )
			parentId = idWords[2];
		QString cacheKey(imageType + "/" + machineId);
		switch ( cacheClass ) {
		case CacheClassImage:
			if ( mImageCache.contains(cacheKey) )
				validCacheKey = cacheKey;
			else {
				QImage image;
				foreach (QString imagePath, imageTypeToFile(imageType).split(";", QString::SkipEmptyParts)) {
					if ( isZippedImageType(imageType) ) {
						unzFile imageFile = mFileMapZip.value(imagePath);
						foreach (int format, mActiveFormatsMap.value(imageType)) {
							QString formatName(mFormatNames[format]);
							foreach (QString extension, mFormatExtensions[format].split(", ", QString::SkipEmptyParts)) {
								QString imageFileName(machineId + "." + extension);
								if ( imageFile && unzLocateFile(imageFile, imageFileName.toUtf8().constData(), 0) == UNZ_OK ) {
									QByteArray imageData;
									char imageBuffer[QMC2_ARCADE_ZIP_BUFSIZE];
									if ( unzOpenCurrentFile(imageFile) != UNZ_OK ) {
										QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: ImageProvider::loadImage(): unable to load image file '%1' from ZIP").arg(imageFileName));
									} else {
										int len = 0;
										while ( (len = unzReadCurrentFile(imageFile, &imageBuffer, QMC2_ARCADE_ZIP_BUFSIZE)) > 0 )
											imageData.append(imageBuffer, len);
										unzCloseCurrentFile(imageFile);
										if ( image.loadFromData(imageData, formatName.toUtf8().constData()) )  {
											mImageCache.insert(cacheKey, new QImage(image));
											validCacheKey = cacheKey;
										}
									}
								}
								if ( !validCacheKey.isEmpty() )
									break;
							}
							if ( !validCacheKey.isEmpty() )
								break;
						}
					} else if ( isSevenZippedImageType(imageType) ) {
						SevenZipFile *sevenZipFile = mFileMap7z.value(imagePath);
						foreach (int format, mActiveFormatsMap.value(imageType)) {
							QString formatName(mFormatNames[format]);
							foreach (QString extension, mFormatExtensions[format].split(", ", QString::SkipEmptyParts)) {
								QString imageFileName(machineId + "." + extension);
								int index = sevenZipFile->indexOfName(imageFileName);
								if ( index >= 0 ) {
									QByteArray imageData;
									bool async = true;
									int readLength = sevenZipFile->read(index, &imageData, &async);
									if ( readLength == 0 && async ) {
										validCacheKey = cacheKey;
										sevenZipFile->setUserData(cacheKey);
									} else if ( image.loadFromData(imageData, formatName.toUtf8().constData()) ) {
										mImageCache.insert(cacheKey, new QImage(image));
										validCacheKey = cacheKey;
									}
									mAsyncMap.insert(imagePath, async);
								}
								if ( !validCacheKey.isEmpty() )
									break;
							}
							if ( !validCacheKey.isEmpty() )
								break;
						}
					}
#if defined(QMC2_ARCADE_LIBARCHIVE_ENABLED)
					else if ( isArchivedImageType(imageType) ) {
						ArchiveFile *archiveFile = mArchiveMap.value(imagePath);
						foreach (int format, mActiveFormatsMap.value(imageType)) {
							QString formatName(mFormatNames[format]);
							foreach (QString extension, mFormatExtensions[format].split(", ", QString::SkipEmptyParts)) {
								QString imageFileName(machineId + "." + extension);
								if ( archiveFile->seekEntry(imageFileName) ) {
									QByteArray imageData;
									if ( archiveFile->readEntry(imageData) > 0 ) {
										if ( image.loadFromData(imageData, formatName.toUtf8().constData()) ) {
											mImageCache.insert(cacheKey, new QImage(image));
											validCacheKey = cacheKey;
										}
									}
								}
								if ( !validCacheKey.isEmpty() )
									break;
							}
							if ( !validCacheKey.isEmpty() )
								break;
						}
					}
#endif
					else {
						foreach (int format, mActiveFormatsMap.value(imageType)) {
							foreach (QString extension, mFormatExtensions[format].split(", ", QString::SkipEmptyParts)) {
								QString fileName(QFileInfo(imagePath + "/" + machineId + "." + extension).absoluteFilePath());
								QImage image;
								if ( image.load(fileName) ) {
									mImageCache.insert(cacheKey, new QImage(image));
									validCacheKey = cacheKey;
								}
								if ( !validCacheKey.isEmpty() )
									break;
							}
							if ( !validCacheKey.isEmpty() )
								break;
						}
						if ( !validCacheKey.isEmpty() )
							break;
					}
					if ( !validCacheKey.isEmpty() )
						break;
				}
			}
			break;

		case CacheClassPixmap:
			if ( mPixmapCache.contains(cacheKey) )
				validCacheKey = cacheKey;
			else {
				QPixmap image;
				foreach (QString imagePath, imageTypeToFile(imageType).split(";", QString::SkipEmptyParts)) {
					if ( isZippedImageType(imageType) ) {
						unzFile imageFile = mFileMapZip.value(imagePath);
						foreach (int format, mActiveFormatsMap.value(imageType)) {
							QString formatName(mFormatNames[format]);
							foreach (QString extension, mFormatExtensions[format].split(", ", QString::SkipEmptyParts)) {
								QString imageFileName(machineId + "." + extension);
								if ( imageFile && unzLocateFile(imageFile, imageFileName.toUtf8().constData(), 0) == UNZ_OK ) {
									QByteArray imageData;
									char imageBuffer[QMC2_ARCADE_ZIP_BUFSIZE];
									if ( unzOpenCurrentFile(imageFile) != UNZ_OK ) {
										QMC2_ARCADE_LOG_STR(QObject::tr("WARNING: ImageProvider::loadImage(): unable to load image file '%1' from ZIP").arg(imageFileName));
									} else {
										int len = 0;
										while ( (len = unzReadCurrentFile(imageFile, &imageBuffer, QMC2_ARCADE_ZIP_BUFSIZE)) > 0 )
											imageData.append(imageBuffer, len);
										unzCloseCurrentFile(imageFile);
										if ( image.loadFromData(imageData, formatName.toUtf8().constData()) )  {
											mPixmapCache.insert(cacheKey, new QPixmap(image));
											validCacheKey = cacheKey;
										}
									}
								}
								if ( !validCacheKey.isEmpty() )
									break;
							}
							if ( !validCacheKey.isEmpty() )
								break;
						}
					} else if ( isSevenZippedImageType(imageType) ) {
						SevenZipFile *sevenZipFile = mFileMap7z.value(imageType);
						foreach (int format, mActiveFormatsMap.value(imageType)) {
							QString formatName(mFormatNames[format]);
							foreach (QString extension, mFormatExtensions[format].split(", ", QString::SkipEmptyParts)) {
								QString imageFileName(machineId + "." + extension);
								int index = sevenZipFile->indexOfName(imageFileName);
								if ( index >= 0 ) {
									QByteArray imageData;
									bool async = true;
									int readLength = sevenZipFile->read(index, &imageData, &async);
									if ( readLength == 0 && async ) {
										validCacheKey = cacheKey;
										sevenZipFile->setUserData(cacheKey);
									} else if ( image.loadFromData(imageData, formatName.toUtf8().constData()) ) {
										mPixmapCache.insert(cacheKey, new QPixmap(image));
										validCacheKey = cacheKey;
									}
									mAsyncMap.insert(imagePath, async);
								}
								if ( !validCacheKey.isEmpty() )
									break;
							}
							if ( !validCacheKey.isEmpty() )
								break;
						}
					}
#if defined(QMC2_ARCADE_LIBARCHIVE_ENABLED)
					else if ( isArchivedImageType(imageType) ) {
						ArchiveFile *archiveFile = mArchiveMap.value(imagePath);
						foreach (int format, mActiveFormatsMap.value(imageType)) {
							QString formatName(mFormatNames[format]);
							foreach (QString extension, mFormatExtensions[format].split(", ", QString::SkipEmptyParts)) {
								QString imageFileName(machineId + "." + extension);
								if ( archiveFile->seekEntry(imageFileName) ) {
									QByteArray imageData;
									if ( archiveFile->readEntry(imageData) > 0 ) {
										if ( image.loadFromData(imageData, formatName.toUtf8().constData()) ) {
											mPixmapCache.insert(cacheKey, new QPixmap(image));
											validCacheKey = cacheKey;
										}
									}
								}
								if ( !validCacheKey.isEmpty() )
									break;
							}
							if ( !validCacheKey.isEmpty() )
								break;
						}
					}
#endif
					else {
						foreach (int format, mActiveFormatsMap.value(imageType)) {
							foreach (QString extension, mFormatExtensions[format].split(", ", QString::SkipEmptyParts)) {
								QString fileName(QFileInfo(imagePath + "/" + machineId + "." + extension).absoluteFilePath());
								QPixmap image;
								if ( image.load(fileName) ) {
									mPixmapCache.insert(cacheKey, new QPixmap(image));
									validCacheKey = cacheKey;
								}
								if ( !validCacheKey.isEmpty() )
									break;
							}
							if ( !validCacheKey.isEmpty() )
								break;
						}
						if ( !validCacheKey.isEmpty() )
							break;
					}
					if ( !validCacheKey.isEmpty() )
						break;
				}
			}
			break;
		}
	}

	if ( validCacheKey.isEmpty() ) {
		if ( globalConfig->parentFallback(imageType) && !parentId.isEmpty() && !imageType.isEmpty() )
			return loadImage(imageType + "/" + parentId, cacheClass);
		else
			return QString();
	} else
		return validCacheKey;
}

QString ImageProvider::imageTypeToFile(QString type)
{
	QString realType;
	if ( isZippedImageType(type) || isSevenZippedImageType(type) || isArchivedImageType(type) ) {
		switch ( mImageTypes.indexOf(type) ) {
		case QMC2_ARCADE_IMGTYPE_PREVIEW:
			return globalConfig->previewFile();
		case QMC2_ARCADE_IMGTYPE_FLYER:
			return globalConfig->flyerFile();
		case QMC2_ARCADE_IMGTYPE_CABINET:
			return globalConfig->cabinetFile();
		case QMC2_ARCADE_IMGTYPE_CONTROLLER:
			return globalConfig->controllerFile();
		case QMC2_ARCADE_IMGTYPE_MARQUEE:
			return globalConfig->marqueeFile();
		case QMC2_ARCADE_IMGTYPE_TITLE:
			return globalConfig->titleFile();
		case QMC2_ARCADE_IMGTYPE_PCB:
			return globalConfig->pcbFile();
		case QMC2_ARCADE_IMGTYPE_SWSNAP:
			return globalConfig->swSnapFile();
		case QMC2_ARCADE_IMGTYPE_ICON:
			return globalConfig->iconFile();
		default:
			realType = mCustomCachePrefixes.key(type);
			if ( mCustomImageTypes.contains(realType) )
				return globalConfig->customArtworkFile(realType);
			else
				return QString();
		}
	} else {
		switch ( mImageTypes.indexOf(type) ) {
		case QMC2_ARCADE_IMGTYPE_PREVIEW:
			return globalConfig->previewFolder();
		case QMC2_ARCADE_IMGTYPE_FLYER:
			return globalConfig->flyerFolder();
		case QMC2_ARCADE_IMGTYPE_CABINET:
			return globalConfig->cabinetFolder();
		case QMC2_ARCADE_IMGTYPE_CONTROLLER:
			return globalConfig->controllerFolder();
		case QMC2_ARCADE_IMGTYPE_MARQUEE:
			return globalConfig->marqueeFolder();
		case QMC2_ARCADE_IMGTYPE_TITLE:
			return globalConfig->titleFolder();
		case QMC2_ARCADE_IMGTYPE_PCB:
			return globalConfig->pcbFolder();
		case QMC2_ARCADE_IMGTYPE_SWSNAP:
			return globalConfig->swSnapFolder();
		case QMC2_ARCADE_IMGTYPE_ICON:
			return globalConfig->iconFolder();
		default:
			realType = mCustomCachePrefixes.key(type);
			if ( mCustomImageTypes.contains(realType) )
				return globalConfig->customArtworkFolder(realType);
			else
				return QString();
		}
	}
}

QString ImageProvider::imageTypeToLongName(QString type)
{
	switch ( mImageTypes.indexOf(type) ) {
	case QMC2_ARCADE_IMGTYPE_PREVIEW:
		return QObject::tr("preview");
	case QMC2_ARCADE_IMGTYPE_FLYER:
		return QObject::tr("flyer");
	case QMC2_ARCADE_IMGTYPE_CABINET:
		return QObject::tr("cabinet");
	case QMC2_ARCADE_IMGTYPE_CONTROLLER:
		return QObject::tr("controller");
	case QMC2_ARCADE_IMGTYPE_MARQUEE:
		return QObject::tr("marquee");
	case QMC2_ARCADE_IMGTYPE_TITLE:
		return QObject::tr("title");
	case QMC2_ARCADE_IMGTYPE_PCB:
		return QObject::tr("PCB");
	case QMC2_ARCADE_IMGTYPE_SWSNAP:
		return QObject::tr("software snapshot");
	case QMC2_ARCADE_IMGTYPE_ICON:
		return QObject::tr("icon");
	default:
		if ( mCustomImageTypes.contains(type) )
			return type.replace("&", QString());
		else
			return QString();
	}
}

bool ImageProvider::isZippedImageType(QString type)
{
	switch ( mImageTypes.indexOf(type) ) {
	case QMC2_ARCADE_IMGTYPE_PREVIEW:
		return globalConfig->previewsZipped();
	case QMC2_ARCADE_IMGTYPE_FLYER:
		return globalConfig->flyersZipped();
	case QMC2_ARCADE_IMGTYPE_CABINET:
		return globalConfig->cabinetsZipped();
	case QMC2_ARCADE_IMGTYPE_CONTROLLER:
		return globalConfig->controllersZipped();
	case QMC2_ARCADE_IMGTYPE_MARQUEE:
		return globalConfig->marqueesZipped();
	case QMC2_ARCADE_IMGTYPE_TITLE:
		return globalConfig->titlesZipped();
	case QMC2_ARCADE_IMGTYPE_PCB:
		return globalConfig->pcbsZipped();
	case QMC2_ARCADE_IMGTYPE_SWSNAP:
		return globalConfig->swSnapsZipped();
	case QMC2_ARCADE_IMGTYPE_ICON:
		return globalConfig->iconsZipped();
	default:
		if ( mCustomImageTypes.contains(type) )
			return globalConfig->customArtworkZipped(type);
		else
			return false;
	}
}

bool ImageProvider::isSevenZippedImageType(QString type)
{
	switch ( mImageTypes.indexOf(type) ) {
	case QMC2_ARCADE_IMGTYPE_PREVIEW:
		return globalConfig->previewsSevenZipped();
	case QMC2_ARCADE_IMGTYPE_FLYER:
		return globalConfig->flyersSevenZipped();
	case QMC2_ARCADE_IMGTYPE_CABINET:
		return globalConfig->cabinetsSevenZipped();
	case QMC2_ARCADE_IMGTYPE_CONTROLLER:
		return globalConfig->controllersSevenZipped();
	case QMC2_ARCADE_IMGTYPE_MARQUEE:
		return globalConfig->marqueesSevenZipped();
	case QMC2_ARCADE_IMGTYPE_TITLE:
		return globalConfig->titlesSevenZipped();
	case QMC2_ARCADE_IMGTYPE_PCB:
		return globalConfig->pcbsSevenZipped();
	case QMC2_ARCADE_IMGTYPE_SWSNAP:
		return globalConfig->swSnapsSevenZipped();
	case QMC2_ARCADE_IMGTYPE_ICON:
		return globalConfig->iconsSevenZipped();
	default:
		if ( mCustomImageTypes.contains(type) )
			return globalConfig->customArtworkSevenZipped(type);
		else
			return false;
	}
}

bool ImageProvider::isArchivedImageType(QString type)
{
	switch ( mImageTypes.indexOf(type) ) {
	case QMC2_ARCADE_IMGTYPE_PREVIEW:
		return globalConfig->previewsArchived();
	case QMC2_ARCADE_IMGTYPE_FLYER:
		return globalConfig->flyersArchived();
	case QMC2_ARCADE_IMGTYPE_CABINET:
		return globalConfig->cabinetsArchived();
	case QMC2_ARCADE_IMGTYPE_CONTROLLER:
		return globalConfig->controllersArchived();
	case QMC2_ARCADE_IMGTYPE_MARQUEE:
		return globalConfig->marqueesArchived();
	case QMC2_ARCADE_IMGTYPE_TITLE:
		return globalConfig->titlesArchived();
	case QMC2_ARCADE_IMGTYPE_PCB:
		return globalConfig->pcbsArchived();
	case QMC2_ARCADE_IMGTYPE_SWSNAP:
		return globalConfig->swSnapsArchived();
	case QMC2_ARCADE_IMGTYPE_ICON:
		return globalConfig->iconsArchived();
	default:
		if ( mCustomImageTypes.contains(type) )
			return globalConfig->customArtworkArchived(type);
		else
			return false;
	}
}

QString ImageProvider::customCachePrefix(QString name)
{
	if ( mCustomCachePrefixes.contains(name) )
		return mCustomCachePrefixes[name];
	else
		return QString();
}

QString ImageProvider::imageFolder(QString type)
{
	switch ( mImageTypes.indexOf(type) ) {
	case QMC2_ARCADE_IMGTYPE_PREVIEW:
		return globalConfig->previewFolder();
	case QMC2_ARCADE_IMGTYPE_FLYER:
		return globalConfig->flyerFolder();
	case QMC2_ARCADE_IMGTYPE_CABINET:
		return globalConfig->cabinetFolder();
	case QMC2_ARCADE_IMGTYPE_CONTROLLER:
		return globalConfig->controllerFolder();
	case QMC2_ARCADE_IMGTYPE_MARQUEE:
		return globalConfig->marqueeFolder();
	case QMC2_ARCADE_IMGTYPE_TITLE:
		return globalConfig->titleFolder();
	case QMC2_ARCADE_IMGTYPE_PCB:
		return globalConfig->pcbFolder();
	case QMC2_ARCADE_IMGTYPE_SWSNAP:
		return globalConfig->swSnapFolder();
	case QMC2_ARCADE_IMGTYPE_ICON:
		return globalConfig->iconFolder();
	default:
		if ( mCustomImageTypes.contains(type) )
			return globalConfig->customArtworkFolder(type);
		else
			return QString();
	}
}

bool ImageProvider::isAsync(QString type)
{
	bool async = false;
	foreach (QString filePath, mFileTypeMap.keys()) {
		if ( mFileTypeMap.value(filePath).compare(type) == 0 )
			if ( mAsyncMap.value(filePath) ) {
				async = true;
				break;
			}
	}
	return async;
}
