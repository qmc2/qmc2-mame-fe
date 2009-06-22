#ifndef _MAWSQDLSETUP_H_
#define _MAWSQDLSETUP_H_

#include "ui_mawsqdlsetup.h"

class MawsQuickDownloadSetup : public QDialog, public Ui::MawsQuickDownloadSetup
{
  Q_OBJECT

  public:
    MawsQuickDownloadSetup(QWidget *parent = 0);
    ~MawsQuickDownloadSetup();

  public slots:
    void on_pushButtonOk_clicked();
    void on_pushButtonCancel_clicked();
};

#endif
