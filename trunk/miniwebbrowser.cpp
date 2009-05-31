#include <QWebHistory>
#include <QWebFrame>

#include "miniwebbrowser.h"
#include "macros.h"
#include "qmc2main.h"

extern MainWindow *qmc2MainWindow;

MiniWebBrowser::MiniWebBrowser(QWidget *parent)
  : QWidget(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MiniWebBrowser::MiniWebBrowser(QWidget *parent = %1)").arg((qulonglong) parent));
#endif

  setupUi(this);

  labelStatus->hide();
  progressBar->hide();

  firstTimeLoadStarted = TRUE;
  firstTimeLoadProgress = TRUE;
  firstTimeLoadFinished = TRUE;

  // we want to manipulate the link activation
  webViewBrowser->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);

  // connect page actions we provide
  connect(webViewBrowser->pageAction(QWebPage::DownloadImageToDisk), SIGNAL(triggered()), this, SLOT(processPageActionDownloadImageToDisk()));
  connect(webViewBrowser->pageAction(QWebPage::DownloadLinkToDisk), SIGNAL(triggered()), this, SLOT(processPageActionDownloadLinkToDisk()));
  connect(webViewBrowser->page(), SIGNAL(downloadRequested(const QNetworkRequest &)), this, SLOT(processPageActionDownloadRequested(const QNetworkRequest &)));
  connect(webViewBrowser->page(), SIGNAL(unsupportedContent(QNetworkReply *)), this, SLOT(processPageActionHandleUnsupportedContent(QNetworkReply *)));
  connect(webViewBrowser->page(), SIGNAL(linkHovered(const QString &, const QString &, const QString &)), this, SLOT(webViewBrowser_linkHovered(const QString &, const QString &, const QString &)));
  connect(webViewBrowser->page(), SIGNAL(statusBarVisibilityChangeRequested(bool)), this, SLOT(webViewBrowser_statusBarVisibilityChangeRequested(bool)));

  // hide page actions we don't provide
  webViewBrowser->pageAction(QWebPage::OpenImageInNewWindow)->setVisible(FALSE);
  webViewBrowser->pageAction(QWebPage::OpenFrameInNewWindow)->setVisible(FALSE);
  webViewBrowser->pageAction(QWebPage::OpenLinkInNewWindow)->setVisible(FALSE);

  // change provided page actions to better fit our usage / integrate into QMC2's look
  webViewBrowser->pageAction(QWebPage::OpenLink)->setText(tr("Open link"));
  webViewBrowser->pageAction(QWebPage::OpenLink)->setIcon(QIcon(QString::fromUtf8(":/data/img/fileopen.png")));
  webViewBrowser->pageAction(QWebPage::DownloadLinkToDisk)->setText(tr("Save link as..."));
  webViewBrowser->pageAction(QWebPage::DownloadLinkToDisk)->setIcon(QIcon(QString::fromUtf8(":/data/img/filesaveas.png")));
  webViewBrowser->pageAction(QWebPage::CopyLinkToClipboard)->setText(tr("Copy link"));
  webViewBrowser->pageAction(QWebPage::CopyLinkToClipboard)->setIcon(QIcon(QString::fromUtf8(":/data/img/editcopy.png")));
  webViewBrowser->pageAction(QWebPage::DownloadImageToDisk)->setText(tr("Save image as..."));
  webViewBrowser->pageAction(QWebPage::DownloadImageToDisk)->setIcon(QIcon(QString::fromUtf8(":/data/img/filesaveas.png")));
  webViewBrowser->pageAction(QWebPage::CopyImageToClipboard)->setText(tr("Copy image"));
  webViewBrowser->pageAction(QWebPage::CopyImageToClipboard)->setIcon(QIcon(QString::fromUtf8(":/data/img/editcopy.png")));
  webViewBrowser->pageAction(QWebPage::Back)->setText(tr("Go back"));
  webViewBrowser->pageAction(QWebPage::Back)->setIcon(QIcon(QString::fromUtf8(":/data/img/back.png")));
  webViewBrowser->pageAction(QWebPage::Forward)->setText(tr("Go forward"));
  webViewBrowser->pageAction(QWebPage::Forward)->setIcon(QIcon(QString::fromUtf8(":/data/img/forward.png")));
  webViewBrowser->pageAction(QWebPage::Reload)->setText(tr("Reload"));
  webViewBrowser->pageAction(QWebPage::Reload)->setIcon(QIcon(QString::fromUtf8(":/data/img/reload.png")));
  webViewBrowser->pageAction(QWebPage::Stop)->setText(tr("Stop"));
  webViewBrowser->pageAction(QWebPage::Stop)->setIcon(QIcon(QString::fromUtf8(":/data/img/stop_browser.png")));
  webViewBrowser->pageAction(QWebPage::Copy)->setText(tr("Copy"));
  webViewBrowser->pageAction(QWebPage::Copy)->setIcon(QIcon(QString::fromUtf8(":/data/img/editcopy.png")));
