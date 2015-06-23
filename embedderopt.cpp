#include "embedderopt.h"
#include "embedder.h"

#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)

#include <QCache>
#include "machinelist.h"
#include "qmc2main.h"
#include "preview.h"
#include "flyer.h"
#include "cabinet.h"
#include "controller.h"
#include "marquee.h"
#include "title.h"
#include "pcb.h"
#include "softwarelist.h"
#include "processmanager.h"
#include "options.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;
extern MachineList *qmc2MachineList;
extern Settings *qmc2Config;
extern QCache<QString, ImagePixmap> qmc2ImagePixmapCache;
extern Preview *qmc2Preview;
extern Flyer *qmc2Flyer;
extern Cabinet *qmc2Cabinet;
extern Controller *qmc2Controller;
extern Marquee *qmc2Marquee;
extern Title *qmc2Title;
extern PCB *qmc2PCB;
extern SoftwareSnap *qmc2SoftwareSnap;
extern bool qmc2UseSoftwareSnapFile;
extern ProcessManager *qmc2ProcessManager;
extern Options *qmc2Options;

EmbedderOptions::EmbedderOptions(QWidget *parent)
	: QWidget(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: EmbedderOptions::EmbedderOptions(QWidget *parent = %1)").arg((qulonglong) parent));
#endif

	hide();
	setupUi(this);

	snapshotViewer = NULL;
	showSnapshotViewer = true;

	// restore settings
	comboBoxScaleMode->setCurrentIndex(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Embedder/SnapshotScaleMode", QMC2_EMBEDDER_SNAP_SCALE_NO_FILTER).toInt());
	spinBoxZoom->setValue(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Embedder/ItemZoom", 100).toInt());
	spinBoxZoom->setPrefix(tr("Zoom") + ": ");

	listWidgetSnapshots->setStyleSheet(listWidgetSnapshots->styleSheet() + " QListView::item:selected { background-color: palette(dark); }");

	adjustIconSizes();
}

EmbedderOptions::~EmbedderOptions()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: EmbedderOptions::~EmbedderOptions()");
#endif

	if ( snapshotViewer )
		delete snapshotViewer;
}

void EmbedderOptions::adjustIconSizes()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: EmbedderOptions::adjustIconSizes()");
#endif

	QFont f;
	f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
	QFontMetrics fm(f);
	QSize iconSize(fm.height() - 2, fm.height() - 2);
	toolButtonTakeSnapshot->setIconSize(iconSize);
	toolButtonClearSnapshots->setIconSize(iconSize);
	toolButtonSaveAs->setIconSize(iconSize);
	toolButtonUseAs->setIconSize(iconSize);
}

void EmbedderOptions::on_spinBoxZoom_valueChanged(int zoom)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: EmbedderOptions::on_spinBoxZoom_valueChanged(int zoom = %1)").arg(zoom));
#endif

	quint64 size = QMC2_EMBED_SNAPSHOT_DEFAULT_ITEM_SIZE * (double)zoom / 100.0;
	listWidgetSnapshots->setIconSize(QSize(size, size));
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Embedder/ItemZoom", zoom);
}

void EmbedderOptions::on_toolButtonTakeSnapshot_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: EmbedderOptions::on_toolButtonTakeSnapshot_clicked()");
#endif

	Embedder *embedder = (Embedder *)parent();
	QPixmap pm = QPixmap::grabWindow(embedder->embeddedWinId);
	QRect rect = pm.rect();
	QSize size = embedder->nativeResolution;
	size.scale(rect.size(), Qt::KeepAspectRatio);
	rect.setSize(size);
	rect.moveCenter(pm.rect().center());
	QPixmap clippedPixmap = pm.copy(rect);
	QListWidgetItem *snapshotItem = new QListWidgetItem(QIcon(clippedPixmap), QString(), listWidgetSnapshots);
	switch ( comboBoxScaleMode->currentIndex() ) {
		case QMC2_EMBEDDER_SNAP_SCALE_NONE:
			snapshotMap[snapshotItem] = clippedPixmap;
			break;
		case QMC2_EMBEDDER_SNAP_SCALE_NO_FILTER:
			snapshotMap[snapshotItem] = clippedPixmap.scaled(embedder->nativeResolution, Qt::KeepAspectRatio, Qt::FastTransformation);
			break;
		case QMC2_EMBEDDER_SNAP_SCALE_FILTERED:
			snapshotMap[snapshotItem] = clippedPixmap.scaled(embedder->nativeResolution, Qt::KeepAspectRatio, Qt::SmoothTransformation);
			break;
	}
	listWidgetSnapshots->scrollToItem(snapshotItem, QAbstractItemView::PositionAtBottom);
}

