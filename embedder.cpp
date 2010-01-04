#include <QtGui>
#if defined(Q_WS_X11)
#include "embedder.h"
#include "qmc2main.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;
extern QSettings *qmc2Config;

Embedder::Embedder(QString name, WId wid, QWidget *parent)
    : QWidget(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::Embedder(QString name = %1, WId wid = %2, QWidget *parent = %3)").arg(name).arg((qulonglong)wid).arg((qulonglong)parent));
#endif

  gridLayout = new QGridLayout(this);

  gameName = name;
  winId = wid;
  embedded = false;

  embedContainer = new QX11EmbedContainer(this);
  embedContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  gridLayout->addWidget(embedContainer, 1, 0);
  gridLayout->setRowStretch(0, 0);
  gridLayout->setRowStretch(1, 1);
  gridLayout->setColumnStretch(0, 1);
  setLayout(gridLayout);

  connect(embedContainer, SIGNAL(clientIsEmbedded()), SLOT(clientEmbedded()));
  connect(embedContainer, SIGNAL(clientClosed()), SLOT(clientClosed()));
  connect(embedContainer, SIGNAL(error(QX11EmbedContainer::Error)), SLOT(clientError(QX11EmbedContainer::Error)));

  embedderOptions = NULL;
  optionsShown = FALSE;

  QTimer::singleShot(0, this, SLOT(embed()));
}

void Embedder::embed()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::embed()"));
#endif

  nativeResolution = QPixmap::grabWindow(winId).size();
  embedContainer->embedClient(winId);
}

void Embedder::release()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::release()"));
#endif

  embedded = false;
  embedContainer->clearFocus();
  embedContainer->discardClient();
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("emulator released, window ID = 0x%1").arg(QString::number(winId, 16)));
}

void Embedder::clientEmbedded()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::clientEmbedded()"));
#endif

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("emulator embedded, window ID = 0x%1").arg(QString::number(winId, 16)));
  // effectively transfer focus to the emulator
  embedContainer->setFocus();
  setFocusProxy(embedContainer);
  embedded = true;

  // this works around a Qt bug when the tool bar is vertical and obscured by the emulator window before embedding
  QTimer::singleShot(0, qmc2MainWindow->toolbar, SLOT(update()));
}

void Embedder::clientClosed()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::clientClosed()"));
#endif

  if ( embedded ) 
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("emulator closed, window ID = 0x%1").arg(QString::number(winId, 16)));
  embedded = false;
  emit closing();
}

void Embedder::clientError(QX11EmbedContainer::Error error)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::clientError(QX11EmbedContainer::Error error = %1)").arg((int)error));
#endif

  switch ( error ) {
    case QX11EmbedContainer::Unknown:
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: embedder: unknown error, window ID = 0x%1").arg(QString::number(winId, 16)));
      break;

    case QX11EmbedContainer::InvalidWindowID:
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: embedder: invalid window ID = 0x%1").arg(QString::number(winId, 16)));
      emit closing();
      break;

    default:
      break;
  }
}

void Embedder::closeEvent(QCloseEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::closeEvent(QCloseEvent *e = %1)").arg((qulonglong)e));
#endif

  release();

  if ( embedderOptions )
    delete embedderOptions;

  QWidget::closeEvent(e);
}

void Embedder::toggleOptions()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::toggleOptions()"));
#endif

  optionsShown = !optionsShown;
  if ( optionsShown ) {
    if ( !embedderOptions ) {
      embedderOptions = new EmbedderOptions(this);
      embedderOptions->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
      gridLayout->addWidget(embedderOptions, 0, 0);
    }
    gridLayout->setRowStretch(0, 1);
    gridLayout->setRowStretch(1, 4);
    embedderOptions->show();
  } else {
    gridLayout->setRowStretch(0, 0);
    gridLayout->setRowStretch(1, 1);
    if ( embedderOptions )
      embedderOptions->hide();
  }
}

void Embedder::adjustIconSizes()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::adjustIconSizes()"));
#endif

  // serious hack to access the tab bar without sub-classing from QTabWidget ;)
  QTabBar *tabBar = qmc2MainWindow->tabWidgetEmbeddedEmulators->findChild<QTabBar *>();
  int index = qmc2MainWindow->tabWidgetEmbeddedEmulators->indexOf(this);

  QFont f;
  f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
  QFontMetrics fm(f);
  QSize buttonSize(fm.height() + 2, fm.height() + 2);
  tabBar->tabButton(index, QTabBar::LeftSide)->setFixedSize(buttonSize);
  tabBar->tabButton(index, QTabBar::RightSide)->setFixedSize(buttonSize);
}

#endif