#if defined(QMC2_BROWSER_EXTRAS_ENABLED)
  webViewBrowser->pageAction(QWebPage::InspectElement)->setText(tr("Inspect"));
  webViewBrowser->pageAction(QWebPage::InspectElement)->setIcon(QIcon(QString::fromUtf8(":/data/img/inspect.png")));
#endif

  // setup browser settings
  webViewBrowser->page()->settings()->setAttribute(QWebSettings::AutoLoadImages, TRUE);
  webViewBrowser->page()->settings()->setAttribute(QWebSettings::JavascriptEnabled, TRUE);
  webViewBrowser->page()->settings()->setAttribute(QWebSettings::JavaEnabled, TRUE);
  webViewBrowser->page()->settings()->setAttribute(QWebSettings::PluginsEnabled, FALSE);
  webViewBrowser->page()->settings()->setAttribute(QWebSettings::PrivateBrowsingEnabled, FALSE);
  webViewBrowser->page()->settings()->setAttribute(QWebSettings::JavascriptCanOpenWindows, FALSE);
  webViewBrowser->page()->settings()->setAttribute(QWebSettings::JavascriptCanAccessClipboard, FALSE);
#if defined(QMC2_BROWSER_EXTRAS_ENABLED)
  webViewBrowser->page()->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, TRUE);
#else
  webViewBrowser->page()->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, FALSE);
#endif
  webViewBrowser->page()->settings()->setAttribute(QWebSettings::LinksIncludedInFocusChain, FALSE);
  webViewBrowser->page()->settings()->setAttribute(QWebSettings::ZoomTextOnly, FALSE);
  webViewBrowser->page()->settings()->setAttribute(QWebSettings::PrintElementBackgrounds, FALSE);
  webViewBrowser->page()->settings()->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled, FALSE);
  webViewBrowser->page()->settings()->setAttribute(QWebSettings::OfflineWebApplicationCacheEnabled, FALSE);
  webViewBrowser->page()->settings()->setAttribute(QWebSettings::LocalStorageDatabaseEnabled, FALSE);

  // status bar timeout connection
  connect(&statusTimer, SIGNAL(timeout()), this, SLOT(statusTimeout()));
}

MiniWebBrowser::~MiniWebBrowser()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MiniWebBrowser::~MiniWebBrowser()");
#endif

}

void MiniWebBrowser::on_lineEditURL_returnPressed()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MiniWebBrowser::on_lineEditURL_returnPressed()");
#endif

  if ( !lineEditURL->text().isEmpty() ) {
    QString text = lineEditURL->text().toLower();
    QUrl url(text, QUrl::TolerantMode);
    if ( url.scheme().isEmpty() )
      if ( !text.startsWith("http://") )
        text.prepend("http://");
    webViewBrowser->load(QUrl(text, QUrl::TolerantMode));
  }
}

void MiniWebBrowser::on_webViewBrowser_linkClicked(const QUrl url)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MiniWebBrowser::on_webViewBrowser_linkClicked(const QUrl url = %1)").arg(url.toString()));
#endif

  if ( url.isValid() )
    webViewBrowser->load(url);
}

void MiniWebBrowser::on_webViewBrowser_urlChanged(const QUrl url)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MiniWebBrowser::on_webViewBrowser_urlChanged(const QUrl url = %1)").arg(url.toString()));
#endif

  lineEditURL->setText(QString::fromUtf8(webViewBrowser->url().toEncoded()));

  QString newTitle = webViewBrowser->page()->mainFrame()->title();
  if ( newTitle.isEmpty() ) newTitle = tr("No title");
  emit titleChanged(newTitle);
}

void MiniWebBrowser::on_webViewBrowser_loadStarted()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MiniWebBrowser::on_webViewBrowser_loadStarted()");
#endif

  progressBar->reset();
  progressBar->setRange(0, 100);
  progressBar->setValue(0);
  QFont f(font());
  f.setPointSize(f.pointSize() - 2);
  progressBar->setFont(f);
  progressBar->setMaximumHeight(f.pointSize() + 4);
  progressBar->show();

  if ( firstTimeLoadStarted ) {
    firstTimeLoadStarted = FALSE;
    homeUrl = webViewBrowser->url();
    webViewBrowser->history()->clear();
    toolButtonStop->setEnabled(TRUE);
    toolButtonReload->setEnabled(FALSE);
    toolButtonBack->setEnabled(FALSE);
    toolButtonForward->setEnabled(FALSE);
    toolButtonHome->setEnabled(TRUE);
  } else {
    toolButtonStop->setEnabled(TRUE);
    toolButtonReload->setEnabled(FALSE);
    toolButtonBack->setEnabled(webViewBrowser->history()->canGoBack());
    toolButtonForward->setEnabled(webViewBrowser->history()->canGoForward());
    toolButtonHome->setEnabled(TRUE);
  }
}

void MiniWebBrowser::on_webViewBrowser_loadProgress(int progress)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MiniWebBrowser::on_webViewBrowser_loadProgress(int progress = %1)").arg(progress));
#endif

  progressBar->setValue(progress);

  if ( firstTimeLoadProgress ) {
    firstTimeLoadProgress = FALSE;
    homeUrl = webViewBrowser->url();
    webViewBrowser->history()->clear();
    toolButtonBack->setEnabled(FALSE);
    toolButtonForward->setEnabled(FALSE);
    toolButtonHome->setEnabled(TRUE);
  } else {
    toolButtonBack->setEnabled(webViewBrowser->history()->canGoBack());
    toolButtonForward->setEnabled(webViewBrowser->history()->canGoForward());
  }
}

