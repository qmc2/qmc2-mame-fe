#ifndef _MARQUEE_H_
#define _MARQUEE_H_

#include <QMap>
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
#if QT_VERSION < 0x040600
    QPixmap *currentMarqueePixmap;
#else
    QPixmap currentMarqueePixmap;
#endif

    Marquee(QWidget *parent = 0);
    ~Marquee();

  public slots:
    void drawCenteredImage(QPixmap *, QPainter *);
    void drawScaledImage(QPixmap *, QPainter *);
    bool loadMarquee(QString, QString, bool checkOnly = FALSE, QString *fileName = NULL);

  protected:
    void paintEvent(QPaintEvent *);
};

#endif
