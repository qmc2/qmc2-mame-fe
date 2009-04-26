#ifndef _FLYER_H_
#define _FLYER_H_

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
class Flyer : public QGLWidget
#else
class Flyer : public QWidget
#endif
{
  Q_OBJECT

  public:
    unzFile flyerFile;

    Flyer(QWidget *parent = 0);
    ~Flyer();

  public slots:
    void drawCenteredImage(QPixmap *, QPainter *);
    void drawScaledImage(QPixmap *, QPainter *);
    bool loadFlyer(QString, QString, bool checkOnly = FALSE, QString *fileName = NULL);

  protected:
    void paintEvent(QPaintEvent *);
};

#endif