void MiniWebBrowser::on_webViewBrowser_loadFinished(bool ok)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MiniWebBrowser::on_webViewBrowser_loadFinished(bool ok = %1)").arg(ok));
#endif

  progressBar->reset();
  progressBar->hide();

  if ( firstTimeLoadFinished ) {
    firstTimeLoadFinished = FALSE;
    homeUrl = webViewBrowser->url();
    webViewBrowser->history()->clear();
    toolButtonBack->setEnabled(FALSE);
    toolButtonForward->setEnabled(FALSE);
  } else {
    toolButtonBack->setEnabled(webViewBrowser->history()->canGoBack());
    toolButtonForward->setEnabled(webViewBrowser->history()->canGoForward());
  }
  toolButtonStop->setEnabled(FALSE);
  toolButtonReload->setEnabled(TRUE);
  toolButtonHome->setEnabled(TRUE);
}

void MiniWebBrowser::on_webViewBrowser_statusBarMessage(const QString &message)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MiniWebBrowser::on_webViewBrowser_statusBarMessage(const QString &message = %1)").arg(message));
#endif

  if ( !message.isEmpty() ) {
    statusTimer.stop();
    labelStatus->setVisible(TRUE);
    QFont f(font());
    f.setPointSize(f.pointSize() - 2);
    labelStatus->setFont(f);
    labelStatus->setMaximumHeight(f.pointSize() + 4);
    labelStatus->setText(message + " ");
  } else {
    labelStatus->clear();
    statusTimer.start(QMC2_BROWSER_STATUS_TIMEOUT);
  }
}

void MiniWebBrowser::on_toolButtonHome_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MiniWebBrowser::on_toolButtonHome_clicked()");
#endif

  if ( homeUrl.isValid() )
    webViewBrowser->load(homeUrl);
}

void MiniWebBrowser::on_toolButtonLoad_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MiniWebBrowser::on_toolButtonLoad_clicked()");
#endif

  if ( !lineEditURL->text().isEmpty() ) {
    QString text = lineEditURL->text().toLower();
    QUrl url(text, QUrl::TolerantMode);
    if ( url.scheme().isEmpty() )
      if ( !text.startsWith("http://") )
        text.prepend("http://");
    webViewBrowser->load(QUrl(text, QUrl::TolerantMode));
  }
}

void MiniWebBrowser::processPageActionDownloadImageToDisk()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MiniWebBrowser::processPageActionDownloadImageToDisk()");
#endif

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "MiniWebBrowser::processPageActionDownloadImageToDisk(): "+ tr("sorry, this feature is not yet implemented"));
}

void MiniWebBrowser::processPageActionDownloadLinkToDisk()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MiniWebBrowser::processPageActionDownloadLinkToDisk()");
#endif

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "MiniWebBrowser::processPageActionDownloadLinkToDisk(): "+ tr("sorry, this feature is not yet implemented"));
}

void MiniWebBrowser::processPageActionDownloadRequested(const QNetworkRequest &request)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MiniWebBrowser::processPageActionDownloadRequested(const QNetworkRequest &request = ...)");
#endif

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "MiniWebBrowser::processPageActionDownloadRequested(): "+ tr("sorry, this feature is not yet implemented"));
}

void MiniWebBrowser::processPageActionHandleUnsupportedContent(QNetworkReply *reply)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MiniWebBrowser::processPageActionHandleUnsupportedContent(QNetworkReply *reply = ...)");
#endif

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "MiniWebBrowser::processPageActionHandleUnsupportedContent(): "+ tr("sorry, this feature is not yet implemented"));
}

void MiniWebBrowser::webViewBrowser_linkHovered(const QString &link, const QString &title, const QString &textContent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MiniWebBrowser::webViewBrowser_linkHovered(const QString &link = %1, const QString &title = %2, const QString &textContent = %3)").arg(link).arg(title).arg(textContent));
#endif
  
  if ( !link.isEmpty() ) {
    statusTimer.stop();
    labelStatus->setVisible(TRUE);
    QFont f(font());
    f.setPointSize(f.pointSize() - 2);
    labelStatus->setFont(f);
    labelStatus->setMaximumHeight(f.pointSize() + 4);
    labelStatus->setText(link + " ");
  } else {
    labelStatus->clear();
    statusTimer.start(QMC2_BROWSER_STATUS_TIMEOUT);
  }
}

void MiniWebBrowser::webViewBrowser_statusBarVisibilityChangeRequested(bool visible)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MiniWebBrowser::webViewBrowser_statusBarVisibilityChangeRequested(bool visible = %1)").arg(visible));
#endif
  
  labelStatus->setVisible(visible);
}

void MiniWebBrowser::statusTimeout()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MiniWebBrowser::statusTimeout()");
#endif

  statusTimer.stop();
  labelStatus->setVisible(FALSE);
}
