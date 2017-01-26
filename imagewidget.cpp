#include <QPixmap>
#include <QImage>
#include <QImageReader>
#include <QDir>
#include <QByteArray>
#include <QBuffer>
#include <QTimer>
#include <QMap>
#include <QClipboard>
#include <QCache>
#include <QTreeWidgetItem>
#include <QPainterPath>

#include "settings.h"
#include "options.h"
#include "imagewidget.h"
#include "customartwork.h"
#include "qmc2main.h"
#include "machinelist.h"
#include "macros.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;
extern Options *qmc2Options;
extern bool qmc2SmoothScaling;
extern bool qmc2RetryLoadingImages;
extern bool qmc2ParentImageFallback;
extern bool qmc2ShowMachineName;
extern bool qmc2ShowMachineNameOnlyWhenRequired;
extern QTreeWidgetItem *qmc2CurrentItem;
extern QHash<QString, QString> qmc2ParentHash;
extern QCache<QString, ImagePixmap> qmc2ImagePixmapCache;

QStringList ImageWidget::formatNames;
QStringList ImageWidget::formatExtensions;
QStringList ImageWidget::formatDescriptions;
QHash<int, ImageWidget *> ImageWidget::artworkHash;

ImageWidget::ImageWidget(QWidget *parent)
	: QWidget(parent)
{
	if ( formatNames.isEmpty() )
		formatNames << "PNG" << "BMP" << "GIF" << "JPG" << "PBM" << "PGM" << "PPM" << "TIFF" << "XBM" << "XPM" << "SVG" << "TGA";
	if ( formatExtensions.isEmpty() )
		formatExtensions << "png" << "bmp" << "gif" << "jpg, jpeg" << "pbm" << "pgm" << "ppm" << "tif, tiff" << "xbm" << "xpm" << "svg" << "tga";
	if ( formatDescriptions.isEmpty() )
		formatDescriptions << tr("Portable Network Graphics") << tr("Windows Bitmap") << tr("Graphic Interchange Format") << tr("Joint Photographic Experts Group") << tr("Portable Bitmap")
				   << tr("Portable Graymap") << tr("Portable Pixmap") << tr("Tagged Image File Format") << tr("X11 Bitmap") << tr("X11 Pixmap") << tr("Scalable Vector Graphics") << tr("Targa Image Format");
	QTimer::singleShot(0, this, SLOT(init()));
}

ImageWidget::~ImageWidget()
{
	closeSource();
}

ImageWidget *ImageWidget::customArtworkWidget(QString name)
{
	foreach (ImageWidget *imw, artworkHash)
		if ( imw->customArtwork() )
			if ( ((CustomArtwork *)imw)->name() == name )
				return imw;
	return 0;
}

void ImageWidget::init()
{
	contextMenu = new QMenu(this);
	contextMenu->hide();

	QString s;
	QAction *action;

	s = tr("Copy image to clipboard");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/editcopy.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(copyToClipboard()));

	s = tr("Copy file path to clipboard");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/editcopy.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(copyPathToClipboard()));
	actionCopyPathToClipboard = action;

	contextMenu->addSeparator();

	s = tr("Refresh cache slot");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/reload.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(refresh()));

	openSource();
}

void ImageWidget::updateArtwork()
{
	QHashIterator<int, ImageWidget *> it(artworkHash);
	while ( it.hasNext() ) {
		it.next();
		it.value()->update();
	}
}

void ImageWidget::reloadArtworkFormats()
{
	QHashIterator<int, ImageWidget *> it(artworkHash);
	while ( it.hasNext() ) {
		it.next();
		it.value()->reloadActiveFormats();
	}
}

