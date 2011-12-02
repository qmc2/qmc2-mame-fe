#ifndef _CABINET_H_
#define _CABINET_H_

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
class Cabinet : public QGLWidget
#else
class Cabinet : public QWidget
#endif
{
  Q_OBJECT

  public:
    unzFile cabinetFile;
    QPixmap currentCabinetPixmap;
    QMenu *contextMenu;
    QString myCacheKey;

    Cabinet(QWidget *parent = 0);
    ~Cabinet();

  public slots:
    void drawCenteredImage(QPixmap *, QPainter *);
    void drawScaledImage(QPixmap *, QPainter *);
    bool loadCabinet(QString, QString, bool checkOnly = FALSE, QString *fileName = NULL);
    void copyToClipboard();
    void refresh();

  protected:
    void paintEvent(QPaintEvent *);
    void contextMenuEvent(QContextMenuEvent *);
};

#endif
