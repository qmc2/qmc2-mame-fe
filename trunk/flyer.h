#ifndef _FLYER_H_
#define _FLYER_H_

#include <QMap>
#include <QMenu>
#include <QWidget>
#include <QPixmap>
#include <QPainter>
#include <QTreeWidgetItem>
#if QMC2_OPENGL == 1
#include <QGLWidget>
#endif

#include "unzip.h"

#if QMC2_OPENGL == 1
class Flyer : public QGLWidget
#else
class Flyer : public QWidget
#endif
{
  Q_OBJECT

  public:
    unzFile flyerFile;
    QPixmap currentFlyerPixmap;
    QMenu *contextMenu;
    QString myCacheKey;

    Flyer(QWidget *parent = 0);
    ~Flyer();

  public slots:
    void drawCenteredImage(QPixmap *, QPainter *);
    void drawScaledImage(QPixmap *, QPainter *);
    bool loadFlyer(QString, QString, bool checkOnly = FALSE, QString *fileName = NULL);
    void copyToClipboard();
    void refresh();

  protected:
    void paintEvent(QPaintEvent *);
    void contextMenuEvent(QContextMenuEvent *);
};

#endif
