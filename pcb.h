#ifndef _PCB_H_
#define _PCB_H_

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
class PCB : public QGLWidget
#else
class PCB : public QWidget
#endif
{
  Q_OBJECT

  public:
    unzFile pcbFile;

    PCB(QWidget *parent = 0);
    ~PCB();

  public slots:
    void drawCenteredImage(QPixmap *, QPainter *);
    void drawScaledImage(QPixmap *, QPainter *);
    bool loadPCB(QString, QString, bool checkOnly = FALSE, QString *fileName = NULL);

  protected:
    void paintEvent(QPaintEvent *);
};

#endif
