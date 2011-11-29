#include <QPixmapCache>
#include <QPixmap>
#include <QImage>
#include <QMatrix>
#include <QByteArray>
#include <QClipboard>

#include "pcb.h"
#include "options.h"
#include "gamelist.h"
#include "qmc2main.h"
#include "macros.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern Options *qmc2Options;
extern Gamelist *qmc2Gamelist;
extern bool qmc2UsePCBFile;
extern bool qmc2GuiReady;
extern bool qmc2ReloadActive;
extern bool qmc2ScaledPCB;
extern bool qmc2SmoothScaling;
extern bool qmc2RetryLoadingImages;
extern bool qmc2ParentImageFallback;
extern bool qmc2ShowGameName;
extern bool qmc2ShowGameNameOnlyWhenRequired;
extern QTreeWidgetItem *qmc2CurrentItem;
extern QSettings *qmc2Config;
extern QMap<QString, QString> qmc2ParentMap;
extern QMap<QString, QString> qmc2GamelistDescriptionMap;

PCB::PCB(QWidget *parent)
#if QMC2_OPENGL == 1
  : QGLWidget(parent)
#else
  : QWidget(parent)
#endif
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: PCB::PCB(QWidget *parent = 0x" + QString::number((ulong)parent, 16) + ")");
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

#if defined(QMC2_EMUTYPE_MAME)
  setToolTip(tr("Game PCB image"));
  setStatusTip(tr("Game PCB image"));
#elif defined(QMC2_EMUTYPE_MESS)
  setToolTip(tr("Machine PCB image"));
  setStatusTip(tr("Machine PCB image"));
#endif

  pcbFile = NULL;
  if ( qmc2UsePCBFile ) {
#if defined(QMC2_EMUTYPE_MAME)
    pcbFile = unzOpen((const char *)qmc2Config->value("MAME/FilesAndDirectories/PCBFile").toString().toAscii());
    if ( pcbFile == NULL )
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open PCB file, please check access permissions for %1").arg(qmc2Config->value("MAME/FilesAndDirectories/PCBFile").toString()));
#elif defined(QMC2_EMUTYPE_MESS)
    pcbFile = unzOpen((const char *)qmc2Config->value("MESS/FilesAndDirectories/PCBFile").toString().toAscii());
    if ( pcbFile == NULL )
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open PCB file, please check access permissions for %1").arg(qmc2Config->value("MESS/FilesAndDirectories/PCBFile").toString()));
#endif
  }
}

PCB::~PCB()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: PCB::~PCB()");
#endif

  if ( qmc2UsePCBFile )
    unzClose(pcbFile);
}

void PCB::paintEvent(QPaintEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: PCB::paintEvent(QPaintEvent *e = 0x" + QString::number((ulong)e, 16) + ")");
#endif

  QPainter p(this);

  if ( !qmc2CurrentItem ) {
    drawCenteredImage(0, &p); // clear PCB widget
    return;
  }

  if ( qmc2CurrentItem->text(QMC2_GAMELIST_COLUMN_GAME) == tr("Waiting for data...") ) {
    drawCenteredImage(0, &p); // clear PCB widget
    return;
  }

  QTreeWidgetItem *topLevelItem = qmc2CurrentItem;
  while ( topLevelItem->parent() )
    topLevelItem = topLevelItem->parent();

  QString gameName = topLevelItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON);

  if ( !QPixmapCache::find("pcb_" + gameName, &currentPCBPixmap) ) {
    qmc2CurrentItem = topLevelItem;
    loadPCB(gameName, gameName);
  }
  if ( qmc2ScaledPCB )
    drawScaledImage(&currentPCBPixmap, &p);
  else
    drawCenteredImage(&currentPCBPixmap, &p);
}

