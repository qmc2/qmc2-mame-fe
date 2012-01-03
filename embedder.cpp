#include <QtGui>

#if defined(Q_WS_X11) || defined(Q_WS_WIN)

#if defined(Q_WS_X11)
#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#elif defined(Q_WS_WIN)
#include <QTest>
#include "windows_tools.h"
#include "procmgr.h"
#endif
#include "embedder.h"
#include "qmc2main.h"
#include "macros.h"

extern MainWindow *qmc2MainWindow;
extern QSettings *qmc2Config;
extern bool qmc2FifoIsOpen;

#if defined(Q_WS_WIN)
#define QMC2_EMBEDDED_STYLE		(LONG)(WS_VISIBLE | WS_CHILDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_MAXIMIZE)
#define QMC2_RELEASED_STYLE		(LONG)(WS_VISIBLE | WS_OVERLAPPEDWINDOW)
extern ProcessManager *qmc2ProcessManager;
#endif

Embedder::Embedder(QString name, QString id, WId wid, bool currentlyPaused, QWidget *parent, QIcon icon)
    : QWidget(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::Embedder(QString name = %1, QString id = %2, WId wid = %3, bool currentlyPaused = %4, QWidget *parent = %5, ...)").arg(name).arg(id).arg((qulonglong)wid).arg(currentlyPaused).arg((qulonglong)parent));
#endif

  gameName = name;
  gameID = id;
  winId = wid;
#if defined(Q_WS_X11)
  embedded = pauseKeyPressed = pausing = resuming = false;
  isPaused = currentlyPaused;
#endif

  setAttribute(Qt::WA_NativeWindow);
  setAttribute(Qt::WA_DontCreateNativeAncestors);
  createWinId();

#if defined(Q_WS_X11)
  embedContainer = new QX11EmbedContainer(this);
#elif defined(Q_WS_WIN)
  embedContainer = new QWidget(this);
#endif
  embedContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  embedContainer->setObjectName("QMC2_EMBED_CONTAINER");
  embedContainer->setAutoFillBackground(true);
  QPalette pal = embedContainer->palette();
  pal.setColor(QPalette::Window, Qt::black);
  embedContainer->setPalette(pal);

  setFocusPolicy(Qt::WheelFocus);
  setFocusProxy(embedContainer);

  gridLayout = new QGridLayout(this);
  gridLayout->getContentsMargins(&cmLeft, &cmTop, &cmRight, &cmBottom);
  gridLayout->setContentsMargins(0, 0, 0, 0);
  gridLayout->setRowStretch(0, 0);
  gridLayout->setRowStretch(1, 4);
  gridLayout->setColumnStretch(0, 4);
  gridLayout->addWidget(embedContainer, 1, 0);
  setLayout(gridLayout);

#if defined(Q_WS_X11)
  connect(embedContainer, SIGNAL(clientIsEmbedded()), SLOT(clientEmbedded()));
  connect(embedContainer, SIGNAL(clientClosed()), SLOT(clientClosed()));
  connect(embedContainer, SIGNAL(error(QX11EmbedContainer::Error)), SLOT(clientError(QX11EmbedContainer::Error)));

  if ( icon.isNull() ) {
	  iconRunning = QIcon(QString::fromUtf8(":/data/img/trafficlight_green.png"));
	  iconPaused = QIcon(QString::fromUtf8(":/data/img/trafficlight_yellow.png"));
	  iconStopped = QIcon(QString::fromUtf8(":/data/img/trafficlight_red.png"));
	  iconUnknown = QIcon(QString::fromUtf8(":/data/img/trafficlight_off.png"));
  } else {
	  QPainter p;
	  QPixmap pm(128, 64);
	  QPixmap pmStatus;
	  QPixmap pmIcon = icon.pixmap(icon.actualSize(QSize(64, 64))).scaled(64, 64, Qt::KeepAspectRatioByExpanding);
	  pmIcon.setMask(pmIcon.createMaskFromColor(QColor(255, 255, 255), Qt::MaskInColor));

	  pmStatus = QIcon(QString::fromUtf8(":/data/img/trafficlight_green.png")).pixmap(64, 64);
	  pm.fill(Qt::transparent);
	  p.begin(&pm);
	  p.setBackgroundMode(Qt::TransparentMode);
	  p.drawPixmap(0, 0, pmStatus);
	  p.drawPixmap(64, 0, pmIcon);
	  p.end();
	  iconRunning = QIcon(pm);

	  pmStatus = QIcon(QString::fromUtf8(":/data/img/trafficlight_yellow.png")).pixmap(64, 64);
	  pm.fill(Qt::transparent);
	  p.begin(&pm);
	  p.setBackgroundMode(Qt::TransparentMode);
	  p.drawPixmap(0, 0, pmStatus);
	  p.drawPixmap(64, 0, pmIcon);
	  p.end();
	  iconPaused = QIcon(pm);

	  pmStatus = QIcon(QString::fromUtf8(":/data/img/trafficlight_red.png")).pixmap(64, 64);
	  pm.fill(Qt::transparent);
	  p.begin(&pm);
	  p.setBackgroundMode(Qt::TransparentMode);
	  p.drawPixmap(0, 0, pmStatus);
	  p.drawPixmap(64, 0, pmIcon);
	  p.end();
	  iconStopped = QIcon(pm);

	  pmStatus = QIcon(QString::fromUtf8(":/data/img/trafficlight_off.png")).pixmap(64, 64);
	  pm.fill(Qt::transparent);
	  p.begin(&pm);
	  p.setBackgroundMode(Qt::TransparentMode);
	  p.drawPixmap(0, 0, pmStatus);
	  p.drawPixmap(64, 0, pmIcon);
	  p.end();
	  iconUnknown = QIcon(pm);
  }
#elif defined(Q_WS_WIN)
  windowHandle = winId;
  embeddingWindow = releasingWindow = checkingWindow = updatingWindow = fullScreen = false;
  connect(&checkTimer, SIGNAL(timeout()), this, SLOT(checkWindow()));
  iconUnknown = icon;
#endif

  embedderOptions = NULL;
  optionsShown = false;

  QTimer::singleShot(0, this, SLOT(embed()));
}

