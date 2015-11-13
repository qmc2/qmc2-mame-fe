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

extern bool qmc2UseSoftwareSnapFile;
extern QCache<QString, ImagePixmap> qmc2ImagePixmapCache;
extern SoftwareSnap *qmc2SoftwareSnap;
extern SoftwareList *qmc2SoftwareList;
extern MainWindow *qmc2MainWindow;
extern Options *qmc2Options;
extern Settings *qmc2Config;
extern QHash<QString, QString> softwareParentHash;
extern bool qmc2ShowGameName;
extern bool qmc2SmoothScaling;
extern bool qmc2ParentImageFallback;
extern bool qmc2RetryLoadingImages;

SoftwareSnapshot::SoftwareSnapshot(QWidget *parent)
#if QMC2_OPENGL == 1
	: QGLWidget(parent)
#else
	: QWidget(parent)
#endif
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

	reloadActiveFormats();
}

SoftwareSnapshot::~SoftwareSnapshot()
{
	if ( qmc2UseSoftwareSnapFile ) {
		foreach (unzFile snapFile, qmc2SoftwareSnap->snapFileMap)
			unzClose(snapFile);
		foreach (SevenZipFile *snapFile, qmc2SoftwareSnap->snapFileMap7z) {
			snapFile->close();
			delete snapFile;
		}
		qmc2SoftwareSnap->snapFileMap.clear();
		qmc2SoftwareSnap->snapFileMap7z.clear();
	}
}

void SoftwareSnapshot::paintEvent(QPaintEvent *e)
{
	QPainter p(this);

	if ( !qmc2SoftwareList->currentItem ) {
		drawCenteredImage(0, &p); // clear snapshot widget
		myCacheKey.clear();
		return;
	}

	QString listName = qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_LIST);
	QString entryName = qmc2SoftwareList->currentItem->text(QMC2_SWLIST_COLUMN_NAME);
	myCacheKey = "sws_" + listName + "_" + entryName;

	ImagePixmap *cpm = qmc2ImagePixmapCache.object(myCacheKey);
	if ( !cpm )
		loadSnapshot(listName, entryName);
	else {
		currentSnapshotPixmap = *cpm;
		currentSnapshotPixmap.imagePath = cpm->imagePath;
	}

	drawScaledImage(&currentSnapshotPixmap, &p);
}

QString SoftwareSnapshot::toBase64()
{
	ImagePixmap pm;
	if ( !currentSnapshotPixmap.isNull() )
		pm = currentSnapshotPixmap;
	else
		pm = qmc2MainWindow->qmc2GhostImagePixmap;
	QByteArray imageData;
	QBuffer buffer(&imageData);
	pm.save(&buffer, "PNG");
	return QString(imageData.toBase64());
}

void SoftwareSnapshot::refresh()
{
	if ( !myCacheKey.isEmpty() ) {
		qmc2ImagePixmapCache.remove(myCacheKey);
		update();
	}
}

void SoftwareSnapshot::sevenZipDataReady()
{
	update();
	enableWidgets(true);
}

void SoftwareSnapshot::enableWidgets(bool enable)
{
	qmc2Options->radioButtonSoftwareSnapSelect->setEnabled(enable);
	qmc2Options->lineEditSoftwareSnapFile->setEnabled(enable);
	qmc2Options->comboBoxSoftwareSnapFileType->setEnabled(enable);
	qmc2Options->toolButtonBrowseSoftwareSnapFile->setEnabled(enable);
}

