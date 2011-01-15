#ifndef _TITLE_H_
#define _TITLE_H_

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
class Title : public QGLWidget
#else
class Title : public QWidget
#endif
{
  Q_OBJECT

  public:
    unzFile titleFile;
#if QT_VERSION < 0x040600
    QPixmap *currentTitlePixmap;
#else
    QPixmap currentTitlePixmap;
#endif
    QMenu *contextMenu;

    Title(QWidget *parent = 0);
    ~Title();

  public slots:
    void drawCenteredImage(QPixmap *, QPainter *);
    void drawScaledImage(QPixmap *, QPainter *);
    bool loadTitle(QString, QString, bool checkOnly = FALSE, QString *fileName = NULL);
    void copyToClipboard();

  protected:
    void paintEvent(QPaintEvent *);
    void contextMenuEvent(QContextMenuEvent *);
};

#endif
