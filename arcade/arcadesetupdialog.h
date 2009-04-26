#ifndef _ARCADESETUPDIALOG_H_
#define _ARCADESETUPDIALOG_H_

#include "ui_arcadesetupdialog.h"

class ArcadeSetupDialog : public QDialog, public Ui::ArcadeSetupDialog
{
  Q_OBJECT

  public:
    ArcadeSetupDialog(QWidget *parent = 0);
    ~ArcadeSetupDialog();

  public slots:

  signals:

  protected:
    void closeEvent(QCloseEvent *);
    void showEvent(QShowEvent *);
    void hideEvent(QHideEvent *);
    void moveEvent(QMoveEvent *);
    void resizeEvent(QResizeEvent *);
};

#endif
