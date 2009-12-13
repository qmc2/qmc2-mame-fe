#ifndef _EMBEDDEROPT_H_
#define _EMBEDDEROPT_H_

#include <QtGui>
#include "ui_embedderopt.h"

#if defined(Q_WS_X11)
class SnapshotViewer : public QWidget
{
  Q_OBJECT

  public:
    QListWidgetItem *myItem;

    SnapshotViewer(QListWidgetItem *item, QWidget *parent = 0);

  protected:
    void leaveEvent(QEvent *);
    void mousePressEvent(QMouseEvent *);
};

class EmbedderOptions : public QWidget, public Ui::EmbedderOptions
{
  Q_OBJECT

  public:
    QMap<QListWidgetItem *, QPixmap> snapshotMap;
    SnapshotViewer *snapshotViewer;

    EmbedderOptions(QWidget *parent = 0);
    ~EmbedderOptions();

  public slots:
    void on_toolButtonTakeSnapshot_clicked();
    void on_toolButtonSaveSnapshot_clicked();
    void on_toolButtonSaveSnapshotAs_clicked();
    void on_listWidgetSnapshots_itemPressed(QListWidgetItem *);
};
#endif

#endif
