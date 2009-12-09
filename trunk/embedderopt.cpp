#include "embedderopt.h"

#ifdef QMC2_DEBUG
#include "qmc2main.h"
#include "macros.h"
extern MainWindow *qmc2MainWindow;
#endif

EmbedderOptions::EmbedderOptions(QWidget *parent)
  : QWidget(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: EmbedderOptions::EmbedderOptions(QWidget *parent = %1)").arg((qulonglong) parent));
#endif

  setupUi(this);

}

EmbedderOptions::~EmbedderOptions()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: EmbedderOptions::~EmbedderOptions()");
#endif

}
