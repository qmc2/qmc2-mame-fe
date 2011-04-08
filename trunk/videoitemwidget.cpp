#include "macros.h"
#include "videoitemwidget.h"

#define QMC2_DEBUG

#ifdef QMC2_DEBUG
#include "qmc2main.h"
extern MainWindow *qmc2MainWindow;
#endif

VideoItemWidget::VideoItemWidget(QImage &vImage, QString &vDescription, QWidget *parent)
  : QWidget(parent)
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: VideoItemWidget::VideoItemWidget(QImage &vImage = ..., QString &vDescription = ..., QWidget *parent = %1)").arg((qulonglong) parent));
#endif

	setupUi(this);

	videoImage = vImage;
	videoDescription = vDescription;
}

VideoItemWidget::~VideoItemWidget()
{
#ifdef QMC2_DEBUG
	qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: VideoItemWidget::~VideoItemWidget()");
#endif

}
