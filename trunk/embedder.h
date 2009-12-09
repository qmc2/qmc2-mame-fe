#ifndef _EMBEDDER_H_
#define _EMBEDDER_H_

#include <QtGui>
#if defined(Q_WS_X11)
#include <QX11EmbedContainer>
#include "embedderopt.h"

class Embedder : public QWidget
{
  Q_OBJECT

  public:
    bool embedded;
    bool optionsShown;
    quint64 winId;
    QX11EmbedContainer *embedContainer;
    EmbedderOptions *embedderOptions;
    QGridLayout *gridLayout;

    Embedder(quint64 wid, QWidget *parent = 0);

  public slots:
    void embed();
    void embed(quint64 wid) { winId = wid; embed(); }
    void release();
    void clientEmbedded();
    void clientClosed();
    void clientError(QX11EmbedContainer::Error);
    void toggleOptions();

  protected:
    void closeEvent(QCloseEvent *);

  signals:
    void closing();
};

#endif

#endif
