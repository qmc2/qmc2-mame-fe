#ifndef _MINIWEBBROWSER_H_
#define _MINIWEBBROWSER_H_

#include "ui_miniwebbrowser.h"

class MiniWebBrowser : public QWidget, public Ui::MiniWebBrowser
{
  Q_OBJECT

  public:
    QUrl homeUrl;
    bool firstTimeLoadStarted,
         firstTimeLoadProgress,
         firstTimeLoadFinished;

    MiniWebBrowser(QWidget *parent = 0);
    ~MiniWebBrowser();

  public slots:
    void on_lineEditURL_returnPressed();
    void on_webViewBrowser_linkClicked(const QUrl);
    void on_webViewBrowser_urlChanged(const QUrl);
    void on_webViewBrowser_loadStarted();
    void on_webViewBrowser_loadFinished(bool);
    void on_webViewBrowser_loadProgress(int);
    void on_toolButtonHome_clicked();
};

#endif
