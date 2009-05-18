#include "detailsetup.h"
#include "macros.h"

#define QMC2_DEBUG

#ifdef QMC2_DEBUG
#include "qmc2main.h"
extern MainWindow *qmc2MainWindow;
#endif

DetailSetup::DetailSetup(QWidget *parent)
  : QDialog(parent)
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, QString("DEBUG: DetailSetup::DetailSetup(QWidget *parent = %1)").arg((qulonglong) parent));
#endif

  setupUi(this);

#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, tr("WARNING: the game/machine detail setup isn't working yet!"));
#endif
}

DetailSetup::~DetailSetup()
{
#ifdef QMC2_DEBUG
  qmc2MainWindow->log(QMC2_LOG_FRONTEND, "DEBUG: DetailSetup::~DetailSetup()");
#endif

}
