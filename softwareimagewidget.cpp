#include <QBuffer>
#include <QCache>
#include <QHash>
#include <QDir>
#include <QClipboard>

#include "softwarelist.h"
#include "softwaresnapshot.h"
#include "qmc2main.h"
#include "options.h"
#include "settings.h"
#include "macros.h"

extern QCache<QString, ImagePixmap> qmc2ImagePixmapCache;
extern SoftwareList *qmc2SoftwareList;
extern MainWindow *qmc2MainWindow;
extern Options *qmc2Options;
extern Settings *qmc2Config;
extern QHash<QString, QString> softwareParentHash;
extern bool qmc2ShowGameName;
extern bool qmc2SmoothScaling;
extern bool qmc2ParentImageFallback;
extern bool qmc2RetryLoadingImages;

QHash<int, SoftwareImageWidget *> SoftwareImageWidget::artworkHash;

SoftwareImageWidget::SoftwareImageWidget(QWidget *parent)
	: QWidget(parent)
{
	QTimer::singleShot(0, this, SLOT(init()));
}

SoftwareImageWidget::~SoftwareImageWidget()
{
	closeSource();
}

void SoftwareImageWidget::init()
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

void SoftwareImageWidget::updateArtwork()
{
	QHashIterator<int, SoftwareImageWidget *> it(artworkHash);
	while ( it.hasNext() ) {
		it.next();
		it.value()->update();
	}
}

void SoftwareImageWidget::reloadArtworkFormats()
{
	QHashIterator<int, SoftwareImageWidget *> it(artworkHash);
	while ( it.hasNext() ) {
		it.next();
		it.value()->reloadActiveFormats();
	}
}

void SoftwareImageWidget::openSource()
{
	if ( useZip() ) {
		foreach (QString filePath, imageZip().split(";", QString::SkipEmptyParts)) {
			unzFile imageFile = unzOpen(filePath.toUtf8().constData());
			if ( imageFile == NULL )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open %1 file, please check access permissions for %2").arg(imageType()).arg(imageZip()));
			else
				imageFileMap[filePath] = imageFile;
		}
	} else if ( useSevenZip() ) {
		foreach (QString filePath, imageZip().split(";", QString::SkipEmptyParts)) {
			SevenZipFile *imageFile = new SevenZipFile(filePath);
			if ( !imageFile->open() )
				qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open %1 file, please check access permissions for %2").arg(imageType()).arg(imageZip()));
			else {
				connect(imageFile, SIGNAL(dataReady()), this, SLOT(sevenZipDataReady()));
				imageFileMap7z[filePath] = imageFile;
			}
		}
	}
	reloadActiveFormats();
}

void SoftwareImageWidget::closeSource()
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
}

void SoftwareImageWidget::paintEvent(QPaintEvent *e)
{
	QPainter p(this);

	if ( !qmc2SoftwareList->currentItem ) {
		drawCenteredImage(0, &p); // clear snapshot widget
		myCacheKey.clear();
		return;
	}

	QString listName = qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_LIST);
	QString entryName = qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_NAME);
	myCacheKey = cachePrefix() + "_" + listName + "_" + entryName;

	ImagePixmap *cpm = qmc2ImagePixmapCache.object(myCacheKey);
	if ( !cpm )
		loadImage(listName, entryName);
	else {
		currentPixmap = *cpm;
		currentPixmap.imagePath = cpm->imagePath;
	}

	if ( scaledImage() || currentPixmap.isGhost )
		drawScaledImage(&currentPixmap, &p);
	else
		drawCenteredImage(&currentPixmap, &p);
}

QString SoftwareImageWidget::toBase64()
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

void SoftwareImageWidget::refresh()
{
	if ( !myCacheKey.isEmpty() ) {
		qmc2ImagePixmapCache.remove(myCacheKey);
		update();
	}
}

void SoftwareImageWidget::sevenZipDataReady()
{
	update();
	enableWidgets(true);
}

