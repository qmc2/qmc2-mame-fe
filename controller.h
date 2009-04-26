#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

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
class Controller : public QGLWidget
#else
class Controller : public QWidget
#endif
{
  Q_OBJECT

  public:
    unzFile controllerFile;

    Controller(QWidget *parent = 0);
    ~Controller();

  public slots:
    void drawCenteredImage(QPixmap *, QPainter *);
    void drawScaledImage(QPixmap *, QPainter *);
    bool loadController(QString, QString, bool checkOnly = FALSE, QString *fileName = NULL);

  protected:
    void paintEvent(QPaintEvent *);
};

#endif