bool PCB::loadPCB(QString gameName, QString onBehalfOf, bool checkOnly, QString *fileName)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: PCB::loadPCB(QString gameName = %1, QString onBehalfOf = %2, bool checkOnly = %3, QString *fileName = %4)").arg(gameName).arg(onBehalfOf).arg(checkOnly).arg((qulonglong)fileName));
#endif

  QPixmap pm;
  char imageBuffer[QMC2_ZIP_BUFFER_SIZE];

  if ( fileName )
    *fileName = "";

  bool fileOk = true;

  if ( qmc2UsePCBFile ) {
    // use PCB file
    QByteArray imageData;
    int len, i;
    QString gameFile = gameName + ".png";

    if ( fileName )
      *fileName = gameFile;

    if ( unzLocateFile(pcbFile, (const char *)gameFile.toAscii(), 0) == UNZ_OK ) {
      if ( unzOpenCurrentFile(pcbFile) == UNZ_OK ) {
        while ( (len = unzReadCurrentFile(pcbFile, &imageBuffer, QMC2_ZIP_BUFFER_SIZE)) > 0 ) {
          for (i = 0; i < len; i++)
            imageData += imageBuffer[i];
        }
        unzCloseCurrentFile(pcbFile);
      } else
        fileOk = false;
    } else
      fileOk = false;

    if ( fileOk )
      fileOk = pm.loadFromData(imageData, "PNG");

    if ( !checkOnly ) {
      if ( fileOk ) {
        QPixmapCache::insert("pcb_" + onBehalfOf, pm); 
        currentPCBPixmap = pm;
      } else {
        QString parentName = qmc2ParentMap[gameName];
        if ( qmc2ParentImageFallback && !parentName.isEmpty() ) {
          loadPCB(parentName, onBehalfOf);
        } else {
          if ( !qmc2RetryLoadingImages )
            QPixmapCache::insert("pcb_" + onBehalfOf, qmc2MainWindow->qmc2GhostImagePixmap);
          currentPCBPixmap = qmc2MainWindow->qmc2GhostImagePixmap;
        }
      }
    }
  } else {
    // use PCB directory
#if defined(QMC2_EMUTYPE_MAME)
    QString imagePath = qmc2Config->value("MAME/FilesAndDirectories/PCBDirectory").toString() + gameName + ".png";
#elif defined(QMC2_EMUTYPE_MESS)
    QString imagePath = qmc2Config->value("MESS/FilesAndDirectories/PCBDirectory").toString() + gameName + ".png";
#endif

    if ( fileName )
      *fileName = gameName + ".png";

    if ( checkOnly ) {
      fileOk = pm.load(imagePath, "PNG");
    } else {
      if ( pm.load(imagePath, "PNG") ) {
        QPixmapCache::insert("pcb_" + onBehalfOf, pm); 
        currentPCBPixmap = pm;
        fileOk = true;
      } else {
        QString parentName = qmc2ParentMap[gameName];
        if ( qmc2ParentImageFallback && !parentName.isEmpty() ) {
          loadPCB(parentName, onBehalfOf);
        } else {
          if ( !qmc2RetryLoadingImages )
            QPixmapCache::insert("pcb_" + onBehalfOf, qmc2MainWindow->qmc2GhostImagePixmap);
          currentPCBPixmap = qmc2MainWindow->qmc2GhostImagePixmap;
          fileOk = false;
        }
      }
    }
  }

  return fileOk;
}

void PCB::drawCenteredImage(QPixmap *pm, QPainter *p)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: PCB::drawCenteredImage(QPixmap *pm = 0x" + QString::number((ulong)pm, 16) + ", QPainter *p = 0x" + QString::number((ulong)p, 16) + ")");
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
    } else {
      drawGameName = true;
    }
  } else
    drawGameName = false;

  if ( drawGameName ) {
    // draw game/machine title
    QString title = QString("%1").arg(qmc2GamelistDescriptionMap[qmc2CurrentItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON)]);
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
    p->setPen(QPen(QColor(255, 255, 255, 0)));
    p->fillRect(r, QBrush(QColor(0, 0, 0, 128), Qt::SolidPattern));
    p->setPen(QPen(QColor(255, 255, 255, 255)));
    p->drawText(r, Qt::AlignCenter | Qt::TextWordWrap, QString("%1").arg(qmc2GamelistDescriptionMap[qmc2CurrentItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON)]));
  }

  p->end();
}

void PCB::drawScaledImage(QPixmap *pm, QPainter *p)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: PCB::drawScaledImage(QPixmap *pm = 0x" + QString::number((ulong)pm, 16) + ", QPainter *p = 0x" + QString::number((ulong)p, 16) + ")");
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

void PCB::copyToClipboard()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: PCB::copyToClipboard()");
#endif

  qApp->clipboard()->setPixmap(currentPCBPixmap);
}

void PCB::contextMenuEvent(QContextMenuEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: PCB::contextMenuEvent(QContextMenuEvent *e = %1)").arg((qulonglong)e));
#endif

  contextMenu->move(qmc2MainWindow->adjustedWidgetPosition(mapToGlobal(e->pos()), contextMenu));
  contextMenu->show();
}