void SoftwareImageWidget::enableWidgets(bool enable)
{
	if ( customArtwork() )
		return;
	qmc2Options->radioButtonSoftwareSnapSelect->setEnabled(enable);
	qmc2Options->lineEditSoftwareSnapFile->setEnabled(enable);
	qmc2Options->comboBoxSoftwareSnapFileType->setEnabled(enable);
	qmc2Options->toolButtonBrowseSoftwareSnapFile->setEnabled(enable);
}

bool SoftwareImageWidget::loadImage(QString listName, QString entryName, bool fromParent)
{
	ImagePixmap pm;
	bool fileOk = true;

	absoluteImagePath().clear();
	myCacheKey = cachePrefix() + "_" + listName + "_" + entryName;

	if ( fromParent ) {
		QString parentKey = softwareParentHash[listName + ":" + entryName];
		if ( !parentKey.isEmpty() && parentKey != "<no_parent>" ) {
			QString parentName = parentKey.split(":", QString::SkipEmptyParts)[1];
			entryName = parentName;
		}
	}

	if ( useZip() ) {
		// try loading image from (semicolon-separated) ZIP archive(s)
		foreach (unzFile snapFile, imageFileMap) {
			if ( snapFile ) {
				foreach (int format, activeFormats) {
					QString formatName = ImageWidget::formatNames[format];
					foreach (QString extension, ImageWidget::formatExtensions[format].split(", ", QString::SkipEmptyParts)) {
						QByteArray imageData;
						QString pathInZip = listName + "/" + entryName + "." + extension;
						if ( unzLocateFile(snapFile, pathInZip.toUtf8().constData(), 0) == UNZ_OK ) {
							if ( unzOpenCurrentFile(snapFile) == UNZ_OK ) {
								char imageBuffer[QMC2_ZIP_BUFFER_SIZE];
								int len;
								while ( (len = unzReadCurrentFile(snapFile, &imageBuffer, QMC2_ZIP_BUFFER_SIZE)) > 0 )
									imageData.append(imageBuffer, len);
								unzCloseCurrentFile(snapFile);
								fileOk = true;
							} else
								fileOk = false;
						} else
							fileOk = false;

						if ( fileOk ) {
							if ( pm.loadFromData(imageData, formatName.toUtf8().constData()) ) {
								qmc2ImagePixmapCache.insert(myCacheKey, new ImagePixmap(pm), pm.toImage().byteCount());
								break;
							} else
								fileOk = false;
						}

						if ( fileOk )
							break;
					}

					if ( fileOk )
						break;
				}
			}

			if ( fileOk )
				break;
		}
	} else if ( useSevenZip() ) {
		// try loading image from (semicolon-separated) 7z archive(s)
		foreach (SevenZipFile *snapFile, imageFileMap7z) {
			if ( snapFile ) {
				QByteArray imageData;
				foreach (int format, activeFormats) {
					QString formatName = ImageWidget::formatNames[format];
					foreach (QString extension, ImageWidget::formatExtensions[format].split(", ", QString::SkipEmptyParts)) {
						bool isFillingDictionary = false;
						QString pathIn7z = listName + "/" + entryName + "." + extension;
						int index = snapFile->indexOfName(pathIn7z);
						if ( index >= 0 ) {
							m_async = true;
							quint64 readLength = snapFile->read(index, &imageData, &m_async);
							if ( readLength == 0 && m_async ) {
								qmc2ImagePixmapCache.remove(myCacheKey);
								isFillingDictionary = true;
								fileOk = true;
							} else
								fileOk = !snapFile->hasError();
						} else
							fileOk = false;

						if ( fileOk ) {
							if ( isFillingDictionary ) {
								currentPixmap = qmc2MainWindow->qmc2GhostImagePixmap;
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
							} else if ( pm.loadFromData(imageData, formatName.toUtf8().constData()) ) {
								qmc2ImagePixmapCache.insert(myCacheKey, new ImagePixmap(pm), pm.toImage().byteCount());
								break;
							} else
								fileOk = false;
						}
					}

					if ( fileOk )
						break;
				}
			}

			if ( fileOk )
				break;
		}
	} else {
		// try loading image from (semicolon-separated) software-snapshot folder(s)
		fileOk = false;
		foreach (QString baseDirectory, qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareSnapDirectory").toString().split(";", QString::SkipEmptyParts)) {
			QDir snapDir(baseDirectory + "/" + listName);
			foreach (int format, activeFormats) {
				foreach (QString extension, ImageWidget::formatExtensions[format].split(", ", QString::SkipEmptyParts)) {
					QString fullEntryName = entryName + "." + extension;
					if ( snapDir.exists(fullEntryName) ) {
						QString filePath = snapDir.absoluteFilePath(fullEntryName);
						if ( pm.load(filePath) ) {
							fileOk = true;
							currentPixmap = pm;
							currentPixmap.imagePath = filePath;
							qmc2ImagePixmapCache.insert(myCacheKey, new ImagePixmap(currentPixmap), currentPixmap.toImage().byteCount()); 
						} else
							fileOk = false;
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

	if ( !fileOk ) {
		if ( qmc2ParentImageFallback && !fromParent )
			return loadImage(listName, entryName, true);
		currentPixmap = qmc2MainWindow->qmc2GhostImagePixmap;
		if ( !qmc2RetryLoadingImages )
			qmc2ImagePixmapCache.insert(myCacheKey, new ImagePixmap(currentPixmap), currentPixmap.toImage().byteCount());
        }

	return fileOk;
}

void SoftwareImageWidget::drawCenteredImage(QPixmap *pm, QPainter *p)
{
	p->eraseRect(rect());

	if ( pm == NULL ) {
		p->end();
		return;
	}

	// last resort if pm->load() retrieved a null pixmap...
	if ( pm->isNull() )
		pm = &qmc2MainWindow->qmc2GhostImagePixmap;

	int posx = (rect().width() - pm->width()) / 2;
	int posy = (rect().height() - pm->height()) / 2;

	p->drawPixmap(posx, posy, *pm);

	if ( qmc2ShowGameName ) {
		// draw entry title
		p->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);
		QString title = qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_TITLE);
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

void SoftwareImageWidget::drawScaledImage(QPixmap *pm, QPainter *p)
{
	if ( pm == NULL ) {
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

void SoftwareImageWidget::copyToClipboard()
{
	qApp->clipboard()->setPixmap(currentPixmap);
}

void SoftwareImageWidget::copyPathToClipboard()
{
	if ( !absoluteImagePath().isEmpty() )
		qApp->clipboard()->setText(absoluteImagePath());
}

void SoftwareImageWidget::contextMenuEvent(QContextMenuEvent *e)
{
	actionCopyPathToClipboard->setVisible(!absoluteImagePath().isEmpty());
	contextMenu->move(qmc2MainWindow->adjustedWidgetPosition(mapToGlobal(e->pos()), contextMenu));
	contextMenu->show();
}

void SoftwareImageWidget::reloadActiveFormats()
{
	activeFormats.clear();
	QStringList imgFmts = qmc2Config->value(QMC2_FRONTEND_PREFIX + "ActiveImageFormats/sws", QStringList()).toStringList();
	if ( imgFmts.isEmpty() )
		activeFormats << QMC2_IMAGE_FORMAT_INDEX_PNG;
	else for (int i = 0; i < imgFmts.count(); i++)
		activeFormats << imgFmts[i].toInt();
}

QString SoftwareImageWidget::cleanDir(QString dirs)
{
	QStringList dirList;
	foreach (QString dir, dirs.split(";", QString::SkipEmptyParts)) {
		if ( !dir.endsWith("/") )
			dir += "/";
		dirList << dir;
	}
	return dirList.join(";");
}
