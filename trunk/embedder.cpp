#include <QtGui>
#if defined(Q_WS_X11)
#include "embedder.h"
#include "qmc2main.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;

Embedder::Embedder(quint64 wid, QWidget *parent)
    : QWidget(parent)
{
  QGridLayout *gridLayout = new QGridLayout(this);
  setLayout(gridLayout);

  winId = wid;
  embedded = false;

  embedContainer = new QX11EmbedContainer(this);
  gridLayout->addWidget(embedContainer, 0, 0);

  connect(embedContainer, SIGNAL(clientIsEmbedded()), SLOT(clientEmbedded()));
  connect(embedContainer, SIGNAL(clientClosed()), SLOT(clientClosed()));
  connect(embedContainer, SIGNAL(error(QX11EmbedContainer::Error)), SLOT(clientError(QX11EmbedContainer::Error)));

  QTimer::singleShot(0, this, SLOT(embed()));
}

void Embedder::embed()
{
  embedContainer->embedClient(winId);
}

void Embedder::release()
{
  embedContainer->clearFocus();
  embedContainer->discardClient();
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("emulator released, window ID = 0x%1").arg(QString::number(winId, 16)));
  embedded = false;
}

void Embedder::clientEmbedded()
{
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("emulator embedded, window ID = 0x%1").arg(QString::number(winId, 16)));
  // effectively transfer focus to the emulator
  embedContainer->setFocus();
  embedded = true;
}

void Embedder::clientClosed()
{
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("emulator closed, window ID = 0x%1").arg(QString::number(winId, 16)));
  embedded = false;

  emit closing();
}

void Embedder::clientError(QX11EmbedContainer::Error error)
{
  switch ( error ) {
    case QX11EmbedContainer::Unknown:
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: embedder: unknown error, window ID = 0x%1").arg(QString::number(winId, 16)));
      break;

    case QX11EmbedContainer::InvalidWindowID:
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: embedder: invalid window ID = 0x%1").arg(QString::number(winId, 16)));
      break;

    default:
      break;
  }
}

void Embedder::closeEvent(QCloseEvent *e)
{
  release();
  QWidget::closeEvent(e);
}

#endif
