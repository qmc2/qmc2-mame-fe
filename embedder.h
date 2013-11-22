#ifndef _EMBEDDER_H_
#define _EMBEDDER_H_

#include <Qt>
#include "macros.h"

#if (defined(QMC2_OS_UNIX) && QT_VERSION < 0x050000) || defined(QMC2_OS_WIN)

#if defined(QMC2_OS_UNIX)
#include <QX11EmbedContainer>
#include "embedderopt.h"
#elif defined(QMC2_OS_WIN)
#include <QtGui>
#include <QTimer>
#include "embedderopt.h"
#endif

class Embedder;

class EmbedContainer : public QWidget
{
	Q_OBJECT

	public:
		EmbedContainer(QWidget *parent = 0);

	protected:
		void resizeEvent(QResizeEvent *);

	private:
		Embedder *embedder;
};

class Embedder : public QWidget
{
	Q_OBJECT

	friend class EmbedContainer;

	public:
		bool embedded;
		bool optionsShown;
		WId embeddedWinId;
		EmbedContainer *embedContainer;
		EmbedderOptions *embedderOptions;
		QGridLayout *gridLayout;
		QString gameName;
		QString gameID;
		QSize nativeResolution;
		int cmLeft, cmTop, cmRight, cmBottom;
#if defined(QMC2_OS_UNIX)
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

	public slots:
		void embed();
		void embed(WId wid) { embeddedWinId = wid; embed(); }
		void release();
		void clientClosed();
		void toggleOptions();
		void adjustIconSizes();
		void forceFocus();
#if defined(QMC2_OS_UNIX)
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

	signals:
		void closing();

#if defined(QMC2_OS_WIN)
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
