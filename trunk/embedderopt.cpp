#include "embedderopt.h"
#include "embedder.h"

#if defined(Q_WS_X11) || defined(Q_WS_WIN)

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

  hide();
  setupUi(this);

  snapshotViewer = NULL;

#if !defined(QMC2_WIP_ENABLED)
  tabWidgetEmbedderOptions->removeTab(tabWidgetEmbedderOptions->indexOf(tabMovies));
#endif

  // restore settings
  checkBoxNativeSnapshotResolution->setChecked(qmc2Config->value(QMC2_FRONTEND_PREFIX + "Embedder/NativeSnapshotResolution", true).toBool());

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
  QTabBar *tabBar = tabWidgetEmbedderOptions->findChild<QTabBar *>();
  if ( tabBar ) tabBar->setIconSize(iconSize);
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
  if ( checkBoxNativeSnapshotResolution->isChecked() )
    snapshotMap[snapshotItem] = clippedPixmap.scaled(embedder->nativeResolution, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  else
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

  Embedder *embedder = (Embedder *)parent();
  if ( !snapshotViewer ) {
    snapshotViewer = new SnapshotViewer(item, this);
    qApp->processEvents();
  }
  snapshotViewer->myItem = item;
  QPixmap pm = snapshotMap[item];
  QSize halfSize = pm.size();
  if ( halfSize.width() > embedder->nativeResolution.width() ) {
    halfSize.scale(halfSize.width() / 2, halfSize.height() / 2, Qt::KeepAspectRatio);
    pm = pm.scaled(halfSize, Qt::KeepAspectRatio);
  }
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
  snapshotViewer->showNormal();
  snapshotViewer->raise();
}

void EmbedderOptions::on_checkBoxNativeSnapshotResolution_toggled(bool enabled)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: EmbedderOptions::on_checkBoxNativeSnapshotResolution_toggled(bool enabled = %1)").arg(enabled));
#endif

  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Embedder/NativeSnapshotResolution", enabled);
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

#if defined(QMC2_WIP_ENABLED)
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

  contextMenu->move(qmc2MainWindow->adjustedWidgetPosition(mapToGlobal(e->pos()), contextMenu));
  contextMenu->show();
}

void SnapshotViewer::useAsPreview()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SnapshotViewer::useAsPreview()");
#endif

  Embedder *embedder = (Embedder *)(parent()->parent());
  EmbedderOptions *embedderOptions = (EmbedderOptions *)parent();
  QPixmapCache::remove("prv_" + embedder->gameName);
  QPixmapCache::insert("prv_" + embedder->gameName, embedderOptions->snapshotMap[myItem]);
  qmc2Preview->update();

  // FIXME: we also need to save the image to the preview path or ZIP archive
}

void SnapshotViewer::useAsTitle()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: SnapshotViewer::useAsTitle()");
#endif

  Embedder *embedder = (Embedder *)(parent()->parent());
  EmbedderOptions *embedderOptions = (EmbedderOptions *)parent();
  QPixmapCache::remove("ttl_" + embedder->gameName);
  QPixmapCache::insert("ttl_" + embedder->gameName, embedderOptions->snapshotMap[myItem]);
  qmc2Title->update();

  // FIXME: we also need to save the image to the title path or ZIP archive
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
  fileName = QFileDialog::getSaveFileName(this, tr("Choose PNG file to store image"), fileName, tr("PNG images (*.png)"));

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

void SnapshotViewer::paintEvent(QPaintEvent *e)
{
  QPainter p(this);
  p.eraseRect(rect());
  p.end();
}

#endif