void EmbedderOptions::on_toolButtonClearSnapshots_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: EmbedderOptions::on_toolButtonClearSnapshots_clicked()");
#endif

	snapshotMap.clear();
}

void EmbedderOptions::on_listWidgetSnapshots_itemPressed(QListWidgetItem *item)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: EmbedderOptions::on_listWidgetSnapshots_itemPressed(QListWidgetItem *item = %1)").arg((qulonglong)item));
#endif

	if ( !snapshotViewer ) {
		snapshotViewer = new SnapshotViewer(item, this);
		connect(snapshotViewer, SIGNAL(scaleRequested(QListWidgetItem *)), this, SLOT(on_listWidgetSnapshots_itemPressed(QListWidgetItem *)));
	}
	snapshotViewer->myItem = item;
	QPixmap pm = snapshotMap[item];
	qreal factor = (qreal)snapshotViewer->zoom / 100.0;
	QSize zoomSize(factor * pm.size().width(), factor * pm.size().height());
	pm = pm.scaled(zoomSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	snapshotViewer->resize(pm.size());
	QRect rect = listWidgetSnapshots->visualItemRect(item);
	rect.translate(4, 2);
	QPoint pos = listWidgetSnapshots->mapToGlobal(rect.topLeft());
	if ( pos.x() + snapshotViewer->width() > qmc2MainWindow->desktopGeometry.width() ) {
		pos = listWidgetSnapshots->mapToGlobal(rect.topRight());
		pos.setX(pos.x() - snapshotViewer->width());
	}
	snapshotViewer->move(pos);
	QPalette pal = snapshotViewer->palette();
	QPainter p;
	p.begin(&pm);
	p.setPen(QPen(QColor(0, 0, 0, 64), 1));
	rect = pm.rect();
	rect.setWidth(rect.width() - 1);
	rect.setHeight(rect.height() - 1);
	p.drawRect(rect);
	p.end();
	pal.setBrush(QPalette::Window, pm);
	snapshotViewer->setPalette(pal);
	if ( showSnapshotViewer ) {
		snapshotViewer->showNormal();
		snapshotViewer->raise();
	} else
		snapshotViewer->hide();
	showSnapshotViewer = true;
}

void EmbedderOptions::on_listWidgetSnapshots_itemSelectionChanged()
{
	bool enable = !listWidgetSnapshots->selectedItems().isEmpty();
	toolButtonSaveAs->setEnabled(enable);
	if ( enable ) {
		showSnapshotViewer = false;
		on_listWidgetSnapshots_itemPressed(listWidgetSnapshots->selectedItems()[0]);
		QTimer::singleShot(0, snapshotViewer, SLOT(updateUseAsMenu()));
	} else {
		toolButtonUseAs->setEnabled(false);
		toolButtonUseAs->setMenu(NULL);
	}
}

void EmbedderOptions::on_comboBoxScaleMode_currentIndexChanged(int mode)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: EmbedderOptions::on_comboBoxScaleMode_currentIndexChanged(int mode = %1)").arg(mode));
#endif

	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Embedder/SnapshotScaleMode", mode);
}

void EmbedderOptions::on_toolButtonSaveAs_clicked()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: EmbedderOptions::on_toolButtonSaveAs_clicked()");
#endif

	if ( snapshotViewer )
		snapshotViewer->saveAs();
}

SnapshotViewer::SnapshotViewer(QListWidgetItem *item, QWidget *parent)
	: QWidget(parent, Qt::Tool | Qt::CustomizeWindowHint | Qt::FramelessWindowHint)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SnapshotViewer::SnapshotViewer(QListWidgetItem *item = %1, QWidget *parent = %2)").arg((qulonglong)item).arg((qulonglong)parent));