void ImageWidget::openSource()
{
	if ( useZip() ) {
		foreach (QString filePath, imageZip().split(';', QString::SkipEmptyParts)) {
			unzFile imageFile = unzOpen(filePath.toUtf8().constData());
			if ( imageFile == 0 )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open %1 file, please check access permissions for %2").arg(imageType()).arg(imageZip()));
			else
				imageFileMap.insert(filePath, imageFile);
		}
	} else if ( useSevenZip() ) {
		foreach (QString filePath, imageZip().split(';', QString::SkipEmptyParts)) {
			SevenZipFile *imageFile = new SevenZipFile(filePath);
			if ( !imageFile->open() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open %1 file, please check access permissions for %2").arg(imageType()).arg(imageZip()));
			else {
				connect(imageFile, SIGNAL(dataReady()), this, SLOT(sevenZipDataReady()));
				imageFileMap7z.insert(filePath, imageFile);
			}
		}
	}
#if defined(QMC2_LIBARCHIVE_ENABLED)
	else if ( useArchive() ) {
		foreach (QString filePath, imageZip().split(';', QString::SkipEmptyParts)) {
			ArchiveFile *imageFile = new ArchiveFile(filePath);
			if ( !imageFile->open() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open %1 file, please check access permissions for %2").arg(imageType()).arg(imageZip()));
			else
				imageArchiveMap.insert(filePath, imageFile);
		}
	}
#endif
	reloadActiveFormats();
}

void ImageWidget::closeSource()
{
	QMapIterator<QString, unzFile> itZip(imageFileMap);
	while ( itZip.hasNext() ) {
		itZip.next();
		unzClose(itZip.value());
	}
	imageFileMap.clear();
	QMapIterator<QString, SevenZipFile*> it7z(imageFileMap7z);
	while ( it7z.hasNext() ) {
		it7z.next();
		it7z.value()->close();
		delete it7z.value();
	}
	imageFileMap7z.clear();
#if defined(QMC2_LIBARCHIVE_ENABLED)
	QMapIterator<QString, ArchiveFile*> itArchive(imageArchiveMap);
	while ( itArchive.hasNext() ) {
		itArchive.next();
		itArchive.value()->close();
		delete itArchive.value();
	}
	imageArchiveMap.clear();
#endif
}

bool ImageWidget::parentFallback()
{
	return qmc2ParentImageFallback && qmc2Config->value(fallbackSettingsKey(), 0).toInt() == 0;
}

void ImageWidget::reloadActiveFormats()
{
	activeFormats.clear();
	QStringList imgFmts;
	if ( customArtwork() )
		imgFmts = qmc2Config->value(QString("Artwork/%1/ActiveFormats").arg(imageType()), QStringList()).toStringList();
	else
		imgFmts = qmc2Config->value(QMC2_FRONTEND_PREFIX + QString("ActiveImageFormats/%1").arg(cachePrefix()), QStringList()).toStringList();
	if ( imgFmts.isEmpty() )
		activeFormats << QMC2_IMAGE_FORMAT_INDEX_PNG;
	else for (int i = 0; i < imgFmts.count(); i++)
		activeFormats << imgFmts.at(i).toInt();
}

QString ImageWidget::cleanDir(QString dirs)
{
	QStringList dirList;
	foreach (QString dir, dirs.split(';', QString::SkipEmptyParts)) {
		if ( !dir.endsWith('/') )
			dir += '/';
		dirList << dir;
	}
	return dirList.join(";");
}

void ImageWidget::paintEvent(QPaintEvent *e)
{
	QPainter p(this);

	if ( !qmc2CurrentItem ) {
		drawCenteredImage(0, &p); // clear image widget
		return;
	}

	if ( qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_MACHINE) == MachineList::trWaitingForData ) {
		drawCenteredImage(0, &p); // clear image widget
		return;
	}

	QTreeWidgetItem *topLevelItem = qmc2CurrentItem;
	while ( topLevelItem->parent() )
		topLevelItem = topLevelItem->parent();

	QString machineName(topLevelItem->text(QMC2_MACHINELIST_COLUMN_NAME));
	cacheKey = cachePrefix() + '_' + machineName;
	ImagePixmap *cpm = qmc2ImagePixmapCache.object(cacheKey);
	if ( !cpm ) {
		qmc2CurrentItem = topLevelItem;
		loadImage(machineName, machineName);
		cpm = qmc2ImagePixmapCache.object(cacheKey);
	}
	if ( cpm ) {
		currentPixmap = *cpm;
		currentPixmap.imagePath = cpm->imagePath;
	}
	if ( scaledImage() || currentPixmap.isGhost )
		drawScaledImage(&currentPixmap, &p);
	else
		drawCenteredImage(&currentPixmap, &p);
}

void ImageWidget::refresh()
{
	if ( !cacheKey.isEmpty() ) {
		qmc2ImagePixmapCache.remove(cacheKey);
		update();
	}
}

void ImageWidget::sevenZipDataReady()
{
	update();
	enableWidgets(true);
}

void ImageWidget::enableWidgets(bool enable)
{
	if ( customArtwork() )
		return;
	switch ( imageTypeNumeric() ) {
		case QMC2_IMGTYPE_PREVIEW:
			qmc2Options->radioButtonPreviewSelect->setEnabled(enable);
			qmc2Options->lineEditPreviewFile->setEnabled(enable);
			qmc2Options->comboBoxPreviewFileType->setEnabled(enable);
			qmc2Options->toolButtonBrowsePreviewFile->setEnabled(enable);
			break;
		case QMC2_IMGTYPE_FLYER:
			qmc2Options->radioButtonFlyerSelect->setEnabled(enable);
			qmc2Options->lineEditFlyerFile->setEnabled(enable);
			qmc2Options->comboBoxFlyerFileType->setEnabled(enable);
			qmc2Options->toolButtonBrowseFlyerFile->setEnabled(enable);
			break;
		case QMC2_IMGTYPE_CABINET:
			qmc2Options->radioButtonCabinetSelect->setEnabled(enable);
			qmc2Options->lineEditCabinetFile->setEnabled(enable);
			qmc2Options->comboBoxCabinetFileType->setEnabled(enable);
			qmc2Options->toolButtonBrowseCabinetFile->setEnabled(enable);
			break;
		case QMC2_IMGTYPE_CONTROLLER:
			qmc2Options->radioButtonControllerSelect->setEnabled(enable);
			qmc2Options->lineEditControllerFile->setEnabled(enable);
			qmc2Options->comboBoxControllerFileType->setEnabled(enable);
			qmc2Options->toolButtonBrowseControllerFile->setEnabled(enable);
			break;
		case QMC2_IMGTYPE_MARQUEE:
			qmc2Options->radioButtonMarqueeSelect->setEnabled(enable);
			qmc2Options->lineEditMarqueeFile->setEnabled(enable);
			qmc2Options->comboBoxMarqueeFileType->setEnabled(enable);
			qmc2Options->toolButtonBrowseMarqueeFile->setEnabled(enable);
			break;
		case QMC2_IMGTYPE_TITLE:
			qmc2Options->radioButtonTitleSelect->setEnabled(enable);
			qmc2Options->lineEditTitleFile->setEnabled(enable);
			qmc2Options->comboBoxTitleFileType->setEnabled(enable);
			qmc2Options->toolButtonBrowseTitleFile->setEnabled(enable);
			break;
		case QMC2_IMGTYPE_PCB:
			qmc2Options->radioButtonPCBSelect->setEnabled(enable);
			qmc2Options->lineEditPCBFile->setEnabled(enable);
			qmc2Options->comboBoxPCBFileType->setEnabled(enable);
			qmc2Options->toolButtonBrowsePCBFile->setEnabled(enable);
			break;
	}
}

bool ImageWidget::loadImage(const QString &machineName, const QString &onBehalfOf, bool checkOnly, QString *fileName, bool loadImages)
{
	ImagePixmap pm;
	char imageBuffer[QMC2_ZIP_BUFFER_SIZE];
	if ( fileName )
		*fileName = "";
	bool fileOk = true;
	QString cacheKey = cachePrefix() + "_" + onBehalfOf;
	if ( useZip() ) {
		// try loading image from (semicolon-separated) ZIP archive(s)
		QByteArray imageData;
		int len;
		foreach (int format, activeFormats) {
			QString formatName(formatNames.value(format));
			foreach (QString extension, formatExtensions.value(format).split(", ", QString::SkipEmptyParts)) {
				QString machineFile(machineName + '.' + extension);
				if ( fileName )
					*fileName = machineFile;
				foreach (unzFile imageFile, imageFileMap) {
					if ( unzLocateFile(imageFile, machineFile.toUtf8().constData(), 0) == UNZ_OK ) {
						if ( unzOpenCurrentFile(imageFile) == UNZ_OK ) {
							while ( (len = unzReadCurrentFile(imageFile, &imageBuffer, QMC2_ZIP_BUFFER_SIZE)) > 0 )
								imageData.append(imageBuffer, len);
							fileOk = true;
							unzCloseCurrentFile(imageFile);
						} else
							fileOk = false;
					} else
						fileOk = false;

					if ( fileOk )
						break;
					else
						imageData.clear();
				}
				if ( fileOk )
					fileOk = pm.loadFromData(imageData, formatName.toUtf8().constData());
				if ( !checkOnly ) {
					if ( fileOk ) {
#if defined(QMC2_DEBUG)
						QMC2_PRINT_STRTXT(QString("ZIP: Image loaded for %1").arg(cacheKey));
#endif
						qmc2ImagePixmapCache.insert(cacheKey, new ImagePixmap(pm), pm.toImage().byteCount());
						currentPixmap = pm;
					} else {
						QString parentName(qmc2ParentHash.value(machineName));
						if ( parentFallback() && !parentName.isEmpty() ) {
							fileOk = loadImage(parentName, onBehalfOf);
						} else {
							currentPixmap = qmc2MainWindow->qmc2GhostImagePixmap;
							if ( !qmc2RetryLoadingImages )
								qmc2ImagePixmapCache.insert(cacheKey, new ImagePixmap(currentPixmap), currentPixmap.toImage().byteCount()); 
#if defined(QMC2_DEBUG)
							QMC2_PRINT_STRTXT(QString("ZIP: Using ghost image for %1").arg(cacheKey));
#endif
						}
					}
				}
				if ( fileOk )
					break;
			}
			if ( fileOk )
				break;
		}
	} else if ( useSevenZip() ) {
		// try loading image from (semicolon-separated) 7z archive(s)
		QByteArray imageData;
		foreach (int format, activeFormats) {
			QString formatName(formatNames.value(format));
			foreach (QString extension, formatExtensions.value(format).split(", ", QString::SkipEmptyParts)) {
				QString machineFile(machineName + '.' + extension);
				if ( fileName )
					*fileName = machineFile;
				bool isFillingDictionary = false;
				foreach (SevenZipFile *imageFile, imageFileMap7z) {
					int index = imageFile->indexOfName(machineFile);
					if ( index >= 0 ) {
						m_async = true;
						quint64 readLength = imageFile->read(index, &imageData, &m_async);
						if ( readLength == 0 && m_async ) {
							currentPixmap = qmc2MainWindow->qmc2GhostImagePixmap;
							qmc2ImagePixmapCache.remove(cacheKey);
							isFillingDictionary = true;
							fileOk = true;
						} else
							fileOk = !imageFile->hasError();
					} else
						fileOk = false;

					if ( fileOk )
						break;
					else
						imageData.clear();
				}
				if ( fileOk )
					fileOk = pm.loadFromData(imageData, formatName.toUtf8().constData());
				if ( !checkOnly ) {
					if ( fileOk ) {
#if defined(QMC2_DEBUG)
						QMC2_PRINT_STRTXT(QString("7z: Image loaded for %1").arg(cacheKey));
#endif
						qmc2ImagePixmapCache.insert(cacheKey, new ImagePixmap(pm), pm.toImage().byteCount());
						currentPixmap = pm;
					} else {
						QString parentName = qmc2ParentHash.value(machineName);
						if ( parentFallback() && !parentName.isEmpty() ) {
							fileOk = loadImage(parentName, onBehalfOf);
						} else {
							currentPixmap = qmc2MainWindow->qmc2GhostImagePixmap;
							if ( !qmc2RetryLoadingImages && !isFillingDictionary )
								qmc2ImagePixmapCache.insert(cacheKey, new ImagePixmap(currentPixmap), currentPixmap.toImage().byteCount()); 
							else {
								QPainter p;
								QString message = tr("Decompressing archive, please wait...");
								p.begin(&currentPixmap);
								p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);
								QFont f(qApp->font());
								f.setWeight(QFont::Bold);
								f.setPointSize(f.pointSize() * 2);
								QFontMetrics fm(f);
								int adjustment = fm.height() / 2;
								p.setFont(f);
								QRect outerRect = p.boundingRect(currentPixmap.rect(), Qt::AlignCenter | Qt::TextWordWrap, message).adjusted(-adjustment, -adjustment, adjustment, adjustment);
								QPainterPath pp;
								pp.addRoundedRect(outerRect, 5, 5);
								p.fillPath(pp, QBrush(QColor(0, 0, 0, 128), Qt::SolidPattern));
								p.setPen(QColor(255, 255, 0, 255));
								p.drawText(currentPixmap.rect(), Qt::AlignCenter | Qt::TextWordWrap, message);
								p.end();
								enableWidgets(false);
							}
#if defined(QMC2_DEBUG)
							QMC2_PRINT_STRTXT(QString("7z: Using ghost image for %1%2").arg(cacheKey).arg(isFillingDictionary ? " (filling up dictionary)" : ""));
#endif
							if ( isFillingDictionary )
								QTimer::singleShot(QMC2_IMG_7Z_DICT_FILL_DELAY, this, SLOT(update()));
						}
					}
				}
				if ( fileOk )
					break;
			}
			if ( fileOk )
				break;
		}
        }
#if defined(QMC2_LIBARCHIVE_ENABLED)
	else if ( useArchive() ) {
		// try loading image from (semicolon-separated) archive(s)
		QByteArray imageData;
		foreach (int format, activeFormats) {
			QString formatName(formatNames.value(format));
			foreach (QString extension, formatExtensions.value(format).split(", ", QString::SkipEmptyParts)) {
				QString machineFile(machineName + '.' + extension);
				if ( fileName )
					*fileName = machineFile;
				foreach (ArchiveFile *imageFile, imageArchiveMap) {
					if ( imageFile->seekEntry(machineFile) )
						fileOk = imageFile->readEntry(imageData) > 0;
					else
						fileOk = false;
					if ( fileOk )
						break;
				}
				if ( fileOk )
					fileOk = pm.loadFromData(imageData, formatName.toUtf8().constData());
				if ( !checkOnly ) {
					if ( fileOk ) {
#if defined(QMC2_DEBUG)
						QMC2_PRINT_STRTXT(QString("Archive: Image loaded for %1").arg(cacheKey));
#endif
						qmc2ImagePixmapCache.insert(cacheKey, new ImagePixmap(pm), pm.toImage().byteCount());
						currentPixmap = pm;
					} else {
						QString parentName(qmc2ParentHash.value(machineName));
						if ( parentFallback() && !parentName.isEmpty() ) {
							fileOk = loadImage(parentName, onBehalfOf);
						} else {
							currentPixmap = qmc2MainWindow->qmc2GhostImagePixmap;
							if ( !qmc2RetryLoadingImages )
								qmc2ImagePixmapCache.insert(cacheKey, new ImagePixmap(currentPixmap), currentPixmap.toImage().byteCount()); 
#if defined(QMC2_DEBUG)
							QMC2_PRINT_STRTXT(QString("Archive: Using ghost image for %1").arg(cacheKey));
#endif
						}
					}
				}
				if ( fileOk )
					break;
			}
			if ( fileOk )
				break;
		}
	}
#endif
	else {
		// try loading image from (semicolon-separated) folder(s)
		foreach (QString baseDirectory, imageDir().split(';', QString::SkipEmptyParts)) {
			QString imgDir(QDir::cleanPath(baseDirectory + '/' + machineName));
			foreach (int format, activeFormats) {
				QString formatName(formatNames.value(format));
				foreach (QString extension, formatExtensions.value(format).split(", ", QString::SkipEmptyParts)) {
					QString imagePath(imgDir + '.' + extension);
					if ( fileName )
						*fileName = imagePath;
					QFile f(imagePath);
					if ( !f.exists() ) {
						QDir dir(imgDir);
						if ( dir.exists() ) {
							QStringList nameFilter;
							nameFilter << "*." + extension;
							QStringList dirEntries = dir.entryList(nameFilter, QDir::Files | QDir::NoDotAndDotDot | QDir::Readable | QDir::CaseSensitive, QDir::Name | QDir::Reversed);
							if ( dirEntries.count() > 0 ) {
								imagePath = imgDir + '/' + dirEntries.first();
								if ( fileName )
									*fileName = imagePath;
							}
						}
					}
					if ( checkOnly ) {
						if ( loadImages )
							fileOk = pm.load(imagePath, formatName.toUtf8().constData());
						else {
							QFile f(imagePath);
							fileOk = f.exists();
							if ( !fileOk ) {
								QString parentName(qmc2ParentHash.value(machineName));
								if ( parentFallback() && !parentName.isEmpty() )
									fileOk = loadImage(parentName, onBehalfOf, checkOnly, fileName, false);
							}
						}
					} else {
						if ( pm.load(imagePath, formatName.toUtf8().constData()) ) {
							pm.imagePath = imagePath;
#if defined(QMC2_DEBUG)
							QMC2_PRINT_STRTXT(QString("Folder: Image loaded for %1").arg(cacheKey));
#endif
							qmc2ImagePixmapCache.insert(cacheKey, new ImagePixmap(pm), pm.toImage().byteCount());
							currentPixmap = pm;
							fileOk = true;
						} else {
							QString parentName(qmc2ParentHash.value(machineName));
							if ( parentFallback() && !parentName.isEmpty() ) {
								fileOk = loadImage(parentName, onBehalfOf);
							} else {
								currentPixmap = qmc2MainWindow->qmc2GhostImagePixmap;
								if ( !qmc2RetryLoadingImages )
									qmc2ImagePixmapCache.insert(cacheKey, new ImagePixmap(currentPixmap), currentPixmap.toImage().byteCount()); 
#if defined(QMC2_DEBUG)
								QMC2_PRINT_STRTXT(QString("Folder: Using ghost image for %1").arg(cacheKey));
#endif
								fileOk = false;
							}
						}
					}
					if ( fileOk )
						break;
				}
				if ( fileOk )
					break;
			}
			if ( fileOk )
				break;
		}
	}
	return fileOk;
}

