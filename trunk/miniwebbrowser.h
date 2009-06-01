#ifndef _MINIWEBBROWSER_H_
#define _MINIWEBBROWSER_H_

#include <QTimer>
#include <QCache>
#include "ui_miniwebbrowser.h"

class MiniWebBrowser : public QWidget, public Ui::MiniWebBrowser
{
  Q_OBJECT

  public:
    QUrl homeUrl;
    static QCache<QString, QIcon> iconCache;
    bool firstTimeLoadStarted,
         firstTimeLoadProgress,
         firstTimeLoadFinished;
    QTimer statusTimer;

    MiniWebBrowser(QWidget *parent = 0);
    ~MiniWebBrowser();

  public slots:
    void on_comboBoxURL_activated();
    void on_webViewBrowser_linkClicked(const QUrl);
    void on_webViewBrowser_urlChanged(const QUrl);
    void on_webViewBrowser_loadStarted();
    void on_webViewBrowser_loadFinished(bool);
    void on_webViewBrowser_loadProgress(int);
    void on_webViewBrowser_statusBarMessage(const QString &);
    void on_webViewBrowser_iconChanged();
    void on_toolButtonHome_clicked();
    void on_toolButtonLoad_clicked();

    // page actions
    void processPageActionDownloadRequested(const QNetworkRequest &);
    void processPageActionHandleUnsupportedContent(QNetworkReply *);

    // other
    void webViewBrowser_linkHovered(const QString &, const QString &, const QString &);
    void webViewBrowser_statusBarVisibilityChangeRequested(bool);
    void statusTimeout();

  signals:
    void titleChanged(QString &);
};

#endif
