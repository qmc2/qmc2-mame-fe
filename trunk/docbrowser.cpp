#include "docbrowser.h"
#include "options.h"
#include "qmc2main.h"
#include "macros.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern Options *qmc2Options;

DocBrowser::DocBrowser(QWidget *parent)
  : QDialog(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DocBrowser::DocBrowser(QWidget *parent = 0x" + QString::number((ulong)parent, 16) + ")");
#endif

  setupUi(this);

  browser = new MiniWebBrowser(this);
  verticalLayout->addWidget(browser);

  widgetSize = QSize(-1, -1);
  widgetPosValid = FALSE;
  ignoreResizeAndMove = TRUE;
}

DocBrowser::~DocBrowser()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DocBrowser::~DocBrowser()");
#endif

}

void DocBrowser::showEvent(QShowEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DocBrowser::showEvent(QShowEvent *e = 0x" + QString::number((ulong)e, 16) + ")");
#endif

  ignoreResizeAndMove = TRUE;

  if ( widgetSize.isValid() )
    resize(widgetSize);
  if ( widgetPosValid )
    move(widgetPos);

  ignoreResizeAndMove = FALSE;

  e->accept();
}

void DocBrowser::resizeEvent(QResizeEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DocBrowser::resizeEvent(QResizeEvent *e = 0x" + QString::number((ulong)e, 16) + ")");
#endif

  if ( !ignoreResizeAndMove )
    widgetSize = size();

  e->accept();
}

void DocBrowser::moveEvent(QMoveEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DocBrowser::moveEvent(QMoveEvent *e = 0x" + QString::number((ulong)e, 16) + ")");
#endif

  if ( !ignoreResizeAndMove ) {
    widgetPos = pos();
    widgetPosValid = TRUE;
  }

  e->accept();
}
