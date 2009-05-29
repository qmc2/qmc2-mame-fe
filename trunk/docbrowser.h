#ifndef _DBROWSER_H_
#define _DBROWSER_H_

#include "ui_docbrowser.h"
#include "miniwebbrowser.h"

class DocBrowser : public QDialog, public Ui::DocBrowser
{
  Q_OBJECT

  public:
    QPoint widgetPos;
    QSize widgetSize;
    bool widgetPosValid;
    bool ignoreResizeAndMove;
    MiniWebBrowser *browser;

    DocBrowser(QWidget *parent = 0);
    ~DocBrowser();

  public slots:

  protected:
    void showEvent(QShowEvent *);
    void moveEvent(QMoveEvent *);
    void resizeEvent(QResizeEvent *);
};

#endif
