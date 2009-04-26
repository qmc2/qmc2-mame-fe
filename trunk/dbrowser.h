#ifndef _DBROWSER_H_
#define _DBROWSER_H_

#include "ui_dbrowser.h"

class DocBrowser : public QDialog, public Ui::DocBrowser
{
  Q_OBJECT

  public:
    QPoint widgetPos;
    QSize widgetSize;
    bool widgetPosValid;
    bool ignoreResizeAndMove;

    DocBrowser(QWidget *parent = 0);
    ~DocBrowser();

  public slots:

  protected:
    void showEvent(QShowEvent *);
    void moveEvent(QMoveEvent *);
    void resizeEvent(QResizeEvent *);
};

#endif