Embedder::~Embedder()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::~Embedder()"));
#endif

}

void Embedder::embed()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::embed()"));
#endif

#if defined(Q_WS_WIN)
	if ( embeddingWindow || releasingWindow )
		return;

	embeddingWindow = true;
	windowHandle = winId;
	RECT wR, cR;
	GetWindowRect(windowHandle, &wR);
	GetClientRect(windowHandle, &cR);
	ShowWindow(windowHandle, SW_HIDE);
	qApp->processEvents();
	originalRect = QRect(wR.left, wR.top, wR.right - wR.left, wR.bottom - wR.top);
	nativeResolution = QRect(cR.left, cR.top, cR.right - cR.left, cR.bottom - cR.top).size();
#endif

	// serious hack to access the tab bar without sub-classing from QTabWidget ;)
	QTabBar *tabBar = qmc2MainWindow->tabWidgetEmbeddedEmulators->findChild<QTabBar *>();
	int index = qmc2MainWindow->tabWidgetEmbeddedEmulators->indexOf(this);
	if ( tabBar ) {
		adjustIconSizes();
#if defined(Q_WS_X11)
		tabBar->setTabIcon(index, iconUnknown);
#elif defined(Q_WS_WIN)
		if ( !iconUnknown.isNull() )
			tabBar->setTabIcon(index, iconUnknown);
#endif
	}

#if defined(Q_WS_X11)
	nativeResolution = QPixmap::grabWindow(winId).size();
  	embedContainer->embedClient(winId);
#elif defined(Q_WS_WIN)
	fullScreen = false;
	embedded = true;
	embeddingWindow = false;
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("emulator #%1 embedded, window ID = %2").arg(gameID).arg((qulonglong)windowHandle));
	QTimer::singleShot(0, this, SLOT(updateWindow()));
	checkTimer.start(250);
#endif
}

void Embedder::release()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::release()"));
#endif

#if defined(Q_WS_WIN)
	SetWindowPos(windowHandle, HWND_BOTTOM, originalRect.x(), originalRect.y(), originalRect.width(), originalRect.height(), SWP_HIDEWINDOW);
#endif
	embedded = false;
	embedContainer->clearFocus();
#if defined(Q_WS_X11)
	embedContainer->discardClient();
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("emulator #%1 released, window ID = 0x%2").arg(gameID).arg(QString::number(winId, 16)));
#elif defined(Q_WS_WIN)
	checkTimer.stop();
	releasingWindow = true;
	if ( checkingWindow || updatingWindow ) {
		QTimer::singleShot(10, this, SLOT(release()));
		return;
	}
	SetParent(windowHandle, NULL);
	SetWindowLong(windowHandle, GWL_STYLE, QMC2_RELEASED_STYLE);
	qmc2MainWindow->raise();
	qmc2MainWindow->activateWindow();
	SetWindowPos(windowHandle, HWND_NOTOPMOST, originalRect.x(), originalRect.y(), originalRect.width(), originalRect.height(), SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_DRAWFRAME);
	UpdateWindow(windowHandle);
	windowHandle = winId = 0;
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("emulator #%1 released, window ID = %2").arg(gameID).arg((qulonglong)winId));
	releasingWindow = false;
