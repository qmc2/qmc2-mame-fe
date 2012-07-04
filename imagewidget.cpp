#include <QPixmapCache>
#include <QPixmap>
#include <QImage>
#include <QImageReader>
#include <QDir>
#include <QByteArray>
#include <QBuffer>
#include <QMap>
#include <QClipboard>
#include <QTreeWidgetItem>

#include "imagewidget.h"
#include "qmc2main.h"
#include "macros.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern bool qmc2SmoothScaling;
extern bool qmc2RetryLoadingImages;
extern bool qmc2ParentImageFallback;
extern bool qmc2ShowGameName;
extern bool qmc2ShowGameNameOnlyWhenRequired;
extern QTreeWidgetItem *qmc2CurrentItem;
extern QMap<QString, QString> qmc2ParentMap;
extern QMap<QString, QString> qmc2GamelistDescriptionMap;

ImageWidget::ImageWidget(QWidget *parent)
#if QMC2_OPENGL == 1
	: QGLWidget(parent)
#else
	: QWidget(parent)
#endif
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ImageWidget::ImageWidget(QWidget *parent = %1)").arg((qulonglong)parent));
#endif

	contextMenu = new QMenu(this);
	contextMenu->hide();

	QString s;
	QAction *action;

	s = tr("Copy to clipboard");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/editcopy.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(copyToClipboard()));
	s = tr("Refresh");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/reload.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(refresh()));

	imageFile = NULL;
	if ( useZip() ) {
		imageFile = unzOpen((const char *)imageZip().toLocal8Bit());
		if ( imageFile == NULL )
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open %1 file, please check access permissions for %2").arg(imageType()).arg(imageZip()));
	}
}

ImageWidget::~ImageWidget()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageWidget::~ImageWidget()");
#endif

	if ( useZip() )
		unzClose(imageFile);
}

QString ImageWidget::cleanDir(QString dirs)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ImageWidget::cleanDir(QString dirs = %1)").arg(dirs));
#endif

	QStringList dirList;
	foreach (QString dir, dirs.split(";", QString::SkipEmptyParts)) {
		if ( !dir.endsWith("/") )
			dir += "/";
		dirList << dir;
	}
	return dirList.join(";");
}

void ImageWidget::paintEvent(QPaintEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ImageWidget::paintEvent(QPaintEvent *e = %1)").arg((qulonglong)e));
#endif

	QPainter p(this);

	if ( !qmc2CurrentItem ) {
		drawCenteredImage(0, &p); // clear image widget
		return;
	}

	if ( qmc2CurrentItem->text(QMC2_GAMELIST_COLUMN_GAME) == tr("Waiting for data...") ) {
		drawCenteredImage(0, &p); // clear image widget
		return;
	}

	QTreeWidgetItem *topLevelItem = qmc2CurrentItem;
	while ( topLevelItem->parent() )
		topLevelItem = topLevelItem->parent();

	QString gameName = topLevelItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON);

	if ( !QPixmapCache::find(cachePrefix() + gameName, &currentPixmap) ) {
		qmc2CurrentItem = topLevelItem;
		loadImage(gameName, gameName);
	}

	cacheKey = cachePrefix() + gameName;

	if ( scaledImage() )
		drawScaledImage(&currentPixmap, &p);
	else
		drawCenteredImage(&currentPixmap, &p);
}

void ImageWidget::refresh()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageWidget::refresh()");
#endif

	if ( !cacheKey.isEmpty() ) {
		QPixmapCache::remove(cacheKey);
		repaint();
	}
}

