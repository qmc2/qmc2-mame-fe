#include <QSettings>

#include "docbrowser.h"
#include "qmc2main.h"
#include "macros.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern QSettings *qmc2Config;

DocBrowser::DocBrowser(QWidget *parent)
#if defined(Q_WS_WIN)
  : QDialog(parent, Qt::Dialog)
#else
  : QDialog(parent, Qt::Dialog | Qt::SubWindow)
#endif
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DocBrowser::DocBrowser(QWidget *parent = 0x" + QString::number((ulong)parent, 16) + ")");
#endif

  setupUi(this);

  browser = new MiniWebBrowser(this);
  verticalLayout->addWidget(browser);

  widgetSize = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/DocBrowser/Size", QSize(600, 600)).toSize();
  resize(widgetSize);

  widgetPos = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/DocBrowser/Pos", QPoint((parent->width() - width()) / 2, (parent->height() - height()) / 2)).toPoint();
  move(widgetPos);

  connect(browser, SIGNAL(titleChanged(QString &)), this, SLOT(titleChanged(QString &)));
}

DocBrowser::~DocBrowser()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DocBrowser::~DocBrowser()");
#endif

  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/DocBrowser/Size", size());
  qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/DocBrowser/Pos", pos());
}

void DocBrowser::showEvent(QShowEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DocBrowser::showEvent(QShowEvent *e = 0x" + QString::number((ulong)e, 16) + ")");
#endif

  e->accept();
}

void DocBrowser::resizeEvent(QResizeEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DocBrowser::resizeEvent(QResizeEvent *e = 0x" + QString::number((ulong)e, 16) + ")");
#endif

  e->accept();
}

void DocBrowser::moveEvent(QMoveEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DocBrowser::moveEvent(QMoveEvent *e = 0x" + QString::number((ulong)e, 16) + ")");
#endif

  e->accept();
}

void DocBrowser::titleChanged(QString &title)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DocBrowser::titleChanged(QString &title = ...");
#endif

  setWindowTitle(tr("MiniWebBrowser") + " :: " + title);
}
