#ifndef _WELCOME_H_
#define _WELCOME_H_

#include <QSettings>
#include "ui_welcome.h"

class Welcome : public QDialog, public Ui::Welcome
{
  Q_OBJECT

  public:
    Welcome(QWidget *parent = 0);
    ~Welcome();

    bool checkOkay;
    QSettings *startupConfig;
    QString variant;
    QString fallbackVariant;

    bool checkConfig();

  public slots:
    void on_pushButtonOkay_clicked();
    void on_toolButtonBrowseExecutableFile_clicked();
    void on_toolButtonBrowseWorkingDirectory_clicked();
    void on_toolButtonBrowseROMPath_clicked();
    void on_toolButtonBrowseSamplePath_clicked();
    void on_toolButtonBrowseHashPath_clicked();
    void setupLanguage();
};

#endif