QString ImageWidget::primaryPathFor(QString machineName)
{
	if ( !useZip() && !useSevenZip() ) {
		QStringList fl(imageDir().split(';', QString::SkipEmptyParts));
		QString baseDirectory;
		if ( !fl.isEmpty() )
			baseDirectory = fl.first();
		return QDir::toNativeSeparators(QDir::cleanPath(baseDirectory + '/' + machineName + ".png"));
	} else // we don't support on-the-fly image replacement for zipped images yet!
		return QString();
}

bool ImageWidget::replaceImage(QString machineName, QPixmap &pixmap)
{
	if ( !useZip() && !useSevenZip() ) {
		QString savePath = primaryPathFor(machineName);
		if ( !savePath.isEmpty() ) {
			bool goOn = true;
			if ( QFile::exists(savePath) ) {
				QString backupPath = savePath + ".bak";
				if ( QFile::exists(backupPath) )
					QFile::remove(backupPath);
				if ( !QFile::copy(savePath, backupPath) ) {
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't create backup of existing image file '%1' as '%2'").arg(savePath).arg(backupPath));
					goOn = false;
				}
			}
			if ( goOn ) {
				QString primaryPath = QFileInfo(savePath).absoluteDir().absolutePath();
				QDir ppDir(primaryPath);
				if ( !ppDir.exists() )
					ppDir.mkpath(primaryPath);
				if ( pixmap.save(savePath, "PNG") ) {
					currentPixmap = pixmap;
					currentPixmap.imagePath = savePath;
					update();
					return true;
				} else {
					qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't create image file '%1'").arg(savePath));
					return false;
				}
			} else
				return false;
		} else {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't determine primary path for image-type '%1'").arg(imageType()));
			return false;
		}
	} else // we don't support on-the-fly image replacement for zipped and 7-zipped images yet!
		return false;
}

