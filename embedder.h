#ifndef _EMBEDDER_H_
#define _EMBEDDER_H_

#include <Qt>

#if defined(Q_WS_X11) || defined(Q_WS_WIN)

#if defined(Q_WS_X11)
#include <QX11EmbedContainer>
#include "embedderopt.h"
#elif defined(Q_WS_WIN)
#include <QtGui>
#include <QTimer>
#include "embedderopt.h"
#endif

class Embedder : public QWidget
{
	Q_OBJECT

	public:
		bool embedded;
		bool optionsShown;
		WId winId;
#if defined(Q_WS_X11)
		QX11EmbedContainer *embedContainer;
#elif defined(Q_WS_WIN)
		QWidget *embedContainer;
#endif
		EmbedderOptions *embedderOptions;
		QGridLayout *gridLayout;
		QString gameName;
		QString gameID;
		QSize nativeResolution;
		int cmLeft, cmTop, cmRight, cmBottom;
#if defined(Q_WS_X11)
		bool pauseKeyPressed;
		bool isPaused;
		bool resuming;
		bool pausing;
#endif
		QIcon iconRunning;
		QIcon iconPaused;
		QIcon iconStopped;
		QIcon iconUnknown;

		Embedder(QString name, QString id, WId wid, bool currentlyPaused = false, QWidget *parent = 0, QIcon icon = QIcon());
		~Embedder();

	public slots:
		void embed();
		void embed(WId wid) { winId = wid; embed(); }
		void release();
#if defined(Q_WS_X11)
		void clientEmbedded();
		void clientError(QX11EmbedContainer::Error);
#endif
		void clientClosed();
		void toggleOptions();
		void adjustIconSizes();
		void forceFocus();
#if defined(Q_WS_X11)
		void simulatePauseKey();
		void pause();
		void resume();
		void showEventDelayed();
		void hideEventDelayed();
#endif

	protected:
		void closeEvent(QCloseEvent *);
		void showEvent(QShowEvent *);
		void hideEvent(QHideEvent *);
		void resizeEvent(QResizeEvent *);

	signals:
		void closing();

#if defined(Q_WS_WIN)
	private slots:
		void checkWindow();
		void updateWindow();

	private:
		QRect originalRect;
		QTimer checkTimer;
		HWND windowHandle;
		QString windowTitle;
		bool embeddingWindow;
		bool releasingWindow;
		bool checkingWindow;
		bool updatingWindow;
		bool fullScreen;
#endif
};

#endif

#endif