bool ImageWidget::loadImage(QString gameName, QString onBehalfOf, bool checkOnly, QString *fileName)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ImageWidget::loadImage(QString gameName = %1, QString onBehalfOf = %2, bool checkOnly = %3, QString *fileName = %4)").arg(gameName).arg(onBehalfOf).arg(checkOnly).arg((qulonglong)fileName));
#endif

	QPixmap pm;
	char imageBuffer[QMC2_ZIP_BUFFER_SIZE];

	if ( fileName )
		*fileName = "";

	bool fileOk = true;

	if ( useZip() ) {
		// try loading image from ZIP
		QByteArray imageData;
		int len, i;
		QString gameFile = gameName + ".png";

		if ( fileName )
			*fileName = gameFile;

		if ( unzLocateFile(imageFile, (const char *)gameFile.toLocal8Bit(), 0) == UNZ_OK ) {
			if ( unzOpenCurrentFile(imageFile) == UNZ_OK ) {
				while ( (len = unzReadCurrentFile(imageFile, &imageBuffer, QMC2_ZIP_BUFFER_SIZE)) > 0 ) {
					for (i = 0; i < len; i++)
						imageData += imageBuffer[i];
				}
				unzCloseCurrentFile(imageFile);
			} else
				fileOk = false;
		} else
			fileOk = false;

		if ( fileOk )
			fileOk = pm.loadFromData(imageData, "PNG");

		if ( !checkOnly ) {
			if ( fileOk ) {
				QPixmapCache::insert(onBehalfOf, pm);
				currentPixmap = pm;
			} else {
				QString parentName = qmc2ParentMap[gameName];
				if ( qmc2ParentImageFallback && !parentName.isEmpty() ) {
					fileOk = loadImage(parentName, onBehalfOf);
				} else {
					if ( !qmc2RetryLoadingImages )
						QPixmapCache::insert(onBehalfOf, qmc2MainWindow->qmc2GhostImagePixmap); 
					currentPixmap = qmc2MainWindow->qmc2GhostImagePixmap;
				}
			}
		}
	} else {
		// try loading image from (semicolon-separated) folder(s)
		foreach (QString baseDirectory, imageDir().split(";", QString::SkipEmptyParts)) {
			QString imgDir = baseDirectory + gameName;
			QString imagePath = imgDir + ".png";

			if ( fileName )
				*fileName = gameName + ".png";

			QFile f(imagePath);
			if ( !f.exists() ) {
				QDir dir(imgDir);
				if ( dir.exists() ) {
					QStringList nameFilter;
					nameFilter << "*.png";
					QStringList dirEntries = dir.entryList(nameFilter, QDir::Files | QDir::NoDotAndDotDot | QDir::Readable | QDir::CaseSensitive, QDir::Name | QDir::Reversed);
					if ( dirEntries.count() > 0 ) {
						imagePath = imgDir + "/" + dirEntries[0];
						QString pathCopy = imagePath;
						if ( fileName )
							*fileName = pathCopy.remove(baseDirectory);
					}
				}
			}

			if ( checkOnly ) {
				fileOk = pm.load(imagePath, "PNG");
			} else {
				if ( pm.load(imagePath, "PNG") ) {
					QPixmapCache::insert(onBehalfOf, pm); 
					currentPixmap = pm;
					fileOk = true;
				} else {
					QString parentName = qmc2ParentMap[gameName];
					if ( qmc2ParentImageFallback && !parentName.isEmpty() ) {
						fileOk = loadImage(parentName, onBehalfOf);
					} else {
						if ( !qmc2RetryLoadingImages )
							QPixmapCache::insert(onBehalfOf, qmc2MainWindow->qmc2GhostImagePixmap); 
						currentPixmap = qmc2MainWindow->qmc2GhostImagePixmap;
						fileOk = false;
					}
				}
			}

			if ( fileOk )
				break;
		}
	}

	return fileOk;
}