#if defined(QMC2_LIBARCHIVE_ENABLED)
bool ImageWidget::checkImage(QString machineName, unzFile zip, SevenZipFile *sevenZip, ArchiveFile *archiveFile, QSize *sizeReturn, int *bytesUsed, QString *fileName, QString *readerError, bool *async, bool *isFillingDict)
#else
bool ImageWidget::checkImage(QString machineName, unzFile zip, SevenZipFile *sevenZip, QSize *sizeReturn, int *bytesUsed, QString *fileName, QString *readerError, bool *async, bool *isFillingDict)
#endif
{
	QImage image;
	char imageBuffer[QMC2_ZIP_BUFFER_SIZE];

	if ( fileName )
		fileName->clear();
	bool fileOk = true;
	if ( useZip() ) {
		// try loading image from (semicolon-separated) ZIP archive(s)
		QByteArray imageData;
		int len;
		foreach (int format, activeFormats) {
			QString formatName(formatNames.value(format));
			foreach (QString extension, formatExtensions.value(format).split(", ", QString::SkipEmptyParts)) {
				QString machineFile(machineName + '.' + extension);
				if ( fileName )
					*fileName = machineFile;
				if ( zip == 0 ) {
					foreach (unzFile imageFile, imageFileMap) {
						if ( unzLocateFile(imageFile, machineFile.toUtf8().constData(), 0) == UNZ_OK ) {
							if ( unzOpenCurrentFile(imageFile) == UNZ_OK ) {
								while ( (len = unzReadCurrentFile(imageFile, &imageBuffer, QMC2_ZIP_BUFFER_SIZE)) > 0 )
									imageData.append(imageBuffer, len);
								fileOk = true;
								unzCloseCurrentFile(imageFile);
							} else
								fileOk = false;
						} else
							fileOk = false;
						if ( fileOk )
							break;
						else
							imageData.clear();
					}
				} else {
					if ( unzLocateFile(zip, machineFile.toUtf8().constData(), 0) == UNZ_OK ) {
						if ( unzOpenCurrentFile(zip) == UNZ_OK ) {
							while ( (len = unzReadCurrentFile(zip, &imageBuffer, QMC2_ZIP_BUFFER_SIZE)) > 0 )
								imageData.append(imageBuffer, len);
							fileOk = true;
							unzCloseCurrentFile(zip);
						} else
							fileOk = false;
					} else
						fileOk = false;
				}
				if ( fileOk ) {
					QBuffer buffer(&imageData);
					QImageReader imageReader(&buffer, formatName.toUtf8().constData());
					fileOk = imageReader.read(&image);
					if ( fileOk ) {
						if ( sizeReturn )
							*sizeReturn = image.size();
						if ( bytesUsed )
							*bytesUsed = image.byteCount();
					} else if ( readerError != 0 && imageReader.error() != QImageReader::FileNotFoundError )
						*readerError = imageReader.errorString();
				}
				if ( fileOk )
					break;
			}
			if ( fileOk )
				break;
		}
	} else if ( useSevenZip() ) {
		// try loading image from (semicolon-separated) 7z archive(s)
		QByteArray imageData;
		foreach (int format, activeFormats) {
			QString formatName(formatNames.value(format));
			foreach (QString extension, formatExtensions.value(format).split(", ", QString::SkipEmptyParts)) {
				QString machineFile(machineName + '.' + extension);
				if ( fileName )
					*fileName = machineFile;
				if ( isFillingDict )
					*isFillingDict = false;
				if ( sevenZip == 0 ) {
					foreach (SevenZipFile *imageFile, imageFileMap7z) {
						int index = imageFile->indexOfName(machineFile);
						if ( index >= 0 ) {
							m_async = true;
							quint64 readLength = imageFile->read(index, &imageData, &m_async);
							if ( readLength == 0 && m_async ) {
								if ( isFillingDict )
									*isFillingDict = true;
								fileOk = true;
							} else
								fileOk = !imageFile->hasError();
						} else
							fileOk = false;
						if ( fileOk )
							break;
						else
							imageData.clear();
					}
				} else {
					int index = sevenZip->indexOfName(machineFile);
					if ( index >= 0 ) {
						if ( async )
							*async = true;
						quint64 readLength = sevenZip->read(index, &imageData, async);
						if ( readLength == 0 && (async && *async) ) {
							if ( isFillingDict )
								*isFillingDict = true;
							fileOk = true;
						} else
							fileOk = !sevenZip->hasError();
					} else
						fileOk = false;
					if ( !fileOk )
						imageData.clear();
				}
				bool ifd = isFillingDict ? *isFillingDict : false;
				if ( fileOk && !ifd ) {
					QBuffer buffer(&imageData);
					QImageReader imageReader(&buffer, formatName.toUtf8().constData());
					fileOk = imageReader.read(&image);
					if ( fileOk ) {
						if ( sizeReturn )
							*sizeReturn = image.size();
						if ( bytesUsed )
							*bytesUsed = image.byteCount();
					} else if ( readerError != 0 && imageReader.error() != QImageReader::FileNotFoundError )
						*readerError = imageReader.errorString();
				}
				if ( fileOk )
					break;
			}
			if ( fileOk )
				break;
		}
	}
#if defined(QMC2_LIBARCHIVE_ENABLED)
	else if ( useArchive() ) {
		// try loading image from (semicolon-separated) archive(s)
		QByteArray imageData;
		foreach (int format, activeFormats) {
			QString formatName(formatNames.value(format));
			foreach (QString extension, formatExtensions.value(format).split(", ", QString::SkipEmptyParts)) {
				QString machineFile(machineName + '.' + extension);
				if ( fileName )
					*fileName = machineFile;
				if ( archiveFile == 0 ) {
					foreach (ArchiveFile *imageFile, imageArchiveMap) {
						if ( imageFile->seekEntry(machineFile) )
							fileOk = imageFile->readEntry(imageData) > 0;
						else
							fileOk = false;
						if ( fileOk )
							break;
					}
				} else {
					if ( archiveFile->seekEntry(machineFile) )
						fileOk = archiveFile->readEntry(imageData) > 0;
					else
						fileOk = false;
				}
				if ( fileOk ) {
					QBuffer buffer(&imageData);
					QImageReader imageReader(&buffer, formatName.toUtf8().constData());
					fileOk = imageReader.read(&image);
					if ( fileOk ) {
						if ( sizeReturn )
							*sizeReturn = image.size();
						if ( bytesUsed )
							*bytesUsed = image.byteCount();
					} else if ( readerError != 0 && imageReader.error() != QImageReader::FileNotFoundError )
						*readerError = imageReader.errorString();
				}
				if ( fileOk )
					break;
			}
			if ( fileOk )
				break;
		}
	}
#endif
	else {
		// try loading image from (semicolon-separated) folder(s)
		foreach (QString baseDirectory, imageDir().split(';', QString::SkipEmptyParts)) {
			QString imgDir(baseDirectory + machineName);
			foreach (int format, activeFormats) {
				QString formatName(formatNames.value(format));
				foreach (QString extension, formatExtensions.value(format).split(", ", QString::SkipEmptyParts)) {
					QString localImagePath(imgDir + '.' + extension);
					if ( fileName )
						*fileName = QDir::toNativeSeparators(localImagePath);
					QImageReader imageReader(localImagePath, formatName.toUtf8().constData());
					fileOk = imageReader.read(&image);
					if ( fileOk ) {
						if ( sizeReturn )
							*sizeReturn = image.size();
						if ( bytesUsed )
							*bytesUsed = image.byteCount();
						break;
					} else if ( readerError != 0 && imageReader.error() != QImageReader::FileNotFoundError )
						*readerError = imageReader.errorString();
					if ( fileOk )
						break;
				}
				if ( fileOk )
					break;
			}
			if ( fileOk )
				break;
		}
	}
	return fileOk;
}

