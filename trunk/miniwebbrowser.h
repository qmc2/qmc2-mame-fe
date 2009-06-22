#ifndef _MINIWEBBROWSER_H_
#define _MINIWEBBROWSER_H_

#include <QWebView>
#include <QMouseEvent>
#include <QTimer>
#include <QCache>
#include "ui_miniwebbrowser.h"

class BrowserWidget : public QWebView
{
  Q_OBJECT

  public:
    QPoint lastMouseClickPosition;
    bool mouseCurrentlyOnView;
    BrowserWidget(QWidget *parent = NULL) : QWebView(parent)
    {
      lastMouseClickPosition = QPoint(-1, -1);
      mouseCurrentlyOnView = FALSE;
    }

  signals:
    void mouseOnView(bool);

  protected:
    void mousePressEvent(QMouseEvent *e)
    {
      lastMouseClickPosition = e->pos();
      QWebView::mousePressEvent(e);
    }
    void enterEvent(QEvent *e)
    {
      QWebView::enterEvent(e);
      mouseCurrentlyOnView = TRUE;
      emit mouseOnView(TRUE);
    }
    void leaveEvent(QEvent *e)
    {
      QWebView::leaveEvent(e);
      mouseCurrentlyOnView = FALSE;
      emit mouseOnView(FALSE);
    }
};

class MiniWebBrowser : public QWidget, public Ui::MiniWebBrowser
{
  Q_OBJECT

  public:
    QUrl homeUrl;
    static QCache<QString, QIcon> iconCache;
    static QStringList supportedSchemes;
    bool firstTimeLoadStarted,
         firstTimeLoadProgress,
         firstTimeLoadFinished;
    QTimer statusTimer;
    BrowserWidget *webViewBrowser;

    MiniWebBrowser(QWidget *parent = 0);
    ~MiniWebBrowser();

  public slots:
    void on_comboBoxURL_activated();
    void on_toolButtonHome_clicked();
    void on_toolButtonLoad_clicked();

    // page actions
    void processPageActionDownloadRequested(const QNetworkRequest &);
    void processPageActionHandleUnsupportedContent(QNetworkReply *);

    // other
    void webViewBrowser_linkClicked(const QUrl);
    void webViewBrowser_urlChanged(const QUrl);
    void webViewBrowser_loadStarted();
    void webViewBrowser_loadFinished(bool);
    void webViewBrowser_loadProgress(int);
    void webViewBrowser_statusBarMessage(const QString &);
    void webViewBrowser_iconChanged();
    void webViewBrowser_linkHovered(const QString &, const QString &, const QString &);
    void webViewBrowser_statusBarVisibilityChangeRequested(bool);
    void webViewBrowser_frameCreated(QWebFrame *);
    void statusTimeout();

  signals:
    void titleChanged(QString &);
};

#endif
