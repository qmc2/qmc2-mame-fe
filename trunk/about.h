#ifndef _ABOUT_H_
#define _ABOUT_H_

#include "ui_about.h"

class About : public QDialog, public Ui::About
{
  Q_OBJECT

  public:
    QPoint widgetPos;
    QSize widgetSize;
    bool widgetPosValid;
    bool ignoreResizeAndMove;
#if defined(Q_WS_MAC)
    QString macVersion;
#elif defined(Q_WS_WIN)
    QString winVersion;
#endif

    About(QWidget *parent = 0);
    ~About();

  public slots:

  protected:
    void showEvent(QShowEvent *);
    void moveEvent(QMoveEvent *);
    void resizeEvent(QResizeEvent *);
};

#endif
