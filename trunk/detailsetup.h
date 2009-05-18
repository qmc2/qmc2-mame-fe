#ifndef _DETAILSETUP_H_
#define _DETAILSETUP_H_

#include "ui_detailsetup.h"

class DetailSetup : public QDialog, public Ui::DetailSetup
{
  Q_OBJECT

  public:
    DetailSetup(QWidget *parent = 0);
    ~DetailSetup();

  public slots:

};

#endif
