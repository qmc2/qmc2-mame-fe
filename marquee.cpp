#include <QPixmapCache>
#include <QPixmap>
#include <QImage>
#include <QMatrix>
#include <QByteArray>

#include "marquee.h"
#include "options.h"
#include "gamelist.h"
#include "qmc2main.h"
#include "macros.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern Options *qmc2Options;
extern Gamelist *qmc2Gamelist;
extern bool qmc2UseMarqueeFile;
extern bool qmc2GuiReady;
extern bool qmc2ReloadActive;
extern bool qmc2ScaledMarquee;
extern bool qmc2SmoothScaling;
extern bool qmc2RetryLoadingImages;
extern bool qmc2ParentImageFallback;
extern bool qmc2ShowGameName;
extern bool qmc2ShowGameNameOnlyWhenRequired;
extern QTreeWidgetItem *qmc2CurrentItem;
extern QSettings *qmc2Config;
extern QMap<QString, QString> qmc2ParentMap;
extern QMap<QString, QString> qmc2GamelistDescriptionMap;

QPixmap *currentMarqueePixmap;

Marquee::Marquee(QWidget *parent)
#if QMC2_OPENGL == 1
  : QGLWidget(parent)
#else
  : QWidget(parent)
#endif
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Marquee::Marquee(QWidget *parent = 0x" + QString::number((ulong)parent, 16) + ")");
#endif

#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
  setToolTip(tr("Game marquee image"));
  setStatusTip(tr("Game marquee image"));
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
  setToolTip(tr("Machine marquee image"));
  setStatusTip(tr("Machine marquee image"));
#endif

  marqueeFile = NULL;
  if ( qmc2UseMarqueeFile ) {
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
    marqueeFile = unzOpen((const char *)qmc2Config->value("MAME/FilesAndDirectories/MarqueeFile").toString().toAscii());
    if ( marqueeFile == NULL )
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open marquee file, please check access permissions for %1").arg(qmc2Config->value("MAME/FilesAndDirectories/MarqueeFile").toString()));
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
    marqueeFile = unzOpen((const char *)qmc2Config->value("MESS/FilesAndDirectories/MarqueeFile").toString().toAscii());
    if ( marqueeFile == NULL )
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("FATAL: can't open marquee file, please check access permissions for %1").arg(qmc2Config->value("MESS/FilesAndDirectories/MarqueeFile").toString()));
#endif
  }
}

Marquee::~Marquee()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Marquee::~Marquee()");
#endif

  if ( qmc2UseMarqueeFile )
    unzClose(marqueeFile);
}

void Marquee::paintEvent(QPaintEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Marquee::paintEvent(QPaintEvent *e = 0x" + QString::number((ulong)e, 16) + ")");
#endif

  QPainter p(this);

  if ( !qmc2CurrentItem ) {
    drawCenteredImage(0, &p); // clear marquee widget
    return;
  }

  if ( qmc2CurrentItem->text(QMC2_GAMELIST_COLUMN_GAME) == tr("Waiting for data...") ) {
    drawCenteredImage(0, &p); // clear marquee widget
    return;
  }

  QTreeWidgetItem *topLevelItem = qmc2CurrentItem;
  while ( topLevelItem->parent() )
    topLevelItem = topLevelItem->parent();

  QString gameName = topLevelItem->child(0)->text(QMC2_GAMELIST_COLUMN_ICON);
  static QPixmap cachedPixmap;

  if ( QPixmapCache::find("mrq_" + gameName, cachedPixmap) ) {
    currentMarqueePixmap = &cachedPixmap;
  } else {
    qmc2CurrentItem = topLevelItem;
    loadMarquee(gameName, gameName);
  }

  if ( qmc2ScaledMarquee )
    drawScaledImage(currentMarqueePixmap, &p);
  else
    drawCenteredImage(currentMarqueePixmap, &p);
}

