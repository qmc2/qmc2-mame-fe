#ifndef _DEMOMODE_H_
#define _DEMOMODE_H_

#include <QProcess>
#include "ui_demomode.h"

class DemoModeDialog : public QDialog, public Ui::DemoModeDialog
{
  Q_OBJECT

  public:
    QStringList selectedGames;
    bool demoModeRunning;

    DemoModeDialog(QWidget *parent = 0);
    ~DemoModeDialog();

  public slots:
    void adjustIconSizes();
    void on_pushButtonRunDemo_clicked();
    void emuFinished(int, QProcess::ExitStatus);
    void startNextEmu();

  protected:
    void closeEvent(QCloseEvent *);
    void hideEvent(QHideEvent *) { closeEvent(NULL); };
};

#endif
