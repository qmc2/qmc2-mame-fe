#ifndef _EMBEDDEROPT_H_
#define _EMBEDDEROPT_H_

#include <QtGui>
#if QT_VERSION >= 0x050000
#include "macros.h"
#endif
#include "ui_embedderopt.h"

#if defined(Q_WS_X11) || defined(Q_WS_WIN)
class SnapshotViewer : public QWidget
{
  Q_OBJECT

  public:
    QListWidgetItem *myItem;
    QMenu *contextMenu;
    QString fileName;

    SnapshotViewer(QListWidgetItem *item, QWidget *parent = 0);

  public slots:
    void useAsPreview();
    void useAsTitle();
    void copyToClipboard();
    void saveAs();

  protected:
    void leaveEvent(QEvent *);
    void mousePressEvent(QMouseEvent *);
    void contextMenuEvent(QContextMenuEvent *);
    void keyPressEvent(QKeyEvent *);
    void paintEvent(QPaintEvent *);
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
    void on_toolButtonClearSnapshots_clicked();
    void on_listWidgetSnapshots_itemPressed(QListWidgetItem *);
    void on_checkBoxNativeSnapshotResolution_toggled(bool);
    void adjustIconSizes();
};
#endif

#endif