void ImageWidget::drawCenteredImage(QPixmap *pm, QPainter *p)
{
	p->eraseRect(rect());

	if ( pm == 0 ) {
		p->end();
		return;
	}

	// last resort if pm->load() retrieved a null pixmap...
	if ( pm->isNull() )
		pm = &qmc2MainWindow->qmc2GhostImagePixmap;

	int posx = (rect().width() - pm->width()) / 2;
	int posy = (rect().height() - pm->height()) / 2;

	p->drawPixmap(posx, posy, *pm);

	bool drawMachineName = false;
	if ( qmc2ShowMachineName ) {
		if ( qmc2ShowMachineNameOnlyWhenRequired ) {
			if ( qmc2MainWindow->hSplitter->sizes()[0] == 0 || qmc2MainWindow->tabWidgetMachineList->currentIndex() != QMC2_MACHINELIST_INDEX ) {
				drawMachineName = true;
			} else {
				drawMachineName = false;
			}
		} else
			drawMachineName = true;
	} else
		drawMachineName = false;

	if ( drawMachineName ) {
		// draw game/machine title
		p->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);
		QString title = qmc2CurrentItem->text(QMC2_MACHINELIST_COLUMN_MACHINE);
		QFont f(qApp->font());
		f.setWeight(QFont::Bold);
		p->setFont(f);
		QFontMetrics fm(f);
		QRect r = rect();
		int adjustment = fm.height() / 2;
		r = r.adjusted(+adjustment, +adjustment, -adjustment, -adjustment);
		QRect outerRect = p->boundingRect(r, Qt::AlignCenter | Qt::TextWordWrap, title);
		r.setTop(r.bottom() - outerRect.height());
		r = p->boundingRect(r, Qt::AlignCenter | Qt::TextWordWrap, title);
		r = r.adjusted(-adjustment, -adjustment, +adjustment, +adjustment);
		r.setBottom(rect().bottom());
		QPainterPath pp;
		pp.addRoundedRect(r, 5, 5);
		p->fillPath(pp, QBrush(QColor(0, 0, 0, 128), Qt::SolidPattern));
		p->setPen(QPen(QColor(255, 255, 255, 255)));
		p->drawText(r, Qt::AlignCenter | Qt::TextWordWrap, title);
	}

	p->end();
}

