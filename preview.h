#ifndef _PREVIEW_H_
#define _PREVIEW_H_

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
class Preview : public QGLWidget
#else
class Preview : public QWidget
#endif
{
  Q_OBJECT 

  public:
    unzFile previewFile;
#if QT_VERSION < 0x040600
    QPixmap *currentPreviewPixmap;
#else
    QPixmap currentPreviewPixmap;
#endif

    Preview(QWidget *parent);
    ~Preview();

  public slots:
    void drawCenteredImage(QPixmap *, QPainter *);
    void drawScaledImage(QPixmap *, QPainter *);
    bool loadPreview(QString, QString, bool checkOnly = FALSE, QString *fileName = NULL);

  protected:
    void paintEvent(QPaintEvent *);
};

#endif
