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
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: MiniWebBrowser::on_lineEditURL_editingFinished()");
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

  toolButtonStop->setEnabled(TRUE);
  toolButtonReload->setEnabled(FALSE);
  toolButtonBack->setEnabled(webViewBrowser->history()->canGoBack());
  toolButtonForward->setEnabled(webViewBrowser->history()->canGoForward());
}

void MiniWebBrowser::on_webViewBrowser_loadProgress(int progress)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MiniWebBrowser::on_webViewBrowser_loadProgress(int progress = %1)").arg(progress));
#endif

  static bool firstTime = TRUE;

  if ( firstTime ) {
    firstTime = FALSE;
    webViewBrowser->history()->clear();
    toolButtonBack->setEnabled(webViewBrowser->history()->canGoBack());
    toolButtonForward->setEnabled(webViewBrowser->history()->canGoForward());
  }
}

void MiniWebBrowser::on_webViewBrowser_loadFinished(bool ok)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: MiniWebBrowser::on_webViewBrowser_loadFinished(bool ok = %1)").arg(ok));
#endif

  static bool firstTime = TRUE;

  if ( firstTime ) {
    firstTime = FALSE;
    webViewBrowser->history()->clear();
  }
  toolButtonStop->setEnabled(FALSE);
  toolButtonReload->setEnabled(TRUE);
  toolButtonBack->setEnabled(webViewBrowser->history()->canGoBack());
  toolButtonForward->setEnabled(webViewBrowser->history()->canGoForward());
}
