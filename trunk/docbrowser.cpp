#include "settings.h"
#include "docbrowser.h"
#include "qmc2main.h"
#include "macros.h"

// external global variables
extern MainWindow *qmc2MainWindow;
extern Settings *qmc2Config;

DocBrowser::DocBrowser(QWidget *parent)
#if defined(QMC2_OS_WIN)
	: QDialog(parent, Qt::Dialog)
#else
	: QDialog(parent, Qt::Dialog | Qt::SubWindow)
#endif
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: DocBrowser::DocBrowser(QWidget *parent = %1)").arg((qulonglong)parent));
#endif

	setupUi(this);

	browser = new MiniWebBrowser(this);
	verticalLayout->addWidget(browser);

	widgetSize = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/DocBrowser/Size", QSize(600, 600)).toSize();
	resize(widgetSize);

	widgetPos = qmc2Config->value(QMC2_FRONTEND_PREFIX + "Layout/DocBrowser/Pos", QPoint((parent->width() - width()) / 2, (parent->height() - height()) / 2)).toPoint();
	move(widgetPos);

	connect(browser, SIGNAL(titleChanged(QString &)), this, SLOT(titleChanged(QString &)));

#if defined(QMC2_OS_MAC)
	setParent(qmc2MainWindow, Qt::Dialog);
#endif
}

DocBrowser::~DocBrowser()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DocBrowser::~DocBrowser()");
#endif

	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/DocBrowser/Size", size());
	qmc2Config->setValue(QMC2_FRONTEND_PREFIX + "Layout/DocBrowser/Pos", pos());
}

void DocBrowser::titleChanged(QString &title)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DocBrowser::titleChanged(QString &title = ...");
#endif

	static QString currentTitle = "QMC2_NO_TITLE";

	if ( title == "QMC2_NO_TITLE" ) {
		setWindowTitle(tr("MiniWebBrowser"));
		currentTitle = "QMC2_NO_TITLE";
	} else {
		if ( title.isEmpty() ) {
			if ( currentTitle == "QMC2_NO_TITLE" )
				setWindowTitle(tr("MiniWebBrowser"));
			else
				setWindowTitle(tr("MiniWebBrowser") + " :: " + currentTitle);
		} else {
			currentTitle = title;
			setWindowTitle(tr("MiniWebBrowser") + " :: " + currentTitle);
		}
	}
}
