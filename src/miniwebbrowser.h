#ifndef MINIWEBBROWSER_H
#define MINIWEBBROWSER_H

#include <Qt>
#include <QWebEngineView>
#include <QMouseEvent>
#include <QTimer>
#include <QCache>

#include "ui_miniwebbrowser.h"
#include "iconlineedit.h"
#include "macros.h"

class MiniWebBrowser;

class BrowserWidget : public QWebEngineView
{
	Q_OBJECT

	public:
		QPoint lastMouseClickPosition;
		bool mouseCurrentlyOnView;
		QTimer bwuDelayTimer;
		MiniWebBrowser *parentBrowser;

		BrowserWidget(QWidget *parent, MiniWebBrowser *browserParent) : QWebEngineView(parent)
		{
			bwuDelayTimer.setSingleShot(true);
			lastMouseClickPosition = QPoint(-1, -1);
			mouseCurrentlyOnView = false;
			parentBrowser = browserParent;
		}

	signals:
		void mouseOnView(bool);

	protected:
		void mousePressEvent(QMouseEvent *e)
		{
			lastMouseClickPosition = e->pos();
			QWebEngineView::mousePressEvent(e);
		}
		void enterEvent(QEvent *e)
		{
			QWebEngineView::enterEvent(e);
			mouseCurrentlyOnView = true;
			emit mouseOnView(true);
		}
		void leaveEvent(QEvent *e)
		{
			QWebEngineView::leaveEvent(e);
			mouseCurrentlyOnView = false;
			emit mouseOnView(false);
		}
		void wheelEvent(QWheelEvent *);
		QWebEngineView *createWindow(QWebEnginePage::WebWindowType);
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
		QTimer searchTimer;
		BrowserWidget *webViewBrowser;
		QString currentTitle;
		IconLineEdit *iconLineEditSearch;

		MiniWebBrowser(QWidget *parent = 0, bool useAsPdfViewer = false);
		~MiniWebBrowser();

		bool isPdfViewer() { return m_isPdfViewer; }

	public slots:
		void on_comboBoxURL_activated(int);
		void comboBoxURL_activated() { on_comboBoxURL_activated(0); };
		void on_toolButtonHome_clicked();
		void on_toolButtonLoad_clicked();
		void on_toolButtonBack_clicked();
		void on_toolButtonForward_clicked();
		void on_toolButtonNext_clicked();
		void on_toolButtonPrevious_clicked();
		void on_toolButtonCaseSensitive_clicked();
		void on_toolButtonHighlight_clicked();
		void on_toolButtonToggleSearchBar_clicked();
		void on_spinBoxZoom_valueChanged(int);
		void changeTitle(QString &);
		void checkBackAndForward();
		void startSearchTimer() { searchTimer.start(QMC2_SEARCH_DELAY); }

		// page actions
		void processPageActionDownloadRequested(const QNetworkRequest &);
		void processPageActionHandleUnsupportedContent(QNetworkReply *);
		void postProcessPageActionInspect();

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
		void statusTimeout();
		void adjustIconSizes();
		void setStatus(QString);
		void clearStatus() { setStatus(QString()); }

	protected:
		void resizeEvent(QResizeEvent *);
		void moveEvent(QMoveEvent *);
		void hideEvent(QHideEvent *);

	signals:
		void titleChanged(QString &);

	private:
		QString m_statusMessage;
		bool m_isPdfViewer;
};

#endif
