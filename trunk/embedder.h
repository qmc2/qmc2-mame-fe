#ifndef _EMBEDDER_H_
#define _EMBEDDER_H_

#include <QtGui>
#if defined(Q_WS_X11)
#include <QX11EmbedContainer>
#include "embedderopt.h"

class TweakedEmbedContainer : public QX11EmbedContainer
{
  Q_OBJECT

  public:
    TweakedEmbedContainer(QWidget *parent) : QX11EmbedContainer(parent) { ; }

  protected:
    void resizeEvent(QResizeEvent *e)
    {
      emit resized();
      QX11EmbedContainer::resizeEvent(e);
    }

  signals:
    void resized();
};

class Embedder : public QWidget
{
  Q_OBJECT

  public:
    bool embedded;
    bool optionsShown;
    WId winId;
    TweakedEmbedContainer *embedContainer;
    EmbedderOptions *embedderOptions;
    QGridLayout *gridLayout;
    QString gameName;
    QString gameID;
    QSize nativeResolution;
    QTimer maximizationTimer;

    Embedder(QString name, QString id, WId wid, QWidget *parent = 0);

  public slots:
    void embed();
    void embed(WId wid) { winId = wid; embed(); }
    void release();
    void clientEmbedded();
    void clientClosed();
    void clientError(QX11EmbedContainer::Error);
    void toggleOptions();
    void adjustIconSizes();
    void forceFocus();
    void maximize();
    void maximizeDelayed();

  protected:
    void closeEvent(QCloseEvent *);
    void resizeEvent(QResizeEvent *);

  signals:
    void closing();
};

#endif

#endif