#endif
}

#if defined(Q_WS_X11)
void Embedder::clientEmbedded()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::clientEmbedded()"));
#endif

  embedded = true;

  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("emulator #%1 embedded, window ID = 0x%2").arg(gameID).arg(QString::number(winId, 16)));

  // this works around a Qt bug when the tool bar is vertical and obscured by the emulator window before embedding
  QTimer::singleShot(0, qmc2MainWindow->toolbar, SLOT(update()));

  int myIndex = qmc2MainWindow->tabWidgetEmbeddedEmulators->indexOf(this);

  if ( qmc2FifoIsOpen ) {
    if ( isPaused )
      qmc2MainWindow->tabWidgetEmbeddedEmulators->setTabIcon(myIndex, iconPaused);
    else
      qmc2MainWindow->tabWidgetEmbeddedEmulators->setTabIcon(myIndex, iconRunning);
  } else
    qmc2MainWindow->tabWidgetEmbeddedEmulators->setTabIcon(myIndex, iconUnknown);

  forceFocus();
}

void Embedder::clientError(QX11EmbedContainer::Error error)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::clientError(QX11EmbedContainer::Error error = %1)").arg((int)error));
#endif

  switch ( error ) {
    case QX11EmbedContainer::InvalidWindowID:
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: embedder: invalid window ID = 0x%1").arg(QString::number(winId, 16)));
      break;

    case QX11EmbedContainer::Unknown:
    default:
      qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: embedder: unknown error, window ID = 0x%1").arg(QString::number(winId, 16)));
      break;
  }

  emit closing();
}
#endif

void Embedder::clientClosed()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::clientClosed()"));
#endif

#if defined(Q_WS_X11)
  if ( embedded )
    qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("emulator #%1 closed, window ID = 0x%2").arg(gameID).arg(QString::number(winId, 16)));
#endif

  embedded = false;

  emit closing();
}

void Embedder::closeEvent(QCloseEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::closeEvent(QCloseEvent *e = %1)").arg((qulonglong)e));
#endif

  if ( embedded )
    release();

  if ( embedderOptions )
    delete embedderOptions;

  QWidget::closeEvent(e);
}

void Embedder::showEvent(QShowEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::showEvent(QShowEvent *e = %1)").arg((qulonglong)e));
#endif

  if ( qmc2MainWindow->toolButtonEmbedderMaximizeToggle->isChecked() )
    QTimer::singleShot(0, qmc2MainWindow->menuBar(), SLOT(hide()));

#if defined(Q_WS_X11)
  if ( embedded )
    QTimer::singleShot(QMC2_EMBED_PAUSERESUME_DELAY, this, SLOT(showEventDelayed()));

  if ( !qmc2FifoIsOpen ) {
    int myIndex = qmc2MainWindow->tabWidgetEmbeddedEmulators->indexOf(this);
    qmc2MainWindow->tabWidgetEmbeddedEmulators->setTabIcon(myIndex, iconUnknown);
  }
#endif

  QWidget::showEvent(e);
}

void Embedder::hideEvent(QHideEvent *e)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::hideEvent(QHideEvent *e = %1)").arg((qulonglong)e));
#endif

#if defined(Q_WS_X11)
  if ( embedded )
    QTimer::singleShot(QMC2_EMBED_PAUSERESUME_DELAY, this, SLOT(hideEventDelayed()));
#endif

  QWidget::hideEvent(e);
}

void Embedder::resizeEvent(QResizeEvent *e)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::resizeEvent(QResizeEvent *e = %1)").arg((qulonglong)e));
#endif

	QWidget::resizeEvent(e);

	if ( embedded ) {
		embedContainer->resize(size());
#if defined(Q_WS_WIN)
		if ( windowHandle ) {
			MoveWindow(windowHandle, 0, 0, embedContainer->width(), embedContainer->height(), true);
			UpdateWindow(windowHandle);
		}
#endif
	}
}

