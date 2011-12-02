#ifndef _MARQUEE_H_
#define _MARQUEE_H_

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
class Marquee : public QGLWidget
#else
class Marquee : public QWidget
#endif
{
  Q_OBJECT

  public:
    unzFile marqueeFile;
    QPixmap currentMarqueePixmap;
    QMenu *contextMenu;
    QString myCacheKey;

    Marquee(QWidget *parent = 0);
    ~Marquee();

  public slots:
    void drawCenteredImage(QPixmap *, QPainter *);
    void drawScaledImage(QPixmap *, QPainter *);
    bool loadMarquee(QString, QString, bool checkOnly = FALSE, QString *fileName = NULL);
    void copyToClipboard();
    void refresh();

  protected:
    void paintEvent(QPaintEvent *);
    void contextMenuEvent(QContextMenuEvent *);
};

#endif
