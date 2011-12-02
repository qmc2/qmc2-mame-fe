#include <QPixmapCache>
#include <QPixmap>
#include <QImage>
#include <QMatrix>
#include <QDir>
#include <QByteArray>
#include <QMap>
#include <QClipboard>

#include "preview.h"
#include "options.h"
#include "gamelist.h"
#include "qmc2main.h"
#include "macros.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern Options *qmc2Options;
extern Gamelist *qmc2Gamelist;
extern bool qmc2UsePreviewFile;
extern bool qmc2GuiReady;
extern bool qmc2ReloadActive;
extern bool qmc2ScaledPreview;
extern bool qmc2SmoothScaling;
extern bool qmc2RetryLoadingImages;
extern bool qmc2ParentImageFallback;
extern bool qmc2ShowGameName;
extern bool qmc2ShowGameNameOnlyWhenRequired;
extern QTreeWidgetItem *qmc2CurrentItem;
extern QSettings *qmc2Config;
extern QMap<QString, QString> qmc2ParentMap;
extern QMap<QString, QString> qmc2GamelistDescriptionMap;

Preview::Preview(QWidget *parent)
#if QMC2_OPENGL == 1
  : QGLWidget(parent)
#else
  : QWidget(parent)
#endif
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Preview::Preview(QWidget *parent = 0x" + QString::number((ulong)parent, 16) + ")");
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

#if defined(QMC2_EMUTYPE_MAME)
  setToolTip(tr("Game preview image"));
  setStatusTip(tr("Game preview image"));
#elif defined(QMC2_EMUTYPE_MESS)
  setToolTip(tr("Machine preview image"));
  setStatusTip(tr("Machine preview image"));
#endif

  previewFile = NULL;
  if ( qmc2UsePreviewFile ) {
#if defined(QMC2_EMUTYPE_MAME)
    previewFile = unzOpen((const char *)qmc2Config->value("MAME/FilesAndDirectories/PreviewFile").toString().toAscii());
    if ( previewFile == NULL )
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open preview file, please check access permissions for %1").arg(qmc2Config->value("MAME/FilesAndDirectories/PreviewFile").toString()));
#elif defined(QMC2_EMUTYPE_MESS)
    previewFile = unzOpen((const char *)qmc2Config->value("MESS/FilesAndDirectories/PreviewFile").toString().toAscii());
    if ( previewFile == NULL )
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open preview file, please check access permissions for %1").arg(qmc2Config->value("MESS/FilesAndDirectories/PreviewFile").toString()));
#endif
  }
}

Preview::~Preview()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Preview::~Preview()");
#endif

  if ( qmc2UsePreviewFile )
    unzClose(previewFile);
}

void Preview::paintEvent(QPaintEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Preview::paintEvent(QPaintEvent *e = 0x" + QString::number((ulong)e, 16) + ")");
#endif

  QPainter p(this);

  if ( !qmc2CurrentItem ) {
    drawCenteredImage(0, &p); // clear preview widget
    return;
  }

  if ( qmc2CurrentItem->text(QMC2_GAMELIST_COLUMN_GAME) == tr("Waiting for data...") ) {
    drawCenteredImage(0, &p); // clear preview widget
    return;
  }

  QTreeWidgetItem *topLevelItem = qmc2CurrentItem;
  while ( topLevelItem->parent() )
    topLevelItem = topLevelItem->parent();

  QString gameName = topLevelItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON);

  if ( !QPixmapCache::find(gameName, &currentPreviewPixmap) ) {
    qmc2CurrentItem = topLevelItem;
    loadPreview(gameName, gameName);
  }
  myCacheKey = gameName;
  if ( qmc2ScaledPreview )
    drawScaledImage(&currentPreviewPixmap, &p);
  else
    drawCenteredImage(&currentPreviewPixmap, &p);
}

void Preview::refresh()
{
	if ( !myCacheKey.isEmpty() ) {
		QPixmapCache::remove(myCacheKey);
		repaint();
	}
}