#endif

	myItem = item;
	setWindowTitle(tr("Snapshot viewer"));

	contextMenu = new QMenu(this);
	contextMenu->hide();

	QString s;
	QAction *action;

	s = tr("Copy to clipboard");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/editcopy.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(copyToClipboard()));

	contextMenu->addSeparator();

	s = tr("Zoom in (+10%)");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/zoom-in.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(zoomIn()));

	s = tr("Zoom out (-10%)");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/zoom-out.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(zoomOut()));

	s = tr("Reset zoom (100%)");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/zoom-none.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(resetZoom()));

	contextMenu->addSeparator();

	s = tr("Save as...");
	action = contextMenu->addAction(s);
	action->setToolTip(s); action->setStatusTip(s);
	action->setIcon(QIcon(QString::fromUtf8(":/data/img/filesaveas.png")));
	connect(action, SIGNAL(triggered()), this, SLOT(saveAs()));

	QList<int> separatorIndizes;
	separatorIndizes << 1 << 2;
	imageTypeNames << tr("Preview") << tr("Title") << tr("Software snapshot") << tr("Flyer") << tr("Cabinet") << tr("Controller") << tr("Marquee") << tr("PCB");
	imageTypeIcons << "camera" << "arcademode" << "pacman" << "thumbnail" << "arcadecabinet" << "joystick" << "marquee" << "circuit";
	cachePrefixes << "prv" << "ttl" << "sws" << "fly" << "cab" << "ctl" << "mrq" << "pcb";

	useAsMenu = contextMenu->addMenu(QIcon(QString::fromUtf8(":/data/img/filesaveas_and_apply.png")), tr("Use as"));

	for (int i = 0; i < imageTypeNames.count(); i++) {
		action = useAsMenu->addAction(imageTypeNames[i]);
		action->setIcon(QIcon(QString(":/data/img/%1.png").arg(imageTypeIcons[i])));
		action->setData(cachePrefixes[i]);
		connect(action, SIGNAL(triggered()), this, SLOT(useAsImage()));
		useAsActions[cachePrefixes[i]] = action;
		if ( imageTypeNames[i] == tr("Software snapshot") ) {
			swsMenu = useAsMenu->addMenu(QIcon(QString(":/data/img/%1.png").arg(imageTypeIcons[i])), tr("Software snapshot"));
			swsMenu->menuAction()->setVisible(false);
		}
		if ( separatorIndizes.contains(i) )
			useAsMenu->addSeparator();
	}

	zoom = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Embedder/SnapshotZoom", 100).toInt();
}

void SnapshotViewer::zoomIn()
{
	zoom += 10;
	if ( zoom > 400 )
		zoom = 400;
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Embedder/SnapshotZoom", zoom);
	emit scaleRequested(myItem);
}

void SnapshotViewer::zoomOut()
{
	zoom -= 10;
	if ( zoom < 10 )
		zoom = 10;
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Embedder/SnapshotZoom", zoom);
	emit scaleRequested(myItem);
}

void SnapshotViewer::resetZoom()
{
	zoom = 100;
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Embedder/SnapshotZoom", zoom);
	emit scaleRequested(myItem);
}

void SnapshotViewer::leaveEvent(QEvent *)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SnapshotViewer::leaveEvent(QEvent *)");
#endif

	if ( contextMenu->isHidden() )
		hide();
}

void SnapshotViewer::mousePressEvent(QMouseEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SnapshotViewer::mousePressEvent(QMouseEvent *e = %1)").arg((qulonglong)e));
#endif

	if ( e->button() != Qt::RightButton ) {
		myItem->setSelected(true);
		hide();
	}
}

void SnapshotViewer::keyPressEvent(QKeyEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SnapshotViewer::keyPressEvent(QKeyPressEvent *e)");
#endif

	if ( e->key() == Qt::Key_Escape ) {
		myItem->setSelected(true);
		hide();
	}
}

