#include "youtubebrowser.h"
#include "macros.h"
#include "qmc2main.h"

extern MainWindow *qmc2MainWindow;
extern QNetworkAccessManager *qmc2NetworkAccessManager;

YouTubeBrowser::YouTubeBrowser(QWidget *parent)
	: QWidget(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: YouTubeBrowser::YouTubeBrowser(QWidget *parent = %1)").arg((qulonglong) parent));
#endif

	setupUi(this);
}

YouTubeBrowser::~YouTubeBrowser()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: YouTubeBrowser::~YouTubeBrowser()");
#endif

}