#if defined(Q_WS_X11)
void Embedder::showEventDelayed()
{
	if ( isVisible() ) {
  		// gain focus
		QTimer::singleShot(QMC2_EMBED_FOCUS_DELAY, this, SLOT(forceFocus()));
		if ( qmc2MainWindow->toolButtonEmbedderAutoPause->isChecked() ) {
			resuming = true;
			resume();
		}
	}
}

void Embedder::hideEventDelayed()
{
	if ( !isVisible() ) {
		if ( qmc2MainWindow->toolButtonEmbedderAutoPause->isChecked() ) {
			pausing = true;
			pause();
		}
	}
}
#endif

void Embedder::toggleOptions()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::toggleOptions()"));
#endif

  optionsShown = !optionsShown;
  if ( optionsShown ) {
    gridLayout->setRowStretch(0, 1);
    gridLayout->setRowStretch(1, 4);
    gridLayout->setContentsMargins(cmLeft, cmTop, cmRight, cmBottom);
    if ( !embedderOptions ) {
      embedderOptions = new EmbedderOptions(this);
      embedderOptions->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
      gridLayout->addWidget(embedderOptions, 0, 0);
    }
    embedderOptions->show();
  } else {
    gridLayout->setRowStretch(0, 0);
    gridLayout->setRowStretch(1, 4);
    gridLayout->setContentsMargins(0, 0, 0, 0);
    if ( embedderOptions )
      embedderOptions->hide();
  }
#if defined(Q_WS_WIN)
  QTimer::singleShot(0, this, SLOT(updateWindow()));
#endif
}

void Embedder::adjustIconSizes()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: Embedder::adjustIconSizes()"));
#endif

  // serious hack to access the tab bar without sub-classing from QTabWidget ;)
  QTabBar *tabBar = qmc2MainWindow->tabWidgetEmbeddedEmulators->findChild<QTabBar *>();
  int index = qmc2MainWindow->tabWidgetEmbeddedEmulators->indexOf(this);

  QFont f;
  f.fromString(qmc2Config->value(QMC2_FRONTEND_PREFIX + "GUI/Font").toString());
  QFontMetrics fm(f);
  QToolButton *optionsButton = (QToolButton *)tabBar->tabButton(index, QTabBar::LeftSide);
  QToolButton *releaseButton = (QToolButton *)tabBar->tabButton(index, QTabBar::RightSide);
  int baseSize = fm.height() + 2;
  QSize optionsButtonSize(2 * baseSize, baseSize);
  QSize releaseButtonSize(baseSize, baseSize);
  optionsButton->setFixedSize(optionsButtonSize + QSize(0, 2));
  releaseButton->setFixedSize(releaseButtonSize);
  tabBar->setIconSize(optionsButtonSize);
}

void Embedder::forceFocus()
{
	if ( embedded ) {
		embedContainer->activateWindow();
		embedContainer->setFocus();
	} else {
		activateWindow();
		setFocus();
	}
}

#if defined(Q_WS_X11)

void Embedder::pause()
{
	if ( !isPaused ) {
		if ( resuming ) {
			// retry soon...
			QTimer::singleShot(QMC2_XKEYEVENT_TRANSITION_TIME, this, SLOT(pause()));
			return;
		}
		simulatePauseKey();
		QTimer::singleShot(2 * QMC2_XKEYEVENT_TRANSITION_TIME, this, SLOT(pause()));
	} else
		pausing = false;
}

void Embedder::resume()
{
	if ( isPaused ) {
		if ( pausing ) {
			// retry soon...
			QTimer::singleShot(QMC2_XKEYEVENT_TRANSITION_TIME, this, SLOT(resume()));
			return;
		}
		simulatePauseKey();
		QTimer::singleShot(2 * QMC2_XKEYEVENT_TRANSITION_TIME, this, SLOT(resume()));
	} else
		resuming = false;
}

void Embedder::simulatePauseKey()
{
	XKeyEvent xev;
	xev.display = QX11Info::display();
	xev.window = winId;
	xev.root = qApp->desktop()->winId();
	xev.subwindow = 0;
	xev.time = QX11Info::appTime();
	xev.x = xev.y = xev.x_root = xev.y_root = 1;
	xev.same_screen = true;
	xev.keycode = XKeysymToKeycode(xev.display, qmc2Config->value(QMC2_FRONTEND_PREFIX + "Embedder/PauseKey", Qt::Key_P).toInt());
	xev.state = 0;

	if ( pauseKeyPressed ) {
		xev.type = KeyRelease;
		XSendEvent(xev.display, xev.window, true, KeyReleaseMask, (XEvent *)&xev);
		pauseKeyPressed = false;
		XFlush(QX11Info::display());
	} else {
		xev.type = KeyPress;
		XSendEvent(xev.display, xev.window, true, KeyPressMask, (XEvent *)&xev);
		pauseKeyPressed = true;
		QTimer::singleShot(QMC2_XKEYEVENT_TRANSITION_TIME, this, SLOT(simulatePauseKey()));
	}
}