bool SoftwareSnapshot::loadSnapshot(QString listName, QString entryName, bool fromParent)
{
	ImagePixmap pm;
	bool fileOk = true;

	currentSnapshotPixmap.imagePath.clear();
	myCacheKey = "sws_" + listName + "_" + entryName;

	if ( fromParent ) {
		QString parentKey = softwareParentHash[listName + ":" + entryName];
		if ( !parentKey.isEmpty() && parentKey != "<no_parent>" ) {
			QString parentName = parentKey.split(":", QString::SkipEmptyParts)[1];
			entryName = parentName;
		}
	}

	if ( qmc2UseSoftwareSnapFile ) {
		if ( qmc2SoftwareSnap->useZip() ) {
			// try loading image from (semicolon-separated) ZIP archive(s)
			if ( qmc2SoftwareSnap->snapFileMap.isEmpty() ) {
				foreach (QString filePath, qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareSnapFile").toString().split(";", QString::SkipEmptyParts)) {
					qmc2SoftwareSnap->snapFileMap[filePath] = unzOpen(filePath.toUtf8().constData());
					if ( qmc2SoftwareSnap->snapFileMap[filePath] == NULL )
						qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open software snap-shot file, please check access permissions for %1").arg(filePath));
				}
			}
			foreach (unzFile snapFile, qmc2SoftwareSnap->snapFileMap) {
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
		} else  if ( qmc2SoftwareSnap->useSevenZip() ) {
			// try loading image from (semicolon-separated) 7z archive(s)
			if ( qmc2SoftwareSnap->snapFileMap7z.isEmpty() ) {
				foreach (QString filePath, qmc2Config->value(QMC2_EMULATOR_PREFIX + "FilesAndDirectories/SoftwareSnapFile").toString().split(";", QString::SkipEmptyParts)) {
					SevenZipFile *snapFile = new SevenZipFile(filePath);
					if ( !snapFile->open() ) {
						  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open software snap-shot file %1").arg(filePath) + " - " + tr("7z error") + ": " + snapFile->lastError());
						  delete snapFile;
					} else {
						qmc2SoftwareSnap->snapFileMap7z[filePath] = snapFile;
						connect(snapFile, SIGNAL(dataReady()), this, SLOT(sevenZipDataReady()));
					}
				}
			}
			foreach (SevenZipFile *snapFile, qmc2SoftwareSnap->snapFileMap7z) {
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
									currentSnapshotPixmap = qmc2MainWindow->qmc2GhostImagePixmap;
									QPainter p;
									QString message = tr("Decompressing archive, please wait...");
									p.begin(&currentSnapshotPixmap);
									p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::HighQualityAntialiasing | QPainter::SmoothPixmapTransform);
									QFont f(qApp->font());
									f.setWeight(QFont::Bold);
									f.setPointSize(f.pointSize() * 2);
									QFontMetrics fm(f);
									int adjustment = fm.height() / 2;
									p.setFont(f);
									QRect outerRect = p.boundingRect(currentSnapshotPixmap.rect(), Qt::AlignCenter | Qt::TextWordWrap, message).adjusted(-adjustment, -adjustment, adjustment, adjustment);
									QPainterPath pp;
									pp.addRoundedRect(outerRect, 5, 5);
									p.fillPath(pp, QBrush(QColor(0, 0, 0, 128), Qt::SolidPattern));
									p.setPen(QColor(255, 255, 0, 255));
									p.drawText(currentSnapshotPixmap.rect(), Qt::AlignCenter | Qt::TextWordWrap, message);
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
							currentSnapshotPixmap = pm;
							currentSnapshotPixmap.imagePath = filePath;
							qmc2ImagePixmapCache.insert(myCacheKey, new ImagePixmap(currentSnapshotPixmap), currentSnapshotPixmap.toImage().byteCount()); 
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
			return loadSnapshot(listName, entryName, true);
		currentSnapshotPixmap = qmc2MainWindow->qmc2GhostImagePixmap;
		if ( !qmc2RetryLoadingImages )
			qmc2ImagePixmapCache.insert(myCacheKey, new ImagePixmap(currentSnapshotPixmap), currentSnapshotPixmap.toImage().byteCount());
        }

	return fileOk;
}

void SoftwareSnapshot::drawCenteredImage(QPixmap *pm, QPainter *p)
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

void SoftwareSnapshot::drawScaledImage(QPixmap *pm, QPainter *p)
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

void SoftwareSnapshot::copyToClipboard()
{
	qApp->clipboard()->setPixmap(currentSnapshotPixmap);
}

void SoftwareSnapshot::copyPathToClipboard()
{
	if ( !currentSnapshotPixmap.imagePath.isEmpty() )
		qApp->clipboard()->setText(currentSnapshotPixmap.imagePath);
}

void SoftwareSnapshot::contextMenuEvent(QContextMenuEvent *e)
{
	actionCopyPathToClipboard->setVisible(!currentSnapshotPixmap.imagePath.isEmpty());
	contextMenu->move(qmc2MainWindow->adjustedWidgetPosition(mapToGlobal(e->pos()), contextMenu));
	contextMenu->show();
}

void SoftwareSnapshot::reloadActiveFormats()
{
	activeFormats.clear();
	QStringList imgFmts = qmc2Config->value(QMC2_FRONTEND_PREFIX + "ActiveImageFormats/sws", QStringList()).toStringList();
	if ( imgFmts.isEmpty() )
		activeFormats << QMC2_IMAGE_FORMAT_INDEX_PNG;
	else for (int i = 0; i < imgFmts.count(); i++)
		activeFormats << imgFmts[i].toInt();
}
