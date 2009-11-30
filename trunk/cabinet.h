#ifndef _CABINET_H_
#define _CABINET_H_

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
class Cabinet : public QGLWidget
#else
class Cabinet : public QWidget
#endif
{
  Q_OBJECT

  public:
    unzFile cabinetFile;
#if QT_VERSION < 0x040600
    QPixmap *currentCabinetPixmap;
#else
    QPixmap currentCabinetPixmap;
#endif

    Cabinet(QWidget *parent = 0);
    ~Cabinet();

  public slots:
    void drawCenteredImage(QPixmap *, QPainter *);
    void drawScaledImage(QPixmap *, QPainter *);
    bool loadCabinet(QString, QString, bool checkOnly = FALSE, QString *fileName = NULL);

  protected:
    void paintEvent(QPaintEvent *);
};

#endif