#elif defined(Q_WS_WIN)

void Embedder::checkWindow()
{
	if ( embeddingWindow || releasingWindow || updatingWindow )
		return;

	if ( !embedded || !windowHandle )
		return;

	checkingWindow = true;

	LONG currentStyle = GetWindowLong(windowHandle, GWL_STYLE);
	if ( currentStyle == 0 ) {
		qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("window ID for emulator #%1 lost, looking for replacement").arg(gameID));
		qApp->processEvents();
		HWND hwnd = NULL;
		int retries = 0;
		Q_PID gamePid = qmc2ProcessManager->getPid(gameID.toInt());
		while ( gamePid && !hwnd && retries++ < QMC2_MAX_WININFO_RETRIES ) {
#if defined(QMC2_EMUTYPE_MAME)
			hwnd = winFindWindowHandleOfProcess(gamePid, "MAME:");
#elif defined(QMC2_EMUTYPE_MESS)
			hwnd = winFindWindowHandleOfProcess(gamePid, "MESS:");
#endif
			if ( !hwnd )
				QTest::qWait(QMC2_WININFO_DELAY);
		}
		if ( releasingWindow || !embedded ) {
			checkingWindow = false;
			return;
		}
		if ( hwnd && hwnd != windowHandle ) {
			HWND oldHandle = windowHandle;
			windowHandle = winId = hwnd;
			ShowWindow(windowHandle, SW_HIDE);
			SetParent(windowHandle, embedContainer->winId());
			SetWindowLong(windowHandle, GWL_STYLE, QMC2_EMBEDDED_STYLE);
			MoveWindow(windowHandle, 0, 0, embedContainer->width(), embedContainer->height(), true);
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("using replacement window ID %1 for emulator #%2").arg((qulonglong)windowHandle).arg(gameID));
			UpdateWindow(windowHandle);
			ShowWindow(windowHandle, SW_SHOW);
			EnableWindow(windowHandle, true);
			SetFocus(windowHandle);
		} else {
			qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("no replacement window ID found for emulator #%1, closing embedder").arg(gameID));
			windowHandle = winId = 0;
			QTimer::singleShot(0, this, SLOT(clientClosed()));
		}
		fullScreen = false;
	} else if ( currentStyle != QMC2_EMBEDDED_STYLE ) {
		if ( currentStyle & WS_OVERLAPPEDWINDOW ) {
			fullScreen = false;
			QTimer::singleShot(0, this, SLOT(updateWindow()));
		} else {
			if ( !fullScreen ) {
				ShowWindow(windowHandle, SW_HIDE);
				SetParent(windowHandle, NULL);
				SetWindowLong(windowHandle, GWL_STYLE, currentStyle | WS_POPUP);
				MoveWindow(windowHandle, 0, 0, qApp->desktop()->width(), qApp->desktop()->height(), true);
				UpdateWindow(windowHandle);
				ShowWindow(windowHandle, SW_SHOW);
				EnableWindow(windowHandle, true);
				SetFocus(windowHandle);
			}
			fullScreen = true;
		}
	} else if ( GetFocus() != windowHandle ) {
		if ( QApplication::activeWindow() == qmc2MainWindow && qmc2MainWindow->tabWidgetGamelist->currentIndex() == QMC2_EMBED_INDEX )
			if ( qmc2MainWindow->tabWidgetEmbeddedEmulators->currentWidget() == this )
				QTimer::singleShot(0, this, SLOT(updateWindow()));
	}

	checkingWindow = false;
}

void Embedder::updateWindow()
{
	if ( embeddingWindow || releasingWindow || updatingWindow )
		return;

	updatingWindow = true;
	ShowWindow(windowHandle, SW_HIDE);
	SetParent(windowHandle, embedContainer->winId());
	SetWindowLong(windowHandle, GWL_STYLE, QMC2_EMBEDDED_STYLE);
	MoveWindow(windowHandle, 0, 0, embedContainer->width(), embedContainer->height(), true);
	EnableWindow(windowHandle, true);
	SetFocus(windowHandle);
	ShowWindow(windowHandle, SW_SHOW);
	updatingWindow = false;
}

#endif

#endif