bool ImageWidget::checkImage(QString gameName, unzFile zip, QSize *sizeReturn, int *bytesUsed, QString *fileName, QString *readerError)
{
	QImage image;
	char imageBuffer[QMC2_ZIP_BUFFER_SIZE];

	if ( fileName )
		*fileName = "";

	bool fileOk = true;

	if ( useZip() ) {
		// try loading image from ZIP
		QByteArray imageData;
		int len, i;
		QString gameFile = gameName + ".png";

		if ( fileName )
			*fileName = gameFile;

		if ( zip == NULL )
			zip = imageFile;

		if ( unzLocateFile(zip, (const char *)gameFile.toLocal8Bit(), 0) == UNZ_OK ) {
			if ( unzOpenCurrentFile(zip) == UNZ_OK ) {
				while ( (len = unzReadCurrentFile(zip, &imageBuffer, QMC2_ZIP_BUFFER_SIZE)) > 0 ) {
					for (i = 0; i < len; i++)
						imageData += imageBuffer[i];
				}
				unzCloseCurrentFile(zip);
			} else
				fileOk = false;
		} else
			fileOk = false;

		if ( fileOk ) {
			QBuffer buffer(&imageData);
			QImageReader imageReader(&buffer, "PNG");
			fileOk = imageReader.read(&image);
			if ( fileOk ) {
				if ( sizeReturn )
					*sizeReturn = image.size();
				if ( bytesUsed )
					*bytesUsed = image.byteCount();
			} else if ( readerError != NULL && imageReader.error() != QImageReader::FileNotFoundError )
				*readerError = imageReader.errorString();
		}
	} else {
		// try loading image from (semicolon-separated) folder(s)
		foreach (QString baseDirectory, imageDir().split(";", QString::SkipEmptyParts)) {
			QString imgDir = baseDirectory + gameName;
			QString imagePath = imgDir + ".png";

			if ( fileName )
				*fileName = QDir::toNativeSeparators(imagePath);

			QImageReader imageReader(imagePath, "PNG");
			fileOk = imageReader.read(&image);

			if ( fileOk ) {
				if ( sizeReturn )
					*sizeReturn = image.size();
				if ( bytesUsed )
					*bytesUsed = image.byteCount();
				break;
			} else if ( readerError != NULL && imageReader.error() != QImageReader::FileNotFoundError )
				*readerError = imageReader.errorString();
		}
	}

	return fileOk;
}

void ImageWidget::drawCenteredImage(QPixmap *pm, QPainter *p)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ImageWidget::drawCenteredImage(QPixmap *pm = %1, QPainter *p = %2)").arg((qulonglong)pm).arg((qulonglong)p));
#endif

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

	bool drawGameName = false;
	if ( qmc2ShowGameName ) {
		if ( qmc2ShowGameNameOnlyWhenRequired ) {
			if ( qmc2MainWindow->hSplitter->sizes()[0] == 0 || qmc2MainWindow->tabWidgetGamelist->currentIndex() != QMC2_GAMELIST_INDEX ) {
				drawGameName = true;
			} else {
				drawGameName = false;
			}
		} else
			drawGameName = true;
	} else
		drawGameName = false;

	if ( drawGameName ) {
		// draw game/machine title
		QString title = qmc2GamelistDescriptionMap[qmc2CurrentItem->text(QMC2_GAMELIST_COLUMN_NAME)];
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
		p->setPen(QColor(255, 255, 255, 0));
		p->fillRect(r, QBrush(QColor(0, 0, 0, 128), Qt::SolidPattern));
		p->setPen(QPen(QColor(255, 255, 255, 255)));
		p->drawText(r, Qt::AlignCenter | Qt::TextWordWrap, title);
	}

	p->end();
}

void ImageWidget::drawScaledImage(QPixmap *pm, QPainter *p)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ImageWidget::drawScaledImage(QPixmap *pm = %1, QPainter *p = %2)").arg((qulonglong)pm).arg((qulonglong)p));
#endif

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

void ImageWidget::copyToClipboard()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: ImageWidget::copyToClipboard()");
#endif

	qApp->clipboard()->setPixmap(currentPixmap);
}

void ImageWidget::contextMenuEvent(QContextMenuEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: ImageWidget::contextMenuEvent(QContextMenuEvent *e = %1)").arg((qulonglong)e));
#endif

	contextMenu->move(qmc2MainWindow->adjustedWidgetPosition(mapToGlobal(e->pos()), contextMenu));
	contextMenu->show();
}