void ImageWidget::drawScaledImage(QPixmap *pm, QPainter *p)
{
	if ( pm == 0 ) {
		p->eraseRect(rect());
		p->end();
		return;
	}

	// last resort if pm->load() retrieved a null pixmap...
	if ( pm->isNull() )
		pm = &qmc2MainWindow->qmc2GhostImagePixmap;

	double desired_width;
	double desired_height;

	if ( pm->width() > pm->height() ) {
		desired_width  = contentsRect().width();
		desired_height = (double)pm->height() * (desired_width / (double)pm->width());
		if ( desired_height > contentsRect().height() ) {
			desired_height = contentsRect().height();
			desired_width  = (double)pm->width() * (desired_height / (double)pm->height());
		}
	} else {
		desired_height = contentsRect().height();
		desired_width  = (double)pm->width() * (desired_height / (double)pm->height());
		if ( desired_width > contentsRect().width() ) {
			desired_width = contentsRect().width();
			desired_height = (double)pm->height() * (desired_width / (double)pm->width());
		}
	}

	QPixmap pmScaled;

	if ( qmc2SmoothScaling )
		pmScaled = pm->scaled((int)desired_width, (int)desired_height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	else
		pmScaled = pm->scaled((int)desired_width, (int)desired_height, Qt::KeepAspectRatio, Qt::FastTransformation);

	drawCenteredImage(&pmScaled, p);
}

void ImageWidget::copyToClipboard()
{
	if ( !currentPixmap.isNull() )
		qApp->clipboard()->setPixmap(currentPixmap);
}

void ImageWidget::copyPathToClipboard()
{
	if ( !absoluteImagePath().isEmpty() )
		qApp->clipboard()->setText(absoluteImagePath());
}

void ImageWidget::contextMenuEvent(QContextMenuEvent *e)
{
	actionCopyPathToClipboard->setVisible(!absoluteImagePath().isEmpty());
	contextMenu->move(qmc2MainWindow->adjustedWidgetPosition(mapToGlobal(e->pos()), contextMenu));
	contextMenu->show();
}

QString ImageWidget::toBase64()
{
	ImagePixmap pm;
	if ( !currentPixmap.isNull() )
		pm = currentPixmap;
	else
		pm = qmc2MainWindow->qmc2GhostImagePixmap;
	QByteArray imageData;
	QBuffer buffer(&imageData);
	pm.save(&buffer, "PNG");
	return QString(imageData.toBase64());
}