bool Marquee::loadMarquee(QString gameName, QString onBehalfOf, bool checkOnly, QString *fileName)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Marquee::loadMarquee(QString gameName = %1, QString onBehalfOf = %2, bool checkOnly = %3, QString *fileName = %4)").arg(gameName).arg(onBehalfOf).arg(checkOnly).arg((qulonglong)fileName));
#endif

  static QPixmap pm;
  static char imageBuffer[QMC2_ZIP_BUFFER_SIZE];

  if ( fileName )
    *fileName = "";

  bool fileOk = TRUE;

  if ( qmc2UseMarqueeFile ) {
    // use marquee file
    QByteArray imageData;
    int len, i;
    QString gameFile = gameName + ".png";

    if ( fileName )
      *fileName = gameFile;

    if ( unzLocateFile(marqueeFile, (const char *)gameFile.toAscii(), 0) == UNZ_OK ) {
      if ( unzOpenCurrentFile(marqueeFile) == UNZ_OK ) {
        while ( (len = unzReadCurrentFile(marqueeFile, &imageBuffer, QMC2_ZIP_BUFFER_SIZE)) > 0 ) {
          for (i = 0; i < len; i++)
            imageData += imageBuffer[i];
        }
        unzCloseCurrentFile(marqueeFile);
      } else
        fileOk = FALSE;
    } else
      fileOk = FALSE;

    if ( fileOk )
      fileOk = pm.loadFromData(imageData);

    if ( !checkOnly ) {
      if ( fileOk ) {
        QPixmapCache::insert("mrq_" + onBehalfOf, pm); 
        currentMarqueePixmap = &pm;
      } else {
        QString parentName = qmc2ParentMap[gameName];
        if ( qmc2ParentImageFallback && !parentName.isEmpty() ) {
          loadMarquee(parentName, onBehalfOf);
        } else {
          if ( !qmc2RetryLoadingImages )
            QPixmapCache::insert("mrq_" + onBehalfOf, qmc2MainWindow->qmc2GhostImagePixmap);
          currentMarqueePixmap = &qmc2MainWindow->qmc2GhostImagePixmap;
        }
      }
    }
  } else {
    // use marquee directory
#if defined(QMC2_SDLMAME) || defined(QMC2_MAME)
    QString imagePath = qmc2Config->value("MAME/FilesAndDirectories/MarqueeDirectory").toString() + gameName + ".png";
#elif defined(QMC2_SDLMESS) || defined(QMC2_MESS)
    QString imagePath = qmc2Config->value("MESS/FilesAndDirectories/MarqueeDirectory").toString() + gameName + ".png";
#endif

    if ( fileName )
      *fileName = gameName + ".png";

    if ( checkOnly ) {
      fileOk = pm.load(imagePath);
    } else {
      if ( pm.load(imagePath) ) {
        QPixmapCache::insert("mrq_" + onBehalfOf, pm); 
        currentMarqueePixmap = &pm;
        fileOk = TRUE;
      } else {
        QString parentName = qmc2ParentMap[gameName];
        if ( qmc2ParentImageFallback && !parentName.isEmpty() ) {
          loadMarquee(parentName, onBehalfOf);
        } else {
          if ( !qmc2RetryLoadingImages )
            QPixmapCache::insert("mrq_" + onBehalfOf, qmc2MainWindow->qmc2GhostImagePixmap);
          currentMarqueePixmap = &qmc2MainWindow->qmc2GhostImagePixmap;
          fileOk = FALSE;
        }
      }
    }
  }

  return fileOk;
}

void Marquee::drawCenteredImage(QPixmap *pm, QPainter *p)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Marquee::drawCenteredImage(QPixmap *pm = 0x" + QString::number((ulong)pm, 16) + ", QPainter *p = 0x" + QString::number((ulong)p, 16) + ")");
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

  bool drawGameName = FALSE;
  if ( qmc2ShowGameName ) {
    if ( qmc2ShowGameNameOnlyWhenRequired ) {
      if ( qmc2MainWindow->hSplitter->sizes()[0] == 0 || qmc2MainWindow->tabWidgetGamelist->currentIndex() != QMC2_GAMELIST_INDEX ) {
        drawGameName = TRUE;
      } else {
        drawGameName = FALSE;
      }
    } else {
      drawGameName = TRUE;
    }
  } else
    drawGameName = FALSE;

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

void Marquee::drawScaledImage(QPixmap *pm, QPainter *p)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: Marquee::drawScaledImage(QPixmap *pm = 0x" + QString::number((ulong)pm, 16) + ", QPainter *p = 0x" + QString::number((ulong)p, 16) + ")");
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