bool Preview::loadPreview(QString gameName, QString onBehalfOf, bool checkOnly, QString *fileName)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Preview::loadPreview(QString gameName = %1, QString onBehalfOf = %2, bool checkOnly = %3, QString *fileName = %4)").arg(gameName).arg(onBehalfOf).arg(checkOnly).arg((qulonglong)fileName));
#endif

  QPixmap pm;
  char imageBuffer[QMC2_ZIP_BUFFER_SIZE];

  if ( fileName )
    *fileName = "";

  bool fileOk = true;

  if ( qmc2UsePreviewFile ) {
    // use preview file
    QByteArray imageData;
    int len, i;
    QString gameFile = gameName + ".png";

    if ( fileName )
      *fileName = gameFile;

    if ( unzLocateFile(previewFile, (const char *)gameFile.toAscii(), 0) == UNZ_OK ) {
      if ( unzOpenCurrentFile(previewFile) == UNZ_OK ) {
        while ( (len = unzReadCurrentFile(previewFile, &imageBuffer, QMC2_ZIP_BUFFER_SIZE)) > 0 ) {
          for (i = 0; i < len; i++)
            imageData += imageBuffer[i];
        }
        unzCloseCurrentFile(previewFile);
      } else
        fileOk = false;
    } else
      fileOk = false;

    if ( fileOk )
      fileOk = pm.loadFromData(imageData, "PNG");

    if ( !checkOnly ) {
      if ( fileOk ) {
        QPixmapCache::insert(onBehalfOf, pm); 
        currentPreviewPixmap = pm;
      } else {
        QString parentName = qmc2ParentMap[gameName];
        if ( qmc2ParentImageFallback && !parentName.isEmpty() ) {
          loadPreview(parentName, onBehalfOf);
        } else {
          if ( !qmc2RetryLoadingImages )
            QPixmapCache::insert(onBehalfOf, qmc2MainWindow->qmc2GhostImagePixmap); 
          currentPreviewPixmap = qmc2MainWindow->qmc2GhostImagePixmap;
        }
      }
    }
  } else {
    // use preview directory
#if defined(QMC2_EMUTYPE_MAME)
    QString baseDirectory = qmc2Config->value("MAME/FilesAndDirectories/PreviewDirectory").toString();
#elif defined(QMC2_EMUTYPE_MESS)
    QString baseDirectory = qmc2Config->value("MESS/FilesAndDirectories/PreviewDirectory").toString();
#endif
    QString imageDir = baseDirectory + gameName;
    QString imagePath = imageDir + ".png";

    if ( fileName )
      *fileName = gameName + ".png";

    QFile f(imagePath);
    if ( !f.exists() ) {
      QDir dir(imageDir);
      if ( dir.exists() ) {
        QStringList nameFilter;
        nameFilter << "*.png";
        QStringList dirEntries = dir.entryList(nameFilter, QDir::Files | QDir::NoDotAndDotDot | QDir::Readable | QDir::CaseSensitive, QDir::Name | QDir::Reversed);
        if ( dirEntries.count() > 0 ) {
          imagePath = imageDir + "/" + dirEntries[0];
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
        currentPreviewPixmap = pm;
        fileOk = true;
      } else {
        QString parentName = qmc2ParentMap[gameName];
        if ( qmc2ParentImageFallback && !parentName.isEmpty() ) {
          loadPreview(parentName, onBehalfOf);
        } else {
          if ( !qmc2RetryLoadingImages )
            QPixmapCache::insert(onBehalfOf, qmc2MainWindow->qmc2GhostImagePixmap); 
          currentPreviewPixmap = qmc2MainWindow->qmc2GhostImagePixmap;
          fileOk = false;
        }
      }
    }
  }

  return fileOk;
}

void Preview::drawCenteredImage(QPixmap *pm, QPainter *p)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Preview::drawCenteredImage(QPixmap *pm = 0x" + QString::number((ulong)pm, 16) + ", QPainter *p = 0x" + QString::number((ulong)p, 16) + ")");
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

void Preview::drawScaledImage(QPixmap *pm, QPainter *p)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Preview::drawScaledImage(QPixmap *pm = 0x" + QString::number((ulong)pm, 16) + ", QPainter *p = 0x" + QString::number((ulong)p, 16) + ")");
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

void Preview::copyToClipboard()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Preview::copyToClipboard()");
#endif

  qApp->clipboard()->setPixmap(currentPreviewPixmap);
}

void Preview::contextMenuEvent(QContextMenuEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Preview::contextMenuEvent(QContextMenuEvent *e = %1)").arg((qulonglong)e));
#endif

  contextMenu->move(qmc2MainWindow->adjustedWidgetPosition(mapToGlobal(e->pos()), contextMenu));
  contextMenu->show();
}
