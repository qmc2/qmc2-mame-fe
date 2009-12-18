#include "embedderopt.h"
#include "embedder.h"

#if defined(Q_WS_X11)

#include "gamelist.h"
#include "qmc2main.h"
#include "preview.h"
#include "title.h"
#include "macros.h"
extern MainWindow *qmc2MainWindow;
extern Gamelist *qmc2Gamelist;
extern QSettings *qmc2Config;
extern Preview *qmc2Preview;
extern Title *qmc2Title;

EmbedderOptions::EmbedderOptions(QWidget *parent)
  : QWidget(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: EmbedderOptions::EmbedderOptions(QWidget *parent = %1)").arg((qulonglong) parent));
#endif

  setupUi(this);

  Embedder *embedder = (Embedder *)parent;
  snapshotViewer = NULL;

#if QMC2_WIP_CODE != 1
  tabWidgetEmbedderOptions->removeTab(tabWidgetEmbedderOptions->indexOf(tabMovies));
  tabWidgetEmbedderOptions->removeTab(tabWidgetEmbedderOptions->indexOf(tabNetplay));
#endif
}

EmbedderOptions::~EmbedderOptions()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: EmbedderOptions::~EmbedderOptions()");
#endif

  if ( snapshotViewer )
    delete snapshotViewer;
}

void EmbedderOptions::on_toolButtonTakeSnapshot_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: EmbedderOptions::on_toolButtonTakeSnapshot_clicked()");
#endif

  Embedder *embedder = (Embedder *)parent();
  QPixmap pm = QPixmap::grabWindow(embedder->winId);
  QRect rect = pm.rect();
  QSize size = embedder->nativeResolution;
  size.scale(rect.size(), Qt::KeepAspectRatio);
  rect.setSize(size);
  rect.moveCenter(pm.rect().center());
  QPixmap clippedPixmap = pm.copy(rect);
  QListWidgetItem *snapshotItem = new QListWidgetItem(QIcon(clippedPixmap), QString(), listWidgetSnapshots);
  snapshotMap[snapshotItem] = clippedPixmap;
  listWidgetSnapshots->scrollToItem(snapshotItem);
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

  if ( !snapshotViewer )
    snapshotViewer = new SnapshotViewer(item, this);
  snapshotViewer->myItem = item;
  QPixmap pm = snapshotMap[item];
  QRect rect = listWidgetSnapshots->visualItemRect(item);
  rect.moveTo(listWidgetSnapshots->mapToGlobal(rect.topLeft()));
  snapshotViewer->resize(pm.size());
  snapshotViewer->move(rect.topLeft());
  QPalette pal = snapshotViewer->palette();
  QPainter p;
  p.begin(&pm);
  p.setPen(QPen(QColor(255, 255, 255, 64), 1));
  rect = pm.rect();
  rect.setWidth(rect.width() - 1);
  rect.setHeight(rect.height() - 1);
  p.drawRect(rect);
  p.end();
  pal.setBrush(QPalette::Window, pm);
  snapshotViewer->setPalette(pal);
  snapshotViewer->showNormal();
  snapshotViewer->raise();
}

SnapshotViewer::SnapshotViewer(QListWidgetItem *item, QWidget *parent)
  : QWidget(parent, Qt::Window | Qt::CustomizeWindowHint | Qt::X11BypassWindowManagerHint)
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

#if QMC2_WIP_CODE == 1
  s = tr("Use as preview");
  action = contextMenu->addAction(s);
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/camera.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(useAsPreview()));

  s = tr("Use as title");
  action = contextMenu->addAction(s);
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/arcademode.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(useAsTitle()));

  contextMenu->addSeparator();
#endif

  s = tr("Save as...");
  action = contextMenu->addAction(s);
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/filesaveas.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(saveAs()));

  s = tr("Copy to clipboard");
  action = contextMenu->addAction(s);
  action->setToolTip(s); action->setStatusTip(s);
  action->setIcon(QIcon(QString::fromUtf8(":/data/img/editcopy.png")));
  connect(action, SIGNAL(triggered()), this, SLOT(copyToClipboard()));
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
    myItem->setSelected(TRUE);
    hide();
  }
}

void SnapshotViewer::contextMenuEvent(QContextMenuEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: SnapshotViewer::contextMenuEvent(QContextMenuEvent *e = %1)").arg((qulonglong)e));
#endif

  contextMenu->move(mapToGlobal(e->pos()));
  contextMenu->show();
}

void SnapshotViewer::useAsPreview()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SnapshotViewer::useAsPreview()");
#endif

  Embedder *embedder = (Embedder *)(parent()->parent());
  QPixmapCache::remove(embedder->gameName);
  QPixmapCache::insert(embedder->gameName, palette().brush(QPalette::Window).texture());
  qmc2Preview->repaint();

  // FIXME: we also need to save the image to the preview path or ZIP archive
}

void SnapshotViewer::useAsTitle()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SnapshotViewer::useAsTitle()");
#endif

  Embedder *embedder = (Embedder *)(parent()->parent());
  QPixmapCache::remove("ttl_" + embedder->gameName);
  QPixmapCache::insert("ttl_" + embedder->gameName, palette().brush(QPalette::Window).texture());
  qmc2Title->repaint();

  // FIXME: we also need to save the image to the title path or ZIP archive
}

void SnapshotViewer::copyToClipboard()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SnapshotViewer::copyToClipboard()");
#endif

  qApp->clipboard()->setPixmap(palette().brush(QPalette::Window).texture());
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
  fileName = QFileDialog::getSaveFileName(this, tr("Choose PNG file to store image"), fileName, tr("PNG images (*.png)"));

  if ( !fileName.isEmpty() ) {
    if ( !palette().brush(QPalette::Window).texture().save(fileName, "PNG") )
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: couldn't save snapshot image to '%1'").arg(fileName));
    QFileInfo fiFilePath(fileName);
    QString storagePath = fiFilePath.absolutePath();
    if ( !storagePath.endsWith("/") ) storagePath.append("/");
    qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "SnapshotViewer/LastStoragePath", storagePath);
  }
}
#endif
