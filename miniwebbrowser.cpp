#include <QWebHistory>
#include "miniwebbrowser.h"

#include "macros.h"
#ifdef QMC2_DEBUG
#include "qmc2main.h"
extern MainWindow *qmc2MainWindow;
#endif

MiniWebBrowser::MiniWebBrowser(QWidget *parent)
  : QWidget(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MiniWebBrowser::MiniWebBrowser(QWidget *parent = %1)").arg((qulonglong) parent));
#endif

  firstTimeLoadStarted = TRUE;
  firstTimeLoadProgress = TRUE;
  firstTimeLoadFinished = TRUE;
  setupUi(this);
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

  if ( !lineEditURL->text().isEmpty() )
    webViewBrowser->load(QUrl(lineEditURL->text()));
}

void MiniWebBrowser::on_webViewBrowser_linkClicked(const QUrl)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MiniWebBrowser::on_webViewBrowser_linkClicked()");
#endif

}

void MiniWebBrowser::on_webViewBrowser_urlChanged(const QUrl)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MiniWebBrowser::on_webViewBrowser_urlChanged()");
#endif

  lineEditURL->setText(webViewBrowser->url().toString());
}

void MiniWebBrowser::on_webViewBrowser_loadStarted()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MiniWebBrowser::on_webViewBrowser_loadStarted()");
#endif

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

  if ( firstTimeLoadProgress ) {
    firstTimeLoadProgress = FALSE;
    homeUrl = webViewBrowser->url();
    webViewBrowser->history()->clear();
    toolButtonBack->setEnabled(FALSE);
    toolButtonForward->setEnabled(FALSE);
    toolButtonHome->setEnabled(TRUE);
  }
}

void MiniWebBrowser::on_webViewBrowser_loadFinished(bool ok)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MiniWebBrowser::on_webViewBrowser_loadFinished(bool ok = %1)").arg(ok));
#endif

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

void MiniWebBrowser::on_toolButtonHome_clicked()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MiniWebBrowser::on_toolButtonHome_clicked()");
#endif

  if ( homeUrl.isValid() )
    webViewBrowser->load(homeUrl);
}