void SnapshotViewer::contextMenuEvent(QContextMenuEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SnapshotViewer::contextMenuEvent(QContextMenuEvent *e = %1)").arg((qulonglong)e));
#endif

	Embedder *embedder = (Embedder *)(parent()->parent());
	EmbedderOptions *embedderOptions = (EmbedderOptions *)parent();

	QMapIterator<QString, QAction *> it(useAsActions);
	bool activateUseAsMenu = false;
	swsMenu->menuAction()->setVisible(false);
	while ( it.hasNext() ) {
		it.next();
		QAction *action = it.value();
		QString cachePrefix = it.key();
		ImageWidget *iw = NULL;
		SoftwareSnap *sws = NULL;
		QString actionText;
		switch ( cachePrefixes.indexOf(cachePrefix) ) {
			case QMC2_EMBEDDER_SNAP_IMGTYPE_PREVIEW:
				iw = qmc2Preview;
				actionText = tr("Preview");
				break;
			case QMC2_EMBEDDER_SNAP_IMGTYPE_TITLE:
				iw = qmc2Title;
				actionText = tr("Title");
				break;
			case QMC2_EMBEDDER_SNAP_IMGTYPE_SWS:
				sws = qmc2SoftwareSnap;
				actionText = tr("Software snapshot");
				break;
			case QMC2_EMBEDDER_SNAP_IMGTYPE_FLYER:
				iw = qmc2Flyer;
				actionText = tr("Flyer");
				break;
			case QMC2_EMBEDDER_SNAP_IMGTYPE_CABINET:
				iw = qmc2Cabinet;
				actionText = tr("Cabinet");
				break;
			case QMC2_EMBEDDER_SNAP_IMGTYPE_CONTROLLER:
				iw = qmc2Controller;
				actionText = tr("Controller");
				break;
			case QMC2_EMBEDDER_SNAP_IMGTYPE_MARQUEE:
				iw = qmc2Marquee;
				actionText = tr("Marquee");
				break;
			case QMC2_EMBEDDER_SNAP_IMGTYPE_PCB:
				iw = qmc2PCB;
				actionText = tr("PCB");
				break;
		}
		if ( iw ) {
			action->setVisible(!iw->useZip());
			if ( action->isVisible() ) {
				activateUseAsMenu = true;
				action->setText(actionText + " (" + iw->primaryPathFor(embedder->gameName) + ")");
			}
		} else if ( sws ) {
			if ( !qmc2UseSoftwareSnapFile ) {
				QProcess *proc = qmc2ProcessManager->process(embedder->gameID.toInt());
				QStringList softwareLists = qmc2ProcessManager->softwareListsMap[proc];
				QStringList softwareNames = qmc2ProcessManager->softwareNamesMap[proc];
				if ( softwareLists.count() > 1 && softwareNames.count() > 1 ) {
					action->setVisible(false);
					swsMenu->clear();
					for (int j = 0; j < softwareLists.count(); j++) {
						QAction *a = swsMenu->addAction(sws->primaryPathFor(softwareLists[j], softwareNames[j]));
						a->setData(QString("sws\t%1").arg(j));
						connect(a, SIGNAL(triggered()), this, SLOT(useAsImage()));
					}
					swsMenu->menuAction()->setVisible(true);
					activateUseAsMenu = true;
				} else if ( softwareLists.count() > 0 && softwareNames.count() > 0 ) {
					action->setVisible(true);
					swsMenu->menuAction()->setVisible(false);
					action->setText(actionText + " (" + sws->primaryPathFor(softwareLists[0], softwareNames[0]) + ")");
					action->setData(QString("sws\t0"));
					activateUseAsMenu = true;
				} else {
					action->setVisible(false);
					swsMenu->menuAction()->setVisible(false);
				}
			} else
				action->setVisible(false);
		} else
			action->setVisible(false);
	}
	useAsMenu->menuAction()->setVisible(activateUseAsMenu);
	embedderOptions->toolButtonUseAs->setEnabled(activateUseAsMenu);
	embedderOptions->toolButtonUseAs->setMenu(useAsMenu);

	if ( e ) {
		contextMenu->move(qmc2MainWindow->adjustedWidgetPosition(mapToGlobal(e->pos()), contextMenu));
		contextMenu->show();
	}
}

void SnapshotViewer::useAsImage()
{
	QAction *action = (QAction *)sender();
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SnapshotViewer::useAsImage()");
#endif

	Embedder *embedder = (Embedder *)(parent()->parent());
	EmbedderOptions *embedderOptions = (EmbedderOptions *)parent();

	QStringList dataList = action->data().toString().split("\t", QString::SkipEmptyParts);

	switch ( cachePrefixes.indexOf(dataList[0]) ) {
		case QMC2_EMBEDDER_SNAP_IMGTYPE_PREVIEW:
			if ( qmc2Preview )
				qmc2Preview->replaceImage(embedder->gameName, embedderOptions->snapshotMap[myItem]);
			break;
		case QMC2_EMBEDDER_SNAP_IMGTYPE_TITLE:
			if ( qmc2Title )
				qmc2Title->replaceImage(embedder->gameName, embedderOptions->snapshotMap[myItem]);
			break;
		case QMC2_EMBEDDER_SNAP_IMGTYPE_SWS:
			if ( qmc2SoftwareSnap ) {
				QProcess *proc = qmc2ProcessManager->process(embedder->gameID.toInt());
				QStringList softwareLists = qmc2ProcessManager->softwareListsMap[proc];
				QStringList softwareNames = qmc2ProcessManager->softwareNamesMap[proc];
				if ( softwareLists.count() > 0 && softwareNames.count() > 0 ) {
					int dataIndex = dataList[1].toInt();
					qmc2SoftwareSnap->replaceImage(softwareLists[dataIndex], softwareNames[dataIndex], embedderOptions->snapshotMap[myItem]);
				}
			}
			break;
		case QMC2_EMBEDDER_SNAP_IMGTYPE_FLYER:
			if ( qmc2Flyer )
				qmc2Flyer->replaceImage(embedder->gameName, embedderOptions->snapshotMap[myItem]);
			break;
		case QMC2_EMBEDDER_SNAP_IMGTYPE_CABINET:
			if ( qmc2Cabinet )
				qmc2Cabinet->replaceImage(embedder->gameName, embedderOptions->snapshotMap[myItem]);
			break;
		case QMC2_EMBEDDER_SNAP_IMGTYPE_CONTROLLER:
			if ( qmc2Controller )
				qmc2Controller->replaceImage(embedder->gameName, embedderOptions->snapshotMap[myItem]);
			break;
		case QMC2_EMBEDDER_SNAP_IMGTYPE_MARQUEE:
			if ( qmc2Marquee )
				qmc2Marquee->replaceImage(embedder->gameName, embedderOptions->snapshotMap[myItem]);
			break;
		case QMC2_EMBEDDER_SNAP_IMGTYPE_PCB:
			if ( qmc2PCB )
				qmc2PCB->replaceImage(embedder->gameName, embedderOptions->snapshotMap[myItem]);
			break;
	}
}

void SnapshotViewer::copyToClipboard()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SnapshotViewer::copyToClipboard()");
#endif

	EmbedderOptions *embedderOptions = (EmbedderOptions *)parent();
	qApp->clipboard()->setPixmap(embedderOptions->snapshotMap[myItem]);
}

void SnapshotViewer::saveAs()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SnapshotViewer::saveAs()");
#endif

	if ( fileName.isEmpty() ) {
		Embedder *embedder = (Embedder *)(parent()->parent());
		fileName = embedder->gameName + ".png";
		if ( qmc2Config->contains(QMC2_FRONTEND_PREFIX + "SnapshotViewer/LastStoragePath") )
			fileName.prepend(qmc2Config->value(QMC2_FRONTEND_PREFIX + "SnapshotViewer/LastStoragePath").toString());
	}

	hide();
	fileName = QFileDialog::getSaveFileName(this, tr("Choose PNG file to store image"), fileName, tr("PNG images (*.png)"), 0, qmc2Options->useNativeFileDialogs() ? (QFileDialog::Options)0 : QFileDialog::DontUseNativeDialog);

	if ( !fileName.isEmpty() ) {
		EmbedderOptions *embedderOptions = (EmbedderOptions *)parent();
		if ( !embedderOptions->snapshotMap[myItem].save(fileName, "PNG") )
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: couldn't save snapshot image to '%1'").arg(fileName));
		QFileInfo fiFilePath(fileName);
		QString storagePath = fiFilePath.absolutePath();
		if ( !storagePath.endsWith("/") ) storagePath.append("/");
		qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "SnapshotViewer/LastStoragePath", storagePath);
	}
}

void SnapshotViewer::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	p.eraseRect(rect());
	p.end();
}

#endif
